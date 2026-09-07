#include "canary.hpp"

WiFiServer sshServer(22);
String sshHookweb = "";
bool SSHenabled=false;
    
void setup_ssh()
{
  SSHenabled=Settingsdoc["SSH"]["enable"] | false;
  sshHookweb=Settingsdoc["SSH"]["url"] | "";
    debugf("SSH Hook URL: %s\n", sshHookweb.c_str());
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