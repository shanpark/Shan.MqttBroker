//
//  mqtt_connect.h
//  Shan.MqttBroker
//
//  Created by Sung Han Park on 2017. 4. 22..
//  Copyright © 2017 Sung Han Park. All rights reserved.
//

#ifndef mqtt_connect_h
#define mqtt_connect_h

#include "fixed_header.h"
#include "client_id_generator_if.h"

enum connect_flag_mask : uint8_t {
	MASK_USERNAME      = 0x80,
	MASK_PASSWORD      = 0x40,
	MASK_WILL_RETAIN   = 0x20,
	MASK_WILL_QOS      = 0x18,
	MASK_WILL_FLAG     = 0x04,
	MASK_CLEAN_SESSION = 0x02
};

////////////////////////////////////////////////////////////////////////////////
// (:config) 접속 후 일정 시간(resonable time) 내에 CONNECT가 수신되어야 한다. 수신되지 않으면
// 끊어버려야 한다. CONNECT는 접속 클라이언트의 첫 번째 패킷이어야 하며 한 번만 수신되어야 한다.
// 두 번째 CONNECT가 수신되면 바로 접속을 끊어버린다. (spec 3.1)

class mqtt_connect : public fixed_header {
public:
	mqtt_connect(const fixed_header& fh, shan::util::streambuf_ptr sb_ptr);
	mqtt_connect(std::string protocol_name, uint8_t protocol_level, bool username_flag, bool password_flag, bool will_retain, uint8_t will_qos, bool will_flag, bool clean_session, uint16_t keep_alive, std::string client_id, std::string will_topic, std::vector<uint8_t> will_message, std::string username, std::vector<uint8_t> password);

	virtual void serialize(shan::util::streambuf_ptr sb_ptr) const override;

	uint8_t check_integrity(client_id_generator_if* generator);

	std::string protocol_name() const { return _protocol_name; }

	bool will_retain() const { return _will_retain; }
	uint8_t will_qos() const { return _will_qos; }
	bool will_flag() const { return _will_flag; }
	bool clean_session() const { return _clean_session; }
	uint16_t keep_alive() const { return _keep_alive; }

	std::string client_id() const { return _client_id; }

	std::string will_topic() const { return _will_topic; }
	std::vector<uint8_t> will_message() const { return _will_message; }
	std::string username() const { return _username; }
	std::vector<uint8_t> password() const { return _password; }

	virtual std::ostream& str(std::ostream& os) const override;

private:
	// variable header
	std::string _protocol_name; // 'MQTT'
	uint8_t _protocol_level; // 4 (spec 3.1.1)

	uint8_t _connect_flags; // connect flags
	bool _username_flag;    //
	bool _password_flag;    //
	bool _will_retain;      //
	uint8_t _will_qos;      //
	bool _will_flag;        //
	bool _clean_session;    //

	uint16_t _keep_alive; // keep alive time(ping interval)

	// payload
	std::string _client_id; // (mandatory) 1~23 bytes, [0-9a-bA-B]+, 길이 0인 문자열이면 server가 unique id를 할당해주고 clean_session은 0이어야 함.
							// 길이가 0이고 clean_session이 1이면 return code 0x02(Identifier rejected)로 CONNACK를 보내야한다. client_id가 기준 미달인 경우에도 0x02로 응답한다.
	std::string _will_topic; // _will_flag가 1인 경우에만
	std::vector<uint8_t> _will_message; // _will_flag가 1인 경우에만
	std::string _username; // _username이 1인 경우에만.
	std::vector<uint8_t> _password; // _password가 1인 경우에만.
};

#endif /* mqtt_connect_h */
