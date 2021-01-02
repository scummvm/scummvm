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

#ifndef BACKENDS_GRAPHICS_ABSTRACT_H
#define BACKENDS_GRAPHICS_ABSTRACT_H

#include "common/system.h"
#include "common/noncopyable.h"
#include "common/keyboard.h"

#include "graphics/mode.h"
#include "graphics/palette.h"

/**
 * Abstract class for graphics manager. Subclasses
 * implement the real functionality.
 */
class GraphicsManager : public PaletteManager {
public:
	virtual ~GraphicsManager() {}

	virtual bool hasFeature(OSystem::Feature f) const = 0;
	virtual void setFeatureState(OSystem::Feature f, bool enable) = 0;
	virtual bool getFeatureState(OSystem::Feature f) const = 0;

	virtual const OSystem::GraphicsMode *getSupportedGraphicsModes() const {
		static const OSystem::GraphicsMode noGraphicsModes[] = {{"NONE", "Normal", 0}, {nullptr, nullptr, 0 }};
		return noGraphicsModes;
	}
	virtual int getDefaultGraphicsMode() const { return 0; }
	virtual bool setGraphicsMode(int mode, uint flags = OSystem::kGfxModeNoFlags) { return (mode == 0); }
	virtual int getGraphicsMode() const { return 0; }
	virtual const OSystem::GraphicsMode *getSupportedShaders() const {
		static const OSystem::GraphicsMode no_shader[2] = {{"NONE", "Normal (no shader)", 0}, {0, 0, 0}};
		return no_shader;
	}
	virtual int getDefaultShader() const { return 0; }
	virtual bool setShader(int id) { return false; }
	virtual int getShader() const { return 0; }
	virtual const OSystem::GraphicsMode *getSupportedStretchModes() const {
		static const OSystem::GraphicsMode noStretchModes[] = {{"NONE", "Normal", 0}, {nullptr, nullptr, 0 }};
		return noStretchModes;
	}
	virtual int getDefaultStretchMode() const { return 0; }
	virtual bool setStretchMode(int mode) { return false; }
	virtual int getStretchMode() const { return 0; }

#ifdef USE_RGB_COLOR
	virtual Graphics::PixelFormat getScreenFormat() const = 0;
	virtual Common::List<Graphics::PixelFormat> getSupportedFormats() const = 0;
#endif
	virtual void initSize(uint width, uint height, const Graphics::PixelFormat *format = NULL) = 0;
	virtual void initSizeHint(const Graphics::ModeList &modes) {}
	virtual int getScreenChangeID() const = 0;

	virtual void beginGFXTransaction() = 0;
	virtual OSystem::TransactionError endGFXTransaction() = 0;

	virtual int16 getHeight() const = 0;
	virtual int16 getWidth() const = 0;
	virtual void setPalette(const byte *colors, uint start, uint num) = 0;
	virtual void grabPalette(byte *colors, uint start, uint num) const = 0;
	virtual void copyRectToScreen(const void *buf, int pitch, int x, int y, int w, int h) = 0;
	virtual Graphics::Surface *lockScreen() = 0;
	virtual void unlockScreen() = 0;
	virtual void fillScreen(uint32 col) = 0;
	virtual void updateScreen() = 0;
	virtual void setShakePos(int shakeXOffset, int shakeYOffset) = 0;
	virtual void setFocusRectangle(const Common::Rect& rect) = 0;
	virtual void clearFocusRectangle() = 0;

	virtual void showOverlay() = 0;
	virtual void hideOverlay() = 0;
	virtual bool isOverlayVisible() const = 0;
	virtual Graphics::PixelFormat getOverlayFormat() const = 0;
	virtual void clearOverlay() = 0;
	virtual void grabOverlay(void *buf, int pitch) const = 0;
	virtual void copyRectToOverlay(const void *buf, int pitch, int x, int y, int w, int h) = 0;
	virtual int16 getOverlayHeight() const = 0;
	virtual int16 getOverlayWidth() const = 0;

	virtual bool showMouse(bool visible) = 0;
	virtual void warpMouse(int x, int y) = 0;
	virtual void setMouseCursor(const void *buf, uint w, uint h, int hotspotX, int hotspotY, uint32 keycolor, bool dontScale = false, const Graphics::PixelFormat *format = NULL) = 0;
	virtual void setCursorPalette(const byte *colors, uint start, uint num) = 0;

	virtual void displayMessageOnOSD(const Common::U32String &msg) {}
	virtual void displayActivityIconOnOSD(const Graphics::Surface *icon) {}


	// Graphics::PaletteManager interface
	//virtual void setPalette(const byte *colors, uint start, uint num) = 0;
	//virtual void grabPalette(byte *colors, uint start, uint num) const = 0;

	virtual void saveScreenshot() {}
	virtual bool lockMouse(bool lock) { return false; }
};

#endif
