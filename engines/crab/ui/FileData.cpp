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

/*
 * This code is based on the CRAB engine
 *
 * Copyright (c) Arvind Raja Yadav
 *
 * Licensed under MIT
 *
 */

#include "common/savefile.h"
#include "crab/crab.h"
#include "crab/ui/FileData.h"

namespace Crab {

using namespace pyrodactyl::ui;

SaveFileData::SaveFileData(const Common::String &file) {
	_path = file;

	// Extract String between _ and . For eg., CRAB_Autosave 1.unr -> Autosave 1
	// 4 => .unr
	size_t pos = file.findFirstOf('_');
	_name = file.substr(pos + 1, file.size() - (pos + 1) - 4);

	if (g_engine->getSaveFileManager()->exists(file)) {
		Common::InSaveFile *savefile = g_engine->getSaveFileManager()->openForLoading(file);

		uint64 len = savefile->size();
		uint8 *data = new uint8[len + 1];
		data[len] = '\0';

		savefile->read(data, len);

		XMLDoc conf(data);
		if (conf.ready()) {
			rapidxml::xml_node<char> *node = conf.doc()->first_node("save");
			if (nodeValid(node)) {
				loadStr(_diff, "diff", node);
				loadStr(_locId, "loc_id", node);
				loadStr(_locName, "loc_name", node);
				loadStr(_charName, "char_name", node);
				loadStr(_time, "time", node);
				loadStr(_preview, "preview", node);
				_blank = false;
			}
		}
	}
}

ModFileData::ModFileData(const Common::String &file) {

#if 0
	if (boost::filesystem::exists(filepath)) {
		XMLDoc conf(filepath.string());
		if (conf.ready()) {
			rapidxml::xml_node<char> *node = conf.doc()->first_node("config");
			if (nodeValid(node)) {
				loadStr(author, "author", node);
				loadStr(version, "version", node);
				loadStr(info, "info", node);
				loadStr(website, "website", node);
				loadStr(preview, "preview", node);
			}
		}
	}
#endif
}

} // End of namespace Crab
