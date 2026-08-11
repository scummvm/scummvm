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

#include "qdengine/qd_fwd.h"
#include "qdengine/parser/xml_tag_buffer.h"

namespace QDEngine {

namespace xml {

tag_buffer::tag_buffer(const tag &tg) : _data_size(tg.data_size() * tg.data_element_size()),
	_data_offset(0),
#ifdef _DEBUG
	data_format_(tg.data_format()),
#endif
	_data(tg.data()) {
}

tag_buffer::tag_buffer(const char *dp, int len) : _data_size(len),
	_data_offset(0),
#ifdef _DEBUG
	data_format_(tag::TAG_DATA_VOID),
#endif
	_data(dp) {
}

tag_buffer::tag_buffer(const tag_buffer &tb) : _data_size(tb._data_size),
	_data_offset(tb._data_offset),
#ifdef _DEBUG
	data_format_(tb.data_format_),
#endif
	_data(tb._data) {
}

tag_buffer &tag_buffer::operator = (const tag_buffer &tb) {
	if (this == &tb) return *this;

	_data_size = tb._data_size;
	_data_offset = tb._data_offset;

#ifdef _DEBUG
	data_format_ = tb.data_format_;
#endif
	_data = tb._data;

	return *this;
}

tag_buffer::~tag_buffer() {
}

tag_buffer &tag_buffer::operator >= (int16 &var) {
	char *p;
	var = (short)strtol(_data + _data_offset, &p, 0);
	_data_offset += p - (_data + _data_offset);

	return *this;
}

tag_buffer &tag_buffer::operator >= (uint16 &var) {
	char *p;
	var = (uint16)strtoul(_data + _data_offset, &p, 0);
	_data_offset += p - (_data + _data_offset);

	return *this;
}

tag_buffer &tag_buffer::operator >= (int &var) {
	char *p;
	var = (int)strtol(_data + _data_offset, &p, 0);
	_data_offset += p - (_data + _data_offset);

	return *this;
}

tag_buffer &tag_buffer::operator >= (uint32 &var) {
	char *p;
	var = (uint32)strtoul(_data + _data_offset, &p, 0);
	_data_offset += p - (_data + _data_offset);

	return *this;
}

tag_buffer &tag_buffer::operator >= (float &var) {
	char *p;
	var = (float)strtod(_data + _data_offset, &p);
	_data_offset += p - (_data + _data_offset);

	return *this;
}

} /* namespace xml */

} // namespace QDEngine
