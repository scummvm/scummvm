/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001/2002 The ScummVM project
 * Copyright (C) 2002 ph0x (GP32 port)
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
 */


#include "backends/intern.h"
#include "common/stdafx.h"
#include "common/scummsys.h"
#include "common/system.h"
#include "common/scummsys.h"
#include "common/stdafx.h"
#include "base/engine.h"
#include "scumm/saveload.h"
#include "common/scaler.h"

#include "portdefs.h"
#include "sdl.h"

class OSystem_GP32 : public OSystem {
public:
	// Set colors of the palette
	void set_palette(const byte *colors, uint start, uint num);

	// Set the size of the video bitmap.
	// Typically, 320x200
	void init_size(uint w, uint h);
	int16 get_height() { return _screenHeight; }
	int16 get_width() { return _screenWidth; }
	
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
	void warp_mouse(int x, int y);
	
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

	// Set the function to be invoked whenever samples need to be generated
	// Format is the sample type format.
	// Only 16-bit signed mode is needed for simon & scumm
	bool set_sound_proc(SoundProc *proc, void *param, SoundFormat format);
	void clear_sound_proc();
	
	// Get or set a property
	uint32 property(int param, Property *value);
		
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

	// Mutex handling
	OSystem::MutexRef create_mutex();
	void lock_mutex(MutexRef mutex);
	void unlock_mutex(MutexRef mutex);
	void delete_mutex(MutexRef mutex);

	// Quit
	void quit();
	
	// Overlay
	void show_overlay();
	void hide_overlay();
	void clear_overlay();
	void grab_overlay(int16 *buf, int pitch);
	void copy_rect_overlay(const int16 *buf, int pitch, int x, int y, int w, int h);

	static OSystem *create(int gfx_mode, bool full_screen);

private:
	SDL_Surface *sdl_tmpscreen;   // temporary screen (for scalers/overlay)
	SDL_Surface *sdl_hwscreen;    // hardware screen
	bool _overlay_visible;

	ScalerProc *_scaler_proc;

	int TMP_SCREEN_WIDTH;

	//uint msec_start;
	//uint32 get_ticks();

	///OSystem_GP32(); // eh?
	/// ~OSystem_GP32();

	// unseen game screen
	SDL_Surface *_screen;
	int _screenWidth, _screenHeight;

	// CD Audio
	///SDL_CD *_cdrom;
	int cd_track, cd_num_loops, cd_start_frame, cd_end_frame;
	uint32 cd_end_time, cd_stop_time, cd_next_second;

	enum {
		DF_WANT_RECT_OPTIM			= 1 << 0,
		DF_UPDATE_EXPAND_1_PIXEL	= 1 << 3
	};

	bool _forceFull; // Force full redraw on next update_screen
	int _scaleFactor;
	int _mode;
	bool _full_screen;
	uint32 _mode_flags;

	enum {
		NUM_DIRTY_RECT = 100,

		MAX_MOUSE_W = 40,
		MAX_MOUSE_H = 40,
		MAX_SCALING = 3
	};

	// Dirty rect managment
	SDL_Rect _dirty_rect_list[100];
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
	MousePos _mouse_cur_state;
	MousePos _mouse_old_state;
	int16 _mouseHotspotX;
	int16 _mouseHotspotY;

	// Shake mode
	int _currentShakePos;
	int _newShakePos;

	// Palette data
	SDL_Color *_currentPalette;
	uint _paletteDirtyStart, _paletteDirtyEnd;


	void add_dirty_rgn_auto(const byte *buf);
	void mk_checksums(const byte *buf);

	static void fill_sound(void *userdata, Uint8 * stream, int len);
	
	void add_dirty_rect(int x, int y, int w, int h);

	void draw_mouse();
	void undraw_mouse();

	void load_gfx_mode();
	void unload_gfx_mode();
	void hotswap_gfx_mode();

	void get_screen_image(byte *buf);	

	void setup_icon();
	void kbd_mouse();

	static OSystem_GP32 *create();
};
