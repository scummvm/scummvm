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

#include "common/stream.h"

#include "qdengine/qd_fwd.h"
#include "qdengine/xmath.h"
#include "qdengine/qdcore/qd_setup.h"
#include "qdengine/qdcore/qd_screen_text.h"
#include "qdengine/system/graphics/gr_dispatcher.h"

#include "qdengine/qdcore/qd_game_object.h"
#include "qdengine/qdcore/qd_game_object_state.h"

#include "qdengine/qdcore/qd_game_dispatcher.h" // В qdGameDispather хранятся шрифты


namespace QDEngine {

qdScreenTextFormat::qdScreenTextFormat() : _arrangement(ARRANGE_VERTICAL),
	_alignment(ALIGN_LEFT),
	_color(0xFFFFFF),
	_hover_color(0xFFFFFF),
	_font_type(QD_FONT_TYPE_NONE),
	_global_depend(true) {
}

const qdScreenTextFormat &qdScreenTextFormat::default_format() {
	return g_engine->_default_format;
}

const qdScreenTextFormat &qdScreenTextFormat::global_text_format() {
	return g_engine->_global_text_format;
}
void qdScreenTextFormat::set_global_text_format(const qdScreenTextFormat &frmt) {
	g_engine->_global_text_format = frmt;
}

const qdScreenTextFormat &qdScreenTextFormat::global_topic_format() {
	return g_engine->_global_topic_format;
}
void qdScreenTextFormat::set_global_topic_format(const qdScreenTextFormat &frmt) {
	g_engine->_global_topic_format = frmt;
}



bool qdScreenTextFormat::load_script(const xml::tag *p) {
	bool load_global_depend = false;
	for (xml::tag::subtag_iterator it = p->subtags_begin(); it != p->subtags_end(); ++it) {
		switch (it->ID()) {
		case QDSCR_TEXT_ALIGN:
			set_arrangement((qdScreenTextFormat::arrangement_t)xml::tag_buffer(*it).get_int());
			break;
		case QDSCR_ALIGN:
			set_alignment((qdScreenTextFormat::alignment_t)xml::tag_buffer(*it).get_int());
			break;
		case QDSCR_TEXT_COLOR:
			set_color(xml::tag_buffer(*it).get_int());
			break;
		case QDSCR_TEXT_HOVER_COLOR:
			set_hover_color(xml::tag_buffer(*it).get_int());
			break;
		case QDSCR_FONT_TYPE:
			set_font_type(xml::tag_buffer(*it).get_int());
			break;
		case QDSCR_GLOBAL_DEPEND:
			_global_depend = (0 != xml::tag_buffer(*it).get_int());
			load_global_depend = true;
			break;
		}
	}

	// Если скрипт старый и инфа о связи с глобальным форматом отсутствует, то
	// является ли формат связанным с глобальным по содержимому
	if (!load_global_depend)
		_global_depend = (global_text_format() == *this);

	return true;
}

bool qdScreenTextFormat::save_script(Common::WriteStream &fh, int indent) const {
	for (int i = 0; i < indent; i++) {
		fh.writeString("\t");
	}
	fh.writeString("<screen_text_format");

	// Если зависим от глобального формата текста, то пишем только сам факт зависимости
	// (пишем факт или отсутствие зависимости в любом случае)
	if (is_global_depend())
		fh.writeString(" global_depend=\"1\"");
	else {
		// Не зависит от глобальных параметров - значит пишем данные о формате
		fh.writeString(" global_depend=\"0\"");

		if (arrangement() != qdScreenTextFormat::default_format().arrangement()) {
			fh.writeString(Common::String::format(" text_align=\"%d\"", (int)arrangement()));
		}

		if (alignment() != qdScreenTextFormat::default_format().alignment()) {
			fh.writeString(Common::String::format(" align=\"%d\"", (int)alignment()));
		}

		if (color() != qdScreenTextFormat::default_format().color()) {
			fh.writeString(Common::String::format(" text_color=\"%d\"", color()));
		}

		if (hover_color() != qdScreenTextFormat::default_format().hover_color()) {
			fh.writeString(Common::String::format(" text_hover_color=\"%d\"", hover_color()));
		}

		if (font_type() != qdScreenTextFormat::default_format().font_type()) {
			fh.writeString(Common::String::format(" font_type=\"%d\"", font_type()));
		}
	}

	fh.writeString("/>\r\n");

	return true;
}

qdScreenText::qdScreenText(const char *p, const Vect2i &pos, qdGameObjectState *owner) : _pos(pos),
	_size(0, 0),
	_owner(owner),
	_hover_mode(false) {
	set_data(p);
}

qdScreenText::qdScreenText(const char *p, const qdScreenTextFormat &fmt, const Vect2i &pos, qdGameObjectState *owner) : _pos(pos),
	_size(0, 0),
	_text_format(fmt),
	_owner(owner) {
	_hover_mode = false;

	set_data(p);
}

qdScreenText::~qdScreenText() {
}

void qdScreenText::redraw(const Vect2i &owner_pos) const {
	int x = owner_pos.x + _pos.x;
	int y = owner_pos.y + _pos.y;

	uint32 col = _hover_mode ? _text_format.hover_color() : _text_format.color();

	const grFont *font = qdGameDispatcher::get_dispatcher()->find_font(_text_format.font_type());

	grDispatcher::instance()->drawAlignedText(x, y, _size.x, _size.y, col, data(), grTextAlign(_text_format.alignment()), 0, 0, font);
	if (g_engine->_debugDraw)
		grDispatcher::instance()->rectangle(x, y, _size.x, _size.y, col, 0, GR_OUTLINED);
}

void qdScreenText::set_data(const char *p) {
	_data = p;

	const grFont *font = qdGameDispatcher::get_dispatcher()->find_font(_text_format.font_type());
	_size.x = grDispatcher::instance()->textWidth(data(), 0, font);
	_size.y = grDispatcher::instance()->textHeight(data(), 0, font);
}

bool qdScreenText::is_owned_by(const qdNamedObject *p) const {
	return (_owner && p == _owner->owner());
}

bool qdScreenText::format_text(int max_width) {
	const grFont *font = qdGameDispatcher::get_dispatcher()->find_font(text_format().font_type());
	if (NULL == font)
		font = grDispatcher::get_default_font();

	bool correct = true;
	int safe_space = -1;
	int cur_wid = 0;
	_data += ' ';	// Adding space for simplifying the algorightm (because of the trailing
					// space we will trigger attempt to format the string ending)
					// The space will not affect the output because it is at the end.

	for (uint i = 0; i < _data.size(); i++) {
		if (_data[i] == '\n') {
			if (cur_wid > max_width) {
				// Safe space is present, so it is safe to split it (e.g. everything fits max_width)
				if (safe_space >= 0) {
					_data.setChar('\n', safe_space);
					i = safe_space; // in for(...) we will move to safe_space + 1
				} else { // it didn't fit (no safe space). But we split it anyway, it is at least something...
					_data.setChar('\n', i);
					correct = false;
				}
			}

			safe_space = -1;
			cur_wid = 0;
		} else if (' ' != _data[i]) { // Not a space -- we accumulate width
			cur_wid += font->find_char((byte)_data[i]).size_x();
		} else { // // Space - it is safe to split here (we remember this position or split here)
			cur_wid += font->size_x() / 2;

			// Width is not exceeded -- remember the current _safe_ space and move on
			if (cur_wid < max_width) {
				safe_space = i;
				continue;
			} else { // The width is exceeded, and we saw a safe space -- have to split. (Everything fits max_width)
				if (safe_space >= 0) {
					_data.setChar('\n', safe_space);
					i = safe_space; // in for(...) we will move to safe_space + 1
				} else { // it didn't fit (no safe space). But we split it anyway, it is at least something...
					_data.setChar('\n', i);
					correct = false;
				}
				safe_space = -1; // We have split -- no more safe space
				cur_wid = 0;     // New part has zero width for now
			}
		}
	}

	_data.erase(_data.size() - 1, 1); // We remove the last symbol (the space that we added initially)

	set_data(_data.c_str()); // Set the data (for text dimensions recalc)

	return correct;
}

} // namespace QDEngine
