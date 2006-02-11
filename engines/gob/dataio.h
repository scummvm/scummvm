/* ScummVM - Scumm Interpreter
 * Copyright (C) 2004 Ivan Dubrov
 * Copyright (C) 2004-2006 The ScummVM project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */
#ifndef GOB_DATAIO_H
#define GOB_DATAIO_H

#include "common/file.h"
#include "gob/gob.h"

namespace Gob {

#define MAX_DATA_FILES	3
#define MAX_SLOT_COUNT	4

class DataIO {
public:
	struct ChunkDesc {
		char chunkName[13];
		uint32 size;
		uint32 offset;
		byte packed;
		ChunkDesc() : size(0), offset(0), packed(0) { chunkName[0] = 0; }
	};

	int16 file_open(const char *path, Common::File::AccessMode mode = Common::File::kFileReadMode);
	Common::File *file_getHandle(int16 handle);
	int16 getChunk(const char *chunkName);
	char freeChunk(int16 handle);
	int32 readChunk(int16 handle, char *buf, int16 size);
	int16 seekChunk(int16 handle, int32 pos, int16 from);
	int32 getChunkSize(const char *chunkName);
	void openDataFile(const char *src);
	void closeDataFile(void);
	char *getUnpackedData(const char *name);
	void closeData(int16 handle);
	int16 openData(const char *path, Common::File::AccessMode mode = Common::File::kFileReadMode);
	int32 readData(int16 handle, char *buf, int16 size);
	void seekData(int16 handle, int32 pos, int16 from);
	int32 getDataSize(const char *name);
	char *getData(const char *path);
	char *getSmallData(const char *path);

	DataIO(class GobEngine *vm);

protected:
	class GobEngine *_vm;
};

}				// End of namespace Gob

#endif
