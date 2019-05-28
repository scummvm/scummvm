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
#ifndef SLUDGE_SPRITES_H
#define SLUDGE_SPRITES_H

#include "graphics/surface.h"
#include "graphics/transparent_surface.h"

namespace Sludge {

struct Sprite {
	int xhot, yhot;
	Graphics::Surface surface;
	Graphics::Surface burnSurface;
};

class SpritePalette {
public:
	uint16 *pal;
	byte *r;
	byte *g;
	byte *b;
	byte originalRed, originalGreen, originalBlue, total;

	SpritePalette() { init(); }
	~SpritePalette() { kill(); }

	void init() {
		pal = nullptr;
		r = g = b = nullptr;
		total = 0;
		originalRed = originalGreen = originalBlue = 255;
	}

	void kill() {
		if (pal) {
			delete[] pal;
			pal = nullptr;
		}
		if (r) {
			delete[] r;
			r = nullptr;
		}
		if (g) {
			delete[] g;
			g = nullptr;
		}
		if (b) {
			delete[] b;
			b = nullptr;
		}
	}

	void setColor(byte red, byte green, byte blue) {
			originalRed = red;
			originalGreen = green;
			originalBlue = blue;
		}
};

struct SpriteBank {
	int total;
	int type;
	Sprite *sprites;
	SpritePalette myPalette;
	bool isFont;
};

// Sprite display informations
struct SpriteDisplay {
	int x, y;
	int width, height;
	bool freeAfterUse;
	Graphics::FLIP_FLAGS flip;
	Graphics::Surface *surface;

	SpriteDisplay(int xpos, int ypos, Graphics::FLIP_FLAGS f, Graphics::Surface *ptr, int w = -1, int h = 1, bool free = false) :
			x(xpos), y(ypos), flip(f), surface(ptr), width(w), height(h), freeAfterUse(free) {
	}
};

// All sprites are sorted into different "layers" (up to 16) according to their relative y position to z-buffer zones
typedef Common::List<SpriteDisplay *> SpriteLayer;
struct SpriteLayers {
	int numLayers;
	SpriteLayer layer[16];
};

} // End of namespace Sludge

#endif
