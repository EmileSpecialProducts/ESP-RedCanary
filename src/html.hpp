#include <arduino.h>

const uint8_t style_css[] PROGMEM = R"rawliteral(

 body {
	background: #36393e;
  margin-right: 10%;
  margin-left: 10% ;
  color: #bfbfbf;
	font-family: sans-serif;
}

.field-icon {
  float: right;
  margin-left: -25px;
  margin-top: -25px;
  position: relative;
  z-index: 2;
}

.container{
  padding-top:50px;
  margin: auto;
}

a:link {
  color: #808080;
}

/* visited link */
a:visited {
  color: #A0A0A0;
}
/* Tables */

table {
	width: 100%;
	min-width: 400px;
	margin-bottom: 2em;
	border-collapse: collapse;
}

th {
  	word-break: break-word;
}

th, td {
	padding: 10px 6px;
	text-align: left;
	vertical-align: top;
	border-bottom: 1px solid #5d5d5d;
}

td:nth-child(1) {
  width: 20%;
}

td:nth-child(2) {
}

h1 {
	font-size: 1.4rem;
	margin-top: 1rem;
	background:
	#2f3136;
	padding: 10px;
	border-radius: 4px;
	border-left: solid #ec5b5b 5px;
	font-weight: 100;
	letter-spacing: 5px;
}
)rawliteral";

const uint8_t editor_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
  <head>
    <title>ESP-RedCanary Editor</title>
    <script src="https://emilespecialproducts.github.io/ESP-RedCanary/editor.js" type="text/javascript"></script> 
  </head>
  <body onload="onBodyLoad();">
    <div id="uploader"></div>
    <div id="tree" class="css-tree"></div>
    <div id="editor"></div>
    <div id="preview" style="display:none;"></div>
    <iframe id=download-frame style='display:none;'></iframe>
  </body>
</html>
)rawliteral";

const uint8_t error404_html[] PROGMEM = R"rawliteral(<!DOCTYPE html>
<html lang="en">
  <head>
    <title>ESP RedCanary</title>
  </head>
  <body>
    Page not found
  </body>
</html>
)rawliteral";

const uint8_t Index_html[] PROGMEM = R"rawliteral(
  <!DOCTYPE html>
<html>
<head>
  <title>ESP Red Canary Settings</title>
   <link rel="stylesheet" type="text/css" href="style.css">
