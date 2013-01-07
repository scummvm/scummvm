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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef HOPKINS_FILES_H
#define HOPKINS_FILES_H

#include "common/scummsys.h"
#include "common/hash-str.h"
#include "common/str.h"
#include "common/stream.h"

namespace Hopkins {

class HopkinsEngine;

class FileManager {
public:
	HopkinsEngine *_vm;

	FileManager();
	void setParent(HopkinsEngine *vm);

	bool fileExists(const Common::String &folder, const Common::String &file);
	byte *loadFile(const Common::String &file);
	int readStream(Common::ReadStream &stream, void *buf, size_t nbytes);
	void initCensorship();
	void constructFilename(const Common::String &folder, const Common::String &file);

	byte *searchCat(const Common::String &file, int a2);
	void constructLinuxFilename(const Common::String &file);

	uint32 fileSize(const Common::String &filename);
};

} // End of namespace Hopkins

#endif /* HOPKINS_GLOBALS_H */
