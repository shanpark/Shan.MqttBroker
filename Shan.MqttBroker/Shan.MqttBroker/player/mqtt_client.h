//
//  mqtt_client.h
//  Shan.MqttBroker
//
//  Created by Sung Han Park on 2017. 4. 25..
//  Copyright © 2017 Sung Han Park. All rights reserved.
//

#ifndef mqtt_client_h
#define mqtt_client_h

#include "net/net.h"
#include "../packet/mqtt_connect.h"
#include "../object/session.h"

using namespace shan;

class mqtt_client : public object {
public:
	mqtt_client(std::shared_ptr<mqtt_connect> packet_ptr, std::size_t channel_id);

	std::size_t channel_id() const { return _channel_id; }
	std::string client_id() const { return _client_id; }
	
	uint8_t stat() const { return _stat; }
	void stat(uint8_t s) { _stat = s; }

	bool clean_session() const { return _clean_session; }

	void set_session_ptr(std::shared_ptr<session> session_ptr) { _session_ptr = session_ptr; }

private:
	std::size_t _channel_id;
	std::string _client_id;

	uint8_t _stat;
	bool _clean_session;

	bool _will_flag;
	uint8_t _will_qos;
	bool _will_retain;
	std::string _will_topic;
	std::vector<uint8_t> _will_message;

	std::shared_ptr<session> _session_ptr;
};

using mqtt_client_ptr = std::shared_ptr<mqtt_client>;
using mqtt_client_wptr = std::weak_ptr<mqtt_client>;

#endif /* mqtt_client_h */