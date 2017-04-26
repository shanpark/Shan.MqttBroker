//
//  mqtt_pingresp.h
//  Shan.MqttBroker
//
//  Created by Sung Han Park on 2017. 4. 24..
//  Copyright © 2017 Sung Han Park. All rights reserved.
//

#ifndef mqtt_pingresp_h
#define mqtt_pingresp_h

#include "fixed_header.h"

class mqtt_pingresp : public fixed_header {
public:
	mqtt_pingresp(const fixed_header& fh, shan::util::streambuf_ptr sb_ptr);
	mqtt_pingresp();

	virtual void serialize(shan::util::streambuf_ptr sb_ptr) override;
};

#endif /* mqtt_pingresp_h */
