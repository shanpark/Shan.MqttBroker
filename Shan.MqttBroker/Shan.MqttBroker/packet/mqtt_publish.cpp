//
//  mqtt_publish.cpp
//  Shan.MqttBroker
//
//  Created by Sung Han Park on 2017. 4. 23..
//  Copyright © 2017 Sung Han Park. All rights reserved.
//

#include "mqtt_publish.h"

mqtt_publish::mqtt_publish(const fixed_header& fh, shan::util::streambuf_ptr sb_ptr)
: fixed_header(fh) {
	// flags
	_dup = ((_flags & MASK_DUP) != 0);
	_qos = ((_flags & MASK_QOS) >> 1);
	_retain = ((_flags & MASK_RETAIN) != 0);
	if ((_qos == 0) && _dup)
		throw mqtt_error("malformed packet received. (publish packet with qos = 0 and dup = 1)");
	if (_qos == 3)
		throw mqtt_error("malformed packet received. (publish packet with qos = 3)");

	std::size_t variable_header_len = 0;

	// topic name
	_topic_name = decode_string(sb_ptr);
	if (_topic_name[0] == '$')
		throw mqtt_error("malformed packet received. (clients can not publish to topics starting with $)");

	for (auto ch : _topic_name) {
		if ((ch == '#') || (ch == '+')) // is_wildcard(ch))
			throw mqtt_error("malformed packet received. (the topic of the PUBLISH has a wildcard.)");
	}
	variable_header_len += (_topic_name.length() + 2);

	// packet id
	if ((_qos == 1) || (_qos == 2)) {
		_packet_id = sb_ptr->read_uint16();
		variable_header_len += 2;
	}

	// payload
	std::size_t payload_len = _remaining_length - variable_header_len;
	if (sb_ptr->in_size() < payload_len)
		throw mqtt_error("malformed packet received. (not enough data - PUBLISH payload.)");
	_payload.resize(payload_len);
	sb_ptr->read(&_payload[0], payload_len);
}

mqtt_publish::mqtt_publish(bool dup, uint8_t qos, bool retain, std::string topic_name, uint16_t packet_id, std::vector<uint8_t>&& payload)
: fixed_header(PUBLISH, 0, 0) {
	// flags
	uint8_t flags = 0;
	if (dup)
		flags = 0x08;

	if (qos == 3)
		throw mqtt_error("try to cretae malformed packet. (PUBLISH QoS is 3)");
	flags |= (qos << 1);

	if (retain)
		flags |= 0x01;

	_flags |= flags;

	// topic name
	if (_topic_name.length() > 0xffff)
		throw mqtt_error("try to cretae malformed packet. (PUBLISH topic name is too long)");
	_topic_name = topic_name;
	_remaining_length = static_cast<uint32_t>(_topic_name.length()) + 2;

	if ((_qos == 1) || (_qos == 2)) {
		_packet_id = packet_id;
		_remaining_length += 2;
	}

	if (payload.size() + _remaining_length > 0x0fffffff)
		throw mqtt_error("try to cretae malformed packet. (PUBLISH payload is too long)");
	_payload = std::move(payload);
	_remaining_length += _payload.size();
}

void mqtt_publish::serialize(shan::util::streambuf_ptr sb_ptr) {
	fixed_header::serialize(sb_ptr);

	encode_string(_topic_name, sb_ptr);

	if ((_qos == 1) || (_qos == 2))
		sb_ptr->write_uint16(_packet_id);


	sb_ptr->write(&_payload[0], _payload.size());
}
