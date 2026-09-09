#ifndef __CANARY_H__
#define __CANARY_H__    
#include <Arduino.h> //  https://github.com/espressif/arduino-esp32/tree/master/cores/esp32
#include <ArduinoJson.h>
#include "wifiscan.hpp"

#include <WiFi.h>      // https://github.com/espressif/arduino-esp32/tree/master/libraries/WiFi
#include <esp_wifi.h>
#include <ESPmDNS.h>   // https://github.com/espressif/arduino-esp32/tree/master/libraries/ESPmDNS
#include <AsyncTCP.h> // https://github.com/ESP32Async/AsyncTCP
#include <HTTPClient.h>

#include <DNSServer.h>
#if defined(OTAPASSWORD)
#include <ArduinoOTA.h>  // ArduinoOTA by Arduino, Juraj  https://github.com/JAndrassy/ArduinoOTA
#endif
#include <ESPAsyncWebServer.h> // https://github.com/ESP32Async/ESPAsyncWebServer
#include <LittleFS.h>
#include <FS.h>
#include <NTPClient.h>   //  https://github.com/arduino-libraries/NTPClient
#include "NTP.hpp"
#include "html.hpp"
#include "SSH.hpp"
#include "Http.hpp"
#include "Rdp.hpp"
#include "Ftp.hpp"
#include "Telnet.hpp"
#include "Http.hpp"
#include "Ping.hpp"
#include "Modbus.hpp"
#include "whitelisted.hpp"



#define DEBUG_PRINT  // manages most of the print and println debug, not all but most

#ifdef DEBUG_PRINT
#define debug_begin(...) Serial.begin(__VA_ARGS__)
#define setdebug(...) Serial.setDebugOutput(__VA_ARGS__)
#define debug(...) Serial.print(__VA_ARGS__)
#define debugln(...) Serial.println(__VA_ARGS__)
#define debugf(...) Serial.printf(__VA_ARGS__)
#else
#undef debug_begin
#undef setdebug
#undef debug
#undef debugln
#undef debugf
#define debug_begin(...) ;
#define debug(...) ;
#define debugln(...) ;
#define setdebug(...) ;
#define debugf(...) ;
#endif

extern NTPClient timeClient;

#if defined(CONFIG_IDF_TARGET_ESP32)
#ifndef PIN_BOOT
  #define PIN_BOOT 0
#endif
#elif defined(CONFIG_IDF_TARGET_ESP32C2)
#ifndef PIN_BOOT
  #define PIN_BOOT 9
#endif
#ifndef LED_BUILTIN
  #define LED_BUILTIN 8
#endif
#elif defined(CONFIG_IDF_TARGET_ESP32C3)
#ifndef PIN_BOOT
  #define PIN_BOOT 9
#endif
#ifndef LED_BUILTIN
  #define LED_BUILTIN 8
#endif
#elif defined(CONFIG_IDF_TARGET_ESP32C5)
#ifndef PIN_BOOT
  #define PIN_BOOT 28
#endif
#ifndef LED_BUILTIN
  #define LED_BUILTIN 27
#endif
#elif defined(CONFIG_IDF_TARGET_ESP32C6)
#ifndef PIN_BOOT
  #define PIN_BOOT 9
#endif
#elif defined(CONFIG_IDF_TARGET_ESP32S2)
#ifndef PIN_BOOT
  #define PIN_BOOT 0
#endif
#ifndef LED_BUILTIN
#define LED_BUILTIN 15
#endif
#elif defined(CONFIG_IDF_TARGET_ESP32S3)
#ifndef PIN_BOOT
  #define PIN_BOOT 0
#endif
#if not defined(PIN_NEOPIXEL)
#define PIN_NEOPIXEL 48
#endif
#endif

#ifdef LED_BUILTIN
#define PIN_LED LED_BUILTIN
#else
#undef PIN_LED
#define PIN_LED -1
#endif

void Log(String Str);
void setup_webserver(void);
void CanarySetup(void);
void CanaryLoop(void);  
extern bool Startreboot;
extern JsonDocument Settingsdoc;
extern char *host;
extern const char * canarytokenURL;

void startHoneypot();
void handleBannerGrab(String webHook,WiFiClient client, uint16_t port, const uint8_t* banner, size_t len);
void handleBannerGrab(String webHook,WiFiClient client, uint16_t port, const char* banner);
void handleHoneypotClient(WiFiClient client);
String dumpBytes(WiFiClient &c, size_t maxLen = 256, uint32_t timeout = 250);
void honeypotLoop();
void logCommand(String webHook,String ip, uint16_t port, String command);
String escapeJSON(String s);


#define JSONCONFIGFILE "/config.json"

#endif // __CANARY_H__