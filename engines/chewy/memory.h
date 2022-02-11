/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef CHEWY_MEMORY_H
#define CHEWY_MEMORY_H

#include "chewy/data.h"
#include "chewy/ngstypes.h"

namespace Chewy {

class memory {
public:
	memory();
	~memory();
	void init();

	taf_info *taf_adr(const char *filename);
	taf_seq_info *taf_seq_adr(int16 image_start, int16 image_anz);
	void tff_adr(const char *filename, byte **speicher);
	byte *void_adr(const char *filename);

	Data *file;
};

} // namespace Chewy

#endif
