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

#ifndef NUVIE_GUI_GUI_TYPES_H
#define NUVIE_GUI_GUI_TYPES_H

/* Basic GUI data types and classes */

#include "ultima/nuvie/misc/sdl_compat.h"

namespace Ultima {
namespace Nuvie {

class GUI_image {

public:
	/* Load an image from a BMP file */
	GUI_image(char *file);

	/* Use a Graphics::ManagedSurface as the image
	   The surface shouldn't be freed while the miage object exists.
	 */
	GUI_image(Graphics::ManagedSurface *picture, int shouldfree = 0);

private:
	Graphics::ManagedSurface *image;
};

class GUI_Color {
public:

	uint8 r;
	uint8 g;
	uint8 b;
//uint8 a;

	uint32 sdl_color;

public:

	GUI_Color(uint8 red, uint8 green, uint8 blue) {
		r = red;
		g = green;
		b = blue;
		sdl_color = 0;
	};
	GUI_Color() {
		r = g = b = 0;
		sdl_color = 0;
	};
	void map_color(Graphics::ManagedSurface *surface) {
		sdl_color = SDL_MapRGB(surface->format, r, g, b);
	};

};

} // End of namespace Nuvie
} // End of namespace Ultima

#endif
