#ifndef __LOGGING_HPP__
#define __LOGGING_HPP__
void handleBannerGrab(String webHook,WiFiClient client, uint16_t port, const uint8_t* banner, size_t len);
void handleBannerGrab(String webHook,WiFiClient client, uint16_t port, const char* banner);
void handleHoneypotClient(WiFiClient client);
void honeypotLoop();String dumpBytes(WiFiClient &c, size_t maxLen = 256, uint32_t timeout = 250);

void logCommand(String webHook,String ip, uint16_t port, String command,String Protocol="");
String escapeJSON(String s);
void setup_logging();

#endif