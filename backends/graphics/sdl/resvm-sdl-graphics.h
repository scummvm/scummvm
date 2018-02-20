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

		Capabilities() :
				desktopWidth(0), desktopHeight(0),
				openGLFrameBuffer(false) {}
	};

	ResVmSdlGraphicsManager(SdlEventSource *source, SdlWindow *window, const Capabilities &capabilities);
	virtual ~ResVmSdlGraphicsManager();

	// SdlGraphicsManager API
	virtual void activateManager() override;
	virtual void deactivateManager() override;
	virtual void notifyVideoExpose() override;
	virtual bool notifyMousePosition(Common::Point mouse) override;

	// GraphicsManager API - Features
	virtual void setFeatureState(OSystem::Feature f, bool enable) override;
	virtual bool getFeatureState(OSystem::Feature f) const override;

	// GraphicsManager API - Graphics mode
	virtual const OSystem::GraphicsMode *getSupportedGraphicsModes() const override;
	virtual int getDefaultGraphicsMode() const override;
	virtual bool setGraphicsMode(int mode) override;
	virtual int getGraphicsMode() const override;
	virtual void resetGraphicsScale() override;
#ifdef USE_RGB_COLOR
	virtual Graphics::PixelFormat getScreenFormat() const override { return _screenFormat; }
	virtual Common::List<Graphics::PixelFormat> getSupportedFormats() const override;
#endif
	virtual void initSize(uint w, uint h, const Graphics::PixelFormat *format = NULL) override;
	virtual int getScreenChangeID() const override { return _screenChangeCount; }
	virtual void beginGFXTransaction() override;
	virtual OSystem::TransactionError endGFXTransaction() override;

protected:
	// PaletteManager API
	virtual void setPalette(const byte *colors, uint start, uint num) override;
	virtual void grabPalette(byte *colors, uint start, uint num) const override;

public:
	// GraphicsManager API - Draw methods
	virtual void copyRectToScreen(const void *buf, int pitch, int x, int y, int w, int h) override;
	virtual Graphics::Surface *lockScreen() override;
	virtual void unlockScreen() override;
	virtual void fillScreen(uint32 col) override;
	virtual void setShakePos(int shakeOffset) override;

	// GraphicsManager API - Focus Rectangle
	virtual void setFocusRectangle(const Common::Rect& rect) override;
	virtual void clearFocusRectangle() override;

	// GraphicsManager API - Overlay
	virtual int16 getOverlayHeight() const override { return _overlayHeight; }
	virtual int16 getOverlayWidth() const override { return _overlayWidth; }
	virtual Graphics::PixelFormat getOverlayFormat() const override { return _overlayFormat; }

	// GraphicsManager API - Mouse
	virtual bool showMouse(bool visible) override;
	virtual bool lockMouse(bool lock) override; // ResidualVM specific method
	virtual void setMouseCursor(const void *buf, uint w, uint h, int hotspotX, int hotspotY, uint32 keycolor, bool dontScale = false, const Graphics::PixelFormat *format = NULL) override;
	virtual void setCursorPalette(const byte *colors, uint start, uint num) override;

#ifdef USE_OSD
	virtual void displayMessageOnOSD(const char *msg) override;
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
	int _overlayWidth, _overlayHeight;

#ifdef USE_RGB_COLOR
	Graphics::PixelFormat _screenFormat;
	Common::List<Graphics::PixelFormat> _supportedFormats;
#endif

	/**
	 * Places where the game can be drawn
	 */
	enum GameRenderTarget {
		kScreen,     /** The game is drawn directly on the screen */
		kSubScreen,  /** The game is drawn to a surface, which is centered on the screen */
		kFramebuffer /** The game is drawn to a framebuffer, which is scaled to fit the screen */
	};

	/** Select the best draw target according to the specified parameters */
	GameRenderTarget selectGameRenderTarget(bool fullscreen, bool accel3d,
	                                        bool engineSupportsArbitraryResolutions,
	                                        bool framebufferSupported,
	                                        bool lockAspectRatio);

	/** Compute the size and position of the game rectangle in the screen */
	Math::Rect2d computeGameRect(GameRenderTarget gameRenderTarget, uint gameWidth, uint gameHeight,
	                             uint effectiveWidth, uint effectiveHeight);

	/** Checks if the render target supports drawing at arbitrary resolutions */
	bool canUsePreferredResolution(GameRenderTarget gameRenderTarget, bool engineSupportsArbitraryResolutions);

	/** Obtain the user configured fullscreen resolution, or default to the desktop resolution */
	Common::Rect getPreferredFullscreenResolution();
};

#endif
