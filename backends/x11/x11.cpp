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

/* The bare pure X11 port done by Lionel 'BBrox' Ulmer */

#include "backends/intern.h"
#include "common/util.h"
#include "base/engine.h"	// Only #included for error() and warning()

#include <stdio.h>
#include <assert.h>

#include <sys/time.h>
#include <unistd.h>

#include <sys/ipc.h>
#include <sys/shm.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/XShm.h>
#include <linux/soundcard.h>

#include <sched.h>
#include <pthread.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>

class OSystem_X11:public OSystem {
public:
	// Set colors of the palette
	void set_palette(const byte *colors, uint start, uint num);

	// Set the size of the video bitmap.
	// Typically, 320x200
	void init_size(uint w, uint h);

	// Draw a bitmap to screen.
	// The screen will not be updated to reflect the new bitmap
	void copy_rect(const byte *buf, int pitch, int x, int y, int w, int h);

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

	// Set function that generates samples 
	bool set_sound_proc(SoundProc proc, void *param, SoundFormat format);
	
	void clear_sound_proc();
	
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

	// Add a callback timer
	void set_timer(TimerProc callback, int interval);

	// Mutex handling
	MutexRef create_mutex();
	void lock_mutex(MutexRef mutex);
	void unlock_mutex(MutexRef mutex);
	void delete_mutex(MutexRef mutex);

        // Overlay handling for the new menu system
	void show_overlay();
	void hide_overlay();
	void clear_overlay();
	void grab_overlay(int16 *, int);
	void copy_rect_overlay(const int16 *, int, int, int, int, int);
	virtual int16 get_height();
	virtual int16 get_width();


	static OSystem *create(int gfx_mode, bool full_screen);

private:
	  OSystem_X11();

	typedef struct {
		int x, y, w, h;
	} dirty_square;

	void create_empty_cursor();
	void draw_mouse(dirty_square *dout);
	void undraw_mouse();
	void update_screen_helper(const dirty_square * d, dirty_square * dout);
	void blit_convert(const dirty_square * d, uint16 *dst, int pitch);

	uint8 *local_fb;
	uint16 *local_fb_overlay;
	bool _overlay_visible;

	int window_width, window_height;
	int fb_width, fb_height;
	int scumm_x, scumm_y;

	uint16 *palette;
	bool _palette_changed;
	Display *display;
	int screen;
	Window window;
	GC black_gc;
	XImage *image;
	pthread_t sound_thread;

	int fake_right_mouse;
	int report_presses;
	int current_shake_pos;
	int new_shake_pos;
	struct timeval start_time;

	enum {
		MAX_NUMBER_OF_DIRTY_SQUARES = 32
	};
	dirty_square ds[MAX_NUMBER_OF_DIRTY_SQUARES];
	int num_of_dirty_square;

	typedef struct {
		int x, y;
		int w, h;
		int hot_x, hot_y;
	} mouse_state;
	mouse_state old_state, cur_state;
	const byte *_ms_buf;
	bool _mouse_visible;
	bool _mouse_state_changed;

	uint32 _timer_duration, _timer_next_expiry;
	bool _timer_active;
	int (*_timer_callback) (int);
};

typedef struct {
	OSystem::SoundProc sound_proc;
	void *param;
	byte format;
} THREAD_PARAM;

#undef CAPTURE_SOUND

