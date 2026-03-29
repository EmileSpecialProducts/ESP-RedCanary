
#include <canary.hpp>

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
// https://adafruit.github.io/Adafruit_NeoPixel/html/class_adafruit___neo_pixel.html
#define NEOPIXEL_NUM 144
int LedMaxCurrent = 500; // mA
int LedPin = -1;         // Pin where the NeoPixel is connected. -1 means no LED
Adafruit_NeoPixel strip{NEOPIXEL_NUM,(int16_t) LedPin, NEO_GRB + NEO_KHZ800};

void setColor(int r, int g, int b, int start =0, int len =-1 )
    {
        if(LedPin>=0)
        {
            if(len <0) len = strip.numPixels();
            if (len+start > strip.numPixels()) len = strip.numPixels()-start;
            for (int i = 0; i < len; i++)
            {
                strip.setPixelColor(i+start, r, g, b);
            }
            size_t numPixels =strip.numPixels();
            for (size_t i = 0; i < numPixels; i++)
            {
                
            }
            strip.show();       
        }
    }

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
    debug("Initializing LittleFS...");
    if (!LittleFS.begin(true)) // FORMAT_LITTLEFS_IF_FAILED
    debugln(" Mount Failed");
  else
    debugln(" Started ");
  
    File file = LittleFS.open(JSONCONFIGFILE, "r");
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
    LedPin = Settingsdoc["LedPin"].is<int>() ? (int)Settingsdoc["LedPin"].as<int>() : -1;
    debugf("LedPin = %d\n", LedPin);
    if(LedPin>=0)
      {
      uint16_t numPixels=1; // at least 1 pixel to be able to set the brightness
      uint16_t LedSize , LedStart;
      LedMaxCurrent = Settingsdoc["LedMaxCurrent"].is<int>() ? (int)Settingsdoc["LedMaxCurrent"].as<int>() : 500;
      //Settingsdoc["http"]["LedStart"].is<int>() ? debugf("http_LedStart = %d\n", (int)Settingsdoc["http"]["LedStart"].as<int>()) : debugln("http_LedStart Not found");
      
      for (size_t i = 0; i < sizeof(Canarys)/sizeof(Canarys[0]); i++)
          {
          LedStart = Settingsdoc[Canarys[i]]["LedStart"].is<int>() ? (int)Settingsdoc[Canarys[i]]["LedStart"].as<int>() : 0;
          LedSize = Settingsdoc[Canarys[i]]["LedSize"].is<int>() ? (int)Settingsdoc[Canarys[i]]["LedSize"].as<int>() : 0;
          if ( numPixels< (LedStart+LedSize) ) numPixels = LedStart+LedSize;
          debugf("%s: LedStart = %d LedSize = %d numPixels=%d\n", Canarys[i], LedStart, LedSize, numPixels);
          }
      strip.updateLength(numPixels);
      strip.setPin(LedPin);
      strip.setMaxCurrent(LedMaxCurrent);
      strip.begin();
      setColor(255, 0, 0);
      }

    debugf("ssid = %s\n", Settingsdoc["ssid"] | "SSID Not found");
    debugf("ssid size = %d\n", Settingsdoc["ssid"].size());
    debugf("password = %s\n", Settingsdoc["password"] | "password Not found");
    debugln(Settingsdoc.as<String>());
    bool OverTheAir = Settingsdoc["OverTheAir"].is<bool>() ? Settingsdoc["OverTheAir"] : false;
    debugf("OTA %d\n", OverTheAir);
    host = Settingsdoc["ServerName"].is<const char*>() ? (char*)Settingsdoc["ServerName"].as<const char*>() : (char *)"ESPRedCanary";    
    debugf("ServerName = %s\n", host );
    WiFi.hostname(host); 
    char *ssid=Settingsdoc["ssid"].is<const char*>() ? (char*)Settingsdoc["ssid"].as<const char*>() : nullptr;
    char *password=Settingsdoc["password"].is<const char*>() ? (char*)Settingsdoc["password"].as<const char*>() : nullptr;
    if(ssid) debugf("ssid len = %d %s\n", strlen(ssid), ssid);
    else debugln("Starting in AP mmode\n");

    if(ssid!=nullptr && password!=nullptr && strlen(ssid)>0)
        {
        WiFi.begin( ssid , password );
        
        for (uint8_t i = 0; i < 20 && !WiFiConected; i++)
            { // wait 10 seconds
                if (WiFi.status() != WL_CONNECTED)
                    delay(500);
                else
                    WiFiConected = true;
            }
        }
    if (!WiFiConected)
        {
            char *apssid=Settingsdoc["apssid"].is<const char*>() ? (char*)Settingsdoc["apssid"].as<const char*>() : (char*)"ESPRedCanary";
            char *appassword=Settingsdoc["appassword"].is<const char*>() ? (char*)Settingsdoc["appassword"].as<const char*>() : (char*)"";
            int APCannel=Settingsdoc["APCannel"].is<int>() ? (int)Settingsdoc["APCannel"].as<int>() : 1;
            if (APCannel < 1 || APCannel > 13)  APCannel = 1;   
            debugf("Started Access Point \"%s\":\"%s\":%d\n", apssid,appassword,APCannel);
            // WiFi.mode(WIFI_AP_STA);
            static DNSServer dnsServer;
            IPAddress apIP(192, 168, 4, 1);

            WiFi.softAP(apssid, appassword, APCannel);
            WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0)); 
            dnsServer.setTTL(300);
            dnsServer.setErrorReplyCode(DNSReplyCode::ServerFailure);

            /* Setup the DNS server redirecting all the domains to the apIP */
            //dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
            //dnsServer->start(DNS_PORT, F("*"), WiFi.softAPIP());
            // This will connect to the UltraWiFiDuck 
            dnsServer.start(53, F("*"), apIP);
            WiFiAPMode = true;
        }
    WiFi.setSleep(false);
  esp_wifi_set_ps(WIFI_PS_NONE); // Esp32 enters the power saving mode by default,
  debug("Connected! IP address: ");
  debugln(WiFi.localIP());
  debug("Connecting to ");
  debugln(WiFi.SSID());

  if (MDNS.begin(host))
  {
    MDNS.addService("http", "tcp", 80);
    debugln("MDNS responder started");
    debug("You can now connect to http://");
    debug(host);
    debug(".local or http://");
    debugln(WiFi.localIP());
  }

