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

#ifndef BBVS_GRAPHICS_H
#define BBVS_GRAPHICS_H

#include "bbvs/spritemodule.h"
#include "common/array.h"
#include "common/system.h"
#include "graphics/surface.h"

namespace Bbvs {

struct DrawListEntry {
	int index;
	int x, y;
	int priority;
};

class DrawList : public Common::Array<DrawListEntry> {
public:
	void add(int index, int x, int y, int priority);
};

class Screen {
public:
	Screen(OSystem *system);
	~Screen();
	void setPalette(Palette &palette);
	void copyToScreen();
	void drawDrawList(DrawList &drawList, SpriteModule *spriteModule);
	void drawSprite(Sprite &sprite, int x, int y);
	void clear();
//protected:
	OSystem *_system;
	Graphics::Surface *_surface;
};

} // End of namespace Bbvs

#endif // BBVS_GRAPHICS_H