#define FRAG_SIZE 4096
static void *sound_and_music_thread(void *params)
{
	/* Init sound */
	int sound_fd, param, frag_size;
	uint8 sound_buffer[FRAG_SIZE];
	OSystem::SoundProc sound_proc = ((THREAD_PARAM *) params)->sound_proc;
	void *proc_param = ((THREAD_PARAM *) params)->param;

#ifdef CAPTURE_SOUND
	FILE *f = fopen("sound.raw", "wb");
#endif

	sound_fd = open("/dev/dsp", O_WRONLY);
	audio_buf_info info;
	if (sound_fd < 0) {
		error("Error opening sound device !\n");
		exit(1);
	}
	param = 0;
	frag_size = FRAG_SIZE /* audio fragment size */ ;
	while (frag_size) {
		frag_size >>= 1;
		param++;
	}
	param--;
	param |= /* audio_fragment_num */ 3 << 16;
	if (ioctl(sound_fd, SNDCTL_DSP_SETFRAGMENT, &param) != 0) {
		error("Error in the SNDCTL_DSP_SETFRAGMENT ioctl !\n");
		exit(1);
	}
	param = AFMT_S16_LE;
	if (ioctl(sound_fd, SNDCTL_DSP_SETFMT, &param) == -1) {
		perror("Error in the SNDCTL_DSP_SETFMT ioctl !\n");
		exit(1);
	}
	if (param != AFMT_S16_LE) {
		error("AFMT_S16_LE not supported !\n");
		exit(1);
	}
	param = 2;
	if (ioctl(sound_fd, SNDCTL_DSP_CHANNELS, &param) == -1) {
		error("Error in the SNDCTL_DSP_CHANNELS ioctl !\n");
		exit(1);
	}
	if (param != 2) {
		error("Stereo mode not supported !\n");
		exit(1);
	}
	param = SAMPLES_PER_SEC;
	if (ioctl(sound_fd, SNDCTL_DSP_SPEED, &param) == -1) {
		perror("Error in the SNDCTL_DSP_SPEED ioctl !\n");
		exit(1);
	}
	if (param != SAMPLES_PER_SEC) {
		error("%d kHz not supported !\n", SAMPLES_PER_SEC);
		exit(1);
	}
	if (ioctl(sound_fd, SNDCTL_DSP_GETOSPACE, &info) != 0) {
		perror("SNDCTL_DSP_GETOSPACE");
		exit(-1);
	}

	sched_yield();
	while (1) {
		uint8 *buf = (uint8 *)sound_buffer;
		int size, written;

		sound_proc(proc_param, (byte *)sound_buffer, FRAG_SIZE);
#ifdef CAPTURE_SOUND
		fwrite(buf, 2, FRAG_SIZE >> 1, f);
		fflush(f);
#endif
		size = FRAG_SIZE;
		while (size > 0) {
			written = write(sound_fd, buf, size);
			buf += written;
			size -= written;
		}
	}

	return NULL;
}

/* Function used to hide the mouse cursor */
void OSystem_X11::create_empty_cursor()
{
	XColor bg;
	Pixmap pixmapBits;
	Cursor cursor = None;
	static const char data[] = { 0 };

	bg.red = bg.green = bg.blue = 0x0000;
	pixmapBits = XCreateBitmapFromData(display, XRootWindow(display, screen), data, 1, 1);
	if (pixmapBits) {
		cursor = XCreatePixmapCursor(display, pixmapBits, pixmapBits, &bg, &bg, 0, 0);
		XFreePixmap(display, pixmapBits);
	}
	XDefineCursor(display, window, cursor);
}

OSystem *OSystem_X11_create()
{
	return OSystem_X11::create(0, 0);
}

OSystem *OSystem_X11::create(int gfx_mode, bool full_screen)
{
	OSystem_X11 *syst = new OSystem_X11();
	return syst;
}

