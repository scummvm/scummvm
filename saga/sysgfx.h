/* ScummVM - Scumm Interpreter
 * Copyright (C) 2004 The ScummVM project
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
#ifndef SYSGFX_H_
#define SYSGFX_H_

namespace Saga {

#define R_COLORSEARCH_SQUARE 0

#define R_RED_WEIGHT 0.299
#define R_GREEN_WEIGHT 0.587
#define R_BLUE_WEIGHT 0.114

struct R_SYSGFX_MODULE {
	int init;

	SDL_Surface *sdl_screen;	/* Screen surface */
	R_SURFACE r_screen;

	SDL_Surface *sdl_back_buf;	/* Double buffer surface */
	R_SURFACE r_back_buf;

	int white_index;
	int black_index;
};

} // End of namespace Saga

#endif				/* SYSGFX_H_ */
