//
//  mqtt_unsuback.h
//  Shan.MqttBroker
//
//  Created by Sung Han Park on 2017. 4. 24..
//  Copyright © 2017 Sung Han Park. All rights reserved.
//

#ifndef mqtt_unsuback_h
#define mqtt_unsuback_h

#include "fixed_header.h"

class mqtt_unsuback : public fixed_header {
public:
	mqtt_unsuback(const fixed_header& fh, shan::util::streambuf_ptr sb_ptr);
	mqtt_unsuback(uint16_t packet_id);

	virtual void serialize(shan::util::streambuf_ptr sb_ptr) override;

private:
	uint16_t _packet_id;
};

#endif /* mqtt_unsuback_h */
