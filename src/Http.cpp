
#include "canary.hpp"
#include <lwip/sockets.h>
AsyncWebServer *HttpServer=nullptr;
AsyncWebServer *HttpsServer=nullptr;
AsyncWebServer *HttpaServer=nullptr;
uint16_t HttpaPort=8080;

String HttpHookweb = "";
String HttpsHookweb = "";
String HttpaHookweb = "";
bool Httpenabled = false;
bool Httpsenabled = false;
bool Httpaenabled = false;

String HttpMCPHookweb = "";
String HttpsMCPHookweb = "";
String HttpaMCPHookweb = "";
bool HttpMCPenabled = false;
bool HttpsMCPenabled = false;
bool HttpaMCPenabled = false;

// https://github.com/me-no-dev/ESPAsyncTCP/blob/master/examples/ClientServer/Server/Server.ino

const char *McpContentType = "application/json";

void sendMcpResponse(AsyncWebServerRequest *request, JsonDocument &response)
{
  String payload;
  serializeJson(response, payload);
  AsyncWebServerResponse *httpResponse = request->beginResponse(200, McpContentType, payload);
  httpResponse->addHeader("Mcp-Session-Id", String((uint32_t)millis(), HEX));
  request->send(httpResponse);
}

void sendMcpError(AsyncWebServerRequest *request, int code, const char *message)
{
  JsonDocument response;
  response["jsonrpc"] = "2.0";
  response["error"]["code"] = code;
  response["error"]["message"] = message;
  sendMcpResponse(request, response);
}

void handleMcpRequest(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total, String webHook,String Protocol="")
{
  String *body = static_cast<String *>(request->_tempObject);
  if (index == 0) {
    delete body;
    body = new String();
    body->reserve(total);
    request->_tempObject = body;
  }
  if (body == nullptr) {
    request->send(500, "text/plain", "MCP request buffer unavailable");
    return;
  }

  body->concat(reinterpret_cast<const char *>(data), len);
  if (index + len < total) return;

  String payload = *body;
  delete body;
  request->_tempObject = nullptr;

  String ip = request->client()->remoteIP().toString();
  logCommand(webHook, ip, request->client()->localPort(), payload,Protocol);

  JsonDocument input;
  DeserializationError error = deserializeJson(input, payload);
  if (error) {
    sendMcpError(request, -32700, "Parse error");
    return;
  }

  const char *method = input["method"] | "";
  if (!input["id"].isNull() && !input["id"].is<const char *>() && !input["id"].is<int>() && !input["id"].is<long>()) {
    sendMcpError(request, -32600, "Invalid request id");
    return;
  }

  if (input["id"].isNull()) {
    request->send(202, "text/plain", "");
    return;
  }

  JsonDocument response;
  response["jsonrpc"] = "2.0";
  response["id"] = input["id"];

  if (strcmp(method, "initialize") == 0) {
    response["result"]["protocolVersion"] = "2025-06-18";
    response["result"]["capabilities"]["tools"] = JsonObject();
    response["result"]["serverInfo"]["name"] = "ESP-RedCanary MCP Simulator";
    response["result"]["serverInfo"]["version"] = "1.0.0";
  } else if (strcmp(method, "ping") == 0) {
    response["result"].to<JsonObject>();
  } else if (strcmp(method, "tools/list") == 0) {
    JsonArray tools = response["result"]["tools"].to<JsonArray>();
    JsonObject status = tools.add<JsonObject>();
    status["name"] = "get_device_status";
    status["description"] = "Return simulated ESP32 honeypot status.";
    status["inputSchema"]["type"] = "object";
    JsonObject scan = tools.add<JsonObject>();
    scan["name"] = "scan_networks";
    scan["description"] = "Return simulated nearby Wi-Fi networks.";
    scan["inputSchema"]["type"] = "object";
  } else if (strcmp(method, "tools/call") == 0) {
    const char *name = input["params"]["name"] | "";
    if (strcmp(name, "get_device_status") == 0) {
      response["result"]["content"][0]["type"] = "text";
      response["result"]["content"][0]["text"] = "ESP32 RedCanary honeypot online; uptime=" + String(millis()) + "ms";
    } else if (strcmp(name, "scan_networks") == 0) {
      response["result"]["content"][0]["type"] = "text";
      response["result"]["content"][0]["text"] = "CafeNet (-54 dBm), OfficeGuest (-67 dBm), PrinterSetup (-78 dBm)";
    } else {
      sendMcpError(request, -32602, "Unknown simulated tool");
      return;
    }
  } else {
    sendMcpError(request, -32601, "Method not found");
    return;
  }
  sendMcpResponse(request, response);
}

