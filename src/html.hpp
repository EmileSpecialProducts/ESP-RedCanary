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
  
  <h2>ESP Red Canary Settings
    <button type="button" onclick="saveConfig()">Save</button>
    <button type="button" onclick="AskReboot()">Reboot</button>
    <a href="editor.html" target="_blank"><img width="25" height="25" src="https://raw.githubusercontent.com/EmileSpecialProducts/EmileSpecialProducts.github.io/refs/heads/main/ESP-RedCanary/editor.png" onerror="this.style.visibility = 'hidden'"></a>
 
  </h2>
  
   <section>
    <h1>Host</h1> 
    <table>
    <tr>
            <td><label for="ServerName">ServerName:</label></td>
            <td><input type="text" id="ServerName" name="ServerName"></td> 
            <td>This is the name that will be used for the ESP32 when it is connected to the network</td>
    </tr>
    <tr>
            <td><label for="ServerPort">Setup port number:</label></td>
            <td><input type="number" id="ServerPort" name="ServerPort"></td>
            <td>This is the port number that will be used for the Canary Setup Page As the Port 80 will be used for the WebServer trigger</td>
    </tr>
    <tr>
            <td><label for="ServerMAC">MAC:</label></td>
            <td><input type="text" id="ServerMAC" name="ServerMAC"></td> 
            <td><a href="https://udger.com/resources/mac-address-vendor" target="_blank"> Mac List </a></td>
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
    <td><input type="checkbox" id="Http_enable" name="Http_enable" onclick="TestHttp()"></td> 
    <td></td>
    </tr>
    <tr>
    <td><label for="Http_url">Http_url:</label></td>
    <td><input type="text" id="Http_url" name="Http_url"></td>
    <td>This is the URL that will be triggered when the Canary is activated</td>
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
    </table>
    </section>
    <section>
    <h1>Modbus (502) </h1>
    <table id="Modbus_table">
    <tr>
    <td><label for="Modbus_enable">Modbus:</label></td>
    <td><input type="checkbox" id="Modbus_enable" name="Modbus_enable" onclick="UpdateHide()"></td>
    <td></td>
    </tr>
      <tr>
      <td><label for="Modbus_url">Modbus_url:</label></td>
      <td><input type="text" id="Modbus_url" name="Modbus_url"></td>
      <td>This is the URL that will be triggered when the Canary is activated</td>
      </tr>
    </table>
    </section>
    <section>
    <h1 title="IP addresses That will not Log or trigger an Alert&#013;You can also specify a subnet mask by using CIDR notation (e.g., 192.168.1.0/24)">Whitelisting</h1>
    <table id="Whitelist_table">
    <tr>
    <th>IP address</th>
    <th>Remove</th>
    </tr>
    <tbody id="Whitelist_rows"></tbody>
    </table>
    <button type="button" onclick="addWhitelistRow()">+</button>
    </section>
    <section>
    <h1>Save and Reboot</h1>
    <button type="button" onclick="saveConfig()">Save</button>
    <button type="button" onclick="AskReboot()">Reboot</button>
    <a href="editor.html" target="_blank"><img width="50" height="50"  src="https://raw.githubusercontent.com/EmileSpecialProducts/EmileSpecialProducts.github.io/refs/heads/main/ESP-RedCanary/editor.png" onerror="this.style.visibility = 'hidden'"></a>
    </section>

  <script>
    function tablehide( table, show = false)
    {
      for (var i = 1, row; row = table.rows[i]; i++ ) {
           row.style.display = show ? '':'none';
        }
    }
    
    function TestHttp()
    {
      console.log( "ServerPort " + document.getElementById("ServerPort").value );
      
      if(document.getElementById("ServerPort").value == 80 )
      {
        document.getElementById("Http_enable").checked = false;
        alert('First change the Setup port'); 
      } 
      else
      UpdateHide();
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
      tablehide(document.getElementById("Modbus_table") ,document.getElementById('Modbus_enable').checked);
    }
    
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

    function addWhitelistRow(value="")
    {
      const row = document.createElement('tr');
      const addressCell = document.createElement('td');
      const addressInput = document.createElement('input');
      addressInput.type = 'text';
      addressInput.className = 'whitelist-ip';
      addressInput.placeholder = '192.168.1.0/24';
      addressInput.value = value;
      addressCell.appendChild(addressInput);

      const actionCell = document.createElement('td');
      const removeButton = document.createElement('button');
      removeButton.type = 'button';
      removeButton.textContent = '-';
      removeButton.title = 'Remove IP address';
      removeButton.onclick = () => row.remove();
      actionCell.appendChild(removeButton);

      row.appendChild(addressCell);
      row.appendChild(actionCell);
      document.getElementById('Whitelist_rows').appendChild(row);
    }

    function getWhitelistIps()
    {
      return Array.from(document.querySelectorAll('.whitelist-ip'))
        .map(input => input.value.trim())
        .filter(value => value.length > 0);
    }

    // Load JSON from ESP32
    fetch('/config.json')
      .then(response => {
        if (!response.ok) {
          console.warn('config.json is not available; using default settings');
          return {};
        }
        return response.json();
      })
      .catch(error => {
        console.warn('Could not load config.json; using default settings:', error);
        return {};
      })
      .then(data => {        
        data = data || {};
        setElementId('ServerMAC',data.ServerMAC, "xx:xx:xx:xx:xx:xx");
        if(typeof data.ServerName !== 'string' || data.ServerName.length==0) data.ServerName=undefined;
        setElementId('ServerName',data.ServerName,"ESPRedCanary")
        setElementId('ServerPort',data.ServerPort,80);
        if( ! data.Http )  data.Http={};
        setElementId('Http_enable',data.Http.enable);
        setElementId('Http_url',data.Http.url);
            if( ! data.Https )  data.Https={};
        setElementId('Https_enable',data.Https.enable);
        setElementId('Https_url',data.Https.url);
            if( ! data.Httpa )  data.Httpa={};
        setElementId('Httpa_enable',data.Httpa.enable);
        setElementId('Httpa_url',data.Httpa.url); 
        if( ! data.SSH )  data.SSH={};
        setElementId('SSH_enable',data.SSH.enable);
        setElementId('SSH_url',data.SSH.url);
        if( ! data.Telnet )  data.Telnet={}; 
        setElementId('Telnet_enable', data.Telnet.enable,false);
        setElementId('Telnet_url', data.Telnet.url);
        if( ! data.Ftp )  data.Ftp={}; 
        setElementId('Ftp_enable', data.Ftp.enable,false);
        setElementId('Ftp_url', data.Ftp.url);
        if( ! data.Rdp )  data.Rdp={}; 
        setElementId('Rdp_enable', data.Rdp.enable,false);
        setElementId('Rdp_url', data.Rdp.url);
        if( ! data.Modbus )  data.Modbus={};
        setElementId('Modbus_enable', data.Modbus.enable,false);
        setElementId('Modbus_url', data.Modbus.url);
        const whitelistIps = data.Whitelisting && Array.isArray(data.Whitelisting.ips)
          ? data.Whitelisting.ips : [];
        whitelistIps.forEach(ip => addWhitelistRow(ip));
        UpdateHide();
      })
      .catch(error => {
        console.error('Error loading config:', error);
      });

    // Send updated JSON back to ESP32
    function saveConfig() {
      
      const updatedConfig = {
        ServerMAC: document.getElementById('ServerMAC').value,
        ServerName: document.getElementById('ServerName').value,
        ServerPort: parseInt(document.getElementById('ServerPort').value),

        Http:{enable: document.getElementById('Http_enable').checked ,
              url: document.getElementById('Http_url').value
            },
        Https:{enable: document.getElementById('Https_enable').checked ,
              url: document.getElementById('Https_url').value},
        Httpa:{enable: document.getElementById('Httpa_enable').checked ,
              url: document.getElementById('Httpa_url').value},
              
        SSH:{enable: document.getElementById('SSH_enable').checked ,
            url: document.getElementById('SSH_url').value},
        Telnet:{enable: document.getElementById('Telnet_enable').checked ,
            url: document.getElementById('Telnet_url').value},
            
        Ftp:{enable: document.getElementById('Ftp_enable').checked ,
            url: document.getElementById('Ftp_url').value},

        Rdp:{enable: document.getElementById('Rdp_enable').checked ,
          url: document.getElementById('Rdp_url').value},

        Modbus:{enable: document.getElementById('Modbus_enable').checked ,
          url: document.getElementById('Modbus_url').value},

        Whitelisting:{ips: getWhitelistIps()}

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
