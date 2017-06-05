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
#include "allfiles.h"
#include "moreio.h"
#include "newfatal.h"
#include "stringy.h"

#include "sludge.h"

#include "common/debug.h"
#include "common/file.h"

#if defined __unix__ && !(defined __APPLE__)
#include <endian.h>
#if __BYTE_ORDER == __BIG_ENDIAN
#define __BIG_ENDIAN__
#endif
#endif

namespace Sludge {

bool allowAnyFilename = true;

void writeString(char *s, Common::WriteStream *stream) {
	int a, len = strlen(s);
	stream->writeUint16BE(len);
	for (a = 0; a < len; ++a) {
		stream->writeByte(s[a] + 1);
	}
}

char *readString(Common::SeekableReadStream *stream) {
	int a, len = stream->readUint16BE();
	char *s = new char[len + 1];
	if (!checkNew(s)) {
		return NULL;
	}
	for (a = 0; a < len; ++a) {
		s[a] = (char)(stream->readByte() - 1);
	}
	s[len] = 0;
	debug(kSludgeDebugDataLoad, "Read string of length %i: %s", len, s);
	return s;
}

float floatSwap(float f) {
	union {
		float f;
		unsigned char b[4];
	} dat1, dat2;

	dat1.f = f;
	dat2.b[0] = dat1.b[3];
	dat2.b[1] = dat1.b[2];
	dat2.b[2] = dat1.b[1];
	dat2.b[3] = dat1.b[0];
	return dat2.f;
}

float getFloat(Common::SeekableReadStream *stream) {
	float f;
	size_t bytes_read = stream->read(&f, sizeof(float));
	if (bytes_read != sizeof(float) && stream->err()) {
		debug("Reading error in getFloat.\n");
	}

#ifdef  __BIG_ENDIAN__
	return floatSwap(f);
#else
	return f;
#endif
}

void putFloat(float f, Common::WriteStream *stream) {
#ifdef  __BIG_ENDIAN__
	f = floatSwap(f);
#endif
	stream->write(&f, sizeof(float));
}

short shortSwap(short s) {
	unsigned char b1, b2;

	b1 = s & 255;
	b2 = (s >> 8) & 255;

	return (b1 << 8) + b2;
}

short getSigned(Common::SeekableReadStream *stream) {
	short f;
	size_t bytes_read = stream->read(&f, sizeof(short));
	if (bytes_read != sizeof(short) && stream->err()) {
		debug("Reading error in getSigned.\n");
	}
#ifdef  __BIG_ENDIAN__
	f = shortSwap(f);
#endif
	return f;
}

void putSigned(short f, Common::WriteStream *stream) {
#ifdef  __BIG_ENDIAN__
	f = shortSwap(f);
#endif
	stream->write(&f, sizeof(short));
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
		for (a = 0; nameIn[a]; ++a) {
#ifdef _WIN32
			if (nameIn[a] == '/') nameIn[a] = '\\';
#else
			if (nameIn[a] == '\\')
				nameIn[a] = '/';
#endif
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
