#include <canary.hpp>

#include <WiFiManager.h> // WifiManager by tzapu  https://github.com/tzapu/WiFiManager

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);
unsigned long PreviousTimeSeconds;
unsigned long PreviousTimeMinutes;
unsigned long PreviousTimeHours;
unsigned long PreviousTimeDay;
unsigned long currentTimeSeconds = 0;
unsigned long NextTime = 0;
uint16_t Config_Reset_Counter = 0;
int OTAUploadBusy = 0;
JsonDocument Settingsdoc;
char *host = (char *)"ESP-RedCanary";
const char * canarytokenURL = (char *)"";
bool WiFiConected = false;
bool WiFiAPMode = false;
bool Startreboot = false;
#if defined(USE_SPI_CARD) && !defined(CONFIG_IDF_TARGET_ESP32)
SPIClass * sd_spi = NULL;
#endif
 
void setup(void)
{
  pinMode(PIN_BOOT, INPUT_PULLUP);
  pinMode(PIN_LED, OUTPUT);
#if defined(CONFIG_IDF_TARGET_ESP32S2) || defined(CONFIG_IDF_TARGET_ESP32) || Serial == USBSerial
  debug_begin(115200);
#else
  debug_begin(115200, SERIAL_8N1, RX, TX);
#endif
  setdebug(true);
  debug("\n");
    debugf("ESP RedCanary\n");
    debugf("esp_idf_version: %d.%d.%d\n" ,ESP_IDF_VERSION_MAJOR ,ESP_IDF_VERSION_MINOR,ESP_IDF_VERSION_PATCH);
    debugf("arduino_version: %d.%d.%d\n" ,ESP_ARDUINO_VERSION_MAJOR,ESP_ARDUINO_VERSION_MINOR,ESP_ARDUINO_VERSION_PATCH);
    debugf("Build Date: " __DATE__ " " __TIME__ "\n");
#if defined(USE_SPI_CARD)
  debug("Initializing SDCard FS...");

  debugf("USE_SPI_CARD SD_PIN_CS = %d SD_PIN_MOSI = %d SD_PIN_MISO = %d SD_PIN_SCK = %d\n", SD_PIN_CS, SD_PIN_MOSI, SD_PIN_MISO, SD_PIN_SCK);

#if defined(CONFIG_IDF_TARGET_ESP32)
  if (SD.begin(SD_PIN_CS)){
    debugln("Started SDCard FS");
  } else {
    debugln("Mount Failed");
  }
#else // CONFIG_IDF_TARGET_ESP32
  // FSPI 0  // ESP32C2, C3, C5, C6, C61, H2, S2, S3, P4 - SPI 2 bus
  // HSPI 1  // ESP32S2, S3, P4 - SPI 3 bus

    #if defined(CONFIG_IDF_TARGET_ESP32C3) || defined(CONFIG_IDF_TARGET_ESP32C6)
      sd_spi = new SPIClass(FSPI);
    #else
      sd_spi = new SPIClass(); // HSPI
    #endif

    int SDmaxSpeed =50;
    bool hasSD = false;
    sd_spi->begin(SD_PIN_SCK,SD_PIN_MISO,SD_PIN_MOSI,SD_PIN_CS);
    while (SDmaxSpeed>4 && hasSD==false)
    {
      if (SD.begin(SD_PIN_CS, *sd_spi, 1000000UL * SDmaxSpeed)){
      debugf("Started SDCard FS Speed: %d\n", SDmaxSpeed);
      hasSD = true;
      } else SDmaxSpeed-=4;
    }  
#endif // CONFIG_IDF_TARGET_ESP32
#elif defined(USE_MMC_CARD)
  debugln("Initializing MMCCard FS...");
  debugf("USE_MMC_CARD MMC_CLK = %d MMC_CMD = %d MMC_DATA0 = %d MMC_DATA1 = %d MMC_DATA2 = %d MMC_DATA3 = %d\n", MMC_CLK, MMC_CMD, MMC_DATA0, MMC_DATA1, MMC_DATA2, MMC_DATA3);
  SD_MMC.setPins(MMC_CLK, MMC_CMD, MMC_DATA0, MMC_DATA1, MMC_DATA2, MMC_DATA3);
  debug("Initializing MMCCard FS...");
  if (!SD_MMC.begin("/sdcard", MMC_DATA1 == GPIO_NUM_NC ? true : false)) // true = oneBit mode, false = 4 bit mode
  {
    debugln("Mount Failed");
  } else {
    debugln("Started MMCCard FS");
  }
#else // USE_SPI_CARD
    debug("Initializing LittleFS...");    
    if (!LittleFS.begin(true)) // FORMAT_LITTLEFS_IF_FAILED
      debugln(" Mount Failed");
    else
      debugln("Started LittleFS ");
#endif
    File file = FILESYSTEM.open(JSONCONFIGFILE, "r");
    if (file)
    {
      deserializeJson(Settingsdoc, file);
      file.close();
      char Canarys[][10] = {"Http","Https","Httpa","SSH","Ftp","Telnet","Rdp","Modbus","Ping"};
      canarytokenURL  = Settingsdoc["canarytokenURL"].is<const char*>() ? (char*)Settingsdoc["canarytokenURL"].as<const char*>() : (char *)"";
      if(strlen(canarytokenURL)>0)
        { // set all canarytoken URL if canarytokenURL is set and not empty
        debugf("canarytokenURL = %s\n", canarytokenURL);
        for (size_t i = 0; i < sizeof(Canarys)/sizeof(Canarys[0]); i++)
            {
              // ToDO: check if the specific canary URL is set and not empty, if not set it to canarytokenURL
            // if( !Settingsdoc[Canarys[i]]["url"].is<const char*>() || strlen( (const char *)Settingsdoc[Canarys[i]]["url"].as<const char *>() )==0 )
            // Settingsdoc[Canarys[i]]["url"] = Settingsdoc[Canarys[i]]["url"].is<char*>() ? (const char *)Settingsdoc[Canarys[i]]["url"].as<char *>() : canarytokenURL;
            }
        }
      debugln(Settingsdoc.as<String>());
    }
    else
    {
      debugln("Failed to open " JSONCONFIGFILE " file for reading");
    }
  // WiFiManager, Local intialization. Once its business is done, there is no need to keep it around
  WiFiManager wm;

  // reset settings - wipe stored credentials for testing
  // these are stored by the esp library
  //  wm.resetSettings();

  // Automatically connect using saved credentials,
  // if connection fails, it starts an access point with the specified name ( "AutoConnectAP"),
  // if empty will auto generate SSID, if password is blank it will be anonymous AP (wm.autoConnect())
  // then goes into a blocking loop awaiting configuration and will return success result

  bool res;
  //sets timeout until configuration portal gets turned off
  //useful to make it all retry or go to sleep
  //in seconds
  wm.setTimeout(180);

  res = wm.autoConnect(host); // auto generated AP name from chipid
  // res = wm.autoConnect(DeviceName); // anonymous ap
  // res = wm.autoConnect("AutoConnectAP","password"); // password protected ap
  if (!res)
  {
    debugln("Failed to connect Restarting");
    delay(5000);
    if (digitalRead(PIN_BOOT) == LOW)
    {
      wm.resetSettings();
    }
    ESP.restart();
  }
  else
  {
    // if you get here you have connected to the WiFi
  }

  WiFi.setSleep(false);
  esp_wifi_set_ps(WIFI_PS_NONE); // Esp32 enters the power saving mode by default,
  debug("Connected! IP address: ");
  debugln(WiFi.localIP());
  debug("Connecting to ");
  debugln(WiFi.SSID());
  host = Settingsdoc["ServerName"].is<const char*>() ? (char*)Settingsdoc["ServerName"].as<const char*>() : (char *)"ESPRedCanary";    
  debugf("ServerName = %s\n", host );
  WiFi.hostname(host); 
    
  if (MDNS.begin(host))
  {
    MDNS.addService("http", "tcp", 80);
    debugln("MDNS responder started");
    debug("You can now connect to http://");
    debug(host);
    debug(".local or http://");
    debugln(WiFi.localIP());
  }

#if defined(OTAPASSWORD)
#warning "OTA enabled and password is set to: " OTAPASSWORD " " 
  debugln("OTA disabled");
  // Port defaults to 3232
  // ArduinoOTA.setPort(3232);

  // Hostname defaults to esp3232-[MAC]
  ArduinoOTA.setHostname(host);
  ArduinoOTA.setPassword(OTAPASSWORD);

  // Password can be set with it's md5 value as well
  // MD5(admin) = 21232f297a57a5a743894a0e4a801fc3
  // ArduinoOTA.setPasswordHash("21232f297a57a5a743894a0e4a801fc3");
  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    } else {  // U_FS
      type = "filesystem";
    }
    // NOTE: if updating FS this would be the place to unmount FS using FS.end()
    OTAUploadBusy=60; // only do a update for 60 sec;
    debugln("Start updating " + type); 
  });
  ArduinoOTA.onEnd([](){
    OTAUploadBusy=0;
    debugln("\nEnd"); 
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total)
                        { debug("Progress: " + String((progress / (total / 100))) + "\r"); });
  ArduinoOTA.onError([](ota_error_t error){
    OTAUploadBusy=0;
    debugln("Error: "+String( error));
    if (error == OTA_AUTH_ERROR) {
      debugln("Auth Failed");
    } else if (error == OTA_BEGIN_ERROR) {
      debugln("Begin Failed");
    } else if (error == OTA_CONNECT_ERROR) {
      debugln("Connect Failed");
    } else if (error == OTA_RECEIVE_ERROR) {
      debugln("Receive Failed");
    } else if (error == OTA_END_ERROR) {
      debugln("End Failed");
    } 
  });
  ArduinoOTA.begin();
