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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef CHEWY_MEMORY_H
#define CHEWY_MEMORY_H

#include "chewy/datei.h"
#include "chewy/ngstypes.h"

namespace Chewy {

class memory {
public:
	memory();
	~memory();
	void init();

	taf_info *taf_adr(const char *filename);
	// FIXME: was using FILE
	taf_seq_info *taf_seq_adr(void *handle, int16 image_start,
	                          int16 image_anz);
	void tff_adr(const char *filename, byte **speicher);
	byte *void_adr(const char *filename);

	datei *file;
private:

	uint32 mem_avail;
};

} // namespace Chewy

#endif
