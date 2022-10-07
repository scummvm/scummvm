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

#include "watchmaker/file_utils.h"
#include "watchmaker/t3d.h"

Common::String readT3dString(Common::SeekableReadStream& stream) {
	char strbuf[T3D_NAMELEN + 1] = {};
	for (int i = 0; i < T3D_NAMELEN; i++) {
		strbuf[i] = stream.readByte();
	}
	return Common::String(strbuf);
}

bool hasFileExtension(const Common::String &str, const Common::String &extension) {
	return str.hasSuffixIgnoreCase("." + extension);
}

bool hasFileExtension(const char *str, Common::String extension) {
	Common::String string(str);
	return hasFileExtension(string, extension);
}
