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
#include "qdengine/parser/qdscr_parser.h"
#include "qdengine/qdcore/qd_condition_data.h"


namespace QDEngine {

qdConditionData::qdConditionData() : _type(DATA_STRING) {
}

qdConditionData::qdConditionData(data_t data_type, int data_size) : _type(data_type) {
	if (data_size)
		alloc_data(data_size);
}

qdConditionData::qdConditionData(const qdConditionData &data) : _type(data._type),
	_data(data._data) {
}

qdConditionData::~qdConditionData() {
}

qdConditionData &qdConditionData::operator = (const qdConditionData &data) {
	if (this == &data) return *this;

	_type = data._type;
	_data = data._data;

	return *this;
}

bool qdConditionData::alloc_data(int size) {
	switch (_type) {
	case DATA_INT:
		size *= sizeof(int);
		break;
	case DATA_FLOAT:
		size *= sizeof(float);
		break;
	case DATA_STRING:
		size++;
		break;
	}

	if ((int)_data.size() < size)
		_data.resize(size);

	return true;
}

bool qdConditionData::load_script(const xml::tag *p) {
	switch (_type) {
	case DATA_INT: {
		xml::tag_buffer buf(*p);
		for (int i = 0; i < p->data_size(); i++)
			put_int(buf.get_int(), i);
	}
	break;
	case DATA_FLOAT: {
		xml::tag_buffer buf(*p);
		for (int i = 0; i < p->data_size(); i++)
			put_float(buf.get_float(), i);
	}
	break;
	case DATA_STRING:
		put_string(p->data());
		break;
	}

	return true;
}

bool qdConditionData::save_script(Common::WriteStream &fh, int indent) const {
	for (int i = 0; i < indent; i++) {
		fh.writeString("\t");
	}

	switch (_type) {
	case DATA_INT:
		fh.writeString(Common::String::format("<condition_data_int>%lu", _data.size() / sizeof(int32)));
		for (uint i = 0; i < _data.size() / sizeof(int32); i++) {
			fh.writeString(Common::String::format(" %d", get_int(i)));
		}
		fh.writeString("</condition_data_int>\r\n");
		break;
	case DATA_FLOAT:
		fh.writeString(Common::String::format("<condition_data_float>%lu", _data.size() / sizeof(float)));
		for (uint i = 0; i < _data.size() / sizeof(float); i++) {
			fh.writeString(Common::String::format(" %f", get_float(i)));
		}
		fh.writeString("</condition_data_float>\r\n");
		break;
	case DATA_STRING:
		fh.writeString("<condition_data_string>");
		if (!_data.empty()) {
			fh.writeString(Common::String::format("%s", qdscr_XML_string(&*_data.begin())));
		}
		fh.writeString("</condition_data_string>\r\n");
		break;
	}

	return true;
}

} // namespace QDEngine
