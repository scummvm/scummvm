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

#ifndef __PS2FILE_IO__
#define __PS2FILE_IO__

typedef unsigned long uint64;
typedef signed long   int64;

#include <stdio.h>
#include "common/scummsys.h"


#define CACHE_SIZE (2048 * 32)
#define MAX_READ_STEP (2048 * 16)
#define MAX_CACHED_FILES 6
#define CACHE_READ_THRESHOLD (16 * 2048)
#define CACHE_FILL_MIN (2048 * 24)
#define READ_ALIGN 64   // align all reads to the size of an EE cache line
#define READ_ALIGN_MASK (READ_ALIGN - 1)


class Ps2File {
public:
	Ps2File(void);
	virtual ~Ps2File(void);
	virtual bool open(const char *name, int mode);
	virtual uint32 read(void *dest, uint32 len);
	virtual uint32 write(const void *src, uint32 len);
	virtual int32 tell(void);
	virtual int32 size(void);
	virtual int seek(int32 offset, int origin);
	virtual bool eof(void);
	virtual bool getErr(void);
	virtual void setErr(bool);


private:
	void cacheReadAhead(void);
	void cacheReadSync(void);

	int _fd;
	uint32 _mode;
	uint32 _fileSize;
	uint32 _filePos;
	uint32 _cacheSize;
	uint32 _cachePos;

	uint8 *_cache;

	int _eof;
	int _sema;


	uint8 *_cacheBuf;
	bool _cacheOpRunning;
	uint32 _physFilePos;
	uint32 _bytesInCache, _cacheOfs;

	uint32 _readBytesBlock;
	bool _stream;
};

FILE *ps2_fopen(const char *fname, const char *mode);
int ps2_fclose(FILE *stream);
int ps2_fflush(FILE *stream);
int ps2_fseek(FILE *stream, long offset, int origin);
uint32 ps2_ftell(FILE *stream);
int ps2_feof(FILE *stream);

size_t ps2_fread(void *buf, size_t r, size_t n, FILE *stream);
int ps2_fgetc(FILE *stream);
char *ps2_fgets(char *buf, int n, FILE *stream);

size_t ps2_fwrite(const void *buf, size_t r, size_t n, FILE *stream);
int ps2_fputc(int c, FILE *stream);
int ps2_fputs(const char *s, FILE *stream);
int ps2_fprintf(FILE *pOut, const char *zFormat, ...);

int ps2_ferror(FILE *stream);
void ps2_clearerr(FILE *stream);

#endif // __PS2FILE_IO__

