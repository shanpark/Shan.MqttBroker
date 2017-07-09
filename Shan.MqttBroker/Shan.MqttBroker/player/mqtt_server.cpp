//
//  mqtt_server.cpp
//  Shan.MqttBroker
//
//  Created by Sung Han Park on 2017. 4. 24..
//  Copyright © 2017 Sung Han Park. All rights reserved.
//

#include "mqtt_server.h"
#include "../config.h"

using namespace shan;

mqtt_server::mqtt_server(net::tcp_server_base* tcp_server_p)
: _tcp_server_ptr(tcp_server_p), _trash_topic(std::make_shared<topic>("$sys/trash")) {
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

void mqtt_server::delete_client_ptr(std::string client_id) {
	std::lock_guard<std::mutex> lock(_client_mutex);
	_clients.erase(client_id);
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

void mqtt_server::delete_session_ptr(std::string client_id) {
	std::lock_guard<std::mutex> lock(_session_mutex);
	_sessions.erase(client_id);
}

bool mqtt_server::is_admin(const std::string& username, const std::vector<uint8_t>& password) {
	std::string pw(password.begin(), password.end());
	if ((username == g_cfg.adminname()) && (pw == g_cfg.adminpass()))
		return true;

	return false;
}

bool mqtt_server::authenticate(const std::string& username, const std::vector<uint8_t>& password) {
	if (g_cfg.username() == username) {
		std::string pw(password.begin(), password.end());
		if (g_cfg.password() == pw)
			return true;
	}
	return false;
}

void mqtt_server::reset_idle_timer(net::tcp_channel_context_base* ctx, uint32_t timer_id, uint64_t timeout) {
	net::tcp_idle_monitor* idle_monitor = static_cast<net::tcp_idle_monitor*>(_tcp_server_ptr->get_channel_handler_ptr(0).get());
	idle_monitor->reset_channel_timer(ctx->channel_id(), timer_id, timeout, nullptr);
}

topic_ptr mqtt_server::get_topic_ptr_to_subscribe(std::string topic_filter, bool is_wild) {
	topic_ptr t_ptr;
	
	if (is_wild) {
		std::lock_guard<std::mutex> lock(_topic_mutex);
		try {
			t_ptr = _wild_topics.at(topic_filter);
		} catch (const std::exception&) {
			t_ptr =  std::make_shared<topic>(topic_filter);
			_wild_topics.insert(std::make_pair(topic_filter, t_ptr));
		}
	}
	else {
		std::lock_guard<std::mutex> lock(_topic_mutex);
		try {
			t_ptr = _topics.at(topic_filter);
		} catch (const std::exception&) {
			t_ptr =  std::make_shared<topic>(topic_filter);
			_topics.insert(std::make_pair(topic_filter, t_ptr));
		}
	}
	
	return t_ptr;
}

/**
 subscribe를 수행한 topic들의 retained message를 client에게 publish한다. wild topic들은
 matching되는 일반 topic의 retained message를 모두 전송한다.
 @param ctx subscribe를 수행한 client의 channel context
 @param subscribed_topics client가 subscribe를 수행한 일반 topic(wild topic이 아닌)들의 벡터
 @param subscribed_wild_topics client가 subscribe를 수행한 wild topic들의 벡터
 */
void mqtt_server::publish_retained_message(net::tcp_channel_context_base* ctx, const std::vector<topic_ptr>& subscribed_topics, const std::vector<topic_ptr>& subscribed_wild_topics) {
	for (auto t_ptr : subscribed_topics)
		if (t_ptr->retained_message())
			t_ptr->publish_retained(_tcp_server_ptr.get(), ctx, t_ptr->retained_message()); // topic이 등록된 qos를 가지고 있기 때문에 topic을 통해서 보내야 한다.
	
	for (auto wt_ptr : subscribed_wild_topics) {
		std::lock_guard<std::mutex> lock(_topic_mutex);
		for (auto pair : _topics) {
			if (pair.second->retained_message() && topic::match(wt_ptr->topic_filter(), pair.second->topic_filter()))
				wt_ptr->publish_retained(_tcp_server_ptr.get(), ctx, pair.second->retained_message());
		}
	}
}

/**
 접속이 끊긴 client의 will message를 지정된 will topic으로 publish 한다. client의 will_flag가
 false인 경우 전송하지 않는다.
 */
void mqtt_server::publish_will_message(mqtt_client_ptr client_ptr) {
	if (client_ptr->will_flag()) {
		try {
			auto& topic_name = client_ptr->will_topic();
			auto qos = client_ptr->will_qos();
			auto retain = client_ptr->will_retain();

			topic_ptr t_ptr;
			{
				std::lock_guard<std::mutex> lock(_topic_mutex);
				t_ptr = _topics.at(topic_name);
			}

			if (t_ptr) {
				auto publish_ptr = std::make_shared<mqtt_publish>(false, qos, retain, topic_name, client_ptr->get_session_ptr()->new_packet_id(), client_ptr->will_message());
				t_ptr->publish(_tcp_server_ptr.get(), publish_ptr);
			}
		} catch (const std::exception&) {
			// no such topic exists.
		}
	}
}

void mqtt_server::unsubscribe_all(mqtt_client_ptr client_ptr) {
	auto& subscriptions = client_ptr->get_session_ptr()->subscriptions();
	for (auto sub_ptr : subscriptions) {
		topic_ptr t_ptr;
		try {
			std::lock_guard<std::mutex> lock(_topic_mutex);
			if (sub_ptr->is_wild()) {
				t_ptr = _wild_topics.at(sub_ptr->topic_filter());
				t_ptr->unsubscribe(client_ptr);
				if (t_ptr->empty())
					_wild_topics.erase(sub_ptr->topic_filter());
			}
			else {
				t_ptr = _topics.at(sub_ptr->topic_filter());
				t_ptr->unsubscribe(client_ptr);
				if (t_ptr->empty())
					_topics.erase(sub_ptr->topic_filter());
			}
		} catch (const std::exception&) {
			// no such topic.
		}
	}
}

void mqtt_server::handle_connect(net::tcp_channel_context_base* ctx, std::shared_ptr<mqtt_connect> packet_ptr) {
	// 첫 번째 CONNECT 패킷인지 검사. 아니면 끊는다.
	if (ctx->param()) // 이미 CONNECT를 받았으면 param이 지정되어 있을 것이다.
		ctx->close(); // 즉, 두 번째 CONNECT를 받았다는 뜻이다. 바로 끊어준다.

	// mqtt_client를 할당하고 context에 저장한다.
	auto client_ptr = std::make_shared<mqtt_client>(packet_ptr, ctx->channel_id());

	// CONNECT 패킷의 protocol 이름을 검사한다. 아니면 응답없이 바로 끊는다.
	if (packet_ptr->protocol_name() != g_cfg.protocol_name())
		ctx->close();

	// CONNECT 패킷의 무결성을 체크하고 아니라면 적당한 return_code를 넣어서 바로 CONNACK를 보낸다.
	uint8_t return_code = packet_ptr->check_integrity(this); // 패킷의 무결성을 먼저 검사한다.
	if (return_code != 0) { // 무결하지 않으면 응답하고 끊도록 한다.
		ctx->write(std::static_pointer_cast<object>(std::make_shared<mqtt_connack>(false, return_code)));
		return; // 응답 후 바로 리턴.
	}

	// 인증이 필요한 경우 사용자 이름, 비밀번호로 인증을 수행한다.
	if (is_admin(packet_ptr->username(), packet_ptr->password())) {
		client_ptr->admin(true); // $sys는 admin만 subscribe가 가능하다.
	}
	else {
		if (g_cfg.authentication()) {
			if (!authenticate(packet_ptr->username(), packet_ptr->password())) {
				return_code = 0x05; // not authorized user.
				ctx->write(std::static_pointer_cast<object>(std::make_shared<mqtt_connack>(false, return_code)));
				return;
			}
		}
	}

	// 이미 같은 client id를 갖는 client가 접속상태이면 기존 접속을 닫아버리고 새로운 client를 등록해주어야 한다. [MQTT-3.1.4-2].
	auto old_client_ptr = get_client_ptr(packet_ptr->client_id());
	if (old_client_ptr) {
		delete_client_ptr(packet_ptr->client_id()); // 기존 등록을 삭제해준다.

		auto old_ctx = _tcp_server_ptr->channel_context_of(old_client_ptr->channel_id());
		if (old_ctx) {
			old_ctx->param(nullptr);
			old_ctx->close();
		}
	}

	// CONNECT 패킷의 clean_session이 false이면 저장된 session을 복구해준다. 아니면 새로운 session을 할당해준다.
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
		}
	}
	else { // client_ptr->clean_session()이 true이면..
		delete_session_ptr(client_ptr->client_id()); // 혹시 전에 남겨두었을 session 정보를 지워줘야 한다. 그래야 정상적인 insert가 된다.

		auto new_s_ptr = std::make_shared<session>(); // alloc new session.
		reg_session_ptr(client_ptr->client_id(), new_s_ptr);
		client_ptr->set_session_ptr(new_s_ptr);
	}

	// new client 등록.
	reg_client_ptr(client_ptr);
	ctx->param(client_ptr); // 정상적인 로그인 과정이 완료되면 context에 param(mqtt_client_ptr)을 설정해준다.

	// connack with success code
	ctx->write(std::static_pointer_cast<object>(std::make_shared<mqtt_connack>(session_present, 0x00))); // success = 0x00

	// keep alive 설정.
	reset_idle_timer(ctx, ID_CLIENT_IDLE, packet_ptr->keep_alive() * 1500); // keep_alive x 1.5

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

		// must retry uncompleted qos1, qos2 messages.
		auto unacked_publishes = client_ptr->get_session_ptr()->unacked_publish_for_ack();
		for (auto pub_ptr : unacked_publishes) {
			pub_ptr->dup(true); // set dup.
			_tcp_server_ptr->write_channel(client_ptr->channel_id(), pub_ptr);
		}
		unacked_publishes = client_ptr->get_session_ptr()->unacked_publish_for_rec();
		for (auto pub_ptr : unacked_publishes) {
			pub_ptr->dup(true);
			_tcp_server_ptr->write_channel(client_ptr->channel_id(), pub_ptr);
		}
		auto unacked_pubrel_ids = client_ptr->get_session_ptr()->unacked_packet_id_for_pubcomp();
		for (auto rel_id : unacked_pubrel_ids) {
			auto rel_ptr = std::make_shared<mqtt_pubrel>(rel_id);
			_tcp_server_ptr->write_channel(client_ptr->channel_id(), rel_ptr);
		}

		// then send retained message
		publish_retained_message(ctx, subscribed_topics, subscribed_wild_topics);
	}
}

