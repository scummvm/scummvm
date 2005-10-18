/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001-2005 The ScummVM project
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

#include <stdio.h>
#include <stdlib.h>

#define	CACHE_SIZE	1024
enum {
	MODE_BUFREAD = 1,
	MODE_BUFWRITE,
	MODE_BUFNONE
};

FILE gStdioOutput = {0,0,0,0,0,0};
static void dummy(Boolean) {};

static LedProc	gStdioLedProc = dummy;
static UInt16	gStdioVolRefNum = sysInvalidRefNum;
static UInt32	gCacheSize = CACHE_SIZE;

// TODO : implement "errno"

void StdioInit(UInt16 volRefNum, const Char *output) {	// DONE
	gStdioVolRefNum = volRefNum;
	gStdioOutput.mode = MODE_BUFWRITE;
			
	VFSFileDelete(gStdioVolRefNum, output);
	VFSFileCreate(gStdioVolRefNum, output);
	VFSFileOpen  (gStdioVolRefNum, output,vfsModeWrite, &gStdioOutput.fileRef);	
}

void StdioSetLedProc(LedProc ledProc) {	// DONE
	if (ledProc)
		gStdioLedProc = ledProc;
	else
		gStdioLedProc = dummy;
}

void StdioSetCacheSize(UInt32 s) {	// DONE
	gCacheSize = s;
}

void StdioRelease() {	// DONE
	// there is no cache on stdout/stderr
	VFSFileClose(gStdioOutput.fileRef);
}

UInt16 fclose(FILE *stream) {	// DONE
	UInt32 numBytesWritten;
	Err e;
	
	if (stream->cacheSize) {
		if (stream->bufSize > 0 && stream->mode == MODE_BUFWRITE)
				VFSFileWrite(stream->fileRef, stream->bufSize, stream->cache, &numBytesWritten);

		MemPtrFree(stream->cache);
	}

	e = VFSFileClose(stream->fileRef);
	e = MemPtrFree(stream);

	return e;
}

UInt16 feof(FILE *stream) {	// DONE
	Err e;
	
	if (stream->cacheSize) {
		switch (stream->mode) {
			case MODE_BUFWRITE:
				return 0;	// never set in this mode
			case MODE_BUFREAD:
				if (stream->bufSize > 0)
					return 0;
				break;
		}
	}
	
	e = VFSFileEOF(stream->fileRef);
	return e;
}

UInt16 ferror(FILE *stream) {
	return (stream->err);
}

Int16 fgetc(FILE *stream) {
	UInt32 numBytesRead;
	Char c;
	
	numBytesRead = fread(&c, 1, 1, stream);
	return (int)(numBytesRead == 1 ? c : EOF);
}

Char *fgets(Char *s, UInt32 n, FILE *stream) {
	UInt32 numBytesRead;

	numBytesRead = fread(s, n, 1, stream);
	if (numBytesRead) {
		UInt32 reset = 0;
		Char *endLine = StrChr(s, '\n');
		
		if (endLine >= s) {
			reset = (endLine - s);
			s[reset] = 0;
			reset = numBytesRead - (reset + 1);
			fseek(stream, -reset, SEEK_CUR);
		}
	
		return s;
	}

	return NULL;
}

FILE *fopen(const Char *filename, const Char *type) { // DONE
	Err err;
	UInt16 openMode;
	Boolean cache = true;
	FILE *fileP = (FILE *)MemPtrNew(sizeof(FILE));
	
	if (!fileP)
		return NULL;
	
	MemSet(fileP, sizeof(FILE), 0);
		
	if (StrCompare(type,"r")==0 || StrCompare(type,"rb")==0) {
		fileP->mode = MODE_BUFREAD;
		openMode = vfsModeRead;

	} else if (StrCompare(type,"w")==0 || StrCompare(type,"wb")==0) {
		fileP->mode = MODE_BUFWRITE;
		openMode = vfsModeCreate|vfsModeWrite;

	} else {
		cache = false;
		fileP->mode = MODE_BUFNONE;
		openMode = vfsModeReadWrite;
	}

	if (cache) {
							fileP->cacheSize = gCacheSize;
		if (gCacheSize)		fileP->cache = (UInt8 *)MemGluePtrNew(gCacheSize);
		if (!fileP->cache)	fileP->cacheSize = 0;
	}

	if (openMode & vfsModeRead) {
		// if read file :
		// first try to load from the specfied card
		err = VFSFileOpen (gStdioVolRefNum, filename, openMode, &fileP->fileRef);
		//if err (not found ?) parse each avalaible card for the specified file
		if (err) {
			UInt16 volRefNum;
			UInt32 volIterator = vfsIteratorStart;
			while (volIterator != vfsIteratorStop) {
				err = VFSVolumeEnumerate(&volRefNum, &volIterator);

				if (!err) {
					err = VFSFileOpen (volRefNum, filename, openMode, &fileP->fileRef);
					if (!err)
						return fileP;
				}
			}
		} else {
			return fileP;
		}
	} else {
		// if write file :
		// use only the specified card
		err = VFSFileDelete(gStdioVolRefNum, filename);	// delete it if exists
		err = VFSFileCreate(gStdioVolRefNum, filename);
		openMode = vfsModeWrite;
		if (!err) {
			err = VFSFileOpen (gStdioVolRefNum, filename, openMode, &fileP->fileRef);
			if (!err)
				return fileP;
		}
	}

	if (fileP->cacheSize)
		MemPtrFree(fileP->cache);

	MemPtrFree(fileP); // prevent memory leak
	return NULL;
}

