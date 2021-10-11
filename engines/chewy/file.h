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

#ifndef CHEWY_FILE_H
#define CHEWY_FILE_H

#include "common/file.h"
#include "common/textconsole.h"

namespace Chewy {

using Common::Stream;

class File {
public:
	/**
	 * Opens up a file, returning it if successful,
	 * or nullptr if not
	 */
	static Common::File *open(const char *name);

	/**
	 * Reads an array of uint16 values
	 */
	static bool readArray(Common::SeekableReadStream *src,
		uint16 *arr, size_t size);
};

inline Stream *chewy_fopen(const char *fname, const char *mode) {
	assert(!strcmp(mode, "rb"));
	return File::open(fname);
}

inline void chewy_fclose(Stream *&stream) {
	delete stream;
	stream = nullptr;
}

inline size_t chewy_fread(void *ptr, size_t size, size_t nmemb, Stream *stream) {
	Common::SeekableReadStream *rs = dynamic_cast<Common::SeekableReadStream *>(stream);
	assert(rs);
	return rs->read(ptr, size * nmemb) / size;
}

inline int chewy_fseek(Stream *stream, long int offset, int whence) {
	Common::SeekableReadStream *rs = dynamic_cast<Common::SeekableReadStream *>(stream);
	assert(rs);
	return rs->seek(offset, whence);
}

inline int chewy_fgetc(Stream *stream) {
	Common::SeekableReadStream *rs = dynamic_cast<Common::SeekableReadStream *>(stream);
	assert(rs);
	return rs->readByte();
}

inline size_t chewy_ftell(Stream *stream) {
	Common::SeekableReadStream *rs = dynamic_cast<Common::SeekableReadStream *>(stream);
	assert(rs);
	return rs->size();
}

inline size_t chewy_fwrite(void *ptr, size_t size, size_t nmemb, Stream *stream) {
	::error("TODO: chewy_fwrite");
	return 0;
}

inline int chewy_fputc(byte c, Stream *stream) {
	::error("TODO: chewy_fputc");
	return 0;
}

} // namespace Chewy

#endif
