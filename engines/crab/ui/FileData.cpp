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

#include "crab/crab.h"
#include "crab/common_header.h"
#include "crab/ui/FileData.h"

namespace Crab {

using namespace pyrodactyl::ui;

SaveFileData::SaveFileData(const Common::String &file) {
	path = file;

	// Extract String between _ and . For eg., CRAB_Autosave 1.unr -> Autosave 1
	// 4 => .unr
	size_t pos = file.findFirstOf('_');
	name = file.substr(pos + 1, file.size() - (pos + 1) - 4);

	if (g_engine->getSaveFileManager()->exists(file)) {
		Common::InSaveFile *savefile = g_engine->getSaveFileManager()->openForLoading(file);

		uint64 len = savefile->size();
		uint8 *data = new uint8[len + 1];
		data[len] = '\0';

		savefile->read(data, len);

		XMLDoc conf(data);
		if (conf.ready()) {
			rapidxml::xml_node<char> *node = conf.Doc()->first_node("save");
			if (NodeValid(node)) {
				LoadStr(diff, "diff", node);
				LoadStr(loc_id, "loc_id", node);
				LoadStr(loc_name, "loc_name", node);
				LoadStr(char_name, "char_name", node);
				LoadStr(time, "time", node);
				LoadStr(preview, "preview", node);
				blank = false;
			}
		}
	}
}

ModFileData::ModFileData(const Common::String &file) {

#if 0
	if (boost::filesystem::exists(filepath)) {
		XMLDoc conf(filepath.string());
		if (conf.ready()) {
			rapidxml::xml_node<char> *node = conf.Doc()->first_node("config");
			if (NodeValid(node)) {
				LoadStr(author, "author", node);
				LoadStr(version, "version", node);
				LoadStr(info, "info", node);
				LoadStr(website, "website", node);
				LoadStr(preview, "preview", node);
			}
		}
	}
#endif
}

} // End of namespace Crab
