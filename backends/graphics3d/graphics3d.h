/* ResidualVM - Graphic Adventure Engine
 *
 * ResidulVM is the legal property of its developers, whose names
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

#ifndef BACKENDS_GRAPHICS3D_ABSTRACT_H
#define BACKENDS_GRAPHICS3D_ABSTRACT_H

#include "common/system.h"
#include "common/noncopyable.h"
#include "common/keyboard.h"
#include "common/config-manager.h"

#include "graphics/mode.h"
#include "graphics/palette.h"

#include "backends/graphics/graphics.h"

/**
 * Abstract class for graphics manager. Subclasses
 * implement the real functionality.
 */
class Graphics3dManager : public GraphicsManager {
public:
	// Following methods are not used by 3D graphics managers
#ifdef USE_RGB_COLOR
	virtual Common::List<Graphics::PixelFormat> getSupportedFormats() const {
		Common::List<Graphics::PixelFormat> supportedFormats;
		return supportedFormats;
	}
#endif
	virtual void setPalette(const byte *colors, uint start, uint num) {}
	virtual void grabPalette(byte *colors, uint start, uint num) const {}
	virtual void copyRectToScreen(const void *buf, int pitch, int x, int y, int w, int h) {}
	virtual Graphics::Surface *lockScreen() { return nullptr; }
	virtual void unlockScreen() {}
	virtual void fillScreen(uint32 col) {}
	virtual void setShakePos(int shakeXOffset, int shakeYOffset) {};
	virtual void setFocusRectangle(const Common::Rect& rect) {}
	virtual void clearFocusRectangle() {}
	virtual void setMouseCursor(const void *buf, uint w, uint h, int hotspotX, int hotspotY, uint32 keycolor, bool dontScale = false, const Graphics::PixelFormat *format = NULL) {}
	virtual void setCursorPalette(const byte *colors, uint start, uint num) {}

	// Stubs for windowed gfx manager calls
	int getWindowWidth() const { return 0; }
	int getWindowHeight() const { return 0; }
	virtual void unlockWindowSize() {}
};

#endif
