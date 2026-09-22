//
// this is based on the work that https://github.com/7h30th3r0n3/NanoC6-ESP32-Honeypot did

#include "canary.hpp"

namespace {
int hexDigit(char value)
{
  if (value >= '0' && value <= '9') return value - '0';
  if (value >= 'A' && value <= 'F') return value - 'A' + 10;
  if (value >= 'a' && value <= 'f') return value - 'a' + 10;
  return -1;
}
}

bool parseMacAddress(const String &value, uint8_t mac[6])
{
  String text = value;
  text.trim();
  if (text.length() != 17) return false;

  for (size_t index = 0; index < 6; index++)
  {
    const size_t position = index * 3;
    if (index < 5 && text[position + 2] != ':') return false;

    const int high = hexDigit(text[position]);
    const int low = hexDigit(text[position + 1]);
    if (high < 0 || low < 0) return false;

    mac[index] = static_cast<uint8_t>((high << 4) | low);
  }
  return true;
}


void CanarySetup()
{
    //startHoneypot();
    setup_logging();
    setup_ssh();
    setup_http();
    setup_rdp();
    setup_ftp();
    setup_telnet();
    setup_Modbus();
}

void CanaryLoop()
{
  loop_ssh();
  loop_http();
  loop_rdp();
  loop_ftp();
  loop_telnet();
  loop_Modbus();
    //honeypotLoop(); 
}
