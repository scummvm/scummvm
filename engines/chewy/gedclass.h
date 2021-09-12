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

#ifndef CHEWY_GEDCLASS_H
#define CHEWY_GEDCLASS_H

#include "chewy/ngstypes.h"

class gedclass {

public:
	gedclass(int16(*user_func)(int16 idx_nr));
	~gedclass();

	void load_ged_pool(char *fname, GedChunkHeader *Gh, int16 ch_nr, char *speicher);

	//FIXME : was FILE*
	void load_ged_pool(void *handle, GedChunkHeader *Gh, int16 ch_nr, char *speicher);
	int16 ged_idx(int16 x, int16 y, int16 x_anz, char *speicher);
	int16 ged_idx(int16 g_idx, int16 x_anz, char *speicher);

private:

	// FIXME : was FILE*
	void load_ged_chunk(GedChunkHeader *Gh, void *handle, int16 nr, char *speicher);

	GedPoolHeader ged_pool_header;
	int16(*GedUserFunc)(int16 idx_nr);

};
#endif
