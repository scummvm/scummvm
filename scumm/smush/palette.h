/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001/2002 The ScummVM project
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

#ifndef SMUSH_PALETTE_H
#define SMUSH_PALETTE_H

#include "config.h"

#include "color.h"

/*! 	@brief simple class for handling a palette.

	This small class is an helper for palettes.
*/
class Palette {
private:
	Color _colors[256];
public:
	Palette() {}
	Palette(byte *ptr)
	{
		for(int32 i = 0; i < 256; i++) {
			_colors[i] = Color(ptr[3 * i + 0], ptr[3 * i + 1], ptr[3 * i + 2]);
		}
	
	}
	const Color & operator[](int32 a) const
	{
		assert(a >= 0 && a < 256);
		return _colors[a];
	}
	Color & operator[](int32 a)
	{
		assert(a >= 0 && a < 256);
		return _colors[a];
	}
};

#endif
