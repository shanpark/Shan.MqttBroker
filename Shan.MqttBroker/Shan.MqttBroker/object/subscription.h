//
//  subscription.h
//  Shan.MqttBroker
//
//  Created by Sung Han Park on 2017. 4. 25..
//  Copyright © 2017 Sung Han Park. All rights reserved.
//

#ifndef subscription_h
#define subscription_h

#include <shan/object.h>

using namespace shan;

/**
 subscription객체는 session에서만 사용된다. session은 동기화가 필요하지 않은 객체이므로
 subscription도 마찬가지로 동기화가 필요하지 않다.
 */
class subscription : public object {
public:
	subscription(std::string topic_filter, uint8_t max_qos, bool is_wild)
	: _topic_filter(topic_filter), _max_qos(max_qos), _is_wild(is_wild) {}

	const std::string& topic_filter() const { return _topic_filter; }
	uint8_t max_qos() const { return _max_qos; }
	bool is_wild() const { return _is_wild; }

	class less {
	public:
		bool operator()(const std::shared_ptr<subscription>& lhs, const std::shared_ptr<subscription>& rhs) const {
			return (lhs->topic_filter() < rhs->topic_filter());
		}
	};

private:
	std::string _topic_filter;
	uint8_t _max_qos;
	bool _is_wild;
};

using subscription_ptr = std::shared_ptr<subscription>;

#endif /* subscription_h */
