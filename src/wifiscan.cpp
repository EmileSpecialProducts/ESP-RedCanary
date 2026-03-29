#include "canary.hpp"
char Authenticate_mode[][21]= 
    {
        "OPEN",         /**< Authenticate mode : open */
        "WEP",              /**< Authenticate mode : WEP */
        "WPA_PSK",          /**< Authenticate mode : WPA_PSK */
        "WPA2_PSK",         /**< Authenticate mode : WPA2_PSK */
        "WPA_WPA2_PSK",     /**< Authenticate mode : WPA_WPA2_PSK */
        "ENTERPRISE",       /**< Authenticate mode : Wi-Fi EAP security, treated the same as WIFI_AUTH_WPA2_ENTERPRISE */
        "WPA3_PSK",         /**< Authenticate mode : WPA3_PSK */
        "WPA2_WPA3_PSK",    /**< Authenticate mode : WPA2_WPA3_PSK */
        "WAPI_PSK",         /**< Authenticate mode : WAPI_PSK */
        "OWE",              /**< Authenticate mode : OWE */
        "WPA3_ENT_192",     /**< Authenticate mode : WPA3_ENT_SUITE_B_192_BIT */
        "WPA3_EXT_PSK",     /**< This authentication mode will yield same result as WIFI_AUTH_WPA3_PSK and not recommended to be used. It will be deprecated in future, please use WIFI_AUTH_WPA3_PSK instead. */
        "WPA3_EXT_PSK_MIXED", /**< This authentication mode will yield same result as WIFI_AUTH_WPA3_PSK and not recommended to be used. It will be deprecated in future, please use WIFI_AUTH_WPA3_PSK instead.*/
        "DPP",              /**< Authenticate mode : DPP */
        "WPA3_ENTERPRISE",  /**< Authenticate mode : WPA3-Enterprise Only Mode */
        "WPA2_WPA3_ENTERPRISE", /**< Authenticate mode : WPA3-Enterprise Transition Mode */
        "WPA_ENTERPRISE"   /**< Authenticate mode : WPA-Enterprise security */
    };
    //WIFI_AUTH_MAX
static bool Startscan= false;

void StartWiFiNetworks()
{
  WiFi.scanDelete();
  WiFi.scanNetworks(true);
  debugln("StartWiFiNetworks: Scanning for networks...");
}

void scanWiFiNetworks(JsonDocument& Wifidoc)
{
// https://github.com/tzapu/WiFiManager/blob/32655b722601ce8bf3c03974bac06a96ceed42e2/WiFiManager.cpp#L1487
   
    int8_t scanResult = WiFi.scanComplete();
    if(scanResult != WIFI_SCAN_RUNNING)
    {
      if(scanResult >= 0)
      {
        debugf("scanWiFiNetworks: %d networks found\n", scanResult);
        for(int i = 0; i < scanResult; i++)
        {
          JsonDocument WifiAp; 
          WifiAp["ssid"] = WiFi.SSID(i);
          WifiAp["rssi"] = WiFi.RSSI(i);
          WifiAp["encryptionType"] = Authenticate_mode[WiFi.encryptionType(i)];
          WifiAp["bssid"] = WiFi.BSSIDstr(i);
          WifiAp["channel"] = WiFi.channel(i);
          Wifidoc.add(WifiAp);
          /*
          String ssid = WiFi.SSID(i);
          int32_t rssi = WiFi.RSSI(i);
          uint8_t encryptionType = WiFi.encryptionType(i);
          String bssid = WiFi.BSSIDstr(i);
          int32_t channel = WiFi.channel(i);
          debugf(" %d: %s, Ch:%d (%ddBm) BSSID: %s Encryption: %d\n", i + 1, ssid.c_str(), channel, rssi, bssid.c_str(), encryptionType);
          */
        }
        WiFi.scanDelete();
        WiFi.scanNetworks(true);
      }
      else
      {
        debugln("scanWiFiNetworks: No networks found");
      }
    }
}
