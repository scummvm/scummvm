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

#include "ultima/nuvie/misc/u6_line_walker.h"

namespace Ultima {
namespace Nuvie {

U6LineWalker::U6LineWalker(uint32 sx, uint32 sy, uint32 ex, uint32 ey) {

	start_x = sx;
	start_y = sy;
	end_x = ex;
	end_y = ey;

	cur_x = start_x;
	cur_y = start_y;

	sint32 delta_x = end_x - start_x;
	sint32 delta_y = end_y - start_y;

	yoffset[0] = 0;
	if (delta_x >= 0) {
		xoffset[0] = 1;
		xoffset[1] = 1;
	} else {
		delta_x = -delta_x;
		xoffset[0] = -1;
		xoffset[1] = -1;
	}

	if (delta_y >= 0) {
		yoffset[1] = 1;
	} else {
		delta_y = -delta_y;
		yoffset[1] = -1;
	}

	if (delta_x < delta_y) {
		sint32 tmp_var = delta_x;
		delta_x = delta_y;
		delta_y = tmp_var;
		yoffset[0] = yoffset[1];
		xoffset[1] = xoffset[0];
		xoffset[0] = 0;
	}

	max_length = delta_x;

	line_counter = delta_y + delta_y - delta_x;
	line_inc[0] = delta_y + delta_y;
	line_inc[1] = (delta_y - delta_x) * 2;

	cur_step = 0;
}

U6LineWalker::~U6LineWalker() {
	// TODO Auto-generated destructor stub
}


bool U6LineWalker::step() {
	if (cur_step >= max_length)
		return false;

	uint8 idx = line_counter < 0 ? 0 : 1;

	line_counter += line_inc[idx];
	cur_x +=  xoffset[idx];
	cur_y +=  yoffset[idx];

	cur_step++;

	return true;
}

bool U6LineWalker::next(uint32 *x, uint32 *y) {
	bool ret = step();

	*x = cur_x;
	*y = cur_y;

	return ret;
}

} // End of namespace Nuvie
} // End of namespace Ultima