</head>
<body>
  
  <h2>ESP Red Canary Settings</h2>
  
   <section>
    <h1>Wifi</h1> 
    <form id="configForm">
    <table>   
    <tr>
            <td><label for="ssid">SSID:</label></td>
            <td><input list="ssidlist" type="text" id="ssid" name="ssid">
            <datalist id="ssidlist">
            </datalist>
            </td>
    </tr>
    <tr>
            <td><label for="password">Password:</label></td>
            <td><input type="password" value="" autocomplete="current-password" required="" id="password">
            <input type="button" id="PasswordEyeButton" onclick="Toggelpassword()" value="&#x1F441;" >
            </td>
    </tr>
        <tr>
            <td><label for="apssid">APSSID:</label></td>
            <td><input  type="text" id="apssid" name="apssid">
            
            </td>
    </tr>
    <tr>
            <td><label for="appassword">ApPassword:</label></td>
            <td><input type="text" value="" id="appassword" name="appassword">
            </td>
    </tr>
        <tr>
            <td><label for="APCannel">APCannel:</label></td>
            <td><input type="number" id="APCannel" name="APCannel"></td> 
    </tr>
    </table>
    </form>
    </section>
    <section>
    <h1>Host</h1> 
    <table>
    <tr>
            <td><label for="ServerName">ServerName:</label></td>
            <td><input type="text" id="ServerName" name="ServerName"></td> 
    </tr>
    <tr>
            <td><label for="ServerMAC">MAC:</label></td>
            <td><input type="text" id="ServerMAC" name="ServerMAC"> <a href="https://udger.com/resources/mac-address-vendor" target="_blank"> Mac List </a></td>
    </tr>
    </table>
    </section>
    
    
    <section>
    <h1>Leds</h1> 
    <table>
    <tr>
            <td><label for="LedMaxCurrent">LedMaxCurrent:</label></td>
            <td><input type="number" id="LedMaxCurrent" name="LedMaxCurrent"></td> 
    </tr>
    <tr>
            <td><label for="LedPin">LedPin:</label></td>
            <td><input type="number" id="LedPin" name="LedPin"></td> 
    </tr>
    </table>
    </section>

    <section>
    <h1>Over The Air</h1> 
    <table>
    <tr>
            <td><label for="OverTheAir">Over The Air Updates:</label></td>
            <td><input type="checkbox" id="OverTheAir" name="OverTheAir"></td> 
    </tr>
    <tr>
            <td><label for="OTAPassword">OTA Password:</label></td>
            <td><input type="text" id="OTAPassword" name="OTAPassword"></td> 
    </tr>
    </table>
    </section>

    <section>
    <h1>Canarytokens</h1> 
    <table>
    <tr>
            <td><label for="canarytokenURL">canarytokenURL:</label></td>
            <td><input type="text" id="canarytokenURL" name="canarytokenURL"></td> 
            <td> This is the Default canarytoken, you can regester a Token at <a href="https://canarytokens.com" target="_blank">canarytokens.com</a>
            </td>
    </tr>
    </table>
    </section>


    <section>
    <h1>Http (80)</h1> 
    <table  id="Http_table">
    <tr> 
    <td><label for="Http_enable">Http:</label></td>
    <td><input type="checkbox" id="Http_enable" name="Http_enable" onclick="UpdateHide()"></td> 
    <td></td>
    </tr>
    <tr>
    <td><label for="Http_url">Http_url:</label></td>
    <td><input type="text" id="Http_url" name="Http_url"></td>
    <td>This is the URL that will be triggered when the Canary is activated</td>
    </tr>
    <tr>
    <td><label for="Http_LedColor">Led:</label></td>
    <td><input type="color" id="Http_LedColor" name="Http_LedColor"></td>
    <td></td>
    </tr>
    <tr>
    <td><label for="Http_LedStart">LedStart:</label></td>
    <td><input type="number" id="Http_LedStart" name="Http_LedStart"></td>
    <td></td>
    </tr>
    <tr>
    <td><label for="Http_LedSize">LedSize:</label></td>
    <td><input type="number" id="Http_LedSize" name="Http_LedSize"></td>
    <td></td>
    </tr>
    <tr>
    <td><label for="Http_LedOnTime">LedOnTime:</label></td>
    <td><input type="number" id="Http_LedOnTime" name="Http_LedOnTime"></td>
    <td>The time the Led stays on when this Canary is activated</td>
    </tr>
    </table>
    </section>

