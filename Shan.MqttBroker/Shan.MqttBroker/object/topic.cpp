//
//  topic.cpp
//  Shan.MqttBroker
//
//  Created by Sung Han Park on 2017. 4. 25..
//  Copyright © 2017 Sung Han Park. All rights reserved.
//

#include "topic.h"
#include "../packet/mqtt_puback.h"

using namespace shan;

topic::topic(std::string topic_filter)
: _topic_filter(topic_filter) {
}

bool topic::empty() {
	std::lock_guard<std::mutex> lock(_client_mutex);
	if (!_retained_message && _clients.empty())
		return true;
	
	return false;
}

std::shared_ptr<mqtt_publish> topic::retained_message() {
	return std::atomic_load(&_retained_message); //... gcc 4.9에서 shared_ptr에 대한 automic_xxx()를 지원하지 않는다. 그래서 어쩔 수 없이 mutex로 교체함. 나중에 다시 복구할 것임.
}

void topic::retained_message(std::shared_ptr<mqtt_publish> retain_message) {
	if (retain_message->payload().size() == 0)
		std::atomic_store(&_retained_message, std::shared_ptr<mqtt_publish>(nullptr)); // payload가 0이면 저장하지 않고 기존 메시지를 삭제한다.
	else
		std::atomic_store(&_retained_message, retain_message); // 새로운 메시지로 교체한다.
}

bool topic::publish(net::tcp_server_base* server_p, std::shared_ptr<mqtt_publish> packet_ptr) {
	if (packet_ptr->retain())
		retained_message(packet_ptr); // retain message 저장. 길이가 0이면 삭제될 것이다.

	bool sent = false;
	{
		std::lock_guard<std::mutex> lock(_client_mutex);

		// publish message to clients
		for (auto pair : _clients) {
			pair.first->publish(server_p, pair.second, false, packet_ptr);
			sent = true;
		}
	}

	return sent;
}

void topic::publish_retained(net::tcp_server_base* server_p, net::tcp_channel_context_base* ctx, std::shared_ptr<mqtt_publish> packet_ptr) {
	mqtt_client_ptr client_ptr = std::static_pointer_cast<mqtt_client>(ctx->param());

	try {
		std::lock_guard<std::mutex> lock(_client_mutex);
		uint8_t max_qos = _clients.at(client_ptr);
		client_ptr->publish(server_p, max_qos, true, packet_ptr);
	} catch (const std::exception&) {
		throw mqtt_error("attempt to publish retained message to a client that is not subscribing.");
	}
}

void topic::subscribe(mqtt_client_ptr client_ptr, uint8_t max_qos) {
	std::lock_guard<std::mutex> lock(_client_mutex);
	
	_clients.erase(client_ptr); // 기존에 같은 client 정보가 있으면 삭제해준다.

	_clients.insert(std::make_pair(client_ptr, max_qos)); // 새로운 client 정보를 넣어준다.
}

void topic::unsubscribe(mqtt_client_ptr client_ptr) {
	std::lock_guard<std::mutex> lock(_client_mutex);

	_clients.erase(client_ptr); // 기존 client 정보를 삭제해준다.
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
