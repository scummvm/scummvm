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

#ifndef NUVIE_MISC_U6_LINE_WALKER_H
#define NUVIE_MISC_U6_LINE_WALKER_H

#include "ultima/nuvie/core/nuvie_defs.h"

namespace Ultima {
namespace Nuvie {

class U6LineWalker {
protected:
	uint32 start_x, start_y;
	uint32 end_x, end_y;

	sint32 xoffset[2], yoffset[2];
	sint32 line_inc[2];
	sint32 line_counter;

	uint32 cur_x, cur_y;
	uint32 max_length;

	uint32 cur_step;

public:
	U6LineWalker(uint32 sx, uint32 sy, uint32 ex, uint32 ey);
	virtual ~U6LineWalker();

	bool step();
	bool next(uint32 *x, uint32 *y);

};

} // End of namespace Nuvie
} // End of namespace Ultima

#endif
