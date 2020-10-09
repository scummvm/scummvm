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
#ifndef DRAGONS_BIGFILE_H
#define DRAGONS_BIGFILE_H

#include "common/array.h"
#include "common/file.h"

namespace Dragons {

class DragonsEngine;

struct FileInfo {
	Common::String filename;
	uint32 offset;
	uint32 size;
	FileInfo() {
		offset = 0;
		size = 0;
		filename = "";
	}
};

class BigfileArchive {
private:
	DragonsEngine *_vm;
	Common::File *_fd;
	uint16 _totalRecords;
	Common::Array<FileInfo> _fileInfoTbl;

public:
	BigfileArchive(DragonsEngine *vm, const char *filename);
	virtual ~BigfileArchive();

	byte *load(const char *filename, uint32 &dataSize);
	bool doesFileExist(const char *filename);

private:
	void loadFileInfoTbl();
	uint32 getResourceId(const char *filename);
};

} // End of namespace dragons

#endif //DRAGONS_BIGFILE_H
