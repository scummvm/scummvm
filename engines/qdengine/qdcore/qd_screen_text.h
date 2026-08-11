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

#ifndef QDENGINE_QDCORE_QD_SCREEN_TEXT_H
#define QDENGINE_QDCORE_QD_SCREEN_TEXT_H

#include "common/stream.h"

#include "qdengine/parser/xml_fwd.h"
#include "qdengine/xmath.h"
#include "qdengine/system/graphics/gr_screen_region.h"
#include "qdengine/system/graphics/gr_font.h"

namespace QDEngine {

class qdGameObjectState;

const int QD_FONT_TYPE_NONE = -1; //! Обозначает отсутствие шрифта

//! Формат экранного текста
class qdScreenTextFormat {
public:
	//! Расположение относительно других текстов.
	enum arrangement_t {
		//! Выравнивание по горизонтали.
		ARRANGE_HORIZONTAL,
		//! Выравнивание по вертикали.
		ARRANGE_VERTICAL
	};

	/// Выравнивание текста.
	enum alignment_t {
		/// по левому краю
		ALIGN_LEFT,
		/// по центру
		ALIGN_CENTER,
		/// по правому краю
		ALIGN_RIGHT
	};

	qdScreenTextFormat();
	~qdScreenTextFormat() { };

	bool operator == (const qdScreenTextFormat &fmt) const {
		return (_color == fmt._color &&
		        _arrangement == fmt._arrangement &&
		        _alignment == fmt._alignment &&
		        _hover_color == fmt._hover_color &&
		        _font_type == fmt.font_type() &&
		        _global_depend == fmt.is_global_depend());
	}

	bool operator != (const qdScreenTextFormat &fmt) const {
		return !(*this == fmt);
	}

	void set_arrangement(arrangement_t al) {
		_arrangement = al;
	}
	arrangement_t arrangement() const {
		return _arrangement;
	}

	void set_alignment(alignment_t al) {
		_alignment = al;
	}
	alignment_t alignment() const {
		return _alignment;
	}

	void set_color(int color) {
		_color = color;
	}
	int color() const {
		return _color;
	}

	void set_hover_color(int color) {
		_hover_color = color;
	}
	int hover_color() const {
		return _hover_color;
	}

	static const qdScreenTextFormat &default_format();
	static const qdScreenTextFormat &global_text_format();
	static void set_global_text_format(const qdScreenTextFormat &frmt);
	static const qdScreenTextFormat &global_topic_format();
	static void set_global_topic_format(const qdScreenTextFormat &frmt);

	void set_font_type(int tp) {
		_font_type = tp;
	}
	int font_type() const {
		return _font_type;
	}

	bool is_global_depend() const {
		return _global_depend;
	}
	void toggle_global_depend(bool flag = true) {
		_global_depend = flag;
	}

	bool load_script(const xml::tag *p);
	bool save_script(Common::WriteStream &fh, int indent = 0) const;

private:

	//! Расположение относительно других текстов.
	arrangement_t _arrangement;
	/// Выравнивание текста.
	alignment_t _alignment;
	//! Цвет текста, RGB().
	int _color;
	//! Цвет текста при наведении на него мыши, RGB().
	int _hover_color;

	//! Тип шрифта
	int _font_type;

	/** Флаг, означающий, что данные должны браться из _gloabal_text_format или
	    global_topic_format_ вместо текущей переменной. */
	bool _global_depend;
};

//! Экранный текст.
class qdScreenText {
public:
	qdScreenText(const char *p, const Vect2i &pos = Vect2i(0, 0), qdGameObjectState *owner = NULL);
	qdScreenText(const char *p, const qdScreenTextFormat &fmt, const Vect2i &pos = Vect2i(0, 0), qdGameObjectState *owner = NULL);
	~qdScreenText();

	//! Экранные координаты центра текста.
	Vect2i screen_pos() {
		return _pos;
	}
	//! Устанавливает экранные координаты центра текста.
	void set_screen_pos(const Vect2i &pos) {
		_pos = pos;
	}

	const char *data() const {
		return _data.c_str();
	}
	//! Устанавливает текст.
	/**
	Текст копируется во внутренние данные.
	*/
	void set_data(const char *p);

	//! Горизонтальный размер текста в пикселах.
	int size_x() const {
		return _size.x;
	}
	//! Вертикальный размер текста в пикселах.
	int size_y() const {
		return _size.y;
	}

	qdScreenTextFormat::arrangement_t arrangement() const {
		return _text_format.arrangement();
	}
	qdScreenTextFormat::alignment_t alignment() const {
		return _text_format.alignment();
	}

	qdScreenTextFormat text_format() const {
		return _text_format;
	}
	void set_text_format(const qdScreenTextFormat &fmt) {
		_text_format = fmt;
	}

	grScreenRegion screen_region() const {
		return grScreenRegion(_pos.x, _pos.y, _size.x, _size.y);
	}

	//! Отрисовка текста.
	void redraw(const Vect2i &owner_pos) const;

	uint32 color() const {
		return _text_format.color();
	}
	void set_color(uint32 col) {
		_text_format.set_color(col);
	}

	//! Возвращает указатель на владельца текста.
	qdGameObjectState *owner() const {
		return _owner;
	}
	//! Устанавливает владельца текста.
	void set_owner(qdGameObjectState *p) {
		_owner = p;
	}
	bool is_owned_by(const qdNamedObject *p) const;

	//! Проверка попадания точки в текст, параметры в экранных координатах.
	bool hit(int x, int y) const {
		if (x >= _pos.x && x < _pos.x + _size.x) {
			if (y >= _pos.y && y < _pos.y + _size.y)
				return true;
		}
		return false;
	}

	void set_hover_mode(bool state) {
		_hover_mode = state;
	}

	// форматирует текст так, чтобы его ширина не превышала max_width (размер в пикселах)
	bool format_text(int max_width);

private:

	//! Экранные координаты текста.
	Vect2i _pos;
	//! Размеры текста в пикселах.
	Vect2i _size;

	//! Текст (данные).
	Common::String _data;

	//! true, если над текстом курсор мыши
	bool _hover_mode;

	//! Cпособ выравнивания текста.
	qdScreenTextFormat _text_format;

	//! Владелец текста.
	mutable qdGameObjectState *_owner;
};

} // namespace QDEngine

#endif // QDENGINE_QDCORE_QD_SCREEN_TEXT_H
