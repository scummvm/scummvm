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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

#include <stdio.h>

FileRef	gStdioOutput = 0;

static LedProc	gStdioLedProc = NULL;
static UInt16	gStdioVolRefNum = sysInvalidRefNum;

static void dummy(Boolean){};

void StdioInit(UInt16 volRefNum, const Char *output, LedProc ledProc) {
	gStdioVolRefNum = volRefNum;

	if (ledProc)
		gStdioLedProc = ledProc;
	else
		gStdioLedProc = dummy;

	VFSFileDelete(gStdioVolRefNum, output);
	VFSFileCreate(gStdioVolRefNum, output);
	VFSFileOpen  (gStdioVolRefNum, output,vfsModeWrite, &gStdioOutput);
}

void StdioRelease() {
	VFSFileClose(gStdioOutput);
}

UInt16 fclose(FileRef *stream) {
	Err error = VFSFileClose(*stream);

	if (error == errNone)
		MemPtrFree(stream);

#ifdef _DEBUG_STDIO
	FrmCustomAlert(FrmWarnAlert,"error fclose",0,0);
#endif
	return error;
}

UInt16 feof(FileRef *stream) {
	Err error = VFSFileEOF(*stream);

#ifdef _DEBUG_STDIO
		switch (error)
		{
			case vfsErrFileEOF:
				FrmCustomAlert(FrmWarnAlert,"vfsErrFileEOF",0,0);
				break;
			case expErrNotOpen:
				FrmCustomAlert(FrmWarnAlert,"expErrNotOpen",0,0);
				break;
			case vfsErrFileBadRef:
				FrmCustomAlert(FrmWarnAlert,"vfsErrFileBadRef",0,0);
				break;
			case vfsErrIsADirectory:
				FrmCustomAlert(FrmWarnAlert,"vfsErrIsADirectory",0,0);
				break;
			case vfsErrNoFileSystem:
				FrmCustomAlert(FrmWarnAlert,"vfsErrNoFileSystem",0,0);
				break;
		}
#endif

	return error;
}

Int16 fgetc(FileRef *stream) {
	UInt32 numBytesRead;
	Err e;
	Char c;

	e = VFSFileRead(*stream, 1, &c, &numBytesRead);
	return (int)(!e ? c : EOF);
}

Char *fgets(Char *s, UInt32 n, FileRef *stream) {
	UInt32 numBytesRead;
	gStdioLedProc(true);
	Err error = VFSFileRead(*stream, n, s, &numBytesRead);
	gStdioLedProc(false);
	if (error == errNone || error == vfsErrFileEOF) {
		UInt32 reset = 0;
		Char *endLine = StrChr(s, '\n');

		if (endLine >= s) {
			reset = (endLine - s);
			s[reset] = 0;
			reset = numBytesRead - (reset + 1);
			VFSFileSeek(*stream, vfsOriginCurrent, -reset);
		}

		return s;
	}
#ifdef _DEBUG_STDIO
		switch (error)
		{
			case expErrNotOpen:
				FrmCustomAlert(FrmWarnAlert,"expErrNotOpen",0,0);
				break;
			case vfsErrFileBadRef:
				FrmCustomAlert(FrmWarnAlert,"vfsErrFileBadRef",0,0);
				break;
			case vfsErrFileEOF:
				FrmCustomAlert(FrmWarnAlert,"vfsErrFileEOF",0,0);
				break;
			case vfsErrFilePermissionDenied:
				FrmCustomAlert(FrmWarnAlert,"vfsErrFilePermissionDenied",0,0);
				break;
			case vfsErrIsADirectory:
				FrmCustomAlert(FrmWarnAlert,"vfsErrIsADirectory",0,0);
				break;
			case vfsErrNoFileSystem:
				FrmCustomAlert(FrmWarnAlert,"vfsErrNoFileSystem",0,0);
				break;
		}
#endif

	return NULL;
}

