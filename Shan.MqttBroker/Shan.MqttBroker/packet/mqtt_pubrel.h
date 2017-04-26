//
//  mqtt_pubrel.h
//  Shan.MqttBroker
//
//  Created by Sung Han Park on 2017. 4. 24..
//  Copyright © 2017 Sung Han Park. All rights reserved.
//

#ifndef mqtt_pubrel_h
#define mqtt_pubrel_h

#include "fixed_header.h"

class mqtt_pubrel : public fixed_header {
public:
	mqtt_pubrel(const fixed_header& fh, shan::util::streambuf_ptr sb_ptr);
	mqtt_pubrel(uint16_t packet_id);

	virtual void serialize(shan::util::streambuf_ptr sb_ptr) override;

private:
	uint16_t _packet_id;
};

#endif /* mqtt_pubrel_h */