void mqtt_server::handle_publish(net::tcp_channel_context_base* ctx, std::shared_ptr<mqtt_publish> packet_ptr) {
	mqtt_client_ptr sender_agent_ptr = std::static_pointer_cast<mqtt_client>(ctx->param());
	if (!sender_agent_ptr)
		throw mqtt_error("First packet is not a CONNECT packet.");

	// receiver behavior.
	if (packet_ptr->qos() == QOS_AT_LEAST_ONCE) {
		ctx->write(std::make_shared<mqtt_puback>(packet_ptr->packet_id()));
	}
	else if (packet_ptr->qos() == QOS_EXACTLY_ONCE) {
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

		sent = t_ptr->publish(_tcp_server_ptr.get(), packet_ptr); // payload 길이가 0인 retain message도 정상적으로 보내는 게 맞다.
	} catch (const std::exception&) {
		// no matching topic exists. do nothing.
	}

	{ // wild topic들 중에 matcing되는 topic들에게도 보낸다.
		std::lock_guard<std::mutex> lock(_topic_mutex);
		for (auto pair : _wild_topics) {
			if (topic::match(pair.first, packet_ptr->topic_name())) {
				sent = pair.second->publish(_tcp_server_ptr.get(), packet_ptr);
			}
		}
	}

	// $sys 토픽으로의 publish는 위의 과정을 거치지 않고 미리 정의된 topic으로 서버가 바로 publish한다.
	// $sys 토픽은 따로 관리되고 그 토픽은 서버만 publish 할 수 있다. client는 $로 시작하는 토픽으로
	// publish를 하면 exception 처리되어 끊어지게 된다.

	if (!sent) { // no matching topic exists
		const auto& topic_name = packet_ptr->topic_name();
		std::vector<uint8_t> payload(topic_name.begin(), topic_name.end());
		auto pub_ptr = std::make_shared<mqtt_publish>(false, 1, false, _trash_topic->topic_filter(), 0xffff, payload);

		// publish된 메시지가 단 하나의 클라이언트에게도 전송되지 못하고 그냥 버려질 때 버려지는
		// publish 메시지의 topic name을 payload로 갖는 메시지가 $sys/trash 토픽으로
		// publish된다.
		_trash_topic->publish(_tcp_server_ptr.get(), pub_ptr);
	}
}

