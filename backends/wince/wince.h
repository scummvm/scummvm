/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001/2002 The ScummVM project
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

#include "stdafx.h"
#include "backends/intern.h"

//#include "commctrl.h"

#if _WIN32_WCE < 300

#include <Wingdi.h>
#include <Winbase.h>
#include <Wtypes.h>

#endif

#include <Winuser.h>
#include <Winnls.h>
#include <sipapi.h>
#include <gx.h>
#include "resource.h"

#include "scumm.h"
//#include "debug.h"
#include "screen.h"
#include "gui/newgui.h"
#include "sound/mididrv.h"
#include "gameDetector.h"
//#include "simon/simon.h"
#include "gapi_keys.h"
#include "config-file.h"


#include "SDL.h"
#include "SDL_audio.h"
#include "SDL_timer.h"
#include "SDL_thread.h"

#include "dynamic_imports.h"

#include "backends/fs/fs.h"

#if (defined(MIPS) || defined(SH3)) && (_WIN32_WCE < 300)
// Comment this out if you don't want to support GameX
#define GAMEX
#endif

#ifdef GAMEX
#include "GameX.h"
#endif

class OSystem_WINCE3;

#include "pocketpc.h"
#include "smartphone.h"

#define MAX_NUMBER_OF_DIRTY_SQUARES 32

class OSystem_WINCE3 : public OSystem {
public:
	// Set colors of the palette
	void set_palette(const byte *colors, uint start, uint num);

	// Set the size of the video bitmap.
	// Typically, 320x200
	void init_size(uint w, uint h);

	// Draw a bitmap to screen.
	// The screen will not be updated to reflect the new bitmap
	void copy_rect(const byte *buf, int pitch, int x, int y, int w, int h);

	// Update the dirty areas of the screen
	void update_screen();

	// Either show or hide the mouse cursor
	bool show_mouse(bool visible);
	
	// Set the position of the mouse cursor
	void set_mouse_pos(int x, int y);
	
	// Set the bitmap that's used when drawing the cursor.
	void set_mouse_cursor(const byte *buf, uint w, uint h, int hotspot_x, int hotspot_y);
	
	// Shaking is used in SCUMM. Set current shake position.
	void set_shake_pos(int shake_pos);
		
	// Get the number of milliseconds since the program was started.
	uint32 get_msecs();
	
	// Delay for a specified amount of milliseconds
	void delay_msecs(uint msecs);
	
	// Create a thread
	void create_thread(ThreadProc *proc, void *param);
	
	// Get the next event.
	// Returns true if an event was retrieved.	
	bool poll_event(Event *event);
	
	// Set function that generates samples 
	bool set_sound_proc(SoundProc *proc, void *param, SoundFormat format);
		
	// Poll cdrom status
	// Returns true if cd audio is playing
	bool poll_cdrom();

	// Play cdrom audio track
	void play_cdrom(int track, int num_loops, int start_frame, int duration);

	// Stop cdrom audio track
	void stop_cdrom();

	// Update cdrom audio status
	void update_cdrom();

	// Add a new callback timer
	void set_timer(int timer, int (*callback)(int));

	// Quit
	void quit();

	// Set a parameter
	uint32 property(int param, Property *value);

	// Overlay
	void show_overlay();
	void hide_overlay();
	void clear_overlay();
	void grab_overlay(int16 *buf, int pitch);
	void copy_rect_overlay(const int16 *buf, int pitch, int x, int y, int w, int h);

	void move_screen(int dx, int dy, int height);

	static OSystem *create(int gfx_mode, bool full_screen);

	// Added for hardware keys mapping

	void addEventKeyPressed(int ascii_code);

	void addEventRightButtonClicked();

	// Mutex functions

	MutexRef create_mutex();
	void lock_mutex(MutexRef);
	void unlock_mutex(MutexRef);
	void delete_mutex(MutexRef);

	// New helpers

	int16 get_height();
	int16 get_width();
	void clear_sound_proc();

	// Windows callbacks & stuff
	static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

//private:
	// Windows callbacks & stuff
	//bool handleMessage();
	

	byte *_gfx_buf;
	byte *_overlay_buf;
	uint _screenHeight;
	uint _screenWidth;
	bool _overlay_visible;
	uint32 _start_time;
	Event _event;
	Event _last_mouse_event;
	HMODULE hInst;
	HWND hWnd;
	bool _display_cursor;	

	enum {
		DF_FORCE_FULL_ON_PALETTE = 1,
		DF_WANT_RECT_OPTIM = 2,
		DF_2xSAI = 4,
		DF_SEPARATE_HWSCREEN = 8,
		DF_UPDATE_EXPAND_1_PIXEL = 16,
	};

	int _mode;
	bool _full_screen;
	bool _mouse_visible;
	bool _mouse_drawn;
	uint32 _mode_flags;

	byte _internal_scaling;

	bool force_full; //Force full redraw on next update_screen
	bool cksum_valid;

	enum {
		NUM_DIRTY_RECT = 100,
		SCREEN_WIDTH = 320,
		SCREEN_HEIGHT = 200,
		CKSUM_NUM = (SCREEN_WIDTH*SCREEN_HEIGHT/(8*8)),

		MAX_MOUSE_W = 80,
		MAX_MOUSE_H = 80,
		MAX_SCALING = 3,

		TMP_SCREEN_OFFS = 320*2 + 8,
	};

	/* CD Audio */
	int cd_track, cd_num_loops, cd_start_frame, cd_end_frame;
	Uint32 cd_end_time, cd_stop_time, cd_next_second;

	struct MousePos {
		int16 x,y,w,h;
	};

	byte *_ms_buf;
	byte *_ms_backup;
	MousePos _ms_cur;
	MousePos _ms_old;
	int16 _ms_hotspot_x;
	int16 _ms_hotspot_y;
	int _current_shake_pos;

	
	static void fill_sound(void *userdata, Uint8 * stream, int len);
	

	void draw_mouse();
	void undraw_mouse();
	void warp_mouse(int x, int y);

	void load_gfx_mode();
	void unload_gfx_mode();

	void hotswap_gfx_mode();

	void get_320x200_image(byte *buf);
};