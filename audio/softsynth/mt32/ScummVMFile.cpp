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
#include "ScummVMFile.h"

using namespace MT32Emu;

bool ScummVMFile::open(const char *filename, OpenMode mode) {
	if (mode != OpenMode_read)
		error("ANSIFile: Only read file operations are permitted");

	return internalFile.open(filename);
}

void ScummVMFile::close() {
	internalFile.close();
}

size_t ScummVMFile::read(void *in, size_t size) {
	return internalFile.read(in, size);
}

bool ScummVMFile::readBit8u(Bit8u *in) {
	if (isEOF()) {
		return false;
	} else {
		*in = (Bit8u)internalFile.readByte();
		return true;
	}
}

bool ScummVMFile::isEOF() {
	return internalFile.eos();
}
