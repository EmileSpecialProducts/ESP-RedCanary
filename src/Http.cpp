
#include "canary.hpp"
#include <lwip/sockets.h>
WiFiServer HttpServer(443);
WiFiServer HttpaServer(8080);
String HttpHookweb = "";
String HttpsHookweb = "";
String HttpaHookweb = "";
bool Httpenabled = false;
bool Httpsenabled = false;
bool Httpaenabled = false;
// https://github.com/me-no-dev/ESPAsyncTCP/blob/master/examples/ClientServer/Server/Server.ino

void setup_http()
{
    Httpenabled=Settingsdoc["Http"]["enable"] | false;
    Httpsenabled=Settingsdoc["Https"]["enable"] | false;
    Httpaenabled=Settingsdoc["Httpa"]["enable"] | false;
    
    HttpHookweb=Settingsdoc["Http"]["url"] | "";
    HttpsHookweb=Settingsdoc["Https"]["url"] | "";
    HttpaHookweb=Settingsdoc["Httpa"]["url"] | "";
    if(Httpsenabled)HttpServer.begin();
    if(Httpaenabled)HttpaServer.begin();
}
void loop_http()
{
if(Httpsenabled)
{
    /* ---------- Http on 443 (no TLS) ---------- */
    if (WiFiClient c = HttpServer.accept())
    handleBannerGrab(HttpsHookweb,
      c, 443,
      "HTTP/1.1 200 OK\r\n"
      "Server: Apache/2.4.52 (Debian)\r\n"
      "Content-Type: text/html\r\n"
      "Content-Length: 44\r\n\r\n"
      "<html><body><h1>It works!</h1></body></html>");
}
if(Httpaenabled) 
  {
    /* ---------- Http alternative (no TLS) ---------- */
    if (WiFiClient c = HttpaServer.accept())
    {
    handleBannerGrab(HttpaHookweb,
      c, 8080,
      "HTTP/1.1 200 OK\r\n"
      "Server: Apache/2.4.52 (Debian)\r\n"
      "Content-Type: text/html\r\n"
      "Content-Length: 44\r\n\r\n"
      "<html><body><h1>It works!</h1></body></html>");
      //c.clear();
      c.stop();
      //HttpaServer.close();
      //closesocket(c);
    }
  }
}