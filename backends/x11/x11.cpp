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

/* The bare pure X11 port done by Lionel 'BBrox' Ulmer */

#include "common/stdafx.h"
#include "common/scummsys.h"
#include "common/system.h"
#include "common/util.h"

#include "backends/intern.h"

#include <stdio.h>
#include <assert.h>

#include <sys/time.h>
#include <unistd.h>

#include <sys/ipc.h>
#include <sys/shm.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/XShm.h>

#ifdef __linux__
#include <linux/soundcard.h>
#else
#include <sys/soundcard.h>
#endif

#include <sched.h>
#include <pthread.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <backends/x11/x11.h>

int main(int argc, char *argv[]) {
	g_system = OSystem_X11::create(0, 0);
	assert(g_system);

	// Invoke the actual ScummVM main entry point:
	int res = scummvm_main(argc, argv);
	g_system->quit();	// TODO: Consider removing / replacing this!
	return res;
}

OSystem *OSystem_X11::create(int gfx_mode, bool full_screen) {
	OSystem_X11 *syst = new OSystem_X11();
	return syst;
}

OSystem_X11::OSystem_X11() {
	/* Some members initialization */
	_fake_right_mouse = 0;
	_report_presses = 1;
	_current_shake_pos = 0;
	_new_shake_pos = 0;
	_palette_changed = false;
	_num_of_dirty_rects = 0;
	_overlay_visible = false;
	_mouse_state_changed = true;
	_mouse_visible = true;
	_ms_buf = NULL;
	_curMouseState.x = 0;
	_curMouseState.y = 0;
	_curMouseState.hot_x = 0;
	_curMouseState.hot_y = 0;
	_curMouseState.w = 0;
	_curMouseState.h = 0;
	_palette16 = 0;
	_palette32 = 0;
	_bytesPerPixel = 0;
	_image = 0;
	_local_fb = 0;
	_local_fb_overlay = 0;
}

OSystem_X11::~OSystem_X11() {
	XFree(_image);
	if (_palette16)
		free(_palette16);

	if (_palette32)
		free(_palette32);

	if (_ms_buf)
		free(_ms_buf);

	free(_local_fb_overlay);
	free(_local_fb);
}

