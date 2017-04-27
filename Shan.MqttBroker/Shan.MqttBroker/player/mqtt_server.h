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
#include <set>
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

using namespace shan;

class mqtt_server : public object, public client_id_generator_if {
public:
	mqtt_server(net::tcp_server_base* service_p);

	// client_id_generator_if interface
	virtual std::string generate_unique_id() override;

	mqtt_client_ptr get_client_ptr(std::string client_id);
	void reg_client_ptr(std::shared_ptr<mqtt_client> client_ptr);

	session_ptr get_session_ptr(std::string client_id);
	void reg_session_ptr(std::string client_id, session_ptr s_ptr);

	bool authorize(std::string username, std::vector<uint8_t> password);
	topic_ptr get_topic_ptr_to_subscribe(std::string topic_filter, bool is_wild);
	void publish_retained_message(net::tcp_channel_context_base* ctx, const std::vector<topic_ptr>& subscribed_topics, const std::vector<topic_ptr>& subscribed_wild_topics);

	void handle_connect(net::tcp_channel_context_base* ctx, std::shared_ptr<mqtt_connect> packet_ptr);
	void handle_publish(net::tcp_channel_context_base* ctx, std::shared_ptr<mqtt_publish> packet_ptr);
	void handle_puback(net::tcp_channel_context_base* ctx, std::shared_ptr<mqtt_puback> packet_ptr);
	void handle_pubrec(net::tcp_channel_context_base* ctx, std::shared_ptr<mqtt_pubrec> packet_ptr);
	void handle_pubrel(net::tcp_channel_context_base* ctx, std::shared_ptr<mqtt_pubrel> packet_ptr);
	void handle_pubcomp(net::tcp_channel_context_base* ctx, std::shared_ptr<mqtt_pubcomp> packet_ptr);
	void handle_subscribe(net::tcp_channel_context_base* ctx, std::shared_ptr<mqtt_subscribe> packet_ptr);
	void handle_unsubscribe(net::tcp_channel_context_base* ctx, std::shared_ptr<mqtt_unsubscribe> packet_ptr);
	void handle_pingreq(net::tcp_channel_context_base* ctx, std::shared_ptr<mqtt_pingreq> packet_ptr);
	void handle_disconnect(net::tcp_channel_context_base* ctx, std::shared_ptr<mqtt_disconnect> packet_ptr);

private:
	std::shared_ptr<net::tcp_server_base> _service_ptr;

	// 아래 멤버들은 모두 mutex로 보호되어야 한다.
	std::mutex _client_mutex;
	std::unordered_map<std::string, mqtt_client_ptr> _clients; // <client_id, client>

	std::mutex _topic_mutex;
	std::map<std::string, topic_ptr> _topics; // <topic_filter, topic>
	std::map<std::string, topic_ptr> _wild_topics; // <topic_filter, topic>

	std::mutex _session_mutex;
	std::unordered_map<std::string, session_ptr> _sessions; // <client_id, session>
};

#endif /* mqtt_server_h */