UInt32 fread(void *ptr, UInt32 size, UInt32 nitems, FILE *stream) {	// DONE
	Err e = errNone;
	UInt32 numBytesRead, rsize = (size * nitems);

	// try to read on a write only stream ?
	if (stream->mode == MODE_BUFWRITE || !rsize)
		return 0;

	// cached ?
	if (stream->cacheSize) {
		// empty buffer ? fill it if required
		if (stream->bufSize == 0 && rsize < stream->cacheSize) {
			gStdioLedProc(true);
			e = VFSFileRead(stream->fileRef, stream->cacheSize, stream->cache, &numBytesRead);
			gStdioLedProc(false);
			stream->bufSize = numBytesRead;
			stream->bufPos = 0;
		} 

		// we have the data in the cache
		if (stream->bufSize >= rsize) {
			MemMove(ptr, (stream->cache + stream->bufPos), rsize);
			stream->bufPos += rsize;
			stream->bufSize -= rsize;
			numBytesRead = rsize;

		// not enough but something ?
		} else if (stream->bufSize > 0) {
			UInt8 *next = (UInt8 *)ptr;
			MemMove(ptr, (stream->cache + stream->bufPos), stream->bufSize);
			rsize -= stream->bufSize;
			gStdioLedProc(true);
			e = VFSFileRead(stream->fileRef, rsize, (next + stream->bufSize), &numBytesRead);
			gStdioLedProc(false);
			numBytesRead += stream->bufSize;
			stream->bufSize = 0;
			stream->bufPos = 0;

		// nothing in the cache ?
		} else {
			gStdioLedProc(true);
			e = VFSFileRead(stream->fileRef, rsize, ptr, &numBytesRead);
			gStdioLedProc(false);
		}

	// no ? direct read
	} else {
		gStdioLedProc(true);
		e = VFSFileRead(stream->fileRef, rsize, ptr, &numBytesRead);
		gStdioLedProc(false);
	}

	if (e == errNone || e == vfsErrFileEOF)
		return (UInt32)(numBytesRead / size);

	return 0;
}

UInt32 fwrite(const void *ptr, UInt32 size, UInt32 nitems, FILE *stream) { // DONE
	Err e = errNone;
	UInt32 numBytesWritten = (size * nitems);
	
	// try to write on a read only stream ?
	if (stream->mode == MODE_BUFREAD || !numBytesWritten)
		return 0;
	
	// cached ?
	if (stream->cacheSize) {
		// can cache it ?
		if ((stream->bufSize + numBytesWritten) <= stream->cacheSize) {
			MemMove((stream->cache + stream->bufSize), ptr, numBytesWritten);
			stream->bufSize += numBytesWritten;

		// not enough room ? write cached data and new data
		} else {
			gStdioLedProc(true);
			e = VFSFileWrite(stream->fileRef, stream->bufSize, stream->cache, &numBytesWritten);
			e = VFSFileWrite(stream->fileRef, (size * nitems), ptr, &numBytesWritten);
			gStdioLedProc(false);
			stream->bufSize = 0;
		}
	
	// no ? direct write
	} else {
		gStdioLedProc(true);
		e = VFSFileWrite(stream->fileRef, (size * nitems), ptr, &numBytesWritten);
		gStdioLedProc(false);
	}

	if ((e == errNone || e == vfsErrFileEOF)) {
		return (UInt32)(numBytesWritten / size);
	}

	return 0;
}

