//
//  subscription.h
//  Shan.MqttBroker
//
//  Created by Sung Han Park on 2017. 4. 25..
//  Copyright © 2017 Sung Han Park. All rights reserved.
//

#ifndef subscription_h
#define subscription_h

#include "../constants.h"

using namespace shan;

class subscription : public object {
public:
	subscription(std::string topic_filter, uint8_t max_qos)
	: _topic_filter(topic_filter), _max_qos(max_qos) {}

	const std::string& topic_filter() const { return _topic_filter; }
	uint8_t max_qos() const { return _max_qos; }

	class less {
	public:
		bool operator()(const std::shared_ptr<subscription>& lhs, const std::shared_ptr<subscription>& rhs) const {
			return (lhs->topic_filter() < rhs->topic_filter());
		}
	};

private:
	std::string _topic_filter;
	uint8_t _max_qos;
};

using subscription_ptr = std::shared_ptr<subscription>;

#endif /* subscription_h */
