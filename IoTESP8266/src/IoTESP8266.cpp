/*
 * IoTESP8266.cpp
 *
 *  Created on: 17 апр. 2017 г.
 *      Author: isudarik
 */
#include "IoTESP8266.h"
#include "SoftwareSerial.h"

IoTESP8266::IoTESP8266(int rx = 10, int tx = 11) {
	espPort = new SoftwareSerial(rx, tx, false);
	serverPort = 8080;
}

IoTESP8266::~IoTESP8266() {
	clearSerialBuffer();
	espPort->end();
	delete espPort;
}

void IoTESP8266::setServerPort(int port) {
	serverPort = port;
}

void IoTESP8266::setServerUrl(String url) {
	serverBaseUrl = url;
}

/**
 * Executes given AT command and returns true if response contained "OK".
 */
boolean IoTESP8266::callAndGetResponseESP8266(String atCommandString) {
	espPort->println(atCommandString);
	delay(500);
	return readESPOutputWithResult();;
}

/**
 * Reads ESP output for 20 attempts and searches for OK string in response.
 * returns true if response contains OK, otherwise false.
 */
boolean IoTESP8266::readESPOutputWithResult() {
	String response = readESPOutput(true, 15);
	Serial.println(response);
	boolean ok = response.indexOf("OK") > -1;
	return ok;
}

/**
 * Reads esp output for specified number of times.
 * If waitForOk true than returns exactly when OK found.
 */
String IoTESP8266::readESPOutput(boolean waitForOk = true, int attempts = 15) {
	String espResponse;
	String temp;
	int waitTimes = 0;
	readResponse: delay(500);
	waitTimes++;
	while (espPort->available() > 0) {
		temp = espPort->readString();
		espResponse += temp;
		espResponse.trim();
	}

	if (waitForOk && espResponse.indexOf("OK") > -1) {
		// found OK -> assume finished
		return espResponse;
	}
	if (waitTimes <= attempts) {
		goto readResponse;
	}
	return espResponse;
}

boolean IoTESP8266::connectToWiFi(String networkSSID, String networkPASS) {
	String cmd = "AT+CWJAP=\"";
	cmd += networkSSID;
	cmd += "\",\"";
	cmd += networkPASS;
	cmd += "\"";

	espPort->println(cmd);
	String response = readESPOutput(true, 25);
	Serial.println(response);
	return response.indexOf("OK") > 0;
}

boolean IoTESP8266::sendHTTPRequestToServer(String method, String url,
		String jsonPayload) {
	return sendHTTPRequest(method, serverBaseUrl, url, jsonPayload, serverPort);
}

boolean IoTESP8266::performRealSend(String method, String url,
		String jsonPayload, String host, int port) {
	String header = method;
	header += " /";
	header += url;
	header += " HTTP/1.1\r\n";
	header += "Content-Type: application/json\r\n";
	header += "Content-Length: ";
	header += (int) (jsonPayload.length());
	header += "\r\n";
	header += "Host: ";
	header += host;
	header += ":";
	header += port;
	header += "\r\n";
	header += "Connection: close\r\n\r\n";
	cipSend(0, header, jsonPayload);
	sendMessageContents(header, jsonPayload);
	String result = readESPOutput(false, 15);
	return result.indexOf("SEND OK") > -1;
}

void IoTESP8266::cipSend(int ch_id, const String& header,
		const String& content) {
	espPort->print("AT+CIPSEND=");
	espPort->print(ch_id);
	espPort->print(",");
	espPort->println(header.length() + content.length());
}

/**
 * Sends HTTP (of specified method) request to given host:port and specified url.
 */
boolean IoTESP8266::sendHTTPRequest(String method, String host, String url,
		String jsonPayload, int port = 80) {
	// always start at 0 channel
	String atCommand = F("AT+CIPSTART=0,\"TCP\",\"");
	atCommand += host;
	atCommand += "\",";
	atCommand += port;
	boolean success = callAndGetResponseESP8266(atCommand);

	if (success) {
		return performRealSend(method, url, jsonPayload, host, port);
	} else {
		return false;
	}
}

/**
 * Sends HTTP header and content, if ESP turned in active mode.
 */
void IoTESP8266::sendMessageContents(String& header, String& content) {
	if (espPort->find(">")) {
		// wait for esp input
		espPort->print(header);
		espPort->print(content);
		delay(200);
	}
}

void IoTESP8266::start(String ssid, String pass, long rate, int timeout) {
	init: releaseResources();
	espPort->setTimeout(timeout);
	espPort->begin(rate); // ESP8266
	if (!callAndGetResponseESP8266("AT+RST")) {
		releaseResources();
		goto init;
	}
	if (!callAndGetResponseESP8266("AT+CWMODE=1")) {
		// client mode
		releaseResources();
		goto init;
	}
	if (!callAndGetResponseESP8266(F("AT+CWAUTOCONN=0"))) {
		// client mode
		releaseResources();
		goto init;
	}
	if (!connectToWiFi(ssid, pass)) {
		releaseResources();
		goto init;
	}
	if (!callAndGetResponseESP8266(F("AT+CIPMODE=0"))) {
		// normal transfer mode
		releaseResources();
		goto init;
	}
	if (!callAndGetResponseESP8266(F("AT+CIPMUX=1"))) {
		// allow multiple connection.
		releaseResources();
		goto init;
	};
	if (!callAndGetResponseESP8266(F("AT+CIPSERVER=1,88"))) {
		// set up server on 88 port
		releaseResources();
		goto init;
	}
	callAndGetResponseESP8266(F("AT+CIPSTO=2")); // server timeout, 2 sec
}

void IoTESP8266::releaseResources() {
	espPort->end();
}

void IoTESP8266::clearSerialBuffer() {
	while (espPort->available() > 0) {
		espPort->read();
	}
}

String IoTESP8266::getCurrentAssignedIP() {
	espPort->println(F("AT+CIFSR")); // read IP configuration
	String ipResponse = readESPOutput(true, 20);
	int ipStartIndex = ipResponse.indexOf("STAIP,\"");
	if (ipStartIndex > -1) {
		ipResponse = ipResponse.substring(ipStartIndex + 7,
				ipResponse.indexOf("\"\r"));
	} else {
		ipResponse = F("0.0.0.0");
	}
	return ipResponse;
}

void IoTESP8266::sendResponse(int ch_id, String status) {
	String header;

	header = "HTTP/1.1 200 OK\r\n";
	header += "Content-Type: application/json\r\n";
	header += "Connection: close\r\n";

	String content;

	content = "{\"status\": \"";
	content += status;
	content += "\"}";

	header += "Content-Length: ";
	header += (int) (content.length());
	header += "\r\n\r\n";

	cipSend(ch_id, header, content);
	delay(20);
	sendMessageContents(header, content);
	delay(200);
	espPort->print("AT+CIPCLOSE=");
	espPort->println(ch_id);
	readESPOutput(true, 2);
}

SoftwareSerial* IoTESP8266::getSerial() {
	return espPort;
}

