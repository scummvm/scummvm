/* Copyright (C) 1994-2003 Revolution Software Ltd
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 */

// general odds and ends

#include "stdafx.h"
#include "driver/driver96.h"
#include "debug.h"
#include "memory.h"
#include "tony_gsdk.h"
#include "sword2.h"
#include "common/file.h"

uint32 Read_file(const char *name, mem **membloc, uint32 uid) {
	// read the file in and place into an allocated MEM_float block

	// FIXME: As far as I can see, this function is only used in debug
	// builds, so maybe it should be removed completely?

	File fh;
	uint32 size;

	if (!fh.open(name)) {
		debug(5, "Read_file cannot open %s", name);
		return 0;
	}

	size = fh.size();

	// reserve enough floating memory for the file
	*membloc = Twalloc(size, MEM_float, uid);
	
	if (fh.read((*membloc)->ad, size) != size) {
		debug(5, "Read_file read fail %d", name);
		return 0;
	}

	fh.close();

	//ok, done it - return bytes read
	return size;
}
