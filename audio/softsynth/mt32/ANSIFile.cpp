/* Copyright (C) 2003, 2004, 2005, 2006, 2008, 2009 Dean Beeler, Jerome Fisher
 * Copyright (C) 2011 Dean Beeler, Jerome Fisher, Sergey V. Mikayev
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation, either version 2.1 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "mt32emu.h"
#include "ANSIFile.h"

using namespace MT32Emu;

bool ANSIFile::open(const char *filename, OpenMode mode) {
	const char *fmode;
	if (mode == OpenMode_read) {
		fmode = "rb";
	} else {
		fmode = "wb";
	}
	fp = fopen(filename, fmode);
	return (fp != NULL);
}

void ANSIFile::close() {
	fclose(fp);
}

size_t ANSIFile::read(void *in, size_t size) {
	return fread(in, 1, size, fp);
}

bool ANSIFile::readBit8u(Bit8u *in) {
	int c = fgetc(fp);
	if (c == EOF) {
		return false;
	}
	*in = (Bit8u)c;
	return true;
}

bool ANSIFile::isEOF() {
	return feof(fp) != 0;
}
