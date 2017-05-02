//
//  mqtt_suback.cpp
//  Shan.MqttBroker
//
//  Created by Sung Han Park on 2017. 4. 24..
//  Copyright © 2017 Sung Han Park. All rights reserved.
//

#include "mqtt_suback.h"

mqtt_suback::mqtt_suback(const fixed_header& fh, shan::util::streambuf_ptr sb_ptr)
: fixed_header(fh) {
	_packet_id = sb_ptr->read_uint16();

	uint8_t code;
	int count = _remaining_length - 2;
	for (int inx = 0 ; inx < count ; inx++) {
		code = sb_ptr->read_uint8();
		if ((code >= 3) && (code != 0x80))
			throw mqtt_error("malformed packet (SUBACK - invalid return code)");
		_return_codes.push_back(code);
	}
}

mqtt_suback::mqtt_suback(uint16_t packet_id)
: fixed_header(SUBACK, 0, 2), _packet_id(packet_id) {
}

void mqtt_suback::serialize(shan::util::streambuf_ptr sb_ptr) const {
	fixed_header::serialize(sb_ptr);

	// packet id
	sb_ptr->write_uint16(_packet_id);

	// return codes
	sb_ptr->write(&_return_codes[0], _return_codes.size());
}

void mqtt_suback::add_return_code(uint8_t return_code) {
	if ((return_code >= 0x03) && (return_code != 0x80))
		throw mqtt_error("invalid result code. (SUBACK)");

	_remaining_length++;
	_return_codes.push_back(return_code);
}

std::ostream& mqtt_suback::str(std::ostream& os) const {
	fixed_header::str(os);

	os << " PID:" << _packet_id
	   << " R_Num:" << _return_codes.size();

	return os;
}