if(OverTheAir)
  {
  // Port defaults to 3232
  // ArduinoOTA.setPort(3232);

  // Hostname defaults to esp3232-[MAC]
  ArduinoOTA.setHostname(host);

  // No authentication by default
  char *OTAPassword=Settingsdoc["OTAPassword"].is<const char*>() ? (char*)Settingsdoc["OTAPassword"].as<const char*>() : nullptr;
  if(OTAPassword) ArduinoOTA.setPassword(OTAPassword);

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
  }

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
  setColor(0, 0, 0);
  StartWiFiNetworks();
  CanarySetup();
}  

void resetWifipasswords()
{
  File file = LittleFS.open(JSONCONFIGFILE, "r");
  deserializeJson(Settingsdoc, file);
  file.close();
  Settingsdoc["ssid"] = "";
  Settingsdoc["password"] = "";
  Settingsdoc["apssid"] = "ESP-RedCanary";
  Settingsdoc["appassword"] = "";
  Settingsdoc["ServerName"] = "ESPRedCanary";
  Settingsdoc["APCannel"] = 1;
  file = LittleFS.open(JSONCONFIGFILE, "w");
  if (file)
  {
    serializeJsonPretty(Settingsdoc, file);
    file.close();
    debugln("WiFi settings reset.");
  }
  else
    debugln("Failed to open config file for writing");
}
void loop(void)
{
  unsigned long Time = millis();
  yield();
  ArduinoOTA.handle();
  if (OTAUploadBusy == 0)
  { // Do not do things that take time when OTA is busy
    //server.handleClient();
    CanaryLoop();
  }
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
    if(LedPin>=0)
    {
      for (int f=0;f<5;f++){
        strip.setPixelColor(0, f*2, f*2, f*2);
        strip.show();       
        delay(10);
      }   
      for (int f=5;f>=0;f--){
        strip.setPixelColor(0, f*2, f*2, f*2);
        strip.show();       
        delay(10);
      }
    }
    
    if (OTAUploadBusy > 0)
      OTAUploadBusy--;
    if (digitalRead(PIN_BOOT) == LOW)
    {
      if (++Config_Reset_Counter > 2)
      {
        // open the webserver after 3 seconds
        setColor(0,255, 0);
      }
      if (++Config_Reset_Counter > 10)
      {                 // press the BOOT 10 sec to reset the WifiManager Settings
        resetWifipasswords();
        debugln("Resetting WiFi settings and rebooting!");
        while(digitalRead(PIN_BOOT) == LOW)
        {
            setColor(0,255, 0);
            delay(100);
            setColor(0, 0, 255);
            delay(100);
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












