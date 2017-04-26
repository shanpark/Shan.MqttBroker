//
//  config.h
//  Shan.MqttBroker
//
//  Created by Sung Han Park on 2017. 4. 25..
//  Copyright © 2017년 Sung Han Park. All rights reserved.
//

#ifndef config_h
#define config_h

class config {
public:
	config()
	: _need_auth(false) {}
	
	bool need_auth() const { return _need_auth; }

private:
	bool _need_auth;
};

#endif /* config_h */
