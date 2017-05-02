//
//  mqtt_connack.h
//  Shan.MqttBroker
//
//  Created by Sung Han Park on 2017. 4. 22..
//  Copyright © 2017 Sung Han Park. All rights reserved.
//

#ifndef mqtt_connack_h
#define mqtt_connack_h

#include "fixed_header.h"

class mqtt_connack : public fixed_header {
public:
	mqtt_connack(const fixed_header& fh, shan::util::streambuf_ptr sb_ptr);
	mqtt_connack(bool session_present, uint8_t connect_return_code);

	virtual void serialize(shan::util::streambuf_ptr sb_ptr) const override;

	virtual std::ostream& str(std::ostream& os) const override;

private:
	// variable header only. no payload
	bool _session_present;
	uint8_t _connect_return_code;
};

#endif /* mqtt_connack_h */
