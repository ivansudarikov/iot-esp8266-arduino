/*
 * IoTComponent.h
 *
 *  Created on: 30 апр. 2017 г.
 *      Author: isudarik
 */

#ifndef LIBRARIES_IOTCOMPONENT_H_
#define LIBRARIES_IOTCOMPONENT_H_
#include "IoTESP8266.h"
#include "Arduino.h"

#define BUFFER_SIZE 128

class IoTComponent {
public:
	IoTComponent(IoTESP8266* esp8266);
	virtual ~IoTComponent();
	virtual void processData(char* pb, int ch_id);
	virtual void setupAndStart();
	virtual String getRegisterJsonPayload(String ipAddress);
	void processIncomingRequest();
	void doWork();
	IoTESP8266* getESPPort() {
		return esp8266;
	}
private:
	void clearBuffer();
	IoTESP8266* esp8266;
	char buffer[BUFFER_SIZE];
};

#endif /* LIBRARIES_IOTCOMPONENT_H_ */
