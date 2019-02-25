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
#ifndef SCUMMVM_SCREEN_H
#define SCUMMVM_SCREEN_H

#include "graphics/surface.h"
#include "graphics/pixelformat.h"

namespace Dragons {

class Screen {
private:
	Graphics::PixelFormat _pixelFormat;
	Graphics::Surface *_backSurface;
public:
	virtual ~Screen();

	Screen();

	Graphics::PixelFormat getPixelFormat() { return _pixelFormat; }
	void copyRectToSurface(const Graphics::Surface &srcSurface, int destX, int destY);
	void copyRectToSurface(const Graphics::Surface &srcSurface, int destX, int destY, Common::Rect srcRect, bool flipX = false);
	void updateScreen();
private:
	void copyRectToSurface(const void *buffer, int srcPitch, int srcWidth, int srcXOffset, int destX, int destY, int width, int height, bool flipX);
	Common::Rect clipRectToScreen(int destX, int destY, const Common::Rect rect);
};

} // End of namespace Dragons

#endif //SCUMMVM_SCREEN_H
