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
 * $URL$
 * $Id$
 *
 */


#include "stdafx.h"
#include "common/scummsys.h"
//#include "graphics/scaler.h"
#include "common/system.h"
#include "backends/intern.h"

#include "gp32std.h"
#include "gp32std_file.h"

FILE *gp_stderr = NULL;
FILE *gp_stdout = NULL;
FILE *gp_stdin = NULL;

// Cache Idea / Code borrowed from the ps2 port
#define USE_CACHE

//////////////////
//File functions


// CACHE
inline bool gp_cacheInPos(GPFILE *stream) {
	return (stream->cachePos <= stream->filePos && stream->filePos < stream->cachePos + stream->bytesInCache);
}

int gp_cacheMiss(GPFILE *stream) {
	unsigned long readcount = 0;

	int copyLen = stream->fileSize - stream->filePos;
	if (copyLen > FCACHE_SIZE)
		copyLen = FCACHE_SIZE;

	stream->cachePos = stream->filePos;
	stream->cacheBufOffs = 0;
	stream->bytesInCache = copyLen;

	ERR_CODE err = GpFileRead(stream->handle, stream->cacheData, copyLen, &readcount);

	stream->physFilePos += copyLen;

	return err;
}

int gp_flushWriteCache(GPFILE *stream) {
	if (stream->bytesInCache == 0)
		return 0;

	ERR_CODE err = GpFileWrite(stream->handle, stream->cacheData, stream->bytesInCache); // flush cache

	stream->filePos += stream->bytesInCache;
	stream->physFilePos += stream->bytesInCache;
	stream->bytesInCache = 0;

	return err;
}

///////////////////////////////////////////////////////////////

GPFILE *gp_fopen(const char *fileName, const char *openMode) {
	uint32 mode;
	GPFILE *file;
	ERR_CODE err;
	char tempPath[256];

	if (!strchr(fileName, '.')) {
		sprintf(tempPath, "%s.", fileName);
		fileName = tempPath;
	}

	file = new GPFILE;

//	NP("%s(\"%s\", \"%s\")", __FUNCTION__, fileName, openMode);

	// FIXME add binary/text support
	if (tolower(openMode[0]) == 'r') {
		mode = OPEN_R;
		GpFileGetSize(fileName, &file->fileSize);
		err = GpFileOpen(fileName, mode, &file->handle);
	} else if (tolower(openMode[0]) == 'w') {
		file->fileSize = 0;
		mode = OPEN_W;
		err = GpFileCreate(fileName, ALWAYS_CREATE, &file->handle);
	} else if (tolower(openMode[0]) == 'a') {
		warning("We do not support 'a' file open mode.");
		delete file;
		return NULL;
	} else {
		error("wrong file mode");
	}

	if (!file) {
		error("%s: cannot create FILE structure", __FUNCTION__);
	}

	if (err) {
		printf("gp_fopen(): IO error %d", err);
		delete file;
		return NULL;
	}

	file->mode = mode;
	file->cachePos = 0;
	file->filePos = 0;
	file->cacheBufOffs = 0;
	file->physFilePos = 0;
	file->bytesInCache = 0;

	return file;
}

int gp_fclose(GPFILE *stream) {
	if (!stream) {
		//warning("closing null file");
		return 1;
	}

/*	if (*(uint32 *)((char *)stream - sizeof(uint32)) == 0x4321) {
		debug(0, "Double closing", __FUNCTION__);
		return 1;
	}
*/

#ifdef USE_CACHE
	if (stream->bytesInCache && stream->mode == OPEN_W) {
		gp_flushWriteCache(stream);
	}
#endif

	ERR_CODE err = GpFileClose(stream->handle);
	delete stream;

	return err;
}

