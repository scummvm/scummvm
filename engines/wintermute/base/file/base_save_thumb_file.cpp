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

/*
 * This file is based on WME Lite.
 * http://dead-code.org/redir.php?target=wmelite
 * Copyright (c) 2011 Jan Nedoma
 */

#include "engines/wintermute/base/base_persistence_manager.h"
#include "engines/wintermute/base/file/base_save_thumb_file.h"
#include "engines/wintermute/base/base.h"
#include "common/memstream.h"

namespace Wintermute {

Common::SeekableReadStream *openThumbFile(const Common::String &filename) {
	if (scumm_strnicmp(filename.c_str(), "savegame:", 9) != 0) {
		return nullptr;
	}

	size_t filenameSize = strlen(filename.c_str()) - 9 + 1;
	char *tempFilename = new char[filenameSize];
	Common::strcpy_s(tempFilename, filenameSize, filename.c_str() + 9);
	for (uint32 i = 0; i < strlen(tempFilename); i++) {
		if (tempFilename[i] < '0' || tempFilename[i] > '9') {
			tempFilename[i] = '\0';
			break;
		}
	}

	// get slot number from name
	int slot = atoi(tempFilename);
	delete[] tempFilename;

	BasePersistenceManager *pm = new BasePersistenceManager();
	if (!pm) {
		return nullptr;
	}

	Common::String slotFilename = pm->getFilenameForSlot(slot);

	if (DID_FAIL(pm->initLoad(slotFilename))) {
		delete pm;
		return nullptr;
	}

	if (pm->_thumbnailDataSize == 0) {
		delete pm;
		return nullptr;
	}

	uint32 size = pm->_thumbnailDataSize;
	byte *data = (byte *)malloc(size);
	memcpy(data, pm->_thumbnailData, size);
	delete pm;

	return new Common::MemoryReadStream(data, size, DisposeAfterUse::YES);
}

} // End of namespace Wintermute
