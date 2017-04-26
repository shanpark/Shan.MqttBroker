//
//  topic.cpp
//  Shan.MqttBroker
//
//  Created by Sung Han Park on 2017. 4. 25..
//  Copyright © 2017 Sung Han Park. All rights reserved.
//

#include "topic.h"

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
	for (auto it = _clients.begin() ; it != _clients.end() ; ) {
		// pair = <mqtt_client_wptr, MaxQoS>
		if (it->first.expired()) { // client disconnected. so deleted.
			it = _clients.erase(it);
		}
		else {
			it->first->publish(it->second, packet_ptr); // (granted qos, packet)
			it++;
		}
	}
}
