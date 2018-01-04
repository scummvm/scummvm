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

#ifndef BACKENDS_GRAPHICS_SURFACESDL_GRAPHICS_H
#define BACKENDS_GRAPHICS_SURFACESDL_GRAPHICS_H

#include "backends/graphics/graphics.h"
#include "backends/graphics/sdl/sdl-graphics.h"
#include "graphics/pixelformat.h"
#include "graphics/scaler.h"
#include "common/events.h"
#include "common/system.h"

#include "backends/events/sdl/sdl-events.h"

#include "backends/platform/sdl/sdl-sys.h"

#ifndef RELEASE_BUILD
// Define this to allow for focus rectangle debugging
#define USE_SDL_DEBUG_FOCUSRECT
#endif

#if !defined(_WIN32_WCE) && !defined(__SYMBIAN32__)
#define USE_OSD	1
#endif

enum {
	GFX_NORMAL = 0,
	GFX_DOUBLESIZE = 1,
	GFX_TRIPLESIZE = 2,
	GFX_2XSAI = 3,
	GFX_SUPER2XSAI = 4,
	GFX_SUPEREAGLE = 5,
	GFX_ADVMAME2X = 6,
	GFX_ADVMAME3X = 7,
	GFX_HQ2X = 8,
	GFX_HQ3X = 9,
	GFX_TV2X = 10,
	GFX_DOTMATRIX = 11
};


class AspectRatio {
	int _kw, _kh;
public:
	AspectRatio() { _kw = _kh = 0; }
	AspectRatio(int w, int h);

	bool isAuto() const { return (_kw | _kh) == 0; }

	int kw() const { return _kw; }
	int kh() const { return _kh; }
};

/**
 * SDL graphics manager
 */
class SurfaceSdlGraphicsManager : public SdlGraphicsManager, public Common::EventObserver {
public:
	SurfaceSdlGraphicsManager(SdlEventSource *sdlEventSource, SdlWindow *window);
	virtual ~SurfaceSdlGraphicsManager();

	virtual void activateManager() override;
	virtual void deactivateManager() override;

	virtual bool hasFeature(OSystem::Feature f) const override;
	virtual void setFeatureState(OSystem::Feature f, bool enable) override;
	virtual bool getFeatureState(OSystem::Feature f) const override;

	virtual const OSystem::GraphicsMode *getSupportedGraphicsModes() const override;
	virtual int getDefaultGraphicsMode() const override;
	virtual bool setGraphicsMode(int mode) override;
	virtual int getGraphicsMode() const override;
	virtual void resetGraphicsScale() override;
#ifdef USE_RGB_COLOR
	virtual Graphics::PixelFormat getScreenFormat() const override { return _screenFormat; }
	virtual Common::List<Graphics::PixelFormat> getSupportedFormats() const override;
#endif
	virtual const OSystem::GraphicsMode *getSupportedShaders() const override;
	virtual int getShader() const override;
	virtual bool setShader(int id) override;
	virtual void initSize(uint w, uint h, const Graphics::PixelFormat *format = NULL) override;
	virtual int getScreenChangeID() const override { return _screenChangeCount; }

	virtual void beginGFXTransaction() override;
	virtual OSystem::TransactionError endGFXTransaction() override;

	virtual int16 getHeight() const override;
	virtual int16 getWidth() const override;

protected:
	// PaletteManager API
	virtual void setPalette(const byte *colors, uint start, uint num) override;
	virtual void grabPalette(byte *colors, uint start, uint num) const override;

public:
	virtual void copyRectToScreen(const void *buf, int pitch, int x, int y, int w, int h) override;
	virtual Graphics::Surface *lockScreen() override;
	virtual void unlockScreen() override;
	virtual void fillScreen(uint32 col) override;
	virtual void updateScreen() override;
	virtual void setShakePos(int shakeOffset) override;
	virtual void setFocusRectangle(const Common::Rect& rect) override;
	virtual void clearFocusRectangle() override;

	virtual Graphics::PixelFormat getOverlayFormat() const override { return _overlayFormat; }
	virtual void clearOverlay() override;
	virtual void grabOverlay(void *buf, int pitch) const override;
	virtual void copyRectToOverlay(const void *buf, int pitch, int x, int y, int w, int h) override;
	virtual int16 getOverlayHeight() const override { return _videoMode.overlayHeight; }
	virtual int16 getOverlayWidth() const override { return _videoMode.overlayWidth; }

