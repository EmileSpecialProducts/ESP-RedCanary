#include <canary.hpp>
AsyncWebServer *server=nullptr;
File uploadFile;

void reply(AsyncWebServerRequest *request, int code, const char *type, const uint8_t *data, size_t len)
{
    debugf("reply Len = %d code = %d Type= %s\n ", len, code, type);  
    //request->send(code, type, data, len);
        AsyncWebServerResponse *response =
            request->beginResponse(code, type, data, len);
            request->contentLength();
        // response->addHeader("Content-Encoding", "gzip");
        // response->addHeader("Content-Encoding", "7zip");
        request->send(response);
}

void reply(AsyncWebServerRequest *request, int code, const char *type, String &data)
{
    debugf("reply Len = %d code = %d Type= %s\n ", data.length(), code, type);  
    //request->send(code, type, data);
        AsyncWebServerResponse *response =
            request->beginResponse(code, type, data);
            request->contentLength();
        // response->addHeader("Content-Encoding", "gzip");
        // response->addHeader("Content-Encoding", "7zip");
        request->send(response);
}


String urlDecode(const String &text)
{
  String decoded = "";
  char temp[] = "0x00";
  unsigned int len = text.length();
  unsigned int i = 0;
  while (i < len)
  {
    char decodedChar;
    char encodedChar = text.charAt(i++);
    if ((encodedChar == '%') && (i + 1 < len))
    {
      temp[2] = text.charAt(i++);
      temp[3] = text.charAt(i++);
      decodedChar = strtol(temp, NULL, 16);
    }
    else
    {
      if (encodedChar == '+')
      {
        decodedChar = ' ';
      }
      else
      {
        decodedChar = encodedChar; // normal ascii char
      }
    }
    decoded += decodedChar;
  }
  return decoded;
}

