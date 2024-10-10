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

#ifndef QDENGINE_QDCORE_QD_SCREEN_TEXT_SET_H
#define QDENGINE_QDCORE_QD_SCREEN_TEXT_SET_H

#include "qdengine/qdcore/qd_screen_text.h"
#include "qdengine/parser/xml_fwd.h"


namespace QDEngine {

//! Набор экранных текстов.
class qdScreenTextSet {
public:
	qdScreenTextSet();
	~qdScreenTextSet();

	//! Возвращает идентификатор набора.
	int ID() const {
		return _ID;
	}
	//! Устанавливает идентификатор набора.
	void set_ID(int id) {
		_ID = id;
	}

	//! Возвращает экранные координаты центра набора.
	const Vect2i &screen_pos() const {
		return _pos;
	}
	//! Устанавливает экранные координаты центра набора.
	void set_screen_pos(const Vect2i &pos) {
		_pos = pos;
	}

	//! Возвращает размеры набора на экране.
	const Vect2i &screen_size() const {
		return _size;
	}
	//! Устанавливает размеры набора на экране.
	void set_screen_size(const Vect2i &sz) {
		_size = sz;
	}

	//! Добавление текста в набор.
	qdScreenText *add_text(const qdScreenText &txt);
	//! Очистка всех текстов набора.
	void clear_texts() {
		_texts.clear();
		arrange_texts();
	}
	//! Очистка всех текстов с владельцем owner.
	void clear_texts(qdNamedObject *owner);

	//! Отрисовка набора.
	void redraw() const;
	bool pre_redraw() const;
	bool post_redraw();

	grScreenRegion screen_region() const;

	//! Расставляет тексты в наборе.
	bool arrange_texts();

	void load_script(const xml::tag *p);
	bool save_script(Common::WriteStream &fh, int indent) const;

	bool need_redraw() const {
		return _need_redraw;
	}

	bool is_empty() const {
		return _texts.empty();
	}

	int size() const {
		return _texts.size();
	}

	void set_max_text_width(int width) {
		_max_text_width = width;
		format_texts();
		arrange_texts();
	}

	//! Возвращает текст, в который попадает точка с экранными координатами (x,y).
	qdScreenText *get_text(int x, int y);

	bool was_changed() const {
		return _was_changed;
	}
	void toggle_changed(bool state) {
		_was_changed = state;
	}

	int new_texts_height() const {
		return _new_texts_height;
	}
	void clear_new_texts_height() {
		_new_texts_height = 0;
	}

	void clear_hover_mode();

private:

	//! Идентификатор набора.
	int _ID;

	//! Экранные координаты центра набора.
	Vect2i _pos;
	//! Размеры области, отведенной под набор на экране.
	Vect2i _size;

	//! Расстояние между соседними текстами в пикселах.
	int _space;

	typedef Common::Array<qdScreenText> texts_container_t;
	//! Тексты.
	texts_container_t _texts;

	//! Устанавливается в true при добавлении/удалении текстов.
	bool _was_changed;

	//! Максимальная ширина текста в пикселах.
	/**
	Если нулевая - не учитывается.
	*/
	int _max_text_width;

	int _new_texts_height;

	bool _need_redraw;
	grScreenRegion _last_screen_region;

	//! Форматирует тексты по ширине, чтобы не вылезали за max_text_width_.
	void format_texts();
};

} // namespace QDEngine

#endif // QDENGINE_QDCORE_QD_SCREEN_TEXT_SET_H
