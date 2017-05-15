//
//  mqtt_client.h
//  Shan.MqttBroker
//
//  Created by Sung Han Park on 2017. 4. 25..
//  Copyright © 2017 Sung Han Park. All rights reserved.
//

#ifndef mqtt_client_h
#define mqtt_client_h

#include <shan/net.h>
#include "../packet/mqtt_connect.h"
#include "../packet/mqtt_publish.h"
#include "../packet/mqtt_puback.h"
#include "../packet/mqtt_pubrec.h"
#include "../packet/mqtt_pubrel.h"
#include "../packet/mqtt_pubcomp.h"
#include "../object/session.h"

using namespace shan;

class mqtt_client : public object {
public:
	mqtt_client(std::shared_ptr<mqtt_connect> packet_ptr, std::size_t channel_id);

	std::size_t channel_id() const { return _channel_id; }
	std::string client_id() const { return _client_id; }

	void admin(bool a) { _admin = a; }
	bool admin() { return _admin; }

	bool clean_session() const { return _clean_session; }

	void set_session_ptr(session_ptr s_ptr) { _session_ptr = s_ptr; }
	session_ptr get_session_ptr() { return _session_ptr; }

	bool will_flag() const { return _will_flag; }
	uint8_t will_qos() const { return _will_qos; }
	bool will_retain() const { return _will_retain; }
	const std::string& will_topic() const { return _will_topic; }
	const std::vector<uint8_t>& will_message() const { return _will_message; }

	void publish(shan::net::tcp_server_base* server_p, uint8_t max_qos, bool retained, std::shared_ptr<mqtt_publish> packet_ptr);
	void handle_puback(std::shared_ptr<mqtt_puback> packet_ptr);
	void handle_pubrec(shan::net::tcp_server_base* server_p, std::shared_ptr<mqtt_pubrec> packet_ptr);
	void handle_pubrel(shan::net::tcp_server_base* server_p, std::shared_ptr<mqtt_pubrel> packet_ptr);
	void handle_pubcomp(std::shared_ptr<mqtt_pubcomp> packet_ptr);

	class less {
	public:
		bool operator()(const std::shared_ptr<mqtt_client>& lhs, const std::shared_ptr<mqtt_client>& rhs) const {
			return (lhs->client_id() < rhs->client_id());
		}
	};

private:
	std::size_t _channel_id;
	std::string _client_id;

	bool _admin;
	bool _clean_session;

	bool _will_flag;
	uint8_t _will_qos;
	bool _will_retain;
	std::string _will_topic;
	std::vector<uint8_t> _will_message;

	session_ptr _session_ptr;
};

using mqtt_client_ptr = std::shared_ptr<mqtt_client>;

#endif /* mqtt_client_h */
