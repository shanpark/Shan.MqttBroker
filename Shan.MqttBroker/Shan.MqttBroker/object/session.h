//
//  session.h
//  Shan.MqttBroker
//
//  Created by Sung Han Park on 2017. 4. 25..
//  Copyright © 2017 Sung Han Park. All rights reserved.
//

#ifndef session_h
#define session_h

#include <set>
#include "../constants.h"
#include "../packet/mqtt_publish.h"
#include "subscription.h"

using namespace shan;

//... client가 clean_session인 경우 channel_disconnected 핸들러에서도 검사해서 삭제해주어야 한다.
/**
 session은 관련 client에서만 access되고 client는 handler에서만 access되므로 동기화가 필요하지 않다.
 */
class session : public object {
public:
	session()
	: _next_packet_id(1) {}

	uint16_t new_packet_id();

	const std::set<subscription_ptr, subscription::less>& subscriptions() const { return _subscriptions; }
	void add_subscription(std::string topic_filter, uint8_t max_qos, bool is_wild);
	void delete_subscription(std::string topic_filter);

	void add_packet_id_for_pubrel(uint16_t packet_id);
	void release_packet_id_for_pubrel(uint16_t packet_id);
	
	void add_packet_id_for_pubcomp(uint16_t packet_id);
	void release_packet_id_for_pubcomp(uint16_t packet_id);

	void add_publish_for_ack(std::shared_ptr<mqtt_publish> publish_ptr);
	void release_publish_for_ack(uint16_t packet_id);

	void add_publish_for_rec(std::shared_ptr<mqtt_publish> publish_ptr);
	void release_publish_for_rec(uint16_t packet_id);

private:
	uint16_t _next_packet_id;
	
	std::set<subscription_ptr, subscription::less> _subscriptions; // <topic_filter, max_qos>

	std::vector<uint16_t> _packet_id_for_pubrel; // when I'm receiver

	std::vector<std::shared_ptr<mqtt_publish>> _publish_for_ack; // when I'm sender
	std::vector<std::shared_ptr<mqtt_publish>> _publish_for_rec; // when I'm sender
	std::vector<uint16_t> _packet_id_for_pubcomp; // when I'm sender
};

using session_ptr = std::shared_ptr<session>;

#endif /* session_h */
