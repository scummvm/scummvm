/*
** Gobliiins 1
** Original game by CoktelVision
**
** Reverse engineered by Ivan Dubrov <WFrag@yandex.ru>
**
*/
#ifndef __DATAIO_H
#define __DATAIO_H

#include "common/file.h"
#include <sys/stat.h>

namespace Gob {

#define MAX_DATA_FILES	3
#define MAX_SLOT_COUNT	4

struct ChunkDesc {
	char chunkName[13];
	uint32 size;
	uint32 offset;
	byte packed;
};

int16 file_open(const char *path, File::AccessMode mode = File::kFileReadMode);
File *file_getHandle(int16 handle);
int16 data_getChunk(const char *chunkName);
char data_freeChunk(int16 handle);
int32 data_readChunk(int16 handle, char *buf, int16 size);
int16 data_seekChunk(int16 handle, int32 pos, int16 from);
int32 data_getChunkSize(const char *chunkName);
void data_openDataFile(const char *src);
void data_closeDataFile(void);
char *data_getUnpackedData(const char *name);
void data_closeData(int16 handle);
int16 data_openData(const char *path, File::AccessMode mode = File::kFileReadMode);
int32 data_readData(int16 handle, char *buf, int16 size);
void data_seekData(int16 handle, int32 pos, int16 from);
int32 data_getDataSize(const char *name);
char *data_getData(const char *path);
char *data_getSmallData(const char *path);

}				// End of namespace Gob

#endif
