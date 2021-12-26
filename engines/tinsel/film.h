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

#ifndef TINSEL_FILM_H	// prevent multiple includes
#define TINSEL_FILM_H

#include "tinsel/dw.h"

namespace Tinsel {

#include "common/pack-start.h"	// START STRUCT PACKING

struct FREEL {
	SCNHANDLE mobj;
	SCNHANDLE script;
} PACKED_STRUCT;

struct FILM {
	int32	frate;
	int32	numreels;
	FREEL	reels[1];
} PACKED_STRUCT;

#include "common/pack-end.h"	// END STRUCT PACKING

} // End of namespace Tinsel

#endif