<section>
    <h1>Https (443)</h1> 
    <table  id="Https_table">
    <tr> 
    <td><label for="Https_enable">Https:</label></td>
    <td><input type="checkbox" id="Https_enable" name="Https_enable" onclick="UpdateHide()"></td> 
    <td></td>
    </tr>
    <tr>
    <td><label for="Https_url">Https_url:</label></td>
    <td><input type="text" id="Https_url" name="Https_url"></td>
    <td>This is the URL that will be triggered when the Canary is activated</td>
    </tr>
    <tr>
    <td><label for="Https_LedColor">Led:</label></td>
    <td><input type="color" id="Https_LedColor" name="Https_LedColor"></td>
    <td></td>
    </tr>
    <tr>
    <td><label for="Https_LedStart">LedStart:</label></td>
    <td><input type="number" id="Https_LedStart" name="Https_LedStart"></td>
    <td></td>
    </tr>
    <tr>
    <td><label for="Https_LedSize">LedSize:</label></td>
    <td><input type="number" id="Https_LedSize" name="Https_LedSize"></td>
    <td></td>
    </tr>
    <tr>
    <td><label for="Https_LedOnTime">LedOnTime:</label></td>
    <td><input type="number" id="Https_LedOnTime" name="Https_LedOnTime"></td>
    <td>The time the Led stays on when this Canary is activated</td>
    </tr>
    </table>
    </section>

 <section>
    <h1>Http alternate (8080)</h1> 
    <table  id="Httpa_table">
    <tr> 
    <td><label for="Httpa_enable">Httpa:</label></td>
    <td><input type="checkbox" id="Httpa_enable" name="Httpa_enable" onclick="UpdateHide()"></td> 
    <td></td>
    </tr>
    <tr>
    <td><label for="Httpa_url">Httpa_url:</label></td>
    <td><input type="text" id="Httpa_url" name="Httpa_url"></td>
    <td>This is the URL that will be triggered when the Canary is activated</td>
    </tr>
    <tr>
    <td><label for="Httpa_LedColor">Led:</label></td>
    <td><input type="color" id="Httpa_LedColor" name="Httpa_LedColor"></td>
    <td></td>
    </tr>
    <tr>
    <td><label for="Httpa_LedStart">LedStart:</label></td>
    <td><input type="number" id="Httpa_LedStart" name="Httpa_LedStart"></td>
    <td></td>
    </tr>
    <tr>
    <td><label for="Httpa_LedSize">LedSize:</label></td>
    <td><input type="number" id="Httpa_LedSize" name="Httpa_LedSize"></td>
    <td></td>
    </tr>
    <tr>
    <td><label for="Httpa_LedOnTime">LedOnTime:</label></td>
    <td><input type="number" id="Httpa_LedOnTime" name="Httpa_LedOnTime"></td>
    <td>The time the Led stays on when this Canary is activated</td>
    </tr>
    </table>
    </section>

    <section>
    <h1>SSH (22) </h1> 
    <table id="SSH_table">
    <tr> 
    <td><label for="SSH_enable">SSH:</label></td>
    <td><input type="checkbox" id="SSH_enable" name="SSH_enable" onclick="UpdateHide()"></td> 
    <td></td>
    
    </tr>
      <tr>
      <td><label for="SSH_url">SSH_url:</label></td>
      <td><input type="text" id="SSH_url" name="SSH_url"></td>
      <td>This is the URL that will be triggered when the Canary is activated</td>
      </tr>
      <tr>
      <td><label for="SSH_LedColor">Led:</label></td>
      <td><input type="color" id="SSH_LedColor" name="SSH_LedColor"></td>
      <td></td>
      </tr>
      <tr>
      <td><label for="SSH_LedStart">LedStart:</label></td>
      <td><input type="number" id="SSH_LedStart" name="SSH_LedStart"></td>
      <td></td>
      </tr>
      <tr>
      <td><label for="SSH_LedSize">LedSize:</label></td>
      <td><input type="number" id="SSH_LedSize" name="SSH_LedSize"></td>
      <td></td>
      </tr>
      <tr>
      <td><label for="SSH_LedOnTime">LedOnTime:</label></td>
      <td><input type="number" id="SSH_LedOnTime" name="SSH_LedOnTime"></td>
      <td>The time in seconds the Led stays on when this Canary is activated</td>
      </tr>
    </table>
    </section>

