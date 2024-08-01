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

#define FORBIDDEN_SYMBOL_ALLOW_ALL
#include "common/stream.h"
#include "qdengine/qd_precomp.h"
#include "qdengine/qdcore/qd_setup.h"
#include "qdengine/qdcore/qd_screen_text.h"
#include "qdengine/system/graphics/gr_dispatcher.h"

#include "qdengine/qdcore/qd_game_object.h"
#include "qdengine/qdcore/qd_game_object_state.h"

#include "qdengine/qdcore/qd_game_dispatcher.h" // В qdGameDispather хранятся шрифты


namespace QDEngine {

qdScreenTextFormat qdScreenTextFormat::_default_format;
qdScreenTextFormat qdScreenTextFormat::_global_text_format;
qdScreenTextFormat qdScreenTextFormat::_global_topic_format;

qdScreenTextFormat::qdScreenTextFormat() : _arrangement(ARRANGE_VERTICAL),
	_alignment(ALIGN_LEFT),
	_color(0xFFFFFF),
	_hover_color(0xFFFFFF),
	_font_type(QD_FONT_TYPE_NONE),
	_global_depend(true) {
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
	_owner(owner) {
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

	const grFont *font = qdGameDispatcher::get_dispatcher()->
	                     find_font(_text_format.font_type());

	grDispatcher::instance()->DrawAlignedText(x, y, _size.x, _size.y, col, data(), grTextAlign(_text_format.alignment()), 0, 0, font);
	if (qdGameConfig::get_config().debug_draw())
		grDispatcher::instance()->Rectangle(x, y, _size.x, _size.y, col, 0, GR_OUTLINED);
}

void qdScreenText::set_data(const char *p) {
	_data = p;

	const grFont *font = qdGameDispatcher::get_dispatcher()->
	                     find_font(_text_format.font_type());
	_size.x = grDispatcher::instance()->TextWidth(data(), 0, font);
	_size.y = grDispatcher::instance()->TextHeight(data(), 0, font);
}

bool qdScreenText::is_owned_by(const qdNamedObject *p) const {
	return (_owner && p == _owner->owner());
}

bool qdScreenText::format_text(int max_width) {
	const grFont *font = qdGameDispatcher::get_dispatcher()->find_font(
	                         text_format().font_type());
	if (NULL == font) font = grDispatcher::get_default_font();

	bool correct = true;
	int safe_space = -1;
	int cur_wid = 0;
	_data += ' '; // Добавляем пробел для упрощения алгоритма (из-за последнего
	// пробела в конце всегда включится попытка форматирования конца).
	// Пробел не отразиться на выводе, т.к. он в конце.

	byte *dp = (byte *)_data.c_str();

	for (int i = 0; i < _data.length(); i++) {
		if (dp[i] == '\n') {
			if (cur_wid > max_width) {
				// безопасный пробел есть - безопасно режем (т.е. все влезает в max_width)
				if (safe_space >= 0) {
					dp[safe_space] = '\n';
					i = safe_space; // в for(...) перейдем к safe_space + 1
				}
				// не влезли (нет безопасного пробела). Но режем все равно - хоть так...
				else {
					dp[i] = '\n';
					correct = false;
				}
			}

			safe_space = -1;
			cur_wid = 0;
		}
		// Не пробел - копим длину
		else if (' ' != dp[i])
			cur_wid += font->find_char(dp[i]).size_x();
		// Пробел - здесь можно резать (запомним эту позицию или разрежем здесь)
		else {
			cur_wid += font->size_x() / 2;

			// Длина не превышена - запомним текущий _безопасный_ пробел и двинемся дальше
			if (cur_wid < max_width) {
				safe_space = i;
				continue;
			}
			// Превысили длину, да еще и пробел встретили - нужно резать
			else {
				// безопасный пробел есть - безопасно режем (т.е. все влезает в max_width)
				if (safe_space >= 0) {
					dp[safe_space] = '\n';
					i = safe_space; // в for(...) перейдем к safe_space + 1
				}
				// не влезли (нет безопасного пробела). Но режем все равно - хоть так...
				else {
					dp[i] = '\n';
					correct = false;
				}
				safe_space = -1; // Разрезали - безопасного пробела нет
				cur_wid = 0;     // Новый кусок пока нулевой длины
			}
		}
	}

	_data.erase(_data.length() - 1, 1); // Удаляем последний символ (пробел добавленный нами)

	set_data(_data.c_str()); // Устанавливаем данные (для пересчета размера текста)

	return correct;
}

} // namespace QDEngine