Int16 fseek(FILE *stream, Int32 offset, Int32 whence) {	// DONE
	UInt32 numBytesWritten;
	Err e;

	if (stream->cacheSize) {
		switch (stream->mode) {
			case MODE_BUFWRITE:
				e = VFSFileWrite(stream->fileRef, stream->bufSize, stream->cache, &numBytesWritten);
				stream->bufSize = 0;
				break;

			case MODE_BUFREAD:
				// reposition file postion if needed
				if (whence == SEEK_CUR)
					e = VFSFileSeek(stream->fileRef, vfsOriginCurrent, -stream->bufSize);
				stream->bufSize = 0;
				stream->bufPos = 0;
				break;
		}
	}
		
	e = VFSFileSeek(stream->fileRef, whence, offset);
	return (e ? -1 : 0);
}

Int32 ftell(FILE *stream) { // DONE
	Err e;
	UInt32 filePos;

	e = VFSFileTell(stream->fileRef ,&filePos);

	if (stream->cacheSize) {
		switch (stream->mode) {
			case MODE_BUFWRITE:
				filePos += stream->bufSize;
				break;

			case MODE_BUFREAD:
				filePos -= stream->bufSize;
				break;
		}
	}

	if (e) return -1; // errno = ?
	return filePos;
}

Int32 fprintf(FILE *stream, const Char *formatStr, ...) { // DONE
	if (!stream->fileRef)
		return 0;

	UInt32 numBytesWritten;
	Char buf[256];
	va_list va;

	va_start(va, formatStr);
	vsprintf(buf, formatStr, va);
	va_end(va);

	numBytesWritten = fwrite(buf, StrLen(buf), 1, stream);
	return numBytesWritten;
}

Int32 printf(const Char *format, ...) { // DONE
	if (!stdout->fileRef)
		return 0;

	UInt32 numBytesWritten;
	Char buf[256];
	va_list va;

	va_start(va, format);
	vsprintf(buf, format, va);
	va_end(va);

	numBytesWritten = fwrite(buf, StrLen(buf), 1, stdout);
	return numBytesWritten;
}

Int32 sprintf(Char* s, const Char* formatStr, ...) {
	Int16 count;
	va_list va;

	va_start(va, formatStr);
	count = vsprintf(s, formatStr, va);
	va_end(va);
	
	return count;
}

Int32 snprintf(Char* s, UInt32 len, const Char* formatStr, ...) {
	// len is ignored
	Int16 count;
	va_list va;

	va_start(va, formatStr);
	count = vsprintf(s, formatStr, va);
	va_end(va);
	
	return count;
}


/* WARNING : vsprintf
 * -------
 * This function can handle only %[+- ][.0][field length][sxXdoiucp] format strings
 * compiler option : 4byte int mode only !
 *
 * TODO : check http://www.ijs.si/software/snprintf/ for a portable implementation of vsnprintf
 * This one make use of sprintf so need to check if it works with PalmOS.
 */

static Char *StrIToBase(Char *s, Int32 i, UInt8 b) {
	const Char *conv = "0123456789ABCDEF";
	Char o;
	Int16 c, n = 0;
	Int32 div, mod;
	
	do {
		div = i / b;
		mod = i % b;
		
		s[n++]	= *(conv + mod);
		i		= div;

	} while (i >= b);

	if (i > 0) {
		s[n + 0] = *(conv + i);
		s[n + 1] = 0;
	} else {
		s[n + 0] = 0;
		n--;
	}

	for (c=0; c <= (n >> 1); c++) {
		o		= s[c];
		s[c]	= s[n - c];
		s[n - c]= o;
	}

	return s;
}

static void StrProcC_(Char *ioStr, UInt16 maxLen) {
	Char *found;
	Int16 length;
	
	while (found = StrStr(ioStr, "`c`")) {
		if (found[3] == 0) { 						// if next char is NULL
			length = maxLen - (found - ioStr + 2);
			MemMove(found, found + 4, length);
			maxLen -= 2;
		}
	}
}