<section>
    <h1>Telnet (23) </h1> 
    <table id="Telnet_table">
    <tr> 
    <td><label for="Telnet_enable">Telnet:</label></td>
    <td><input type="checkbox" id="Telnet_enable" name="Telnet_enable" onclick="UpdateHide()"></td> 
    <td></td>
    
    </tr>
      <tr>
      <td><label for="Telnet_url">Telnet_url:</label></td>
      <td><input type="text" id="Telnet_url" name="Telnet_url"></td>
      <td>This is the URL that will be triggered when the Canary is activated</td>
      </tr>
      <tr>
      <td><label for="Telnet_LedColor">Led:</label></td>
      <td><input type="color" id="Telnet_LedColor" name="Telnet_LedColor"></td>
      <td></td>
      </tr>
      <tr>
      <td><label for="Telnet_LedStart">LedStart:</label></td>
      <td><input type="number" id="Telnet_LedStart" name="Telnet_LedStart"></td>
      <td></td>
      </tr>
      <tr>
      <td><label for="Telnet_LedSize">LedSize:</label></td>
      <td><input type="number" id="Telnet_LedSize" name="Telnet_LedSize"></td>
      <td></td>
      </tr>
      <tr>
      <td><label for="Telnet_LedOnTime">LedOnTime:</label></td>
      <td><input type="number" id="Telnet_LedOnTime" name="Telnet_LedOnTime"></td>
      <td>The time in seconds the Led stays on when this Canary is activated</td>
      </tr>
    </table>
    </section>
    
<section>
    <h1>Ftp (21) </h1> 
    <table id="Ftp_table">
    <tr> 
    <td><label for="Ftp_enable">Ftp:</label></td>
    <td><input type="checkbox" id="Ftp_enable" name="Ftp_enable" onclick="UpdateHide()"></td> 
    <td></td>
    
    </tr>
      <tr>
      <td><label for="Ftp_url">Ftp_url:</label></td>
      <td><input type="text" id="Ftp_url" name="Ftp_url"></td>
      <td>This is the URL that will be triggered when the Canary is activated</td>
      </tr>
      <tr>
      <td><label for="Ftp_LedColor">Led:</label></td>
      <td><input type="color" id="Ftp_LedColor" name="Ftp_LedColor"></td>
      <td></td>
      </tr>
      <tr>
      <td><label for="Ftp_LedStart">LedStart:</label></td>
      <td><input type="number" id="Ftp_LedStart" name="Ftp_LedStart"></td>
      <td></td>
      </tr>
      <tr>
      <td><label for="Ftp_LedSize">LedSize:</label></td>
      <td><input type="number" id="Ftp_LedSize" name="Ftp_LedSize"></td>
      <td></td>
      </tr>
      <tr>
      <td><label for="Ftp_LedOnTime">LedOnTime:</label></td>
      <td><input type="number" id="Ftp_LedOnTime" name="Ftp_LedOnTime"></td>
      <td>The time in seconds the Led stays on when this Canary is activated</td>
      </tr>
    </table>
    </section>

    
