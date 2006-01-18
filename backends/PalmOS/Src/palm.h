/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001-2006 The ScummVM project
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
 * $Header$
 *
 */

#ifndef PALM_H
#define PALM_H

#include <SonyClie.h>
#include "common/system.h"
#include "cdaudio.h"
//#include "arm/native.h"

typedef struct {
	void *proc;
	void *param;

	void	*handle;	// sound handle
	UInt32	size;		// buffer size
	UInt32	slot;
	UInt32 	active,		// is the sound handler active
			set,		// is the buffer filled
			wait;		// do we need to wait for sound completion
	void	*dataP,		// main buffer
			*tmpP;		// tmp buffer (convertion)
} SoundDataType;

// OSD resource id
#define kDrawKeyState	3000
#define kDrawNumPad		3010
#define kDrawBatLow		3020

// OSD key state
enum {
	MD_NONE = 0,
	MD_CMD,
	MD_ALT,
	MD_CTRL
};

// gfx modes
enum {
	GFX_NORMAL = 0,
	GFX_FLIPPING = 100,	// Palmos
	GFX_BUFFERED = 101,	// Palmos
	GFX_WIDE = 102 // palmos
};

class OSystem_PALMOS : public OSystem {
public:
	// Set colors of the palette
	void setPalette(const byte *colors, uint start, uint num);

	const GraphicsMode *getSupportedGraphicsModes() const;
	bool setGraphicsMode(int mode);
	int getGraphicsMode() const;
	int getDefaultGraphicsMode() const;

	int getOutputSampleRate() const;
	bool openCD(int drive);
	void setWindowCaption(const char *caption);	// TODO : _inGame = true = don't set

	bool hasFeature(Feature f);
	void setFeatureState(Feature f, bool enable);
	bool getFeatureState(Feature f);

	// Set the size of the video bitmap.
	// Typically, 320x200
	void initSize(uint w, uint h, int overlayScale);

	// Draw a bitmap to screen.
	// The screen will not be updated to reflect the new bitmap
	void copyRectToScreen(const byte *buf, int pitch, int x, int y, int w, int h);

	// Update the dirty areas of the screen
	void updateScreen();

	void clearScreen();

	// Either show or hide the mouse cursor
	bool showMouse(bool visible);

	// Set the position of the mouse cursor
	void set_mouse_pos(int x, int y);

	// Warp the mouse cursor. Where set_mouse_pos() only informs the
	// backend of the mouse cursor's current position, this function
	// actually moves the cursor to the specified position.
	void warpMouse(int x, int y);


	// Set the bitmap that's used when drawing the cursor.
	void setMouseCursor(const byte *buf, uint w, uint h, int hotspot_x, int hotspot_y, byte keycolor);

	// Shaking is used in SCUMM. Set current shake position.
	void setShakePos(int shake_pos);

	// Get the number of milliseconds since the program was started.
	uint32 getMillis();

	// Delay for a specified amount of milliseconds
	void delayMillis(uint msecs);

	// Get the next event.
	// Returns true if an event was retrieved.
	bool pollEvent(Event &event);

	void SimulateArrowKeys(Event &event, Int8 iHoriz, Int8 iVert);

	/** @name Sound */
	//@{
	/**
	 * Set the audio callback which is invoked whenever samples need to be generated.
	 * Currently, only the 16-bit signed mode is ever used for Simon & Scumm
	 * @param proc		pointer to the callback.
	 * @param param		an arbitrary parameter which is stored and passed to proc.
	 */
	bool setSoundCallback(SoundProc proc, void *param);

	/**
	 * Remove any audio callback previously set via setSoundCallback, thus effectively
	 * stopping all audio output immediately.
	 * @see setSoundCallback
	 */
	void clearSoundCallback();
	//@}

	// Poll cdrom status
	// Returns true if cd audio is playing
	bool pollCD();

	// Play cdrom audio track
	void playCD(int track, int num_loops, int start_frame, int duration);

	// Stop cdrom audio track
	void stopCD();

	// Update cdrom audio status
	void updateCD();

	// Add a callback timer
	void setTimerCallback(TimerProc callback, int timer);

	// Mutex handling
	MutexRef createMutex();
	void lockMutex(MutexRef mutex);
	void unlockMutex(MutexRef mutex);
	void deleteMutex(MutexRef mutex);