#endif
  setup_webserver();  

  timeClient.begin();
  timeClient.setUpdateInterval(1000 * 60 * 60 * 24); // 24 uur
  timeClient.update();

  unsigned long currentTimeSeconds = timeClient.getEpochTime();

  String message;
  message.reserve(512);
  message = getFormattedDateTime(currentTimeSeconds);
  message += " Reboot from: ";
  message += ESP.getChipModel();
  message += "_";
  message += WiFi.macAddress();
  message += " LocalIpAddres: " + WiFi.localIP().toString();
  message += " SSID: " + String(WiFi.SSID());
  message += " Rssi: " + String(WiFi.RSSI());
  message += " Total heap: " + String(ESP.getHeapSize() / 1024);
  message += " Free heap: " + String(ESP.getFreeHeap() / 1024);
  message += " Total PSRAM: " + String(ESP.getPsramSize() / 1024);
  message += " Free PSRAM: " + String(ESP.getFreePsram() / 1024);
  message += " bytes getFreeHeap: " +String(ESP.getFreeHeap()) ; 
  message += " byte esp_get_free_heap_size: "+ String(esp_get_free_heap_size());
  message += " byte free internal_heap_size: "+ String(esp_get_free_internal_heap_size());
  message += " byte ArduinoLoopTaskStackSize: "+ String(getArduinoLoopTaskStackSize());
  message += " byte getSketchSize: " + String(ESP.getSketchSize());
  message += " Temperature: " + String(temperatureRead()) + " °C "; // internal TemperatureSensor
  message += " FlashChipSize: " + String(ESP.getFlashChipSize());
  message += " FlashChipSpeed: " + String(ESP.getFlashChipSpeed());
