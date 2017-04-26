//
//  client_id_generator_if.h
//  Shan.MqttBroker
//
//  Created by Sung Han Park on 2017. 4. 22..
//  Copyright © 2017 Sung Han Park. All rights reserved.
//

#ifndef client_id_generator_if_h
#define client_id_generator_if_h

class client_id_generator_if {
public:
	virtual std::string generate_unique_id() = 0;
};

#endif /* client_id_generator_if_h */
