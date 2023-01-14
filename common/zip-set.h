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

#ifndef COMMON_ZIP_SET_H
#define COMMON_ZIP_SET_H

#include "common/archive.h"

namespace Common {

/**
 * Scan the given directory for a set of versioned zip packs and put them
 * into the provided SearchSet
 *
 * There are 2 types of files: defaultFile, that could potentially sit
 * in the themepath or even be built-in into the executable; and so called
 * pack files typically containing creation date in their names.
 *
 * These files are put in the SearchSet in a sorted way, thus, the latter
 * packs have higher priority.
 *
 * Example: gui-icons.dat, gui-icons-20211112.dat, gui-icons-20220602.dat
 *
 * Used by: icons packs and shader packs
 *
 * @param[out] searchSet    The SearchSet to modify
 * @param[in]  defaultFile  Name of the default file
 * @param[in]  packsMask    Mask of the pack files
 * @param[in]  packsPath    Name of ConfMan variable with Path to the packs. Default is "iconspath""
 *
 * @return True if the string has been parsed correctly, false if an error
 *
 */
bool generateZipSet(SearchSet &searchSet, const char *defaultFile, const char *packsMask, const char *packsPath = "iconspath");

}

#endif
