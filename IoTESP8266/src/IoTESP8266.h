/*
 * IotESP8266.h
 *
 *  Created on: 17 апр. 2017 г.
 *      Author: isudarik
 */
#ifndef IOTESP8266_H
#define IOTESP8266_H

#include <Arduino.h>
#include <SoftwareSerial.h>
#include <Stream.h>

class IoTESP8266 {
public:
	IoTESP8266();
	IoTESP8266(int rx, int tx);
	~IoTESP8266();
	void setServerUrl(String serverUrl);
	void setServerPort(int port);
	void start(String ssid, String pass, long rate = 9600, int timeout = 750);
	/**
	 * Sends HTTP (of specified method) request to given host:port and specified url.
	 * In other words starts TCP connection to host:port and then sends
	 */
	boolean sendHTTPRequest(String method, String host, String url, String jsonPayload, int port = 80);
	boolean sendHTTPRequestToServer(String method, String url,
			String jsonPayload);
	String sendATCommand(String atCommand);
	String getCurrentAssignedIP();
	SoftwareSerial* getSerial();
	void clearSerialBuffer();
	void sendResponse(int ch_id, String status);
private:
	boolean callAndGetResponseESP8266(String atCommandString);
	boolean readESPOutputWithResult();
	String readESPOutput(boolean waitForOk, int attempts);
	boolean connectToWiFi(String networkSSID, String networkPASS);
	void cipSend(int ch_id, const String& header, const String& content);
	void sendMessageContents(String& header, String& content);
	void releaseResources();
	boolean performRealSend(String method, String url, String jsonPayload,
			String host, int port);
	SoftwareSerial* espPort;
	String serverBaseUrl;
	int serverPort;
};

#endif /* LIBRARIES_IOTESP8266_SRC_IOTESP8266_H_ */