OSystem_X11::OSystem_X11()
{
	char buf[512];
	XWMHints *wm_hints;
	XGCValues values;
	XTextProperty window_name;
	char *name = (char *)&buf;

	/* Some members initialization */
	fake_right_mouse = 0;
	report_presses = 1;
	current_shake_pos = 0;
	new_shake_pos = 0;
	_palette_changed = false;
	num_of_dirty_square = 0;
	_overlay_visible = false;
	_mouse_state_changed = true;
	_mouse_visible = true;
	cur_state.x = 0;
	cur_state.y = 0;
	cur_state.hot_x = 0;
	cur_state.hot_y = 0;
	cur_state.w = 0;
	cur_state.h = 0;
	
	/* For the window title */
	sprintf(buf, "ScummVM");

	display = XOpenDisplay(NULL);
	if (display == NULL) {
		error("Could not open display !\n");
		exit(1);
	}
	screen = DefaultScreen(display);

	window_width = 320;
	window_height = 200;
	scumm_x = 0;
	scumm_y = 0;
	window = XCreateSimpleWindow(display, XRootWindow(display, screen), 0, 0, 320, 200, 0, 0, 0);
	wm_hints = XAllocWMHints();
	if (wm_hints == NULL) {
		error("Not enough memory to allocate Hints !\n");
		exit(1);
	}
	wm_hints->flags = InputHint | StateHint;
	wm_hints->input = True;
	wm_hints->initial_state = NormalState;
	XStringListToTextProperty(&name, 1, &window_name);
	XSetWMProperties(display, window, &window_name, &window_name,
	                 NULL /* argv */ , 0 /* argc */ , NULL /* size hints */ ,
	                 wm_hints, NULL /* class hints */ );
	XFree(wm_hints);

	XSelectInput(display, window,
	             ExposureMask | KeyPressMask | KeyReleaseMask |
	             PointerMotionMask | ButtonPressMask | ButtonReleaseMask | StructureNotifyMask);

	values.foreground = BlackPixel(display, screen);
	black_gc = XCreateGC(display, window, GCForeground, &values);

	XMapWindow(display, window);
	XFlush(display);

	while (1) {
		XEvent event;
		XNextEvent(display, &event);
		switch (event.type) {
		case Expose:
			goto out_of_loop;
		}
	}
out_of_loop:
	create_empty_cursor();

	/* Initialize the timer routines */
	_timer_active = false;

	/* And finally start the local timer */
	gettimeofday(&start_time, NULL);
}

uint32 OSystem_X11::get_msecs()
{
	struct timeval current_time;
	gettimeofday(&current_time, NULL);
	return (uint32)(((current_time.tv_sec - start_time.tv_sec) * 1000) +
	                ((current_time.tv_usec - start_time.tv_usec) / 1000));
}

void OSystem_X11::init_size(uint w, uint h)
{
	static XShmSegmentInfo shminfo;

	fb_width = w;
	fb_height = h;

	if ((fb_width != 320) || (fb_height != 200)) {
		/* We need to change the size of the X11 window */
		XWindowChanges new_values;

		new_values.width = fb_width;
		new_values.height = fb_height;

		XConfigureWindow(display, window, CWWidth | CWHeight, &new_values);
	}
	image =	XShmCreateImage(display, DefaultVisual(display, screen), 16, ZPixmap, NULL, &shminfo, fb_width, fb_height);
	shminfo.shmid = shmget(IPC_PRIVATE, fb_width * fb_height * 2, IPC_CREAT | 0700);
	shminfo.shmaddr = (char *)shmat(shminfo.shmid, 0, 0);
	image->data = shminfo.shmaddr;
	shminfo.readOnly = False;
	if (XShmAttach(display, &shminfo) == 0) {
		error("Could not attach shared memory segment !\n");
		exit(1);
	}
	shmctl(shminfo.shmid, IPC_RMID, 0);

	/* Initialize the 'local' frame buffer and the palette */
	local_fb = (uint8 *)calloc(fb_width * fb_height, sizeof(uint8));
	local_fb_overlay = (uint16 *)calloc(fb_width * fb_height, sizeof(uint16));
	palette = (uint16 *)calloc(256, sizeof(uint16));
}

bool OSystem_X11::set_sound_proc(SoundProc proc, void *param, SoundFormat format)
{
	static THREAD_PARAM thread_param;

	/* And finally start the music thread */
	thread_param.param = param;
	thread_param.sound_proc = proc;
	thread_param.format = format;

	if (format == SOUND_16BIT)
		pthread_create(&sound_thread, NULL, sound_and_music_thread, (void *)&thread_param);
	else
		warning("Only support 16 bit sound for now. Disabling sound ");

	return true;
}

