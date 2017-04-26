//
//  mqtt_subscribe.cpp
//  Shan.MqttBroker
//
//  Created by Sung Han Park on 2017. 4. 24..
//  Copyright © 2017 Sung Han Park. All rights reserved.
//

#include "mqtt_subscribe.h"

mqtt_subscribe::mqtt_subscribe(const fixed_header& fh, shan::util::streambuf_ptr sb_ptr)
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
		remain -= (topic.length() + 2);

		bool is_wild_topic = false;
		if (!is_topic_filter_valid(topic, is_wild_topic))
			throw mqtt_error("A SUBSCRIBE with invalid topic filter.");

		uint8_t max_qos = sb_ptr->read_uint8();
		if (max_qos >= 3)
			throw mqtt_error("malformed packet. (SUBSCRIBE topic filter QoS)");
		remain--;

		_topic_filters.emplace_back(topic);
		_max_qoses.push_back(max_qos);
		_is_wild.push_back(is_wild_topic);
	}

	if (_topic_filters.empty())
		throw mqtt_error("A SUBSCRIBE with no topic filter was received.");
}

mqtt_subscribe::mqtt_subscribe(uint16_t packet_id)
: fixed_header(SUBSCRIBE, 2, 2), _packet_id(packet_id) {
}

void mqtt_subscribe::serialize(shan::util::streambuf_ptr sb_ptr) {
	fixed_header::serialize(sb_ptr);

	// packet id (variable header)
	sb_ptr->write_uint16(_packet_id);

	// topic filter (payload)
	auto len = _topic_filters.size();
	for (int inx = 0 ; inx < len ; inx++) {
		encode_string(_topic_filters[inx], sb_ptr);
		sb_ptr->write_uint8(_max_qoses[inx]);
	}
}

void mqtt_subscribe::add_topic_filter(std::string topic_filter, uint8_t max_qos) {
	if (max_qos >= 3)
		throw mqtt_error("malformed packet. (SUBSCRIBE topic filter QoS)");

	_remaining_length += (2 + topic_filter.length());
	_topic_filters.push_back(topic_filter);

	_remaining_length ++;
	_max_qoses.push_back(max_qos);
}

bool mqtt_subscribe::is_topic_filter_valid(std::string topic_filter, bool& is_wild) {
	is_wild = false;

	char ch, prev_ch = '\0';
	auto len = topic_filter.length();
	for (int inx = 0 ; inx < len ; inx++) {
		ch = topic_filter[inx];
		if (ch == '#') {
			is_wild = true;
			if (((inx != 0) && (prev_ch != '/')) || (inx + 1 != len))
				return false;
		}
		else if (ch == '+') {
			is_wild = true;
			if (((inx != 0) && (prev_ch != '/')) || ((inx + 1 != len) && (topic_filter[inx + 1] != '/')))
				return false;
		}
		prev_ch = ch;
	}

	return true;
}
