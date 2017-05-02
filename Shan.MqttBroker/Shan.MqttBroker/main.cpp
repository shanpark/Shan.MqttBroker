//
//  main.cpp
//  Shan.MqttBroker
//
//  Created by Sung Han Park on 2017. 4. 22..
//  Copyright © 2017 Sung Han Park. All rights reserved.
//

#include <iostream>
#include <csignal>
#include "net/net_ssl.h"
#include "constants.h"
#include "config.h"
#include "handler/mqtt_codec.h"
#include "handler/packet_handler.h"
#include "player/mqtt_server.h"

using namespace shan;

config g_cfg;

static net::tcp_server_base* g_service_p;
extern "C" void sig_handler(int sig) {
	g_service_p->request_stop();
}

void load_config(int argc, const char * argv[]) {
	g_cfg.handle_arg(argc, argv);
	g_cfg.load();
}

/**
 spdlog의 기본 파일 이름 생성 규칙이 맘에 들지 않아서 custom 이름 생성자를 따로 구현하였다.
 YYYY-MM-DD.log 접미사가 붙는 규칙이다.
 */
struct custom_file_name_calculator {
	// Create filename for the form basename_YYYY-MM-DD.log
	static spdlog::filename_t calc_filename(const spdlog::filename_t& basename) {
		std::tm tm = spdlog::details::os::localtime();
		std::conditional<std::is_same<spdlog::filename_t::value_type, char>::value, fmt::MemoryWriter, fmt::WMemoryWriter>::type w;
		w.write(SPDLOG_FILENAME_T("{}_{:04d}-{:02d}-{:02d}.log"), basename, tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday);
		return w.str();
	}
};

void setup_logger() {
	std::vector<spdlog::sink_ptr> sinks;
	sinks.push_back(std::make_shared<spdlog::sinks::stdout_sink_mt>());

	try {
		sinks.push_back(std::make_shared<spdlog::sinks::daily_file_sink<std::mutex, custom_file_name_calculator>>(g_cfg.log_path(), 0, 0));
	} catch (const spdlog::spdlog_ex& e) {
		std::cout << "spdlog initialization failed: " << e.what() << std::endl;
	}

	auto combined_logger = std::make_shared<spdlog::logger>("logger", begin(sinks), end(sinks));
	combined_logger->flush_on(spdlog::level::info);

	spdlog::register_logger(combined_logger);
	spdlog::set_pattern("[%C/%m/%d %T.%e][%L] %v"); // 설정값에 따라 조정되어야 함.
	spdlog::set_level(g_cfg.log_level()); // new log level from config file.
}

static std::string get_password(std::size_t max_length, net::ssl_password_purpose purpose) {
	return g_cfg.cert_password();
}

void run() {
	net::tcp_server_base* server_p;

	if (g_cfg.use_tls()) {
		net::ssl_server* ssl_server_p = new net::ssl_server(shan::net::TLSV12_SERVER, g_cfg.worker_thread());
		ssl_server_p->set_options(net::DEF_OPT | net::SINGLE_DH_USE | net::NO_SSLV2 | net::NO_SSLV3);
		ssl_server_p->set_password_callback(get_password);
		ssl_server_p->use_certificate_chain_file(g_cfg.certificate());
		ssl_server_p->use_private_key_file(g_cfg.cert_key(), net::PEM);

		server_p = ssl_server_p;
	}
	else {
		server_p = new net::tcp_server(g_cfg.worker_thread());
	}

	mqtt_server server(server_p); // server will release g_server_p.

	server_p->add_channel_handler(new net::tcp_idle_monitor(ID_NO_CONNECT, g_cfg.connect_timeout(), nullptr));
	server_p->add_channel_handler(new mqtt_codec());
	server_p->add_channel_handler(new packet_handler(&server));
	server_p->start(g_cfg.port()); // port number from config.

	g_service_p = server_p;
	std::signal(SIGINT, sig_handler);

	LOGGER()->info("Shan.MqttBroker is started.");

	server_p->wait_stop();
}

int main(int argc, const char * argv[]) {
	load_config(argc, argv);

	setup_logger();

	run();

	LOGGER()->info("Shan.MqttBroker stopped.");
}
