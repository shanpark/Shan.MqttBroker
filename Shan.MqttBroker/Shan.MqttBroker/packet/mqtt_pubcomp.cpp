//
//  mqtt_pubcomp.cpp
//  Shan.MqttBroker
//
//  Created by Sung Han Park on 2017. 4. 24..
//  Copyright © 2017 Sung Han Park. All rights reserved.
//

#include "mqtt_pubcomp.h"

mqtt_pubcomp::mqtt_pubcomp(const fixed_header& fh, shan::util::streambuf_ptr sb_ptr)
: fixed_header(fh) {
	_packet_id = sb_ptr->read_uint16();
}

mqtt_pubcomp::mqtt_pubcomp(uint16_t packet_id)
: fixed_header(PUBCOMP, 0, 2), _packet_id(packet_id) {
}

void mqtt_pubcomp::serialize(shan::util::streambuf_ptr sb_ptr) {
	fixed_header::serialize(sb_ptr);

	sb_ptr->write_uint16(_packet_id);
}
