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

#ifndef BACKENDS_GRAPHICS_SURFACESDL_GRAPHICS_H
#define BACKENDS_GRAPHICS_SURFACESDL_GRAPHICS_H

#include "backends/graphics/graphics.h"
#include "backends/graphics/sdl/sdl-graphics.h"
#include "graphics/pixelformat.h"
#include "graphics/scaler.h"
#include "graphics/scalerplugin.h"
#include "common/events.h"
#include "common/mutex.h"

#include "backends/events/sdl/sdl-events.h"

#include "backends/platform/sdl/sdl-sys.h"

#ifndef RELEASE_BUILD
// Define this to allow for focus rectangle debugging
#define USE_SDL_DEBUG_FOCUSRECT
#endif

enum {
	GFX_SURFACESDL = 0
};


/**
 * SDL graphics manager
 */
class SurfaceSdlGraphicsManager : public SdlGraphicsManager {
public:
	SurfaceSdlGraphicsManager(SdlEventSource *sdlEventSource, SdlWindow *window);
	virtual ~SurfaceSdlGraphicsManager();

	bool hasFeature(OSystem::Feature f) const override;
	void setFeatureState(OSystem::Feature f, bool enable) override;
	bool getFeatureState(OSystem::Feature f) const override;

	const OSystem::GraphicsMode *getSupportedGraphicsModes() const override;
	int getDefaultGraphicsMode() const override;
	bool setGraphicsMode(int mode, uint flags = OSystem::kGfxModeNoFlags) override;
	int getGraphicsMode() const override;
	uint getDefaultScaler() const override;
	uint getDefaultScaleFactor() const override;
	bool setScaler(uint mode, int factor) override;
	uint getScaler() const override;
	uint getScaleFactor() const override;
#ifdef USE_RGB_COLOR
	Graphics::PixelFormat getScreenFormat() const override { return _screenFormat; }
	Common::List<Graphics::PixelFormat> getSupportedFormats() const override;
#endif
#if SDL_VERSION_ATLEAST(2, 0, 0)
	const OSystem::GraphicsMode *getSupportedStretchModes() const override;
	int getDefaultStretchMode() const override;
	bool setStretchMode(int mode) override;
	int getStretchMode() const override;
#endif
	void initSize(uint w, uint h, const Graphics::PixelFormat *format = NULL) override;
	int getScreenChangeID() const override { return _screenChangeCount; }

	void beginGFXTransaction() override;
	OSystem::TransactionError endGFXTransaction() override;

	int16 getHeight() const override;
	int16 getWidth() const override;

protected:
	// PaletteManager API
	void setPalette(const byte *colors, uint start, uint num) override;
	void grabPalette(byte *colors, uint start, uint num) const override;
	virtual void initGraphicsSurface();

	/**
	 * Convert from the SDL pixel format to Graphics::PixelFormat
	 * @param in    The SDL pixel format to convert
	 * @param out   A pixel format to be written to
	 */
	Graphics::PixelFormat convertSDLPixelFormat(SDL_PixelFormat *in) const;
public:
	void copyRectToScreen(const void *buf, int pitch, int x, int y, int w, int h) override;
	Graphics::Surface *lockScreen() override;
	void unlockScreen() override;
	void fillScreen(uint32 col) override;
	void fillScreen(const Common::Rect &r, uint32 col) override;
	void updateScreen() override;
	void setFocusRectangle(const Common::Rect& rect) override;
	void clearFocusRectangle() override;

	Graphics::PixelFormat getOverlayFormat() const override { return _overlayFormat; }
	void clearOverlay() override;
	void grabOverlay(Graphics::Surface &surface) const override;
	void copyRectToOverlay(const void *buf, int pitch, int x, int y, int w, int h) override;
	int16 getOverlayHeight() const override { return _videoMode.overlayHeight; }
	int16 getOverlayWidth() const override { return _videoMode.overlayWidth; }

