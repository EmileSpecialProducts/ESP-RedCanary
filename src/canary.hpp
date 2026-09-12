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

#if defined(USE_MMC_CARD)

#endif

#if defined(USE_SPI_CARD)
#include <SPI.h>
#include <SD.h>
#define FILESYSTEM SD

#if !defined(SD_PIN_CS) 
#define SD_PIN_CS SS
#endif
#if !defined(SD_PIN_MOSI)
#define SD_PIN_MOSI MOSI
#endif 

#if !defined(SD_PIN_MISO) 
#define SD_PIN_MISO MISO
#endif

#if !defined(SD_PIN_SCK)
#define SD_PIN_SCK SCK
#endif

#elif defined(USE_MMC_CARD)
#include <FS.h>
#include <SD_MMC.h>
#define FILESYSTEM SD_MMC

#if !defined(MMC_CMD) 
#warning "Set the MMC_CMD pin in PlatformIO.ini file "
#endif

#if !defined(MMC_CLK)
#warning "Set the MMC_CLK pin in PlatformIO.ini file "
#endif

#if !defined(MMC_DATA0)
#warning "Set the MMC_DATA0 pin in PlatformIO.ini file "
#endif

#if !defined(MMC_DATA1)
#define MMC_DATA1  GPIO_NUM_NC
#endif

#if !defined(MMC_DATA2)
#define MMC_DATA2  GPIO_NUM_NC
#endif

#if !defined(MMC_DATA3)
#define MMC_DATA3  GPIO_NUM_NC
#endif

#else // USESDCARD
#include <LittleFS.h>
#define FILESYSTEM LittleFS
#endif

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

#if defined(DEBUG_PRINT)
#define debug_begin(...) Serial.begin(__VA_ARGS__)
#define setdebug(...) Serial.setDebugOutput(__VA_ARGS__)
#define debug(...) Serial.print(__VA_ARGS__)
#define debugln(...) Serial.println(__VA_ARGS__)
#define debugf(...) Serial.printf(__VA_ARGS__)
#else
#define debug_begin(...) do { } while (false)
#define debug(...) do { } while (false)
#define debugln(...) do { } while (false)
#define setdebug(...) do { } while (false)
#define debugf(...) do { } while (false)
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