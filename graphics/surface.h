/* ScummVM - Scumm Interpreter
 * Copyright (C) 2002-2004 The ScummVM project
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

#ifndef GRAPHICS_SURFACE_H
#define GRAPHICS_SURFACE_H

#include "common/scummsys.h"

namespace Graphics {


/**
 * An arbitrary graphics surface, which can be the target (or source) of blit
 * operations, font rendering, etc.
 */
struct Surface {
	void *pixels;
	uint16 w;
	uint16 h;
	uint16 pitch;
	uint8 bytesPerPixel;
	Surface() : pixels(0), w(0), h(0), pitch(0), bytesPerPixel(0) {}
};


} // End of namespace Graphics


#endif
