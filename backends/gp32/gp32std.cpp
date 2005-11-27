/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001-2005 The ScummVM project
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
 * $Header$
 *
 */

#include "stdafx.h"
#include "common/scummsys.h"
#include "common/scaler.h"
#include "common/system.h"
#include "backends/intern.h"

#include "base/engine.h"

#include "backends/gp32/gp32std.h"
#include "backends/gp32/gp32std_grap.h"

FILE *gp_stderr = NULL;
FILE *gp_stdout = NULL;
FILE *gp_stdin = NULL;

#define DEBUG_MAX 4
char debline[DEBUG_MAX][256];
static int debnext = 0;

void _dprintf(const char *s, ...) {
	int deba, deb;
	char buf[1024];
	va_list va;

	va_start(va, s);
	vsprintf(buf, s, va);
	va_end(va);

	strcpy(debline[debnext], buf);

	gp_fillRect(frameBuffer1, 0, 200, 320, 40, 0);
	
	for (deb = debnext + 1, deba = 0; deb < DEBUG_MAX; deb++, deba++) {
		gp_textOut(frameBuffer1, 0, 200 + 8 * deba, debline[deb], 0xFFFF);
	}
	for (deb = 0; deb <= debnext; deb++, deba++) {
		gp_textOut(frameBuffer1, 0, 200 + 8 * deba, debline[deb], 0xFFFF);
	}
	debnext++;
	if (debnext == DEBUG_MAX)
		debnext = 0;
}

//////////////////
//File functions
// FOR LATER USE
/*
#define SM_PATH_SIZE		256

const char smRootPath[] = "gp:\\";
char smCurrentPath[SM_PATH_SIZE] = "gp:\\";		// must end with '\'

int smMakePath(const char *path, char *smPath) {
  // copy root or current directory
  {
    const char *p;
    if ((*path == '/') || (*path == '\\'))
    {
      path++;
      p = smRootPath;
    }
    else
      p = smCurrentPath;
    while (*p) *smPath++ = *p++;
  }

  // add filenames/directories. remove "." & ".."
  do
  {
    switch (*path)
    {
    case 0:
    case '/':
    case '\\':
      if (*(smPath-1) == '\\')
      {
        // already ends with '\'
      }
      else if ((*(smPath-1) == '.') && (*(smPath-2) == '\\'))
      {
        smPath--;	// remove '.' and end with '\'
      }
      else if ((*(smPath-1) == '.') && (*(smPath-2) == '.') && (*(smPath-3) == '\\'))
      {
        smPath -= 3;	// remove "\.."
        if (*(smPath-1) == ':') *smPath++ = '\\';	// "dev0:" -> "dev0:\"
        else while (*(smPath-1) != '\\') smPath--;	// remove one directory and end with '\'
      }
      else
      {
        *smPath++ = '\\';	// just add '\'
      }
      break;

    default:
      *smPath++ = *path;
      break;
    }
  }
  while (*path++);
  
  *smPath = '\\';

//  *--smPath = 0;	// remove last '\' and null-terminate
  *smPath = 0;	// remove last '\' and null-terminate

  return 0;
}
*/
GPFILE *gp_fopen(const char *fileName, const char *openMode) {
	//FIXME: allocation, mode, malloc -> new
	uint32 mode;
	GPFILE *file;
	ERR_CODE err;
	char s[256];

	if (!strchr(fileName, '.')) {
		sprintf(s, "%s.", fileName);
		fileName = s;
	}

	file = (GPFILE *)malloc(sizeof(GPFILE));

//	NP("%s(\"%s\", \"%s\")", __FUNCTION__, fileName, openMode);

	// FIXME add binary/text support
	if (tolower(openMode[0]) == 'r') {
		mode = OPEN_R;
		GpFileGetSize(fileName, &file->size);
		err = GpFileOpen(fileName, mode, &file->handle);
	} else if (tolower(openMode[0]) == 'w') {
		//printf("open if as W");
		file->size = 0;	// FIXME? new file has no size?
		file->cachePos = 0;
		mode = OPEN_W;
		err = GpFileCreate(fileName, ALWAYS_CREATE, &file->handle);
	} else {
		error("wrong file mode");
	}

	if (!file) {
		error("%s: cannot create FILE structure", __FUNCTION__);
	}
	if (err) {
//		BP("%s: IO error %d", __FUNCTION__, err);
		free(file);
		return NULL;
	}

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
	}			// return 1 ??
*/

	if (stream->cachePos) {
		GpFileWrite(stream->handle, (char *)stream->cacheData, stream->cachePos); // flush cache
		stream->cachePos = 0;
	}

	ERR_CODE err = GpFileClose(stream->handle);
	free(stream);

	return err;
}

int gp_fseek(GPFILE *stream, long offset, int whence) {
	ulong dummy;

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
	return GpFileSeek(stream->handle, whence, offset, (long *)&dummy);
}

size_t gp_fread(void *ptr, size_t size, size_t n, GPFILE *stream) {
	ulong readcount = 0;
	ERR_CODE err = GpFileRead(stream->handle, ptr, size * n, &readcount); //fixme? size*n
	return readcount / size;	//FIXME?
}