void OSystem_X11::clear_sound_proc() {
	// TODO implement this...
	// The sound_thread has to be stopped in a nice way. In particular,
	// using pthread_kill would be a bad idea. Rather, use pthread_cancel,
	// or maybe a global variable, to achieve this.
	// This method shouldn't return until the sound thread really has stopped.
}


void OSystem_X11::set_palette(const byte *colors, uint start, uint num)
{
	const byte *data = colors;
	uint16 *pal = &(palette[start]);

	do {
		*pal++ = ((data[0] & 0xF8) << 8) | ((data[1] & 0xFC) << 3) | (data[2] >> 3);
		data += 4;
		num--;
	} while (num > 0);

	_palette_changed = true;
}

#define AddDirtyRec(xi,yi,wi,hi) 				\
  if (num_of_dirty_square < MAX_NUMBER_OF_DIRTY_SQUARES) {	\
    ds[num_of_dirty_square].x = xi;				\
    ds[num_of_dirty_square].y = yi;				\
    ds[num_of_dirty_square].w = wi;				\
    ds[num_of_dirty_square].h = hi;				\
    num_of_dirty_square++;					\
  }

void OSystem_X11::copy_rect(const byte *buf, int pitch, int x, int y, int w, int h)
{
	uint8 *dst;

	if (y < 0) {
		h += y;
		buf -= y * pitch;
		y = 0;
	}
	if (h > (fb_height - y)) {
		h = fb_height - y;
	}

	dst = local_fb + fb_width * y + x;

	if (h <= 0)
		return;

	AddDirtyRec(x, y, w, h);
	while (h-- > 0) {
		memcpy(dst, buf, w);
		dst += fb_width;
		buf += pitch;
	}
}

void OSystem_X11::move_screen(int dx, int dy, int height) {

	if ((dx == 0) && (dy == 0))
		return;

	if (dx == 0) {
		// vertical movement
		if (dy > 0) {
			// move down
			// copy from bottom to top
			for (int y = height - 1; y >= dy; y--)
				copy_rect(local_fb + fb_width * (y - dy), fb_width, 0, y, fb_width, 1);
		} else {
			// move up
			// copy from top to bottom
			for (int y = 0; y < height + dx; y++)
				copy_rect(local_fb + fb_width * (y - dy), fb_width, 0, y, fb_width, 1);
		}
	} else if (dy == 0) {
		// horizontal movement
		if (dx > 0) {
			// move right
			// copy from right to left
			for (int x = fb_width - 1; x >= dx; x--)
				copy_rect(local_fb + x - dx, fb_width, x, 0, 1, height);
		} else {
			// move left
			// copy from left to right
			for (int x = 0; x < fb_width; x++)
				copy_rect(local_fb + x - dx, fb_width, x, 0, 1, height);
		}
	} else {
		// free movement
		// not neccessary for now
	}
}

void OSystem_X11::blit_convert(const dirty_square * d, uint16 *dst, int pitch)
{
	uint8 *ptr_src  = local_fb + (fb_width * d->y) + d->x;
	uint16 *ptr_dst = dst + (fb_width * d->y) + d->x;
	int x, y;

	for (y = 0; y < d->h; y++) {
		for (x = 0; x < d->w; x++) {
			*ptr_dst++ = palette[*ptr_src++];
		}
		ptr_dst += pitch - d->w;
		ptr_src += fb_width - d->w;
	}
}

