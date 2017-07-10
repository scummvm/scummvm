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
#include "sludge/stringy.h"
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

char *readString(Common::SeekableReadStream *stream) {
	int a, len = stream->readUint16BE();
	char *s = new char[len + 1];
	if (!checkNew(s)) {
		return NULL;
	}
	for (a = 0; a < len; a++) {
		s[a] = (char)(stream->readByte() - 1);
	}
	s[len] = 0;
	debug(kSludgeDebugDataLoad, "Read string of length %i: %s", len, s);
	return s;
}

char *encodeFilename(char *nameIn) {
	if (!nameIn)
		return NULL;
	if (allowAnyFilename) {
		char *newName = new char[strlen(nameIn) * 2 + 1];
		if (!checkNew(newName))
			return NULL;

		int i = 0;
		while (*nameIn) {
			switch (*nameIn) {
				case '<':
					newName[i++] = '_';
					newName[i++] = 'L';
					break;
				case '>':
					newName[i++] = '_';
					newName[i++] = 'G';
					break;
				case '|':
					newName[i++] = '_';
					newName[i++] = 'P';
					break;
				case '_':
					newName[i++] = '_';
					newName[i++] = 'U';
					break;
				case '\"':
					newName[i++] = '_';
					newName[i++] = 'S';
					break;
				case '\\':
					newName[i++] = '_';
					newName[i++] = 'B';
					break;
				case '/':
					newName[i++] = '_';
					newName[i++] = 'F';
					break;
				case ':':
					newName[i++] = '_';
					newName[i++] = 'C';
					break;
				case '*':
					newName[i++] = '_';
					newName[i++] = 'A';
					break;
				case '?':
					newName[i++] = '_';
					newName[i++] = 'Q';
					break;

				default:
					newName[i++] = *nameIn;
					break;
			}
			newName[i] = 0;
			nameIn++;
		}
		return newName;
	} else {
		int a;
		for (a = 0; nameIn[a]; a++) {
			if (nameIn[a] == '\\')
				nameIn[a] = '/';
		}

		return copyString(nameIn);
	}
}

char *decodeFilename(char *nameIn) {
	if (allowAnyFilename) {
		char *newName = new char[strlen(nameIn) + 1];
		if (!checkNew(newName))
			return NULL;

		int i = 0;
		while (*nameIn) {
			if (*nameIn == '_') {
				nameIn++;
				switch (*nameIn) {
					case 'L':
						newName[i] = '<';
						nameIn++;
						break;
					case 'G':
						newName[i] = '>';
						nameIn++;
						break;
					case 'P':
						newName[i] = '|';
						nameIn++;
						break;
					case 'U':
						newName[i] = '_';
						nameIn++;
						break;
					case 'S':
						newName[i] = '\"';
						nameIn++;
						break;
					case 'B':
						newName[i] = '\\';
						nameIn++;
						break;
					case 'F':
						newName[i] = '/';
						nameIn++;
						break;
					case 'C':
						newName[i] = ':';
						nameIn++;
						break;
					case 'A':
						newName[i] = '*';
						nameIn++;
						break;
					case 'Q':
						newName[i] = '?';
						nameIn++;
						break;
					default:
						newName[i] = '_';
				}
			} else {
				newName[i] = *nameIn;
				nameIn++;
			}
			i++;

		}
		newName[i] = 0;
		return newName;
	} else {
		return copyString(nameIn);
	}
}

} // End of namespace Sludge
