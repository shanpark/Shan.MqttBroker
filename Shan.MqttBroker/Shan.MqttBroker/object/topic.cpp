//
//  topic.cpp
//  Shan.MqttBroker
//
//  Created by Sung Han Park on 2017. 4. 25..
//  Copyright © 2017 Sung Han Park. All rights reserved.
//

#include "topic.h"
#include "../packet/mqtt_puback.h"

topic::topic(std::string topic_filter)
: _topic_filter(topic_filter) {
}

bool topic::match(const std::string& topic_filter, const std::string& topic_name) {
	auto fit = topic_filter.begin();
	auto fend = topic_filter.end();
	auto nit = topic_name.begin();
	auto nend = topic_name.end();

	while (true) {
		if ((fit == fend) && (nit == nend))
			return true;
		else if ((fit == fend) && (nit != nend))
			return false;
		else if ((fit != fend) && (nit == nend)) {
			if (*fit == '/') {
				fit++;
				if ((fit != fend) && (*fit == '#'))
					return true;
			}
			return false;
		}

		if (*fit == '#') {
			return true;
		}
		else if (*fit == '+') {
			fit++;
			while ((nit != nend) && (*nit != '/'))
				nit++;
		}
		else {
			if (*fit == *nit) {
				fit++;
				nit++;
			}
			else {
				return false;
			}
		}
	}
}

void topic::publish(std::shared_ptr<mqtt_publish> packet_ptr) {
	std::lock_guard<std::mutex> lock(_mutex);

//...	// replace retain message
//	if (packet_ptr->retain()) {
//		if (packet_ptr->payload().size() == 0)
//			_retain_message = nullptr;
//		else
//			_retain_message = packet_ptr;
//	}

	// publish message to clients
	for (auto pair : _clients)
		pair.first->publish(pair.second, false, packet_ptr);
}

void topic::publish_retained(net::tcp_channel_context_base* ctx, std::shared_ptr<mqtt_publish> packet_ptr) {
	mqtt_client_ptr client_ptr = std::static_pointer_cast<mqtt_client>(ctx->param());

	try {
		std::lock_guard<std::mutex> lock(_mutex);
		uint8_t max_qos = _clients.at(client_ptr);
		client_ptr->publish(max_qos, true, packet_ptr);
	} catch (const std::exception& e) {
		throw mqtt_error("try to publish retained message to a client not to subscribe.");
	}
}

void topic::subscribe(mqtt_client_ptr client_ptr, uint8_t max_qos) {
	auto it = _clients.find(client_ptr);
	if (it != _clients.end())
		_clients.erase(it);

	_clients.insert(std::make_pair(client_ptr, max_qos));
}
