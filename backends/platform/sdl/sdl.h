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

#ifndef SDL_COMMON_H
#define SDL_COMMON_H

#if defined(__SYMBIAN32__)
#include <esdl\SDL.h>
#else
#include <SDL.h>
#endif

#include "backends/base-backend.h"
#include "graphics/scaler.h"


namespace Audio {
	class MixerImpl;
}

#if !defined(_WIN32_WCE) && !defined(__SYMBIAN32__)
// Uncomment this to enable the 'on screen display' code.
#define USE_OSD	1
#endif

#if defined(MACOSX)
// On Mac OS X, we need to double buffer the audio buffer, else anything
// which produces sampled data with high latency (like the MT-32 emulator)
// will sound terribly.
// This could be enabled for more / most ports in the future, but needs some
// testing.
#define MIXER_DOUBLE_BUFFERING 1
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


class OSystem_SDL : public BaseBackend {
public:
	OSystem_SDL();
	virtual ~OSystem_SDL();

	virtual void initBackend();

	void beginGFXTransaction(void);
	TransactionError endGFXTransaction(void);

	// Set the size of the video bitmap.
	// Typically, 320x200
	virtual void initSize(uint w, uint h); // overloaded by CE backend

	virtual int getScreenChangeID() const { return _screenChangeCount; }

	// Set colors of the palette
	void setPalette(const byte *colors, uint start, uint num);

	// Get colors of the palette
	void grabPalette(byte *colors, uint start, uint num);

	// Draw a bitmap to screen.
	// The screen will not be updated to reflect the new bitmap
	virtual void copyRectToScreen(const byte *src, int pitch, int x, int y, int w, int h); // overloaded by CE backend (FIXME)

	virtual Graphics::Surface *lockScreen();
	virtual void unlockScreen();

	// Update the dirty areas of the screen
	void updateScreen();

	// Either show or hide the mouse cursor
	bool showMouse(bool visible);

	// Warp the mouse cursor. Where set_mouse_pos() only informs the
	// backend of the mouse cursor's current position, this function
	// actually moves the cursor to the specified position.
	virtual void warpMouse(int x, int y); // overloaded by CE backend (FIXME)

	// Set the bitmap that's used when drawing the cursor.
#ifdef ENABLE_16BIT
	virtual void setMouseCursor(const byte *buf, uint w, uint h, int hotspot_x, int hotspot_y, uint32 keycolor, int cursorTargetScale, uint8 bitDepth = 8); // overloaded by CE backend (FIXME)
#else
	virtual void setMouseCursor(const byte *buf, uint w, uint h, int hotspot_x, int hotspot_y, byte keycolor, int cursorTargetScale); // overloaded by CE backend (FIXME)
#endif

	// Set colors of cursor palette
	void setCursorPalette(const byte *colors, uint start, uint num);

	// Disables or enables cursor palette
	void disableCursorPalette(bool disable) {
		_cursorPaletteDisabled = disable;
		blitCursor();
	}

	// Shaking is used in SCUMM. Set current shake position.
	void setShakePos(int shake_pos);

	// Get the number of milliseconds since the program was started.
	uint32 getMillis();

	// Delay for a specified amount of milliseconds
	void delayMillis(uint msecs);

	// Get the next event.
	// Returns true if an event was retrieved.
	virtual bool pollEvent(Common::Event &event); // overloaded by CE backend

	// Define all hardware keys for keymapper
	virtual Common::HardwareKeySet *getHardwareKeySet();

	// Set function that generates samples
	virtual void setupMixer();
	static void mixCallback(void *s, byte *samples, int len);

	virtual void closeMixer();

	virtual Audio::Mixer *getMixer();

	// Poll CD status
	// Returns true if cd audio is playing
	bool pollCD();

	// Play CD audio track
	void playCD(int track, int num_loops, int start_frame, int duration);

	// Stop CD audio track
	void stopCD();

	// Update CD audio status
	void updateCD();

	// Quit
	virtual void quit(); // overloaded by CE backend

	virtual void getTimeAndDate(struct tm &t) const;
	virtual Common::TimerManager *getTimerManager();

	// Mutex handling
	MutexRef createMutex();
	void lockMutex(MutexRef mutex);
	void unlockMutex(MutexRef mutex);
	void deleteMutex(MutexRef mutex);

	// Overlay
	virtual Graphics::PixelFormat getOverlayFormat() const { return _overlayFormat; }

#ifdef ENABLE_16BIT
	// Game screen
	virtual Graphics::PixelFormat getScreenFormat() const { return _screenFormat; }
#endif

	virtual void showOverlay();
	virtual void hideOverlay();
	virtual void clearOverlay();
	virtual void grabOverlay(OverlayColor *buf, int pitch);
	virtual void copyRectToOverlay(const OverlayColor *buf, int pitch, int x, int y, int w, int h);
	virtual int16 getHeight();
	virtual int16 getWidth();
	virtual int16 getOverlayHeight()  { return _videoMode.overlayHeight; }
	virtual int16 getOverlayWidth()   { return _videoMode.overlayWidth; }

	virtual const GraphicsMode *getSupportedGraphicsModes() const;
	virtual int getDefaultGraphicsMode() const;
	virtual bool setGraphicsMode(int mode);
	virtual int getGraphicsMode() const;

	virtual void setWindowCaption(const char *caption);
	virtual bool openCD(int drive);

	virtual bool hasFeature(Feature f);
	virtual void setFeatureState(Feature f, bool enable);
	virtual bool getFeatureState(Feature f);

#ifdef USE_OSD
	void displayMessageOnOSD(const char *msg);
#endif