void Log(String Str)
{
  debugln(Str);
  File LogFile = LittleFS.open("/log.txt", FILE_APPEND);
  LogFile.println(Str);
  LogFile.close();
}
extern bool WiFiAPMode;
void setup_webserver()
{
  int Portnumber = Settingsdoc["ServerPort"].is<const int>() ? (int)Settingsdoc["ServerPort"].as<const int>() : (int)80; 
  debugf("Setup WebServer on Port %d\n", Settingsdoc["ServerPort"].as<const int>());
  debugf("Starting WebServer on Port %d\n", Portnumber);
  server = new AsyncWebServer(Portnumber);
  if(server==nullptr){ debugln("Failed to create AsyncWebServer"); return; }
  server->on("/diskinfo", AsyncWebRequestMethod::HTTP_GET, [](AsyncWebServerRequest *request)
    {
      String output="";
        output+= "{\"totalBytes\":" + String(LittleFS.totalBytes())+",";
        output+= "\"usedBytes\":" + String(LittleFS.usedBytes())+","; 
        output+= "\"freeBytes\":" + String(LittleFS.totalBytes()-LittleFS.usedBytes())+"}";            
        request->send(200, "text/json", output);
    });

server->on("/list", AsyncWebRequestMethod::HTTP_GET, [](AsyncWebServerRequest *request)
    {
        if (!request->hasArg("dir"))
          return request->send(500, "text/plain", "BAD ARGS\r\n"); 
        String path = request->arg("dir");
        String output="[";
        if(path != "/" && !LittleFS.exists(path)) return request->send(500, "text/plain", "BAD PATH\r\n"); 
        File dir = LittleFS.open(path);
        path = String();
        if (!dir.isDirectory())
        {
          dir.close();
          return request->send(500, "text/plain", "NOT DIR\r\n"); 
        }
        dir.rewindDirectory();
        for (int cnt = 0; true; ++cnt)
        {
          File entry = dir.openNextFile();
          if (!entry)
            break;          
          output += String(cnt++>0?",":"") +"{\"type\":\"" + String((entry.isDirectory()) ? "dir" : "file")+"\"" 
          + ",\"name\":\"" + String(entry.name())+"\"" 
          + String((entry.isDirectory()) ?"":",\"size\":\"" + String(entry.size())+"\"") 
          + "}";
          entry.close();
        }
        output+="]";
        request->send(200, "text/json", output);
        dir.close();
      });
      
server->on("/edit", AsyncWebRequestMethod::HTTP_PUT, [](AsyncWebServerRequest *request)
{
  debugln("Create ");
  if (request->args() == 0)
  return request->send(500, "text/plain", "BAD ARGS\r\n");  
  String path = request->arg(0);
  debugln("Create: " + path);
  if (path == "/" || LittleFS.exists(path))
  {
    request->send(500, "text/plain", "BAD PATH\r\n"+ path); 
    return;
  }
  if(path.indexOf('.') > 0){
    debugln("CreateFile: " + path);
    File file = LittleFS.open(path, "w");
    if (file)
    {
      file.write((const uint8_t *)" ", 1); // must write one char
      file.close();
    }
  }
  else
  {
    debugln("CreateDir: " + path);
    LittleFS.mkdir(path);
  }
  request->send(200, "text/plain", ""); 
});

server->on("/edit", AsyncWebRequestMethod::HTTP_DELETE, [](AsyncWebServerRequest *request)
{
  debugln("Delete ");
  if (request->args() == 0)
    return request->send(500, "text/plain", "BAD ARGS\r\n");  
  String path = request->arg(0);
  debugln("Delete: " + path);
  if(path.indexOf('.') > 0){
    if (path == "/" || !LittleFS.exists(path))
    {
      request->send(500, "text/plain", "BAD PATH\r\n" + path); 
      return;
    }
    debugln("Delete file "+path);
    
    LittleFS.remove(path);
    request->send(200, "text/plain", "");
  } else
  {
    debugln("Delete Dir "+path);
    LittleFS.rmdir(path);
    request->send(200, "text/plain", "");
  } 
});

server->on("/edit", AsyncWebRequestMethod::HTTP_POST, 
        [](AsyncWebServerRequest *request)
          {
            //debugln("File upload completed " + request->url());
            //request->send(200, "text/plain; chartset=\"UTF-8\"", "File upload completed");
            //request->send(200);
            request->redirect("/"); 
          }, 
        [](AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final)
          {
            //debugf("Upload[%s]: start=%u, len=%u, final=%d\n", filename.c_str(), index, len, final);
            if (!index) {
            request->_tempFile = LittleFS.open("/"+ filename, "w+");
            }
            if (len) request->_tempFile.write(data, len);
            if (final) {
            request->_tempFile.close();
            } 
          }
    );
    server->on("/save-config", AsyncWebRequestMethod::HTTP_POST , [](AsyncWebServerRequest *request){
    // Handle POST in body
    }, NULL, [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
        JsonDocument doc;
        DeserializationError error = deserializeJson(doc, data);
        if (!error) {
            
        File file = LittleFS.open(JSONCONFIGFILE, FILE_WRITE);
        if (file) {
            //serializeJson(doc, file);
            serializeJsonPretty(doc, file);
            file.close();
            request->send(200, "text/plain", "Saved");
        } else {
            request->send(500, "text/plain", "Failed to open file");
        }
        } else {
        request->send(400, "text/plain", "Invalid JSON");
        }
      });
    server->on("/reboot", AsyncWebRequestMethod::HTTP_GET, [](AsyncWebServerRequest *request)
    {
        AsyncWebServerResponse *response =
            request->beginResponse(200, "text/html", "Rebooting");
            request->contentLength();
            request->send(response);
            Startreboot=true;
            debugln("Rebooting...");
    });
    server->on("/wifi-config", AsyncWebRequestMethod::HTTP_GET, [](AsyncWebServerRequest *request)
    {
        JsonDocument Wifidoc;
            scanWiFiNetworks(Wifidoc); 
            debug("scanWiFiNetworks: ");
            serializeJson(Wifidoc, Serial);
            debugf("Wifidoc.size()= %d\n",Wifidoc.size());
          if( Wifidoc.size() == 0 )
            return request->send(500, "text/plain", "SCAN FAILED\r\n");
          else
            {  String response;
                serializeJson(Wifidoc, response);
                reply(request, 200, "application/json", response);
                return;
                //return request->send(200, "application/json", response);
            }
      });
      
    server->on("/", AsyncWebRequestMethod::HTTP_GET, [](AsyncWebServerRequest *request)
    { request->redirect("/index.html"); });
    
    server->onNotFound([](AsyncWebServerRequest *request)
    { 
        debugf("url NotFound %s , Method =%s\n", request->url().c_str(), request->methodToString());
        if (request->method() == HTTP_GET)
        {
            if (LittleFS.exists(request->url())) // exists will give a error in the error log see: https://github.com/espressif/arduino-esp32/issues/7615
            {
                request->send(LittleFS, request->url(), String(), false);
            }
            else
            {
            if (request->url() == "/index.html")
                    reply(request, 200, "text/html", Index_html, sizeof(Index_html) - 1);
            else if (request->url() == "/style.css")
                    reply(request, 200, "text/css", style_css, sizeof(style_css) - 1);        
            else if (request->url() == "/editor.html")
                    reply(request, 200, "text/html", editor_html, sizeof(editor_html) - 1);        
            else if (request->url() == "/error404.html")
                    reply(request, 404, "text/html", error404_html, sizeof(error404_html) - 1);
            else 
                    {
                      if(WiFiAPMode == true)
                        request->redirect("/index.html");
                      else
                        request->redirect("/error404.html");
                    }
            }
        }
        else if (request->method() == AsyncWebRequestMethod::HTTP_POST)
        {
            reply(request, 404, "text/html", error404_html, sizeof(error404_html) - 1);
        }
    });
 
  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Origin", "*"); 
  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Methods", "GET, PUT, POST, DELETE, HEAD");
  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Headers", "content-type");
  server->begin();  
  debugln("HTTP server started");
  }

