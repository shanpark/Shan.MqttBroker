//
//  session.h
//  Shan.MqttBroker
//
//  Created by Sung Han Park on 2017. 4. 25..
//  Copyright © 2017 Sung Han Park. All rights reserved.
//

#ifndef session_h
#define session_h

#include "../constants.h"
#include "subscription.h"

using namespace shan;

class session : public object {

private:
	std::vector<subscription_ptr> _subscriptions;
};

using session_ptr = std::shared_ptr<session>;

#endif /* session_h */
