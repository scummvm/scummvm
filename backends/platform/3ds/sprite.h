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

#define TEXTURE_TRANSFER_FLAGS \
	(GX_TRANSFER_FLIP_VERT(1) | GX_TRANSFER_OUT_TILED(1) | GX_TRANSFER_RAW_COPY(0) | \
	GX_TRANSFER_IN_FORMAT(GX_TRANSFER_FMT_RGBA8) | GX_TRANSFER_OUT_FORMAT(GX_TRANSFER_FMT_RGBA8) | \
	GX_TRANSFER_SCALING(GX_TRANSFER_SCALE_NO))

typedef struct {
	float position[3];
	float texcoord[2];
} vertex;

class Sprite : public Graphics::Surface {
public:
	Sprite();
	~Sprite();
	void create(uint16 width, uint16 height, const Graphics::PixelFormat &format);
	void free();
	void convertToInPlace(const Graphics::PixelFormat &dstFormat, const byte *palette = 0);
	void render();
	void clear(uint32 color = 0);
	void markDirty(){ dirtyPixels = true; }

	void setPosition(int x, int y);
	void setScale(float x, float y);
	float getScaleX(){ return scaleX; }
	float getScaleY(){ return scaleY; }
	C3D_Mtx* getMatrix();

	uint16 actualWidth;
	uint16 actualHeight;

private:
	bool dirtyPixels;
	bool dirtyMatrix;
	C3D_Mtx modelview;
	C3D_Tex texture;
	vertex* vertices;
	int posX;
	int posY;
	float scaleX;
	float scaleY;
};

#endif
