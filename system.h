/* Interface to what's below ScummVM */

class OSystem {
public:
	typedef int ThreadProc(void *param);
	typedef void SoundProc(void *param, int16 *buf, int len);

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
//		EVENT_KEYUP = 2,
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
		PARAM_TOGGLE_FULLSCREEN = 1,
		PARAM_WINDOW_CAPTION = 2,
		PARAM_OPEN_CD = 3,
		PARAM_HOTSWAP_GFX_MODE = 4,
		PARAM_SHOW_DEFAULT_CURSOR = 5,
	};

	enum {
		SOUND_NONE = 0,
		SOUND_8BIT = 1,
		SOUND_16BIT = 2,
	};
	
	// Set colors of the palette
	virtual void set_palette(const byte *colors, uint start, uint num) = 0;

	// Set the size of the video bitmap.
	// Typically, 320x200
	virtual void init_size(uint w, uint h, byte sound) = 0;

	// Draw a bitmap to screen.
	// The screen will not be updated to reflect the new bitmap
	virtual void copy_rect(const byte *buf, int pitch, int x, int y, int w, int h) = 0;

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
	virtual void set_sound_proc(void *param, SoundProc *proc) = 0;
	
	virtual uint32 set_param(int param, uint32 value) = 0;
		
	// Quit
	virtual void quit() = 0;
};


/* Factory functions. This means we don't have to include the
 * OSystem_SDL header file. (which in turn would require the SDL headers)
 */

/* OSystem_SDL */
OSystem *OSystem_SDL_create(int gfx_driver, bool full_screen);

enum {
	GFX_NORMAL = 0,
	GFX_DOUBLESIZE = 1,
	GFX_TRIPLESIZE = 2,
	GFX_2XSAI = 3,
	GFX_SUPER2XSAI = 4,
	GFX_SUPEREAGLE = 5,
};


/* Graphics drivers */
enum {
	GD_AUTO = 0,
	GD_SDL = 1,
	GD_WIN32 = 2,
	GD_X = 3,
};



