#include "canary.hpp"

namespace {
bool parseIPv4(String value, uint32_t &address)
{
    value.trim();
    IPAddress parsed;
    if (!parsed.fromString(value.c_str())) return false;

    address = (static_cast<uint32_t>(parsed[0]) << 24) |
              (static_cast<uint32_t>(parsed[1]) << 16) |
              (static_cast<uint32_t>(parsed[2]) << 8) |
              parsed[3];
    return true;
}

bool matchesIPv4Cidr(String candidate, String cidr)
{
    const int separator = cidr.indexOf('/');
    if (separator < 1) return false;

    String networkText = cidr.substring(0, separator);
    String prefixText = cidr.substring(separator + 1);
    prefixText.trim();
    if (prefixText.length() == 0) return false;
    for (size_t index = 0; index < prefixText.length(); index++) {
        if (!isDigit(prefixText[index])) return false;
    }

    const int prefixLength = prefixText.toInt();
    if (prefixLength < 0 || prefixLength > 32) return false;

    uint32_t candidateAddress;
    uint32_t networkAddress;
    if (!parseIPv4(candidate, candidateAddress) || !parseIPv4(networkText, networkAddress)) return false;

    const uint32_t mask = prefixLength == 0 ? 0 : 0xffffffffUL << (32 - prefixLength);
    return (candidateAddress & mask) == (networkAddress & mask);
}
}

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
            String whitelistEntry = ipno.as<String>();
            whitelistEntry.trim();
            debugf("Whitelisting IP: %s\n", whitelistEntry.c_str());
            if (whitelistEntry == ip || matchesIPv4Cidr(ip, whitelistEntry))
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
