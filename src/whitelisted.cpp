#include "canary.hpp"
void InitWhitelisted()
{

}
bool LogWhitelisted()
{
    if(Settingsdoc["Whitelisted"]["logEnable"].is<bool>())
    {
          if(Settingsdoc["Whitelisted"]["logEnable"]==true)
          {
              debugln("Logging Whitelisted IPs is Enabled");
              return true;
          }
    }
    return true;    
}

bool CanryWhitelisted()
{
    if(Settingsdoc["Whitelisted"]["SEMEnable"].is<bool>())
    {
          if(Settingsdoc["Whitelisted"]["SEMEnable"]==true)
          {
              debugln("SEMEnable Whitelisted IPs is Enabled");
              return true;
          }
    }
    return true;    
}

bool IsWhitelisted(String ip)
{
    debugf("Whitelisted IP: %s\n", ip.c_str());
    // char *apssid=Settingsdoc["apssid"].is<const char*>() ? (char*)Settingsdoc["apssid"].as<const char*>() : (char*)"ESPRedCanary";
    if(Settingsdoc["Whitelisting"]["ips"].is<JsonArray>())
    {
        JsonArray ips = Settingsdoc["Whitelisting"]["ips"].as<JsonArray>();
        for (JsonVariant ipno : ips) 
        {
            debugf("Whitelisting IP: %s\n", ipno.as<const char*>());
            if( String(ipno.as<const char*>()) == ip )
            {
                debugln("Device is Whitelisted");
                return true;
            }
        }
    } else
    {
        debugln("No Whitelisted IPs found");
    }
    return false;    
}
