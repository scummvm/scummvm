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

#include "bolt/bolt.h"
#include "bolt/xplib/xplib.h"

#include "common/file.h"

namespace Bolt {

void XpLib::fileError(const char *message) {
	stopCycle();
	error(message);
}

Common::File *XpLib::openFile(const char *fileName, int16 flags) {
	// Read-write (mode 3) and write (mode 2) are unsupported and not needed by Cartoon Carnival...
	assert(flags == 1);

	Common::File *file = new Common::File();
	if (!file->exists(fileName)) {
		fileError("File does not exist.");
	}

	file->open(fileName);
	if (!file->isOpen()) {
		fileError("Disc error.  The Disc may be dirty.");
		return nullptr;
	}

	return file;
}

void XpLib::closeFile(Common::File *handle) {
	if (handle->isOpen()) {
		handle->close();
	}
}

bool XpLib::readFile(Common::File *handle, void *buffer, uint32 *size) {
	uint32 sizeRead = handle->read(buffer, *size);
	bool readEntireSize = sizeRead == *size;
	*size = sizeRead;

	if (!readEntireSize) {
		fileError("Disc error.  The Disc may be dirty.");
	}

	return readEntireSize;
}

bool XpLib::setFilePos(Common::File *handle, int32 offset, int32 origin) {
	return handle->seek(offset + origin, SEEK_SET);
}

void *XpLib::allocMem(uint32 size) {
	void *result = malloc(size);
	if (!result)
		error("XpLib::allocMem(): Not enough memory");

	memset(result, 0, size);
	return result;
}

void *XpLib::tryAllocMem(uint32 size) {
	void *result = (void *)malloc(size);
	if (!result)
		warning("XpLib::tryAllocMem(): Couldn't allocate memory, returning nullptr");
	else
		memset(result, 0, size);

	return result;
}

void XpLib::freeMem(void *mem) {
	free(mem);
}

} // End of namespace Bolt
