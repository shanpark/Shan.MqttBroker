//
//  mqtt_pingreq.h
//  Shan.MqttBroker
//
//  Created by Sung Han Park on 2017. 4. 24..
//  Copyright © 2017 Sung Han Park. All rights reserved.
//

#ifndef mqtt_pingreq_h
#define mqtt_pingreq_h

#include "fixed_header.h"

class mqtt_pingreq : public fixed_header {
public:
	mqtt_pingreq(const fixed_header& fh, shan::util::streambuf_ptr sb_ptr);
	mqtt_pingreq();

	virtual void serialize(shan::util::streambuf_ptr sb_ptr) const override;

	virtual std::ostream& str(std::ostream& os) const override;
};

#endif /* mqtt_pingreq_h */
