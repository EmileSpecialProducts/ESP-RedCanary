//
// this is based on the work that https://github.com/7h30th3r0n3/NanoC6-ESP32-Honeypot did

#include "canary.hpp"


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
