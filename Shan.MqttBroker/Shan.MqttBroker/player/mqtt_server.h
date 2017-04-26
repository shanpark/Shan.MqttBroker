//
//  mqtt_server_h
//  Shan.MqttBroker
//
//  Created by Sung Han Park on 2017. 4. 24..
//  Copyright © 2017 Sung Han Park. All rights reserved.
//

#ifndef mqtt_server_h
#define mqtt_server_h

#include <unordered_map>
#include "net/net.h"
#include "../packet/client_id_generator_if.h"
#include "../packet/mqtt_connect.h"
#include "../packet/mqtt_connack.h"
#include "../packet/mqtt_publish.h"
#include "../packet/mqtt_puback.h"
#include "../packet/mqtt_pubrec.h"
#include "../packet/mqtt_pubrel.h"
#include "../packet/mqtt_pubcomp.h"
#include "../packet/mqtt_subscribe.h"
#include "../packet/mqtt_suback.h"
#include "../packet/mqtt_unsubscribe.h"
#include "../packet/mqtt_unsuback.h"
#include "../packet/mqtt_pingreq.h"
#include "../packet/mqtt_pingresp.h"
#include "../packet/mqtt_disconnect.h"
#include "mqtt_client.h"
#include "../object/session.h"
#include "../object/topic.h"

class mqtt_server : public shan::object, public client_id_generator_if {
public:
	mqtt_server(shan::net::tcp_server_base* service_p);

	// client_id_generator_if interface
	virtual std::string generate_unique_id() override;

	mqtt_client_ptr get_client_ptr(std::string client_id);
	void reg_client_ptr(std::shared_ptr<mqtt_client> client_ptr);

	session_ptr get_session_ptr(std::string client_id);
	void reg_session_ptr(std::string client_id, session_ptr s_ptr);

	bool authorize(std::string username, std::vector<uint8_t> password);

	void handle_connect(shan::net::tcp_channel_context_base* ctx, std::shared_ptr<mqtt_connect> packet_ptr);
	void handle_publish(shan::net::tcp_channel_context_base* ctx, std::shared_ptr<mqtt_publish> packet_ptr);
	void handle_puback(shan::net::tcp_channel_context_base* ctx, std::shared_ptr<mqtt_puback> packet_ptr);
	void handle_pubrec(shan::net::tcp_channel_context_base* ctx, std::shared_ptr<mqtt_pubrec> packet_ptr);
	void handle_pubrel(shan::net::tcp_channel_context_base* ctx, std::shared_ptr<mqtt_pubrel> packet_ptr);
	void handle_pubcomp(shan::net::tcp_channel_context_base* ctx, std::shared_ptr<mqtt_pubcomp> packet_ptr);
	void handle_subscribe(shan::net::tcp_channel_context_base* ctx, std::shared_ptr<mqtt_subscribe> packet_ptr);

private:
	std::shared_ptr<shan::net::tcp_server_base> _service_ptr;

	std::unordered_map<std::string, mqtt_client_ptr> _clients; // <client_id, client>

	std::map<std::string, topic_ptr> _topics; // <topic_filter, topic>
	std::map<std::string, topic_ptr> _wild_topics; // <topic_filter, topic>

	std::set<std::shared_ptr<mqtt_publish>, mqtt_publish::less> _retained_messages;

	std::unordered_map<std::string, session_ptr> _sessions; // <client_id, session>
};

#endif /* mqtt_server_h */
