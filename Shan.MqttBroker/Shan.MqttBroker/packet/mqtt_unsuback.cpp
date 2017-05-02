//
//  mqtt_unsuback.cpp
//  Shan.MqttBroker
//
//  Created by Sung Han Park on 2017. 4. 24..
//  Copyright © 2017 Sung Han Park. All rights reserved.
//

#include "mqtt_unsuback.h"

mqtt_unsuback::mqtt_unsuback(const fixed_header& fh, shan::util::streambuf_ptr sb_ptr)
: fixed_header(fh) {
	_packet_id = sb_ptr->read_uint16();
}

mqtt_unsuback::mqtt_unsuback(uint16_t packet_id)
: fixed_header(UNSUBACK, 0, 2), _packet_id(packet_id) {
}

void mqtt_unsuback::serialize(shan::util::streambuf_ptr sb_ptr) const {
	fixed_header::serialize(sb_ptr);

	// packet id
	sb_ptr->write_uint16(_packet_id);
}

std::ostream& mqtt_unsuback::str(std::ostream& os) const {
	fixed_header::str(os);

	os << " PID:" << _packet_id;

	return os;
}