void OSystem_X11::update_screen_helper(const dirty_square * d, dirty_square * dout)
{
	if (_overlay_visible == false) {
		blit_convert(d, (uint16 *) image->data, fb_width);
	} else {
		uint16 *ptr_src = local_fb_overlay + (fb_width * d->y) + d->x;
		uint16 *ptr_dst = ((uint16 *)image->data) + (fb_width * d->y) + d->x;
		int y;
		
		for (y = 0; y < d->h; y++) {
			memcpy(ptr_dst, ptr_src, d->w * sizeof(uint16));
			ptr_dst += fb_width;
			ptr_src += fb_width;
		}	
	}
	if (d->x < dout->x)
		dout->x = d->x;
	if (d->y < dout->y)
		dout->y = d->y;
	if ((d->x + d->w) > dout->w)
		dout->w = d->x + d->w;
	if ((d->y + d->h) > dout->h)
		dout->h = d->y + d->h;
}

void OSystem_X11::update_screen()
{
	bool full_redraw = false;
	bool need_redraw = false;
	static const dirty_square ds_full = { 0, 0, fb_width, fb_height };
	dirty_square dout = { fb_width, fb_height, 0, 0 };
	
	if (_palette_changed) {
		full_redraw = true;
		num_of_dirty_square = 0;
		_palette_changed = false;
	} else if (num_of_dirty_square >= MAX_NUMBER_OF_DIRTY_SQUARES) {
		full_redraw = true;
		num_of_dirty_square = 0;
	}

	if (full_redraw) {
		update_screen_helper(&ds_full, &dout);
		need_redraw = true;
	} else if ((num_of_dirty_square > 0) || (_mouse_state_changed == true)) {
		need_redraw = true;
		while (num_of_dirty_square > 0) {
			num_of_dirty_square--;
			update_screen_helper(&(ds[num_of_dirty_square]), &dout);
		}
	}

	/* Then 'overlay' the mouse on the image */
	draw_mouse(&dout);

	if (current_shake_pos != new_shake_pos) {
		/* Redraw first the 'black borders' in case of resize */
		if (current_shake_pos < new_shake_pos)
			XFillRectangle(display, window, black_gc, 0, current_shake_pos, window_width, new_shake_pos);
		else
			XFillRectangle(display, window, black_gc, 0, window_height - current_shake_pos,
			               window_width, window_height - new_shake_pos);
		XShmPutImage(display, window, DefaultGC(display, screen), image,
		             0, 0, scumm_x, scumm_y + new_shake_pos, fb_width, fb_height, 0);
		current_shake_pos = new_shake_pos;
	} else if (need_redraw == true) {
		XShmPutImage(display, window, DefaultGC(display, screen), image,
		             dout.x, dout.y, scumm_x + dout.x, scumm_y + dout.y + current_shake_pos,
		             dout.w - dout.x, dout.h - dout.y, 0);
		XFlush(display);
	}
}

bool OSystem_X11::show_mouse(bool visible)
{
	if (_mouse_visible == visible)
		return visible;

	bool last = _mouse_visible;
	_mouse_visible = visible;

	if ((visible == false) && (_mouse_state_changed == false)) {
		undraw_mouse();
	}
	_mouse_state_changed = true;

	return last;
}

void OSystem_X11::quit()
{
	exit(0);
}

void OSystem_X11::undraw_mouse()
{
	AddDirtyRec(old_state.x, old_state.y, old_state.w, old_state.h);
}

