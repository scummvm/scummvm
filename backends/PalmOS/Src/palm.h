/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001-2003 The ScummVM project
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
#include "SonySndLib.h"
#include "vibrate.h"
#include "globals.h"
#include "system.h"
/*
typedef struct {

	UInt16 HRrefNum;
	UInt16 volRefNum;
	FileRef	logFile;

	Boolean screenLocked;
	Boolean vibrator;

} GlobalsDataType;
*/
//extern UInt16 gHRrefNum;
//extern Boolean gVibrator;
//extern Boolean gFlipping;
//extern Boolean gScreenLocked;
//extern GlobalsDataType *gVars;

//Err CheckHRmode();


Err HwrDisplayPalette(UInt8 operation, Int16 startIndex, 
			 	  			 UInt16 paletteEntries, RGBColorType *tableP)
							SYS_TRAP(sysTrapHwrDisplayPalette);
							

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
	
	// Create a thread
	void *create_thread(ThreadProc *proc, void *param);
	
	// Get the next event.
	// Returns true if an event was retrieved.	
	bool poll_event(Event *event);
	
	void SimulateArrowKeys(Event *event, Int8 iHoriz, Int8 iVert, Boolean repeat);

	// Set function that generates samples 
	bool set_sound_proc(void *param, SoundProc *proc, byte sound);

	// Poll cdrom status
	// Returns true if cd audio is playing
	bool poll_cdrom();

	// Play cdrom audio track
	void play_cdrom(int track, int num_loops, int start_frame, int end_frame);

	// Stop cdrom audio track
	void stop_cdrom();

	// Update cdrom audio status
	void update_cdrom();

	// Add a callback timer
	void set_timer(int timer, int (*callback)(int));

	// Mutex handling
	void *create_mutex(void);
	void lock_mutex(void *mutex);
	void unlock_mutex(void *mutex);
	void delete_mutex(void *mutex);

	// Quit
	void quit();
	bool _quit;

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

	static OSystem *create(UInt16 gfx_mode);

	UInt8 _sndHandle;
	Boolean _isPlaying;

protected:
	bool _overlay_visible;

private:
	struct {
		Int16 state;
		Int16 position;
		UInt32 time;
		UInt32 wait;
		UInt8 color;
		Char text[100];
	} _msg;

	void addMessage(const Char *msg);
	void drawMessage();
	void deleteMessage();
	
	typedef void (OSystem_PALMOS::*RendererProc)(void);
	RendererProc _renderer_proc;

	UInt8 *_sndData;

	void update_screen__flipping();
	void update_screen__dbuffer();
	void update_screen__direct();

	WinHandle h_palm_screen;
	WinHandle h_palm_offscreen;
	WinHandle h_palm_tmpscreen;
	
	byte *palm_screen;
	byte *palm_offscreen;
	byte *palm_tmpscreen;

	bool _mouse_visible;
	bool _mouse_drawn;

	enum {
		MAX_MOUSE_W = 40,
		MAX_MOUSE_H = 40
	};

	int SCREEN_WIDTH, SCREEN_HEIGHT;
	bool _overlaySaved;

	struct MousePos {
		int16 x,y,w,h;
	};

	UInt16 _mode;
	byte *_mouse_data;
	byte *_mouse_backup;
	MousePos _mouse_cur_state;
	MousePos _mouse_old_state;
	int16 _mouse_hotspot_x;
	int16 _mouse_hotspot_y;
	int _current_shake_pos;
	int _new_shake_pos;
	
	UInt16 _decaly, _screeny;
	Boolean _vibrate;
	UInt32 _exit_delay;
	
	struct {
		uint32 duration, next_expiry;
		bool active;
		int (*callback) (int);
	} _timer;

	struct {
		bool active;
		ThreadProc *proc;
		void *param;
		
		struct {
			UInt32 value;
			UInt32 status;
		} sleep;

	} _thread;

	struct {
		bool active;	
		SoundProc *proc;
		void *param;
	} _sound;

	// Palette data
	RGBColorType *_currentPalette;
	uint _paletteDirtyStart, _paletteDirtyEnd;

	void check_sound();

	void draw_mouse();
	void undraw_mouse();

	void load_gfx_mode();
	void unload_gfx_mode();
/*
	void hotswap_gfx_mode();

	void get_320x200_image(byte *buf);
*/	static void autosave();

	// PALM spec

	void drawKeyState();

	Int32 lastKeyPressed;
	UInt32 lastKeyRepeat;
	UInt8 lastKeyModifier;

	eventsEnum lastEvent;

	// sound support
	SndPcmFormatType _snd_format;
	SndPcmOptionsType _snd_options;
	

	OSystem_PALMOS();

};

#endif