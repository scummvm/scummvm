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
 * Base class for a SDL based graphics manager.
 *
 * It features a few extra a few extra features required by SdlEventSource.
 */
class ResVmSdlGraphicsManager : public SdlGraphicsManager, public Common::EventObserver {
public:
	struct Capabilities {
		uint desktopWidth, desktopHeight;
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
	virtual void notifyMousePos(Common::Point mouse) override;

	virtual void setFeatureState(OSystem::Feature f, bool enable);
	virtual bool getFeatureState(OSystem::Feature f);

	virtual const OSystem::GraphicsMode *getSupportedGraphicsModes() const;
	virtual int getDefaultGraphicsMode() const;
	virtual bool setGraphicsMode(int mode);
	virtual int getGraphicsMode() const;
	virtual void resetGraphicsScale();
#ifdef USE_RGB_COLOR
	virtual Graphics::PixelFormat getScreenFormat() const { return _screenFormat; }
	virtual Common::List<Graphics::PixelFormat> getSupportedFormats() const;
#endif
	virtual void initSize(uint w, uint h, const Graphics::PixelFormat *format = NULL);
	virtual int getScreenChangeID() const { return _screenChangeCount; }

	virtual void beginGFXTransaction();
	virtual OSystem::TransactionError endGFXTransaction();

protected:
	// PaletteManager API
	virtual void setPalette(const byte *colors, uint start, uint num);
	virtual void grabPalette(byte *colors, uint start, uint num);

public:
	virtual void copyRectToScreen(const void *buf, int pitch, int x, int y, int w, int h);
	virtual Graphics::Surface *lockScreen();
	virtual void unlockScreen();
	virtual void fillScreen(uint32 col);
	virtual void setShakePos(int shakeOffset);
	virtual void setFocusRectangle(const Common::Rect& rect);
	virtual void clearFocusRectangle();
	virtual void setCursorPalette(const byte *colors, uint start, uint num);

	//ResidualVM specific implementions:
	virtual int16 getOverlayHeight() { return _overlayHeight; }
	virtual int16 getOverlayWidth() { return _overlayWidth; }
	virtual Graphics::PixelFormat getOverlayFormat() const { return _overlayFormat; }

	virtual bool showMouse(bool visible);
	virtual bool lockMouse(bool lock); // ResidualVM specific method
	virtual void setMouseCursor(const void *buf, uint w, uint h, int hotspotX, int hotspotY, uint32 keycolor, bool dontScale = false, const Graphics::PixelFormat *format = NULL);

#ifdef USE_OSD
	virtual void displayMessageOnOSD(const char *msg);
#endif

	// Override from Common::EventObserver
	bool notifyEvent(const Common::Event &event) override;

protected:
	const Capabilities &_capabilities;

	bool _fullscreen;
	bool _lockAspectRatio;

	int _screenChangeCount;

	bool _overlayVisible;
	Graphics::PixelFormat _overlayFormat;
	int _overlayWidth, _overlayHeight;

	virtual void closeOverlay() = 0;

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
