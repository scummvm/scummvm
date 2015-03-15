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

#include "sherlock/graphics.h"

namespace Sherlock {

Surface::Surface(uint16 width, uint16 height) {
	create(width, height, Graphics::PixelFormat::createFormatCLUT8());
}

Surface::~Surface() {
    free();
}

void Surface::fillRect(int x1, int y1, int x2, int y2, byte color) {
    Graphics::Surface::fillRect(Common::Rect(x1, y1, x2, y2), color);
}

void Surface::drawSprite(int x, int y, SpriteFrame *spriteFrame, bool flipped, bool altFlag) {
	
	
}


} // End of namespace Sherlock
