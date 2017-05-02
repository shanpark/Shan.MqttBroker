//
//  config.cpp
//  Shan.MqttBroker
//
//  Created by Sung Han Park on 2017. 4. 25..
//  Copyright © 2017년 Sung Han Park. All rights reserved.
//

#include <fstream>
#include "constants.h"
#include "config.h"

void config::handle_arg(int argc, const char * argv[]) {
	for (auto inx = 1 ; inx < argc ; inx++) {
		if (std::strcmp(argv[inx], "-f") == 0) {
			if ((inx < argc) && file_readable(argv[++inx]))
				_config_path = argv[inx];
			else
				std::cerr << argv[inx] << " is not readable." << std::endl;
		}
		else {
			std::cerr << "Unknown option: " << argv[inx] << std::endl;
		}
	}
}

void config::load() {
	if (_config_path.empty())
		_config_path = DEFAULT_CONFIG_FILE;

	try {
		std::ifstream ifs(_config_path.c_str());
		_properties.load(ifs);
	} catch (const std::exception& e) {
		std::cerr << "An error occurred while loading the config file. Server will use default values. [" << e.what() << "]" << std::endl;
		return;
	}

	std::string value;
	try {
		value = _properties.get("worker_thread");
		_worker_thread = std::atoi(value.c_str());
	} catch (const std::exception&) {
		_worker_thread = 4;
	}

	try {
		value = _properties.get("port");
		_port = std::atoi(value.c_str());
	} catch (const std::exception&) {
		_port = 0;
	}

	try {
		value = _properties.get("protocol_name");
		_protocol_name = value;
	} catch (const std::exception&) {
		_protocol_name = "MQTT";
	}

	try {
		value = _properties.get("connect_timeout");
		_connect_timeout = std::strtoll(value.c_str(), nullptr, 10);
	} catch (const std::exception&) {
		_connect_timeout = 30000;
	}

	try {
		value = _properties.get("use_tls");
		_use_tls = (value == "true") ? true : false;
	} catch (const std::exception&) {
		_use_tls = false;
	}

	try {
		value = _properties.get("certificate");
		_certificate = value;
	} catch (const std::exception&) {
		_certificate = "mqttd.pem";
	}

	try {
		value = _properties.get("cert_key");
		_cert_key = value;
	} catch (const std::exception&) {
		_cert_key = "mqttd.pem";
	}

	try {
		value = _properties.get("cert_password");
		_cert_password = value;
	} catch (const std::exception&) {
		_cert_password = "";
	}

	try {
		value = _properties.get("authentication");
		_authentication = (value == "true") ? true : false;
	} catch (const std::exception&) {
		_authentication = false;
	}

	try {
		value = _properties.get("username");
		_username = value;
	} catch (const std::exception&) {
		_username = "";
	}

	try {
		value = _properties.get("password");
		_password = value;
	} catch (const std::exception&) {
		_password = "";
	}

	try {
		value = _properties.get("adminname");
		_adminname = value;
	} catch (const std::exception&) {
		_adminname = "mqadmin";
	}

	try {
		value = _properties.get("adminpass");
		_adminpass = value;
	} catch (const std::exception&) {
		_adminpass = "mqadmin";
	}

	try {
		value = _properties.get("log_path");
		_log_path = value;
	} catch (const std::exception&) {
		_log_path = "mqttd";
	}

	try {
		value = _properties.get("log_level");
		if (value == "trace")
			_log_level = spdlog::level::trace;
		else if (value == "debug")
			_log_level = spdlog::level::debug;
		else if (value == "info")
			_log_level = spdlog::level::info;
		else if (value == "warn")
			_log_level = spdlog::level::warn;
		else if (value == "error")
			_log_level = spdlog::level::err;
		else if (value == "critical")
			_log_level = spdlog::level::critical;
		else if (value == "off")
			_log_level = spdlog::level::off;
	} catch (const std::exception&) {
		_log_level = spdlog::level::info;
	}

	// default port number set
	if (port() == 0) {
		if (use_tls())
			_port = 8883;
		else
			_port = 1883;
	}

	if (use_tls()) {
		if (!file_readable(g_cfg.certificate().c_str())) {
			std::cerr << "Certificate file is not readable [" << g_cfg.certificate() << "]" << std::endl;
			_use_tls = false;
		}
		else if (!file_readable(g_cfg.cert_key().c_str())) {
			std::cerr << "Cert Key file is not readable [" << g_cfg.cert_key() << "]" << std::endl;
			_use_tls = false;
		}
	}
}

bool config::file_readable(const char* file_path) {
	auto fp = std::fopen(file_path, "r");
	if (fp) {
		std::fclose(fp);
		return true;
	}

	return false;
}
