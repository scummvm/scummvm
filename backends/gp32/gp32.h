/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001/2004 The ScummVM project
 * Copyright (C) 2002 Ph0x - GP32 Backend
 * Copyright (C) 2003/2004 DJWillis - GP32 Backend
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

#ifndef GP32_H
#define GP32_H

/////////////////////////////////////////////////////////////////////////////
// Header for GP32 ScummVM Backend
/////////////////////////////////////////////////////////////////////////////

//Standard ScummVM includes.
#include "common/stdafx.h"
#include "common/scummsys.h"
#include "common/scaler.h"
#include "common/system.h"
#include "backends/intern.h"

#include "base/engine.h"
//#include "scumm/saveload.h"

//Graphics wrapper - SDL Derived.
#include "backends/gp32/graphics.h"

//Pre-ScummVM Config stuff
//#include "backends/gp32/config.h"

#define LCD_WIDTH 				320
#define LCD_HEIGHT 				240
#define LCD_WIDTH_CENTER 		LCD_WIDTH / 2
#define LCD_HEIGHT_CENTER 		LCD_HEIGHT / 2
#define LCD_SIZE				(LCD_WIDTH * LCD_HEIGHT)

// Graphics Bits
#define GAME_SURFACE		0
#define GAME_SURFACE2		1
#define DEBUG_SURFACE		2

extern int nflip, keydata;	// Flip Index
extern GP_HPALETTE PAL; //palette

//void Delay(int ms);
//void FadeToBlack(int delay_time);


#define BUFFERCOUNT 2
extern GPDRAWSURFACE LCDbuffer[BUFFERCOUNT + 1];  //buffers

#define BACKGROUND_COLOR		0	//Black color
#define COLOUR_8BIT_MODE		8
#define COLOUR_16BIT_MODE		16

/* Assembly routines from cpuspeed.s */
extern "C" {void cpu_speed(int,int,int);}	// CPU Speed control (3 int's, clock, bus, dividor)

/* Fake main() entry point */
//int main_scummvm(int argc, char *argv[]);

//Prototypes
class OSystem_GP32 : public OSystem
{
public:
	OSystem_GP32();
	virtual ~OSystem_GP32();

	// Set colors of the palette
	void setPalette(const byte *colors, uint start, uint num);

	// Set the size of the video bitmap.
	// Typically, 320x200
	void initSize(uint w, uint h, int overlayScale);
	int16 getHeight() { return _screenHeight; }
	int16 getWidth() { return _screenWidth; }

	// Draw a bitmap to screen.
	// The screen will not be updated to reflect the new bitmap
	void copyRectToScreen(const byte *buf, int pitch, int x, int y, int w, int h);

	// Update the dirty areas of the screen
	void updateScreen();

	// Either show or hide the mouse cursor
	bool showMouse(bool visible);

	void warpMouse(int x, int y);

	// Set the bitmap that's used when drawing the cursor.
	void setMouseCursor(const byte *buf, uint w, uint h, int hotspot_x, int hotspot_y, byte keycolor = 255, int cursorTargetScale = 1);

	// Shaking is used in SCUMM. Set current shake position.
	void setShakePos(int shake_pos);

	// Get the number of milliseconds since the program was started.
	uint32 getMillis();

	// Delay for a specified amount of milliseconds
	void delayMillis(uint msecs);

	// Get the next event.
	// Returns true if an event was retrieved.
	bool pollEvent(Event &event);

	// Set the function to be invoked whenever samples need to be generated
	// Format is the sample type format.
	// Only 16-bit signed mode is needed for simon & scumm
	bool setSoundCallback(SoundProc proc, void *param);

	// Clear Sound Sample
	void clearSoundCallback();

	// OLD - Delete when sure not needed.
	// Get or set a property
	//uint32 property(int param, Property *value);
	//virtual uint32 property(int param, Property *value);

	// Poll cdrom status
	// Returns true if cd audio is playing
	bool pollCD();

	// Play cdrom audio track
	void playCD(int track, int num_loops, int start_frame, int duration);

	// Stop cdrom audio track
	void stopCD();

	// Update cdrom audio status
	void updateCD();

	// Add a new callback timer
	void setTimerCallback(TimerProc callback, int timer);

	// Mutex handling
	OSystem::MutexRef createMutex();
	void lockMutex(MutexRef mutex);
	void unlockMutex(MutexRef mutex);
	void deleteMutex(MutexRef mutex);
	// DJWillis

	//helper
	int16 get_height();
	int16 get_width();