void OSystem_X11::draw_mouse(dirty_square *dout)
{
	_mouse_state_changed = false;

	if (_mouse_visible == false)
		return;

	int xdraw = cur_state.x - cur_state.hot_x;
	int ydraw = cur_state.y - cur_state.hot_y;
	int w = cur_state.w;
	int h = cur_state.h;
	int real_w;
	int real_h;

	uint16 *dst;
	const byte *buf = _ms_buf;

	if (ydraw < 0) {
		real_h = h + ydraw;
		buf += (-ydraw) * w;
		ydraw = 0;
	} else {
		real_h = (ydraw + h) > fb_height ? (fb_height - ydraw) : h;
	}
	if (xdraw < 0) {
		real_w = w + xdraw;
		buf += (-xdraw);
		xdraw = 0;
	} else {
		real_w = (xdraw + w) > fb_width ? (fb_width - xdraw) : w;
	}

	dst = ((uint16 *) image->data) + (ydraw * fb_width) + xdraw;

	if ((real_h == 0) || (real_w == 0)) {
		return;
	}

	
	if (xdraw < dout->x)
		dout->x = xdraw;
	if (ydraw < dout->y)
		dout->y = ydraw;
	if ((xdraw + real_w) > dout->w)
		dout->w = xdraw + real_w;
	if ((ydraw + real_h) > dout->h)
		dout->h = ydraw + real_h;

	old_state.x = xdraw;
	old_state.y = ydraw;
	old_state.w = real_w;
	old_state.h = real_h;

	while (real_h > 0) {
		int width = real_w;
		while (width > 0) {
			byte color = *buf;
			if (color != 0xFF) {
				*dst = palette[color];
			}
			buf++;
			dst++;
			width--;
		}
		buf += w - real_w;
		dst += fb_width - real_w;
		real_h--;
	}
}

void OSystem_X11::set_mouse_pos(int x, int y)
{
	if ((x != cur_state.x) || (y != cur_state.y)) {
		cur_state.x = x;
		cur_state.y = y;
		if (_mouse_state_changed == false) {
			undraw_mouse();
		}
		_mouse_state_changed = true;
	}
}

void OSystem_X11::warp_mouse(int x, int y)
{
	set_mouse_pos(x, y);
}

void OSystem_X11::set_mouse_cursor(const byte *buf, uint w, uint h, int hotspot_x, int hotspot_y)
{
	cur_state.w = w;
	cur_state.h = h;
	cur_state.hot_x = hotspot_x;
	cur_state.hot_y = hotspot_y;
	_ms_buf = buf;

	if (_mouse_state_changed == false) {
		undraw_mouse();
	}
	_mouse_state_changed = true;
}

void OSystem_X11::set_shake_pos(int shake_pos)
{
	if (new_shake_pos != shake_pos) {
		if (_mouse_state_changed == false) {
			undraw_mouse();
		}
		_mouse_state_changed = true;
	}
	new_shake_pos = shake_pos;
}

uint32 OSystem_X11::property(int param, Property *value)
{
	switch (param) {
	case PROP_GET_SAMPLE_RATE:
		return SAMPLES_PER_SEC;
	case PROP_GET_FULLSCREEN:
		return 0;
	}
	warning("Property not implemented yet (%d) ", param);
	return 0;
}

bool OSystem_X11::poll_cdrom()
{
	return false;
}

void OSystem_X11::play_cdrom(int track, int num_loops, int start_frame, int duration)
{
}

void OSystem_X11::stop_cdrom()
{
}

void OSystem_X11::update_cdrom()
{
}

void OSystem_X11::delay_msecs(uint msecs)
{
	usleep(msecs * 1000);
}

