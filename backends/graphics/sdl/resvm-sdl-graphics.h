/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
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

#ifndef BACKENDS_GRAPHICS_SDL_RESVMSDLGRAPHICS_H
#define BACKENDS_GRAPHICS_SDL_RESVMSDLGRAPHICS_H

#include "backends/graphics/sdl/sdl-graphics.h"

#include "common/events.h"
#include "common/rect.h"

#include "math/rect2d.h"

class SdlEventSource;

/**
 * Base class for a ResidualVM SDL based graphics manager.
 *
 * Used to share reusable methods between SDL graphics managers
 */
class ResVmSdlGraphicsManager : public SdlGraphicsManager, public Common::EventObserver {
public:
	/**
	 * Capabilities of the current device
	 */
	struct Capabilities {
		/**
		 * Desktop resolution
		 */
		uint desktopWidth;
		uint desktopHeight;

		/**
		 * Is the device capable of rendering to OpenGL framebuffers
		 */
		bool openGLFrameBuffer;

		/** Supported levels of MSAA when using the OpenGL renderers */
		Common::Array<uint> openGLAntiAliasLevels;

		Capabilities() :
				desktopWidth(0), desktopHeight(0),
				openGLFrameBuffer(false) {}
	};

	ResVmSdlGraphicsManager(SdlEventSource *source, SdlWindow *window, const Capabilities &capabilities);
	~ResVmSdlGraphicsManager() override;

	// SdlGraphicsManager API
	void activateManager() override;
	void deactivateManager() override;
	void notifyVideoExpose() override;
	bool notifyMousePosition(Common::Point &mouse) override;

	// GraphicsManager API - Features
	void setFeatureState(OSystem::Feature f, bool enable) override;
	bool getFeatureState(OSystem::Feature f) const override;

	// GraphicsManager API - Graphics mode
	const OSystem::GraphicsMode *getSupportedGraphicsModes() const override;
	int getDefaultGraphicsMode() const override;
	bool setGraphicsMode(int mode) override;
	int getGraphicsMode() const override;
	void resetGraphicsScale() override;
#ifdef USE_RGB_COLOR
	Graphics::PixelFormat getScreenFormat() const override { return _screenFormat; }
	Common::List<Graphics::PixelFormat> getSupportedFormats() const override;
#endif
	void initSize(uint w, uint h, const Graphics::PixelFormat *format = nullptr) override;
	int getScreenChangeID() const override { return _screenChangeCount; }
	void beginGFXTransaction() override;
	OSystem::TransactionError endGFXTransaction() override;

protected:
	// PaletteManager API
	void setPalette(const byte *colors, uint start, uint num) override;
	void grabPalette(byte *colors, uint start, uint num) const override;

public:
	// GraphicsManager API - Draw methods
	void copyRectToScreen(const void *buf, int pitch, int x, int y, int w, int h) override;
	Graphics::Surface *lockScreen() override;
	void unlockScreen() override;
	void fillScreen(uint32 col) override;
	void setShakePos(int shakeOffset) override;
	void saveScreenshot() override;

	// GraphicsManager API - Focus Rectangle
	void setFocusRectangle(const Common::Rect& rect) override;
	void clearFocusRectangle() override;

	// GraphicsManager API - Overlay
	Graphics::PixelFormat getOverlayFormat() const override { return _overlayFormat; }

	// GraphicsManager API - Mouse
	bool showMouse(bool visible) override;
	bool lockMouse(bool lock) override; // ResidualVM specific method
	void setMouseCursor(const void *buf, uint w, uint h, int hotspotX, int hotspotY, uint32 keycolor, bool dontScale = false, const Graphics::PixelFormat *format = nullptr) override;
	void setCursorPalette(const byte *colors, uint start, uint num) override;

#ifdef USE_OSD
	void displayMessageOnOSD(const char *msg) override;
#endif

	// Common::EventObserver API
	bool notifyEvent(const Common::Event &event) override;

	/**
	 * Checks if mouse is locked or not.
	 * Avoid to emulate a mouse movement from joystick if locked.
	 */
	bool isMouseLocked() const;

protected:
	const Capabilities &_capabilities;

	bool _fullscreen;
	bool _lockAspectRatio;
	uint _engineRequestedWidth, _engineRequestedHeight;

	int _screenChangeCount;

	bool _overlayVisible;
	Graphics::PixelFormat _overlayFormat;

#ifdef USE_RGB_COLOR
	Graphics::PixelFormat _screenFormat;
	Common::List<Graphics::PixelFormat> _supportedFormats;
#endif

	/** Obtain the user configured fullscreen resolution, or default to the desktop resolution */
	Common::Rect getPreferredFullscreenResolution();

	/** Save a screenshot to the specified file */
	virtual bool saveScreenshot(const Common::String &file) const  = 0;
};

#endif
