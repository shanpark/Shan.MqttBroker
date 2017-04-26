//
//  fixed_header.cpp
//  Shan.MqttBroker
//
//  Created by Sung Han Park on 2017. 4. 22..
//  Copyright © 2017 Sung Han Park. All rights reserved.
//

#include "fixed_header.h"

using namespace shan::net;

fixed_header::fixed_header(shan::util::streambuf_ptr sb_ptr) {
	uint8_t byte = sb_ptr->read_uint8();
	_type = static_cast<packet_type>(byte >> 4);
	_flags = (byte & 0x0f);

	switch (_type) {
	case PUBREL:
	case SUBSCRIBE:
	case UNSUBSCRIBE:
		if (_flags != 0x02)
			throw mqtt_error("malformed packet. (fixed header flags bits)");
		break;

	case CONNECT:
	case CONNACK:
	case PUBACK:
	case PUBREC:
	case PUBCOMP:
	case SUBACK:
	case UNSUBACK:
	case PINGREQ:
	case PINGRESP:
	case DISCONNECT:
		if (_flags != 0x00)
			throw mqtt_error("malformed packet. (fixed header flags bits)");
		break;
	case PUBLISH:
		if ((_flags & MASK_QOS) == 0x06)
			throw mqtt_error("malformed packet. (fixed header flags bits)");
		break;
	}

	decode_remaining_length(sb_ptr);
}

fixed_header::fixed_header(packet_type type, uint8_t flags, uint32_t remaining_length)
: _type(type), _flags(flags), _remaining_length(remaining_length) {
	switch (_type) {
		case PUBREL:
		case SUBSCRIBE:
		case UNSUBSCRIBE:
			if (_flags != 0x02)
				throw mqtt_error("malformed packet. (fixed header flags bits)");
			break;

		case CONNECT:
		case CONNACK:
		case PUBACK:
		case PUBREC:
		case PUBCOMP:
		case SUBACK:
		case UNSUBACK:
		case PINGREQ:
		case PINGRESP:
		case DISCONNECT:
			if (_flags != 0x00)
				throw mqtt_error("malformed packet. (fixed header flags bits)");
			break;
		case PUBLISH:
			if ((_flags & MASK_QOS) == 0x06)
				throw mqtt_error("malformed packet. (fixed header flags bits)");
			break;
	}
}

void fixed_header::serialize(shan::util::streambuf_ptr sb_ptr) {
	uint8_t byte = ((_type << 4) | _flags);
	sb_ptr->write_uint8(byte);
	
	encode_remaining_length(sb_ptr);
}

std::string fixed_header::decode_string(shan::util::streambuf_ptr sb_ptr) {
	// read length
	std::size_t length = sb_ptr->read_uint16();

	// read string
	std::string result(length, '\0');
	if (sb_ptr->read(&result[0], length) < length)
		throw mqtt_error("not enough data. (decode_string)");

	return result;
}

void fixed_header::encode_string(const std::string& str, shan::util::streambuf_ptr sb_ptr) {
	std::size_t length = str.length();
	if (length > 0xffff)
		throw mqtt_error("too long string. (encode_string)");

	// write length
	sb_ptr->write_uint16(static_cast<uint16_t>(length));

	// write string
	sb_ptr->write(&str[0], length);
}

std::vector<uint8_t> fixed_header::decode_binary(shan::util::streambuf_ptr sb_ptr) {
	// read length
	std::size_t length = sb_ptr->read_uint16();

	// read binary data
	std::vector<uint8_t> result(length);
	if (sb_ptr->read(&result[0], length) < length)
		throw mqtt_error("not enough data. (decode_binary)");

	return result;
}

void fixed_header::encode_binary(const std::vector<uint8_t>& bin, shan::util::streambuf_ptr sb_ptr) {
	std::size_t length = bin.size();
	if (length > 0xffff)
		throw mqtt_error("too long binary. (encode_binary)");

	// write length
	sb_ptr->write_uint16(static_cast<uint16_t>(length));

	// write string
	sb_ptr->write(&bin[0], length);
}

void fixed_header::decode_remaining_length(shan::util::streambuf_ptr sb_ptr) {
	uint8_t encoded_byte;
	uint32_t length = 0;
	int shift = 0;

	do {
		encoded_byte = sb_ptr->read_uint8();
		length += ((encoded_byte & 0x7f) << shift);
		shift += 7;
		if (shift > 21)
			throw mqtt_error("malformed packet received. (malformed remaining length)");
	} while (encoded_byte & 0x80);

	_remaining_length = length;
}

void fixed_header::encode_remaining_length(shan::util::streambuf_ptr sb_ptr) {
	uint8_t encoded_byte;

	if (_remaining_length > 0x0FFFFFFF)
		throw mqtt_error("too big remaining length.");

	uint32_t length = _remaining_length;
	do {
		encoded_byte = length % 0x80;
		length >>= 7;
		if (length > 0)
			encoded_byte |= 0x80;
		sb_ptr->write_uint8(encoded_byte);
	} while (length > 0);
}
