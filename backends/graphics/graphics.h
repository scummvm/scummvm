/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef BACKENDS_GRAPHICS_ABSTRACT_H
#define BACKENDS_GRAPHICS_ABSTRACT_H

#include "common/system.h"
#include "common/noncopyable.h"
#include "common/keyboard.h"
#include "common/rotationmode.h"

#include "graphics/mode.h"
#include "graphics/paletteman.h"

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
#if defined(USE_IMGUI)
	virtual void setImGuiCallbacks(const ImGuiCallbacks &callbacks) { }
	virtual void *getImGuiTexture(const Graphics::Surface &image, const byte *palette, int palCount) { return nullptr; }
	virtual void freeImGuiTexture(void *texture) { }
#endif
	virtual bool setShader(const Common::Path &fileName) { return false; }
	virtual const OSystem::GraphicsMode *getSupportedStretchModes() const {
		static const OSystem::GraphicsMode noStretchModes[] = {{"NONE", "Normal", 0}, {nullptr, nullptr, 0 }};
		return noStretchModes;
	}
	virtual int getDefaultStretchMode() const { return 0; }
	virtual bool setStretchMode(int mode) { return false; }
	virtual int getStretchMode() const { return 0; }
	virtual Common::RotationMode getRotationMode() const { return Common::kRotationNormal; }
	virtual uint getDefaultScaler() const { return 0; }
	virtual uint getDefaultScaleFactor() const { return 1; }
	virtual bool setScaler(uint mode, int factor) { return false; }
	virtual uint getScaler() const { return 0; }
	virtual uint getScaleFactor() const { return 1; }

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
	virtual void fillScreen(const Common::Rect &r, uint32 col) = 0;
	virtual void updateScreen() = 0;
	virtual void setShakePos(int shakeXOffset, int shakeYOffset) = 0;
	virtual void setFocusRectangle(const Common::Rect& rect) = 0;
	virtual void clearFocusRectangle() = 0;

	virtual void showOverlay(bool inGUI) = 0;
	virtual void hideOverlay() = 0;
	virtual bool isOverlayVisible() const = 0;
	virtual Graphics::PixelFormat getOverlayFormat() const = 0;
	virtual void clearOverlay() = 0;
	virtual void grabOverlay(Graphics::Surface &surface) const = 0;
	virtual void copyRectToOverlay(const void *buf, int pitch, int x, int y, int w, int h) = 0;
	virtual int16 getOverlayHeight() const = 0;
	virtual int16 getOverlayWidth() const = 0;
	virtual float getHiDPIScreenFactor() const { return 1.0f; }

	virtual bool showMouse(bool visible) = 0;
	virtual void warpMouse(int x, int y) = 0;
	virtual void setMouseCursor(const void *buf, uint w, uint h, int hotspotX, int hotspotY, uint32 keycolor, bool dontScale = false, const Graphics::PixelFormat *format = nullptr, const byte *mask = nullptr) = 0;
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
