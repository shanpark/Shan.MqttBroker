//
//  mqtt_suback.h
//  Shan.MqttBroker
//
//  Created by Sung Han Park on 2017. 4. 24..
//  Copyright © 2017 Sung Han Park. All rights reserved.
//

#ifndef mqtt_suback_h
#define mqtt_suback_h

#include "fixed_header.h"

class mqtt_suback : public fixed_header {
public:
	mqtt_suback(const fixed_header& fh, shan::util::streambuf_ptr sb_ptr);
	mqtt_suback(uint16_t packet_id);

	virtual void serialize(shan::util::streambuf_ptr sb_ptr) const override;

	void add_return_code(uint8_t return_code);

	virtual std::ostream& str(std::ostream& os) const override;

private:
	uint16_t _packet_id;

	std::vector<uint8_t> _return_codes;
};

#endif /* mqtt_suback_h */
