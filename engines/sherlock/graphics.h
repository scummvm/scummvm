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

#ifndef SHERLOCK_GRAPHICS_H
#define SHERLOCK_GRAPHICS_H

#include "common/rect.h"
#include "graphics/surface.h"

#include "sherlock/sprite.h"

namespace Sherlock {

#define PALETTE_SIZE 768
#define PALETTE_COUNT 256

class SherlockEngine;

class Surface : public Graphics::Surface {
public:
    Surface(uint16 width, uint16 height);
    ~Surface();
	void fillRect(int x1, int y1, int x2, int y2, byte color);
	void drawSprite(int x, int y, SpriteFrame *spriteFrame, bool flipped = false, bool altFlag = false);
};

class Screen : public Surface {
private:
	SherlockEngine *_vm;
	int _fontNumber;
	Surface _backBuffer1, _backBuffer2;
public:
	Screen(SherlockEngine *vm);

	void setFont(int fontNumber);

	void update();

	void getPalette(byte palette[PALETTE_SIZE]);

	void setPalette(const byte palette[PALETTE_SIZE]);

	void fadeToBlack();
};

} // End of namespace Sherlock

#endif
