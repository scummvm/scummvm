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

//=============================================================================
//
// Functions for exchanging configuration data between key-value tree and
// INI file.
//
//=============================================================================

#ifndef AGS_SHARED_UTIL_INI_UTIL_H
#define AGS_SHARED_UTIL_INI_UTIL_H

#include "ags/lib/std/map.h"
#include "ags/shared/util/string.h"

namespace AGS3 {
namespace AGS {
namespace Shared {

typedef std::map<String, String>         StringOrderMap;
typedef StringOrderMap::const_iterator   StrStrOIter;

typedef std::map<String, StringOrderMap> ConfigTree;
typedef ConfigTree::const_iterator       ConfigNode;

namespace IniUtil {

// Parse the contents of given file as INI format and insert values
// into the tree. The pre-existing tree items, if any, are NOT erased.
// Returns FALSE if the file could not be opened.
bool Read(const String &file, ConfigTree &tree);
// Serialize given tree to the stream in INI text format.
// The INI format suggests only one nested level (group - items).
// The first level values are treated as a global section items.
// The sub-nodes beyond 2nd level are ignored completely.
void Write(const String &file, const ConfigTree &tree);
// Serialize given tree to the string in INI text format.
// TODO: implement proper memory/string stream compatible with base Stream
// class and merge this with Write function.
void WriteToString(String &s, const ConfigTree &tree);
// Parse the contents of given source stream as INI format and merge
// with values of the given tree while doing only minimal replaces;
// write the result into destination stream.
// If item already exists, only value is overwrited, if section exists,
// new items are appended to the end of it; completely new sections are
// appended to the end of text.
// Source and destination streams may refer either to different objects,
// or same stream opened for both reading and writing.
// Returns FALSE if the file could not be opened for writing.
bool Merge(const String &file, const ConfigTree &tree);

} // namespace IniUtil

} // namespace Shared
} // namespace AGS
} // namespace AGS3

#endif
