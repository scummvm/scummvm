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

#include "common/str.h"

#include "qdengine/qd_fwd.h"
#include "qdengine/parser/qdscr_parser.h"
#include "qdengine/parser/xml_tag_buffer.h"
#include "qdengine/qdcore/qd_setup.h"
#include "qdengine/qdcore/qd_minigame_config.h"


namespace QDEngine {

qdMinigameConfigParameter::qdMinigameConfigParameter() : _data_type(PRM_DATA_STRING) {
	_data_count = 1;
	_is_data_valid = true;
}

qdMinigameConfigParameter::~qdMinigameConfigParameter() {
}

bool qdMinigameConfigParameter::load_script(const xml::tag *p) {
	set_data_string(p->data());

	for (xml::tag::subtag_iterator it = p->subtags_begin(); it != p->subtags_end(); ++it) {
		switch (it->ID()) {
		case QDSCR_NAME:
			set_name(it->data());
			break;
//			case QDSCR_COMMENT:
//				set_comment(it->data());
//				break;
//			case QDSCR_TYPE:
//				set_data_type(data_type_t(xml::tag_buffer(*it).get_int()));
//				break;
		case QDSCR_SIZE:
			set_data_count(xml::tag_buffer(*it).get_int());
			break;
		}
	}

	return true;
}

bool qdMinigameConfigParameter::save_script(Common::WriteStream &fh, int indent) const {
	for (int i = 0; i < indent; i++) {
		fh.writeString("\t");
	}
	fh.writeString("<minigame_config_prm");

	fh.writeString(Common::String::format(" name=\"%s\"", qdscr_XML_string(name())));
//	fh < " type=\"" <= int(_data_type) < "\"";

//	if(!comment_.empty())
//		fh < " comment=\"" < qdscr_XML_string(comment()) < "\"";

	if (_data_count > 1) {
		fh.writeString(Common::String::format(" size=\"%d\"", _data_count));
	}

	fh.writeString(">");

	fh.writeString(qdscr_XML_string(data_string()));

	fh.writeString("</minigame_config_prm>\r\n");

	return true;

}

bool qdMinigameConfigParameter::validate_data() {
	_is_data_valid = true;
	return true;
}

bool qdMinigameConfigParameter::load_ini(const Common::Path ini_file, const char *ini_section) {
	set_name(ini_section);
	Common::String str = getIniKey(ini_file, ini_section, "type");
	if (!str.empty()) {
		if (str.equalsIgnoreCase("string"))
			set_data_type(PRM_DATA_STRING);
		else if (str.equalsIgnoreCase("file"))
			set_data_type(PRM_DATA_FILE);
		else if (str.equalsIgnoreCase("object"))
			set_data_type(PRM_DATA_OBJECT);
	}

	str = getIniKey(ini_file, ini_section, "count");
	if (!str.empty())
		set_data_count(atoi(str.c_str()));

	str = getIniKey(ini_file, ini_section, "value");
	if (!str.empty()) {
		set_data_string(str.c_str());
	}

	str = getIniKey(ini_file, ini_section, "comment");
	if (!str.empty())
		set_comment(str.c_str());

	return true;
}
} // namespace QDEngine
