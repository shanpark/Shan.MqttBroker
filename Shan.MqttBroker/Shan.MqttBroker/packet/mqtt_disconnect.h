//
//  mqtt_disconnect.h
//  Shan.MqttBroker
//
//  Created by Sung Han Park on 2017. 4. 24..
//  Copyright © 2017년 Sung Han Park. All rights reserved.
//

#ifndef mqtt_disconnect_h
#define mqtt_disconnect_h

#include "fixed_header.h"

class mqtt_disconnect : public fixed_header {
public:
	mqtt_disconnect(const fixed_header& fh, shan::util::streambuf_ptr sb_ptr);
	mqtt_disconnect();

	virtual void serialize(shan::util::streambuf_ptr sb_ptr) override;
};

#endif /* mqtt_disconnect_h */