bool OSystem_X11::poll_event(Event *scumm_event)
{
	/* First, handle timers */
	uint32 current_msecs = get_msecs();

	if (_timer_active && (current_msecs >= _timer_next_expiry)) {
		_timer_duration = _timer_callback(_timer_duration);
		_timer_next_expiry = current_msecs + _timer_duration;
	}

	while (XPending(display)) {
		XEvent event;

		XNextEvent(display, &event);
		switch (event.type) {
		case Expose:{
				int real_w, real_h;
				int real_x, real_y;
				real_x = event.xexpose.x;
				real_y = event.xexpose.y;
				real_w = event.xexpose.width;
				real_h = event.xexpose.height;
				if (real_x < scumm_x) {
					real_w -= scumm_x - real_x;
					real_x = 0;
				} else {
					real_x -= scumm_x;
				}
				if (real_y < scumm_y) {
					real_h -= scumm_y - real_y;
					real_y = 0;
				} else {
					real_y -= scumm_y;
				}
				if ((real_h <= 0) || (real_w <= 0))
					break;
				if ((real_x >= fb_width) || (real_y >= fb_height))
					break;

				if ((real_x + real_w) >= fb_width) {
					real_w = fb_width - real_x;
				}
				if ((real_y + real_h) >= fb_height) {
					real_h = fb_height - real_y;
				}

				/* Compute the intersection of the expose event with the real ScummVM display zone */
				AddDirtyRec(real_x, real_y, real_w, real_h);
			}
			break;

		case KeyPress:{
				/* I am using keycodes here and NOT keysyms to be sure that even if the user
				   remaps his iPAQ's keyboard, it will still work.
				 */
				int keycode = -1;
				int ascii = -1;
				byte mode = 0;

				if (event.xkey.state & 0x01)
					mode |= KBD_SHIFT;
				if (event.xkey.state & 0x04)
					mode |= KBD_CTRL;
				if (event.xkey.state & 0x08)
					mode |= KBD_ALT;
				switch (event.xkey.keycode) {

				case 9:								/* Escape on my PC */
				case 130:							/* Calendar on the iPAQ */
					keycode = 27;
					break;

				case 71:								/* F5 on my PC */
				case 128:							/* Record on the iPAQ */
					keycode = 319;
					break;

				case 65:								/* Space on my PC */
				case 131:							/* Schedule on the iPAQ */
					keycode = 32;
					break;

				case 132:
					report_presses = 0;
					break;
	
				case 133:
					fake_right_mouse = 1;
					break;

				default:{
						KeySym xsym;
						xsym = XKeycodeToKeysym(display, event.xkey.keycode, 0);
						keycode = xsym;
						if ((xsym >= 'a') && (xsym <= 'z') && (event.xkey.state & 0x01))
							xsym &= ~0x20;		/* Handle shifted keys */
						ascii = xsym;
					}
				}
				if (keycode != -1) {
					scumm_event->event_code = EVENT_KEYDOWN;
					scumm_event->kbd.keycode = keycode;
					scumm_event->kbd.ascii = (ascii != -1 ? ascii : keycode);
					scumm_event->kbd.flags = mode;
					return true;
				}
		}
		break;

		case KeyRelease:{
				/* I am using keycodes here and NOT keysyms to be sure that even if the user
				   remaps his iPAQ's keyboard, it will still work.
				 */
				int keycode = -1;
				int ascii = -1;
				byte mode = 0;

				if (event.xkey.state & 0x01)
					mode |= KBD_SHIFT;
				if (event.xkey.state & 0x04)
					mode |= KBD_CTRL;
				if (event.xkey.state & 0x08)
					mode |= KBD_ALT;
				switch (event.xkey.keycode) {
				case 132:							/* 'Q' on the iPAQ */
					report_presses = 1;
					break;

				case 133:							/* Arrow on the iPAQ */
					fake_right_mouse = 0;
					break;

				default:{
						KeySym xsym;
						xsym = XKeycodeToKeysym(display, event.xkey.keycode, 0);
						keycode = xsym;
						if ((xsym >= 'a') && (xsym <= 'z') && (event.xkey.state & 0x01))
							xsym &= ~0x20;		/* Handle shifted keys */
						ascii = xsym;
					}
				}
				if (keycode != -1) {
					scumm_event->event_code = EVENT_KEYUP;
					scumm_event->kbd.keycode = keycode;
					scumm_event->kbd.ascii = (ascii != -1 ? ascii : keycode);
					scumm_event->kbd.flags = mode;
					return true;
				}
			}
			break;

		case ButtonPress:
			if (report_presses != 0) {
				if (event.xbutton.button == 1) {
					if (fake_right_mouse == 0) {
						scumm_event->event_code = EVENT_LBUTTONDOWN;
					} else {
						scumm_event->event_code = EVENT_RBUTTONDOWN;
					}
				} else if (event.xbutton.button == 3)
					scumm_event->event_code = EVENT_RBUTTONDOWN;
				scumm_event->mouse.x = event.xbutton.x - scumm_x;
				scumm_event->mouse.y = event.xbutton.y - scumm_y;
				return true;
			}
			break;

		case ButtonRelease:
			if (report_presses != 0) {
				if (event.xbutton.button == 1) {
					if (fake_right_mouse == 0) {
						scumm_event->event_code = EVENT_LBUTTONUP;
					} else {
						scumm_event->event_code = EVENT_RBUTTONUP;
					}
				} else if (event.xbutton.button == 3)
					scumm_event->event_code = EVENT_RBUTTONUP;
				scumm_event->mouse.x = event.xbutton.x - scumm_x;
				scumm_event->mouse.y = event.xbutton.y - scumm_y;
				return true;
			}
			break;

		case MotionNotify:
			scumm_event->event_code = EVENT_MOUSEMOVE;
			scumm_event->mouse.x = event.xmotion.x - scumm_x;
			scumm_event->mouse.y = event.xmotion.y - scumm_y;
			set_mouse_pos(scumm_event->mouse.x, scumm_event->mouse.y);
			return true;

		case ConfigureNotify:{
				if ((window_width != event.xconfigure.width) || (window_height != event.xconfigure.height)) {
					window_width = event.xconfigure.width;
					window_height = event.xconfigure.height;
					scumm_x = (window_width - fb_width) / 2;
					scumm_y = (window_height - fb_height) / 2;
					XFillRectangle(display, window, black_gc, 0, 0, window_width, window_height);
				}
			}
			break;

		default:
			printf("Unhandled event : %d\n", event.type);
			break;
		}
	}

	return false;
}

