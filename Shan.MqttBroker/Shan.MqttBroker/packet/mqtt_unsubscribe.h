//
//  mqtt_unsubscribe.h
//  Shan.MqttBroker
//
//  Created by Sung Han Park on 2017. 4. 24..
//  Copyright © 2017 Sung Han Park. All rights reserved.
//

#ifndef mqtt_unsubscribe_h
#define mqtt_unsubscribe_h

#include "fixed_header.h"

class mqtt_unsubscribe : public fixed_header {
public:
	mqtt_unsubscribe(const fixed_header& fh, shan::util::streambuf_ptr sb_ptr);
	mqtt_unsubscribe(uint16_t packet_id);

	virtual void serialize(shan::util::streambuf_ptr sb_ptr) override;

	void add_topic_filter(std::string topic_filter);

private:
	uint16_t _packet_id;
	std::vector<std::string> _topic_filters;
};

#endif /* mqtt_unsubscribe_h */
