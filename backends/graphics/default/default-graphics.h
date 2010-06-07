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

#ifndef BACKENDS_GRAPHICS_DEFAULT_H
#define BACKENDS_GRAPHICS_DEFAULT_H

#include "common/system.h"
#include "common/noncopyable.h"

class DefaultGraphicsManager : Common::NonCopyable {
public:
	DefaultGraphicsManager() {}
	~DefaultGraphicsManager() {}

	bool hasGraphicsFeature(OSystem::Feature f);
	void setGraphicsFeatureState(OSystem::Feature f, bool enable) {}
	bool getGraphicsFeatureState(OSystem::Feature f);

	const OSystem::GraphicsMode *getSupportedGraphicsModes();
	int getDefaultGraphicsMode() { return 0; }
	bool setGraphicsMode(int mode) { return true; }
	int getGraphicsMode() { return 0; }
	inline Graphics::PixelFormat getScreenFormat() const {
		return Graphics::PixelFormat::createFormatCLUT8();
	};
	inline Common::List<Graphics::PixelFormat> getSupportedFormats() const {
		Common::List<Graphics::PixelFormat> list;
		list.push_back(Graphics::PixelFormat::createFormatCLUT8());
		return list;
	};
	void initSize(uint width, uint height, const Graphics::PixelFormat *format = NULL) {}
	int16 getHeight() { return 0; }
	int16 getWidth() { return 0; }
	void setPalette(const byte *colors, uint start, uint num) {}
	void grabPalette(byte *colors, uint start, uint num) {}
	void copyRectToScreen(const byte *buf, int pitch, int x, int y, int w, int h) {}
	Graphics::Surface *lockScreen() { return NULL; }
	void unlockScreen() {}
	void fillScreen(uint32 col) {}
	void updateScreen() {}
	void setShakePos(int shakeOffset) {}
	void showOverlay() {}
	void hideOverlay() {}
	Graphics::PixelFormat getOverlayFormat() { return Graphics::PixelFormat(); }
	void clearOverlay() {}
	void grabOverlay(OverlayColor *buf, int pitch) {}
	void copyRectToOverlay(const OverlayColor *buf, int pitch, int x, int y, int w, int h) {}
	int16 getOverlayHeight() { return 0; }
	int16 getOverlayWidth() { return 0; }
	bool showMouse(bool visible) {}
	void warpMouse(int x, int y) {}
	void setMouseCursor(const byte *buf, uint w, uint h, int hotspotX, int hotspotY, uint32 keycolor, int cursorTargetScale = 1, const Graphics::PixelFormat *format = NULL) {}
};


#endif