size_t gp_fwrite(const void *ptr, size_t size, size_t n, GPFILE *stream) {
	int len = size * n;

	if (!stream) {
		//warning("writing to null file");
		return 0;
	}

	if (stream->cachePos + len < FCACHE_SIZE) {
		memcpy(stream->cacheData + stream->cachePos, ptr, len);
		stream->cachePos += len;
	} else {
		if (stream->cachePos) {
			GpFileWrite(stream->handle, stream->cacheData, stream->cachePos);	// flush cache
			stream->cachePos = 0;
		}

		ERR_CODE err = GpFileWrite(stream->handle, ptr, len);
		if (!err)
			return n;
		else
			return -err;
	}
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

//FIXME!
int gp_feof(GPFILE *stream) {
	return gp_ftell(stream) >= stream->size;
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

/////////////////////
//Memory management
#undef memcpy
#undef memset
void *gp_memcpy(void *dst, const void *src, size_t count) {
	return memcpy(dst, src, count);
}

void *gp_memset(void *dst, int val, size_t count) {
	return memset(dst, val, count);
}

void *gp_malloc(size_t size) {
	uint32 np;
	uint32 *up;

	np = (uint32) gm_malloc(size + sizeof(uint32));

	if (np) {
		up = (uint32 *) np;
		*up = 0x1234;
		return (void *)(np + sizeof(uint32));
	}

	return NULL;
}

void *gp_calloc(size_t nitems, size_t size) {
	void *p = gp_malloc(nitems * size);	//gpcalloc doesnt clear?

	gp_memset(p, 0, nitems * size);

//	if (*(uint8 *)p != 0)
//		warning("%s: calloc doesn't clear!", __FUNCTION__);	//fixme: was error

	return p;
}

void gp_free(void *block) {
	uint32 np;
	uint32 *up;

	if (!block) {
		return;
	}

	np = ((uint32) block) - sizeof(uint32);
	up = (uint32 *) np;
	if (*up == 0x4321) {
		warning("%s: double deallocation!", __FUNCTION__);
		return;
	}

	if (*up != 0x1234) {
		warning("%s: corrupt block!", __FUNCTION__);
		return;
	}
	*up = 0x4321;

	gm_free(up);
}

//////////////////////////////////////////////////
// GP32 stuff
//////////////////////////////////////////////////
void *operator new(size_t size) {
//	printf("BP:operator new(%d)", size);
	return memset(malloc(size), 0xE7, size);
}

void operator delete(void *ptr) {
//	printf("operator delete(%x)", ptr);
	free(ptr);
}

////////////////////
//String functions
char *gp_strcpy(char *dst, const char *src) {
	char *pDst = dst;

	while (*pDst++ = *src++)
		;

	return dst;
}

char *gp_strncpy(char *dst, const char *src, size_t count) {
	char *start = dst;

	while (count && (*dst++ = *src++))
		count--;

	if (count)
		while (--count)
			*dst++ = '\0';

	return start;
}

char *gp_strcat(char *dst, const char *src) {
	char *pDst = dst;

	while (*pDst)
		pDst++;

	while (*pDst++ = *src++)
		;

	return dst;

}

char *gp_strdup(const char *str) {
        char *memory;

        if (!str)
                return NULL;

        if (memory = (char *)malloc(strlen(str) + 1))
                return gp_strcpy(memory, str);

        return NULL;
}

int gp_strcasecmp(const char *dst, const char *src) {
	int f, l;
	do {
		f = tolower((unsigned char)(*(dst++)));
		l = tolower((unsigned char)(*(src++)));
	} while (f && (f == l));

	return f - l;
}

int gp_strncasecmp(const char *dst, const char *src, size_t count) {
	int f,l;

	if (count) {
		do {
			f = tolower((unsigned char)(*(dst++)));
			l = tolower((unsigned char)(*(src++)));
		} while (--count && f && (f == l));
		return f - l;
	}

	return 0;
}

//FIXME: Handle LONG string
void gp_sprintf(char *str, const char *fmt, ...) {
	char s[512];
	va_list marker;

	va_start(marker, fmt);
	vsnprintf(s, 512, fmt, marker);
	va_end(marker);

	gp_strcpy(str, s);
}

int gp_printf(const char *fmt, ...) {
	char s[256];
	va_list marker;

	va_start(marker, fmt);
	vsnprintf(s, 256, fmt, marker);
	va_end(marker);

	_dprintf("%s", s);
	//gp_delay(100);
	return 0;
}

void gp_delay(uint32 msecs) {
	int startTime = GpTickCountGet();
	while (GpTickCountGet() < startTime + msecs);
}

void gp_exit(int code) {
	if (!code) {
		printf("  ----------------------------------------");
		printf("       Your GP32 is now restarting...     ");
		printf("  ----------------------------------------");
		printf("");

		gp_delay(3000);

		GpAppExit();
	} else {
		printf("Exit Code %d", code);
		while (1);
	}
}

// Debug functions
void GPDEBUG(const char *fmt, ...) {
	char s[256];
	va_list marker;

	va_start(marker, fmt);
	vsnprintf(s, 256, fmt, marker);
	va_end(marker);

	_dprintf("%s", s);
}

void NP(const char *fmt, ...) {
	return;
	char s[256];
	va_list marker;

	va_start(marker, fmt);
	vsnprintf(s, 256, fmt, marker);
	va_end(marker);

	_dprintf("NP:%s", s);
	//gp_delay(50);
}

void LP(const char *fmt, ...) {
	return;
	char s[256];
	va_list marker;

	va_start(marker, fmt);
	vsnprintf(s, 256, fmt, marker);
	va_end(marker);

	_dprintf("LP:%s", s);
	//gp_delay(300);
}

void SP(const char *fmt, ...) {
	return;
	char s[256];
	va_list marker;

	va_start(marker, fmt);
	vsnprintf(s, 256, fmt, marker);
	va_end(marker);

	_dprintf("SP:%s", s);
	//gp_delay(50);
}

void BP(const char *fmt, ...) {
	return;
	char s[256];
	va_list marker;

	va_start(marker, fmt);
	vsnprintf(s, 256, fmt, marker);
	va_end(marker);

	_dprintf("BP:%s", s);
	//gp_delay(2000);
}
