#include "canary.hpp"

WiFiServer ftpServer(21);
String ftpHookweb = "";
bool ftpenabled = false;

void setup_ftp()
{
    ftpenabled=Settingsdoc["Ftp"]["enable"] | false;
    ftpHookweb=Settingsdoc["Ftp"]["url"] | "";
    if(ftpenabled) ftpServer.begin();
}

void loop_ftp()
{
  if(ftpenabled) 
  {
  /* ---------- FTP : ProFTPD 1.3.7c ---------- */
  if (WiFiClient client = ftpServer.accept()){
    handleBannerGrab(ftpHookweb,client, 21, "220 ProFTPD 1.3.7c Server (Debian) [::ffff:192.168.1.10]\r\n");
    logCommand(ftpHookweb, client.remoteIP().toString(), 21, "FTP login","FTP"); 
    }
  }  
}