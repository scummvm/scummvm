/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001-2006 The ScummVM project
 * Copyright (C) 2002 Ph0x - GP32 Backend
 * Copyright (C) 2003/2004 DJWillis - GP32 Backend
 * Copyright (C) 2005 Won Star - GP32 Backend
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
//#define USE_CACHE

//////////////////
//File functions

GPFILE *gp_fopen(const char *fileName, const char *openMode) {
	//FIXME: allocation, mode
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
		printf("gp_gopen(): IO error %d", err);
		delete file;
		return NULL;
	}

	file->mode = mode;
	file->cachePos = 0;
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
	if (stream->cachePos && stream->mode == OPEN_W) {
		GpFileWrite(stream->handle, (char *)stream->cacheData, stream->cachePos); // flush cache
		stream->cachePos = 0;
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
	return GpFileSeek(stream->handle, whence, offset, (long *)&stream->filePos);	//FIXME?
}

size_t gp_fread(void *ptr, size_t size, size_t n, GPFILE *stream) {
	ulong readcount = 0;
	int len = size * n;
	
#ifdef USE_CACHE
	uint8 *dest = (uint8 *)ptr;

	while (len && (stream->filePos != stream->fileSize)) {
		if (stream->cachePos <= filePos && filePos < stream->cachePos + stream->bytesInCache) {
			uint32 startPos = (stream->cacheBufOffs + (stream->filePos - stream->cachePos)) % FCACHE_SIZE;
			uint32 copyLen = stream->bytesInCache - (stream->filePos - stream->cachePos);
			if (copyLen > len)
				copyLen = len;
			if (startPos + copyLen > FCACHE_SIZE)
				copyLen = FCACHE_SIZE - startPos;

			memcpy(dest, cacheData + startPos, copyLen);

			filePos += copyLen;
			dest += copyLen;
			len -= copyLen;
		} else {
#endif
			ERR_CODE err = GpFileRead(stream->handle, ptr, len, &readcount);

#ifdef USE_CACHE
			stream->filePos += len;
		}
	}
#endif

	return readcount / size;	//FIXME?
}

size_t gp_fwrite(const void *ptr, size_t size, size_t n, GPFILE *stream) {
	int len = size * n;

	if (!stream) {
		//warning("writing to null file");
		return 0;
	}

#ifdef USE_CACHE
	if (stream->cachePos + len < FCACHE_SIZE) {
		memcpy(stream->cacheData + stream->cachePos, ptr, len);
		stream->cachePos += len;
	} else {
		if (stream->cachePos) {
			GpFileWrite(stream->handle, stream->cacheData, stream->cachePos);	// flush cache
			stream->cachePos = 0;
		}

#endif
		ERR_CODE err = GpFileWrite(stream->handle, ptr, len);
		if (!err)
			return n;
		else
			return -err;
#ifdef USE_CACHE
	}
#endif
	return 0;
}

//FIXME? use standard func
long gp_ftell(GPFILE *stream) {
	ulong pos = 0;
	ERR_CODE err = GpFileSeek(stream->handle, FROM_CURRENT, 0, (long*)&pos);
	return pos;
}

void gp_clearerr(GPFILE *stream)
{
}

int gp_feof(GPFILE *stream) {
	return gp_ftell(stream) >= stream->fileSize;
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
