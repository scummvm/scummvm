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

/**
 * Provides convenient access to a subfolder within the engine data
 * zip file for the mm engine, by mapping it into ScummVM's filesystem.
 * You simply specify the subfolder for your game, and the contents of
 * that folder will be mapped in so you can calling Common::File::open
 * with their filenames directly like file1.png or data/file1.png,
 * depending on the useDataPrefix flag passed in the function below.
 *
 * The mm engine has an associated data zip file whose contents are
 * in /devtools/create_mm/files/. Each game has their own subfolder
 * within it, and as part of the release process, this folder will be
 * zipped up as a file 'mm.dat', and included with ScummVM releases.
 *
 * For development purposes, if you go the Game Options dialog for a
 * game in the ScummVM launcher, and set the extra path to
 * /devtools/create_mm, the engine will access the files directly
 * without requiring you to keep regenerating mm.dat.
 *
 */

#ifndef MM_UTILS_ENGINE_DATA_H
#define MM_UTILS_ENGINE_DATA_H

#include "common/str.h"

namespace MM {

/**
 * Loads the engine data file and maps it into the filessystem
 * so that the files within it can be opened as normal files.
 * @param subfolder		Subfolder in the engine data for the game
 * @param reqMajorVersion	Required major version number the
 * folder contents need to be (as specified in version.txt)
 * @param reqMinorVersion   Required minor version
 * @param errorMsg		If an error occurs, contains a description
 * @param useDataPrefix	If true, the subfolder gets mapped into
 * ScummVM as a 'data' subfolder, to differentiate them from files
 * directly in the game folder. If false, the default, then filenames\
 */
extern bool load_engine_data(const Common::String &subfolder,
	int reqMajorVersion, int reqMinorVersion,
	Common::U32String &errorMsg, bool useDataPrefix = false);

} // namespace MM

#endif
