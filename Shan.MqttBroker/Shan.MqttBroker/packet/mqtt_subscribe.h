//
//  mqtt_subscribe.h
//  Shan.MqttBroker
//
//  Created by Sung Han Park on 2017. 4. 24..
//  Copyright © 2017 Sung Han Park. All rights reserved.
//

#ifndef mqtt_subscribe_h
#define mqtt_subscribe_h

#include <map>
#include "fixed_header.h"

class mqtt_subscribe : public fixed_header {
public:
	mqtt_subscribe(const fixed_header& fh, shan::util::streambuf_ptr sb_ptr);
	mqtt_subscribe(uint16_t packet_id);

	virtual void serialize(shan::util::streambuf_ptr sb_ptr) const override;

	void add_topic_filter(std::string topic_filter, uint8_t max_qos);
	static bool is_topic_filter_valid(std::string topic_filter, bool& is_wild);

	uint16_t packet_id() const { return _packet_id; }
	const std::vector<std::string>& topic_filters() const { return _topic_filters; }
	const std::vector<uint8_t>& max_qoses() const { return _max_qoses; }

	const std::vector<bool>& is_wild() const { return _is_wild; }

	virtual std::ostream& str(std::ostream& os) const override;

private:
	uint16_t _packet_id;
	std::vector<std::string> _topic_filters;
	std::vector<uint8_t> _max_qoses;

	std::vector<bool> _is_wild;
};

#endif /* mqtt_subscribe_h */
