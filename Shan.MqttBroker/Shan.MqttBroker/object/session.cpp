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

		// 내가 시작한 packet 송수신들에서 이미 사용중인 id는 사용해서는 안된다.
		// 아래 기록들은 server가 전송한 publish 패킷 송수신 시나리오에서 응답을 기다리는 것들이다.
		// 여기에서 사용되고 있다면 다른 id를 다시 받도록 한다.
		if (std::find_if(_publish_for_ack.begin(), _publish_for_ack.end(), pred) != _publish_for_ack.end())
			continue; // already used. try next.

		if (std::find_if(_publish_for_rec.begin(), _publish_for_rec.end(), pred) != _publish_for_rec.end())
			continue; // already used. try next.

		if (std::find(_packet_id_for_pubcomp.begin(), _packet_id_for_pubcomp.end(), new_id) != _packet_id_for_pubcomp.end())
			continue;
		
		// 서버가 시작하는 패킷은 PUBLISH 밖에 없으므로 위의 3가지 경우가 packet id를 생성해서 사용중인 경우의 전부이다.
		
		return new_id;
	} while (true);
}

void session::add_subscription(std::string topic_filter, uint8_t max_qos, bool is_wild) {
	auto sub_ptr = std::make_shared<subscription>(topic_filter, max_qos, is_wild);
	
	// 기존의 subscription을 삭제하고
	_subscriptions.erase(sub_ptr); // 같은 topic_filter를 갖는 subscription이 사제될 것이다.
	
	// 다시 새로운 subscription을 넣는다.
	_subscriptions.insert(sub_ptr);
}

void session::delete_subscription(std::string topic_filter) {
	auto sub_ptr = std::make_shared<subscription>(topic_filter, 0, false); // search를 위해 임시 객체 생성
	_subscriptions.erase(sub_ptr);
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
