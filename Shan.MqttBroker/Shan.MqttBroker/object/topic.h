//
//  topic.h
//  Shan.MqttBroker
//
//  Created by Sung Han Park on 2017. 4. 25..
//  Copyright © 2017 Sung Han Park. All rights reserved.
//

#ifndef topic_h
#define topic_h

#include <map>
#include <memory>
#include "../constants.h"
#include "../packet/mqtt_publish.h"
#include "../player/mqtt_client.h"

using namespace shan;

/**
 topic은 다중 스레드에서 access하므로 적절히 동기화되어야 한다.
 */
class topic : public object {
public:
	topic(std::string topic_filter);

	bool empty();

	const std::string& topic_filter() const { return _topic_filter; }

	std::shared_ptr<mqtt_publish> retained_message();
	void retained_message(std::shared_ptr<mqtt_publish> retain_message);

	bool publish(net::tcp_server_base* server_p, std::shared_ptr<mqtt_publish> packet_ptr);
	void publish_retained(net::tcp_server_base* server_p, net::tcp_channel_context_base* ctx, std::shared_ptr<mqtt_publish> packet_ptr);

	void subscribe(mqtt_client_ptr client_ptr, uint8_t max_qos);
	void unsubscribe(mqtt_client_ptr client_ptr);
	
	static bool match(const std::string& topic_filter, const std::string& topic_name);

private:
	const std::string _topic_filter;
	std::shared_ptr<mqtt_publish> _retained_message; // atomic_xxx()로 보호된다.

	std::mutex _client_mutex;
	std::map<mqtt_client_ptr, uint8_t, mqtt_client::less> _clients;
};

using topic_ptr = std::shared_ptr<topic>;

#endif /* topic_h */
