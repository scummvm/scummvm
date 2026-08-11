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

#include "qdengine/qd_fwd.h"
#include "qdengine/parser/xml_tag_buffer.h"
#include "qdengine/parser/qdscr_parser.h"
#include "qdengine/qdcore/qd_game_dispatcher.h"
#include "qdengine/qdcore/qd_counter.h"
#include "qdengine/qdcore/qd_interface_counter.h"
#include "qdengine/system/graphics/gr_dispatcher.h"


namespace QDEngine {

qdInterfaceCounter::qdInterfaceCounter() : _counter(0) {
	_digits = 4;

	_lastValue = -1;

	_textFormat.toggle_global_depend(false);
}

qdInterfaceCounter::qdInterfaceCounter(const qdInterfaceCounter &counter) : qdInterfaceElement(counter),
	_textFormat(counter._textFormat),
	_digits(counter._digits),
	_counterName(counter._counterName),
	_counter(counter._counter) {
	_lastValue = -1;
	_textFormat.toggle_global_depend(false);
}

qdInterfaceCounter::~qdInterfaceCounter() {
}

qdInterfaceCounter &qdInterfaceCounter::operator = (const qdInterfaceCounter &counter) {
	if (this == &counter) return *this;

	*static_cast<qdInterfaceElement *>(this) = counter;

	_textFormat = counter._textFormat;

	_digits = counter._digits;

	_counterName = counter._counterName;
	_counter = counter._counter;

	_lastValue = -1;

	return *this;
}

bool qdInterfaceCounter::save_script_body(Common::WriteStream &fh, int indent) const {
	for (int i = 0; i <= indent; i++) {
		fh.writeString("\t");
	}
	fh.writeString(Common::String::format("<counter_name>%s</counter_name>\r\n", qdscr_XML_string(counterName())));

	_textFormat.save_script(fh, indent + 1);

	for (int i = 0; i <= indent; i++) {
		fh.writeString("\t");
	}
	fh.writeString(Common::String::format("<digits>%d</digits>\r\n", _digits));

	return true;
}

bool qdInterfaceCounter::load_script_body(const xml::tag *p) {
	for (xml::tag::subtag_iterator it = p->subtags_begin(); it != p->subtags_end(); ++it) {
		switch (it->ID()) {
		case QDSCR_COUNTER_NAME:
			setCounterName(it->data());
			break;
		case QDSCR_SCREEN_TEXT_FORMAT:
			_textFormat.load_script(&*it);
			break;
		case QDSCR_COUNTER_DIGITS:
			_digits = xml::tag_buffer(*it).get_int();
			break;
		}
	}

	return true;
}

Common::String qdInterfaceCounter::data() const {
	Common::String str;

	int val = 0;
	if (_counter)
		val = _counter->value();

	if (val < 0)
		str += "-";

	int delta = 10;
	for (int i = 1; i < _digits; i++) {
		if (abs(val) < abs(delta))
			str += "0";

		delta *= 10;
	}

	str += Common::String::format("%d", abs(val));
	debugC(3, kDebugLog, "qdInterfaceCounter::data() %s", str.c_str());
	return str;
}

bool qdInterfaceCounter::mouse_handler(int x, int y, mouseDispatcher::mouseEvent ev) {
	return true;
}

bool qdInterfaceCounter::keyboard_handler(Common::KeyCode vkey) {
	return false;
}

grScreenRegion qdInterfaceCounter::screen_region() const {
	Vect2i sz = Vect2i(size_x(), size_y());
	Vect2i pos = r();

	return grScreenRegion(pos.x, pos.y, sz.x, sz.y);
}

bool qdInterfaceCounter::init(bool is_game_active) {
	_counter = qdGameDispatcher::get_dispatcher()->get_counter(counterName());
	return true;
}

bool qdInterfaceCounter::redraw() const {
	qdInterfaceElement::redraw();

	if (_counter) {
		const grFont *font = qdGameDispatcher::get_dispatcher()->
		                     find_font(_textFormat.font_type());

		Vect2i sz = Vect2i(size_x(), size_y());
		Vect2i pos = r() - sz / 2;
		grDispatcher::instance()->drawAlignedText(pos.x, pos.y, sz.x, sz.y,
		        _textFormat.color(), data().c_str(), GR_ALIGN_LEFT, 0, 0, font);
	}

	return true;
}

bool qdInterfaceCounter::need_redraw() const {
	if (qdInterfaceElement::need_redraw())
		return true;

	if (_counter)
		return _lastValue != _counter->value();

	return false;
}

bool qdInterfaceCounter::post_redraw() {
	qdInterfaceElement::post_redraw();

	if (_counter)
		_lastValue = _counter->value();

	return true;
}

int qdInterfaceCounter::size_x() const {
	const grFont *font = qdGameDispatcher::get_dispatcher()->find_font(_textFormat.font_type());
	return grDispatcher::instance()->textWidth(data().c_str(), 0, font);
}

int qdInterfaceCounter::size_y() const {
	const grFont *font = qdGameDispatcher::get_dispatcher()->find_font(_textFormat.font_type());
	return grDispatcher::instance()->textHeight(data().c_str(), 0, font);
}

void qdInterfaceCounter::setCounter(const qdCounter *counter) {
	_counter = counter;
	_counterName = (counter) ? counter->name() : "";
}

const char *qdInterfaceCounter::counterName() const {
	return _counterName.c_str();
}

void qdInterfaceCounter::setCounterName(const char *name) {
	_counterName = name;
}
} // namespace QDEngine
