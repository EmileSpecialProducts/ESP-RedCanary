#include "canary.hpp"

WiFiServer rdpServer(3389);
String rdpHookweb = "";
bool rdpenabled = false;
void setup_rdp()
{
  rdpenabled=Settingsdoc["Rdp"]["enable"] | false;
  rdpHookweb=Settingsdoc["Rdp"]["url"] | "";
  if(rdpenabled) rdpServer.begin();
}
void loop_rdp()
{
  if(rdpenabled)
  {
    /* ---------- RDP Connection‑Confirm ------------------------------- */
    if (WiFiClient c = rdpServer.accept()) {
      static const uint8_t RDP_CC[] = {
        0x03, 0x00, 0x00, 0x13,
        0x0E, 0xD0, 0x00, 0x00,
        0x12, 0x34, 0x00, 0x02,
        0x00, 0x08, 0x00
      };
      handleBannerGrab(rdpHookweb,c, 3389, RDP_CC, sizeof(RDP_CC));
    }
  }
}