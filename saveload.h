/* ScummVM - Scumm Interpreter
 * Copyright (C) 2002 The ScummVM project
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

#ifndef SAVELOAD_H
#define SAVELOAD_H

struct SerializerStream {
#ifdef NONSTANDARD_SAVE
	void *context;

	bool fopen(const char *filename, const char *mode);
	void fclose();
	int fread(void *buf, int size, int cnt);
	int fwrite(void *buf, int size, int cnt);
#else
	FILE *out;

	FILE *fopen(const char *filename, const char *mode) {
		return out = ::fopen(filename, mode);
	}
	void fclose() {
		::fclose(out);
	}
	int fread(void *buf, int size, int cnt) {
		return ::fread(buf, size, cnt, out);
	}
	int fwrite(void *buf, int size, int cnt) {
		return ::fwrite(buf, size, cnt, out);
	}
#endif
};

#endif
