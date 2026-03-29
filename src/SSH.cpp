#include "canary.hpp"

WiFiServer sshServer(22);
String sshHookweb = "";
int sshledstart = 0;
int sshLedSize = 0;
String sshLedColor="";
bool SSHenabled=false;
    
void setup_ssh()
{
  SSHenabled=Settingsdoc["SSH"]["enable"] | false;
  sshHookweb=Settingsdoc["SSH"]["url"] | "";
    sshledstart = Settingsdoc["SSH"]["LedStart"].is<int>() ? (int)Settingsdoc["SSH"]["LedStart"].as<int>() : 0;
    sshLedSize = Settingsdoc["SSH"]["LedSize"].is<int>() ? (int)Settingsdoc["SSH"]["LedSize"].as<int>() : 0;
    sshLedColor = Settingsdoc["SSH"]["LedColor"].is<const char*>() ? (const char*)Settingsdoc["SSH"]["LedColor"].as<const char*>() : "";
    debugf("SSH Hook URL: %s\n", sshHookweb.c_str());
    debugf("SSH LedStart: %d\n", sshledstart);  
    debugf("SSH LedSize: %d\n", sshLedSize);  
    debugf("SSH LedColor: %s\n", sshLedColor.c_str());  
  if (SSHenabled) sshServer.begin();
}

void loop_ssh()
{
  if (SSHenabled)
  {
     /* ---------- SSH : OpenSSH 8.5p1 ----------- */
   if (WiFiClient c = sshServer.accept()) {
    if (!c) return;
    String ip = c.remoteIP().toString();
    c.print("SSH-2.0-OpenSSH_8.5p1 Debian-1\r\n");
    logCommand(sshHookweb, ip, 22, "SSH"); // dumpBytes(c)
    /* keep the channel open a bit so nmap --script ssh-hostkey can
       finish the key‑exchange probe                           */
    unsigned long t0 = millis();
    while (c.connected() && millis() - t0 < 3000) delay(1);
    c.stop();
  }
 }
}