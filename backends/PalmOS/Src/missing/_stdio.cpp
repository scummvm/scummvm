/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001-2003 The ScummVM project
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

#include "stdio.h"
#include "extend.h"
///////////////////////////////////////////////////////////////////////////////
//FileRef gLogFile;

// WARNING :	printf functions must only be used if you compile your code with
// 				4byte int option, use standard functions if 2byte int mode
// TODO : enable use of 2byte or 4byte (ifdef PRINTF_4BYTE ...)


static void DrawStatus(Boolean show) {
	UInt8 x,y;
	UInt8 *screen = (UInt8 *)(BmpGetBits(WinGetBitmap(WinGetDisplayWindow())));
	UInt8 color = (show? gVars->indicator.on : gVars->indicator.off);

	if (gVars->screenLocked)
		if (screen == gVars->flipping.pageAddr1)
			screen = gVars->flipping.pageAddr2;
		else
			screen = gVars->flipping.pageAddr1;

	screen += 320 + 305;
	for(y=0;y<3;y++)
	{
		for(x=0;x<14;x++)
			screen[x] = color;
		screen += 319;
	}
}
///////////////////////////////////////////////////////////////////////////////
UInt16 fclose(FileRef *stream) {
	Err error = VFSFileClose(*stream);
	
	if (error == errNone)
		MemPtrFree(stream);

#ifdef DEBUG
	FrmCustomAlert(FrmWarnAlert,"error fclose",0,0);
#endif
	return error;
}
///////////////////////////////////////////////////////////////////////////////
UInt16 feof(FileRef *stream) {
	Err error = VFSFileEOF(*stream);

#ifdef DEBUG
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
///////////////////////////////////////////////////////////////////////////////
Char *fgets(Char *s, UInt32 n, FileRef *stream) {
	UInt32 numBytesRead;
	DrawStatus(true);
	Err error = VFSFileRead(*stream, n, s, &numBytesRead);
	DrawStatus(false);
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
#ifdef DEBUG
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
///////////////////////////////////////////////////////////////////////////////
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
		err = VFSFileOpen (gVars->volRefNum, filename, openMode, fileRefP);
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
		// FIXME : vfsModeCreate|vfsModeWrite will failed on OS3.5 Clié
		err = VFSFileDelete(gVars->volRefNum, filename);	// delete it if exists
		err = VFSFileCreate(gVars->volRefNum, filename);
		openMode = vfsModeWrite;
		if (!err) {
			err = VFSFileOpen (gVars->volRefNum, filename, openMode, fileRefP);
			if (!err)
				return fileRefP;
		}
	}

#ifdef DEBUG
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
///////////////////////////////////////////////////////////////////////////////
UInt32 fread(void *ptr, UInt32 size, UInt32 nitems, FileRef *stream) {
	UInt32 numBytesRead;
	DrawStatus(true);
	Err error = VFSFileRead(*stream, size*nitems, ptr, &numBytesRead);
	DrawStatus(false);
	if (error == errNone || error == vfsErrFileEOF)
		return (UInt32)(numBytesRead/size);

#ifdef DEBUG
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
///////////////////////////////////////////////////////////////////////////////
UInt32 fwrite(const void *ptr, UInt32 size, UInt32 nitems, FileRef *stream) {
	UInt32 numBytesWritten;
	DrawStatus(true);
	Err error = VFSFileWrite(*stream, size*nitems, ptr, &numBytesWritten);
	DrawStatus(false);

	if (error == errNone || error == vfsErrFileEOF)
		return (UInt32)(numBytesWritten/size);

	return NULL;
}
///////////////////////////////////////////////////////////////////////////////
Int32 fseek(FileRef *stream, Int32 offset, Int32 whence) {
	Err error = VFSFileSeek(*stream, whence, offset);
	return error;
}
///////////////////////////////////////////////////////////////////////////////
UInt32 ftell(FileRef *stream) {
	Err e;
	UInt32 filePos;

	e = VFSFileTell(*stream,&filePos);
	if (e != errNone)
		return e;
	
	return filePos;
}
///////////////////////////////////////////////////////////////////////////////
UInt16 fprintf(FileRef *stream, const Char *format, ...) {
	if (!*stream)
		return 0;

	UInt32 numBytesWritten;
	Char buf[256];
	va_list va;

	va_start(va, format);
	vsprintf(buf, format, va);
	va_end(va);

	VFSFileWrite (*stream, StrLen(buf), buf, &numBytesWritten);
	return numBytesWritten;
}
///////////////////////////////////////////////////////////////////////////////
Int16 printf(const Char *format, ...) {
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
///////////////////////////////////////////////////////////////////////////////
Int16 sprintf(Char* s, const Char* formatStr, ...) {
	Char buf[256];
	Int16 count;
	va_list va;

	va_start(va, formatStr);
	count = vsprintf(buf, formatStr, va);
	va_end(va);
	
	StrCopy(s,buf);
	return count;
}
///////////////////////////////////////////////////////////////////////////////
static void StrProcessCModifier(Char *ioStr, UInt16 maxLen) {
	Char *found;
	UInt16 length;
	
	while (found = StrChr(ioStr, '`')) {
		if (found[1] == 0) { // if next char is NULL
			length = maxLen - (found - ioStr);
			MemMove(found, found + 2, length);
			maxLen -= 2;
		}
	}
}

Int16 vsprintf(Char* s, const Char* formatStr, _Palm_va_list argParam) {
	Char format[256];
	Int16 retval;
	// TODO : need a better modifier
	StrCopy(format,formatStr);
	StrReplace(format, 256, "%ld",	"%d");
	StrReplace(format, 256, "%li",	"%i");
	StrReplace(format, 256, "%lx",	"%x");
	StrReplace(format, 256, "%lx",	"%X");
	StrReplace(format, 256, "%2ld",	"%2d");
	StrReplace(format, 256, "%03ld","%.3d");
	StrReplace(format, 256, "%02ld","%.2d");
	StrReplace(format, 256, "%01ld","%.1d");
	StrReplace(format, 256, "%02ld","%02d");

	StrReplace(format, 256, "%2ld","%2d");
	StrReplace(format, 256, "%3ld","%3d");
	StrReplace(format, 256, "%4ld","%4d");
	StrReplace(format, 256, "%5ld","%5d");
	StrReplace(format, 256, "%6ld","%6d");
	StrReplace(format, 256, "%02lx","%02x");
	StrReplace(format, 256, "`%c%c","%c");

	retval = StrVPrintF(s, format, argParam);	// wrong value may be return due to %c%c
	StrProcessCModifier(s, 256);
	
	return retval;
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
