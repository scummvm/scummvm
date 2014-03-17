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

#ifndef ILLUSIONS_SCREEN_H
#define ILLUSIONS_SCREEN_H

#include "illusions/spritedrawqueue.h"
#include "illusions/spritedecompressqueue.h"
#include "graphics/surface.h"

namespace Illusions {

class IllusionsEngine;

class Screen {
public:
	Screen(IllusionsEngine *vm);
	~Screen();
	Graphics::Surface *allocSurface(int16 width, int16 height);
	Graphics::Surface *allocSurface(SurfInfo &surfInfo);
	bool isDisplayOn();
	uint16 getColorKey2();
	Graphics::Surface *getBackSurface();
	void updateSprites();
	void drawSurface10(int16 destX, int16 destY, Graphics::Surface *surface, Common::Rect &srcRect, uint16 colorKey);
	void drawSurface11(int16 destX, int16 destY, Graphics::Surface *surface, Common::Rect &srcRect);
	void drawSurface20(Common::Rect &dstRect, Graphics::Surface *surface, Common::Rect &srcRect, uint16 colorKey);
	void drawSurface21(Common::Rect &dstRect, Graphics::Surface *surface, Common::Rect &srcRect);
public:
	IllusionsEngine *_vm;
	bool _displayOn;
	uint16 _colorKey2;
	SpriteDecompressQueue *_decompressQueue;
	SpriteDrawQueue *_drawQueue;
	Graphics::Surface *_backSurface;
};

} // End of namespace Illusions

#endif // ILLUSIONS_SCREEN_H
