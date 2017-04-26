//
//  mqtt_disconnect.cpp
//  Shan.MqttBroker
//
//  Created by Sung Han Park on 2017. 4. 24..
//  Copyright © 2017년 Sung Han Park. All rights reserved.
//

#include "mqtt_disconnect.h"

mqtt_disconnect::mqtt_disconnect(const fixed_header& fh, shan::util::streambuf_ptr sb_ptr)
: fixed_header(fh) {}

mqtt_disconnect::mqtt_disconnect()
: fixed_header(DISCONNECT, 0, 0) {
}

void mqtt_disconnect::serialize(shan::util::streambuf_ptr sb_ptr) {
	fixed_header::serialize(sb_ptr);
}
