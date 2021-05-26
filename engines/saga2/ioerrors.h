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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * aint32 with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 *
 * Based on the original sources
 *   Faery Tale II -- The Halls of the Dead
 *   (c) 1993-1996 The Wyrmkeep Entertainment Co.
 */

#ifndef SAGA2_IOERRORS_H
#define SAGA2_IOERRORS_H

namespace Saga2 {

#ifdef _WIN32
#define F_OK 0
#define W_OK 2
#define R_OK 4
#endif

/* ===================================================================== *
   Inlines
 * ===================================================================== */

//-----------------------------------------------------------------------
//	error checking fopen

inline FILE *ftaopen(const char *filename, const char *access) {
#if 0
	FILE *fp = fopen(filename, access);
	if (fp == NULL)
		systemConfigError(cpResFileMissing, "opening file '%s'", filename);
	return fp;
#endif
	warning("STUB: ftaopen()");
	return nullptr;
}

//-----------------------------------------------------------------------
//	error checking fclose

inline int ftaclose(void *fp) {
#if 0
	int r = fclose(fp);
	if (r)
		systemConfigError(cpResFileClose, "closing a program file");
	return r;
#endif
	warning("STUB: ftaclose()");
	return 0;
}

//-----------------------------------------------------------------------
//	error checking fseek

inline int ftaseek(void *fp, size_t offset, int relativeTo) {
#if 0
	int r = fseek(fp, offset, relativeTo);
	if (r)
		systemConfigError(cpResFileSeek, "seeking in a program file");
	return r;
#endif
	warning("STUB: ftaseek()");
	return 0;
}

//-----------------------------------------------------------------------
//	error checking fread

inline size_t ftaread(void *targ, size_t size, size_t count, void *fp) {
#if 0
	size_t r = fread(targ, size, count, fp);
	if (r < count)
		systemConfigError(cpResFileRead, "reading a program file");
	return r;
#endif
	warning("STUB: ftaread()");
	return 0;
}

//-----------------------------------------------------------------------
//	error checking fwrite

inline size_t ftawrite(void *targ, size_t size, size_t count, void *fp) {
#if 0
	size_t r = fwrite(targ, size, count, fp);
	if (r < count)
		systemConfigError(cpResFileWrite, "writing a program file");
	return r;
#endif
	warning("STUB: ftawrite()");
	return 0;
}

//-----------------------------------------------------------------------
//	check file existence

inline bool fileExists(const char f[]) {
#if 0
#ifndef _WIN32
	struct stat st;         // Added because without it
	stat(f, &st);    // Watcom doesnt always notice CD changes
#endif
	return (!access(f, F_OK));
#endif
	warning("STUB: fileExists()");
	return false;
}

//-----------------------------------------------------------------------
//	check file readability

inline bool fileReadable(const char f[]) {
#if 0
#ifndef _WIN32
	struct stat st;         // Added because without it
	stat(f, &st);    // Watcom doesnt always notice CD changes
#endif
	return (!access(f, R_OK));
#endif
	warning("STUB: fileReadable()");
	return false;
}

//-----------------------------------------------------------------------
//	get the file date

inline int32 getFileDate(const char f[]) {
#if 0
#ifdef _WIN32
	struct stat st;
	stat(f, &st);
	return st.st_mtime;
#else
	struct stat st;
	stat(f, &st);
	return st.st_mtime;
#endif
#endif
	warning("STUB: getFileDate()");
	return 0;
}

} // end of namespace Saga2

#endif