	virtual void setMouseCursor(const void *buf, uint w, uint h, int hotspotX, int hotspotY, uint32 keycolor, bool dontScale = false, const Graphics::PixelFormat *format = NULL) override;
	virtual void setCursorPalette(const byte *colors, uint start, uint num) override;

#ifdef USE_OSD
	virtual void displayMessageOnOSD(const char *msg) override;
	virtual void displayActivityIconOnOSD(const Graphics::Surface *icon) override;
#endif

	// Override from Common::EventObserver
	virtual bool notifyEvent(const Common::Event &event) override;

	// SdlGraphicsManager interface
	virtual void notifyVideoExpose() override;
	virtual void notifyResize(const int width, const int height) override;

protected:
#ifdef USE_OSD
	/** Surface containing the OSD message */
	SDL_Surface *_osdMessageSurface;
	/** Transparency level of the OSD message */
	uint8 _osdMessageAlpha;
	/** When to start the fade out */
	uint32 _osdMessageFadeStartTime;
	/** Enum with OSD options */
	enum {
		kOSDFadeOutDelay = 2 * 1000,	/** < Delay before the OSD is faded out (in milliseconds) */
		kOSDFadeOutDuration = 500,		/** < Duration of the OSD fade out (in milliseconds) */
		kOSDInitialAlpha = 80			/** < Initial alpha level, in percent */
	};
	/** Screen rectangle where the OSD message is drawn */
	SDL_Rect getOSDMessageRect() const;
	/** Clear the currently displayed OSD message if any */
	void removeOSDMessage();
	/** Surface containing the OSD background activity icon */
	SDL_Surface *_osdIconSurface;
	/** Screen rectangle where the OSD background activity icon is drawn */
	SDL_Rect getOSDIconRect() const;

	void updateOSD();
	void drawOSD();
#endif

	virtual bool gameNeedsAspectRatioCorrection() const override {
		return _videoMode.aspectRatioCorrection;
	}

	virtual void handleResizeImpl(const int width, const int height) override;

	virtual int getGraphicsModeScale(int mode) const override;

#if SDL_VERSION_ATLEAST(2, 0, 0)
	/* SDL2 features a different API for 2D graphics. We create a wrapper
	 * around this API to keep the code paths as close as possible. */
	SDL_Renderer *_renderer;
	SDL_Texture *_screenTexture;
	void deinitializeRenderer();
	void recreateScreenTexture();

	virtual SDL_Surface *SDL_SetVideoMode(int width, int height, int bpp, Uint32 flags);
	void SDL_UpdateRects(SDL_Surface *screen, int numrects, SDL_Rect *rects);
#endif

	/** Unseen game screen */
	SDL_Surface *_screen;
	Graphics::PixelFormat _screenFormat;
	Graphics::PixelFormat _cursorFormat;
#ifdef USE_RGB_COLOR
	Common::List<Graphics::PixelFormat> _supportedFormats;

	/**
	 * Update the list of supported pixel formats.
	 * This method is invoked by loadGFXMode().
	 */
	void detectSupportedFormats();
#endif

	/** Temporary screen (for scalers) */
	SDL_Surface *_tmpscreen;
	/** Temporary screen (for scalers) */
	SDL_Surface *_tmpscreen2;

	SDL_Surface *_overlayscreen;
	Graphics::PixelFormat _overlayFormat;

	enum {
		kTransactionNone = 0,
		kTransactionActive = 1,
		kTransactionRollback = 2
	};

	struct TransactionDetails {
		bool sizeChanged;
		bool needHotswap;
		bool needUpdatescreen;
#if SDL_VERSION_ATLEAST(2, 0, 0)
		bool needTextureUpdate;
#endif
#ifdef USE_RGB_COLOR
		bool formatChanged;
#endif
	};
	TransactionDetails _transactionDetails;

	struct VideoState {
		bool setup;

		bool fullscreen;
		bool aspectRatioCorrection;
		AspectRatio desiredAspectRatio;

#if SDL_VERSION_ATLEAST(2, 0, 0)
		bool filtering;
#endif

		int mode;
		int scaleFactor;

		int screenWidth, screenHeight;
		int overlayWidth, overlayHeight;
		int hardwareWidth, hardwareHeight;
#ifdef USE_RGB_COLOR
		Graphics::PixelFormat format;
#endif
	};
	VideoState _videoMode, _oldVideoMode;

#if defined(WIN32) && !SDL_VERSION_ATLEAST(2, 0, 0)
	/**
	 * Original BPP to restore the video mode on unload.
	 *
	 * This is required to make listing video modes for the OpenGL output work
	 * on Windows 8+. On these systems OpenGL modes are only available for
	 * 32bit formats. However, we setup a 16bit format and thus mode listings
	 * for OpenGL will return an empty list afterwards.
	 *
	 * In theory we might require this behavior on non-Win32 platforms too.
	 * However, SDL sometimes gives us invalid pixel formats for X11 outputs
	 * causing crashes when trying to setup the original pixel format.
	 * See bug #7038 "IRIX: X BadMatch when trying to start any 640x480 game".
	 */
	uint8 _originalBitsPerPixel;
#endif