	void setMouseCursor(const void *buf, uint w, uint h, int hotspotX, int hotspotY, uint32 keycolor, bool dontScale = false, const Graphics::PixelFormat *format = NULL, const byte *mask = NULL) override;
	void setMouseCursor(const void *buf, uint w, uint h, int hotspotX, int hotspotY, uint32 keycolor, bool dontScale, const Graphics::PixelFormat *format, const byte *mask, bool disableKeyColor);
	void setCursorPalette(const byte *colors, uint start, uint num) override;

#ifdef USE_OSD
	void displayMessageOnOSD(const Common::U32String &msg) override;
	void displayActivityIconOnOSD(const Graphics::Surface *icon) override;
#endif

	// Override from Common::EventObserver
	bool notifyEvent(const Common::Event &event) override;

	// SdlGraphicsManager interface
	void notifyVideoExpose() override;
	void notifyResize(const int width, const int height) override;

#if defined(USE_IMGUI) && defined(USE_IMGUI_SDLRENDERER2)
	void *getImGuiTexture(const Graphics::Surface &image, const byte *palette, int palCount) override;
	void freeImGuiTexture(void *texture) override;
#endif

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

	class AspectRatio {
		int _kw, _kh;
	public:
		AspectRatio() { _kw = _kh = 0; }
		AspectRatio(int w, int h);

		bool isAuto() const { return (_kw | _kh) == 0; }

		int kw() const { return _kw; }
		int kh() const { return _kh; }
	};

	static AspectRatio getDesiredAspectRatio();

	bool gameNeedsAspectRatioCorrection() const override {
		return _videoMode.aspectRatioCorrection;
	}
	int getGameRenderScale() const override {
		return _videoMode.scaleFactor;
	}

	void handleResizeImpl(const int width, const int height) override;

	virtual void setupHardwareSize();

	void fixupResolutionForAspectRatio(AspectRatio desiredAspectRatio, int &width, int &height) const;

#if SDL_VERSION_ATLEAST(2, 0, 0)
	/* SDL2 features a different API for 2D graphics. We create a wrapper
	 * around this API to keep the code paths as close as possible. */
	SDL_Renderer *_renderer;
	SDL_Texture *_screenTexture;
	void deinitializeRenderer();
	void recreateScreenTexture();

	virtual SDL_Surface *SDL_SetVideoMode(int width, int height, int bpp, Uint32 flags);
	virtual void SDL_UpdateRects(SDL_Surface *screen, int numrects, SDL_Rect *rects);
	int SDL_SetColors(SDL_Surface *surface, SDL_Color *colors, int firstcolor, int ncolors);
	int SDL_SetAlpha(SDL_Surface *surface, Uint32 flag, Uint8 alpha);
	int SDL_SetColorKey(SDL_Surface *surface, Uint32 flag, Uint32 key);
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
	bool _useOldSrc;
	Graphics::PixelFormat _overlayFormat;
	bool _isDoubleBuf, _isHwPalette;

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
		bool needDisplayResize;
#endif
#ifdef USE_RGB_COLOR
		bool formatChanged;
#endif

