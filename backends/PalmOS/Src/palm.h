/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001-2004 The ScummVM project
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

#ifndef PALM_H
#define PALM_H

#include <SonyClie.h>
#include "common/system.h"
#include "cdaudio.h"

Err HwrDisplayPalette(UInt8 operation, Int16 startIndex, 
			 	  			 UInt16 paletteEntries, RGBColorType *tableP)
							SYS_TRAP(sysTrapHwrDisplayPalette);

typedef struct {
	bool active;	
	OSystem::SoundProc proc;
	void *param;
	OSystem::SoundFormat format;
	SndStreamRef sndRefNum;
} SoundDataType;

//-- 02-12-17 --////////////////////////////////////////////////////////////////
class OSystem_PALMOS : public OSystem {
public:
	// Set colors of the palette
	void set_palette(const byte *colors, uint start, uint num);

	// Set the size of the video bitmap.
	// Typically, 320x200
	void init_size(uint w, uint h);

	// Draw a bitmap to screen.
	// The screen will not be updated to reflect the new bitmap
	void copy_rect(const byte *buf, int pitch, int x, int y, int w, int h);

	// Moves the screen content around by the given amount of pixels
	// but only the top height pixel rows, the rest stays untouched
	void move_screen(int dx, int dy, int height);

	// Update the dirty areas of the screen
	void update_screen();

	// Either show or hide the mouse cursor
	bool show_mouse(bool visible);

	// Set the position of the mouse cursor
	void set_mouse_pos(int x, int y);

	// Warp the mouse cursor. Where set_mouse_pos() only informs the
	// backend of the mouse cursor's current position, this function
	// actually moves the cursor to the specified position.
	void warp_mouse(int x, int y);


	// Set the bitmap that's used when drawing the cursor.
	void set_mouse_cursor(const byte *buf, uint w, uint h, int hotspot_x, int hotspot_y);

	// Shaking is used in SCUMM. Set current shake position.
	void set_shake_pos(int shake_pos);
		
	// Get the number of milliseconds since the program was started.
	uint32 get_msecs();
	
	// Delay for a specified amount of milliseconds
	void delay_msecs(uint msecs);
	
	// Get the next event.
	// Returns true if an event was retrieved.	
	bool poll_event(Event *event);
	
	void SimulateArrowKeys(Event *event, Int8 iHoriz, Int8 iVert);

	/** @name Sound */
	//@{
	/**
	 * Set the audio callback which is invoked whenever samples need to be generated.
	 * Currently, only the 16-bit signed mode is ever used for Simon & Scumm
	 * @param proc		pointer to the callback.
	 * @param param		an arbitrary parameter which is stored and passed to proc.
	 * @param format	the sample type format.
	 */
	bool set_sound_proc(SoundProc proc, void *param, SoundFormat format);
	
	/**
	 * Remove any audio callback previously set via set_sound_proc, thus effectively
	 * stopping all audio output immediately.
	 * @see set_sound_proc
	 */
	void clear_sound_proc();
	//@} 

	// Poll cdrom status
	// Returns true if cd audio is playing
	bool poll_cdrom();

	// Play cdrom audio track
	void play_cdrom(int track, int num_loops, int start_frame, int duration);

	// Stop cdrom audio track
	void stop_cdrom();

	// Update cdrom audio status
	void update_cdrom();

	// Add a callback timer
	void set_timer(TimerProc callback, int timer);

	// Mutex handling
	MutexRef create_mutex();
	void lock_mutex(MutexRef mutex);
	void unlock_mutex(MutexRef mutex);
	void delete_mutex(MutexRef mutex);

	// Quit
	void quit();
	bool _selfQuit;
	UInt8 _quitCount;

	// Overlay
	void show_overlay();
	void hide_overlay();
	void clear_overlay();
	void grab_overlay(byte *buf, int pitch);
	void copy_rect_overlay(const byte *buf, int pitch, int x, int y, int w, int h);

	int16 get_width();
	int16 get_height();
	byte RGBToColor(uint8 r, uint8 g, uint8 b);
	void ColorToRGB(byte color, uint8 &r, uint8 &g, uint8 &b);
	// Set a parameter
	uint32 property(int param, Property *value);

	// Savefile management
	SaveFileManager *get_savefile_manager();

	static OSystem *create(UInt16 gfx_mode, bool full_screen);

	UInt8 _sndHandle;
	Boolean _isSndPlaying;

protected:
	byte *_tmpScreenP, *_tmpBackupP;
	bool _overlayVisible;

private:
	typedef void (OSystem_PALMOS::*RendererProc)();
	RendererProc _renderer_proc;

	UInt8 *_sndDataP, *_sndTempP;

	void update_screen__flipping();
	void update_screen__dbuffer();
	void update_screen__direct();
	void update_screen__wide();

	WinHandle _screenH;
	WinHandle _offScreenH;
	Boolean _fullscreen;
	struct {
		Coord x;
		Coord y;
		UInt32 addr;
	} _screenOffset;

public:
	byte *_screenP;
	int _offScreenPitch;
	int _screenPitch;
	
private:
	byte *_offScreenP;

	bool _mouseVisible;
	bool _mouseDrawn;

	enum {
		MAX_MOUSE_W = 40,	// must be 80x80 with 640x480 games
		MAX_MOUSE_H = 40
	};

	int _screenWidth, _screenHeight;

	struct MousePos {
		int16 x,y,w,h;
	};

	UInt16 _mode;
	byte *_mouseDataP;
	byte *_mouseBackupP;
	MousePos _mouseCurState;
	MousePos _mouseOldState;
	int16 _mouseHotspotX;
	int16 _mouseHotspotY;
	int _current_shake_pos;
	int _new_shake_pos;
	
	Boolean _vibrate;
	UInt32 _exit_delay;
	
	struct {
		uint32 duration, nextExpiry;
		bool active;
		TimerProc callback;
	} _timer;

	SoundDataType _sound;

	// Palette data
	RGBColorType *_currentPalette;
	uint _paletteDirtyStart, _paletteDirtyEnd;

	void check_sound();

	void draw_mouse();
	void undraw_mouse();

	void load_gfx_mode();
	void unload_gfx_mode();
	static void autosave();

	CDAudio *_cdPlayer;
	// PALM spec

	Int32 _lastKeyPressed;
	UInt32 _lastKeyRepeat;
	UInt8 _lastKeyModifier;
	
	Boolean _useNumPad;
	Boolean _useHRmode;

	eventsEnum _lastEvent;

	OSystem_PALMOS();

};

#endif