#if ESP_ARDUINO_VERSION != ESP_ARDUINO_VERSION_VAL(2, 0, 17)
  // [ESP::getFlashChipMode crashes on ESP32S3 boards](https://github.com/espressif/arduino-esp32/issues/9816)
  message += " FlashChipMode: ";
  switch (ESP.getFlashChipMode())
  {
  case FM_QIO:
    message += "FM_QIO";
    break;
  case FM_QOUT:
    message += "FM_QOUT";
    break;
  case FM_DIO:
    message += "FM_DIO";
    break;
  case FM_DOUT:
    message += "FM_DOUT";
    break;
  case FM_FAST_READ:
    message += "FM_FAST_READ";
    break;
  case FM_SLOW_READ:
    message += "FM_SLOW_READ";
    break;
  default:
    message += String(ESP.getFlashChipMode());
  }
#endif
  message += " esp_idf_version: " + String(esp_get_idf_version());
  message += " arduino_version: " + String(ESP_ARDUINO_VERSION_MAJOR) + "." + String(ESP_ARDUINO_VERSION_MINOR) + "." + String(ESP_ARDUINO_VERSION_PATCH);
  message += " Build Date: " + String(__DATE__ " " __TIME__);
  Log(message);
  NextTime = millis() + 1000;
  StartWiFiNetworks();
  CanarySetup();
}  
void loop(void)
{
  unsigned long Time = millis();
  yield();
#if defined(OTAPASSWORD)
  ArduinoOTA.handle();
  if (OTAUploadBusy == 0)
  { // Do not do things that take time when OTA is busy
    //server.handleClient();
    CanaryLoop();
  }
#endif
  if(Startreboot) ESP.restart();
    
  if (PreviousTimeDay != (currentTimeSeconds / (60 * 60 * 24)))
  { // Day Loop
    PreviousTimeDay = (currentTimeSeconds / (60 * 60 * 24));
    timeClient.update();
  }
  if (PreviousTimeHours != (currentTimeSeconds / (60 * 60)))
  { // Hours Loop
    PreviousTimeHours = (currentTimeSeconds / (60 * 60));
  }

  if (PreviousTimeMinutes != (currentTimeSeconds / 60))
  { // Minutes loop
    PreviousTimeMinutes = (currentTimeSeconds / 60);
    if ((WiFi.status() != WL_CONNECTED))
    { // if WiFi is down, try reconnecting
      WiFi.disconnect();
      WiFi.reconnect();
    }
  }

  if (Time >= NextTime) // This will fail after 71 days
  {
    NextTime = millis() + 1000;
    currentTimeSeconds++;     
    if (OTAUploadBusy > 0)
      OTAUploadBusy--;
    if (digitalRead(PIN_BOOT) == LOW)
    {
      if (++Config_Reset_Counter > 2)
      {
        // open the webserver after 3 seconds
      }
      if (++Config_Reset_Counter > 10)
      {                 // press the BOOT 10 sec to reset the WifiManager Settings
        WiFiManager wm; // WiFiManager, Local intialization. Once its business is done, there is no need to keep it around
        delay(500);
        wm.resetSettings();
        debugln("Resetting WiFi settings and rebooting!");
        while(digitalRead(PIN_BOOT) == LOW)
        {

        }
        ESP.restart();
      }
    }
    else
    {
      Config_Reset_Counter = 0;
    }
  }
}