		TransactionDetails() {
			sizeChanged = false;
			needHotswap = false;
			needUpdatescreen = false;

#if SDL_VERSION_ATLEAST(2, 0, 0)
			needTextureUpdate = false;
			needDisplayResize = false;
#endif
#ifdef USE_RGB_COLOR
			formatChanged = false;
#endif
		}
	};
	TransactionDetails _transactionDetails;

	struct VideoState {
		bool setup;

		bool fullscreen;
		bool aspectRatioCorrection;
		AspectRatio desiredAspectRatio;
		bool filtering;

		int mode;
#if SDL_VERSION_ATLEAST(2, 0, 0)
		int stretchMode;
#endif
		bool vsync;

		uint scalerIndex;
		int scaleFactor;

		int screenWidth, screenHeight;
		int overlayWidth, overlayHeight;
		int hardwareWidth, hardwareHeight;
#ifdef USE_RGB_COLOR
		Graphics::PixelFormat format;
#endif

		VideoState() {
			setup = false;
			fullscreen = false;
			aspectRatioCorrection = false;
			// desiredAspectRatio set to (0, 0) by AspectRatio constructor
			filtering = false;

			mode = GFX_SURFACESDL;
#if SDL_VERSION_ATLEAST(2, 0, 0)
			stretchMode = 0;
#endif
			vsync = false;

			scalerIndex = 0;
			scaleFactor = 0;

			screenWidth = 0;
			screenHeight = 0;
			overlayWidth = 0;
			overlayHeight = 0;
			hardwareWidth = 0;
			hardwareHeight = 0;
#ifdef USE_RGB_COLOR
			// format set to 0 values by Graphics::PixelFormat constructor
#endif
		}
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

	int _transactionMode;

	// Indicates whether it is needed to free _hwSurface in destructor
	bool _displayDisabled;

	const PluginList &_scalerPlugins;
	ScalerPluginObject *_scalerPlugin;
	Scaler *_scaler, *_mouseScaler;
	uint _maxExtraPixels;
	uint _extraPixels;

	bool _screenIsLocked;
	Graphics::Surface _framebuffer;

	int _screenChangeCount;

	enum {
		NUM_DIRTY_RECT = 100,
		MAX_SCALING = 3
	};

	// Dirty rect management
	// When double-buffering we need to redraw both updates from
	// current frame and previous frame. For convenience we copy
	// them here before traversing the list.
	SDL_Rect _dirtyRectList[2 * NUM_DIRTY_RECT];
	int _numDirtyRects;

	SDL_Rect _prevDirtyRectList[NUM_DIRTY_RECT];
	int _numPrevDirtyRects;

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

	SDL_Rect _mouseLastRect, _mouseNextRect;
	MousePos _mouseCurState;
#ifdef USE_RGB_COLOR
	uint32 _mouseKeyColor;
#else
	byte _mouseKeyColor;
#endif
	bool _disableMouseKeyColor;
	byte _mappedMouseKeyColor;
	bool _cursorDontScale;
	bool _cursorPaletteDisabled;
	SDL_Surface *_mouseOrigSurface;
	SDL_Surface *_mouseSurface;

	// Shake mode
	// This is always set to 0 when building with SDL2.
	int _currentShakeXOffset;
	int _currentShakeYOffset;

	// Palette data
	SDL_Color *_currentPalette;
	uint _paletteDirtyStart, _paletteDirtyEnd;

	SDL_Color *_overlayPalette;
	bool _isInOverlayPalette;

	// Cursor palette data
	SDL_Color *_cursorPalette;

	/**
	 * Mutex which prevents multiple threads from interfering with each other
	 * when accessing the screen.
	 */
	Common::Mutex _graphicsMutex;

#ifdef USE_SDL_DEBUG_FOCUSRECT
	bool _enableFocusRectDebugCode;
	bool _enableFocusRect;
	Common::Rect _focusRect;
#endif

	virtual void addDirtyRect(int x, int y, int w, int h, bool inOverlay, bool realCoordinates = false);

	virtual void drawMouse();
	virtual void undrawMouse();
	virtual void blitCursor();

	virtual void internUpdateScreen();
	virtual void updateScreen(SDL_Rect *dirtyRectList, int actualDirtyRects);

	virtual bool loadGFXMode();
	virtual void unloadGFXMode();
	virtual bool hotswapGFXMode();

	virtual void setAspectRatioCorrection(bool enable);
	void setFilteringMode(bool enable);
	void setVSync(bool enable);

	bool saveScreenshot(const Common::Path &filename) const override;
	virtual void setGraphicsModeIntern();
	virtual void getDefaultResolution(uint &w, uint &h);

private:
	void setFullscreenMode(bool enable);
	void handleScalerHotkeys(uint mode, int factor);

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

	/**
	 * Special case for scalers that use the useOldSrc feature (currently
	 * only the Edge scalers). The variable is checked after closing the
	 * overlay, so that the creation of a new output buffer for the scaler
	 * can be triggered.
	 */
	bool _needRestoreAfterOverlay;
	bool _prevForceRedraw;
	bool _prevCursorNeedsRedraw;
};

#endif
