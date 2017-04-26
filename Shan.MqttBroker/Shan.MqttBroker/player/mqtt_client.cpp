//
//  mqtt_client.cpp
//  Shan.MqttBroker
//
//  Created by Sung Han Park on 2017. 4. 25..
//  Copyright © 2017 Sung Han Park. All rights reserved.
//

#include "mqtt_client.h"

mqtt_client::mqtt_client(std::shared_ptr<mqtt_connect> packet_ptr, std::size_t channel_id)
: _channel_id(channel_id), _client_id(packet_ptr->client_id()), _stat(CL_CREATED)
, _will_flag(packet_ptr->will_flag()), _will_qos(packet_ptr->will_qos()), _will_retain(packet_ptr->will_retain()), _clean_session(packet_ptr->clean_session())
, _will_topic(packet_ptr->will_topic()), _will_message(packet_ptr->will_message()) {
}
