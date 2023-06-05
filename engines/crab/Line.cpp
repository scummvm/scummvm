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

/*
 * This code is based on the CRAB engine
 *
 * Copyright (c) Arvind Raja Yadav
 *
 * Licensed under MIT
 *
 */

#include "crab/crab.h"
#include "crab/Line.h"

namespace Crab {

//------------------------------------------------------------------------
// Purpose: Draw a line from start to end
//------------------------------------------------------------------------
void DrawLine(const int &x1, const int &y1, const int &x2, const int &y2,
			  const uint8 &r, const uint8 &g, const uint8 &b, const uint8 &a) {
	g_engine->_screen->drawLine(x1, y1, x2, y2, g_engine->_format->ARGBToColor(a, r, g, b));

#if 0
	SDL_SetRenderDrawColor(gRenderer, r, g, b, a);
	SDL_RenderDrawLine(gRenderer, x1, y1, x2, y2);
#endif
}

} // End of namespace Crab
