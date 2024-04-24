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
 * Provides support for a simplified way of creating a data file for
 * an engine using a standard zip file, and mapping it into ScummVM's
 * filesystem.
 * 
 * This allows contents of the zip file to be accessed by calling
 * Common::File::open with their filenames directly, like file1.png
 * or data/file1.png.
 *
 * You should create a folder named /devtools/create_<engine>/files to
 * put your engine's files within, with any necessary extraction code
 * in /devtools/create_<engine>/, that will save the files in files/
 * 
 * For development purposes, if you go the Game Options dialog for a
 * game in the ScummVM launcher, and set the Extra Path in the Paths
 * tab to /devtools/create_<engine>, the engine will access the files
 * directly without requiring you to keep regenerating a data file.
 * And when you're finished, and want to create a final data file,
 * you can zip up the files/ folder, rename the .zip to have a .dat
 * extension, and save it in /dists/engine-data.
 *
 * For example, the mm engine has it's files in /devtools/create_mm/files,
 * and it's contents are zipped up and saved as /dists/engine-data/mm.dat.
 */

#ifndef COMMON_ENGINE_DATA_H
#define COMMON_ENGINE_DATA_H

#include "common/path.h"

namespace Common {

/**
 * Loads the engine data file and maps it into the filessystem
 * so that the files within it can be opened as normal files.
 * @param datFilename	.dat filename
 * @param subfolder		Subfolder in the engine data for the game
 * @param reqMajorVersion	Required major version number the
 * folder contents need to be (as specified in version.txt)
 * @param reqMinorVersion   Required minor version
 * @param errorMsg		If an error occurs, contains a description
 * @param useDataPrefix	If true, the subfolder gets mapped into
 * ScummVM as a 'data' subfolder, to differentiate them from files
 * directly in the game folder. If false, the default, then filenames\
 */
extern bool load_engine_data(const Common::Path &datFilename,
	const Common::String &subfolder, int reqMajorVersion, int reqMinorVersion,
	Common::U32String &errorMsg, bool useDataPrefix = false);

} // namespace Common

#endif
