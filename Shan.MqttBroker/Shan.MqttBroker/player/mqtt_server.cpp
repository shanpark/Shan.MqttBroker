//
//  mqtt_server.cpp
//  Shan.MqttBroker
//
//  Created by Sung Han Park on 2017. 4. 24..
//  Copyright © 2017 Sung Han Park. All rights reserved.
//

#include "mqtt_server.h"

using namespace shan;

mqtt_server::mqtt_server(net::tcp_server_base* service_p)
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
		std::lock_guard<std::mutex> lock(_client_mutex);
		return _clients.at(client_id);
	} catch (const std::exception&) {
		return nullptr;
	}
}

void mqtt_server::reg_client_ptr(mqtt_client_ptr c_ptr) {
	std::lock_guard<std::mutex> lock(_client_mutex);
	_clients.insert(std::make_pair(c_ptr->client_id(), c_ptr));
}

session_ptr mqtt_server::get_session_ptr(std::string client_id) {
	try {
		std::lock_guard<std::mutex> lock(_session_mutex);
		return _sessions.at(client_id);
	} catch (const std::exception&) {
		return nullptr;
	}
}

void mqtt_server::reg_session_ptr(std::string client_id, session_ptr s_ptr) {
	std::lock_guard<std::mutex> lock(_session_mutex);
	_sessions.insert(std::make_pair(client_id, s_ptr));
}

bool mqtt_server::authorize(std::string username, std::vector<uint8_t> password) {
	return true; // TODO
}

topic_ptr mqtt_server::get_topic_ptr_to_subscribe(std::string topic_filter, bool is_wild) {
	topic_ptr t_ptr;
	
	if (is_wild) {
		std::lock_guard<std::mutex> lock(_topic_mutex);
		try {
			t_ptr = _wild_topics.at(topic_filter);
		} catch (const std::exception& e) {
			t_ptr =  std::make_shared<topic>(topic_filter);
			_wild_topics.insert(std::make_pair(topic_filter, t_ptr));
		}
	}
	else {
		std::lock_guard<std::mutex> lock(_topic_mutex);
		try {
			t_ptr = _topics.at(topic_filter);
		} catch (const std::exception& e) {
			t_ptr =  std::make_shared<topic>(topic_filter);
			_topics.insert(std::make_pair(topic_filter, t_ptr));
		}
	}
	
	return t_ptr;
}