int gp_fseek(GPFILE *stream, long offset, int whence) {
	switch (whence) {
	case SEEK_SET:
		whence = FROM_BEGIN;
		break;
	case SEEK_CUR:
		whence = FROM_CURRENT;
		break;
	case SEEK_END:
		whence = FROM_END;
		break;
	}

	ERR_CODE err;
#ifdef USE_CACHE
	// need to flush cache
	if (stream->mode == OPEN_W) { // write
		gp_flushWriteCache(stream);
		err = GpFileSeek(stream->handle, whence, offset, (long *)&stream->filePos);
	} else { // read
		if (whence == SEEK_CUR)
			offset += stream->physFilePos - stream->filePos;

		err = GpFileSeek(stream->handle, whence, offset, (long *)&stream->physFilePos);
		stream->filePos = stream->physFilePos;

		if (!gp_cacheInPos(stream)) { // cache miss
			gp_cacheMiss(stream);
		}
	}
#endif

	return 1;
	//return 0;	//FIXME?
}

size_t gp_fread(void *ptr, size_t size, size_t n, GPFILE *stream) {
	unsigned int len = size * n;
	uint8 *dest = (uint8 *)ptr;

#ifdef USE_CACHE
	while (len && !gp_feof(stream)) {
		if (gp_cacheInPos(stream)) {
			uint32 startPos = (stream->cacheBufOffs + (stream->filePos - stream->cachePos)) % FCACHE_SIZE;
			uint32 copyLen = stream->bytesInCache - (stream->filePos - stream->cachePos);
			if (copyLen > len)
				copyLen = len;
			if (startPos + copyLen > FCACHE_SIZE)
				copyLen = FCACHE_SIZE - startPos;

			memcpy(dest, stream->cacheData + startPos, copyLen);

			stream->filePos += copyLen;
			dest += copyLen;
			len -= copyLen;
		} else { // cache miss or cache empty
			gp_cacheMiss(stream);
		}
	}
#else
	ulong readcount = 0;
	ERR_CODE err = GpFileRead(stream->handle, ptr, len, &readcount);
	stream->physFilePos += len;
	stream->filePos += len;
#endif

	return 1; //readcount / size;	//FIXME
}

size_t gp_fwrite(const void *ptr, size_t size, size_t n, GPFILE *stream) {
	int len = size * n;
	uint8 *srcBuf = (uint8 *)ptr;

	if (!stream) {
		//warning("writing to null file");
		return 0;
	}

#ifdef USE_CACHE
	while (len) {
		uint32 copyLen;
		if (stream->bytesInCache + len > FCACHE_SIZE)
			copyLen = FCACHE_SIZE - stream->bytesInCache;
		else
			copyLen = len;

		srcBuf += copyLen;
		len -= copyLen;

		if (stream->bytesInCache == FCACHE_SIZE) {
			gp_flushWriteCache(stream);
		}
	}
#else
	ERR_CODE err = GpFileWrite(stream->handle, ptr, len);
	if (!err)
		return n;
	else
		return -err;
#endif
	return 1;
}

long gp_ftell(GPFILE *stream) {
	ulong pos = 0;
	pos = stream->filePos;
	//ERR_CODE err = GpFileSeek(stream->handle, FROM_CURRENT, 0, (long*)&pos);
	return pos;
}

void gp_clearerr(GPFILE *stream)
{
}

int gp_feof(GPFILE *stream) {
	return (unsigned long)gp_ftell(stream) >= stream->fileSize;
}

char gp_fgetc(GPFILE *stream) {
	char c[1];

	gp_fread(&c[0], 1, 1, stream);
	return c[0];
}

char *gp_fgets(char *s, int n, GPFILE *stream) {
	int i = 0;

	while (!gp_feof(stream) && i < n) {
		gp_fread(&s[i], 1, 1, stream);
		if (s[i] == '\n') {
			s[i + 1] = 0;
			return s;
		}
		i++;
	}
	if (gp_feof(stream))
		return NULL;
	else
		return s;
}

int gp_fprintf(GPFILE *stream, const char *fmt, ...) {
	char s[256];
	va_list marker;

	va_start(marker, fmt);
	vsnprintf(s, 256, fmt, marker);
	va_end(marker);

	return gp_fwrite(s, 1, strlen(s), stream);
}

int gp_fflush(GPFILE *stream) {
	return 0;
}

int gp_ferror(GPFILE *stream) {
	return 0;
}
