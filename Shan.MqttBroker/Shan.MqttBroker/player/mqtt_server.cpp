//
//  mqtt_server.cpp
//  Shan.MqttBroker
//
//  Created by Sung Han Park on 2017. 4. 24..
//  Copyright © 2017 Sung Han Park. All rights reserved.
//

#include "mqtt_server.h"

using namespace shan;

mqtt_server::mqtt_server(shan::net::tcp_server_base* service_p)
: _service_ptr(service_p) {
	std::srand(static_cast<unsigned int>(std::time(0)));
}

std::string mqtt_server::generate_unique_id() {
	std::string client_id;

	while (true) {
		client_id = "@" + std::to_string(std::rand()) + std::to_string(std::rand());
		auto client_ptr = get_client_ptr(client_id);
		if (!client_ptr)
			break;
	}

	return client_id;
}

mqtt_client_ptr mqtt_server::get_client_ptr(std::string client_id) {
	try {
		return _clients.at(client_id);
	} catch (const std::exception&) {
		return nullptr;
	}
}

void mqtt_server::reg_client_ptr(mqtt_client_ptr c_ptr) {
	_clients.insert(std::make_pair(c_ptr->client_id(), c_ptr));
}

session_ptr mqtt_server::get_session_ptr(std::string client_id) {
	try {
		return _sessions.at(client_id);
	} catch (const std::exception&) {
		return nullptr;
	}
}

void mqtt_server::reg_session_ptr(std::string client_id, session_ptr s_ptr) {
	_sessions.insert(std::make_pair(client_id, s_ptr));
}

bool mqtt_server::authorize(std::string username, std::vector<uint8_t> password) {
	return true; // TODO
}

void mqtt_server::handle_connect(net::tcp_channel_context_base* ctx, std::shared_ptr<mqtt_connect> packet_ptr) {
	// 첫 번째 CONNECT 패킷인지 검사. 아니면 끊는다.
	if (ctx->param()) // 이미 CONNECT를 받았으면 param이 지정되어 있을 것이다.
		ctx->close(); // 바로 끊어준다.

	// mqtt_client를 할당하고 context에 저장한다.
	auto client_ptr = std::make_shared<mqtt_client>(packet_ptr, ctx->channel_id());
	ctx->param(client_ptr); // set mqtt_client object as a context param.

	// CONNECT 패킷의 무결성을 체크하고 아니라면 적당한 return_code를 넣어서 바로 CONNACK를 보낸다.
	uint8_t return_code = packet_ptr->check_integrity(this); // 패킷의 무결성을 먼저 검사한다.
	if (return_code != 0) { // 무결하지 않으면 응답하고 끊도록 한다.
		client_ptr->stat(CL_DISCONNECTED); // channel_written에서 stat을 검사해서 끊어준다.
		auto res_packet = std::static_pointer_cast<object>(std::make_shared<mqtt_connack>(false, return_code));
		ctx->write(res_packet);
		return; // 응답 후 바로 리턴.
	}

	// CONNECT 패킷의 protocol 이름을 검사한다. 아니면 응답없이 바로 끊는다.
	if (packet_ptr->protocol_name() != "MQTT") // (:config) protocol name
		ctx->close();

	// CONNECT 패킷의 clean_session이 false이면 저장된 session을 복구해준다.
	bool session_present = false; // 복구된 경우 session_present를 true로 설정해준다.
	if (!client_ptr->clean_session()) {
		auto session_ptr = get_session_ptr(client_ptr->client_id());
		if (session_ptr) {
			client_ptr->set_session_ptr(session_ptr); // set prev session.
			session_present = true;
		}
		else {
			auto new_session_ptr = std::make_shared<session>(); // alloc new session.
			reg_session_ptr(client_ptr->client_id(), new_session_ptr);
			client_ptr->set_session_ptr(new_session_ptr);
			session_present = false;
		}
	}

	// 인증이 필요한 경우 사용자 이름, 비밀번호로 인증을 수행한다.
	if (cfg.need_auth()) { // (:config) 
		if (!authorize(packet_ptr->username(), packet_ptr->password())) {
			return_code = 0x05; // not authorized user.

			client_ptr->stat(CL_DISCONNECTED); // channel_written에서 stat을 검사해서 끊어줘야 한다.
			auto res_packet = std::static_pointer_cast<object>(std::make_shared<mqtt_connack>(false, return_code));
			ctx->write(res_packet);
			return;
		}
	}

	// 이미 같은 client id를 갖는 client가 접속상태이면 기존 접속을 닫아버리고 새로운 client를 등록해주어야 한다. [MQTT-3.1.4-2].
	auto old_client_ptr = get_client_ptr(packet_ptr->client_id());
	if (old_client_ptr) {
		old_client_ptr->stat(CL_DISCONNECTED);
		_service_ptr->close_channel(old_client_ptr->channel_id());
	}

	// new client 등록.
	reg_client_ptr(client_ptr);
	client_ptr->stat(CL_CONNECTED);

	// connack with success code
	auto res_packet = std::static_pointer_cast<object>(std::make_shared<mqtt_connack>(session_present, 0x00)); // success = 0x00
	ctx->write(res_packet);

	// keep alive 설정.
	net::tcp_idle_monitor* idle_monitor = static_cast<net::tcp_idle_monitor*>(_service_ptr->get_channel_handler_ptr(0).get());
	idle_monitor->reset_channel_timer(ctx->channel_id(), ID_CLIENT_IDLE, packet_ptr->keep_alive() * 1500, nullptr); // keep_alive x 1.5
}

void mqtt_server::handle_publish(shan::net::tcp_channel_context_base* ctx, std::shared_ptr<mqtt_publish> packet_ptr) {
	bool sent = false;
	try {
		auto topic_ptr = _topics.at(packet_ptr->topic_name());
		topic_ptr->publish(packet_ptr);
		sent = true;
	} catch (const std::exception&) {
	}

	for (auto pair : _wild_topics) {
		if (topic::match(pair.first, packet_ptr->topic_name())) {
			pair.second->publish(packet_ptr);
			sent = true;
		}
	}

	//... $sys 토픽으로의 publish는 위의 과정을 거치지 않고 미리 정의된 topic으로 바로 publish한다.
	// $sys 토픽은 따로 관리되고 그 토픽은 서버만 publish 할 수 있다. client는 $로 시작하는 토픽으로
	// publish를 하면 exception 처리되어 끊어지게 된다.

	if (!sent) { // no matching topic exists
		//... publish $sys/message/dropped
	}
}