	// Quit
	void quit();

	// Overlay
	void showOverlay();
	void hideOverlay();
	void clearOverlay();
	void grabOverlay(byte *buf, int pitch);
	void copyRectToOverlay(const byte *buf, int pitch, int x, int y, int w, int h);

	int16 getWidth();
	int16 getHeight();
	byte RGBToColor(uint8 r, uint8 g, uint8 b);
	void ColorToRGB(byte color, uint8 &r, uint8 &g, uint8 &b);

	// Savefile management
	Common::SaveFileManager *getSavefileManager();

	static OSystem *create();

protected:
	byte *_tmpScreenP, *_tmpBackupP, *_tmpHotSwapP; // TODO : rename _tmpScreenP
	bool _overlayVisible;

private:
	typedef void (OSystem_PALMOS::*RendererProc)();
	RendererProc _renderer_proc;

	void updateScreen_flipping();
	void updateScreen_buffered();
	void updateScreen_direct();
	void updateScreen_widePortrait();
	void updateScreen_wideLandscape();
	void updateScreen_wideZodiac();

	void *ptrP[5];	// various ptr

	WinHandle _screenH, _offScreenH;
	Boolean _fullscreen, _adjustAspectRatio, _wide;
	struct {
		Coord x;
		Coord y;
		UInt32 addr;
	} _screenOffset;

	byte *_screenP, *_offScreenP;
	int _offScreenPitch;
	int _screenPitch;


	bool _mouseVisible;
	bool _mouseDrawn;

	enum {
		MAX_MOUSE_W = 40,	// must be 80x80 with 640x480 games ?
		MAX_MOUSE_H = 40
	};

	int _screenWidth, _screenHeight;

	struct MousePos {
		int16 x,y,w,h;
	};

	UInt16 _mode, _initMode, _setMode;
	Boolean _modeChanged, _gfxLoaded;
	byte *_mouseDataP;
	byte *_mouseBackupP;
	MousePos _mouseCurState;
	MousePos _mouseOldState;
	int16 _mouseHotspotX;
	int16 _mouseHotspotY;
	int _current_shake_pos;
	int _new_shake_pos;
	byte _mouseKeyColor;

	UInt32 _exit_delay;

	struct {
		uint32 duration, nextExpiry;
		bool active;
		TimerProc callback;
	} _timer;

	// Audio
	int _samplesPerSec;
	SoundDataType _sound;

	// Palette data
	RGBColorType *_currentPalette;
	uint _paletteDirtyStart, _paletteDirtyEnd;

	void draw_mouse();
	void undraw_mouse();

	void sound_handler();
	void timer_handler(UInt32 current_msecs);

	void getCoordinates(EventPtr event, Coord *x, Coord *y);
	void draw1BitGfx(UInt16 id, Int32 x, Int32 y, Boolean clear);

	void load_gfx_mode();
	void hotswap_gfx_mode(int mode);
	void unload_gfx_mode();

	// Battery
	Boolean _showBatLow;
	void battery_handler();

	// Alarm
/*	Boolean _alarmRaised;
	void alarm_handler();
*/
	CDAudio *_cdPlayer;
	// PALM spec

	enum {
		kLastKeyNone			= 0,
		kLastKeyMouse			= 1	<< 0,

		kLastKeyCalc			= 1 << 30,
		kLastKeyAny				= 1 << 31
	};


	UInt32 _keyMask;
	struct {
		UInt32 bitUp;
		UInt32 bitDown;
		UInt32 bitLeft;
		UInt32 bitRight;
		UInt32 bitButLeft;
	} _keyMouse;

	Int32 _lastKeyPressed;
	UInt32 _lastKeyRepeat;
	UInt8 _lastKeyModifier;

	Boolean _useNumPad;
	Boolean _useHRmode;

	eventsEnum _lastEvent;

	OSystem_PALMOS();
	void init_intern();
};

Err HwrDisplayPalette(UInt8 operation, Int16 startIndex,
			 	  			 UInt16 paletteEntries, RGBColorType *tableP)
							SYS_TRAP(sysTrapHwrDisplayPalette);

// Sound
void pcm2adpcm(Int16 *src, UInt8 *dst, UInt32 length);
Err sndCallback(void* UserDataP, SndStreamRef stream, void* bufferP, UInt32 *bufferSizeP);

#endif
