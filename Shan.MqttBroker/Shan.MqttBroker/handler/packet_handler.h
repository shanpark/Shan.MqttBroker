//
//  packet_handler.h
//  Shan.MqttBroker
//
//  Created by Sung Han Park on 2017. 4. 22..
//  Copyright © 2017 Sung Han Park. All rights reserved.
//

#ifndef packet_handler_h
#define packet_handler_h

#include "../constants.h"
#include "../player/mqtt_server.h"

using namespace shan;

class packet_handler : public net::tcp_channel_handler {
public:
	packet_handler(mqtt_server* server)
	: _server(server) {}

	virtual void channel_created(net::tcp_channel_context_base* ctx, net::tcp_channel_base* channel) override;
	virtual void channel_connected(net::tcp_channel_context_base* ctx) override;
	virtual void channel_read(net::tcp_channel_context_base* ctx, object_ptr& data) override;
	virtual void channel_rdbuf_empty(net::tcp_channel_context_base* ctx) override;
	virtual void channel_write(net::tcp_channel_context_base* ctx, object_ptr& data) override;
	virtual void channel_written(net::tcp_channel_context_base* ctx, std::size_t bytes_transferred) override;
	virtual void channel_disconnected(net::tcp_channel_context_base* ctx) override;
	virtual void user_event(net::tcp_channel_context_base* ctx, int64_t id, shan::object_ptr data_ptr) override;
	virtual void exception_caught(net::tcp_channel_context_base* ctx, const std::exception& e) override;

private:
	mqtt_server* _server; // do not change to shared_ptr.
};

#endif /* packet_handler_h */
