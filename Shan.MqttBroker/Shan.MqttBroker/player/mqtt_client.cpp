//
//  mqtt_client.cpp
//  Shan.MqttBroker
//
//  Created by Sung Han Park on 2017. 4. 25..
//  Copyright © 2017 Sung Han Park. All rights reserved.
//

#include "mqtt_client.h"

mqtt_client::mqtt_client(std::shared_ptr<mqtt_connect> packet_ptr, std::size_t channel_id)
: _channel_id(channel_id), _client_id(packet_ptr->client_id()), _admin(false), _clean_session(packet_ptr->clean_session())
, _will_flag(packet_ptr->will_flag()), _will_qos(packet_ptr->will_qos()), _will_retain(packet_ptr->will_retain())
, _will_topic(packet_ptr->will_topic()), _will_message(packet_ptr->will_message()) {
}

void mqtt_client::publish(shan::net::tcp_server_base* server_p, uint8_t max_qos, bool retained, std::shared_ptr<mqtt_publish> packet_ptr) {
	// The QoS of Payload Messages sent in response to a Subscription MUST be the minimum of the QoS of the originally published message and the maximum QoS granted by the Server.
	uint8_t qos = std::min(packet_ptr->qos(), max_qos);

	// allocate new publish packet.
	// When sending a PUBLISH Packet to a Client the Server MUST set the RETAIN flag to 1 if a message is sent as a result of a new subscription being made by a Client [MQTT-3.3.1-8].
	// It MUST set the RETAIN flag to 0 when a PUBLISH Packet is sent to a Client because it matches an established subscription regardless of how the flag was set in the message it received [MQTT-3.3.1-9].
	auto publish_ptr = std::make_shared<mqtt_publish>(false, qos, retained, packet_ptr->topic_name(), _session_ptr->new_packet_id(), packet_ptr->payload());

	if (qos == QOS_AT_LEAST_ONCE)
		_session_ptr->add_publish_for_ack(publish_ptr);
	else if (qos == QOS_EXACTLY_ONCE)
		_session_ptr->add_publish_for_rec(publish_ptr);
	
	server_p->write_channel(_channel_id, publish_ptr);
}

void mqtt_client::handle_puback(std::shared_ptr<mqtt_puback> packet_ptr) {
	// I'm sender
	_session_ptr->release_publish_for_ack(packet_ptr->packet_id());
}

void mqtt_client::handle_pubrec(shan::net::tcp_server_base* server_p, std::shared_ptr<mqtt_pubrec> packet_ptr) {
	// I'm senter
	_session_ptr->release_publish_for_rec(packet_ptr->packet_id());

	_session_ptr->add_packet_id_for_pubcomp(packet_ptr->packet_id());

	auto pubrel_ptr = std::make_shared<mqtt_pubrel>(packet_ptr->packet_id());
	server_p->write_channel(_channel_id, pubrel_ptr);
}

void mqtt_client::handle_pubrel(shan::net::tcp_server_base* server_p, std::shared_ptr<mqtt_pubrel> packet_ptr) {
	// I'm receiver
	_session_ptr->release_packet_id_for_pubrel(packet_ptr->packet_id());

	auto pubcomp_ptr = std::make_shared<mqtt_pubcomp>(packet_ptr->packet_id());
	server_p->write_channel(_channel_id, pubcomp_ptr);
}

void mqtt_client::handle_pubcomp(std::shared_ptr<mqtt_pubcomp> packet_ptr) {
	// I'm senter
	_session_ptr->release_packet_id_for_pubcomp(packet_ptr->packet_id());
}
