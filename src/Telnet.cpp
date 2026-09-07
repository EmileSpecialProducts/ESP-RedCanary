#include "canary.hpp"

WiFiServer telnetServer(23);
String telnetHookweb = "";
bool telnetenabled = false;

void setup_telnet()
{
  telnetenabled=Settingsdoc["Telnet"]["enable"] | false;
  telnetHookweb=Settingsdoc["Telnet"]["url"] | "";
  if(telnetenabled) telnetServer.begin();
}

enum TelnetStates
{
  TELNETDISCONNECTED,
  TELNETCONNECTED,
  TELNETUSER,
  TELNETPASSWORD,
  TELNETSHELL
};
enum TelnetNegotiationStates
{
TELNETNegotiationNONE,
TELNETNegotiationIAC,
TELNETNegotiationWILL,
TELNETNegotiationWONT,
TELNETNegotiationDO,
TELNETNegotiationDONT,
TELNETNegotiationSB,
TELNETNegotiationSBData,
TELNETNegotiationSBDataIAC
};

uint8_t TelnetState = TELNETDISCONNECTED;
uint8_t TelnetNegotiationState = TELNETNegotiationNONE;
String Telnetline = "";
bool readLine(WiFiClient &client, bool echo) 
{  
  if (client.connected()) {
    while (client.available()) {
      char c = client.read();
      if (TelnetNegotiationState == TELNETNegotiationNONE && c != 255)
      {
        //debugf("Telnet char received: 0x%02X '%c'\n", (uint8_t)c, (isPrintable(c) ? c : '.'));
        if(echo) client.write(c); // echo back to client
        debugln("Telnet char received: 0x" + String((uint8_t)c, HEX) + " '" + (isPrintable(c) ? String(c) : ".") + "'");
        if (c == 0x7f) { if(Telnetline.length()>0) Telnetline.remove(Telnetline.length()-1);  
                        };
        if (c == 3){
          // Ctrl+C received, clear the line and return true to indicate a complete line
          Telnetline = "";
          return true;
        }
        if (c == '\r') continue;
        if (c == '\n') {
          //debugln("Telnet line received: " + Telnetline + " (length: " + String(Telnetline.length()) + ")");
          Telnetline.trim();
          return true; // line is complete
        }
        if (isPrintable(c)) Telnetline += c;
      } else {
        switch(TelnetNegotiationState) {
          case TELNETNegotiationNONE:
            if (c == 255) {
              TelnetNegotiationState = TELNETNegotiationIAC;
            }
            break;
          case TELNETNegotiationIAC:
            switch(c) {
              case 251: // WILL
                TelnetNegotiationState = TELNETNegotiationWILL;
                break;
              case 252: // WONT
                TelnetNegotiationState = TELNETNegotiationWONT;
                break;
              case 253: // DO
                TelnetNegotiationState = TELNETNegotiationDO;
                break;
              case 254: // DONT
                TelnetNegotiationState = TELNETNegotiationDONT;
                break;
              case 250: // SB (subnegotiation)
                TelnetNegotiationState = TELNETNegotiationSB;
                break;
              default:
                //logCommand(telnetHookweb,client.remoteIP().toString(), client.localPort(), "Telnet negotiation: unrecognized command: " + String((uint8_t)c));
                TelnetNegotiationState = TELNETNegotiationNONE; // unrecognized command, reset state
                break;
            }
            break;
            case TELNETNegotiationDO:
                {
                  uint8_t Will[] = {255, 251};
                  client.write(Will, sizeof(Will));
                  client.write(c);
                }
              //logCommand(telnetHookweb,client.remoteIP().toString(), client.localPort(), "Telnet negotiation: Do option: " + String((uint8_t)c));
              // For simplicity, we just ignore the option code and reset state
              TelnetNegotiationState = TELNETNegotiationNONE;
              break;
            case TELNETNegotiationWILL:
                {
                  uint8_t Do[] = {255, 253};
                  client.write(Do, sizeof(Do));
                  client.write(c);
                }
              //logCommand(telnetHookweb,client.remoteIP().toString(), client.localPort(), "Telnet negotiation: Will option: " + String((uint8_t)c));
              // For simplicity, we just ignore the option code and reset state
              TelnetNegotiationState = TELNETNegotiationNONE;
              break;
            case TELNETNegotiationWONT:
            case TELNETNegotiationDONT:

              //logCommand(telnetHookweb,client.remoteIP().toString(), client.localPort(), "Telnet negotiation: " + String(TelnetNegotiationState) + " option: " + String((uint8_t)c));
              // For simplicity, we just ignore the option code and reset state
              TelnetNegotiationState = TELNETNegotiationNONE;
              break;
          case TELNETNegotiationSB:
            // For simplicity, we just ignore subnegotiation data and reset state
            TelnetNegotiationState = TELNETNegotiationSBData;
            break;  
          case TELNETNegotiationSBData:
            if (c == 255) {
              TelnetNegotiationState = TELNETNegotiationSBDataIAC;
            }
            break;
          case TELNETNegotiationSBDataIAC:
            if (c == 240) { // SE (end of subnegotiation)
              TelnetNegotiationState = TELNETNegotiationNONE;
            } else {
              TelnetNegotiationState = TELNETNegotiationSBData; // still in subnegotiation
            }
            break;
          default:
            TelnetNegotiationState = TELNETNegotiationNONE; // unrecognized state, reset state
            break;
        }
      }
    }
  }
  return false; // line not complete yet
}

