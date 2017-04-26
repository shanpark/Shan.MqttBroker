//
//  mqtt_pingresp.cpp
//  Shan.MqttBroker
//
//  Created by Sung Han Park on 2017. 4. 24..
//  Copyright © 2017 Sung Han Park. All rights reserved.
//

#include "mqtt_pingresp.h"

mqtt_pingresp::mqtt_pingresp(const fixed_header& fh, shan::util::streambuf_ptr sb_ptr)
: fixed_header(fh) {}

mqtt_pingresp::mqtt_pingresp()
: fixed_header(PINGRESP, 0, 0) {
}

void mqtt_pingresp::serialize(shan::util::streambuf_ptr sb_ptr) {
	fixed_header::serialize(sb_ptr);
}
