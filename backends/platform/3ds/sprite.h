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

#ifndef GRAPHICS_SPRITE_3DS_H
#define GRAPHICS_SPRITE_3DS_H

#define FORBIDDEN_SYMBOL_EXCEPTION_time_h

#include "graphics/surface.h"
#include <3ds.h>
#include <citro3d.h>

namespace _3DS {

typedef struct {
	float position[3];
	float texcoord[2];
} vertex;

struct GfxMode3DS;

class Sprite : public Graphics::Surface {
public:
	Sprite();
	~Sprite();
	void create(uint16 width, uint16 height, const GfxMode3DS *mode);
	void free();
	void convertToInPlace(const Graphics::PixelFormat &dstFormat, const byte *palette = 0);
	void transfer();
	void render();
	void clear(uint32 color = 0);
	void markDirty(){ dirtyPixels = true; }

	void setPosition(int x, int y);
	void setOffset(uint16 x, uint16 y);
	void setScale(float x, float y);
	float getScaleX() const { return scaleX; }
	float getScaleY() const { return scaleY; }
	int getPosX() const { return posX; }
	int getPosY() const { return posY; }
	C3D_Mtx* getMatrix();

	void setFilteringMode(bool enableLinearFiltering);

	uint16 actualWidth;
	uint16 actualHeight;

private:
	uint32 textureTransferFlags;
	bool dirtyPixels;
	bool dirtyMatrix;
	C3D_Mtx modelview;
	C3D_Tex texture;
	vertex* vertices;
	int posX;
	int posY;
	uint16 offsetX;
	uint16 offsetY;
	float scaleX;
	float scaleY;
};

} // namespace _3DS

#endif
