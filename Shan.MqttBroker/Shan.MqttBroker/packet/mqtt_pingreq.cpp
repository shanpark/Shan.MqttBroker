//
//  mqtt_pingreq.cpp
//  Shan.MqttBroker
//
//  Created by Sung Han Park on 2017. 4. 24..
//  Copyright © 2017 Sung Han Park. All rights reserved.
//

#include "mqtt_pingreq.h"

mqtt_pingreq::mqtt_pingreq(const fixed_header& fh, shan::util::streambuf_ptr sb_ptr)
: fixed_header(fh) {}

mqtt_pingreq::mqtt_pingreq()
: fixed_header(PINGREQ, 0, 0) {
}

void mqtt_pingreq::serialize(shan::util::streambuf_ptr sb_ptr) const {
	fixed_header::serialize(sb_ptr);
}

std::ostream& mqtt_pingreq::str(std::ostream& os) const {
	fixed_header::str(os);
	return os;
}
