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
#include "qdengine/system/graphics/gr_dispatcher.h"
#include "qdengine/parser/qdscr_parser.h"
#include "qdengine/parser/xml_tag_buffer.h"
#include "qdengine/qdcore/qd_screen_text_set.h"
#include "qdengine/qdcore/qd_game_dispatcher.h"
#include "qdengine/qdcore/qd_game_object_state.h"


namespace QDEngine {

qdScreenTextSet::qdScreenTextSet() : _ID(0),
	_pos(0, 0),
	_size(0, 0),
	_space(5),
	_was_changed(false),
	_need_redraw(false) {
	_max_text_width = 0;
	_new_texts_height = 0;
	_texts.reserve(16);
}

qdScreenTextSet::~qdScreenTextSet() {
}

void qdScreenTextSet::redraw() const {
	for (auto &it : _texts) {
		it.redraw(_pos);
	}

//	grDispatcher::instance()->Rectangle(pos_.x - size_.x/2,pos_.y - size_.y/2,size_.x,size_.x,0xFFFFFF,0,GR_OUTLINED);
}

bool qdScreenTextSet::arrange_texts() {
	if (_texts.empty()) return false;

	int row_sy = _texts[0].size_y();

	_texts[0].set_screen_pos(Vect2i(_space, 0));

	Vect2i text_pos(0, 0);

	for (uint i = 1; i < _texts.size(); i++) {
		if (_texts[i].arrangement() == qdScreenTextFormat::ARRANGE_HORIZONTAL) {
			if (row_sy < _texts[i].size_y())
				row_sy = _texts[i].size_y();

			text_pos.x += _texts[i - 1].size_x() + _space;
		} else {
			text_pos.x = 0;
			text_pos.y += row_sy + _space;

			row_sy = _texts[i].size_y();
		}

		_texts[i].set_screen_pos(text_pos);
	}

	int row_start = 0;
	int row_size = 1;

	for (uint i = 0; i < _texts.size(); i++) {
		if (_texts[i].arrangement() == qdScreenTextFormat::ARRANGE_VERTICAL || i == _texts.size() - 1) {
			int sx = 0;
			for (int j = 0; j < row_size; j++)
				sx += _texts[row_start + j].size_x() + _space;

			int dx = 0;
			switch (_texts[row_start].alignment()) {
			case qdScreenTextFormat::ALIGN_CENTER:
				dx = (_max_text_width - sx - _space) / 2;
				break;
			case qdScreenTextFormat::ALIGN_RIGHT:
				dx = _max_text_width - sx - _space;
				break;
			default:
				break;
			}

			for (int j = 0; j < row_size; j++) {
				Vect2i pos = _texts[row_start + j].screen_pos();
				pos.x += dx;
				_texts[row_start + j].set_screen_pos(pos);
			}

			row_size = 1;
			row_start = i;
		} else
			row_size++;
	}

	int sx = 0;
	int sy = 0;

	for (uint i = 0; i < _texts.size(); i++) {
		if (_texts[i].screen_pos().x + _texts[i].size_x() > sx)
			sx = _texts[i].screen_pos().x + _texts[i].size_x();
		if (_texts[i].screen_pos().y + _texts[i].size_y() > sy)
			sy = _texts[i].screen_pos().y + _texts[i].size_y();
	}

	_size = Vect2i(sx, sy);
	for (uint i = 0; i < _texts.size(); i++) {
		Vect2i pos = _texts[i].screen_pos();
		pos -= _size / 2;
		_texts[i].set_screen_pos(pos);
	}

	_need_redraw = true;
	return true;
}

qdScreenText *qdScreenTextSet::get_text(int x, int y) {
	for (auto &it : _texts) {
		if (it.hit(x - _pos.x, y - _pos.y)) return &it;
	}

	return NULL;
}

void qdScreenTextSet::clear_texts(qdNamedObject *owner) {
	bool ret = false;

	debugC(5, kDebugText, "qdScreenTextSet::clear_texts('%s') for ID: %d, size was: %d", owner->toString().c_str(), _ID, _texts.size());

	// Equivalent of
	// texts_container_t::iterator it = std::remove_if(_texts.begin(), _texts.end(), std::bind2nd(std::mem_fun_ref(&qdScreenText::is_owned_by), owner));

	for (int i = 0; i < _texts.size();) {
		if (_texts[i].is_owned_by(owner)) {
			_texts.remove_at(i);

			ret = true;
		} else {
			i++;
		}
	}

	if (ret) {
		_need_redraw = true;
		arrange_texts();
		toggle_changed(true);
	}

	debugC(5, kDebugText, "qdScreenTextSet::clear_texts(): size: %d", _texts.size());
}

bool qdScreenTextSet::pre_redraw() const {
	if (_need_redraw) {
		if (qdGameDispatcher * dp = qdGameDispatcher::get_dispatcher()) {
			dp->add_redraw_region(screen_region());
			dp->add_redraw_region(_last_screen_region);
		}
	}

	return true;
}

bool qdScreenTextSet::post_redraw() {
	_need_redraw = false;
	_last_screen_region = screen_region();
	return true;
}

grScreenRegion qdScreenTextSet::screen_region() const {
	grScreenRegion reg;
	for (texts_container_t::const_iterator it = _texts.begin(); it != _texts.end(); ++it)
		reg += it->screen_region();
	return reg;
}

void qdScreenTextSet::load_script(const xml::tag *p) {
	Vect2f r;
	for (xml::tag::subtag_iterator it = p->subtags_begin(); it != p->subtags_end(); ++it) {
		switch (it->ID()) {
		case QDSCR_ID:
			xml::tag_buffer(*it) > _ID;
			break;
		case QDSCR_POS2D:
			xml::tag_buffer(*it) > r.x > r.y;
			_pos = r;
			break;
		case QDSCR_SCREEN_SIZE:
			xml::tag_buffer(*it) > _size.x > _size.y;
			break;
		}
	}
}

bool qdScreenTextSet::save_script(Common::WriteStream &fh, int indent) const {
	for (int i = 0; i < indent; i++) {
		fh.writeString("\t");
	}

	fh.writeString(Common::String::format("<text_set ID=\"%d\"", _ID));

	if (_pos.x || _pos.y) {
		fh.writeString(Common::String::format(" pos_2d=\"%d %d\"", _pos.x, _pos.y));
	}

	if (_size.x || _size.y) {
		fh.writeString(Common::String::format(" screen_size=\"%d %d\"", _size.x, _size.y));
	}

	fh.writeString("/>\r\n");

	return true;
}

qdScreenText *qdScreenTextSet::add_text(const qdScreenText &txt) {
	int sy = _size.y;

	debugC(2, kDebugText, "qdScreenTextSet::add_text('%s') for ID: %d, '%s'", transCyrillic(txt.data()), _ID,
			txt.owner() ? txt.owner()->toString().c_str() : "<none>");

	_texts.push_back(txt);

	if (_max_text_width)
		_texts.back().format_text(_max_text_width - _space * 2);

	arrange_texts();
	toggle_changed(true);

	_new_texts_height += _size.y - sy;

	debugC(3, kDebugText, "qdScreenTextSet::add_text(): size: %d", _texts.size());

	return &_texts.back();
}

void qdScreenTextSet::clear_hover_mode() {
	for (texts_container_t::iterator it = _texts.begin(); it != _texts.end(); ++it)
		it->set_hover_mode(false);
}


void qdScreenTextSet::format_texts() {
	if (!_max_text_width) return;

	for (texts_container_t::iterator it = _texts.begin(); it != _texts.end(); ++it)
		it->format_text(_max_text_width);
}

} // namespace QDEngine
