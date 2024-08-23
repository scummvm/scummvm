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

#include "common/debug.h"
#include "common/stream.h"

#include "qdengine/qdengine.h"
#include "qdengine/qd_fwd.h"
#include "qdengine/parser/xml_tag_buffer.h"
#include "qdengine/parser/qdscr_parser.h"
#include "qdengine/qdcore/qd_counter.h"
#include "qdengine/qdcore/qd_game_object_state.h"


namespace QDEngine {

qdCounterElement::qdCounterElement() : _state(NULL),
	_last_state_status(false),
	_increment_value(true) {
}

qdCounterElement::~qdCounterElement() {
}

qdCounterElement::qdCounterElement(const qdGameObjectState *p, bool inc_value) : _state(p),
	_state_reference(p),
	_last_state_status(false),
	_increment_value(inc_value) {
}

bool qdCounterElement::init() {
//	if(!_state){
	_state = dynamic_cast<const qdGameObjectState *>(_state_reference.object());
	if (!_state) {
		debugC(3, kDebugLog, "qdCounterElement::init() failed");
		return false;
	}
//	}

	_last_state_status = false;

	return true;
}

bool qdCounterElement::quant() {
	if (_state) {
		bool result = false;

		bool status = _state->is_active();
		if (status && !_last_state_status)
			result = true;

		_last_state_status = status;

		return result;
	}

	return false;
}

bool qdCounterElement::load_script(const xml::tag *p) {
	for (xml::tag::subtag_iterator it = p->subtags_begin(); it != p->subtags_end(); ++it) {
		xml::tag_buffer buf(*it);
		switch (it->ID()) {
		case QDSCR_NAMED_OBJECT:
			_state_reference.load_script(&*it);
			break;
		case QDSCR_COUNTER_INC_VALUE:
			_increment_value = (xml::tag_buffer(*it).get_int()) ? true : false;
			break;
		}
	}

	return true;
}

bool qdCounterElement::save_script(Common::WriteStream &fh, int indent) const {
	for (int i = 0; i < indent; i++) {
		fh.writeString("\t");
	}

	fh.writeString("<counter_element");

	if (!_increment_value) {
		fh.writeString(" inc_value=\"0\"");
	}
	fh.writeString(">\r\n");

	if (_state) {
		qdNamedObjectReference ref(_state);
		ref.save_script(fh, indent + 1);
	}

	for (int i = 0; i < indent; i++) {
		fh.writeString("\t");
	}
	fh.writeString("</counter_element>\r\n");

	return true;
}

bool qdCounterElement::load_data(Common::SeekableReadStream &fh, int save_version) {
	char v;
	v = fh.readByte();
	_last_state_status = v;
	return true;
}

bool qdCounterElement::save_data(Common::WriteStream &fh) const {
	fh.writeByte(_last_state_status);
	return true;
}

qdCounter::qdCounter() : _value(0),
	_value_limit(0) {
}

qdCounter::~qdCounter() {
}

void qdCounter::set_value(int value) {
	_value = value;

	if (_value_limit > 0 && _value >= _value_limit)
		_value = 0;

	if (check_flag(POSITIVE_VALUE) && _value < 0)
		_value = 0;
}

void qdCounter::add_value(int value_delta) {
	_value += value_delta;

	if (_value_limit > 0 && _value >= _value_limit)
		_value = 0;

	if (check_flag(POSITIVE_VALUE) && _value < 0)
		_value = 0;
}

bool qdCounter::add_element(const qdGameObjectState *p, bool inc_value) {
	element_container_t::const_iterator it = Common::find(_elements.begin(), _elements.end(), p);
	if (it != _elements.end())
		return false;

	_elements.push_back(qdCounterElement(p, inc_value));
	return true;
}

bool qdCounter::remove_element(const qdGameObjectState *p) {
	element_container_t::iterator it = Common::find(_elements.begin(), _elements.end(), p);
	if (it != _elements.end()) {
		_elements.erase(it);
		return true;
	}

	return false;
}

bool qdCounter::remove_element(int idx) {
	assert(idx >= 0 && idx < (int)_elements.size());

	_elements.erase(_elements.begin() + idx);
	return true;
}

void qdCounter::quant() {
	int value_change = 0;
	for (element_container_t::iterator it = _elements.begin(); it != _elements.end(); ++it) {
		if (it->quant()) {
			if (it->increment_value())
				value_change++;
			else
				value_change--;
		}
	}

	_value += value_change;

	if (_value_limit > 0 && _value >= _value_limit)
		_value = 0;

	if (check_flag(POSITIVE_VALUE) && _value < 0)
		_value = 0;
}

bool qdCounter::load_script(const xml::tag *p) {
	int num_elements = 0;
	for (xml::tag::subtag_iterator it = p->subtags_begin(); it != p->subtags_end(); ++it) {
		xml::tag_buffer buf(*it);
		switch (it->ID()) {
		case QDSCR_COUNTER_ELEMENT:
			num_elements++;
			break;
		}
	}

	_elements.reserve(num_elements);

	for (xml::tag::subtag_iterator it = p->subtags_begin(); it != p->subtags_end(); ++it) {
		xml::tag_buffer buf(*it);
		switch (it->ID()) {
		case QDSCR_NAME:
			set_name(it->data());
			break;
		case QDSCR_FLAG:
			set_flag(xml::tag_buffer(*it).get_int());
			break;
		case QDSCR_COUNTER_ELEMENT: {
			qdCounterElement el;
			el.load_script(&*it);
			_elements.push_back(el);
		}
		break;
		case QDSCR_COUNTER_LIMIT:
			xml::tag_buffer(*it) > _value_limit;
			break;
		}
	}

	return true;
}

bool qdCounter::save_script(Common::WriteStream &fh, int indent) const {
	for (int i = 0; i < indent; i++) {
		fh.writeString("\t");
	}

	fh.writeString("<counter");

	fh.writeString(Common::String::format(" name=\"%s\"", qdscr_XML_string(name())));

	if (_value_limit) {
		fh.writeString(Common::String::format(" limit=\"%d\"", _value_limit));
	}

	if (flags()) {
		fh.writeString(Common::String::format(" flags=\"%d\"", flags()));
	}
	fh.writeString(">\r\n");

	for (auto &it : _elements) {
		it.save_script(fh, indent + 1);
	}

	for (int i = 0; i < indent; i++) {
		fh.writeString("\t");
	}
	fh.writeString("</counter>\r\n");

	return true;
}

bool qdCounter::load_data(Common::SeekableReadStream &fh, int save_version) {
	debugC(3, kDebugSave, "  qdCounter::load_data(): before %ld", fh.pos());
	int sz;
	_value = fh.readSint32LE();
	sz = fh.readSint32LE();

	if (sz != (int)_elements.size())
		return false;

	for (auto &it : _elements)
		it.load_data(fh, save_version);

	debugC(3, kDebugSave, "  qdCounter::load_data(): after %ld", fh.pos());
	return true;
}

bool qdCounter::save_data(Common::WriteStream &fh) const {
	debugC(3, kDebugSave, "  qdCounter::save_data(): before %ld", fh.pos());
	fh.writeSint32LE(_value);
	fh.writeSint32LE(_elements.size());

	for (auto &it : _elements) {
		it.save_data(fh);
	}

	debugC(3, kDebugSave, "  qdCounter::save_data(): after %ld", fh.pos());
	return true;
}

void qdCounter::init() {
	for (element_container_t::iterator it = _elements.begin(); it != _elements.end(); ++it)
		it->init();

	_value = 0;
}

} // namespace QDEngine
