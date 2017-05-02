//
//  mqtt_connect.cpp
//  Shan.MqttBroker
//
//  Created by Sung Han Park on 2017. 4. 22..
//  Copyright © 2017 Sung Han Park. All rights reserved.
//

#include <cctype>
#include "mqtt_connect.h"

mqtt_connect::mqtt_connect(const fixed_header& fh, shan::util::streambuf_ptr sb_ptr)
: fixed_header(fh) {
	////////////////////////////////////////////////////////////////////////////
	// variable header

	// protocol name
	_protocol_name = decode_string(sb_ptr);

	// protocol level
	_protocol_level = sb_ptr->read_uint8();

	// connect flags
	_connect_flags = sb_ptr->read_uint8();
	_username_flag = ((_connect_flags & MASK_USERNAME) != 0);
	_password_flag = ((_connect_flags & MASK_PASSWORD) != 0);
	_will_retain = ((_connect_flags & MASK_WILL_RETAIN) != 0);
	_will_qos = (_connect_flags & MASK_WILL_QOS) >> 3;
	_will_flag = ((_connect_flags & MASK_WILL_FLAG) != 0);
	_clean_session = ((_connect_flags & MASK_CLEAN_SESSION) != 0);

	if ((_connect_flags & 0x01) != 0) // bit 0 of connect flags is reserved. must be zero.
		throw mqtt_error("malformed packet received. (CONNECT - connect flags)");
	if (!_will_flag) {
		if ((_will_qos != 0) || (_will_retain))
			throw mqtt_error("malformed packet received. (CONNECT - connect flags - will flag, will qos, will_retain)");
	}
	if (_will_qos >= QOS_INVALID)
		throw mqtt_error("malformed packet received. (CONNECT - connect flags - will QoS)");

	// keep alive (A Keep Alive value of zero(0) has the effect of turning off the keep alive mechanism)
	_keep_alive = sb_ptr->read_uint16();

	////////////////////////////////////////////////////////////////////////////
	// payload

	// client id
	_client_id = decode_string(sb_ptr);

	if (_will_flag) {
		_will_topic = decode_string(sb_ptr);
		_will_message = decode_binary(sb_ptr);
	}

	if (_username_flag)
		_username = decode_string(sb_ptr);
	if (_password_flag)
		_password = decode_binary(sb_ptr);
}

mqtt_connect::mqtt_connect(std::string protocol_name, uint8_t protocol_level, bool username_flag, bool password_flag, bool will_retain, uint8_t will_qos, bool will_flag, bool clean_session,
						   uint16_t keep_alive, std::string client_id, std::string will_topic, std::vector<uint8_t> will_message, std::string username, std::vector<uint8_t> password)
: fixed_header(CONNECT, 0, 0), _protocol_name(protocol_name), _protocol_level(protocol_level), _username_flag(username_flag), _password_flag(password_flag), _will_retain(will_retain), _will_qos(will_qos), _will_flag(will_flag)
, _clean_session(clean_session), _keep_alive(keep_alive), _client_id(client_id), _will_topic(will_topic), _will_message(will_message), _username(username), _password(password) {
	_remaining_length += static_cast<uint32_t>(_protocol_name.length() + 2); // protocol name
	_remaining_length += 1; // protocol level

	if (!_will_flag) {
		if ((_will_qos != 0) || (_will_retain))
			throw mqtt_error("malformed packet. (CONNECT - connect flags - will flag, will qos, will_retain)");
	}
	if (_will_qos >= QOS_INVALID)
		throw mqtt_error("malformed packet. (CONNECT - connect flags - will QoS)");

	_connect_flags = 0;
	if (_username_flag)
		_connect_flags |= MASK_USERNAME;
	if (_password_flag)
		_connect_flags |= MASK_PASSWORD;
	if (_will_retain)
		_connect_flags |= MASK_WILL_RETAIN;
	_connect_flags |= ((_will_qos << 3) & MASK_WILL_QOS);
	if (_clean_session)
		_connect_flags |= MASK_CLEAN_SESSION;

	_remaining_length += 1; // connect flag
	_remaining_length += 2; // keep_alive

	_remaining_length += static_cast<uint32_t>(2 + _client_id.length());

	if (_will_flag) {
		_remaining_length += static_cast<uint32_t>(2 + will_topic.length());
		_remaining_length += static_cast<uint32_t>(2 + will_message.size());
	}

	if (_username_flag)
		_remaining_length += static_cast<uint32_t>(2 + _username.length());
	if (_password_flag)
		_remaining_length += static_cast<uint32_t>(2 + _password.size());
}

void mqtt_connect::serialize(shan::util::streambuf_ptr sb_ptr) const {
	fixed_header::serialize(sb_ptr);

	encode_string(_protocol_name, sb_ptr);

	sb_ptr->write_uint8(_protocol_level);

	sb_ptr->write_uint8(_connect_flags);

	sb_ptr->write_uint16(_keep_alive);

	encode_string(_client_id, sb_ptr);

	if (_will_flag) {
		encode_string(_will_topic, sb_ptr);
		encode_binary(_will_message, sb_ptr);
	}

	if (_username_flag)
		encode_string(_username, sb_ptr);
	if (_password_flag)
		encode_binary(_password, sb_ptr);
}

uint8_t mqtt_connect::check_integrity(client_id_generator_if* generator) {
	if (_protocol_level != 4)
		return 0x01; // unsupported protocol level

	if (_client_id.length() > 23)
		return 0x02; // Identifier rejected
	for (char ch : _client_id) {
		if (!std::isalnum(ch))
			return 0x02; // Identifier rejected
	}
	if (_client_id.length() == 0) { // special case.
		if (!_clean_session)
			return 0x02; // Identifier rejected

		_client_id = generator->generate_unique_id(); // server should assign an unique client id.
	}

	if (!_username_flag && _password_flag)
		return 0x04; // username, password malformed.

	// 인증 실패는 0x05이다. 이 검사는 packet 로직을 처리하는 곳에서 반드시 수행해야 한다.
	// 오래 걸리는 작업일 수도 있으므로 여기서는 integrity만 검사한다.

	return 0x00; // integrity OK.
}

std::ostream& mqtt_connect::str(std::ostream& os) const {
	fixed_header::str(os);

	os << " " << _protocol_name << " " << static_cast<int>(_protocol_level);
	os << ((_username_flag) ? " UN:1" : " UN:0")
	   << ((_password_flag) ? " PW:1" : " PW:0")
	   << ((_will_retain) ? " WR:1" : " WR:0")
	   << " WQoS:" << static_cast<int>(_will_qos)
	   << ((_will_flag) ? " WF:1" : " WF:0")
	   << ((_clean_session) ? " CS:1" : " CS:0")
	   << " KA:" << _keep_alive
	   << " CID:" <<  _client_id;

	return os;
}