void mqtt_server::publish_retained_message(net::tcp_channel_context_base* ctx, const std::vector<topic_ptr>& subscribed_topics, const std::vector<topic_ptr>& subscribed_wild_topics) {
	for (auto t_ptr : subscribed_topics)
		if (t_ptr->retained_message())
			t_ptr->publish_retained(ctx, t_ptr->retained_message()); // topic이 등록된 qos를 가지고 있기 때문에 topic을 통해서 보내야 한다.
	
	for (auto wt_ptr : subscribed_wild_topics) {
		std::lock_guard<std::mutex> lock(_topic_mutex);
		for (auto pair : _topics) {
			if (pair.second->retained_message() && topic::match(wt_ptr->topic_filter(), pair.second->topic_filter()))
				wt_ptr->publish_retained(ctx, pair.second->retained_message());
		}
	}
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
		ctx->write(std::static_pointer_cast<object>(std::make_shared<mqtt_connack>(false, return_code)));
		return; // 응답 후 바로 리턴.
	}

	// CONNECT 패킷의 protocol 이름을 검사한다. 아니면 응답없이 바로 끊는다.
	if (packet_ptr->protocol_name() != "MQTT") // (:config) protocol name
		ctx->close();

	// CONNECT 패킷의 clean_session이 false이면 저장된 session을 복구해준다.
	bool session_present = false; // 복구된 경우 session_present를 true로 설정해준다.
	if (!client_ptr->clean_session()) {
		auto s_ptr = get_session_ptr(client_ptr->client_id());
		if (s_ptr) {
			client_ptr->set_session_ptr(s_ptr);
			session_present = true;
		}
		else {
			auto new_s_ptr = std::make_shared<session>(); // alloc new session.
			reg_session_ptr(client_ptr->client_id(), new_s_ptr);
			client_ptr->set_session_ptr(new_s_ptr);
			session_present = false;
		}
	}

	// 인증이 필요한 경우 사용자 이름, 비밀번호로 인증을 수행한다.
	if (g_cfg.need_auth()) { // (:config)
		if (!authorize(packet_ptr->username(), packet_ptr->password())) {
			return_code = 0x05; // not authorized user.

			client_ptr->stat(CL_DISCONNECTED); // channel_written에서 stat을 검사해서 끊어줘야 한다.
			ctx->write(std::static_pointer_cast<object>(std::make_shared<mqtt_connack>(false, return_code)));
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
	ctx->write(std::static_pointer_cast<object>(std::make_shared<mqtt_connack>(session_present, 0x00))); // success = 0x00

	// keep alive 설정.
	net::tcp_idle_monitor* idle_monitor = static_cast<net::tcp_idle_monitor*>(_service_ptr->get_channel_handler_ptr(0).get());
	idle_monitor->reset_channel_timer(ctx->channel_id(), ID_CLIENT_IDLE, packet_ptr->keep_alive() * 1500, nullptr); // keep_alive x 1.5
	
	// 새로 복구된 session의 subscription들을 복구해 준다. 복구된 session이 있으면 client는 다시 subscribe를 하지 않는다.
	// 따라서 session에 저장된 subscription 정보를 토대로 다시 복구해 주어야 하고 retained message들도 보내주는 게 맞다.
 	if (session_present) {
		auto s_ptr = client_ptr->get_session_ptr();
		std::vector<topic_ptr> subscribed_topics;
		std::vector<topic_ptr> subscribed_wild_topics;
		for (auto sub_ptr : s_ptr->subscriptions()) { // <topic_filter, max_qos>
			auto& topic_filter = sub_ptr->topic_filter();
			auto max_qos = sub_ptr->max_qos();
			auto is_wild = sub_ptr->is_wild();
			
			topic_ptr t_ptr = get_topic_ptr_to_subscribe(topic_filter, is_wild);
			if (is_wild)
				subscribed_wild_topics.push_back(t_ptr); // for retained message.
			else
				subscribed_topics.push_back(t_ptr); // for retained message.

			// add client to topic.
			t_ptr->subscribe(client_ptr, max_qos);
		}
		
		// then send retained message
		publish_retained_message(ctx, subscribed_topics, subscribed_wild_topics);
	}
}

void mqtt_server::handle_publish(net::tcp_channel_context_base* ctx, std::shared_ptr<mqtt_publish> packet_ptr) {
	mqtt_client_ptr sender_agent_ptr = std::static_pointer_cast<mqtt_client>(ctx->param());

	// receiver behavior.
	if (packet_ptr->qos() == 1) {
		ctx->write(std::make_shared<mqtt_puback>(packet_ptr->packet_id()));
	}
	else if (packet_ptr->qos() == 2) {
		sender_agent_ptr->get_session_ptr()->add_packet_id_for_pubrel(packet_ptr->packet_id());
		ctx->write(std::make_shared<mqtt_pubrec>(packet_ptr->packet_id()));
	}

	// forwarding publish
	bool sent = false;
	try {
		topic_ptr t_ptr;
		{
			std::lock_guard<std::mutex> lock(_topic_mutex);
			t_ptr = _topics.at(packet_ptr->topic_name());
		}
		
		if (packet_ptr->retain()) {
			if (packet_ptr->payload().size() == 0)
				t_ptr->retained_message(nullptr); // 기존 retained message 삭제.
			else
				t_ptr->retained_message(packet_ptr); // 새 message로 교체.
		}
		sent = t_ptr->publish(packet_ptr); // payload 길이가 0인 retain message도 정상적으로 보내는 게 맞다.
	} catch (const std::exception&) {
		// no matching topic exists. do nothing.
	}

	{ // wild topic들 중에 matcing되는 topic들에게도 보낸다.
		std::lock_guard<std::mutex> lock(_topic_mutex);
		for (auto pair : _wild_topics) {
			if (topic::match(pair.first, packet_ptr->topic_name())) {
				sent = pair.second->publish(packet_ptr);
			}
		}
	}

	//... $sys 토픽으로의 publish는 위의 과정을 거치지 않고 미리 정의된 topic으로 바로 publish한다.
	// $sys 토픽은 따로 관리되고 그 토픽은 서버만 publish 할 수 있다. client는 $로 시작하는 토픽으로
	// publish를 하면 exception 처리되어 끊어지게 된다.

	if (!sent) { // no matching topic exists
//...		_trash_topic->handle_publish(ctx, packet_ptr);
	}
}

void mqtt_server::handle_puback(net::tcp_channel_context_base* ctx, std::shared_ptr<mqtt_puback> packet_ptr) {
	mqtt_client_ptr receiver_agent_ptr = std::static_pointer_cast<mqtt_client>(ctx->param());

	receiver_agent_ptr->handle_puback(packet_ptr);
}

void mqtt_server::handle_pubrec(net::tcp_channel_context_base* ctx, std::shared_ptr<mqtt_pubrec> packet_ptr) {
	mqtt_client_ptr receiver_agent_ptr = std::static_pointer_cast<mqtt_client>(ctx->param());

	receiver_agent_ptr->handle_pubrec(packet_ptr);
}

void mqtt_server::handle_pubrel(net::tcp_channel_context_base* ctx, std::shared_ptr<mqtt_pubrel> packet_ptr) {
	mqtt_client_ptr receiver_agent_ptr = std::static_pointer_cast<mqtt_client>(ctx->param());

	receiver_agent_ptr->handle_pubrel(packet_ptr);
}

void mqtt_server::handle_pubcomp(net::tcp_channel_context_base* ctx, std::shared_ptr<mqtt_pubcomp> packet_ptr) {
	mqtt_client_ptr receiver_agent_ptr = std::static_pointer_cast<mqtt_client>(ctx->param());

	receiver_agent_ptr->handle_pubcomp(packet_ptr);
}

void mqtt_server::handle_subscribe(net::tcp_channel_context_base* ctx, std::shared_ptr<mqtt_subscribe> packet_ptr) {
	mqtt_client_ptr sender_agent_ptr = std::static_pointer_cast<mqtt_client>(ctx->param());

	auto suback_ptr = std::make_shared<mqtt_suback>(packet_ptr->packet_id());

	std::vector<topic_ptr> subscribed_topics;
	std::vector<topic_ptr> subscribed_wild_topics;

	auto count = packet_ptr->topic_filters().size();
	for (auto inx = 0 ; inx < count ; inx++) {
		auto& topic_filter = packet_ptr->topic_filters()[inx];
		auto max_qos = packet_ptr->max_qoses()[inx];
		auto is_wild = packet_ptr->is_wild()[inx];

		topic_ptr t_ptr = get_topic_ptr_to_subscribe(topic_filter, is_wild);
		if (is_wild)
			subscribed_wild_topics.push_back(t_ptr); // for retained message.
		else
			subscribed_topics.push_back(t_ptr); // for retained message.

		// add client to topic.
		t_ptr->subscribe(sender_agent_ptr, max_qos);

		// add subscription to session.
		sender_agent_ptr->get_session_ptr()->add_subscription(topic_filter, max_qos, is_wild);

		// add return code
		suback_ptr->add_return_code(max_qos); // client가 요청한 qos 그대로 허용한다.
	}

	// send SUBACK first
	ctx->write(suback_ptr);

	// then send retained message
	publish_retained_message(ctx, subscribed_topics, subscribed_wild_topics);
}

void mqtt_server::handle_unsubscribe(net::tcp_channel_context_base* ctx, std::shared_ptr<mqtt_unsubscribe> packet_ptr) {
	mqtt_client_ptr sender_agent_ptr = std::static_pointer_cast<mqtt_client>(ctx->param());
	
	topic_ptr t_ptr;
	auto& topic_filters = packet_ptr->topic_filters();
	for (auto topic_filter : topic_filters) {
		{
			std::lock_guard<std::mutex> lock(_topic_mutex);
			try {
				t_ptr = _topics.at(topic_filter);
			} catch (const std::exception&) {
				try {
					t_ptr = _wild_topics.at(topic_filter);
				} catch (const std::exception&) {
					t_ptr = nullptr;
				}
			}
		}
		
		if (t_ptr) {
			t_ptr->unsubscribe(sender_agent_ptr);
			sender_agent_ptr->get_session_ptr()->delete_subscription(topic_filter);
		}
	}
	
	ctx->write(std::make_shared<mqtt_unsuback>(packet_ptr->packet_id()));
}

void mqtt_server::handle_pingreq(net::tcp_channel_context_base* ctx, std::shared_ptr<mqtt_pingreq> packet_ptr) {
	ctx->write(std::make_shared<mqtt_pingresp>());
}

void mqtt_server::handle_disconnect(net::tcp_channel_context_base* ctx, std::shared_ptr<mqtt_disconnect> packet_ptr) {
	mqtt_client_ptr sender_agent_ptr = std::static_pointer_cast<mqtt_client>(ctx->param());

	sender_agent_ptr->clear_will_message();
	
	if (sender_agent_ptr->clean_session()) { // client가 clean session이 설정된 경우 종료 전에 session을 삭제해준다.
		std::lock_guard<std::mutex> lock(_session_mutex);
		auto it = _sessions.find(sender_agent_ptr->client_id());
		_sessions.erase(it);
		
		sender_agent_ptr->set_session_ptr(nullptr);
	}
}
