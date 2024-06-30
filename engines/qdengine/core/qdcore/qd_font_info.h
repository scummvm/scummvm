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

#ifndef QDENGINE_CORE_QDCORE_QD_FONT_INFO_H
#define QDENGINE_CORE_QDCORE_QD_FONT_INFO_H

#include "qdengine/core/parser/xml_fwd.h"
#include "qdengine/core/parser/xml_tag.h"
#include "qdengine/core/parser/xml_tag_buffer.h"
#include "qdengine/core/parser/qdscr_parser.h"
#include "qdengine/core/system/graphics/gr_font.h"
#include "qdengine/core/qdcore/qd_named_object.h"
#include "qdengine/core/qdcore/qd_object_list_container.h"


namespace QDEngine {


//! Шрифт
class qdFontInfo : public qdNamedObject {
public:
	qdFontInfo() : type_(0), font_(NULL) {}
	explicit qdFontInfo(int tp) : type_(tp), font_(NULL) {}
	// Копирующий конструктор
	qdFontInfo(const qdFontInfo &fi);
	~qdFontInfo();

	int named_object_type() const {
		return QD_NAMED_OBJECT_FONT_INFO;
	}

	bool operator == (const qdFontInfo &fi) const {
		return (fi.type() == type_) && (fi.font_file_name_ == font_file_name_);
	}
	bool operator == (int t) const {
		return (t == type_);
	}
	qdFontInfo &operator = (const qdFontInfo &fi);

	int type() const {
		return type_;
	}
	void set_type(int tp) {
		type_ = tp;
	}

	void set_font_file_name(const char *fname) {
		font_file_name_ = fname;
	}
	const char *font_file_name() const {
		return font_file_name_.c_str();
	}

	const grFont *font() const {
		return font_;
	}

	bool load_script(const xml::tag *p);
	bool save_script(class XStream &fh, int indent = 0) const;

	//! Грузит шрифт из файлов: font_file_name_ (хранит путь к *.tga части)
	//! и файла аналогичного font_file_name_, но с расш-ем. *.idx (индексная часть)
	bool load_font();

private:

	//! Тип шрифта.
	int type_;
	//! Файл шрифта (*.tga).
	std::string font_file_name_;
	//! Сам шрифт
	grFont *font_;
};

typedef std::list<qdFontInfo *> qdFontInfoList;


} // namespace QDEngine

#endif // QDENGINE_CORE_QDCORE_QD_FONT_INFO_H