void setup_mcp(AsyncWebServer *server, const String &webHook,String Protocol="")
{
  if (server == nullptr) return;

  server->on("/mcp", AsyncWebRequestMethod::HTTP_GET, [](AsyncWebServerRequest *request) {
    AsyncWebServerResponse *response = request->beginResponse(405, "text/plain", "MCP endpoint requires POST");
    response->addHeader("Allow", "POST");
    request->send(response);
  });
  server->on(
    "/mcp", AsyncWebRequestMethod::HTTP_POST,
    [](AsyncWebServerRequest *request) {}, nullptr,
    [webHook,Protocol](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
      handleMcpRequest(request, data, len, index, total, webHook, Protocol);
    }
  );
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void HandelNotFount(AsyncWebServerRequest *request, String Directory, String webHook,String Protocol="")
    { 
        String ip = request->client()->remoteIP().toString();
        String payload = request->url();//dumpBytes(request->client());
        int port = request->client()->localPort();
        logCommand(webHook,ip, port, payload,Protocol);
        
        debugf("HandelNotFount %s , Method =%s\n", request->url().c_str(), request->methodToString());
        debugf("%s\n", (Directory + request->url()).c_str());
        if (request->method() == AsyncWebRequestMethod::HTTP_GET)
        {
            if (FILESYSTEM.exists(Directory + request->url())) // exists will give a error in the error log see: https://github.com/espressif/arduino-esp32/issues/7615
                request->send(FILESYSTEM, Directory + request->url(), String(), false);
            else
                request->send( 404, "text/html", error404_html, sizeof(error404_html) - 1);
        }
        else if (request->method() == AsyncWebRequestMethod::HTTP_POST)
        {
            request->send( 404, "text/html", error404_html, sizeof(error404_html) - 1);
        }
    }

void HandelNotFountHTTP(AsyncWebServerRequest *request)
{
  HandelNotFount(request,"/http",HttpHookweb,"http");
}
void HandelNotFountHTTPS(AsyncWebServerRequest *request)
{
  HandelNotFount(request,"/https",HttpsHookweb,"https");
}
void HandelNotFountHTTPA(AsyncWebServerRequest *request)
{
  HandelNotFount(request,"/httpa",HttpaHookweb,"httpa");
}

void setup_http()
{
    Httpenabled=Settingsdoc["Http"]["enable"] | false;
    Httpsenabled=Settingsdoc["Https"]["enable"] | false;
    Httpaenabled=Settingsdoc["Httpa"]["enable"] | false;
    HttpMCPenabled=Settingsdoc["Http"]["enableMCP"] | false;
    HttpsMCPenabled=Settingsdoc["Https"]["enableMCP"] | false;
    HttpaMCPenabled=Settingsdoc["Httpa"]["enableMCP"] | false;
    
    HttpHookweb=Settingsdoc["Http"]["url"] | "";
    HttpsHookweb=Settingsdoc["Https"]["url"] | "";
    HttpaHookweb=Settingsdoc["Httpa"]["url"] | "";
    HttpaPort=Settingsdoc["Httpa"]["port"] | 8080;
    
    HttpMCPHookweb=Settingsdoc["Http"]["urlMCP"] | "";
    HttpsMCPHookweb=Settingsdoc["Https"]["urlMCP"] | "";
    HttpaMCPHookweb=Settingsdoc["Httpa"]["urlMCP"] | "";
  //DefaultHeaders::Instance().addHeader("Access-Control-Allow-Origin", "*"); 
  //DefaultHeaders::Instance().addHeader("Access-Control-Allow-Methods", "GET, PUT, POST, DELETE, HEAD");
  //DefaultHeaders::Instance().addHeader("Access-Control-Allow-Headers", "content-type");

  if(Httpenabled || HttpMCPenabled)
  {
    HttpServer=new AsyncWebServer(80);
    if(Httpenabled)     HttpServer->on("/", AsyncWebRequestMethod::HTTP_GET, [](AsyncWebServerRequest *request) { request->redirect("/index.html"); });
    if(HttpMCPenabled)  setup_mcp(HttpServer, HttpMCPHookweb,"http");
    if(Httpenabled)     HttpServer->onNotFound(HandelNotFountHTTP);
    HttpServer->begin();
  } 
  if(Httpsenabled || HttpsMCPenabled)
  {
    HttpsServer=new AsyncWebServer(443);

    if(Httpsenabled)    HttpsServer->on("/", AsyncWebRequestMethod::HTTP_GET, [](AsyncWebServerRequest *request) { request->redirect("/index.html"); });
    if(HttpsMCPenabled) setup_mcp(HttpsServer, HttpsMCPHookweb,"https");
    if(Httpsenabled)    HttpsServer->onNotFound(HandelNotFountHTTPS);
    HttpsServer->begin();
  }
  if(Httpaenabled || HttpaMCPenabled)
  {
    HttpaServer=new AsyncWebServer(HttpaPort);
    if(Httpaenabled)    HttpaServer->on("/", AsyncWebRequestMethod::HTTP_GET, [](AsyncWebServerRequest *request) { request->redirect("/index.html"); });
    if(HttpaMCPenabled) setup_mcp(HttpaServer, HttpaMCPHookweb,"httpa");
    if(Httpaenabled)    HttpaServer->onNotFound(HandelNotFountHTTPA);
    HttpaServer->begin();
  }
}
void loop_http()
{
}