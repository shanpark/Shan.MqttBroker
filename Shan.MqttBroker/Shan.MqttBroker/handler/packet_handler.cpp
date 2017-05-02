//
//  packet_handler.cpp
//  Shan.MqttBroker
//
//  Created by Sung Han Park on 2017. 4. 22..
//  Copyright © 2017 Sung Han Park. All rights reserved.
//

#include <iostream>
#include "spdlog/spdlog.h"
#include "packet_handler.h"
#include "../player/mqtt_client.h"

using namespace shan::net;

void packet_handler::channel_created(tcp_channel_context_base* ctx, tcp_channel_base* channel) {
}

void packet_handler::channel_connected(tcp_channel_context_base* ctx) {
}

void packet_handler::channel_read(tcp_channel_context_base* ctx, shan::object_ptr& data) {
	std::shared_ptr<fixed_header> packet_ptr = std::dynamic_pointer_cast<fixed_header>(data);
	if (!packet_ptr)
		throw std::runtime_error("the parameter of channel_read() is not a mqtt packet object.");

	switch (packet_ptr->type()) {
		case CONNECT:
			_server->handle_connect(ctx, std::static_pointer_cast<mqtt_connect>(packet_ptr));
			break;
		case CONNACK:
			throw mqtt_error("CONNACK packet was received.");
			break;
		case PUBLISH:
			_server->handle_publish(ctx, std::static_pointer_cast<mqtt_publish>(packet_ptr));
			break;
		case PUBACK:
			_server->handle_puback(ctx, std::static_pointer_cast<mqtt_puback>(packet_ptr));
			break;
		case PUBREC:
			_server->handle_pubrec(ctx, std::static_pointer_cast<mqtt_pubrec>(packet_ptr));
			break;
		case PUBREL:
			_server->handle_pubrel(ctx, std::static_pointer_cast<mqtt_pubrel>(packet_ptr));
			break;
		case PUBCOMP:
			_server->handle_pubcomp(ctx, std::static_pointer_cast<mqtt_pubcomp>(packet_ptr));
			break;
		case SUBSCRIBE:
			_server->handle_subscribe(ctx, std::static_pointer_cast<mqtt_subscribe>(packet_ptr));
			break;
		case SUBACK:
			throw mqtt_error("SUBACK packet was received.");
			break;
		case UNSUBSCRIBE:
			_server->handle_unsubscribe(ctx, std::static_pointer_cast<mqtt_unsubscribe>(packet_ptr));
			break;
		case UNSUBACK:
			throw mqtt_error("UNSUBACK packet was received.");
			break;
		case PINGREQ:
			_server->handle_pingreq(ctx, std::static_pointer_cast<mqtt_pingreq>(packet_ptr));
			break;
		case PINGRESP:
			throw mqtt_error("PINGRESP packet was received.");
			break;
		case DISCONNECT:
			_server->handle_disconnect(ctx, std::static_pointer_cast<mqtt_disconnect>(packet_ptr));
			break;
	}
}

void packet_handler::channel_rdbuf_empty(tcp_channel_context_base* ctx) {
}

void packet_handler::channel_write(tcp_channel_context_base* ctx, shan::object_ptr& data) {
}

void packet_handler::channel_written(tcp_channel_context_base* ctx, std::size_t bytes_transferred) {
	if (!ctx->param()) // context에 param이 없다는 것은 disconnected 상태라는 뜻이다.
		ctx->close(); // 그냥 닫으면 된다.
}

void packet_handler::channel_disconnected(tcp_channel_context_base* ctx) {
	_server->handle_channel_disconnected(ctx);

	std::ostringstream os;
	_server->str(os);
	LOGGER()->trace(os.str().c_str());
}

void packet_handler::user_event(tcp_channel_context_base* ctx, int64_t id, shan::object_ptr data_ptr) {
	if (id == ID_NO_CONNECT) {
		LOGGER()->warn("A client did not send the CONNECT packet within the specified time. Channel closed.");
		ctx->close();
	}
	else if (id == ID_CLIENT_IDLE) {
		LOGGER()->warn("A client did not send the PINGREQ packet within the keep-alive time. Channel closed.");
		ctx->close();
	}
	else if (id == ID_CLIENT_DISCONNECTED) {
		LOGGER()->warn("A client does not disconnect after sending the DISCONNECT packet. Channel closed.");
		ctx->close();
	}
}

void packet_handler::exception_caught(tcp_channel_context_base* ctx, const std::exception& e) {
	LOGGER()->error("Error: {}", e.what());

	ctx->close(); // 채널을 닫는다.
}
