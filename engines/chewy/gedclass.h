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

#ifndef CHEWY_GEDCLASS_H
#define CHEWY_GEDCLASS_H

#include "chewy/ngstypes.h"

namespace Chewy {

class GedClass {
public:
	GedClass() {}

	void load_ged_pool(GedChunkHeader *Gh, int16 ch_nr, byte *speicher);

	int16 getBarrierId(int16 x, int16 y, int16 x_anz, byte *speicher);
	int16 getBarrierId(int16 g_idx, byte *speicher);
};

} // namespace Chewy

#endif
