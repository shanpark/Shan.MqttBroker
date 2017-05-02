//
//  constants.h
//  Shan.MqttBroker
//
//  Created by Sung Han Park on 2017. 4. 22..
//  Copyright © 2017 Sung Han Park. All rights reserved.
//

#ifndef constants_h
#define constants_h

#include <cstdint>
#include "exception.h"

#define DEFAULT_CONFIG_FILE	"mqttd.cfg"

enum packet_type : uint8_t {
	CONNECT     = 1,
	CONNACK     = 2,
	PUBLISH     = 3,
	PUBACK      = 4,
	PUBREC      = 5,
	PUBREL      = 6,
	PUBCOMP     = 7,
	SUBSCRIBE   = 8,
	SUBACK      = 9,
	UNSUBSCRIBE = 10,
	UNSUBACK    = 11,
	PINGREQ     = 12,
	PINGRESP    = 13,
	DISCONNECT  = 14
};

enum flag_mask : uint8_t {
	MASK_DUP    = 0x08,
	MASK_QOS    = 0x06,
	MASK_RETAIN = 0x01,

	MASK_SESSION_PRESENT = 0x01
};

enum qos : uint8_t {
	QOS_AT_MOST_ONCE = 0,
	QOS_AT_LEAST_ONCE = 1,
	QOS_EXACTLY_ONCE = 2,
	QOS_INVALID = 3
};

enum timer_id {
	ID_NO_CONNECT = 1,
	ID_CLIENT_IDLE,
	ID_CLIENT_DISCONNECTED,
};

class config;
extern config g_cfg;

#define LOGGER()	(spdlog::get("logger"))

#endif /* constants_h */
