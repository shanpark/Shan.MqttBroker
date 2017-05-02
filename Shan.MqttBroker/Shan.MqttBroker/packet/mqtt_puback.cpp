//
//  mqtt_puback.cpp
//  Shan.MqttBroker
//
//  Created by Sung Han Park on 2017. 4. 23..
//  Copyright © 2017 Sung Han Park. All rights reserved.
//

#include "mqtt_puback.h"

mqtt_puback::mqtt_puback(const fixed_header& fh, shan::util::streambuf_ptr sb_ptr)
: fixed_header(fh) {
	_packet_id = sb_ptr->read_uint16();
}

mqtt_puback::mqtt_puback(uint16_t packet_id)
: fixed_header(PUBACK, 0, 2), _packet_id(packet_id) {
}

void mqtt_puback::serialize(shan::util::streambuf_ptr sb_ptr) const {
	fixed_header::serialize(sb_ptr);

	sb_ptr->write_uint16(_packet_id);
}

std::ostream& mqtt_puback::str(std::ostream& os) const {
	fixed_header::str(os);

	os << " PID:" << _packet_id;

	return os;
}
