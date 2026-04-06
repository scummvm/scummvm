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
 */

#ifndef SCOOBY_GFX_H
#define SCOOBY_GFX_H

#include "scooby/md/vdp.h"
#include "graphics/surface.h"

namespace Scooby {

class Gfx {
private:
	Graphics::PixelFormat _pixelFormat;
	Graphics::Surface *_backSurface;
	VDP *_vdp;
public:
	Gfx(VDP *vdp);

	void drawFrame();

	void loadTilemap(VDP::PlaneType planeType, byte *tilemapData, uint16 x, uint16 y, uint16 w, uint16 h);
	void loadTilemap(uint16 destAddress, byte *tilemapData, uint16 w, uint16 h, uint16 pitch);

private:
	void updateScreen();
};

} // End of namespace Scooby

#endif //SCOOBY_GFX_H