	ScalerProc *_scalerProc;
	int _scalerType;
	int _transactionMode;

	// Indicates whether it is needed to free _hwSurface in destructor
	bool _displayDisabled;

	bool _screenIsLocked;
	Graphics::Surface _framebuffer;

	int _screenChangeCount;

	int _currentShader;
	int _numShaders;

	enum {
		NUM_DIRTY_RECT = 100,
		MAX_SCALING = 3
	};

	// Dirty rect management
	SDL_Rect _dirtyRectList[NUM_DIRTY_RECT];
	int _numDirtyRects;

	struct MousePos {
		// The size and hotspot of the original cursor image.
		int16 w, h;
		int16 hotX, hotY;

		// The size and hotspot of the pre-scaled cursor image, in real
		// coordinates.
		int16 rW, rH;
		int16 rHotX, rHotY;

		// The size and hotspot of the pre-scaled cursor image, in game
		// coordinates.
		int16 vW, vH;
		int16 vHotX, vHotY;

		MousePos() : w(0), h(0), hotX(0), hotY(0),
					rW(0), rH(0), rHotX(0), rHotY(0), vW(0), vH(0),
					vHotX(0), vHotY(0)
			{ }
	};

	byte *_mouseData;
	SDL_Rect _mouseBackup;
	MousePos _mouseCurState;
#ifdef USE_RGB_COLOR
	uint32 _mouseKeyColor;
#else
	byte _mouseKeyColor;
#endif
	bool _cursorDontScale;
	bool _cursorPaletteDisabled;
	SDL_Surface *_mouseOrigSurface;
	SDL_Surface *_mouseSurface;
	enum {
		kMouseColorKey = 1
	};

	// Shake mode
	int _currentShakePos;
	int _newShakePos;

	// Palette data
	SDL_Color *_currentPalette;
	uint _paletteDirtyStart, _paletteDirtyEnd;

	// Cursor palette data
	SDL_Color *_cursorPalette;

	/**
	 * Mutex which prevents multiple threads from interfering with each other
	 * when accessing the screen.
	 */
	OSystem::MutexRef _graphicsMutex;

#ifdef USE_SDL_DEBUG_FOCUSRECT
	bool _enableFocusRectDebugCode;
	bool _enableFocusRect;
	Common::Rect _focusRect;
#endif

	virtual void addDirtyRect(int x, int y, int w, int h, bool realCoordinates = false);

	virtual void drawMouse();
	virtual void undrawMouse();
	virtual void blitCursor();

	virtual void internUpdateScreen();
	virtual void updateShader();

	virtual bool loadGFXMode();
	virtual void unloadGFXMode();
	virtual bool hotswapGFXMode();

	virtual void setAspectRatioCorrection(bool enable);
#if SDL_VERSION_ATLEAST(2, 0, 0)
	void setFilteringMode(bool enable);
#endif

	virtual bool saveScreenshot(const char *filename);
	virtual void setGraphicsModeIntern();

private:
	void setFullscreenMode(bool enable);
	bool handleScalerHotkeys(Common::KeyCode key);
	bool isScalerHotkey(const Common::Event &event);
	void toggleFullScreen();

	/**
	 * Converts the given point from the overlay's coordinate space to the
	 * game's coordinate space.
	 */
	Common::Point convertOverlayToGame(const int x, const int y) const {
		if (getOverlayWidth() == 0 || getOverlayHeight() == 0) {
			error("convertOverlayToGame called without a valid overlay");
		}

		return Common::Point(x * getWidth() / getOverlayWidth(),
							 y * getHeight() / getOverlayHeight());
	}

	/**
	 * Converts the given point from the game's coordinate space to the
	 * overlay's coordinate space.
	 */
	Common::Point convertGameToOverlay(const int x, const int y) const {
		if (getWidth() == 0 || getHeight() == 0) {
			error("convertGameToOverlay called without a valid overlay");
		}

		return Common::Point(x * getOverlayWidth() / getWidth(),
							 y * getOverlayHeight() / getHeight());
	}
};

#endif
