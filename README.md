<p align="center">
  <a href="https://buymeacoffee.com/emilespecialproducts">
    <img alt="BuymeaCoffee" src="https://www.buymeacoffee.com/assets/img/custom_images/orange_img.png">
  </a>
</p>
<p align="center">
<img alt="WiFi Duck Logo" src="web/Under-Construction.png" width="256">
</p>


# ESP-RedCanary

This is the ESP-RedCanary example for esp-arduino ESP32/S2/S3/C3/C5/C6 Not yet supported ESP32-C2/C61/P4.
You can best use the ESP32-S3 16MB CAM version, that has SDCard slot ( or the 32MB if you can get it)  
And only the ESP32S3 8MB is used for development at the moment that has OTA.

It still will need a lot of work so it is "Under-Construction" (Q2-2026) 

This is based on Arduino Release v3.3.11 from pioarduino [(p)eople (i)nitiated (o)ptimized (arduino)](https://github.com/pioarduino/platform-espressif32) 

[//]: # To test this software you can use the [Web Programming portal](https://emilespecialproducts.github.io/ESP-RedCanary/upload.html) to Program your ESP-Chip, No development software needed.  

After programming you will need to setup the Wifi connection this is done by connection to the ESP Access Point (ESP-RedCanary)
And set up your Wifi 
After that you will need to set up the Web Bug canarytokens at [https://canarytokens.org/](https://canarytokens.org/)
You need to past the "Canarytoken URL" in to the setup page.
You can add separate tokens for the diffrent protocols
And then connect to the http://espredcanary.local/index.html or to the IP addres.
You can find the IP by using a serial monitor You can find a Serial monitor for example (serialterminal.com)[https://www.serialterminal.com]



You can find a link the to [Youtube](https://youtu.be/SGOuzqJMnVM) for the SPIFFS version

ToDo:
- Max log file size
- Max number of log files
- Add the ESP32-C2
- Add the ESP32-C61
- Add MMC SDCard support for ESP32-S3 CAM 16MB  
- Add SPI SDCard Support
- Add a jdbc/ODBC database client
- Add the MAC settings
- Do more Testing
- Create an instruction youtube video
- Update the documentation    


