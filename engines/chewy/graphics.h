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

#ifndef CHEWY_GRAPHICS_H
#define CHEWY_GRAPHICS_H

#include "chewy/chewy.h"
#include "common/rect.h"
#include "graphics/surface.h"

namespace Chewy {

class SpriteResource;
class Font;

class Graphics {
public:
	Graphics(ChewyEngine *vm);
	virtual ~Graphics();

	void drawImage(Common::String filename, int imageNum);
	void drawSprite(Common::String filename, int spriteNum, uint x, uint y);
	void drawRect(Common::Rect r, byte color);
	void playVideo(uint num);
	void loadFont(Common::String filename);
	void drawText(Common::String text, uint x, uint y);
	void setDescSurface(Common::Point pos);
	void restoreDescSurface();

private:
	void drawTransparent(uint16 x, uint16 y, byte *data, uint16 width, uint16 height, byte transparentColor);

	ChewyEngine *_vm;
	Font *_font;
	Common::Point _descPos;
	::Graphics::Surface _descSurface;
};

} // End of namespace Chewy

#endif
