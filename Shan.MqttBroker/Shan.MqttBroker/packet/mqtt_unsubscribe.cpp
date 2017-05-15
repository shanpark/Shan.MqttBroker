//
//  mqtt_unsubscribe.cpp
//  Shan.MqttBroker
//
//  Created by Sung Han Park on 2017. 4. 24..
//  Copyright © 2017 Sung Han Park. All rights reserved.
//

#include "mqtt_unsubscribe.h"
#include "mqtt_subscribe.h"

mqtt_unsubscribe::mqtt_unsubscribe(const fixed_header& fh, shan::util::streambuf_ptr sb_ptr)
: fixed_header(fh) {
	int32_t remain = static_cast<int32_t>(_remaining_length);

	////////////////////////////////////////////////////////////////////////////
	// variable header

	// packet id
	_packet_id = sb_ptr->read_uint16();
	remain -= 2;

	////////////////////////////////////////////////////////////////////////////
	// payload

	while (remain > 0) {
		std::string topic = decode_string(sb_ptr);

		if (topic.empty())
			throw mqtt_error("malformed packet received. (empty topic filter)");

		remain -= static_cast<int32_t>(topic.length() + 2);

		bool is_wild_topic = false;
		mqtt_subscribe::is_topic_filter_valid(topic, is_wild_topic);

		_topic_filters.emplace_back(topic);
		_is_wild.push_back(is_wild_topic);
	}

	if (_topic_filters.empty())
		throw mqtt_error("A UNSUBSCRIBE with no topic filter was received.");
}

mqtt_unsubscribe::mqtt_unsubscribe(uint16_t packet_id)
: fixed_header(UNSUBSCRIBE, 2, 2), _packet_id(packet_id) {
}

void mqtt_unsubscribe::serialize(shan::util::streambuf_ptr sb_ptr) const {
	throw mqtt_error("broker does not send a UNSUBSCRIBE packet.");
}

void mqtt_unsubscribe::add_topic_filter(std::string topic_filter) {
	_remaining_length += static_cast<uint32_t>(2 + topic_filter.length());
	_topic_filters.push_back(topic_filter);
}

std::ostream& mqtt_unsubscribe::str(std::ostream& os) const {
	fixed_header::str(os);

	os << " PID:" << _packet_id
	   << " T_Num:" << _topic_filters.size();

	return os;
}