void mqtt_server::handle_puback(net::tcp_channel_context_base* ctx, std::shared_ptr<mqtt_puback> packet_ptr) {
	mqtt_client_ptr receiver_agent_ptr = std::static_pointer_cast<mqtt_client>(ctx->param());
	if (!receiver_agent_ptr)
		throw mqtt_error("First packet is not a CONNECT packet.");

	receiver_agent_ptr->handle_puback(packet_ptr);
}

void mqtt_server::handle_pubrec(net::tcp_channel_context_base* ctx, std::shared_ptr<mqtt_pubrec> packet_ptr) {
	mqtt_client_ptr receiver_agent_ptr = std::static_pointer_cast<mqtt_client>(ctx->param());
	if (!receiver_agent_ptr)
		throw mqtt_error("First packet is not a CONNECT packet.");

	receiver_agent_ptr->handle_pubrec(_tcp_server_ptr.get(), packet_ptr);
}

void mqtt_server::handle_pubrel(net::tcp_channel_context_base* ctx, std::shared_ptr<mqtt_pubrel> packet_ptr) {
	mqtt_client_ptr receiver_agent_ptr = std::static_pointer_cast<mqtt_client>(ctx->param());
	if (!receiver_agent_ptr)
		throw mqtt_error("First packet is not a CONNECT packet.");

	receiver_agent_ptr->handle_pubrel(_tcp_server_ptr.get(), packet_ptr);
}

