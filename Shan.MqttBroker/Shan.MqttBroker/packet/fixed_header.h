//
//  fixed_header.h
//  Shan.MqttBroker
//
//  Created by Sung Han Park on 2017. 4. 22..
//  Copyright © 2017 Sung Han Park. All rights reserved.
//

#ifndef fixed_header_h
#define fixed_header_h

#include "../constants.h"
#include <shan/util/streambuf.h>

class fixed_header : public shan::object {
public:
	fixed_header(shan::util::streambuf_ptr sb_ptr);
	fixed_header(packet_type type, uint8_t flags, uint32_t remaining_length);

	virtual void serialize(shan::util::streambuf_ptr sb_ptr) const;

	packet_type type() const { return _type; }
	uint32_t remaining_length() const { return _remaining_length; }

	static std::string decode_string(shan::util::streambuf_ptr sb_ptr);
	static void encode_string(const std::string& str, shan::util::streambuf_ptr sb_ptr);
	static std::vector<uint8_t> decode_binary(shan::util::streambuf_ptr sb_ptr);
	static void encode_binary(const std::vector<uint8_t>& bin, shan::util::streambuf_ptr sb_ptr);

	virtual std::ostream& str(std::ostream& os) const override;

protected:
	void decode_remaining_length(shan::util::streambuf_ptr sb_ptr);
	void encode_remaining_length(shan::util::streambuf_ptr sb_ptr) const;

protected:
	packet_type _type;
	uint8_t _flags;
	uint32_t _remaining_length; // 뒤 따라올 variable header, payload의 총 길이.
};

#endif /* fixed_header_h */
