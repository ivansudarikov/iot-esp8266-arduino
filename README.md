# iot-eps8266-arduino
This is kind of "driver" for IoT projects where it is required to combine ESP8266 and Arduino.
AT-comands are used for controlling ESP8266 device.

The idea is that there is IoT component that sends its data to server (in this case in json format).

To start using esp8266 you can do:

```C++
  IoTESP8266* esp8266 = new IoTESP8266(10, 11); // Create new connection to esp8266. Arduino (10, 11) -> esp (RX, TX)
	esp8266->setServerUrl("192.168.1.1"); // ip address of you "central IoT server".
  esp8266->setServerPort(8080);
  /*
   * wi-fi to connect SSID, password, baud rate (between esp and arduino), timeout for SoftwareSerial
   */
	esp8266->start("YourWiFiSSID", "Password", 9600, 1000); 
  /*
   * Sends HTTP POST to 192.168.1.1:8080/status with {"STATUS" : "IOT_INITIATED"} json.
   */
	esp8266->sendHTTPRequestToServer("POST", "/status", "{/"STATUS/" : /"IOT_INITIATED"/}");
```

Of course you can request any URL you want by correposnding method (see implementation).
