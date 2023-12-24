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

#ifndef M4_FILEIO_FILEIO_H
#define M4_FILEIO_FILEIO_H

#include "common/stream.h"

namespace M4 {

enum AccessMode {
	UNOPENED,
	READ,
	WRITE,
	READ_OR_WRITE
};

enum FileMode {
	BINARY,
	TEXT,
	BINARYW,
	TEXTW
};

struct Hag_Name_Record {
	char filename[33];
	byte hagfile;
	struct Hag_Name_Record *next;
};

struct Hag_Record {
	char hag_name[33];
	byte  hagfile;
	Common::Stream *hag_fp;
	uint32  hag_pos;
	Hag_Record *next;
};

Common::Stream *f_io_open(const Common::Path &filename, const Common::String &mode);
void f_io_close(Common::Stream *stream);

} // namespace M4

#endif
