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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#ifndef DRACI_SCREEN_H
#define DRACI_SCREEN_H

#include "draci/surface.h"
#include "draci/sprite.h"

namespace Draci {

enum ScreenParameters {
	kScreenWidth = 320,
	kScreenHeight = 200,
	kNumColours = 256,
	kDefaultTransparent = 255
};

class DraciEngine;

class Screen {

public:
	Screen(DraciEngine *vm);
	~Screen();

	void setPaletteEmpty(unsigned int numEntries = kNumColours);
	void setPalette(byte *data, uint16 start, uint16 num);
	byte *getPalette() const;
	void copyToScreen() const;
	void clearScreen() const;
	void fillScreen(uint8 colour) const;
	Surface *getSurface();	
	void drawRect(Common::Rect &r, uint8 colour);

private:
	Surface *_surface;
	byte *_palette;
	DraciEngine *_vm;
};

} // End of namespace Draci

#endif // DRACI_SCREEN_H
