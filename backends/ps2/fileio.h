/* ScummVM - Scumm Interpreter
 * Copyright (C) 2004-2005 The ScummVM project
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

#ifndef __PS2FILE_IO__
#define __PS2FILE_IO__

#include "scummsys.h"

class AccessFio {
public:
	AccessFio(void);
	virtual ~AccessFio(void);
	virtual bool open(const char *name, int mode);
	virtual void read(void *dest, uint32 size);
	virtual void write(const void *src, uint32 size);
	virtual int seek(int32 offset, int whence);
	virtual int32 sync(void);
	virtual bool poll(void);
	virtual bool fioAvail(void);
protected:
	int _handle;
};

class File;

FILE *ps2_fopen(const char *fname, const char *mode);
int ps2_fclose(FILE *stream);
int ps2_fflush(FILE *stream);
int ps2_fseek(FILE *stream, long offset, int origin);
long ps2_ftell(FILE *stream);
int ps2_feof(FILE *stream); 
long ps2_fsize(FILE *stream);

size_t ps2_fread(void *buf, size_t r, size_t n, FILE *stream);
int ps2_fgetc(FILE *stream);
char *ps2_fgets(char *buf, int n, FILE *stream);

size_t ps2_fwrite(const void *buf, size_t r, size_t n, FILE *stream);
int ps2_fputc(int c, FILE *stream);
int ps2_fputs(const char *s, FILE *stream);
int ps2_fprintf(FILE *pOut, const char *zFormat, ...);

AccessFio *fetchHandle(File *file);

#endif // __PS2FILE_IO__
