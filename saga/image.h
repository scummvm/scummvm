/* ScummVM - Scumm Interpreter
 * Copyright (C) 2005 The ScummVM project
 *
 * The ReInherit Engine is (C)2000-2003 by Daniel Balsom.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

// SAGA Image resource management header file

#ifndef SAGA_IMAGE_H__
#define SAGA_IMAGE_H__

namespace Saga {

#define MIN_IMG_RLECODE    3
#define MODEX_SCANLINE_LIMIT 200

#define SAGA_IMAGE_DATA_OFFSET 776
#define SAGA_IMAGE_HEADER_LEN  8

struct IMAGE_HEADER {
	int width;
	int height;
};

} // End of namespace Saga

#endif