void mqtt_server::handle_pubcomp(net::tcp_channel_context_base* ctx, std::shared_ptr<mqtt_pubcomp> packet_ptr) {
	mqtt_client_ptr receiver_agent_ptr = std::static_pointer_cast<mqtt_client>(ctx->param());
	if (!receiver_agent_ptr)
		throw mqtt_error("First packet is not a CONNECT packet.");

	receiver_agent_ptr->handle_pubcomp(packet_ptr);
}

void mqtt_server::handle_subscribe(net::tcp_channel_context_base* ctx, std::shared_ptr<mqtt_subscribe> packet_ptr) {
	mqtt_client_ptr sender_agent_ptr = std::static_pointer_cast<mqtt_client>(ctx->param());
	if (!sender_agent_ptr)
		throw mqtt_error("First packet is not a CONNECT packet.");

	auto suback_ptr = std::make_shared<mqtt_suback>(packet_ptr->packet_id());

	std::vector<topic_ptr> subscribed_topics;
	std::vector<topic_ptr> subscribed_wild_topics;

	auto count = packet_ptr->topic_filters().size();
	for (decltype(count) inx = 0 ; inx < count ; inx++) {
		auto& topic_filter = packet_ptr->topic_filters()[inx];
		auto max_qos = packet_ptr->max_qoses()[inx];
		auto is_wild = packet_ptr->is_wild()[inx];

		if (max_qos < QOS_INVALID) { // topic_filter가 invalid인 경우 max_qos는 0x80이 들어있다.
			if (topic_filter[0] == '$') {
				if (sender_agent_ptr->admin()) {
					if (topic_filter == "$sys/trash")
						_trash_topic->subscribe(sender_agent_ptr, QOS_EXACTLY_ONCE);
					else {
						LOGGER()->warn("sys topic '{}' does not exists.", topic_filter);
						max_qos = 0x80;
					}
				}
				else {
					LOGGER()->warn("Admin can only subscribe sys topics. [{} is not an admin client.]", sender_agent_ptr->client_id());
					max_qos = 0x80;
				}
			}
			else {
				topic_ptr t_ptr = get_topic_ptr_to_subscribe(topic_filter, is_wild);
				if (is_wild)
					subscribed_wild_topics.push_back(t_ptr); // for retained message.
				else
					subscribed_topics.push_back(t_ptr); // for retained message.

				// add client to topic.
				t_ptr->subscribe(sender_agent_ptr, max_qos);

				// add subscription to session.
				sender_agent_ptr->get_session_ptr()->add_subscription(topic_filter, max_qos, is_wild);
			}
		}
		else {
			max_qos = 0x80; // 이미 0x80이겠지만 혹시 나중에 다른 값들의 확장에 대비해서 다시 0x80으로 설정한다.
		}

		// add return code
		suback_ptr->add_return_code(max_qos); // topic_fileter가 valid한 경우 client가 요청한 qos 그대로 허용한다.
	}

	// send SUBACK first
	ctx->write(suback_ptr);

	// then send retained message
	publish_retained_message(ctx, subscribed_topics, subscribed_wild_topics);
}

