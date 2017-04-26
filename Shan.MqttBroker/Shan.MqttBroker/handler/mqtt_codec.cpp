//
//  mqtt_codec.cpp
//  Shan.MqttBroker
//
//  Created by Sung Han Park on 2017. 4. 22..
//  Copyright © 2017 Sung Han Park. All rights reserved.
//

#include "mqtt_codec.h"
#include "../packet/mqtt_connect.h"
#include "../packet/mqtt_connack.h"
#include "../packet/mqtt_publish.h"
#include "../packet/mqtt_puback.h"
#include "../packet/mqtt_pubrec.h"
#include "../packet/mqtt_pubrel.h"
#include "../packet/mqtt_pubcomp.h"
#include "../packet/mqtt_subscribe.h"
#include "../packet/mqtt_suback.h"
#include "../packet/mqtt_unsubscribe.h"
#include "../packet/mqtt_unsuback.h"
#include "../packet/mqtt_pingreq.h"
#include "../packet/mqtt_pingresp.h"
#include "../packet/mqtt_disconnect.h"

using namespace shan;

void mqtt_codec::channel_read(net::tcp_channel_context_base* ctx, object_ptr& data) {
	util::streambuf_ptr sb_ptr = std::dynamic_pointer_cast<util::streambuf>(data);
	if (!sb_ptr)
		throw std::runtime_error("the parameter of channel_read() is not a streambuf object.");

	if (sb_ptr->in_size() >= 2) {
		try {
			sb_ptr->mark();
			fixed_header fh(sb_ptr);

			if (sb_ptr->in_size() < fh.remaining_length()) {
				sb_ptr->reset();
				ctx->done(true);
				return;
			}

			// mqtt packet generation. and return packet object.
			switch (fh.type()) {
				case CONNECT:
					data = std::static_pointer_cast<object>(std::make_shared<mqtt_connect>(fh, sb_ptr));
					break;
				case CONNACK:
					data = std::static_pointer_cast<object>(std::make_shared<mqtt_connack>(fh, sb_ptr));
					break;
				case PUBLISH:
					data = std::static_pointer_cast<object>(std::make_shared<mqtt_publish>(fh, sb_ptr));
					break;
				case PUBACK:
					data = std::static_pointer_cast<object>(std::make_shared<mqtt_puback>(fh, sb_ptr));
					break;
				case PUBREC:
					data = std::static_pointer_cast<object>(std::make_shared<mqtt_pubrec>(fh, sb_ptr));
					break;
				case PUBREL:
					data = std::static_pointer_cast<object>(std::make_shared<mqtt_pubrel>(fh, sb_ptr));
					break;
				case PUBCOMP:
					data = std::static_pointer_cast<object>(std::make_shared<mqtt_pubcomp>(fh, sb_ptr));
					break;
				case SUBSCRIBE:
					data = std::static_pointer_cast<object>(std::make_shared<mqtt_subscribe>(fh, sb_ptr));
					break;
				case SUBACK:
					data = std::static_pointer_cast<object>(std::make_shared<mqtt_suback>(fh, sb_ptr));
					break;
				case UNSUBSCRIBE:
					data = std::static_pointer_cast<object>(std::make_shared<mqtt_unsubscribe>(fh, sb_ptr));
					break;
				case UNSUBACK:
					data = std::static_pointer_cast<object>(std::make_shared<mqtt_unsuback>(fh, sb_ptr));
					break;
				case PINGREQ:
					data = std::static_pointer_cast<object>(std::make_shared<mqtt_pingreq>(fh, sb_ptr));
					break;
				case PINGRESP:
					data = std::static_pointer_cast<object>(std::make_shared<mqtt_pingresp>(fh, sb_ptr));
					break;
				case DISCONNECT:
					data = std::static_pointer_cast<object>(std::make_shared<mqtt_disconnect>(fh, sb_ptr));
					break;
			}
		} catch (const util::not_enough_error& e) {
			sb_ptr->reset();
		}
	}
	else {
		ctx->done(true);
	}
}

void mqtt_codec::channel_write(net::tcp_channel_context_base* ctx, object_ptr& data) {
	std::shared_ptr<fixed_header> packet_ptr = std::dynamic_pointer_cast<fixed_header>(data);
	if (!packet_ptr)
		throw std::runtime_error("the parameter of channel_write() is not a mqtt packet object.");

	// serialize packet object
	util::streambuf_ptr sb_ptr = std::make_shared<util::streambuf>(8 + packet_ptr->remaining_length());
	packet_ptr->serialize(sb_ptr);

	// return streambuf object
	data = sb_ptr;
}
