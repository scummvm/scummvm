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

#include "backends/fs/stdiostream.h"

#include <errno.h>

#if defined(MACOSX) || defined(IPHONE)
#include "CoreFoundation/CoreFoundation.h"
#endif


#ifdef __PLAYSTATION2__
	// for those replaced fopen/fread/etc functions
	typedef unsigned long	uint64;
	typedef signed long	int64;
	#include "backends/platform/ps2/fileio.h"

	#define fopen(a, b)			ps2_fopen(a, b)
	#define fclose(a)			ps2_fclose(a)
	#define fseek(a, b, c)			ps2_fseek(a, b, c)
	#define ftell(a)			ps2_ftell(a)
	#define feof(a)				ps2_feof(a)
	#define fread(a, b, c, d)		ps2_fread(a, b, c, d)
	#define fwrite(a, b, c, d)		ps2_fwrite(a, b, c, d)

	//#define fprintf				ps2_fprintf	// used in common/util.cpp
	//#define fflush(a)			ps2_fflush(a)	// used in common/util.cpp

	//#define fgetc(a)			ps2_fgetc(a)	// not used
	//#define fgets(a, b, c)			ps2_fgets(a, b, c)	// not used
	//#define fputc(a, b)			ps2_fputc(a, b)	// not used
	//#define fputs(a, b)			ps2_fputs(a, b)	// not used

	//#define fsize(a)			ps2_fsize(a)	// not used -- and it is not a standard function either
#endif

#ifdef __DS__

	// These functions replease the standard library functions of the same name.
	// As this header is included after the standard one, I have the chance to #define
	// all of these to my own code.
	//
	// A #define is the only way, as redefinig the functions would cause linker errors.

	// These functions need to be #undef'ed, as their original definition
	// in devkitarm is done with #includes (ugh!)
	#undef feof
	#undef clearerr
	//#undef getc
	//#undef ferror

	#include "backends/fs/ds/ds-fs.h"


	//void	std_fprintf(FILE* handle, const char* fmt, ...);	// used in common/util.cpp
	//void	std_fflush(FILE* handle);	// used in common/util.cpp

	//char*	std_fgets(char* str, int size, FILE* file);	// not used
	//int	std_getc(FILE* handle);	// not used
	//char*	std_getcwd(char* dir, int dunno);	// not used
	//void	std_cwd(char* dir);	// not used
	//int	std_ferror(FILE* handle);	// not used

	// Only functions used in the ScummVM source have been defined here!
	#define fopen(name, mode)					DS::std_fopen(name, mode)
	#define fclose(handle)						DS::std_fclose(handle)
	#define fread(ptr, size, items, file)		DS::std_fread(ptr, size, items, file)
	#define fwrite(ptr, size, items, file)		DS::std_fwrite(ptr, size, items, file)
	#define feof(handle)						DS::std_feof(handle)
	#define ftell(handle)						DS::std_ftell(handle)
	#define fseek(handle, offset, whence)		DS::std_fseek(handle, offset, whence)
	#define clearerr(handle)					DS::std_clearerr(handle)

	//#define printf(fmt, ...)					consolePrintf(fmt, ##__VA_ARGS__)

	//#define fprintf(file, fmt, ...)				{ char str[128]; sprintf(str, fmt, ##__VA_ARGS__); DS::std_fwrite(str, strlen(str), 1, file); }
	//#define fflush(file)						DS::std_fflush(file)	// used in common/util.cpp

	//#define fgets(str, size, file)				DS::std_fgets(str, size, file)	// not used
	//#define getc(handle)						DS::std_getc(handle)	// not used
	//#define getcwd(dir, dunno)					DS::std_getcwd(dir, dunno)	// not used
	#define ferror(handle)						DS::std_ferror(handle)

#endif

#ifdef __SYMBIAN32__
	#undef feof
	#undef clearerr

	#define FILE void

	FILE*	symbian_fopen(const char* name, const char* mode);
	void	symbian_fclose(FILE* handle);
	size_t	symbian_fread(const void* ptr, size_t size, size_t numItems, FILE* handle);
	size_t	symbian_fwrite(const void* ptr, size_t size, size_t numItems, FILE* handle);
	bool	symbian_feof(FILE* handle);
	long int symbian_ftell(FILE* handle);
	int		symbian_fseek(FILE* handle, long int offset, int whence);
	void	symbian_clearerr(FILE* handle);

	// Only functions used in the ScummVM source have been defined here!
	#define fopen(name, mode)					symbian_fopen(name, mode)
	#define fclose(handle)						symbian_fclose(handle)
	#define fread(ptr, size, items, file)		symbian_fread(ptr, size, items, file)
	#define fwrite(ptr, size, items, file)		symbian_fwrite(ptr, size, items, file)
	#define feof(handle)						symbian_feof(handle)
	#define ftell(handle)						symbian_ftell(handle)
	#define fseek(handle, offset, whence)		symbian_fseek(handle, offset, whence)
	#define clearerr(handle)					symbian_clearerr(handle)
#endif


StdioStream::StdioStream(void *handle) : _handle(handle) {
	assert(handle);
}

StdioStream::~StdioStream() {
	fclose((FILE *)_handle);
}

bool StdioStream::ioFailed() const {
	return ferror((FILE *)_handle) != 0;
}

void StdioStream::clearIOFailed() {
	clearerr((FILE *)_handle);
}

bool StdioStream::eos() const {
	return feof((FILE *)_handle) != 0;
}

uint32 StdioStream::pos() const {
	// FIXME: ftell can return -1 to indicate an error (in which case errno gets set)
	//        Maybe we should support that, too?
	return ftell((FILE *)_handle);
}

uint32 StdioStream::size() const {
	uint32 oldPos = ftell((FILE *)_handle);
	fseek((FILE *)_handle, 0, SEEK_END);
	uint32 length = ftell((FILE *)_handle);
	fseek((FILE *)_handle, oldPos, SEEK_SET);

	return length;
}

void StdioStream::seek(int32 offs, int whence) {
	assert(_handle);

	if (fseek((FILE *)_handle, offs, whence) != 0)
		clearerr((FILE *)_handle);	// FIXME: why do we call clearerr here?
		
	// FIXME: fseek has a return value to indicate errors; 
	//        Maybe we should support that, too?
}

uint32 StdioStream::read(void *ptr, uint32 len) {
	return (uint32)fread((byte *)ptr, 1, len, (FILE *)_handle);
}

uint32 StdioStream::write(const void *ptr, uint32 len) {
	return (uint32)fwrite(ptr, 1, len, (FILE *)_handle);
}

void StdioStream::flush() {
	// TODO: Should check the return value of fflush, and if it is non-zero,
	// check errno and set an error flag.
	fflush((FILE *)_handle);
}

StdioStream *StdioStream::makeFromPath(const Common::String &path, bool writeMode) {
	FILE *handle = fopen(path.c_str(), writeMode ? "wb" : "rb");
	if (handle)
		return new StdioStream(handle);
	return 0;
}
