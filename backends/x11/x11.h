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
 * $URL$
 * $Id$
 *
 */

class OSystem_X11:public OSystem {
public:
	OSystem_X11();
	~OSystem_X11();

	void initBackend();

	// Determine whether the backend supports the specified feature.
	bool hasFeature(Feature f);

	// En-/disable the specified feature.
	void setFeatureState(Feature f, bool enable);

	// Query the state of the specified feature.
	bool getFeatureState(Feature f);

	// Retrieve a list of all graphics modes supported by this backend.
	const GraphicsMode *getSupportedGraphicsModes() const;

	// Return the ID of the 'default' graphics mode.
	int getDefaultGraphicsMode() const;

	// Switch to the specified graphics mode.
	bool setGraphicsMode(int mode);

	// Determine which graphics mode is currently active.
	int getGraphicsMode() const;

	// Set colors of the palette
	void setPalette(const byte *colors, uint start, uint num);

	// Set the size of the video bitmap.
	// Typically, 320x200
	void initSize(uint w, uint h, int overlaySize);

	// Draw a bitmap to screen.
	// The screen will not be updated to reflect the new bitmap
	void copyRectToScreen(const byte *buf, int pitch, int x, int y, int w, int h);

	// Update the dirty areas of the screen
	void updateScreen();

	// Either show or hide the mouse cursor
	bool showMouse(bool visible);

	// Set the position of the mouse cursor
	void set_mouse_pos(int x, int y);

	// Warp the mouse cursor. Where set_mouse_pos() only informs the
	// backend of the mouse cursor's current position, this function
	// actually moves the cursor to the specified position.
	void warpMouse(int x, int y);

	// Set the bitmap that's used when drawing the cursor.
	void setMouseCursor(const byte *buf, uint w, uint h, int hotspotX, int hotspotY, byte keycolor = 255, int cursorTargetScale = 1);

	// Shaking is used in SCUMM. Set current shake position.
	void setShakePos(int shake_pos);

	// Get the number of milliseconds since the program was started.
	uint32 getMillis();

	// Delay for a specified amount of milliseconds
	void delayMillis(uint msecs);

	// Get the next event.
	// Returns true if an event was retrieved.
	bool pollEvent(Event &event);

	// Set function that generates samples
	bool setSoundCallback(SoundProc proc, void *param);
	void clearSoundCallback();

	// Determine the output sample rate. Audio data provided by the sound
	// callback will be played using this rate.
	int getOutputSampleRate() const;

	// Initialise the specified CD drive for audio playback.
	bool openCD(int drive);

	// Poll cdrom status
	// Returns true if cd audio is playing
	bool pollCD();

	// Play cdrom audio track
	void playCD(int track, int num_loops, int start_frame, int duration);

	// Stop cdrom audio track
	void stopCD();

	// Update cdrom audio status
	void updateCD();

	// Quit
	void quit();

	// Add a callback timer
	void setTimerCallback(TimerProc callback, int interval);

	// Mutex handling
	MutexRef createMutex();
	void lockMutex(MutexRef mutex);
	void unlockMutex(MutexRef mutex);
	void deleteMutex(MutexRef mutex);

	// Overlay handling for the new menu system
	void showOverlay();
	void hideOverlay();
	void clearOverlay();
	void grabOverlay(int16 *, int);
	void copyRectToOverlay(const int16 *, int, int, int, int, int);
	virtual int16 getHeight();
	virtual int16 getWidth();

	virtual void grabPalette(byte *colors, uint start, uint num);

	// Set a window caption or any other comparable status display to the
	// given value.
	void setWindowCaption(const char *caption);

	static OSystem *create(int gfx_mode, bool full_screen);

private:
	typedef struct {
		int x, y;
		int w, h;
		int hot_x, hot_y;
	} MouseState;

	typedef struct {
		int x, y, w, h;
	} DirtyRect;

	enum {
		MAX_NUMBER_OF_DIRTY_RECTS = 32
	};

	void create_empty_cursor();
	void draw_mouse(DirtyRect *dout);
	void undraw_mouse();
	void updateScreen_helper(const DirtyRect *d, DirtyRect *dout);
	void blit_convert(const DirtyRect *d, uint8 *dst, int pitch);
	void blit(const DirtyRect *d, uint16 *dst, int pitch);

	uint8 *_local_fb;
	uint16 *_local_fb_overlay;
	bool _overlay_visible;

	int _window_width, _window_height;
	int _fb_width, _fb_height;
	int _scumm_x, _scumm_y;

	uint16 *_palette16;
	uint32 *_palette32;

	bool _palette_changed;
	Display *_display;
	int _screen, _depth;
	uint8 _bytesPerPixel;
	Window _window;
	GC _black_gc;
	XImage *_image;
	pthread_t _sound_thread;

	struct timeval _start_time;

	int _fake_right_mouse;
	int _report_presses;
	int _current_shake_pos;
	int _new_shake_pos;
	DirtyRect _ds[MAX_NUMBER_OF_DIRTY_RECTS];
	int _num_of_dirty_rects;

	MouseState _oldMouseState, _curMouseState;
	byte *_ms_buf;
	bool _mouse_visible;
	bool _mouse_state_changed;
	byte _mouseKeycolor;

	uint32 _timer_duration, _timer_next_expiry;
	bool _timer_active;
	int (*_timer_callback) (int);
};

typedef struct {
	OSystem::SoundProc sound_proc;
	void *param;
} THREAD_PARAM;
