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

#include "crab/FileData.h"

namespace Crab {

include <iomanip>

using namespace pyrodactyl::ui;

FileData::FileData(const boost::filesystem::path &filepath) {
	if (boost::filesystem::exists(filepath)) {
		name = boost::filesystem::basename(filepath);
		path = filepath.string();

#if defined(__WIN32__) || defined(__APPLE__)
		std::time_t temp = boost::filesystem::last_write_time(filepath);
		last_modified = NumberToString(std::put_time(std::localtime(&temp), "%d %b %Y %H:%M:%S"));
#else
		// Workaround for GNU C++ not having implemented std::put_time
		std::time_t temp = boost::filesystem::last_write_time(filepath);
		char timestr[100];

		std::strftime(timestr, sizeof(timestr), "%d %b %Y %H:%M:%S", std::localtime(&temp));
		last_modified = timestr;
#endif
	} else
		name = "New Save";
}

SaveFileData::SaveFileData(const boost::filesystem::path &filepath) : FileData(filepath) {
	if (boost::filesystem::exists(filepath)) {
		XMLDoc conf(filepath.string());
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

ModFileData::ModFileData(boost::filesystem::path filepath) : FileData(filepath) {
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
}

} // End of namespace Crab
