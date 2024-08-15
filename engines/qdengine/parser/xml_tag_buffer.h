/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef QDENGINE_PARSER_XML_TAG_BUFFER_H
#define QDENGINE_PARSER_XML_TAG_BUFFER_H

#include "common/endian.h"
#include "qdengine/parser/xml_tag.h"

#ifdef _DEBUG
#define XML_ASSERT(a) assert(a)
#else
#define XML_ASSERT(a)
#endif

namespace QDEngine {

namespace xml {

class tag_buffer {
public:
	tag_buffer(const tag &tg);
	tag_buffer(const char *dp, int len);
	tag_buffer(const tag_buffer &tb);
	~tag_buffer();

	tag_buffer &operator = (const tag_buffer &tb);

	tag_buffer &operator >= (int16 &var);
	tag_buffer &operator >= (uint16 &var);
	tag_buffer &operator >= (int &var);
	tag_buffer &operator >= (uint32 &var);
	tag_buffer &operator >= (float &var);

	tag_buffer &operator > (int16 &var) {
		XML_ASSERT(data_format_ == tag::TAG_DATA_VOID || data_format_ == tag::TAG_DATA_SHORT);
		var = (int16)READ_LE_UINT16(_data + _data_offset);
		_data_offset += sizeof(int16);

		return *this;
	}
	tag_buffer &operator > (uint16 &var) {
		XML_ASSERT(data_format_ == tag::TAG_DATA_VOID || data_format_ == tag::TAG_DATA_UNSIGNED_SHORT);
		var = READ_LE_UINT16(_data + _data_offset);
		_data_offset += sizeof(uint16);

		return *this;
	}
	tag_buffer &operator > (int &var) {
		XML_ASSERT(data_format_ == tag::TAG_DATA_VOID || data_format_ == tag::TAG_DATA_INT);
		var = (int32)READ_LE_UINT32(_data + _data_offset);
		_data_offset += sizeof(int32);

		return *this;
	}
	tag_buffer &operator > (uint32 &var) {
		XML_ASSERT(data_format_ == tag::TAG_DATA_VOID || data_format_ == tag::TAG_DATA_UNSIGNED_INT);
		var = READ_LE_UINT32(_data + _data_offset);
		_data_offset += sizeof(uint32);

		return *this;
	}
	tag_buffer &operator > (float &var) {
		XML_ASSERT(data_format_ == tag::TAG_DATA_VOID || data_format_ == tag::TAG_DATA_FLOAT);
		var = READ_LE_FLOAT32(_data + _data_offset);
		_data_offset += 4;

		return *this;
	}

	int16 get_short() {
		XML_ASSERT(data_format_ == tag::TAG_DATA_VOID || data_format_ == tag::TAG_DATA_SHORT);
		int16 v;
		*this > v;
		return v;
	}
	uint16 get_ushort() {
		XML_ASSERT(data_format_ == tag::TAG_DATA_VOID || data_format_ == tag::TAG_DATA_UNSIGNED_SHORT);
		uint16 v;
		*this > v;
		return v;
	}
	int get_int() {
		XML_ASSERT(data_format_ == tag::TAG_DATA_VOID || data_format_ == tag::TAG_DATA_INT);
		int v;
		*this > v;
		return v;
	}
	uint32 get_uint() {
		XML_ASSERT(data_format_ == tag::TAG_DATA_VOID || data_format_ == tag::TAG_DATA_UNSIGNED_INT);
		uint32 v;
		*this > v;
		return v;
	}
	float get_float() {
		XML_ASSERT(data_format_ == tag::TAG_DATA_VOID || data_format_ == tag::TAG_DATA_FLOAT);
		float v;
		*this > v;
		return v;
	}

	bool end_of_storage() const {
		return _data_size > _data_offset;
	}
	void reset() {
		_data_offset = 0;
	}

private:
	int _data_size;
	int _data_offset;

#ifdef _DEBUG
	tag::tag_data_format data_format_;
#endif

	const char *_data;
};

} /* namespace xml */

} // namespace QDEngine

#endif // QDENGINE_PARSER_XML_TAG_BUFFER_H