<section>
    <h1>Rdp (3389) </h1> 
    <table id="Rdp_table">
    <tr> 
    <td><label for="Rdp_enable">Rdp:</label></td>
    <td><input type="checkbox" id="Rdp_enable" name="Rdp_enable" onclick="UpdateHide()"></td> 
    <td></td>
    
    </tr>
      <tr>
      <td><label for="Rdp_url">Rdp_url:</label></td>
      <td><input type="text" id="Rdp_url" name="Rdp_url"></td>
      <td>This is the URL that will be triggered when the Canary is activated</td>
      </tr>
      <tr>
      <td><label for="Rdp_LedColor">Led:</label></td>
      <td><input type="color" id="Rdp_LedColor" name="Rdp_LedColor"></td>
      <td></td>
      </tr>
      <tr>
      <td><label for="Rdp_LedStart">LedStart:</label></td>
      <td><input type="number" id="Rdp_LedStart" name="Rdp_LedStart"></td>
      <td></td>
      </tr>
      <tr>
      <td><label for="Rdp_LedSize">LedSize:</label></td>
      <td><input type="number" id="Rdp_LedSize" name="Rdp_LedSize"></td>
      <td></td>
      </tr>
      <tr>
      <td><label for="Rdp_LedOnTime">LedOnTime:</label></td>
      <td><input type="number" id="Rdp_LedOnTime" name="Rdp_LedOnTime"></td>
      <td>The time in seconds the Led stays on when this Canary is activated</td>
      </tr>
    </table>
    </section>


    <button type="button" onclick="saveConfig()">Save</button>
    <button type="button" onclick="AskReboot()">Reboot</button>
    <a href="editor.html" target="_blank"><img src="https://raw.githubusercontent.com/EmileSpecialProducts/EmileSpecialProducts.github.io/refs/heads/main/ESP-RedCanary/editor.png" onerror="this.style.visibility = 'hidden'"></a>


  <script>
    function tablehide( table, show = false)
    {
      for (var i = 1, row; row = table.rows[i]; i++ ) {
           row.style.display = show ? '':'none';
        }
    }
    
    function UpdateHide()
    {
      tablehide(document.getElementById("Http_table") ,document.getElementById('Http_enable').checked); 
      tablehide(document.getElementById("Https_table") ,document.getElementById('Https_enable').checked); 
      tablehide(document.getElementById("Httpa_table") ,document.getElementById('Httpa_enable').checked);  
      tablehide(document.getElementById("SSH_table") ,document.getElementById('SSH_enable').checked);
      tablehide(document.getElementById("Telnet_table") ,document.getElementById('Telnet_enable').checked);
      tablehide(document.getElementById("Ftp_table") ,document.getElementById('Ftp_enable').checked);
      tablehide(document.getElementById("Rdp_table") ,document.getElementById('Rdp_enable').checked);
    }
    ////                                                                                ////  
    function Toggelpassword() {
    var inputpassword = document.getElementById("password");
    var EyeButton = document.getElementById("PasswordEyeButton");
      if (inputpassword.type === "password") {
        inputpassword.type = "text";
      } else {
        inputpassword.type = "password"
      }
    }
    ////                                                                                ////    
    
    const input = document.getElementById('ssid');
    const datalist = document.getElementById('ssidlist');

    input.addEventListener('focus', () => {
      input.value = ''; // Clear the input to show all options
    });

    input.addEventListener('blur', () => {
      setTimeout(() => {
        //input.value = ''; // Optionally clear input after selection
      }, 100); // Delay to allow selection
    });
    
    function setElementId( id, value,DefaultValue="") 
    {
      var element = document.getElementById(id);
      if( element.type== "checkbox" ) 
      {   
          if (DefaultValue ==="") DefaultValue=false;
          element.checked = value === undefined ? DefaultValue : value ;
      } else
      {
          element.value = value === undefined ? DefaultValue : value;
      }
    }

    // Load JSON from ESP32
    fetch('/config.json')
      .then(response => response.json())
      .then(data => {
        setElementId('ssid',data.ssid);
        setElementId('password',data.password);
        setElementId('apssid',data.apssid);
        setElementId('appassword',data.appassword);
        setElementId('APCannel',data.APCannel,1);
        
        setElementId('ServerMAC',data.ServerMAC, "xx:xx:xx:xx:xx:xx");
        if(data.ServerName.length==0) data.ServerName=undefined;
        setElementId('ServerName',data.ServerName,"ESPRedCanary")
        
        setElementId('LedMaxCurrent',data.LedMaxCurrent,500);
        setElementId('LedPin',data.LedPin,-1);
        setElementId('OverTheAir',data.OverTheAir);
        setElementId('OTAPassword',data.OTAPassword, "RedCanary" );
        
        if( ! data.Http )  data.Http={};
        setElementId('Http_enable',data.Http.enabl);
        setElementId('Http_url',data.Http.url);
        setElementId('Http_LedColor',data.Http.LedColor,"#FF0000");
        setElementId('Http_LedStart',data.Http.LedStart,0); 
        setElementId('Http_LedSize',data.Http.LedSize,0);
        setElementId('Http_LedOnTime',data.Http.LedOnTime,0);
            if( ! data.Https )  data.Https={};
        setElementId('Https_enable',data.Https.enabl);
        setElementId('Https_url',data.Https.url);
        setElementId('Https_LedColor',data.Https.LedColor,"#FF0000");
        setElementId('Https_LedStart',data.Https.LedStart,0); 
        setElementId('Https_LedSize',data.Https.LedSize,0);
        setElementId('Https_LedOnTime',data.Https.LedOnTime,0);
            if( ! data.Httpa )  data.Httpa={};
        setElementId('Httpa_enable',data.Httpa.enabl);
        setElementId('Httpa_url',data.Httpa.url);
        setElementId('Httpa_LedColor',data.Httpa.LedColor,"#FF0000");
        setElementId('Httpa_LedStart',data.Httpa.LedStart,0); 
        setElementId('Httpa_LedSize',data.Httpa.LedSize,0);
        setElementId('Httpa_LedOnTime',data.Httpa.LedOnTime,0);
        
        if( ! data.SSH )  data.SSH={};
        setElementId('SSH_enable',data.SSH.enable);
        setElementId('SSH_url',data.SSH.url);
        setElementId('SSH_LedColor',data.SSH.LedColor,"#FF8000");
        setElementId('SSH_LedStart',data.SSH.LedStart,0);
        setElementId('SSH_LedSize',data.SSH.LedSize,0);
        setElementId('SSH_LedOnTime',data.SSH.LedOnTime,60);
        
        if( ! data.Telnet )  data.Telnet={}; 
        setElementId('Telnet_enable', data.Telnet.enable,false);
        setElementId('Telnet_url', data.Telnet.url);
        setElementId('Telnet_LedColor', data.Telnet.LedColor,"#FF8000");
        setElementId('Telnet_LedStart', data.Telnet.LedStart,0);
        setElementId('Telnet_LedSize', data.Telnet.LedSize,0);
        setElementId('Telnet_LedOnTime', data.Telnet.LedOnTime,60);
        
        if( ! data.Ftp )  data.Ftp={}; 
        setElementId('Ftp_enable', data.Ftp.enable,false);
        setElementId('Ftp_url', data.Ftp.url);
        setElementId('Ftp_LedColor', data.Ftp.LedColor,"#FF8000");
        setElementId('Ftp_LedStart', data.Ftp.LedStart,0);
        setElementId('Ftp_LedSize', data.Ftp.LedSize,0);
        setElementId('Ftp_LedOnTime', data.Ftp.LedOnTime,60);
        
        if( ! data.Rdp )  data.Rdp={}; 
        setElementId('Rdp_enable', data.Rdp.enable,false);
        setElementId('Rdp_url', data.Rdp.url);
        setElementId('Rdp_LedColor', data.Rdp.LedColor,"#FF8000");
        setElementId('Rdp_LedStart', data.Rdp.LedStart,0);
        setElementId('Rdp_LedSize', data.Rdp.LedSize,0);
        setElementId('Rdp_LedOnTime', data.Rdp.LedOnTime,60);
        
        UpdateHide();
      });

    // Send updated JSON back to ESP32
    function saveConfig() {
      
      const updatedConfig = {
        ssid: document.getElementById('ssid').value,
        password: document.getElementById('password').value,
        apssid: document.getElementById('apssid').value,
        APCannel: parseInt(document.getElementById('APCannel').value),
        appassword: document.getElementById('appassword').value,
        ServerMAC: document.getElementById('ServerMAC').value,
        ServerName: document.getElementById('ServerName').value,
        LedMaxCurrent: parseInt(document.getElementById('LedMaxCurrent').value),
        LedPin: parseInt(document.getElementById('LedPin').value),
        OverTheAir: document.getElementById('OverTheAir').checked ,
        OTAPassword: document.getElementById('OTAPassword').value,

        Http:{enable: document.getElementById('Http_enable').checked ,
              LedColor: document.getElementById('Http_LedColor').value,
              LedStart: parseInt(document.getElementById('Http_LedStart').value),
              LedSize: parseInt(document.getElementById('Http_LedSize').value),
              url: document.getElementById('Http_url').value},
        Https:{enable: document.getElementById('Https_enable').checked ,
              LedColor: document.getElementById('Https_LedColor').value,
              LedStart: parseInt(document.getElementById('Https_LedStart').value),
              LedSize: parseInt(document.getElementById('Https_LedSize').value),
              url: document.getElementById('Https_url').value},
        Httpa:{enable: document.getElementById('Httpa_enable').checked ,
              LedColor: document.getElementById('Httpa_LedColor').value,
              LedStart: parseInt(document.getElementById('Httpa_LedStart').value),
              LedSize: parseInt(document.getElementById('Httpa_LedSize').value),
              url: document.getElementById('Httpa_url').value},
              
        SSH:{enable: document.getElementById('SSH_enable').checked ,
            LedColor: document.getElementById('SSH_LedColor').value,
            LedStart: parseInt(document.getElementById('SSH_LedStart').value),
            LedSize: parseInt(document.getElementById('SSH_LedSize').value),
            url: document.getElementById('SSH_url').value},
        Telnet:{enable: document.getElementById('Telnet_enable').checked ,
            LedColor: document.getElementById('Telnet_LedColor').value,
            LedStart: parseInt(document.getElementById('Telnet_LedStart').value),
            LedSize: parseInt(document.getElementById('Telnet_LedSize').value),
            url: document.getElementById('Telnet_url').value},
            
        Ftp:{enable: document.getElementById('Ftp_enable').checked ,
            LedColor: document.getElementById('Ftp_LedColor').value,
            LedStart: parseInt(document.getElementById('Ftp_LedStart').value),
            LedSize: parseInt(document.getElementById('Ftp_LedSize').value),
            url: document.getElementById('Ftp_url').value},

        Rdp:{enable: document.getElementById('Rdp_enable').checked ,
            LedColor: document.getElementById('Rdp_LedColor').value,
            LedStart: parseInt(document.getElementById('Rdp_LedStart').value),
            LedSize: parseInt(document.getElementById('Rdp_LedSize').value),
            url: document.getElementById('Rdp_url').value}

      };
      console.log(updatedConfig);
      fetch('/save-config', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify(updatedConfig,null, "\t")
      })
      .then(response => {
        if (response.ok) {
          if ( confirm('Config saved! reboot to activate ?') == true )
              Reboot();
        } else {
          alert('Failed to save config.');
        }
      });
    }
    
    function  AskReboot()
    {
      if (confirm("Reboot the ESP32?")) {
      Reboot();
        
      }
    }
    
    function  Reboot(){
        fetch('/reboot', {
          method: 'GET',
        })
        .then(response => {
          if (response.ok) {
            location.reload();
          } else {
            alert('Failed to reboot.');
          }
        });
    }

    const myTimeout = setInterval(GetWiFissid, 5000);
    // clearInterval(myTimeout);
    async function GetWiFissid() {
      try {
      fetch('/wifi-config')
      .then(response =>{
          console.log('Response Status Code:', response.status);
          // Check if the response is successful (status code 200-299)
          if (response.ok) {
            return response.json(); // Parse the response as JSON
          } else {
            //return null;
            // throw new Error(`HTTP error! Status: ${response.status}`);
          }
      })
      .then(data => {
        //console.log(data);
        if(data !== undefined)
        {
          ssidlist = document.getElementById('ssidlist');
          var newlist=ssidlist.cloneNode(false); // Create a new empty datalist
          data.forEach(function(item){
            var option = document.createElement('option');
              option.value = item.ssid;
            newlist.appendChild(option);
          });
          if (ssidlist.innerHTML !== newlist.innerHTML)
            {
              ssidlist.innerHTML = newlist.innerHTML;
            }
          }
        return Promise.resolve();})
      .catch(error => {
        console.error('Error:', error.message);
        return Promise.reject();
        // Handle errors
      });
      }
      catch(error) {
        console.log('Catch Error:', error.message);
      }
    };
    document.addEventListener('keydown', e => {
        if (e.ctrlKey && e.key === 's') {
            // Prevent the Save dialog to open
            e.preventDefault();
            saveConfig();
            console.log('CTRL + S');
        }
    });

  </script>
</body>
</html>
)rawliteral";
