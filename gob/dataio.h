/* ScummVM - Scumm Interpreter
 * Copyright (C) 2004 Ivan Dubrov
 * Copyright (C) 2004-2005 The ScummVM project
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */
#ifndef GOB_DATAIO_H
#define GOB_DATAIO_H

#include "common/file.h"

namespace Gob {

#define MAX_DATA_FILES	3
#define MAX_SLOT_COUNT	4

struct ChunkDesc {
	char chunkName[13];
	uint32 size;
	uint32 offset;
	byte packed;
};

int16 file_open(const char *path, Common::File::AccessMode mode = Common::File::kFileReadMode);
Common::File *file_getHandle(int16 handle);
int16 data_getChunk(const char *chunkName);
char data_freeChunk(int16 handle);
int32 data_readChunk(int16 handle, char *buf, int16 size);
int16 data_seekChunk(int16 handle, int32 pos, int16 from);
int32 data_getChunkSize(const char *chunkName);
void data_openDataFile(const char *src);
void data_closeDataFile(void);
char *data_getUnpackedData(const char *name);
void data_closeData(int16 handle);
int16 data_openData(const char *path, Common::File::AccessMode mode = Common::File::kFileReadMode);
int32 data_readData(int16 handle, char *buf, int16 size);
void data_seekData(int16 handle, int32 pos, int16 from);
int32 data_getDataSize(const char *name);
char *data_getData(const char *path);
char *data_getSmallData(const char *path);

}				// End of namespace Gob

#endif
