/* ScummVM - Scumm Interpreter
 * Copyright (C) 2005-2006 The ScummVM project
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

#include "common/scummsys.h"

class Ps2File {
public:
	Ps2File(int64 cacheId);
	virtual ~Ps2File(void);
	virtual bool open(const char *name) = 0;
	virtual uint32 read(void *dest, uint32 len) = 0;
	virtual uint32 write(const void *src, uint32 len) = 0;
	virtual uint32 tell(void) = 0;
	virtual uint32 size(void) = 0;
	virtual int seek(int32 offset, int origin) = 0;
	virtual bool eof(void) = 0;
	int64 _cacheId;
private:
};

class Ps2SmushFile : public Ps2File {
public:
	Ps2SmushFile(int64 cacheId);
	virtual ~Ps2SmushFile(void);
	virtual bool open(const char *name);
	virtual uint32 read(void *dest, uint32 len);
	virtual uint32 write(const void *src, uint32 len);
	virtual uint32 tell(void);
	virtual uint32 size(void);
	virtual int seek(int32 offset, int origin);
	virtual bool eof(void);
private:
	uint32 _filePos, _fileSize;
	int _id;
};

FILE *ps2_fopen(const char *fname, const char *mode);
int ps2_fclose(FILE *stream);
int ps2_fflush(FILE *stream);
int ps2_fseek(FILE *stream, long offset, int origin);
uint32 ps2_ftell(FILE *stream);
int ps2_feof(FILE *stream);
uint32 ps2_fsize(FILE *stream);

size_t ps2_fread(void *buf, size_t r, size_t n, FILE *stream);
int ps2_fgetc(FILE *stream);
char *ps2_fgets(char *buf, int n, FILE *stream);

size_t ps2_fwrite(const void *buf, size_t r, size_t n, FILE *stream);
int ps2_fputc(int c, FILE *stream);
int ps2_fputs(const char *s, FILE *stream);
int ps2_fprintf(FILE *pOut, const char *zFormat, ...);

#endif // __PS2FILE_IO__

