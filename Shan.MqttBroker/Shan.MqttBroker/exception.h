//
//  exception.h
//  Shan.MqttBroker
//
//  Created by Sung Han Park on 2017. 4. 22..
//  Copyright © 2017 Sung Han Park. All rights reserved.
//

#ifndef exception_h
#define exception_h

#include <stdexcept>

class mqtt_error : public std::runtime_error {
public:
	explicit mqtt_error(const std::string& what) : std::runtime_error(what) {}
	explicit mqtt_error(const char* what) : std::runtime_error(what) {}
};

#endif /* exception_h */
