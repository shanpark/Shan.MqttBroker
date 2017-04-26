//
//  client_checker_if.h
//  Shan.MqttBroker
//
//  Created by Sung Han Park on 2017. 4. 22..
//  Copyright © 2017 Sung Han Park. All rights reserved.
//

#ifndef client_checker_if_h
#define client_checker_if_h

class client_checker_if {
public:
	virtual bool authorized(std::string username, std::vector<uint8_t> password) = 0;
};

#endif /* client_checker_if_h */
