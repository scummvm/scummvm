/* ScummVM - Scumm Interpreter
 * Dreamcast port
 * Copyright (C) 2002-2004  Marcus Comstedt
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

#include <common/system.h>
#include <ronin/soundcommon.h>

#define NUM_BUFFERS 4

class OSystem_Dreamcast : public OSystem {

 public:
  OSystem_Dreamcast();

  // Set colors of the palette
  void set_palette(const byte *colors, uint start, uint num);

  // Set the size of the video bitmap.
  // Typically, 320x200
  void initSize(uint w, uint h);
  int16 get_height() { return _screen_h; }
  int16 get_width() { return _screen_w; }

  // Draw a bitmap to screen.
  // The screen will not be updated to reflect the new bitmap
  void copy_rect(const byte *buf, int pitch, int x, int y, int w, int h);
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
  
  // Get the next event.
  // Returns true if an event was retrieved.	
  bool poll_event(Event *event);
  
  // Set function that generates samples 
  bool setSoundCallback(SoundProc proc, void *param);
  void clearSoundCallback();
		
  // Poll cdrom status
  // Returns true if cd audio is playing
  bool poll_cdrom();

  // Play cdrom audio track
  void play_cdrom(int track, int num_loops, int start_frame, int duration);
  
  // Stop cdrom audio track
  void stop_cdrom();

  // Update cdrom audio status
  void update_cdrom();

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

  // Add a callback timer
  virtual void set_timer(TimerProc callback, int timer);

  // Mutex handling
  virtual MutexRef create_mutex();
  virtual void lock_mutex(MutexRef mutex);
  virtual void unlock_mutex(MutexRef mutex);
  virtual void delete_mutex(MutexRef mutex);

  // Savefile handling
  virtual SaveFileManager *get_savefile_manager();


  static OSystem *create();


 private:

  int _ms_cur_x, _ms_cur_y, _ms_cur_w, _ms_cur_h, _ms_old_x, _ms_old_y;
  int _ms_hotspot_x, _ms_hotspot_y, _ms_visible, _devpoll;
  int _current_shake_pos, _screen_w, _screen_h;
  int _overlay_x, _overlay_y;
  unsigned char *_ms_buf;
  SoundProc _sound_proc;
  void *_sound_proc_param;
  bool _overlay_visible, _overlay_dirty, _screen_dirty;
  int _screen_buffer, _overlay_buffer, _mouse_buffer;
  bool _hires, _hires_mouse;
  float _overlay_fade;

  uint32 _timer_duration, _timer_next_expiry;
  bool _timer_active;
  int (*_timer_callback) (int);

  unsigned char *screen;
  unsigned short *mouse;
  unsigned short *overlay;
  void *screen_tx[NUM_BUFFERS];
  void *mouse_tx[NUM_BUFFERS];
  void *ovl_tx[NUM_BUFFERS];
  unsigned short palette[256];

  short temp_sound_buffer[RING_BUFFER_SAMPLES];

  void checkSound();

  void drawMouse(int xdraw, int ydraw, int w, int h,
		 unsigned char *buf, bool visible);

};

extern int handleInput(struct mapledev *pad,
		       int &mouse_x, int &mouse_y,
		       byte &shiftFlags);
extern void initSound();
extern bool selectGame(char *&, char *&, class Icon &);

