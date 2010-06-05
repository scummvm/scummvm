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

#ifndef BACKENDS_GRAPHICS_SDL_H
#define BACKENDS_GRAPHICS_SDL_H

#include "backends/graphics/default/default-graphics.h"
#include "common/system.h"
#include "graphics/scaler.h"

#if defined(__SYMBIAN32__)
#include <esdl\SDL.h>
#else
#include <SDL.h>
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

class SdlGraphicsManager : public DefaultGraphicsManager {
public:
	SdlGraphicsManager();
	~SdlGraphicsManager();

	bool hasGraphicsFeature(OSystem::Feature f);
	void setGraphicsFeatureState(OSystem::Feature f, bool enable);
	bool getGraphicsFeatureState(OSystem::Feature f);

	const OSystem::GraphicsMode *getSupportedGraphicsModes() const;
	int getDefaultGraphicsMode() const;
	bool setGraphicsMode(int mode);
	bool setGraphicsMode(const char *name);
	int getGraphicsMode() const;

#ifdef USE_RGB_COLOR
	// Game screen
	virtual Graphics::PixelFormat getScreenFormat() const { return _screenFormat; }

	// Highest supported
	virtual Common::List<Graphics::PixelFormat> getSupportedFormats();
#endif
	void beginGFXTransaction();
	OSystem::TransactionError endGFXTransaction();
	virtual void initSize(uint w, uint h, const Graphics::PixelFormat *format = NULL);
	virtual int16 getHeight();
	virtual int16 getWidth();
	void setPalette(const byte *colors, uint start, uint num);
	void grabPalette(byte *colors, uint start, uint num);
	virtual void copyRectToScreen(const byte *buf, int pitch, int x, int y, int w, int h);
	virtual Graphics::Surface *lockScreen();
	virtual void unlockScreen();
	void fillScreen(uint32 col);
	void updateScreen();
	void setShakePos(int shakeOffset);
	virtual void showOverlay();
	virtual void hideOverlay();
	virtual Graphics::PixelFormat getOverlayFormat() const { return _overlayFormat; }
	void clearOverlay();
	void grabOverlay(OverlayColor *buf, int pitch);
	void copyRectToOverlay(const OverlayColor *buf, int pitch, int x, int y, int w, int h);
	virtual int16 getOverlayHeight()  { return _videoMode.overlayHeight; }
	virtual int16 getOverlayWidth()   { return _videoMode.overlayWidth; }
	bool showMouse(bool visible);
	virtual void warpMouse(int x, int y);
	virtual void setMouseCursor(const byte *buf, uint w, uint h, int hotspotX, int hotspotY, uint32 keycolor, int cursorTargetScale = 1, const Graphics::PixelFormat *format = NULL);
	void setCursorPalette(const byte *colors, uint start, uint num);
	void disableCursorPalette(bool disable) {
		_cursorPaletteDisabled = disable;
		blitCursor();
	}
	virtual int getScreenChangeID() const { return _screenChangeCount; }
	
protected:
#ifdef USE_OSD
	SDL_Surface *_osdSurface;
	Uint8 _osdAlpha;			// Transparency level of the OSD
	uint32 _osdFadeStartTime;	// When to start the fade out
	enum {
		kOSDFadeOutDelay = 2 * 1000,	// Delay before the OSD is faded out (in milliseconds)
		kOSDFadeOutDuration = 500,		// Duration of the OSD fade out (in milliseconds)
		kOSDColorKey = 1,
		kOSDInitialAlpha = 80			// Initial alpha level, in percent
	};
#endif

	// hardware screen
	SDL_Surface *_hwscreen;

	// unseen game screen
	SDL_Surface *_screen;
#ifdef USE_RGB_COLOR
	Graphics::PixelFormat _screenFormat;
	Graphics::PixelFormat _cursorFormat;
#endif

	// temporary screen (for scalers)
	SDL_Surface *_tmpscreen;
	SDL_Surface *_tmpscreen2;

	// overlay
	SDL_Surface *_overlayscreen;
	bool _overlayVisible;
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
		bool normal1xScaler;
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

	virtual void setGraphicsModeIntern(); // overloaded by CE backend

	/** Force full redraw on next updateScreen */
	bool _forceFull;
	ScalerProc *_scalerProc;
	int _scalerType;
	int _transactionMode;

	bool _screenIsLocked;
	Graphics::Surface _framebuffer;

	/** Current video mode flags (see DF_* constants) */
	bool _modeChanged;
	int _screenChangeCount;

	enum {
		NUM_DIRTY_RECT = 100,
		MAX_SCALING = 3
	};

	// Dirty rect management
	SDL_Rect _dirtyRectList[NUM_DIRTY_RECT];
	int _numDirtyRects;

	void setMousePos(int x, int y);

	struct MousePos {
		// The mouse position, using either virtual (game) or real
		// (overlay) coordinates.
		int16 x, y;

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

		MousePos() : x(0), y(0), w(0), h(0), hotX(0), hotY(0),
		             rW(0), rH(0), rHotX(0), rHotY(0), vW(0), vH(0),
		             vHotX(0), vHotY(0)
			{ }
	};

	bool _mouseVisible;
	bool _mouseNeedsRedraw;
	byte *_mouseData;
	SDL_Rect _mouseBackup;
	MousePos _mouseCurState; // Move to events?
#ifdef USE_RGB_COLOR
	uint32 _mouseKeyColor;
#else
	byte _mouseKeyColor;
#endif
	int _cursorTargetScale;
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

	virtual void addDirtyRect(int x, int y, int w, int h, bool realCoordinates = false); // overloaded by CE backend

	virtual void drawMouse(); // overloaded by CE backend
	virtual void undrawMouse(); // overloaded by CE backend (FIXME)
	virtual void blitCursor(); // overloaded by CE backend (FIXME)

	virtual void internUpdateScreen(); // overloaded by CE backend

	virtual bool loadGFXMode(); // overloaded by CE backend
	virtual void unloadGFXMode(); // overloaded by CE backend
	virtual bool hotswapGFXMode(); // overloaded by CE backend

	void setFullscreenMode(bool enable);
	void setAspectRatioCorrection(bool enable);

	virtual bool saveScreenshot(const char *filename); // overloaded by CE backend

	int effectiveScreenHeight() const;

	void setupIcon();

	bool handleScalerHotkeys(const SDL_KeyboardEvent &key); // Move to events?
	bool isScalerHotkey(const Common::Event &event); // Move to events?
};


#endif
