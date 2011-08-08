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

#ifndef MT32EMU_ANSI_FILE_H
#define MT32EMU_ANSI_FILE_H

#include <cstdio>

#include "File.h"

namespace MT32Emu {

class ANSIFile: public File {
private:
	FILE *fp;
public:
	bool open(const char *filename, OpenMode mode);
	void close();
	size_t read(void *in, size_t size);
	bool readBit8u(Bit8u *in);
	bool isEOF();
};

}

#endif
