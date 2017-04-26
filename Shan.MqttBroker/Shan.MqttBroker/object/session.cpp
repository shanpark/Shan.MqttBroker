//
//  session.cpp
//  Shan.MqttBroker
//
//  Created by Sung Han Park on 2017. 4. 25..
//  Copyright © 2017 Sung Han Park. All rights reserved.
//

#include "session.h"

uint16_t session::new_packet_id() {
	do {
		uint16_t new_id = _next_packet_id++;
		if (new_id == 0)
			new_id = _next_packet_id++;

		auto pred = [new_id](const std::shared_ptr<mqtt_publish>& publish_ptr){
			return (publish_ptr->packet_id() == new_id);
		};

		if (std::find_if(_publish_for_ack.begin(), _publish_for_ack.end(), pred) != _publish_for_ack.end())
			continue; // already used. try next.

		if (std::find_if(_publish_for_rec.begin(), _publish_for_rec.end(), pred) != _publish_for_rec.end())
			continue; // already used. try next.

		//... 위의 두 벡터 말고 더 있있면 그것도 여기 추가해 주어야 한다.
		return new_id;
	} while (true);
}

void session::add_subscription(std::string topic_filter, uint8_t max_qos) {
	auto sub_ptr = std::make_shared<subscription>(topic_filter, max_qos);

	auto it = _subscriptions.find(sub_ptr);
	if (it != _subscriptions.end())
		_subscriptions.erase(it);

	_subscriptions.insert(sub_ptr);
}

void session::add_packet_id_for_pubrel(uint16_t packet_id) {
	_packet_id_for_pubrel.push_back(packet_id);
}

void session::release_packet_id_for_pubrel(uint16_t packet_id) {
	auto it = std::find(_packet_id_for_pubrel.begin(), _packet_id_for_pubrel.end(), packet_id);
	if (it != _packet_id_for_pubrel.end())
		_packet_id_for_pubrel.erase(it);
}

void session::add_packet_id_for_pubcomp(uint16_t packet_id) {
	_packet_id_for_pubcomp.push_back(packet_id);
}

void session::release_packet_id_for_pubcomp(uint16_t packet_id) {
	auto it = std::find(_packet_id_for_pubcomp.begin(), _packet_id_for_pubcomp.end(), packet_id);
	if (it != _packet_id_for_pubcomp.end())
		_packet_id_for_pubcomp.erase(it);
}

void session::add_publish_for_ack(std::shared_ptr<mqtt_publish> publish_ptr) {
	_publish_for_ack.push_back(publish_ptr);
}

void session::release_publish_for_ack(uint16_t packet_id) {
	auto pred = [packet_id](const std::shared_ptr<mqtt_publish>& publish_ptr){
		return (publish_ptr->packet_id() == packet_id);
	};

	auto it = std::find_if(_publish_for_ack.begin(), _publish_for_ack.end(), pred);
	if (it != _publish_for_ack.end())
		_publish_for_ack.erase(it);
}

void session::add_publish_for_rec(std::shared_ptr<mqtt_publish> publish_ptr) {
	_publish_for_rec.push_back(publish_ptr);
}

void session::release_publish_for_rec(uint16_t packet_id) {
	auto pred = [packet_id](const std::shared_ptr<mqtt_publish>& publish_ptr){
		return (publish_ptr->packet_id() == packet_id);
	};

	auto it = std::find_if(_publish_for_rec.begin(), _publish_for_rec.end(), pred);
	if (it != _publish_for_rec.end())
		_publish_for_rec.erase(it);
}
