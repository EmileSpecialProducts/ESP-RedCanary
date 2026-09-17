#include "canary.hpp"

const char* logPath = "/honeypot_logs.txt";
const char* OldlogPath = "/honeypot_Old.txt";

bool Loging_enable=false;
int LogFileNumbers =3;
size_t  LogFileSize=100000;
String Loging_file = "canarylog";
String  WebhookURL;

void setup_logging()
{
  Loging_enable =Settingsdoc["Loging"]["enable"] | false;
  LogFileSize   =Settingsdoc["Loging"]["LogFileSize"] | 100000;
  LogFileNumbers=Settingsdoc["Loging"]["LogFileNumbers"] | 3;
  Loging_file   =Settingsdoc["Loging"]["Loging_file"] | "canarylog";
  if(Loging_file[0]!='/')Loging_file="/"+Loging_file;
}

String dumpBytes(WiFiClient &c, size_t maxLen , uint32_t timeout ) {
  String s;
  unsigned long t0 = millis();
  while (millis() - t0 < timeout && s.length() < maxLen && c.connected()) {
    while (c.available() && s.length() < maxLen) {
      uint8_t b = c.read();
      if (isprint(b) || b == '\r' || b == '\n')
        s += (char)b;
      else {
        char buf[5];
        sprintf(buf, "\\x%02X", b);
        s += buf;
      }
    }
    delay(1);
  }
  return s.length() ? s : "(no data)";
}

String escapeJSON(String s) {
  String result = "";

  for (unsigned int i = 0; i < s.length(); i++) {
    char c = s[i];
    // Autoriser seulement les caractères imprimables + retour chariot et saut de ligne
    if (c >= 32 || c == '\n' || c == '\r') {
      switch (c) {
        case '\\': result += "\\\\"; break;
        case '\"': result += "\\\""; break;
        case '\n': result += "\\n"; break;
        case '\r': result += "\\r"; break;
        default: result += c; break;
      }
    }
  }

  return result;
}

void LogTofile(String webHook, String ip, uint16_t port, String command,String Protocol) {
  debug("Logfile");
  debug(Loging_enable);
  debug(Loging_file);
  debug(LogFileNumbers);
  debugln(LogFileSize);
  static String Extention=".log"; 
  size_t maxFileSize = LogFileSize;
  int backupCount = LogFileNumbers-1 > 0 ? LogFileNumbers-1 : 0;
  File logFile = FILESYSTEM.open(Loging_file+Extention, FILE_APPEND);

  if (!logFile) return;
  if (maxFileSize > 0 && logFile.size()  > maxFileSize) {
    logFile.close();

    if (backupCount > 0) {
      for (int index = backupCount; index > 1; --index) {
        String currentBackup = String(Loging_file) + "." + String(index - 1)+Extention;
        String nextBackup = String(Loging_file) + "." + String(index)+Extention;
        FILESYSTEM.remove(nextBackup);
        FILESYSTEM.rename(currentBackup, nextBackup);
      }

      String firstBackup = String(Loging_file) + ".1" + Extention;
      FILESYSTEM.remove(firstBackup);
      FILESYSTEM.rename(Loging_file+Extention, firstBackup);
    } else {
      FILESYSTEM.remove(Loging_file+Extention);
    }

    logFile = FILESYSTEM.open(Loging_file+Extention, FILE_APPEND);
    if (!logFile) return;
  }
  {
    String logline;
    logline.reserve(150);
    logline = getFormattedDateTime(timeClient.getEpochTime()); 
    logline += " [" + String(millis()) + "]";
    logline += "IP: " + ip + " - Port: " + String(port);
    if(Protocol.length() >0) logline += " - Protocol: " + Protocol;
    logline += " - Command: " + command;
    logFile.println(logline );
  }
  logFile.close();
}

void LogToCanary(String webHook, String ip, uint16_t port, String command,String Protocol) {  
  if(webHook.length() == 0)
  {
      webHook= Settingsdoc["canarytokenURL"].is<const char*>() ? String(Settingsdoc["canarytokenURL"].as<const char*>()) : String("");
  }
  if(webHook.length() == 0){
      debugln("No Canary Webhook URL configured");
      return;
  }
  
  debug("Canary Webhook URL: ");
  String url=webHook + "?SourceIP=" + ip + "&TargetIP="+ WiFi.localIP().toString() +"&TargetHost="+host +"&port=" + String(port) + "&command=" + command +"&Protocol=" +Protocol;
  debugln(url);
  if (WiFi.status() == WL_CONNECTED && webHook.length() > 0) {
    HTTPClient http;
    http.begin(url);
    int httpCode =http.GET();
    debugf("Webhook sent to %s\n", webHook.c_str());
    debugf("Payload: %s\n", url.c_str());
    debugf("HTTP response code: %d\n", httpCode);
    debug( http.getString());
    http.end();
  }  
}
 
void logCommand(String webHook, String ip, uint16_t port, String command,String Protocol) 
{
  bool Whitelisted = false;
  Whitelisted = IsWhitelisted(ip);
  debugln(getFormattedDateTime(timeClient.getEpochTime()) +" [" + String(millis()) + "] IP: " + ip + " - Port: " + String(port) + " Whitelisted "+ Whitelisted+" - Command: " + command);      
 
  if(!Whitelisted)
  {
    LogTofile( webHook,  ip,  port,  command, Protocol);
    LogToCanary( webHook,  ip,  port,  command, Protocol);
  }
}
void handleBannerGrab(String webHook,WiFiClient client, uint16_t port, const uint8_t* banner, size_t len) {
  if (!client.connected()) return;
  String ip = client.remoteIP().toString();
  String payload = dumpBytes(client);
  client.write(banner, len);
  delay(50);
  logCommand(webHook,ip, port, payload);
  //client.stop();
}

void handleBannerGrab(String webHook,WiFiClient client, uint16_t port, const char* banner) {
  handleBannerGrab(webHook,client, port, (const uint8_t*) banner,strlen(banner));
}
