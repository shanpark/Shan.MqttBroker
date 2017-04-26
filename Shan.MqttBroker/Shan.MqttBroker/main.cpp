//
//  main.cpp
//  Shan.MqttBroker
//
//  Created by Sung Han Park on 2017. 4. 22..
//  Copyright © 2017 Sung Han Park. All rights reserved.
//

#include "constants.h"
#include "handler/mqtt_codec.h"
#include "handler/packet_handler.h"
#include "player/mqtt_server.h"

using namespace shan;

config g_cfg;
net::tcp_server_base* g_server_p; // singleton of server. life ends with the run().

void run() {
	// (:config) config.use_tls()
	g_server_p = new net::tcp_server(/*config.thread_count()*/);
//	g_server_p = new net::ssl_server(shan::net::TLSV12_SERVER/*, config.thread_count()*/);

	mqtt_server server(g_server_p); // server will release g_server_p.

	g_server_p->add_channel_handler(new net::tcp_idle_monitor(ID_NO_CONNECT, 30 * 1000 /*config.connect_timeout()*/, nullptr)); // (:config) connect timeout
	g_server_p->add_channel_handler(new mqtt_codec());
	g_server_p->add_channel_handler(new packet_handler(&server));
	g_server_p->start(1883); // (:config) port number

	g_server_p->wait_stop();
}

int main(int argc, const char * argv[]) {

	run();
}
