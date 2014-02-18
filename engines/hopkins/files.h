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

#ifndef HOPKINS_FILES_H
#define HOPKINS_FILES_H

#include "common/scummsys.h"
#include "common/hash-str.h"
#include "common/str.h"
#include "common/stream.h"

namespace Hopkins {

class HopkinsEngine;

// RES_ANI = 4 has been removed because it's not used
enum CatMode { RES_INI = 1, RES_REP = 2, RES_LIN = 3, RES_PER = 5,
               RES_PIC = 6, RES_SAN = 7, RES_SLI = 8, RES_VOI = 9 };

class FileManager {
public:
	uint32 _catalogPos;
	uint32 _catalogSize;

	HopkinsEngine *_vm;

	FileManager(HopkinsEngine *vm);

	bool fileExists(const Common::String &file);
	byte *loadFile(const Common::String &file);
	int readStream(Common::ReadStream &stream, void *buf, size_t nbytes);
	void initCensorship();
	byte *searchCat(const Common::String &file, CatMode mode, bool &fileFoundFl);
	uint32 fileSize(const Common::String &filename);
};

} // End of namespace Hopkins

#endif /* HOPKINS_GLOBALS_H */
