//
//  mqtt_publish.h
//  Shan.MqttBroker
//
//  Created by Sung Han Park on 2017. 4. 23..
//  Copyright © 2017 Sung Han Park. All rights reserved.
//

#ifndef mqtt_publish_h
#define mqtt_publish_h

#include <vector>
#include "fixed_header.h"

class mqtt_publish : public fixed_header {
public:
	mqtt_publish(const fixed_header& fh, shan::util::streambuf_ptr sb_ptr);
	mqtt_publish(bool dup, uint8_t qos, bool retain, std::string topic_name, uint16_t packet_id, const std::vector<uint8_t>& payload);

	virtual void serialize(shan::util::streambuf_ptr sb_ptr) const override;

	void dup(bool d) { _dup = d; }
	bool dup() const { return _dup; }
	uint8_t qos() const { return _qos; }
	bool retain() const { return _retain; }
	
	const std::string& topic_name() const { return _topic_name; }
	uint16_t packet_id() const { return _packet_id; }

	const std::vector<uint8_t>& payload() const { return _payload; }

	virtual std::ostream& str(std::ostream& os) const override;

	class less {
	public:
		bool operator()(const std::shared_ptr<mqtt_publish>& lhs, const std::shared_ptr<mqtt_publish>& rhs) const {
			return (lhs->topic_name() < rhs->topic_name());
		}
	};

private:
	bool _dup;
	uint8_t _qos;
	bool _retain;

	std::string _topic_name;
	uint16_t _packet_id;

	std::vector<uint8_t> _payload;
};

#endif /* mqtt_publish_h */