	// Quit
	void quit();

	// Overlay
	void showOverlay();
	void hideOverlay();
	void clearOverlay();
	void grabOverlay(int16 *buf, int pitch);
	void copyRectToOverlay(const int16 *buf, int pitch, int x, int y, int w, int h);

	//static OSystem *create(int gfx_mode, bool full_screen);

	int16 RBGToColor(uint8 r, uint8 g, uint8 b);
	void colorToRBG(int16 color, uint8 &r, uint8 &g, uint8 &b);

	//void *gm_yocalloc(size_t num_elem, size_t size_elem);
	//void *gm_realloc(void *ptr, size_t size);

	// New 0.6.0 +
	const GraphicsMode *getSupportedGraphicsModes() const;
	int getDefaultGraphicsMode() const;
	bool setGraphicsMode(int mode);
	int getGraphicsMode() const;

	void setWindowCaption(const char *caption);
	bool openCD(int drive);

	// Determine the output sample rate. Audio data provided by the sound
	// Callback sound be played using this rate.
	// FIXME: GP32 Hardcodes rate currently.
	int getOutputSampleRate() const;

	bool hasFeature(Feature f);
	void setFeatureState(Feature f, bool enable);
	bool getFeatureState(Feature f);

	//static OSystem *create();

private:

	// Set the position of the mouse cursor - Not part of OSystem anymore.
	void set_mouse_pos(int x, int y);

	gpSurface *_tmpscreen;   // temporary screen (for scalers/overlay)
	gpSurface *_hwscreen;    // hardware screen
	bool _overlayVisible;

	ScalerProc *_scaler_proc;

	int TMP_SCREEN_WIDTH;

	//uint msec_start;
	//uint32 get_ticks();

	// unseen game screen
	gpSurface *_screen;
	int _screenWidth, _screenHeight;

	// CD Audio, Not used in GP32 backend
	int cd_track, cd_num_loops, cd_start_frame, cd_end_frame;
	uint32 cd_end_time, cd_stop_time, cd_next_second;

	enum {
		DF_WANT_RECT_OPTIM			= 1 << 0,
		DF_UPDATE_EXPAND_1_PIXEL	= 1 << 3
	};

	bool _forceFull; // Force full redraw on next updateScreen
	int _scaleFactor;
	int _mode;
	bool _full_screen;
	uint32 _mode_flags;
	bool _modeChanged;

	/** True if aspect ratio correction is enabled. */
	bool _adjustAspectRatio;

	enum {
		NUM_DIRTY_RECT = 100,
		MAX_MOUSE_W = 40,
		MAX_MOUSE_H = 40,
		MAX_SCALING = 3
	};

	// Dirty rect managment
	gpRect _dirty_rect_list[100];
	int _num_dirty_rects;
	uint32 *_dirty_checksums;
	bool cksum_valid;
	int CKSUM_NUM;

	// Keyboard mouse emulation
	struct KbdMouse {
		int16 x, y, x_vel, y_vel, x_max, y_max, x_down_count, y_down_count;
		uint32 last_time, delay_time, x_down_time, y_down_time;
	} km;

	struct MousePos {
		int16 x, y, w, h;
	};

	bool _mouseVisible;
	bool _mouseDrawn;
	byte *_mouseData;
	byte *_mouseBackup;
	MousePos _mouseCurState;
	MousePos _mouseOldState;
	int16 _mouseHotspotX;
	int16 _mouseHotspotY;
	byte _mouseKeycolor;

	// Shake mode
	int _currentShakePos;
	int _newShakePos;

	// Palette data
	gpColor *_currentPalette;
	uint _paletteDirtyStart, _paletteDirtyEnd;

	/**
	 * Mutex which prevents multiple threads from interfering with each other
	 * when accessing the screen.
	 */
	MutexRef _graphicsMutex;

	void add_dirty_rgn_auto(const byte *buf);
	void mk_checksums(const byte *buf);

	static void fill_sound(void *userdata, Uint8 * stream, int len);

	void add_dirty_rect(int x, int y, int w, int h);

	void draw_mouse();
	void undraw_mouse();

	void internUpdateScreen();

	void load_gfx_mode();
	void unload_gfx_mode();
	void hotswap_gfx_mode();

	void setFullscreenMode(bool enable);

	void get_screen_image(byte *buf);

	void setup_icon();
	void kbd_mouse();
	//static OSystem_GP32 *create();
};

#else
	#warning GP32.H Called more then once.
#endif /* GP32_H */