void OSystem_X11::initBackend() {
	char buf[512];
	XWMHints *wm_hints;
	XGCValues values;
	XTextProperty window_name;
	char *name = (char *)&buf;
	/* For the_window title */
	sprintf(buf, "ScummVM");

	_display = XOpenDisplay(NULL);
	if (_display == NULL) {
		error("Could not open display !\n");
		exit(1);
	}

	if (XShmQueryExtension(_display)!=True)
		error("No Shared Memory Extension present");

	_screen = DefaultScreen(_display);
	_depth = DefaultDepth(_display, _screen);
	switch (_depth) {
		case 16 :
			_bytesPerPixel = 2;
			break;
		case 24 :
		case 32 :
			_bytesPerPixel = 4;
			break;
	}

	if (!_bytesPerPixel)
		error("Your screen depth is %ibit. Values other than 16, 24 and 32bit are currently not supported", _depth);

	_window_width = 320;
	_window_height = 200;
	_scumm_x = 0;
	_scumm_y = 0;
	_window = XCreateSimpleWindow(_display, XRootWindow(_display, _screen), 0, 0, 320, 200, 0, 0, 0);
	wm_hints = XAllocWMHints();
	if (wm_hints == NULL) {
		error("Not enough memory to allocate Hints !\n");
		exit(1);
	}
	wm_hints->flags = InputHint | StateHint;
	wm_hints->input = True;
	wm_hints->initial_state = NormalState;
	XStringListToTextProperty(&name, 1, &window_name);
	XSetWMProperties(_display, _window, &window_name, &window_name,
	                 NULL /* argv */ , 0 /* argc */ , NULL /* size hints */ ,
	                 wm_hints, NULL /* class hints */ );
	XFree(wm_hints);

	XSelectInput(_display, _window,
	             ExposureMask | KeyPressMask | KeyReleaseMask |
	             PointerMotionMask | ButtonPressMask | ButtonReleaseMask | StructureNotifyMask);

	values.foreground = BlackPixel(_display, _screen);
	_black_gc = XCreateGC(_display, _window, GCForeground, &values);

	XMapWindow(_display, _window);
	XFlush(_display);

	_fb_width = 0;
	_fb_height = 0;

	if (!_palette16)
		_palette16 = (uint16 *)calloc(256, sizeof(uint16));
	if (!_palette32 && _bytesPerPixel == 4)
	_palette32 = (uint32 *)calloc(256, sizeof(uint32));

	while (1) {
		XEvent event;
		XNextEvent(_display, &event);
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
	gettimeofday(&_start_time, NULL);

}

#undef CAPTURE_SOUND
#define FRAG_SIZE 4096

static void *sound_and_music_thread(void *params) {
	/* Init sound */
	int sound_fd, param, frag_size;
	uint8 sound_buffer[FRAG_SIZE];
	OSystem::SoundProc sound_proc = ((THREAD_PARAM *)params)->sound_proc;
	void *proc_param = ((THREAD_PARAM *)params)->param;

#ifdef CAPTURE_SOUND
	FILE *f = fopen("sound.raw", "wb");
#endif

	sound_fd = open("/dev/dsp", O_WRONLY);
	audio_buf_info info;
	if (sound_fd < 0) {
		warning("Error opening sound device!\n");
		return NULL;
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
		warning("Error in the SNDCTL_DSP_SETFRAGMENT ioctl!\n");
		return NULL;
	}
	param = AFMT_S16_LE;
	if (ioctl(sound_fd, SNDCTL_DSP_SETFMT, &param) == -1) {
		warning("Error in the SNDCTL_DSP_SETFMT ioctl!\n");
		return NULL;;
	}
	if (param != AFMT_S16_LE) {
		warning("AFMT_S16_LE not supported!\n");
		return NULL;
	}
	param = 2;
	if (ioctl(sound_fd, SNDCTL_DSP_CHANNELS, &param) == -1) {
		warning("Error in the SNDCTL_DSP_CHANNELS ioctl!\n");
		return NULL;
	}
	if (param != 2) {
		warning("Stereo mode not supported!\n");
		return NULL;
	}
	param = SAMPLES_PER_SEC;
	if (ioctl(sound_fd, SNDCTL_DSP_SPEED, &param) == -1) {
		warning("Error in the SNDCTL_DSP_SPEED ioctl!\n");
		return NULL;
	}
	if (param != SAMPLES_PER_SEC) {
		warning("%d kHz not supported!\n", SAMPLES_PER_SEC);
		return NULL;
	}
	if (ioctl(sound_fd, SNDCTL_DSP_GETOSPACE, &info) != 0) {
		warning("SNDCTL_DSP_GETOSPACE");
		return NULL;
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
void OSystem_X11::create_empty_cursor() {
	XColor bg;
	Pixmap pixmapBits;
	Cursor cursor = None;
	static const char data[] = { 0 };

	bg.red = bg.green = bg.blue = 0x0000;
	pixmapBits = XCreateBitmapFromData(_display, XRootWindow(_display, _screen), data, 1, 1);
	if (pixmapBits) {
		cursor = XCreatePixmapCursor(_display, pixmapBits, pixmapBits, &bg, &bg, 0, 0);
		XFreePixmap(_display, pixmapBits);
	}
	XDefineCursor(_display, _window, cursor);
}

bool OSystem_X11::hasFeature(Feature f) {
	return false;
}

void OSystem_X11::setFeatureState(Feature f, bool enable) {
}

bool OSystem_X11::getFeatureState(Feature f) {
	return false;
}

const OSystem::GraphicsMode *OSystem_X11::getSupportedGraphicsModes() const {
	static const OSystem::GraphicsMode mode = {"1x", "Normal mode", 0};
	return &mode;
}

int OSystem_X11::getDefaultGraphicsMode() const {
	return 0;
}

bool OSystem_X11::setGraphicsMode(int mode) {
	return (mode == 0);
}

int OSystem_X11::getGraphicsMode() const {
	return 0;
}


uint32 OSystem_X11::getMillis() {
	struct timeval current_time;
	gettimeofday(&current_time, NULL);
	return (uint32)(((current_time.tv_sec - _start_time.tv_sec) * 1000) +
	                ((current_time.tv_usec - _start_time.tv_usec) / 1000));
}

void OSystem_X11::initSize(uint w, uint h, int overlaySize) {
	//debug("initSize(%i, %i, %i)", w, h, overlaySize);
	static XShmSegmentInfo shminfo;

	if (((uint)_fb_width != w) || ((uint)_fb_height != w)) {
		_fb_width = w;
		_fb_height = h;

		/* We need to change the size of the X11_window */
		XWindowChanges new_values;

		new_values.width = _fb_width;
		new_values.height = _fb_height;

		XConfigureWindow(_display,_window, CWWidth | CWHeight, &new_values);

		if (_image)
			XFree(_image);
		_image = XShmCreateImage(_display, DefaultVisual(_display, _screen), _depth, ZPixmap, NULL, &shminfo,_fb_width,_fb_height);
		if (!_image)
			error("Couldn't get image by XShmCreateImage()");

		shminfo.shmid = shmget(IPC_PRIVATE, _image->bytes_per_line * _image->height, IPC_CREAT | 0700);
		if (shminfo.shmid < 0)
			error("Couldn't allocate image data by shmget()");

		_image->data = shminfo.shmaddr = (char *)shmat(shminfo.shmid, 0, 0);
		shminfo.readOnly = False;
		if (XShmAttach(_display, &shminfo) == 0) {
			error("Could not attach shared memory segment !\n");
			exit(1);
		}
		shmctl(shminfo.shmid, IPC_RMID, 0);

		if (_local_fb)
			free(_local_fb);
		if (_local_fb_overlay)
			free(_local_fb_overlay);	
		/* Initialize the 'local' frame buffer and the palette */
		_local_fb = (uint8 *)calloc(_fb_width * _fb_height, sizeof(uint8));
		_local_fb_overlay = (uint16 *)calloc(_fb_width * _fb_height, sizeof(uint16));

	}
}

bool OSystem_X11::setSoundCallback(SoundProc proc, void *param) {
	static THREAD_PARAM thread_param;

	/* And finally start the music thread */
	thread_param.param = param;
	thread_param.sound_proc = proc;

	pthread_create(&_sound_thread, NULL, sound_and_music_thread, (void *)&thread_param);

	return true;
}

void OSystem_X11::clearSoundCallback() {
	// TODO implement this...
	// The sound_thread has to be stopped in a nice way. In particular,
	// using pthread_kill would be a bad idea. Rather, use pthread_cancel,
	// or maybe a global variable, to achieve this.
	// This method shouldn't return until the sound thread really has stopped.
}


void OSystem_X11::setPalette(const byte *colors, uint start, uint num) {
	uint16 *pal = &(_palette16[start]);
	const byte *data = colors;

	if (_bytesPerPixel == 4) {
			for (uint i = start; i < start+num; i++) {
				//_palette32[i] = ((uint32 *)colors)[i];
				_palette32[i] = (colors[i * 4 + 0] << 16) | (colors[i * 4 + 1] << 8) | (colors[i * 4 + 2] << 0);
			}
	}

	do {
		*pal++ = ((data[0] & 0xF8) << 8) | ((data[1] & 0xFC) << 3) | (data[2] >> 3);
		data += 4;
		num--;
	} while (num > 0);

	_palette_changed = true;
}

#define AddDirtyRec(xi,yi,wi,hi) 				\
  if (_num_of_dirty_rects < MAX_NUMBER_OF_DIRTY_RECTS) {	\
    _ds[_num_of_dirty_rects].x = xi;				\
    _ds[_num_of_dirty_rects].y = yi;				\
    _ds[_num_of_dirty_rects].w = wi;				\
    _ds[_num_of_dirty_rects].h = hi;				\
    _num_of_dirty_rects++;					\
  }

void OSystem_X11::copyRectToScreen(const byte *buf, int pitch, int x, int y, int w, int h) {
	uint8 *dst;

	if (y < 0) {
		h += y;
		buf -= y * pitch;
		y = 0;
	}
	if (h > (_fb_height - y)) {
		h = _fb_height - y;
	}

	dst = _local_fb + _fb_width * y + x;

	if (h <= 0)
		return;

	AddDirtyRec(x, y, w, h);
	while (h-- > 0) {
		memcpy(dst, buf, w);
		dst +=_fb_width;
		buf += pitch;
	}
}

void OSystem_X11::blit(const DirtyRect *d, uint16 *dst, int pitch) {
	uint8 *ptr_src = _local_fb + (_fb_width * d->y) + d->x;
	uint16 *ptr_dst = dst + ((_fb_width * d->y) + d->x);
	int x, y;

	for (y = 0; y < d->h; y++) {
		for (x = 0; x < d->w; x++) {
			*ptr_dst++ = _palette16[*ptr_src++];
		}
		ptr_dst += pitch - d->w;
		ptr_src +=_fb_width - d->w;
	}
}

void OSystem_X11::blit_convert(const DirtyRect *d, uint8 *dst, int pitch) {
	uint8 *ptr_src = _local_fb + (_fb_width * d->y) + d->x;
	uint8 *ptr_dst = dst + ((_fb_width * d->y) + d->x) * _bytesPerPixel;
	int x, y;

	switch (_bytesPerPixel) {
		case 2:
			for (y = 0; y < d->h; y++) {
				for (x = 0; x < d->w; x++) {
					*ptr_dst = _palette16[*ptr_src++];
					ptr_dst += _bytesPerPixel;
				}
				ptr_dst += (pitch - d->w) * _bytesPerPixel;
				ptr_src +=_fb_width - d->w;
			}
			break;
		case 4:
			for (y = 0; y < d->h; y++) {
				for (x = 0; x < d->w; x++) {
					*(uint32 *)ptr_dst = _palette32[*ptr_src];
					ptr_dst += _bytesPerPixel;
					ptr_src++;
				}
				ptr_dst += (pitch - d->w) * _bytesPerPixel;
				ptr_src += _fb_width - d->w;
			}
	}
}

void OSystem_X11::updateScreen_helper(const DirtyRect *d, DirtyRect *dout) {

	if (_overlay_visible == false) {
		blit_convert(d, (uint8 *)_image->data, _fb_width);
	} else {
		uint16 *ptr_src = _local_fb_overlay + (_fb_width * d->y) + d->x;
		uint8 *ptr_dst = (uint8 *)_image->data + ((_fb_width * d->y) + d->x) * _bytesPerPixel;

		int y;

		switch (_bytesPerPixel) {
				case 2:
					for (y = 0; y < d->h; y++) {
						memcpy(ptr_dst, ptr_src, d->w * sizeof(uint16));
						ptr_dst += _fb_width * sizeof(uint16);
						ptr_src += _fb_width;
					}
					break;
				case 4:
					uint16 currLine, x;
					register uint16 currPixel;
					for (y = d->y; y < d->y + d->h; y++) {
						currLine = y * _fb_width;
						for (x = d->x; x < d->x + d->w; x++) {
							currPixel = _local_fb_overlay[(currLine + x)];
							*(uint32 *)ptr_dst = ((currPixel & 0xF800) << 8) + ((currPixel & 0x07E0) << 5) + 
													((currPixel & 0x001F) << 3);
							ptr_dst += sizeof(uint32);
						}
						ptr_dst += (_fb_width - d->w) * _bytesPerPixel;
					}

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

void OSystem_X11::updateScreen() {
	bool full_redraw = false;
	bool need_redraw = false;
	static const DirtyRect ds_full = { 0, 0, _fb_width, _fb_height };
	DirtyRect dout = {_fb_width, _fb_height, 0, 0 };

	if (_palette_changed) {
		full_redraw = true;
		_num_of_dirty_rects = 0;
		_palette_changed = false;
	} else if (_num_of_dirty_rects >= MAX_NUMBER_OF_DIRTY_RECTS) {
		full_redraw = true;
		_num_of_dirty_rects = 0;
	}

	if (full_redraw) {
		updateScreen_helper(&ds_full, &dout);
		need_redraw = true;
	} else if ((_num_of_dirty_rects > 0) || (_mouse_state_changed == true)) {
		need_redraw = true;
		while (_num_of_dirty_rects > 0) {
			_num_of_dirty_rects--;
			updateScreen_helper(&(_ds[_num_of_dirty_rects]), &dout);
		}
	}

	/* Then 'overlay' the mouse on the image */
	draw_mouse(&dout);

	if (_current_shake_pos != _new_shake_pos) {
		/* Redraw first the 'black borders' in case of resize */
		if (_current_shake_pos < _new_shake_pos)
			XFillRectangle(_display,_window, _black_gc, 0, _current_shake_pos, _window_width, _new_shake_pos);
		else
			XFillRectangle(_display,_window, _black_gc, 0, _window_height - _current_shake_pos,
			              _window_width,_window_height - _new_shake_pos);
		XShmPutImage(_display, _window, DefaultGC(_display, _screen), _image,
		             0, 0, _scumm_x, _scumm_y + _new_shake_pos, _fb_width, _fb_height, 0);
		_current_shake_pos = _new_shake_pos;
	} else if (need_redraw == true) {
		XShmPutImage(_display, _window, DefaultGC(_display, _screen), _image,
		             dout.x, dout.y, _scumm_x + dout.x, _scumm_y + dout.y + _current_shake_pos,
		             dout.w - dout.x, dout.h - dout.y, 0);
		XFlush(_display);
	}
}

bool OSystem_X11::showMouse(bool visible)
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

void OSystem_X11::quit() {
	exit(0);
}

void OSystem_X11::setWindowCaption(const char *caption) {
	//debug("setWindowCaption('%s')", caption);
}

void OSystem_X11::undraw_mouse() {
	AddDirtyRec(_oldMouseState.x, _oldMouseState.y, _oldMouseState.w, _oldMouseState.h);
}

void OSystem_X11::draw_mouse(DirtyRect *dout) {
	//debug("draw_mouse()");
	_mouse_state_changed = false;

	if (_mouse_visible == false)
		return;

	int xdraw = _curMouseState.x - _curMouseState.hot_x;
	int ydraw = _curMouseState.y - _curMouseState.hot_y;
	int w = _curMouseState.w;
	int h = _curMouseState.h;
	int real_w;
	int real_h;

	uint8 *dst;
	const byte *buf = _ms_buf;

	if (ydraw < 0) {
		real_h = h + ydraw;
		buf += (-ydraw) * w;
		ydraw = 0;
	} else {
		real_h = (ydraw + h) > _fb_height ? (_fb_height - ydraw) : h;
	}
	if (xdraw < 0) {
		real_w = w + xdraw;
		buf += (-xdraw);
		xdraw = 0;
	} else {
		real_w = (xdraw + w) > _fb_width ? (_fb_width - xdraw) : w;
	}

	dst = (uint8 *)_image->data + ((ydraw *_fb_width) + xdraw) * _bytesPerPixel;

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

	_oldMouseState.x = xdraw;
	_oldMouseState.y = ydraw;
	_oldMouseState.w = real_w;
	_oldMouseState.h = real_h;

	while (real_h > 0) {
		int width = real_w;
		while (width > 0) {
			byte color = *buf;
			if (color != _mouseKeycolor) {
				if (_depth == 16)
					*(uint16 *)dst = _palette16[color];
				else {
					*(uint32 *)dst = _palette32[color];
				}
			}
			buf++;
			dst += _bytesPerPixel;
			width--;
		}
		buf += w - real_w;
		dst += (_fb_width - real_w) * _bytesPerPixel;
		real_h--;
	}
}

void OSystem_X11::set_mouse_pos(int x, int y) {
	if ((x != _curMouseState.x) || (y != _curMouseState.y)) {
		_curMouseState.x = x;
		_curMouseState.y = y;
		if (_mouse_state_changed == false) {
			undraw_mouse();
		}
		_mouse_state_changed = true;
	}
}

void OSystem_X11::warpMouse(int x, int y) {
	set_mouse_pos(x, y);
}

void OSystem_X11::setMouseCursor(const byte *buf, uint w, uint h, int hotspot_x, int hotspot_y, byte keycolor, int cursorTargetScale) {
	_curMouseState.w = w;
	_curMouseState.h = h;
	_curMouseState.hot_x = hotspot_x;
	_curMouseState.hot_y = hotspot_y;

	if (_ms_buf)
		free(_ms_buf);
	_ms_buf = (byte *) malloc(w * h);
	memcpy(_ms_buf, buf, w * h);

	if (_mouse_state_changed == false) {
		undraw_mouse();
	}
	_mouseKeycolor = keycolor;
	_mouse_state_changed = true;
}

void OSystem_X11::setShakePos(int shake_pos) {
	if (_new_shake_pos != shake_pos) {
		if (_mouse_state_changed == false) {
			undraw_mouse();
		}
		_mouse_state_changed = true;
	}
	_new_shake_pos = shake_pos;
}

int OSystem_X11::getOutputSampleRate() const {
	return SAMPLES_PER_SEC;
}

bool OSystem_X11::openCD(int drive) {
	return false;
}

bool OSystem_X11::pollCD() {
	return false;
}

void OSystem_X11::playCD(int track, int num_loops, int start_frame, int duration) {
}

void OSystem_X11::stopCD() {
}

void OSystem_X11::updateCD() {
}

void OSystem_X11::delayMillis(uint msecs) {
	usleep(msecs * 1000);
}

bool OSystem_X11::pollEvent(Event &scumm_event) {
	/* First, handle timers */
	uint32 current_msecs = getMillis();

	if (_timer_active && (current_msecs >= _timer_next_expiry)) {
		_timer_duration = _timer_callback(_timer_duration);
		_timer_next_expiry = current_msecs + _timer_duration;
	}

	while (XPending(_display)) {
		XEvent event;

		XNextEvent(_display, &event);
		switch (event.type) {
		case Expose:{
				int real_w, real_h;
				int real_x, real_y;
				real_x = event.xexpose.x;
				real_y = event.xexpose.y;
				real_w = event.xexpose.width;
				real_h = event.xexpose.height;
				if (real_x < _scumm_x) {
					real_w -= _scumm_x - real_x;
					real_x = 0;
				} else {
					real_x -= _scumm_x;
				}
				if (real_y < _scumm_y) {
					real_h -= _scumm_y - real_y;
					real_y = 0;
				} else {
					real_y -= _scumm_y;
				}
				if ((real_h <= 0) || (real_w <= 0))
					break;
				if ((real_x >=_fb_width) || (real_y >=_fb_height))
					break;

				if ((real_x + real_w) >=_fb_width) {
					real_w =_fb_width - real_x;
				}
				if ((real_y + real_h) >=_fb_height) {
					real_h =_fb_height - real_y;
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
					_report_presses = 0;
					break;

				case 133:
					_fake_right_mouse = 1;
					break;

				default:{
						KeySym xsym;
						xsym = XKeycodeToKeysym(_display, event.xkey.keycode, 0);
						keycode = xsym;
						if ((xsym >= 'a') && (xsym <= 'z') && (event.xkey.state & 0x01))
							xsym &= ~0x20;		/* Handle shifted keys */
						ascii = xsym;
					}
				}
				if (keycode != -1) {
					scumm_event.type = EVENT_KEYDOWN;
					scumm_event.kbd.keycode = keycode;
					scumm_event.kbd.ascii = (ascii != -1 ? ascii : keycode);
					scumm_event.kbd.flags = mode;
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
					_report_presses = 1;
					break;

				case 133:							/* Arrow on the iPAQ */
					_fake_right_mouse = 0;
					break;

				default:{
						KeySym xsym;
						xsym = XKeycodeToKeysym(_display, event.xkey.keycode, 0);
						keycode = xsym;
						if ((xsym >= 'a') && (xsym <= 'z') && (event.xkey.state & 0x01))
							xsym &= ~0x20;		/* Handle shifted keys */
						ascii = xsym;
					}
				}
				if (keycode != -1) {
					scumm_event.type = EVENT_KEYUP;
					scumm_event.kbd.keycode = keycode;
					scumm_event.kbd.ascii = (ascii != -1 ? ascii : keycode);
					scumm_event.kbd.flags = mode;
					return true;
				}
			}
			break;

		case ButtonPress:
			if (_report_presses != 0) {
				if (event.xbutton.button == 1) {
					if (_fake_right_mouse == 0) {
						scumm_event.type = EVENT_LBUTTONDOWN;
					} else {
						scumm_event.type = EVENT_RBUTTONDOWN;
					}
				} else if (event.xbutton.button == 3)
					scumm_event.type = EVENT_RBUTTONDOWN;
				scumm_event.mouse.x = event.xbutton.x - _scumm_x;
				scumm_event.mouse.y = event.xbutton.y - _scumm_y;
				return true;
			}
			break;

		case ButtonRelease:
			if (_report_presses != 0) {
				if (event.xbutton.button == 1) {
					if (_fake_right_mouse == 0) {
						scumm_event.type = EVENT_LBUTTONUP;
					} else {
						scumm_event.type = EVENT_RBUTTONUP;
					}
				} else if (event.xbutton.button == 3)
					scumm_event.type = EVENT_RBUTTONUP;
				scumm_event.mouse.x = event.xbutton.x - _scumm_x;
				scumm_event.mouse.y = event.xbutton.y - _scumm_y;
				return true;
			}
			break;

		case MotionNotify:
			scumm_event.type = EVENT_MOUSEMOVE;
			scumm_event.mouse.x = event.xmotion.x - _scumm_x;
			scumm_event.mouse.y = event.xmotion.y - _scumm_y;
			set_mouse_pos(scumm_event.mouse.x, scumm_event.mouse.y);
			return true;

		case ConfigureNotify:{
				if ((_window_width != event.xconfigure.width) || (_window_height != event.xconfigure.height)) {
					_window_width = event.xconfigure.width;
					_window_height = event.xconfigure.height;
					_scumm_x = (_window_width -_fb_width) / 2;
					_scumm_y = (_window_height -_fb_height) / 2;
					XFillRectangle(_display, _window, _black_gc, 0, 0, _window_width, _window_height);
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

void OSystem_X11::setTimerCallback(TimerProc callback, int interval) {
	if (callback != NULL) {
		_timer_duration = interval;
		_timer_next_expiry = getMillis() + interval;
		_timer_callback = callback;
		_timer_active = true;
	} else {
		_timer_active = false;
	}
}

OSystem::MutexRef OSystem_X11::createMutex(void) {
	pthread_mutex_t *mutex = (pthread_mutex_t *) malloc(sizeof(pthread_mutex_t));
	pthread_mutex_init(mutex, NULL);
	return (MutexRef)mutex;
}

void OSystem_X11::lockMutex(MutexRef mutex) {
	pthread_mutex_lock((pthread_mutex_t *) mutex);
}

void OSystem_X11::unlockMutex(MutexRef mutex) {
	pthread_mutex_unlock((pthread_mutex_t *) mutex);
}

void OSystem_X11::deleteMutex(MutexRef mutex) {
	pthread_mutex_destroy((pthread_mutex_t *) mutex);
	free(mutex);
}

void OSystem_X11::showOverlay() {
	_overlay_visible = true;
}

void OSystem_X11::hideOverlay() {
	_overlay_visible = false;
	_palette_changed = true; // This is to force a full redraw to hide the overlay
}

void OSystem_X11::clearOverlay() {
	if (_overlay_visible == false)
		return;
	DirtyRect d = { 0, 0, _fb_width, _fb_height };
	AddDirtyRec(0, 0, _fb_width, _fb_height);
	blit(&d, _local_fb_overlay, _fb_width);
}

void OSystem_X11::grabOverlay(int16 *dest, int pitch) {
	if (_overlay_visible == false)
		return;

	DirtyRect d = { 0, 0, _fb_width, _fb_height };
	blit(&d, (uint16 *)dest, pitch);
}

void OSystem_X11::copyRectToOverlay(const int16 *src, int pitch, int x, int y, int w, int h) {
	if (_overlay_visible == false)
		return;
	uint16 *dst = _local_fb_overlay + x + (y * _fb_width);
	AddDirtyRec(x, y, w, h);
	while (h > 0) {
		memcpy(dst, src, w * sizeof(*dst));
		dst +=_fb_width;
		src += pitch;
		h--;
	}
}

int16 OSystem_X11::getHeight() {
	return _fb_height;
}

int16 OSystem_X11::getWidth() {
	return _fb_width;
}

void OSystem_X11::grabPalette(byte *colors, uint start, uint num) {
	warning("Dummy: grabPalette()");
}
