<p align="center">
  <a href="https://buymeacoffee.com/emilespecialproducts">
    <img alt="BuymeaCoffee" src="https://www.buymeacoffee.com/assets/img/custom_images/orange_img.png">
  </a>
</p>
<p align="center">
<img alt="WiFi Duck Logo" src="web/Under-Construction.png" width="256">
</p>

# ESP-RedCanary
It still will need some work, so it is "Under-Construction" (Q2-2026) 

This is the Honeypot called ESP-RedCanary for esp-arduino ESP32/S2/S3/C3/C5/C6 

Not yet supported are the ESP32-C2/C61/S31.

You can best use the ESP32-S3 16MB CAM version, that has a SDCard slot for example [ESP32-S3 N16R8 CAM](https://nl.aliexpress.com/item/1005008589091526.html) this is under 8$, But is you are on a budget use the [ESP32C3](https://nl.aliexpress.com/item/1005007446721319.html) thats under 2$ and still has 2 floppy drive storage for logs. 

It will response to HTTP, MCP,RDP, SSH, Telnet, ModBus request and alert a SIEM system or a canarytokens.
This then can trigger a support ticket.

# Programming
To test this software, you can use the [Web Programming portal](https://emilespecialproducts.github.io/ESP-RedCanary/upload.html) to Program your ESP-Chip, no ESP development software needed.  

After programming you will need to setup the Wi-Fi connection this is done by connection to the ESP Access Point (ESP-RedCanary) And set up your Wi-Fi
And then connect to the [http://espredcanary.local](http://espredcanary.local) or to the IP address.
You will probably need an extra reboot to see the ESP-RedCanary setup page.

# CanaryTokens 
After that you will need to set up the Web Bug CanaryTokens at [https://canarytokens.org/](https://canarytokens.org/)
You need to past the "Canarytoken URL" into the setup page.
You can add separate tokens for the different protocols, or just one main token.

If the [http://espredcanary.local](http://espredcanary.local) does not work, use the IP address.
You can find the IP by using a serial monitor You can find a Serial monitor for example [serialterminal.com](https://www.serialterminal.com) or [espconnect](https://espconnect.msnow.icu/)
This is based on Arduino Release v3.3.11 from pioarduino [(p)eople (i)nitiated (o)ptimized (arduino)](https://github.com/pioarduino/platform-espressif32) 

You can find a link the to explorer [Youtube](https://youtu.be/SGOuzqJMnVM) for the SPIFFS version

# ToDo:

- Add a jdbc/ODBC/MySQL database client
- Add the MAC settings
- Do more Testing
- Add automatic testing
- Create an instruction YouTube video
- Update the documentation 
- I am alive event    
- Add the ESP32-C2  ( its added but platform not working yet)
- Add the ESP32-C61 ( its added but platform not working yet)

