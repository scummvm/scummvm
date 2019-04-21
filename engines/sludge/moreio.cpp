/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "common/debug.h"
#include "common/file.h"

#include "sludge/allfiles.h"
#include "sludge/moreio.h"
#include "sludge/newfatal.h"
#include "sludge/sludge.h"

namespace Sludge {

bool allowAnyFilename = true;

void writeString(Common::String s, Common::WriteStream *stream) {
	int a, len = s.size();
	stream->writeUint16BE(len);
	for (a = 0; a < len; a++) {
		stream->writeByte(s[a] + 1);
	}
}

Common::String readString(Common::SeekableReadStream *stream) {
	int len = stream->readUint16BE();
	Common::String res = "";
	for (int a = 0; a < len; a++) {
		res += (char)(stream->readByte() - 1);
	}
	return res;
}

Common::String encodeFilename(const Common::String &nameIn) {
	Common::String newName = "";
	if (nameIn.empty())
		return newName;
	if (allowAnyFilename) {
		for (uint i = 0; i < nameIn.size(); ++i) {
			switch (nameIn[i]) {
				case '<':
					newName += '_';
					newName += 'L';
					break;
				case '>':
					newName += '_';
					newName += 'G';
					break;
				case '|':
					newName += '_';
					newName += 'P';
					break;
				case '_':
					newName += '_';
					newName += 'U';
					break;
				case '\"':
					newName += '_';
					newName += 'S';
					break;
				case '\\':
					newName += '_';
					newName += 'B';
					break;
				case '/':
					newName += '_';
					newName += 'F';
					break;
				case ':':
					newName += '_';
					newName += 'C';
					break;
				case '*':
					newName += '_';
					newName += 'A';
					break;
				case '?':
					newName += '_';
					newName += 'Q';
					break;

				default:
					newName += nameIn[i];
					break;
			}
		}
	} else {
		newName.clear();
		newName = nameIn;
		for (uint i = 0; i < newName.size(); ++i) {
			if (newName[i] == '\\')
				newName.setChar('/', i);
		}
	}
	return newName;
}

Common::String decodeFilename(const Common::String &nameIn) {
	Common::String newName ="";
	if (allowAnyFilename) {
		for (uint i = 0; i < nameIn.size(); ++i) {
			if (nameIn[i] == '_') {
				++i;
				switch (nameIn[i]) {
					case 'L':
						newName += '<';
						break;
					case 'G':
						newName += '>';
						break;
					case 'P':
						newName += '|';
						break;
					case 'U':
						newName += '_';
						break;
					case 'S':
						newName += '\"';
						break;
					case 'B':
						newName += '\\';
						break;
					case 'F':
						newName += '/';
						break;
					case 'C':
						newName += ':';
						break;
					case 'A':
						newName += '*';
						break;
					case 'Q':
						newName += '?';
						break;
					default:
						newName += '_';
						--i;
						break;
				}
			} else {
				newName += nameIn[i];
			}
		}
		return newName;
	} else {
		newName.clear();
		newName = nameIn;
	}
	return newName;
}

char *createCString(const Common::String &s) {
	uint n = s.size() + 1;
	char *res = new char[n];
	if (!checkNew(res)) {
		fatal("createCString : Unable to copy String");
		return NULL;
	}
	memcpy(res, s.c_str(), n);
	return res;
}

} // End of namespace Sludge
