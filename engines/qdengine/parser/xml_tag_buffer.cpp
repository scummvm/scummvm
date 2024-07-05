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

#include "qdengine/qd_precomp.h"
#include "qdengine/parser/xml_tag_buffer.h"

namespace QDEngine {

namespace xml {

tag_buffer::tag_buffer(const tag &tg) : data_size_(tg.data_size() * tg.data_element_size()),
	data_offset_(0),
#ifdef _DEBUG
	data_format_(tg.data_format()),
#endif
	data_(tg.data()) {
}

tag_buffer::tag_buffer(const char *dp, int len) : data_size_(len),
	data_offset_(0),
#ifdef _DEBUG
	data_format_(tag::TAG_DATA_VOID),
#endif
	data_(dp) {
}

tag_buffer::tag_buffer(const tag_buffer &tb) : data_size_(tb.data_size_),
	data_offset_(tb.data_offset_),
#ifdef _DEBUG
	data_format_(tb.data_format_),
#endif
	data_(tb.data_) {
}

tag_buffer &tag_buffer::operator = (const tag_buffer &tb) {
	if (this == &tb) return *this;

	data_size_ = tb.data_size_;
	data_offset_ = tb.data_offset_;

#ifdef _DEBUG
	data_format_ = tb.data_format_;
#endif
	data_ = tb.data_;

	return *this;
}

tag_buffer::~tag_buffer() {
}

tag_buffer &tag_buffer::operator >= (short &var) {
	char *p;
	var = (short)strtol(data_ + data_offset_, &p, 0);
	data_offset_ += p - (data_ + data_offset_);

	return *this;
}

tag_buffer &tag_buffer::operator >= (unsigned short &var) {
	char *p;
	var = (unsigned short)strtoul(data_ + data_offset_, &p, 0);
	data_offset_ += p - (data_ + data_offset_);

	return *this;
}

tag_buffer &tag_buffer::operator >= (int &var) {
	char *p;
	var = (int)strtol(data_ + data_offset_, &p, 0);
	data_offset_ += p - (data_ + data_offset_);

	return *this;
}

tag_buffer &tag_buffer::operator >= (unsigned int &var) {
	char *p;
	var = (unsigned int)strtoul(data_ + data_offset_, &p, 0);
	data_offset_ += p - (data_ + data_offset_);

	return *this;
}

tag_buffer &tag_buffer::operator >= (float &var) {
	char *p;
	var = (float)strtod(data_ + data_offset_, &p);
	data_offset_ += p - (data_ + data_offset_);

	return *this;
}

}; /* namespace xml */

} // namespace QDEngine