FileRef *fopen(const Char *filename, const Char *type) {
	Err err;
	UInt16 openMode;
	FileRef *fileRefP = (FileRef *)MemPtrNew(sizeof(FileRef *));

	if (StrCompare(type,"r")==0)
		openMode = vfsModeRead;
	else if (StrCompare(type,"rb")==0)
		openMode = vfsModeRead;
	else if (StrCompare(type,"w")==0)
		openMode = vfsModeCreate|vfsModeWrite;
	else if (StrCompare(type,"wb")==0)
		openMode = vfsModeCreate|vfsModeWrite;
	else
		openMode = vfsModeReadWrite;

	if (openMode & vfsModeRead) {
		// if read file :
		// first try to load from the specfied card
		err = VFSFileOpen (gStdioVolRefNum, filename, openMode, fileRefP);
		//if err (not found ?) parse each avalaible card for the specified file
		if (err) {
			UInt16 volRefNum;
			UInt32 volIterator = vfsIteratorStart;
			while (volIterator != vfsIteratorStop) {
				err = VFSVolumeEnumerate(&volRefNum, &volIterator);

				if (!err) {
					err = VFSFileOpen (volRefNum, filename, openMode, fileRefP);
					if (!err)
						return fileRefP;
				}
			}
		} else {
			return fileRefP;
		}
	} else {
		// if write file :
		// use only the specified card
		err = VFSFileDelete(gStdioVolRefNum, filename);	// delete it if exists
		err = VFSFileCreate(gStdioVolRefNum, filename);
		openMode = vfsModeWrite;
		if (!err) {
			err = VFSFileOpen (gStdioVolRefNum, filename, openMode, fileRefP);
			if (!err)
				return fileRefP;
		}
	}

#ifdef _DEBUG_STDIO
	else
	{
		switch (err)
		{
			case expErrCardReadOnly:
				FrmCustomAlert(FrmWarnAlert,"expErrCardReadOnly",0,0);
				break;
			case expErrNotOpen:
				FrmCustomAlert(FrmWarnAlert,"expErrNotOpen",0,0);
				break;
			case vfsErrBadName:
				FrmCustomAlert(FrmWarnAlert,"vfsErrBadName",0,0);
				break;
			case vfsErrFileNotFound:
				FrmCustomAlert(FrmWarnAlert,"vfsErrFileNotFound",0,0);
				break;
			case vfsErrFilePermissionDenied:
				FrmCustomAlert(FrmWarnAlert,"vfsErrFilePermissionDenied",0,0);
				break;
			case vfsErrVolumeBadRef:
				FrmCustomAlert(FrmWarnAlert,"vfsErrVolumeBadRef",0,0);
				break;
			default:
				FrmCustomAlert(FrmWarnAlert,"unknow",0,0);
				break;
		}
	}
#endif

	MemPtrFree(fileRefP); // prevent memory leak
	return NULL;
}

UInt32 fread(void *ptr, UInt32 size, UInt32 nitems, FileRef *stream) {
	UInt32 numBytesRead;
	gStdioLedProc(true);
	Err error = VFSFileRead(*stream, size*nitems, ptr, &numBytesRead);
	gStdioLedProc(false);
	if (error == errNone || error == vfsErrFileEOF)
		return (UInt32)(numBytesRead/size);

#ifdef _DEBUG_STDIO
		switch (error)
		{
			case expErrNotOpen:
				FrmCustomAlert(FrmWarn,"expErrNotOpen",0,0);
				break;
			case vfsErrFileBadRef:
				FrmCustomAlert(FrmWarn,"vfsErrFileBadRef",0,0);
				break;
			case vfsErrFileEOF:
				FrmCustomAlert(FrmWarn,"vfsErrFileEOF",0,0);
				break;
			case vfsErrFilePermissionDenied:
				FrmCustomAlert(FrmWarn,"vfsErrFilePermissionDenied",0,0);
				break;
			case vfsErrIsADirectory:
				FrmCustomAlert(FrmWarn,"vfsErrIsADirectory",0,0);
				break;
			case vfsErrNoFileSystem:
				FrmCustomAlert(FrmWarn,"vfsErrNoFileSystem",0,0);
				break;
		}
#endif
	return 0;
}

UInt32 fwrite(const void *ptr, UInt32 size, UInt32 nitems, FileRef *stream) {
	UInt32 numBytesWritten;
	gStdioLedProc(true);
	Err error = VFSFileWrite(*stream, size*nitems, ptr, &numBytesWritten);
	gStdioLedProc(false);

	if (error == errNone || error == vfsErrFileEOF)
		return (UInt32)(numBytesWritten/size);

	return NULL;
}

Int32 fseek(FileRef *stream, Int32 offset, Int32 whence) {
	Err error = VFSFileSeek(*stream, whence, offset);
	return error;
}

UInt32 ftell(FileRef *stream) {
	Err e;
	UInt32 filePos;

	e = VFSFileTell(*stream,&filePos);
	if (e != errNone)
		return e;

	return filePos;
}

Int32 fprintf(FileRef *stream, const Char *formatStr, ...) {
	if (!*stream)
		return 0;

	UInt32 numBytesWritten;
	Char buf[256];
	va_list va;

	va_start(va, formatStr);
	vsprintf(buf, formatStr, va);
	va_end(va);

	VFSFileWrite (*stream, StrLen(buf), buf, &numBytesWritten);
	return numBytesWritten;
}

Int32 printf(const Char *format, ...) {
	if (!*stdout)
		return 0;

	UInt32 numBytesWritten;
	Char buf[256];
	va_list va;

	va_start(va, format);
	vsprintf(buf, format, va);
	va_end(va);

	VFSFileWrite (*stdout, StrLen(buf), buf, &numBytesWritten);
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
 * TODO : check http://www.ijs.si/software/snprintf/ for a potable implementation of vsnprintf
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
		while (found[next] == '0' || found[next] == ' ')	// WARNING : reduce size only (TODO ?)
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
			if (*mod == 'c') {
				StrCopy(tmp, "`c`%c%c");

			} else if (*mod == 'p') {
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
	StrProcC_(result, 256);
	StrProcXO(result, 256, tmp);
	StrCopy(s, result);

	return StrLen(s);
}
