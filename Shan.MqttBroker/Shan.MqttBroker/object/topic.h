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
#include "../constants.h"
#include "../packet/mqtt_publish.h"
#include "../player/mqtt_client.h"

using namespace shan;

class topic : public object {
public:
	topic(std::string topic_filter);

	static bool match(const std::string& topic_filter, const std::string& topic_name);

	void publish(std::shared_ptr<mqtt_publish> packet_ptr);
	
private:
	std::string _topic_filter;
	std::map<mqtt_client_wptr, uint8_t> _clients; //... 중복 클라이언트 처리를 위해서 less than, equal을 구현해야 한다.
};

using topic_ptr = std::shared_ptr<topic>;

#endif /* topic_h */