	virtual Common::SaveFileManager *getSavefileManager();
	virtual FilesystemFactory *getFilesystemFactory();
	virtual void addSysArchivesToSearchSet(Common::SearchSet &s, int priority = 0);

	virtual Common::SeekableReadStream *createConfigReadStream();
	virtual Common::WriteStream *createConfigWriteStream();

protected:
	bool _inited;

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
#ifdef ENABLE_16BIT
	Graphics::PixelFormat _screenFormat;

	//HACK This is a temporary hack to get 16-bit graphics
	//displaying quickly, which will be removed in favor of 
	//configuring the format of _screen on a per-game basis
	SDL_Surface *_screen16;	
#endif

	// temporary screen (for scalers)
	SDL_Surface *_tmpscreen;
	SDL_Surface *_tmpscreen2;

	// overlay
	SDL_Surface *_overlayscreen;
	bool _overlayVisible;
	Graphics::PixelFormat _overlayFormat;

	// Audio
	int _samplesPerSec;

	// CD Audio
	SDL_CD *_cdrom;
	int _cdTrack, _cdNumLoops, _cdStartFrame, _cdDuration;
	uint32 _cdEndTime, _cdStopTime;

	enum {
		DF_WANT_RECT_OPTIM			= 1 << 0
	};

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
	};
	TransactionDetails _transactionDetails;

	struct VideoState {
		bool setup;

		bool fullscreen;
		bool aspectRatioCorrection;

		int mode;
		int scaleFactor;

		int screenWidth, screenHeight;
		int overlayWidth, overlayHeight;
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
	uint32 _modeFlags;
	bool _modeChanged;
	int _screenChangeCount;

	enum {
		NUM_DIRTY_RECT = 100,
		MAX_SCALING = 3
	};

	// Dirty rect management
	SDL_Rect _dirtyRectList[NUM_DIRTY_RECT];
	int _numDirtyRects;
	uint32 *_dirtyChecksums;
	bool _cksumValid;
	int _cksumNum;

	// Keyboard mouse emulation.  Disabled by fingolfin 2004-12-18.
	// I am keeping the rest of the code in for now, since the joystick
	// code (or rather, "hack") uses it, too.
	struct KbdMouse {
		int16 x, y, x_vel, y_vel, x_max, y_max, x_down_count, y_down_count;
		uint32 last_time, delay_time, x_down_time, y_down_time;
	};

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

	// mouse
	KbdMouse _km;
	bool _mouseVisible;
	bool _mouseNeedsRedraw;
	byte *_mouseData;
	SDL_Rect _mouseBackup;
	MousePos _mouseCurState;
	byte _mouseKeyColor;
	int _cursorTargetScale;
#ifdef ENABLE_16BIT
	uint8 _cursorBitDepth;
#endif
	bool _cursorPaletteDisabled;
	SDL_Surface *_mouseOrigSurface;
	SDL_Surface *_mouseSurface;
	enum {
		kMouseColorKey = 1
	};

	// joystick
	SDL_Joystick *_joystick;

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
	MutexRef _graphicsMutex;

#ifdef MIXER_DOUBLE_BUFFERING
	SDL_mutex *_soundMutex;
	SDL_cond *_soundCond;
	SDL_Thread *_soundThread;
	bool _soundThreadIsRunning;
	bool _soundThreadShouldQuit;

	byte _activeSoundBuf;
	uint _soundBufSize;
	byte *_soundBuffers[2];

	void mixerProducerThread();
	static int SDLCALL mixerProducerThreadEntry(void *arg);
	void initThreadedMixer(Audio::MixerImpl *mixer, uint bufSize);
	void deinitThreadedMixer();
#endif

	FilesystemFactory *_fsFactory;
	Common::SaveFileManager *_savefile;
	Audio::MixerImpl *_mixer;

	SDL_TimerID _timerID;
	Common::TimerManager *_timer;

protected:
	void addDirtyRgnAuto(const byte *buf);
	void makeChecksums(const byte *buf);

	virtual void addDirtyRect(int x, int y, int w, int h, bool realCoordinates = false); // overloaded by CE backend

	virtual void drawMouse(); // overloaded by CE backend
	virtual void undrawMouse(); // overloaded by CE backend (FIXME)
	virtual void blitCursor(); // overloaded by CE backend (FIXME)

	/** Set the position of the virtual mouse cursor. */
	void setMousePos(int x, int y);
	virtual void fillMouseEvent(Common::Event &event, int x, int y); // overloaded by CE backend
	void toggleMouseGrab();

	virtual void internUpdateScreen(); // overloaded by CE backend

	virtual bool loadGFXMode(); // overloaded by CE backend
	virtual void unloadGFXMode(); // overloaded by CE backend
	virtual bool hotswapGFXMode(); // overloaded by CE backend

	void setFullscreenMode(bool enable);
	void setAspectRatioCorrection(bool enable);

	virtual bool saveScreenshot(const char *filename); // overloaded by CE backend

	int effectiveScreenHeight() const {
		return (_videoMode.aspectRatioCorrection ? real2Aspect(_videoMode.screenHeight) : _videoMode.screenHeight)
			* _videoMode.scaleFactor;
	}

	void setupIcon();
	void handleKbdMouse();

	virtual bool remapKey(SDL_Event &ev, Common::Event &event);

	void handleScalerHotkeys(const SDL_KeyboardEvent &key);
};

#endif
