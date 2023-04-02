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

#ifndef CRAB_FILEDATA_H
#define CRAB_FILEDATA_H

#include "crab/XMLDoc.h"
#include "crab/common_header.h"
#include "crab/loaders.h"

namespace Crab {

namespace pyrodactyl {
namespace ui {
class FileData {
public:
	std::string name, path, last_modified;
	//FileData(const boost::filesystem::path &filepath);
};

class SaveFileData : public FileData {
public:
	std::string loc_id, loc_name, char_name, diff, time, preview;

	// This is to account for the first save slot, called "New Save", which doesn't actually have a file
	bool blank;

	//SaveFileData(const boost::filesystem::path &filepath);
	SaveFileData(const bool empty);
};

class ModFileData : public FileData {
public:
	std::string author, version, info, website, preview;
	//ModFileData(boost::filesystem::path filepath);
};

// The types of data shown about the save file
enum { DATA_SAVENAME,
	   DATA_LASTMODIFIED,
	   DATA_BUTTON_TOTAL };

// Both of these are capped at DATA_HOVER_TOTAL
enum { DATA_LOCNAME,
	   DATA_DIFFICULTY,
	   DATA_TIMEPLAYED,
	   DATA_PLAYERNAME };
enum { DATA_AUTHOR,
	   DATA_VERSION,
	   DATA_INFO,
	   DATA_WEBSITE };
const int DATA_HOVER_TOTAL = 4;
} // End of namespace ui
} // End of namespace pyrodactyl

} // End of namespace Crab

#endif // CRAB_FILEDATA_H
