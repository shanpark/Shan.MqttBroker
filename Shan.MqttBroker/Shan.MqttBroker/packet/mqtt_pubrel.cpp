//
//  mqtt_pubrel.cpp
//  Shan.MqttBroker
//
//  Created by Sung Han Park on 2017. 4. 24..
//  Copyright © 2017 Sung Han Park. All rights reserved.
//

#include "mqtt_pubrel.h"

mqtt_pubrel::mqtt_pubrel(const fixed_header& fh, shan::util::streambuf_ptr sb_ptr)
: fixed_header(fh) {
	_packet_id = sb_ptr->read_uint16();
}

mqtt_pubrel::mqtt_pubrel(uint16_t packet_id)
: fixed_header(PUBREL, 2, 2), _packet_id(packet_id) {
}

void mqtt_pubrel::serialize(shan::util::streambuf_ptr sb_ptr) {
	fixed_header::serialize(sb_ptr);

	sb_ptr->write_uint16(_packet_id);
}
