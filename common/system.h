/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
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

#ifndef COMMON_SYSTEM_H
#define COMMON_SYSTEM_H

#include "scummsys.h"

// Interface to the ScummVM backend

class OSystem {
public:
	typedef int ThreadProc(void *param);
	typedef void SoundProc(void *param, byte *buf, int len);

	struct Event {
		int event_code;
		struct {
			uint16 ascii;
			byte flags;
			int keycode;
		} kbd;
		struct {
			int x,y;
		} mouse;
	};

	enum {
		EVENT_KEYDOWN = 1,
		EVENT_KEYUP = 2,
		EVENT_MOUSEMOVE = 3,
		EVENT_LBUTTONDOWN = 4,
		EVENT_LBUTTONUP = 5,
		EVENT_RBUTTONDOWN = 6,
		EVENT_RBUTTONUP = 7,
	};

	enum {
		KBD_CTRL = 1,
		KBD_ALT = 2,
		KBD_SHIFT = 4,
	};

	enum {
		PROP_TOGGLE_FULLSCREEN = 1,
		PROP_SET_WINDOW_CAPTION = 2,
		PROP_OPEN_CD = 3,
		PROP_SET_GFX_MODE = 4,
		PROP_SHOW_DEFAULT_CURSOR = 5,
		PROP_GET_SAMPLE_RATE = 6,
		PROP_GET_FULLSCREEN = 7
	};
	union Property {
		char *caption;
		int cd_num;
		int gfx_mode;
		bool show_cursor;
	};
	
	enum {
		SOUND_8BIT = 0,
		SOUND_16BIT = 1,
	};
	
	// Set colors of the palette
	virtual void set_palette(const byte *colors, uint start, uint num) = 0;

	// Set the size of the video bitmap.
	// Typically, 320x200
	virtual void init_size(uint w, uint h) = 0;

	// Draw a bitmap to screen.
	// The screen will not be updated to reflect the new bitmap
	virtual void copy_rect(const byte *buf, int pitch, int x, int y, int w, int h) = 0;

	// Moves the screen content around by the given amount of pixels
	// but only the top height pixel rows, the rest stays untouched
	virtual void move_screen(int dx, int dy, int height) = 0;

	// Update the dirty areas of the screen
	virtual void update_screen() = 0;

	// Either show or hide the mouse cursor
	virtual bool show_mouse(bool visible) = 0;
	
	// Set the position of the mouse cursor
	virtual void set_mouse_pos(int x, int y) = 0;
	
	// Set the bitmap that's used when drawing the cursor.
	virtual void set_mouse_cursor(const byte *buf, uint w, uint h, int hotspot_x, int hotspot_y) = 0;
	
	// Shaking is used in SCUMM. Set current shake position.
	virtual void set_shake_pos(int shake_pos) = 0;
		
	// Get the number of milliseconds since the program was started.
	virtual uint32 get_msecs() = 0;
	
	// Delay for a specified amount of milliseconds
	virtual void delay_msecs(uint msecs) = 0;
	
	// Create a thread
	virtual void *create_thread(ThreadProc *proc, void *param) = 0;
	
	// Get the next event.
	// Returns true if an event was retrieved.	
	virtual bool poll_event(Event *event) = 0;

	// Set the function to be invoked whenever samples need to be generated
	// Format is the sample type format.
	// Only 16-bit signed mode is needed for simon & scumm
	virtual bool set_sound_proc(void *param, SoundProc *proc, byte format) = 0;
	
	// Get or set a property
	virtual uint32 property(int param, Property *value) = 0;
		
	// Poll cdrom status
	// Returns true if cd audio is playing
	virtual bool poll_cdrom() = 0;

	// Play cdrom audio track
	virtual void play_cdrom(int track, int num_loops, int start_frame, int end_frame) = 0;

	// Stop cdrom audio track
	virtual void stop_cdrom() = 0;

	// Update cdrom audio status
	virtual void update_cdrom() = 0;

	// Add a new callback timer
	virtual void set_timer(int timer, int (*callback)(int)) = 0;

	// Mutex handling
	virtual void *create_mutex(void) = 0;
	virtual void lock_mutex(void *mutex) = 0;
	virtual void unlock_mutex(void *mutex) = 0;
	virtual void delete_mutex(void *mutex) = 0;

	// Quit
	virtual void quit() = 0;
};


/* Factory functions. This means we don't have to include the
 * OSystem_SDL header file. (which in turn would require the SDL headers)
 */

/* OSystem_SDL */
extern OSystem *OSystem_SDL_create(int gfx_driver, bool full_screen);
extern OSystem *OSystem_NULL_create();
extern OSystem *OSystem_MorphOS_create(int game_id, int gfx_driver, bool full_screen);
extern OSystem *OSystem_Dreamcast_create();
extern OSystem *OSystem_WINCE3_create();
extern OSystem *OSystem_X11_create();
extern OSystem *OSystem_MAC_create(int gfx_mode, bool full_screen);

enum {
	GFX_NORMAL = 0,
	GFX_DOUBLESIZE = 1,
	GFX_TRIPLESIZE = 2,
	GFX_2XSAI = 3,
	GFX_SUPER2XSAI = 4,
	GFX_SUPEREAGLE = 5,
	GFX_ADVMAME2X = 6
};


/* Graphics drivers */
enum {
	GD_NULL = 0,
	GD_SDL,
	GD_X,
	GD_MORPHOS,
	GD_WINCE,
	GD_MAC,
	GD_DC
};

enum {
#ifdef _WIN32_WCE
	SAMPLES_PER_SEC =  11025
#else
	SAMPLES_PER_SEC =  22050
#endif
};

#endif 
