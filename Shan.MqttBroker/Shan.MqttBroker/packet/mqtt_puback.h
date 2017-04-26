//
//  mqtt_puback.h
//  Shan.MqttBroker
//
//  Created by Sung Han Park on 2017. 4. 23..
//  Copyright © 2017 Sung Han Park. All rights reserved.
//

#ifndef mqtt_puback_h
#define mqtt_puback_h

#include "fixed_header.h"

class mqtt_puback : public fixed_header {
public:
	mqtt_puback(const fixed_header& fh, shan::util::streambuf_ptr sb_ptr);
	mqtt_puback(uint16_t packet_id);

	virtual void serialize(shan::util::streambuf_ptr sb_ptr) override;

private:
	uint16_t _packet_id;
};

#endif /* mqtt_puback_h */
