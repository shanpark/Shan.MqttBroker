//
//  mqtt_pubrec.cpp
//  Shan.MqttBroker
//
//  Created by Sung Han Park on 2017. 4. 23..
//  Copyright © 2017 Sung Han Park. All rights reserved.
//

#include "mqtt_pubrec.h"

mqtt_pubrec::mqtt_pubrec(const fixed_header& fh, shan::util::streambuf_ptr sb_ptr)
: fixed_header(fh) {
	_packet_id = sb_ptr->read_uint16();
}

mqtt_pubrec::mqtt_pubrec(uint16_t packet_id)
: fixed_header(PUBREC, 0, 2), _packet_id(packet_id) {
}

void mqtt_pubrec::serialize(shan::util::streambuf_ptr sb_ptr) const {
	fixed_header::serialize(sb_ptr);

	sb_ptr->write_uint16(_packet_id);
}

std::ostream& mqtt_pubrec::str(std::ostream& os) const {
	fixed_header::str(os);

	os << " PID:" << _packet_id;

	return os;
}
