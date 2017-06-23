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
 * *
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

#ifndef PRINCE_GRAPHICS_H
#define PRINCE_GRAPHICS_H

#include "graphics/surface.h"

namespace Prince {

class PrinceEngine;
class MhwanhDecoder;
struct DrawNode;

class GraphicsMan {
public:
	GraphicsMan(PrinceEngine *vm);
	~GraphicsMan();

	void update(Graphics::Surface *screen);

	void change();

	void setPalette(const byte *palette);
	void makeShadowTable(int brightness, byte *shadowTable);

	void draw(Graphics::Surface *screen, const Graphics::Surface *s);
	void drawTransparentSurface(Graphics::Surface *screen, int32 posX, int32 posY, const Graphics::Surface *s, int secondTransColor = 0);
	void drawAsShadowSurface(Graphics::Surface *screen, int32 posX, int32 posY, const Graphics::Surface *s, byte *shadowTable);
	void drawTransparentWithBlendSurface(Graphics::Surface *screen, int32 posX, int32 posY, const Graphics::Surface *s);

	static void drawTransparentDrawNode(Graphics::Surface *screen, DrawNode *drawNode);
	static void drawTransparentWithTransDrawNode(Graphics::Surface *screen, DrawNode *drawNode);
	static void drawAsShadowDrawNode(Graphics::Surface *screen, DrawNode *drawNode);
	static void drawMaskDrawNode(Graphics::Surface *screen, DrawNode *drawNode);
	static void drawBackSpriteDrawNode(Graphics::Surface *screen, DrawNode *drawNode);

	byte getBlendTableColor(byte pixelColor, byte backgroundPixelColor, byte *blendTable);

	Graphics::Surface *_frontScreen;
	Graphics::Surface *_screenForInventory;
	Graphics::Surface *_mapScreen;
	const Graphics::Surface *_roomBackground;

	byte *_shadowTable70;
	byte *_shadowTable50;

	static const byte kShadowColor = 191;

private:
	PrinceEngine *_vm;
	bool _changed;
};

} // End of namespace Prince

#endif