void OSystem_X11::set_timer(TimerProc callback, int interval)
{
	if (callback != NULL) {
		_timer_duration = timer;
		_timer_next_expiry = get_msecs() + timer;
		_timer_callback = callback;
		_timer_active = true;
	} else {
		_timer_active = false;
	}
}

OSystem::MutexRef OSystem_X11::create_mutex(void)
{
	pthread_mutex_t *mutex = (pthread_mutex_t *) malloc(sizeof(pthread_mutex_t));
	pthread_mutex_init(mutex, NULL);
	return (MutexRef)mutex;
}

void OSystem_X11::lock_mutex(MutexRef mutex)
{
	pthread_mutex_lock((pthread_mutex_t *) mutex);
}

void OSystem_X11::unlock_mutex(MutexRef mutex)
{
	pthread_mutex_unlock((pthread_mutex_t *) mutex);
}

void OSystem_X11::delete_mutex(MutexRef mutex)
{
	pthread_mutex_destroy((pthread_mutex_t *) mutex);
	free(mutex);
}

void OSystem_X11::show_overlay() {
	_overlay_visible = true;
}

void OSystem_X11::hide_overlay() {
	_overlay_visible = false;
	_palette_changed = true; // This is to force a full redraw to hide the overlay
}

void OSystem_X11::clear_overlay() {
	if (_overlay_visible == false)
		return;
	dirty_square d = { 0, 0, fb_width, fb_height };
	AddDirtyRec(0, 0, fb_width, fb_height);
	blit_convert(&d, local_fb_overlay, fb_width);
}

void OSystem_X11::grab_overlay(int16 *dest, int pitch) {
	if (_overlay_visible == false)
		return;
	
	dirty_square d = { 0, 0, fb_width, fb_height };
	blit_convert(&d, (uint16 *) dest, pitch);
}

void OSystem_X11::copy_rect_overlay(const int16 *src, int pitch, int x, int y, int w, int h) {
	if (_overlay_visible == false)
		return;
	uint16 *dst = local_fb_overlay + x + (y * fb_width);
	AddDirtyRec(x, y, w, h);
	while (h > 0) {
		memcpy(dst, src, w * sizeof(*dst));
		dst += fb_width;
		src += pitch;
		h--;
	}
}

int16 OSystem_X11::get_height() {
	return fb_height;
}

int16 OSystem_X11::get_width() {
	return fb_width;
}

