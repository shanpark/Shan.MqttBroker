//
//  mqtt_connack.cpp
//  Shan.MqttBroker
//
//  Created by Sung Han Park on 2017. 4. 22..
//  Copyright © 2017 Sung Han Park. All rights reserved.
//

#include "mqtt_connack.h"

mqtt_connack::mqtt_connack(const fixed_header& fh, shan::util::streambuf_ptr sb_ptr)
: fixed_header(fh) {
	uint8_t byte = sb_ptr->read_uint8();
	if (byte > 1)
		throw mqtt_error("malformed packet. (CONNACK)");
	_session_present = ((byte & MASK_SESSION_PRESENT) != 0);

	_connect_return_code = sb_ptr->read_uint8();
	if (_connect_return_code >= 6)
		throw mqtt_error("malformed packet. (CONNACK - unknown connect return code)");
}

mqtt_connack::mqtt_connack(bool session_present, uint8_t connect_return_code)
: fixed_header(CONNACK, 0, 2), _session_present(session_present), _connect_return_code(connect_return_code) {
}

void mqtt_connack::serialize(shan::util::streambuf_ptr sb_ptr) const {
	fixed_header::serialize(sb_ptr);

	if (_session_present)
		sb_ptr->write_uint8(0x01);
	else
		sb_ptr->write_uint8(0x00);

	sb_ptr->write_uint8(_connect_return_code);
}

std::ostream& mqtt_connack::str(std::ostream& os) const {
	fixed_header::str(os);

	os << ((_session_present) ? " SP:1" : " SP:0")
	   << " RC:" << static_cast<int>(_connect_return_code);

	return os;
}
