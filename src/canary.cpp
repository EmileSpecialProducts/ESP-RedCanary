//
// this is based on the work that https://github.com/7h30th3r0n3/NanoC6-ESP32-Honeypot did

#include "canary.hpp"

const char* logPath = "/honeypot_logs.txt";
const char* OldlogPath = "/honeypot_Old.txt";
String  WebhookURL;

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

void logCommand(String webHook, String ip, uint16_t port, String command) {
  //return; // Disable logging and canary for testing purposes
  bool Whitelisted = false;
  Whitelisted = IsWhitelisted(ip);
  debugln(getFormattedDateTime(timeClient.getEpochTime()) +" [" + String(millis()) + "] IP: " + ip + " - Port: " + String(port) + " Whitelisted "+ Whitelisted+" - Command: " + command);  
  if(LogWhitelisted() || !Whitelisted) 
    { 
      
      size_t maxfilesize= LittleFS.totalBytes()/3; // limit log file to 1/3 of total FS size
      File logFile = LittleFS.open(logPath, FILE_APPEND);
      debugf("logfile size: %d / %d\n", logFile.size(), maxfilesize);
      
      if( logFile.size()> maxfilesize ) {
        logFile.close();
        LittleFS.remove(OldlogPath);
        LittleFS.rename(logPath, OldlogPath); // archive old log
        logFile = LittleFS.open(logPath, FILE_APPEND);
      }
      if (!logFile) return;

      logFile.println(getFormattedDateTime(timeClient.getEpochTime()) +" [" + String(millis()) + "] IP: " + ip + " - Port: " + String(port) + " - Command: " + command);
      logFile.close();

      debugln("IP: " + ip + " | Port: " + String(port) + " | CMD: " + command + "|Escaped " + command);
    }

    if(CanryWhitelisted()|| !Whitelisted)
    { 
      if(webHook.length() == 0)
      {
          webHook= Settingsdoc["canarytokenURL"].is<const char*>() ? String(Settingsdoc["canarytokenURL"].as<const char*>()) : String("");
      }
      if(webHook.length() == 0){
          debugln("No Canary Webhook URL configured");
          return;
      }
      
      debug("Canary Webhook URL: ");
      String url=webHook + "?SourceIP=" + ip + "&TargetIP="+ WiFi.localIP().toString() +"&TargetHost="+host +"&port=" + String(port) + "&command=" + command;
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
      
    #ifdef __NOT_IN_USE__   
        http.begin(webHook);
        http.addHeader("Content-Type", "application/json");
      // String msg = "{\"content\":\"📡 **Honeypot**\\n🔍 IP: " + ip +
      //              "\\n📌 Port: " + String(port) +
      //              "\\n💻 Command: " + escapeJSON(command) +
      //              "\\n__________________________\"}";

        String msg = "{\"content\":\"Honeypot\",\"IP\":\"" + ip +"\"" +
                    ",\"Port\":" + String(port) +
                    ",\"Command\":"  + "\"SSH\"" +
                    "}";
        int httpCode =http.POST(msg);
        debugf("Webhook sent to %s\n", webHook.c_str());
        debugf("Payload: %s\n", msg.c_str());
        debugf("HTTP response code: %d\n", httpCode);
        debug( http.getString());
        http.end();
    #endif
      }  
    }
  
  /* ToDo 
  for (int i = 0; i < 2; i++) {
    strip.setPixelColor(0, strip.Color(255, 0, 0)); strip.show(); delay(150);
    strip.setPixelColor(0, strip.Color(0, 0, 0)); strip.show(); delay(150);
  }
  strip.setPixelColor(0, strip.Color(255, 0, 0)); strip.show();
  */
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

void CanarySetup()
{
    //startHoneypot();
    setup_ssh();
    setup_http();
    setup_rdp();
    setup_ftp();
    setup_telnet();
    setup_Modbus();
}

void CanaryLoop()
{
  loop_ssh();
  loop_http();
  loop_rdp();
  loop_ftp();
  loop_telnet();
  loop_Modbus();
    //honeypotLoop(); 
}
