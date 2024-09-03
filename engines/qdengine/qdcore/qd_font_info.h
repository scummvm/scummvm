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

#ifndef QDENGINE_QDCORE_QD_FONT_INFO_H
#define QDENGINE_QDCORE_QD_FONT_INFO_H

#include "qdengine/parser/xml_fwd.h"
#include "qdengine/parser/xml_tag.h"
#include "qdengine/parser/xml_tag_buffer.h"
#include "qdengine/parser/qdscr_parser.h"
#include "qdengine/system/graphics/gr_font.h"
#include "qdengine/qdcore/qd_named_object.h"
#include "qdengine/qdcore/qd_object_list_container.h"

namespace QDEngine {

//! Шрифт
class qdFontInfo : public qdNamedObject {
public:
	qdFontInfo() : _type(0), _font(NULL) {}
	explicit qdFontInfo(int tp) : _type(tp), _font(NULL) {}
	// Копирующий конструктор
	qdFontInfo(const qdFontInfo &fi);
	~qdFontInfo();

	int named_object_type() const {
		return QD_NAMED_OBJECT_FONT_INFO;
	}

	bool operator == (const qdFontInfo &fi) const {
		return (fi.type() == _type) && (fi._font_file_name == _font_file_name);
	}
	bool operator == (int t) const {
		return (t == _type);
	}
	qdFontInfo &operator = (const qdFontInfo &fi);

	int type() const {
		return _type;
	}
	void set_type(int tp) {
		_type = tp;
	}

	void set_font_file_name(const Common::Path fname) {
		_font_file_name = fname;
	}
	const Common::Path font_file_name() const {
		return _font_file_name;
	}

	const grFont *font() const {
		return _font;
	}

	bool load_script(const xml::tag *p);
	bool save_script(Common::WriteStream &fh, int indent = 0) const;

	//! Грузит шрифт из файлов: font_file_name_ (хранит путь к *.tga части)
	//! и файла аналогичного font_file_name_, но с расш-ем. *.idx (индексная часть)
	bool load_font();

private:

	//! Тип шрифта.
	int _type;
	//! Файл шрифта (*.tga).
	Common::Path _font_file_name;
	//! Сам шрифт
	grFont *_font;
};

typedef Std::list<qdFontInfo *> qdFontInfoList;


} // namespace QDEngine

#endif // QDENGINE_QDCORE_QD_FONT_INFO_H
