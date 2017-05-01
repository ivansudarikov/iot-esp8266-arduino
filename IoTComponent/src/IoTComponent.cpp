/*
 * IoTComponent.cpp
 *
 *  Created on: 30 апр. 2017 г.
 *      Author: isudarik
 */

#include "IoTComponent.h"
#include "IoTESP8266.h"

IoTComponent::IoTComponent(IoTESP8266* esp) {
	esp8266 = esp;
}

IoTComponent::~IoTComponent() {
	esp8266->clearSerialBuffer();
	clearBuffer();
	delete esp8266;
}

void IoTComponent::processIncomingRequest() {
	int ch_id, packet_len;
	char* pb;
	Serial.println(buffer);
	sscanf(buffer + 5, "%d,%d", &ch_id, &packet_len);
	if (packet_len > 0) {
		pb = buffer + 5;
		while (*pb != ':') {
			pb++;
		}
		pb++;
		processData(pb, ch_id);
	}
}

void IoTComponent::doWork() {
	setupAndStart();
	SoftwareSerial* espSerial = esp8266->getSerial();
	while (true) {
		espSerial->readBytesUntil('\n', buffer, BUFFER_SIZE);
		Serial.print(buffer);
		if (strncmp(buffer, "+IPD,", 5) == 0) {
			processIncomingRequest();
		}
		clearBuffer();
	}
}

void IoTComponent::clearBuffer(void) {
	for (int i = 0; i < BUFFER_SIZE; i++) {
		buffer[i] = 0;
	}
}