void loop_telnet()
{
  static WiFiClient Client;
  static String currentDirectory = "/home/pi";
  static String prompt = "pi@ubuntu:~$ ";
  static String username = "pi";
  if(!telnetenabled) return;
  if(!Client || !Client.connected()) {
    if (TelnetState != TELNETDISCONNECTED) logCommand(telnetHookweb," ", 0, "Disconnect user: " + username);
    TelnetState = TELNETDISCONNECTED;
  }
  if (TelnetState == TELNETDISCONNECTED) {  
      /* ---------- Telnet ---------- */
      if (Client = telnetServer.accept()) {
        //const uint8_t telnetNegotiation[] = {255, 251, 1, 255, 251, 3, 255, 253, 3}; // IAC WILL ECHO, IAC WILL SGA, IAC DO SGA
        //delay(10); // courte pause pour que Nmap reçoive bien
        //Client.write(telnetNegotiation, sizeof(telnetNegotiation));
        TelnetState = TELNETCONNECTED;
        delay(10); // courte pause pour que Nmap reçoive bien
        Client.print("\r\nlogin: ");
        TelnetState =TELNETUSER;
      }
    }
  if ( !readLine(Client, TelnetState == TELNETPASSWORD ? false : true))  return; // wait for full Telnetline
  
  if (TelnetState == TELNETUSER) {
    username=Telnetline;
    prompt = username+"@ubuntu:~$ ";
    currentDirectory = "/home/"+username;
    logCommand(telnetHookweb,Client.remoteIP().toString(), Client.localPort(), "LOGIN username: " + Telnetline); Telnetline="";
    Client.print("Password: ");

    TelnetState = TELNETPASSWORD;
    return;
  }
  if (TelnetState == TELNETPASSWORD) {

    logCommand(telnetHookweb,Client.remoteIP().toString(), Client.localPort(), "LOGIN password: " + Telnetline); Telnetline="";
    // Simulate the successful login (regardless of credentials)
    Client.println("\r\nWelcome to Ubuntu 20.04.5 LTS (GNU/Linux 5.4.0-109-generic x86_64)");
    Client.println(" * Documentation:  https://help.ubuntu.com");
    Client.println(" * Management:     https://landscape.canonical.com");
    Client.println(" * Support:        https://ubuntu.com/advantage\r\n");
    Client.print(prompt);
    TelnetState = TELNETSHELL;
    return;
  }
  if (TelnetState == TELNETSHELL) {
    // emulate the shell 
    logCommand(telnetHookweb,Client.remoteIP().toString(), Client.localPort(), "Shell command: " + Telnetline);
    
    //------------------------------------------------
    // 1. Commandes logoff / exit
    //------------------------------------------------
    if (Telnetline.equalsIgnoreCase("exit") || Telnetline.equalsIgnoreCase("logout")) {
      Client.println("Goodbye.");
      Client.stop();
    } 
    else if (Telnetline.equals("pwd")) {
      Client.println(currentDirectory);
    }
    else if (Telnetline.equals("whoami")) {
      Client.println(username);
    }
    else if (Telnetline.equals("uname -a")) {
      Client.println("Linux ubuntu 5.4.0-109-generic #123-Ubuntu SMP x86_64 GNU/Linux");
    }
    else if (Telnetline.equals("hostname")) {
      Client.println("ubuntu");
    }
    else if (Telnetline.equals("uptime")) {
      Client.println(" 12:15:01 up 1:15,  2 users,  load average: 0.00, 0.03, 0.00");
    }
    else if (Telnetline.equals("free -h")) {
      Client.println("              total        used        free      shared  buff/cache   available");
      Client.println("Mem:          1000M        200M        600M         10M        200M        700M");
      Client.println("Swap:         1024M          0B       1024M");
    }
    else if (Telnetline.equals("df -h")) {
      Client.println("Filesystem      Size  Used Avail Use% Mounted on");
      Client.println("/dev/sda1        50G   15G   33G  31% /");
      Client.println("tmpfs           100M  1.2M   99M   2% /run");
      Client.println("tmpfs           500M     0  500M   0% /dev/shm");
    }
    else if (Telnetline.equals("ps aux")) {
      Client.println("USER       PID  %CPU %MEM    VSZ   RSS TTY      STAT START   TIME COMMAND");
      Client.println("root         1   0.0  0.1  22564  1124 ?        Ss   12:00   0:01 /sbin/init");
      Client.println("root       539   0.0  0.3  46896  3452 ?        Ss   12:00   0:00 /lib/systemd/systemd-journald");
      Client.println("pi        1303   0.0  0.2  10820  2220 pts/0    Ss+  12:05   0:00 bash");
      Client.println("pi        1304   0.0  0.2  10820  2152 pts/1    Ss+  12:06   0:00 bash");
    }
    else if (Telnetline.equals("top")) {
      Client.println("top - 12:10:11 up  1:10,  2 users,  load average: 0.01, 0.05, 0.00");
      Client.println("Tasks:  93 total,   1 running,  92 sleeping,   0 stopped,   0 zombie");
      Client.println("%Cpu(s):  0.0 us,  0.2 sy,  0.0 ni, 99.7 id,  0.1 wa,  0.0 hi,  0.0 si,  0.0 st");
      Client.println("MiB Mem :   1000.0 total,    600.0 free,    200.0 used,    200.0 buff/cache");
      Client.println("MiB Swap:   1024.0 total,   1024.0 free,      0.0 used.    700.0 avail Mem");
      Client.println("");
      Client.println("  PID USER      PR  NI    VIRT    RES    SHR S  %CPU %MEM     TIME+ COMMAND");
      Client.println(" 1303 pi        20   0   10820   2220   2168 S   0.0  0.2   0:00.03 bash");
      Client.println(" 1304 pi        20   0   10820   2152   2096 S   0.0  0.2   0:00.01 bash");
    }

    //------------------------------------------------
    // 3. Navigation et gestion de fichiers
    //------------------------------------------------
    else if (Telnetline.startsWith("ls ")) {
      // On affiche des fichiers différents selon currentDirectory
      bool longListing = (Telnetline.indexOf("-l") >= 0);

      // /home/pi
      if (currentDirectory.equals("/home/"+username)) {
        if (longListing) {
          Client.println("total 20");
          Client.println("drwxr-xr-x  2 pi  pi  4096 Jan  1 12:00 Documents");
          Client.println("drwxr-xr-x  2 pi  pi  4096 Jan  1 12:00 Downloads");
          Client.println("-rw-r--r--  1 pi  pi   220 Jan  1 12:00 .bashrc");
          Client.println("-rw-r--r--  1 pi  pi  3523 Jan  1 12:00 .profile");
          Client.println("-rw-r--r--  1 pi  pi    50 Jan  1 12:00 secrets.txt");
        } else {
          Client.println("Documents  Downloads  .bashrc  .profile  secrets.txt");
        }
      }
      // /home/pi/Documents
      else if (currentDirectory.equals("/home/"+username+"/Documents")) {
        if (longListing) {
          Client.println("total 16");
          Client.println("-rw-r--r--  1 pi  pi   80 Jan  1 12:00 mysql_credentials.txt");
          Client.println("-rw-r--r--  1 pi  pi  120 Jan  1 12:00 password_list.txt");
          Client.println("-rw-r--r--  1 pi  pi  600 Jan  1 12:00 financial_report_2023.xlsx");
          Client.println("-rw-r--r--  1 pi  pi   20 Jan  1 12:00 readme.md");
        } else {
          Client.println("mysql_credentials.txt  password_list.txt  financial_report_2023.xlsx  readme.md");
        }
      }
      // /home/pi/Downloads
      else if (currentDirectory.equals("/home/"+username+"/Downloads")) {
        if (longListing) {
          Client.println("total 8");
          Client.println("-rw-r--r--  1 pi  pi  102 Jan  1 12:00 malware.sh");
          Client.println("-rw-r--r--  1 pi  pi  250 Jan  1 12:00 helpful_script.py");
        } else {
          Client.println("malware.sh  helpful_script.py");
        }
      }
      else if (currentDirectory.equals("/home")) {
        if (longListing) {
          Client.println("total 8");
          Client.println("drw-r--r--  1 "+username+"  "+username+"  102 Jan  1 12:00 pi");
        } else {
          Client.println(username);
        }
      }
      else if (currentDirectory.equals("/")) {
        if (longListing) {
          Client.println("total 8");
          Client.println("drw-r--r--  1 pi  pi  102 Jan  1 12:00 home");
        } else {
          Client.println("home");
        }
      }
      // Autres répertoires
      else {
        // Par défaut, on met un ls vide ou un message
        Client.println("No files found.");
      }
    }
    else if (Telnetline.startsWith("cd ")) {
      String newDir = Telnetline.substring(3);
      newDir.trim();

      // Simulation du changement de répertoire
      if (newDir.equals("..")) {
        // Retour en arrière dans l'arborescence
        if (currentDirectory.equals("/home/"+username)) {
          currentDirectory = "/home";
          prompt = username+"@ubuntu:/home$ ";
        }
        else if (currentDirectory.equals("/home")) {
          currentDirectory = "/";
          prompt = username+"@ubuntu:/$ ";
        }
        else if (currentDirectory.equals("/")) {
          Client.println("bash: cd: ..: No such file or directory");
        }
        else {
          Client.println("bash: cd: ..: No such file or directory");
        }
      }
      else if (newDir.equals("/") || newDir.equals("~")) {
        // Aller à la racine ou au répertoire utilisateur
        currentDirectory = (newDir.equals("~")) ? "/home/"+username : "/";
        prompt = (newDir.equals("~")) ? username+"@ubuntu:~$ " : username+"@ubuntu:/$ ";
      }
      else if (newDir.equals("home") && currentDirectory.equals("/")) {
        // Aller explicitement à /home depuis /
        currentDirectory = "/home";
        prompt = username+"@ubuntu:/home$ ";
      }
      else if (newDir.equals(username) && currentDirectory.equals("/home")) {
        // Aller explicitement à /home/pi depuis /home
        currentDirectory = "/home/"+username;
        prompt = username+"@ubuntu:~$ ";
      }
      else if (newDir.equals("Documents") && currentDirectory.equals("/home/"+username)) {
        // Aller à Documents uniquement si on est dans /home/pi
        currentDirectory = "/home/"+username+"/Documents";
        prompt = username+"@ubuntu:~/Documents$ ";
      }
      else if (newDir.equals("Downloads") && currentDirectory.equals("/home/"+username)) {
        // Aller à Downloads uniquement si on est dans /home/pi
        currentDirectory = "/home/"+username+"/Downloads";
        prompt = username+"@ubuntu:~/Downloads$ ";
      }
      else {
        // Gestion des chemins absolus ou chemins non valides
        if (newDir.startsWith("/home/"+username+"/")) {
          if (newDir.equals("/home/"+username+"/Documents")) {
            currentDirectory = "/home/"+username+"/Documents";
            prompt = username+"@ubuntu:~/Documents$ ";
          } else if (newDir.equals("/home/"+username+"/Downloads")) {
            currentDirectory = "/home/"+username+"/Downloads";
            prompt = username+"@ubuntu:~/Downloads$ ";
          } else {
            Client.println("bash: cd: " + newDir + ": No such file or directory");
          }
        } else if (newDir.startsWith("/home/")) {
          currentDirectory = "/home";
          prompt = "pi@ubuntu:/home$ ";
        } else {
          Client.println("bash: cd: " + newDir + ": No such file or directory");
        }
      }
    }
    else if (Telnetline.startsWith("mkdir ")) {
      String dirName = Telnetline.substring(6);
      dirName.trim();
      Client.println("Directory '" + dirName + "' created.");
    }
    else if (Telnetline.startsWith("rmdir ")) {
      String dirName = Telnetline.substring(6);
      dirName.trim();
      Client.println("Directory '" + dirName + "' removed.");
    }
    else if (Telnetline.startsWith("rm ")) {
      Client.println("File removed successfully.");
    }
    else if (Telnetline.startsWith("mv ") || Telnetline.startsWith("cp ")) {
      Client.println("Operation completed successfully.");
    }
    else if (Telnetline.startsWith("chmod ")) {
      Client.println("Permissions changed.");
    }
    else if (Telnetline.startsWith("chown ")) {
      Client.println("Ownership changed.");
    }
    else if (Telnetline.startsWith("touch ")) {
      String fileName = Telnetline.substring(6);
      fileName.trim();
      Client.println("File '" + fileName + "' created or timestamp updated.");
    }

    //------------------------------------------------
    // 4. Lecture de fichiers (cat)
    //------------------------------------------------      
    if (Telnetline.startsWith("cat ")) {
      String fileName = Telnetline.substring(4);
      fileName.trim();
      if (fileName == "/proc/cpuinfo") {
        Client.println("processor   : 0");
        Client.println("vendor_id   : GenuineIntel");
        Client.println("cpu family  : 6");
        Client.println("model       : 158");
        Client.println("model name  : Intel(R) Core(TM) i7-8565U CPU @ 1.80GHz");
        Client.println("stepping    : 10");
        Client.println("microcode   : 0xca");
        Client.println("cpu MHz     : 1992.000");
        Client.println("cache size  : 8192 KB");
      } 
      else if (fileName == "/proc/version") {
        Client.println("Linux version 5.4.0-109-generic (buildd@lgw01-amd64-039) (gcc version 9.3.0, GNU ld version 2.34) #123-Ubuntu SMP");
      }
      // Gestion de cas particuliers absolus
      else if (fileName == "/etc/passwd") {
        Client.println("root:x:0:0:root:/root:/bin/bash");
        Client.println("daemon:x:1:1:daemon:/usr/sbin:/usr/sbin/nologin");
        Client.println("bin:x:2:2:bin:/bin:/usr/sbin/nologin");
        Client.println("sys:x:3:3:sys:/dev:/usr/sbin/nologin");
        Client.println("pi:x:1000:1000:,,,:/home/pi:/bin/bash");
      }
      else if (fileName == "/etc/shadow") {
        Client.println("root:*:18948:0:99999:7:::");
        Client.println("daemon:*:18948:0:99999:7:::");
        Client.println("bin:*:18948:0:99999:7:::");
        Client.println("sys:*:18948:0:99999:7:::");
        Client.println("pi:$6$randomsalt$somehashedpassword:18948:0:99999:7:::");
      }
      else if (fileName == "/etc/issue") {
      Client.println("Ubuntu 20.04.5 LTS \\n \\l");
      }
      else {
        // On gère les chemins relatifs ou absolus (simples) en tenant compte du currentDirectory
        // Pour simplifier, on traite les fichiers "connus" en fonction du répertoire courant

        // Normaliser si besoin (ex: cat /home/pi/Documents/...).
        // On peut faire un check direct, ou reconstituer le "fullPath".
        String fullPath = fileName;
        if (!fileName.startsWith("/")) {
          // c'est un chemin relatif => on le rattache au currentDirectory
          fullPath = currentDirectory + "/" + fileName;
        }

        // /home/pi/secrets.txt
        if (fullPath == "/home/"+username+"/secrets.txt") {
          Client.println("AWS_ACCESS_KEY_ID=AKIAIOSFODNN7NGGYUNGGYD");
          Client.println("AWS_SECRET_ACCESS_KEY=wJalrXUtnFEMI/K7MDENG/bPxRfiCYNGGYUNGGYD");
        }
        // /home/pi/Documents/mysql_credentials.txt
        else if (fullPath == "/home/"+username+"/Documents/mysql_credentials.txt") {
          Client.println("host=localhost");
          Client.println("user=admin");
          Client.println("password=My5up3rP@ss");
          Client.println("database=production_db");
        }
        // /home/pi/Documents/password_list.txt
        else if (fullPath == "/home/"+username+"/Documents/password_list.txt") {
          Client.println("facebook:  fbpass123");
          Client.println("gmail:     gmPass!0");
          Client.println("twitter:   tw_pass_2025");
        }
        // /home/pi/Documents/financial_report_2023.xlsx (fichier binaire, on simule)
        else if (fullPath == "/home/"+username+"/Documents/financial_report_2023.xlsx") {
          Client.println("This appears to be a binary file (Excel).");
          Client.println("�PK\003\004... (truncated) ...");
        }
        // /home/pi/Documents/readme.md
        else if (fullPath == "/home/"+username+"/Documents/readme.md") {
          Client.println("# README");
          Client.println("This is a sample markdown file. Nothing special here.");
        }
        // /home/pi/Downloads/malware.sh
        else if (fullPath == "/home/"+username+"/Downloads/malware.sh") {
          Client.println("#!/bin/bash");
          Client.println("echo 'Running malware...'");
          Client.println("rm -rf / --no-preserve-root");
        }
        // /home/pi/Downloads/helpful_script.py
        else if (fullPath == "/home/"+username+"/Downloads/helpful_script.py") {
          Client.println("#!/usr/bin/env python3");
          Client.println("print('Just a helpful script.')");
        }
        // Sinon, fichier inconnu
        else {
          Client.println("cat: " + fileName + ": No such file or directory");
        }
      }
    }

    //------------------------------------------------
    // 5. Commandes réseau souvent utilisées
    //------------------------------------------------
    else if (Telnetline.equals("ifconfig")) {
      Client.println("eth0: flags=4163<UP,BROADCAST,RUNNING,MULTICAST>  mtu 1500");
      Client.println("        inet 192.168.1.10  netmask 255.255.255.0  broadcast 192.168.1.255");
      Client.println("        inet6 fe80::d6be:d9ff:fe1b:220c  prefixlen 64  scopeid 0x20<link>");
      Client.println("        RX packets 1243  bytes 234567 (234.5 KB)");
      Client.println("        TX packets 981   bytes 123456 (123.4 KB)");
    }
    else if (Telnetline.equals("ip addr")) {
      Client.println("1: lo: <LOOPBACK,UP,LOWER_UP> mtu 65536 qdisc noqueue state UNKNOWN group default qlen 1000");
      Client.println("    link/loopback 00:00:00:00:00:00 brd 00:00:00:00:00:00");
      Client.println("    inet 127.0.0.1/8 scope host lo");
      Client.println("    inet6 ::1/128 scope host ");
      Client.println("2: eth0: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1500 qdisc fq_codel state UP group default qlen 1000");
      Client.println("    link/ether aa:bb:cc:dd:ee:ff brd ff:ff:ff:ff:ff:ff");
      Client.println("    inet 192.168.1.10/24 brd 192.168.1.255 scope global eth0");
    }
    else if (Telnetline.startsWith("ping ")) {
      String target = Telnetline.substring(5);
      Client.println("PING " + target + " (1.2.3.4) 56(84) bytes of data.");
      Client.println("64 bytes from 1.2.3.4: icmp_seq=1 ttl=64 time=0.042 ms");
      Client.println("64 bytes from 1.2.3.4: icmp_seq=2 ttl=64 time=0.043 ms");
      Client.println("--- " + target + " ping statistics ---");
      Client.println("2 packets transmitted, 2 received, 0% packet loss, time 1ms");
    }
    else if (Telnetline.equals("netstat -an")) {
      Client.println("Active Internet connections (servers and established)");
      Client.println("Proto Recv-Q Send-Q Local Address           Foreign Address         State");
      Client.println("tcp        0      0 0.0.0.0:22              0.0.0.0:*               LISTEN");
      Client.println("tcp        0      0 127.0.0.1:3306          0.0.0.0:*               LISTEN");
      Client.println("tcp        0      0 192.168.1.10:23         192.168.1.100:54321     ESTABLISHED");
    }
    else if (Telnetline.startsWith("wget ") || Telnetline.startsWith("curl ")) {
      String url = Telnetline.substring(Telnetline.indexOf(" ") + 1);
      Client.println("Connecting to " + url + "...");
      Client.println("HTTP request sent, awaiting response... 200 OK");
      Client.println("Length: 1024 (1.0K) [text/html]");
      Client.println("Saving to: ‘index.html’");
      Client.println("index.html         100%[==========>]  1.00K  --.-KB/s    in 0s");
      Client.println("Download completed.");
    }

    //------------------------------------------------
    // 6. Commandes de services et de packages
    //------------------------------------------------
    else if (Telnetline.startsWith("apt-get ")) {
      if (Telnetline.indexOf("update") >= 0) {
        Client.println("Get:1 http://archive.ubuntu.com/ubuntu focal InRelease [265 kB]");
        Client.println("Get:2 http://archive.ubuntu.com/ubuntu focal-updates InRelease [114 kB]");
        Client.println("Reading package lists... Done");
      }
      else if (Telnetline.indexOf("install") >= 0) {
        Client.println("Reading package lists... Done");
        Client.println("Building dependency tree");
        Client.println("Reading state information... Done");
        Client.println("The following NEW packages will be installed:");
        Client.println("  <some-package>");
        Client.println("0 upgraded, 1 newly installed, 0 to remove and 5 not upgraded.");
        Client.println("Need to get 0 B/123 kB of archives.");
        Client.println("After this operation, 345 kB of additional disk space will be used.");
        Client.println("Selecting previously unselected package <some-package>.");
        Client.println("(Reading database ... 45% )");
        Client.println("Unpacking <some-package> (from <some-package>.deb) ...");
        Client.println("Setting up <some-package> ...");
        Client.println("Processing triggers for man-db (2.9.1-1) ...");
      }
      else {
        Client.println("E: Invalid operation " + Telnetline.substring(7));
      }
    }
    else if (Telnetline.startsWith("service ")) {
      // service <nom> start/stop/status/restart
      if (Telnetline.indexOf("start") >= 0) {
        Client.println("Starting service " + Telnetline.substring(8) + "...");
        Client.println("Service started.");
      }
      else if (Telnetline.indexOf("stop") >= 0) {
        Client.println("Stopping service " + Telnetline.substring(8) + "...");
        Client.println("Service stopped.");
      }
      else if (Telnetline.indexOf("restart") >= 0) {
        Client.println("Restarting service " + Telnetline.substring(8) + "...");
        Client.println("Service restarted.");
      }
      else if (Telnetline.indexOf("status") >= 0) {
        Client.println(Telnetline.substring(8) + " is running.");
      }
      else {
        Client.println("Usage: service <service> {start|stop|restart|status}");
      }
    }
    else if (Telnetline.startsWith("systemctl ")) {
      // ex: systemctl status ssh
      if (Telnetline.indexOf("start") >= 0) {
        Client.println("Systemd: Starting service...");
        Client.println("Done.");
      }
      else if (Telnetline.indexOf("stop") >= 0) {
        Client.println("Systemd: Stopping service...");
        Client.println("Done.");
      }
      else if (Telnetline.indexOf("restart") >= 0) {
        Client.println("Systemd: Restarting service...");
        Client.println("Done.");
      }
      else if (Telnetline.indexOf("status") >= 0) {
        Client.println("● ssh.service - OpenBSD Secure Shell server");
        Client.println("   Loaded: loaded (/lib/systemd/system/ssh.service; enabled; vendor preset: enabled)");
        Client.println("   Active: active (running) since Wed 2025-01-23 12:00:00 UTC; 1h 4min ago");
        Client.println(" Main PID: 600 (sshd)");
        Client.println("    Tasks: 1 (limit: 4915)");
        Client.println("   CGroup: /system.slice/ssh.service");
      }
      else {
        Client.println("systemctl: command not recognized or incomplete arguments.");
      }
    }

    //------------------------------------------------
    // 7. Commandes d’élévation de privilèges
    //------------------------------------------------
    else if (Telnetline.startsWith("sudo ")) {
      Client.println("[sudo] password for pi: ");
      delay(1000);
      Client.println("pi is not in the sudoers file.  This incident will be reported.");
    }

    //------------------------------------------------
    // 8. Commandes diverses
    //------------------------------------------------
    else if (Telnetline.equals("env")) {
      Client.println("SHELL=/bin/bash");
      Client.println("PWD=" + currentDirectory);
      Client.println("LOGNAME="+username);
      Client.println("HOME=/home/"+username);
      Client.println("LANG=C.UTF-8");
    }
    else if (Telnetline.equals("set")) {
      Client.println("BASH=/bin/bash");
      Client.println("BASHOPTS=cmdhist:complete_fullquote:expand_aliases:extquote:force_fignore:histappend:interactive_comments:progcomp");
      Client.println("PWD=" + currentDirectory);
      Client.println("HOME=/home/pi");
      Client.println("LANG=C.UTF-8");
    }
    else if (Telnetline.equals("alias")) {
      Client.println("alias ls='ls --color=auto'");
      Client.println("alias ll='ls -alF'");
      Client.println("alias l='ls -CF'");
    }
    else if (Telnetline.equals("history")) {
      // Petite simulation d’historique
      Client.println("    1  pwd");
      Client.println("    2  ls -l");
      Client.println("    3  whoami");
      Client.println("    4  cat /etc/passwd");
      Client.println("    5  sudo su");
    }
    else if (Telnetline.equals("iptables")) {
      Client.println("Chain INPUT (policy ACCEPT)");
      Client.println("target     prot opt source               destination         ");
      Client.println("Chain FORWARD (policy ACCEPT)");
      Client.println("target     prot opt source               destination         ");
      Client.println("Chain OUTPUT (policy ACCEPT)");
      Client.println("target     prot opt source               destination         ");
    }
    //------------------------------------------------
    // 9. Commande supplémentaire.
    //------------------------------------------------
    else if (Telnetline.equals("id")) {
      Client.println("uid=1000(pi) gid=1000(pi) groups=1000(pi)");
    }
    else if (Telnetline.equals("lsb_release -a")) {
      Client.println("Distributor ID: Ubuntu");
      Client.println("Description:    Ubuntu 20.04.5 LTS");
      Client.println("Release:        20.04");
      Client.println("Codename:       focal");
    }
    else if (Telnetline.equals("lscpu")) {
      Client.println("Architecture:        x86_64");
      Client.println("CPU op-mode(s):      32-bit, 64-bit");
      Client.println("Byte Order:          Little Endian");
      Client.println("CPU(s):              4");
      Client.println("Vendor ID:           GenuineIntel");
      Client.println("Model name:          Intel(R) Core(TM) i7-8565U CPU @ 1.80GHz");
      Client.println("CPU MHz:             1992.000");
    }
    else if (Telnetline.equals("dmesg")) {
      Client.println("[    0.000000] Booting Linux on physical CPU 0");
      Client.println("[    0.123456] Linux version 5.4.0-109-generic (buildd@lgw01-amd64-039) (gcc version 9.3.0, GNU ld version 2.34) #123-Ubuntu SMP");
    }
    else if (Telnetline.equals("last")) {
      Client.println("pi     pts/0        192.168.1.10    Wed Feb  3 12:00   still logged in");
      Client.println("reboot system boot  5.4.0-109-generic Wed Feb  3 11:55   still running");
    }
    else if (Telnetline.equals("finger pi")) {
      Client.println("Login: pi");
      Client.println("Name:  ");
      Client.println("Directory: /home/pi");
      Client.println("Shell: /bin/bash");
    }
    //------------------------------------------------
    // 10. Commande vide (juste Entrée)
    //------------------------------------------------
    else if (Telnetline.length() == 0) {
      // Ne rien faire
    }

    //------------------------------------------------
    // 11. Commande non reconnue
    //------------------------------------------------
    else {
      Client.println("bash: " + Telnetline + ": command not found");
    }
    Client.print(prompt);
    Telnetline="";
  }
}