void mqtt_server::handle_unsubscribe(net::tcp_channel_context_base* ctx, std::shared_ptr<mqtt_unsubscribe> packet_ptr) {
	mqtt_client_ptr sender_agent_ptr = std::static_pointer_cast<mqtt_client>(ctx->param());
	if (!sender_agent_ptr)
		throw mqtt_error("First packet is not a CONNECT packet.");

	topic_ptr t_ptr;
	auto count = packet_ptr->topic_filters().size();
	for (decltype(count) inx = 0 ; inx < count ; inx++) {
		auto& topic_filter = packet_ptr->topic_filters()[inx];
		auto is_wild = packet_ptr->is_wild()[inx];

		t_ptr = nullptr;
		try {
			std::lock_guard<std::mutex> lock(_topic_mutex);
			if (is_wild)
				t_ptr = _wild_topics.at(topic_filter);
			else
				t_ptr = _topics.at(topic_filter);
		} catch (const std::exception&) {
		}

		if (t_ptr) {
			t_ptr->unsubscribe(sender_agent_ptr);
			sender_agent_ptr->get_session_ptr()->delete_subscription(topic_filter);

			if (t_ptr->empty()) {
				std::lock_guard<std::mutex> lock(_topic_mutex);
				if (is_wild)
					_wild_topics.erase(topic_filter);
				else
					_topics.erase(topic_filter);
			}
		}
	}

	ctx->write(std::make_shared<mqtt_unsuback>(packet_ptr->packet_id()));
}

void mqtt_server::handle_pingreq(net::tcp_channel_context_base* ctx, std::shared_ptr<mqtt_pingreq> packet_ptr) {
	mqtt_client_ptr sender_agent_ptr = std::static_pointer_cast<mqtt_client>(ctx->param());
	if (!sender_agent_ptr)
		throw mqtt_error("First packet is not a CONNECT packet.");

	ctx->write(std::make_shared<mqtt_pingresp>());
}

void mqtt_server::handle_disconnect(net::tcp_channel_context_base* ctx, std::shared_ptr<mqtt_disconnect> packet_ptr) {
	mqtt_client_ptr sender_agent_ptr = std::static_pointer_cast<mqtt_client>(ctx->param());
	if (!sender_agent_ptr)
		throw mqtt_error("First packet is not a CONNECT packet.");

	ctx->param(nullptr); // DISCONNECTED를 받으면 바로 context로부터 param(mqtt_client_ptr)을 제거해준다.

	// clear session과 상관없이 항상 unsubscribe를 해준다. session이 복구되면 알아서 다시 subscribe할 것이다.
	unsubscribe_all(sender_agent_ptr);

	// client가 clean session이 설정된 경우 종료 전에 session을 삭제해준다.
	if (sender_agent_ptr->clean_session())
		delete_session_ptr(sender_agent_ptr->client_id());

	delete_client_ptr(sender_agent_ptr->client_id()); // mqtt_server로부터 삭제.

	reset_idle_timer(ctx, ID_CLIENT_DISCONNECTED, 10 * 1000); // 10초 내에 client가 끊지 않으면 서버가 끊도록 한다.
}

void mqtt_server::handle_channel_disconnected(net::tcp_channel_context_base* ctx) {
	mqtt_client_ptr client_agent_ptr = std::static_pointer_cast<mqtt_client>(ctx->param());
	if (client_agent_ptr) { // 아직 DISCONNECT 패킷을 받지 못한 상태에서 끊겼다는 뜻이다.
		LOGGER()->warn("A client was disconnected without the DISCONNECT packet.");

		// DISCONNECT를 받지 못한 상태에서 끊겼으므로 will message를 보내야 한다.
		publish_will_message(client_agent_ptr);

		// clear session과 상관없이 항상 unsubscribe를 해준다. session이 복구되면 알아서 다시 subscribe할 것이다.
		unsubscribe_all(client_agent_ptr);

		// client가 clean session이 설정된 경우 종료 전에 session을 삭제해준다.
		if (client_agent_ptr->clean_session())
			delete_session_ptr(client_agent_ptr->client_id());

		delete_client_ptr(client_agent_ptr->client_id()); // mqtt_server로부터 삭제.
	}
	else { // 이미 DISCONNECT 패킷을 받은 상태이면 ctx->param()이 제거된 상태가 된다. 또는 아직 CONNECT도 받지 못한 클라이언트가 끊겼다는 뜻이다.
		// 이미 DISCONNECT를 받은 상태이므로 접속 해제 처리는 이미 수행된 상태이다. 또는 아직 CONNECT도 못보냈으므로 해줄 건 없다.
	}
}

std::ostream& mqtt_server::str(std::ostream& os) const {
	os << "[Server Stat - Client:" << _clients.size() << " Topic:" << _topics.size() << " WildTopic:" << _wild_topics.size() << " Session:" << _sessions.size() << "]";
	return os;
}
