//
//  config.h
//  Shan.MqttBroker
//
//  Created by Sung Han Park on 2017. 4. 25..
//  Copyright © 2017년 Sung Han Park. All rights reserved.
//

#ifndef config_h
#define config_h

#include "spdlog/spdlog.h"
#include "constants.h"
#include "util/properties.h"

using namespace shan;

class config {
public:
	void handle_arg(int argc, const char * argv[]);
	void load();

	int worker_thread() const { return _worker_thread; }
	uint16_t port() const { return _port; }
	std::string protocol_name() const { return _protocol_name; }
	uint64_t connect_timeout() const { return _connect_timeout; }
	bool use_tls() const { return _use_tls; }
	std::string certificate() const { return _certificate; }
	std::string cert_key() const { return _cert_key; }
	std::string cert_password() const { return _cert_password; }
	bool authentication() const { return _authentication; }
	std::string username() const { return _username; }
	std::string password() const { return _password; }
	std::string adminname() const { return _adminname; }
	std::string adminpass() const { return _adminpass; }

	spdlog::level::level_enum log_level() const { return _log_level; }
	std::string log_path() const { return _log_path; }

	bool file_readable(const char* file_path);

private:
	std::string _config_path;

	int _worker_thread;
	uint16_t _port;
	std::string _protocol_name;
	uint64_t _connect_timeout;
	bool _use_tls;
	std::string _certificate;
	std::string _cert_key;
	std::string _cert_password;
	bool _authentication;
	std::string _username;
	std::string _password;
	std::string _adminname;
	std::string _adminpass;

	spdlog::level::level_enum _log_level;
	std::string _log_path;

	util::properties _properties;
};

#endif /* config_h */