static void StrProcXO(Char *ioStr, UInt16 maxLen, Char *tmp) {
	Char *found, *last, mod, fill;
	Int16 len, count, next;
	Int32 val;
		
	while (found = StrChr(ioStr, '`')) {
		last = StrChr(found + 1, '`');
		
		if (!last)
			return;

		*last	= 0;
		next 	= 0;
		fill	= *(found + 1);
		mod		= *(found + 2);
		count	= StrAToI(found + 3);

		len = maxLen - (last - ioStr);
		MemMove(found, (last + 1), len);

		// x and X always 8char on palmos ... o set to 8char (not supported on palmos)
		while ((found[next] == '0' || found[next] == ' ') && next < 8)	// WARNING : reduce size only (TODO ?)
			next++;
		
		// convert to base 8
		if (mod == 'o') {
			StrNCopy(tmp, found + next, 8 - next);
			tmp[8 - next]	= 0;
			val				= StrAToI(tmp);
			StrIToBase(tmp, val, 8);				// now we have the same but in octal
			next			= 8 - StrLen(tmp);
			MemMove(found + next, tmp, StrLen(tmp));
		} else {
			// if val is 0, keep last 0
			if (next == 8)
				next = 7;
		}

		if ((8 - next) > count)
			count = 8 - next;
			
		if (count == 0)
			count = 1;

		len = maxLen - (found - ioStr) - (8 - count);
		MemSet(found, next, fill);
		MemMove(found, found + (8 - count), len);

		//  ... and upper case
		if (mod == 'x') {
			while (count--) {
				if (*found >='A' && *found <='F')
					*found = (*found + 32);
				found++;
			}
		}
	}
}

Int32 vsprintf(Char* s, const Char* formatStr, _Palm_va_list argParam) {
	Char format[256], result[256], tmp[32];
	
	Char *found, *mod, *num;
	UInt32 next;
	Boolean zero;
	Int16 count, len;
	
	MemSet(format, sizeof(format), 'x');
	MemSet(result, sizeof(result), 'y');
	MemSet(tmp, sizeof(tmp), 'z');

	StrCopy(format,formatStr);	// copy actual formatStr to temp buffer
	next = 0;					// start of the string

	while (found = StrChr(format + next, '%')) {
		mod = found + 1;
		
		if (*mod == '%') {			// just a % ?
			mod++;

		} else {
			if (*mod == '+' ||
				*mod == '-' ||
				*mod == ' '	)		// skip
				mod++;
			
			if (*mod == '0' ||
				*mod == '.' ) {
				*mod++ = '0';
				zero =  true;
			} else {
				zero = false;
			}
			
			num = mod;
			while (	*mod >= '0' &&
					*mod <= '9'	)	// search format char
				mod++;
			
			// get the numeric value
			if (num < mod) {
				StrNCopy(tmp, num, mod - num);
				tmp[mod - num] = 0;
				count = StrAToI(tmp);
			} else {
				count = 0;
			}

			if (*mod == 'l')		// already set to %...l(x) ?
				mod++;

			// prepare new format
#if !defined(COMPILE_ZODIAC) || defined(PALMOS_68K)
			if (*mod == 'c') {
				StrCopy(tmp, "`c`%c%c");

			} else
#endif
			if (*mod == 'p') {
				StrCopy(tmp, "%08lX");			// %x = %08X in palmos

			} else {
				len = 0;
	
				switch (*mod) {
					case 'x':
					case 'X':
					case 'o':
						tmp[0] = '`';
						tmp[1] = (zero) ? '0' : ' ';
						tmp[2] = *mod;	
						StrIToA(tmp + 3, count);
						len += StrLen(tmp);
						tmp[len++] = '`';
						tmp[len] = 0;
						
						if (*mod == 'o') {	// set as base 10 num and convert later
							*mod = 'd';
							count = 8;		// force 8char
						}

						break;
				}
					
				StrNCopy(tmp + len, found, (num - found));
				len += (num - found);

				if (count) {
					StrIToA(tmp + len, count);
					len += StrLen(tmp + len);
				}
				
				if (*mod == 'd' ||
					*mod == 'i' ||
					*mod == 'x' ||
					*mod == 'X' ||
					*mod == 'u' 
					) {
					tmp[len++] = 'l';
				}

				tmp[len + 0] = *mod;
				tmp[len + 1] = 0;
			}

			mod++;
			MemMove(found + StrLen(tmp), mod, StrLen(mod) + 1);
			StrNCopy(found, tmp, StrLen(tmp));
			mod = found + StrLen(tmp);
		}
		
		next = (mod - format);
	}
	
	// Copy result in a temp buffer to process last formats
	StrVPrintF(result, format, argParam);
#if !defined(COMPILE_ZODIAC) || defined(PALMOS_68K)
	StrProcC_(result, 256);
#endif
	StrProcXO(result, 256, tmp);
	StrCopy(s, result);
	
	return StrLen(s);
}
