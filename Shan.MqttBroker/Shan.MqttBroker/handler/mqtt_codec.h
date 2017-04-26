//
//  mqtt_codec.h
//  Shan.MqttBroker
//
//  Created by Sung Han Park on 2017. 4. 22..
//  Copyright © 2017 Sung Han Park. All rights reserved.
//

#ifndef mqtt_codec_h
#define mqtt_codec_h

#include "constants.h"

using namespace shan;

class mqtt_codec : public net::tcp_channel_handler {
public:
	virtual void channel_read(net::tcp_channel_context_base* ctx, object_ptr& data) override;
	virtual void channel_write(net::tcp_channel_context_base* ctx, object_ptr& data) override;
};

#endif /* mqtt_codec_h */
