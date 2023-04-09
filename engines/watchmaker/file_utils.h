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

#ifndef WATCHMAKER_FILE_UTILS_H
#define WATCHMAKER_FILE_UTILS_H

#include "common/stream.h"

namespace Watchmaker {

Common::String readT3dString(Common::SeekableReadStream &stream);
// TODO: This can probably just be replaced with .hasSuffixIgnoreCase
bool hasFileExtension(const Common::String &str, const Common::String &extension);
bool hasFileExtension(const char *str, Common::String extension);
Common::String replaceExtension(const char *str, const Common::String &extension);

} // End of namespace Watchmaker

#endif // WATCHMAKER_FILE_UTILS_H
