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
#include "common/path.h"

#include "qdengine/qd_precomp.h"
#include "qdengine/qdcore/qd_font_info.h"
#include "qdengine/qdcore/qd_file_manager.h"

namespace QDEngine {

qdFontInfo::qdFontInfo(const qdFontInfo &fi) : qdNamedObject(fi),
	type_(fi.type()), font_file_name_(fi.font_file_name()) {
	if (NULL != fi.font()) {
		font_ = new grFont;
		*font_ = *fi.font();
	} else
		font_ = NULL;
}


qdFontInfo::~qdFontInfo() {
	delete font_;
}

qdFontInfo &qdFontInfo::operator = (const qdFontInfo &fi) {
	if (this == &fi) return *this;

	*static_cast<qdNamedObject *>(this) = fi;

	type_ = fi.type();
	font_file_name_ = fi.font_file_name();

	delete font_;

	if (NULL != fi.font()) {
		font_ = new grFont;
		*font_ = *fi.font();
	} else
		font_ = NULL;

	return *this;
}


bool qdFontInfo::load_script(const xml::tag *p) {
	for (xml::tag::subtag_iterator it = p->subtags_begin(); it != p->subtags_end(); ++it) {
		switch (it->ID()) {
		case QDSCR_TYPE:
			set_type(xml::tag_buffer(*it).get_int());
			break;
		case QDSCR_FILE:
			set_font_file_name(Common::Path(it->data(), '\\').toString().c_str());
			break;
		case QDSCR_NAME:
			set_name(it->data());
			break;
		}
	}

	return true;
}

bool qdFontInfo::save_script(Common::SeekableWriteStream &fh, int indent) const {
	for (int i = 0; i < indent; i++) {
		fh.writeString("\t");
	}
	fh.writeString(Common::String::format("<font_info type=\"%d\"", type_));

	if (!font_file_name_.empty()) {
		fh.writeString(Common::String::format(" file=\"%s\"", qdscr_XML_string(font_file_name_.c_str())));
	}

	if (name()) {
		fh.writeString(Common::String::format(" name=\"%s\"", qdscr_XML_string(name())));
	}

	fh.writeString("/>\r\n");
	return true;
}

bool qdFontInfo::load_font() {
	grFont *buf_font = new grFont;

	bool load_fl = false; // По умолчанию загрузка прошла успешно

	Common::SeekableReadStream *fh;
	if (qdFileManager::instance().open_file(&fh, font_file_name(), false)) {
		// Грузим альфу шрифта из .tga
		if (buf_font->load_alpha(fh)) {
			// Меняем расширение с .tga на .idx
			Common::String fpath(font_file_name());
			Common::String tgaExt = ".tga";
			Common::String idxExt = ".idx";

			Common::replace(fpath, tgaExt, idxExt);

			// Открываем .idx и грузим индекс
			if (qdFileManager::instance().open_file(&fh, fpath.c_str(), false)) {
				if (buf_font->load_index(fh))
					load_fl = true;
			}
		}
	}

	delete fh;

	if (!load_fl) {
		delete buf_font;
		return false;
	}

	font_ = buf_font;
	return true;
}
} // namespace QDEngine
