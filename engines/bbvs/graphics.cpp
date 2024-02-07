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

#include "graphics/paletteman.h"

#include "bbvs/graphics.h"

namespace Bbvs {

void DrawList::add(int index, int x, int y, int priority) {
	debug(5, "DrawList::add() %d (%d, %d) %d", index, x, y, priority);
	DrawListEntry drawListEntry;
	drawListEntry.index = index;
	drawListEntry.x = x;
	drawListEntry.y = y;
	drawListEntry.priority = priority;
	// Insert the sprite at the correct position
	uint insertIndex = 0;
	while (insertIndex < size() && (*this)[insertIndex].priority <= priority)
		++insertIndex;
	insert_at(insertIndex, drawListEntry);
}

Screen::Screen(OSystem *system) : _system(system) {
	_surface = new Graphics::Surface();
	_surface->create(320, 240, Graphics::PixelFormat::createFormatCLUT8());
}

Screen::~Screen() {
	_surface->free();
	delete _surface;
}

void Screen::setPalette(Palette &palette) {
	byte pal[768];
	memset(pal, 0, 768);
	memcpy(&pal[palette.start * 3], palette.data, palette.count * 3);
	_system->getPaletteManager()->setPalette(pal, 0, 256);
}

void Screen::copyToScreen() {
	_system->copyRectToScreen((const byte*)_surface->getBasePtr(0, 0), _surface->pitch, 0, 0, 320, 240);
	_system->updateScreen();
}

void Screen::clear() {
	_surface->fillRect(Common::Rect(0, 0, 320, 240), 0);
}

void Screen::drawDrawList(DrawList &drawList, SpriteModule *spriteModule) {
	for (uint i = 0; i < drawList.size(); ++i) {
		debug(4, "index: %d; x: %d; y: %d; priority: %d", drawList[i].index, drawList[i].x, drawList[i].y, drawList[i].priority);
		Sprite sprite = spriteModule->getSprite(drawList[i].index);
		drawSprite(sprite, drawList[i].x, drawList[i].y);
	}
}

void Screen::drawSprite(Sprite &sprite, int x, int y) {
	debug(5, "Screen::drawSprite()");

	int destX, destY, width, height, skipX = 0, skipY = 0;

	destX = sprite.xOffs + x;
	destY = sprite.yOffs + y;

	if (destX >= _surface->w || destY >= _surface->h)
		return;

	height = sprite.height;
	if (destY < 0) {
		height += destY;
		if (height <= 0)
			return;
		skipY = -destY;
		destY = 0;
	}
	if (destY + height > _surface->h)
		height = _surface->h - destY;

	width = sprite.width;
	if (destX < 0) {
		width += destX;
		if (width <= 0)
			return;
		skipX = -destX;
		destX = 0;
	}
	if (destX + width >= _surface->w)
		width = _surface->w - destX;

	debug(6, "drawSprite() (%d, %d, %d, %d); skipX: %d; skipY: %d; %d", destX, destY, width, height, skipX, skipY, sprite.type);

	if (sprite.type == 1) {
		for (int yc = 0; yc < height; ++yc) {
			byte *source = sprite.getRow(skipY + yc);
			byte *dest = (byte*)_surface->getBasePtr(destX, destY + yc);
			int currWidth = -skipX;
			while (currWidth < width) {
				int8 op = *source++;
				if (op < 0) {
					currWidth += (-op);
				} else {
					while (op >= 0 && currWidth < width) {
						if (currWidth >= 0)
							dest[currWidth] = *source;
						++source;
						++currWidth;
						--op;
					}
				}
			}
		}
	} else {
		for (int yc = 0; yc < height; ++yc) {
			byte *source = sprite.getRow(skipY + yc) + skipX;
			byte *dest = (byte*)_surface->getBasePtr(destX, destY + yc);
			memcpy(dest, source, width);
		}
	}

	debug(5, "Screen::drawSprite() OK");
}

} // End of namespace Bbvs
