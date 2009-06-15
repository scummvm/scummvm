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
 * $URL$
 * $Id$
 *
 */

#ifndef ASYLUM_SCREEN_H_
#define ASYLUM_SCREEN_H_

#include "asylum/asylum.h"
#include "asylum/graphics.h"
#include "asylum/respack.h"

#include "common/system.h"  // for OSystem
#include "graphics/surface.h"

namespace Asylum {

class Screen {
public:
	Screen(OSystem *sys);
	~Screen();

	void copyToBackBuffer(byte *buffer, int pitch, int x, int y, int width, int height);
	void copyBackBufferToScreen();
	void copyRectToScreen(byte *buffer, int pitch, int x, int y, int width, int height);
	void copyRectToScreenWithTransparency(byte *buffer, int pitch, int x, int y, int width, int height);
	void setPalette(byte *rgbPalette);
	void setPalette(ResourcePack *resPack, int entry) { setPalette(resPack->getResource(entry)->data + 32); }

	void showCursor() { _sys->showMouse(true); }
	void hideCursor() { _sys->showMouse(false); }
	void setCursor(byte *data, byte width, byte height) { _sys->setMouseCursor(data, width, height, 0, 0, 0); }
	void setCursor(GraphicResource *cursorRes, int frame) {
		GraphicFrame *mouseCursor = cursorRes->getFrame(frame);
		setCursor((byte *)mouseCursor->surface.pixels, mouseCursor->surface.w, mouseCursor->surface.h);
	}

private:
	Graphics::Surface _backBuffer;
	OSystem *_sys;
};

} // end of namespace Asylum

#endif
