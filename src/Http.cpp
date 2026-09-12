
#include "canary.hpp"
#include <lwip/sockets.h>
AsyncWebServer *HttpServer=nullptr;
AsyncWebServer *HttpsServer=nullptr;
AsyncWebServer *HttpaServer=nullptr;

String HttpHookweb = "";
String HttpsHookweb = "";
String HttpaHookweb = "";
bool Httpenabled = false;
bool Httpsenabled = false;
bool Httpaenabled = false;
// https://github.com/me-no-dev/ESPAsyncTCP/blob/master/examples/ClientServer/Server/Server.ino

void HandelNotFount(AsyncWebServerRequest *request, String Directory, String webHook)
    { 
        String ip = request->client()->remoteIP().toString();
        String payload = request->url();//dumpBytes(request->client());
        int port = request->client()->localPort();
        logCommand(webHook,ip, port, payload);
        
        debugf("HandelNotFount %s , Method =%s\n", request->url().c_str(), request->methodToString());
        debugf("%s\n", (Directory + request->url()).c_str());
        if (request->method() == AsyncWebRequestMethod::HTTP_GET)
        {
            if (FILESYSTEM.exists(Directory + request->url())) // exists will give a error in the error log see: https://github.com/espressif/arduino-esp32/issues/7615
            {
                request->send(FILESYSTEM, Directory + request->url(), String(), false);
            }
            else
            {
            if (request->url() == "/error404.html")
                    request->send( 404, "text/html", error404_html, sizeof(error404_html) - 1);
            else 
                    request->redirect("/error404.html");
            }
        }
        else if (request->method() == AsyncWebRequestMethod::HTTP_POST)
        {
            request->send( 404, "text/html", error404_html, sizeof(error404_html) - 1);
        }
    }

void HandelNotFountHTTP(AsyncWebServerRequest *request)
{
  HandelNotFount(request,"/http",HttpHookweb);
}
void HandelNotFountHTTPS(AsyncWebServerRequest *request)
{
  HandelNotFount(request,"/https",HttpsHookweb);
}
void HandelNotFountHTTPA(AsyncWebServerRequest *request)
{
  HandelNotFount(request,"/httpa",HttpaHookweb);
}

void setup_http()
{
    Httpenabled=Settingsdoc["Http"]["enable"] | false;
    Httpsenabled=Settingsdoc["Https"]["enable"] | false;
    Httpaenabled=Settingsdoc["Httpa"]["enable"] | false;
    
    HttpHookweb=Settingsdoc["Http"]["url"] | "";
    HttpsHookweb=Settingsdoc["Https"]["url"] | "";
    HttpaHookweb=Settingsdoc["Httpa"]["url"] | "";
  //DefaultHeaders::Instance().addHeader("Access-Control-Allow-Origin", "*"); 
  //DefaultHeaders::Instance().addHeader("Access-Control-Allow-Methods", "GET, PUT, POST, DELETE, HEAD");
  //DefaultHeaders::Instance().addHeader("Access-Control-Allow-Headers", "content-type");

  if(Httpenabled)
  {
    HttpServer=new AsyncWebServer(80);
    HttpServer->on("/", AsyncWebRequestMethod::HTTP_GET, [](AsyncWebServerRequest *request)
    { request->redirect("/index.html"); });
    HttpServer->onNotFound(HandelNotFountHTTP);
    HttpServer->begin();
  } 
  if(Httpsenabled)
  {
    HttpsServer=new AsyncWebServer(443);
    HttpsServer->on("/", AsyncWebRequestMethod::HTTP_GET, [](AsyncWebServerRequest *request)
    { request->redirect("/index.html"); });
    HttpsServer->onNotFound(HandelNotFountHTTPS);
    HttpsServer->begin();
  }
  if(Httpaenabled)
  {
    HttpaServer=new AsyncWebServer(8080);
    HttpaServer->on("/", AsyncWebRequestMethod::HTTP_GET, [](AsyncWebServerRequest *request)
    { request->redirect("/index.html"); });
    HttpaServer->onNotFound(HandelNotFountHTTPA);
    HttpaServer->begin();
  }
}
void loop_http()
{
}