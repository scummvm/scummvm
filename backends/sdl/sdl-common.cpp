/* ScummVM - Scumm Interpreter
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

#include "sdl-common.h"
#include "sound/mididrv.h"
#include "common/scaler.h"
#include "common/util.h"

#include "scummvm.xpm"

// FIXME move joystick defines out and replace with confile file options
// we should really allow users to map any key to a joystick button
#define JOY_DEADZONE 3200
#define JOY_ANALOG
// #define JOY_INVERT_Y
#define JOY_XAXIS 0
#define JOY_YAXIS 1
// buttons
#define JOY_BUT_LMOUSE 0
#define JOY_BUT_RMOUSE 2
#define JOY_BUT_ESCAPE 3
#define JOY_BUT_PERIOD 1
#define JOY_BUT_SPACE 4
#define JOY_BUT_F5 5

OSystem *OSystem_SDL_create(int gfx_mode, bool full_screen, bool aspect_ratio, int joystick_num) {
	return OSystem_SDL_Common::create(gfx_mode, full_screen, aspect_ratio, joystick_num);
}

OSystem *OSystem_SDL_Common::create(int gfx_mode, bool full_screen, bool aspect_ratio, int joystick_num) {
	OSystem_SDL_Common *syst = OSystem_SDL_Common::create_intern();

	syst->init_intern(gfx_mode, full_screen, aspect_ratio, joystick_num);

	return syst;
}

void OSystem_SDL_Common::init_intern(int gfx_mode, bool full_screen, bool aspect_ratio, int joystick_num) {

	_mode = gfx_mode;
	_full_screen = full_screen;
	_adjustAspectRatio = aspect_ratio;
	_mode_flags = 0;
	uint32 sdlFlags;

	sdlFlags = SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER;

	if (joystick_num > -1)
		sdlFlags |= SDL_INIT_JOYSTICK;

	if (SDL_Init(sdlFlags) ==-1) {
		error("Could not initialize SDL: %s.\n", SDL_GetError());
	}

	_graphicsMutex = create_mutex();

	SDL_ShowCursor(SDL_DISABLE);
	
	// Enable unicode support if possible
	SDL_EnableUNICODE(1); 

#ifndef MACOSX		// Don't set icon on OS X, as we use a nicer external icon there
	// Setup the icon
	setup_icon();
#endif

	// enable joystick
	if (joystick_num > -1 && SDL_NumJoysticks() > 0) {
		printf("Using joystick: %s\n", SDL_JoystickName(0));
		init_joystick(joystick_num);
	}
}

void OSystem_SDL_Common::set_timer(TimerProc callback, int timer) {
	SDL_SetTimer(timer, (SDL_TimerCallback) callback);
}

OSystem_SDL_Common::OSystem_SDL_Common()
	: _screen(0), _screenWidth(0), _screenHeight(0),
	_tmpscreen(0), _tmpScreenWidth(0), _overlayVisible(false),
	_cdrom(0), _modeChanged(false), _dirty_checksums(0),
	_mouseVisible(false), _mouseDrawn(false), _mouseData(0),
	_mouseHotspotX(0), _mouseHotspotY(0),
	_currentShakePos(0), _newShakePos(0),
	_paletteDirtyStart(0), _paletteDirtyEnd(0),
	_graphicsMutex(0) {

	// allocate palette storage
	_currentPalette = (SDL_Color *)calloc(sizeof(SDL_Color), 256);

	// allocate the dirty rect storage
	_mouseBackup = (byte *)malloc(MAX_MOUSE_W * MAX_MOUSE_H * MAX_SCALING * 2);

	// reset mouse state
	memset(&km, 0, sizeof(km));
}

OSystem_SDL_Common::~OSystem_SDL_Common() {
//	unload_gfx_mode();

	if (_dirty_checksums)
		free(_dirty_checksums);
	free(_currentPalette);
	free(_mouseBackup);
	delete_mutex(_graphicsMutex);

	SDL_ShowCursor(SDL_ENABLE);
	SDL_Quit();
}

void OSystem_SDL_Common::init_size(uint w, uint h) {
	// Avoid redundant res changes
	if ((int)w == _screenWidth && (int)h == _screenHeight)
		return;

	_screenWidth = w;
	_screenHeight = h;

	if (h != 200)
		_adjustAspectRatio = false;

	CKSUM_NUM = (_screenWidth * _screenHeight / (8 * 8));
	if (_dirty_checksums)
		free(_dirty_checksums);
	_dirty_checksums = (uint32 *)calloc(CKSUM_NUM * 2, sizeof(uint32));

	unload_gfx_mode();
	load_gfx_mode();
}

void OSystem_SDL_Common::copy_rect(const byte *buf, int pitch, int x, int y, int w, int h) {
	if (_screen == NULL)
		return;

	StackLock lock(_graphicsMutex, this);	// Lock the mutex until this function ends
	
	if (((long)buf & 3) == 0 && pitch == _screenWidth && x==0 && y==0 &&
			w==_screenWidth && h==_screenHeight && _mode_flags&DF_WANT_RECT_OPTIM) {
		/* Special, optimized case for full screen updates.
		 * It tries to determine what areas were actually changed,
		 * and just updates those, on the actual display. */
		add_dirty_rgn_auto(buf);
	} else {
		/* Clip the coordinates */
		if (x < 0) {
			w += x;
			buf -= x;
			x = 0;
		}

		if (y < 0) {
			h += y;
			buf -= y * pitch;
			y = 0;
		}

		if (w > _screenWidth - x) {
			w = _screenWidth - x;
		}

		if (h > _screenHeight - y) {
			h = _screenHeight - y;
		}

		if (w <= 0 || h <= 0)
			return;

		cksum_valid = false;
		add_dirty_rect(x, y, w, h);
	}

	/* FIXME: undraw mouse only if the draw rect intersects with the mouse rect */
	if (_mouseDrawn)
		undraw_mouse();

	if (SDL_LockSurface(_screen) == -1)
		error("SDL_LockSurface failed: %s.\n", SDL_GetError());

	byte *dst = (byte *)_screen->pixels + y * _screenWidth + x;

	if (_screenWidth==pitch && pitch==w) {
		memcpy(dst, buf, h*w);
	} else {
		do {
			memcpy(dst, buf, w);
			dst += _screenWidth;
			buf += pitch;
		} while (--h);
	}

	SDL_UnlockSurface(_screen);
}


void OSystem_SDL_Common::move_screen(int dx, int dy, int height) {

	// Short circuit check - do we have to do anything anyway?
	if ((dx == 0 && dy == 0) || height <= 0)
		return;
	
	int x, y;
	
	// We'll have to do a full screen redraw anyway, so set the flag.
	_forceFull = true;
	
	// Hide the mouse
	if (_mouseDrawn)
		undraw_mouse();

	// FIXME - calling copy_rect repeatedly is horribly inefficient, as it (un)locks the surface repeatedly
	// and it performs unneeded clipping checks etc.
	// Furthermore, this code is not correct, techincally: the pixels members of an SDLSource may be 0
	// while it is not locked (e.g. for HW surfaces which are stored in the graphic card's VRAM).
	
	// vertical movement
	if (dy > 0) {
		// move down - copy from bottom to top
		for (y = height - 1; y >= dy; y--)
			copy_rect((byte *)_screen->pixels + _screenWidth * (y - dy), _screenWidth, 0, y, _screenWidth, 1);
	} else if (dy < 0) {
		// move up - copy from top to bottom
		dy = -dy;
		for (y = dy; y < height; y++)
			copy_rect((byte *)_screen->pixels + _screenWidth * y, _screenWidth, 0, y - dy, _screenWidth, 1);
	}

	// horizontal movement
	if (dx > 0) {
		// move right - copy from right to left
		for (x = _screenWidth - 1; x >= dx; x--)
			copy_rect((byte *)_screen->pixels + x - dx, _screenWidth, x, 0, 1, height);
	} else if (dx < 0)  {
		// move left - copy from left to right
		dx = -dx;
		for (x = dx; x < _screenWidth; x++)
			copy_rect((byte *)_screen->pixels + x, _screenWidth, x - dx, 0, 1, height);
	}
}

void OSystem_SDL_Common::add_dirty_rect(int x, int y, int w, int h) {
	if (_forceFull)
		return;

	if (_num_dirty_rects == NUM_DIRTY_RECT)
		_forceFull = true;
	else {
		SDL_Rect *r = &_dirty_rect_list[_num_dirty_rects++];
		// Extend the dirty region by 1 pixel for scalers
		// that "smear" the screen, e.g. 2xSAI
		if (_mode_flags & DF_UPDATE_EXPAND_1_PIXEL) {
			x--;
			y--;
			w+=2;
			h+=2;
		}

		// clip
		if (x < 0) {
			w += x; x = 0;
		}

		if (y < 0) {
			h += y;
			y=0;
		}

		if (w > _screenWidth - x) {
			w = _screenWidth - x;
		}

		if (h > _screenHeight - y) {
			h = _screenHeight - y;
		}

		if (_adjustAspectRatio)
			makeRectStretchable(x, y, w, h);
	
		r->x = x;
		r->y = y;
		r->w = w;
		r->h = h;
	}
}

#define ROL(a,n) a = (a << (n)) | (a >> (32 - (n)))
#define DOLINE(x) a ^= ((const uint32*)buf)[0 + (x) * (_screenWidth / 4)]; b ^= ((const uint32 *)buf)[1 + (x) * (_screenWidth / 4)]

void OSystem_SDL_Common::mk_checksums(const byte *buf) {
	uint32 *sums = _dirty_checksums;
	uint x,y;
	const uint last_x = (uint)_screenWidth / 8;
	const uint last_y = (uint)_screenHeight / 8;

	/* the 8x8 blocks in buf are enumerated starting in the top left corner and
	 * reading each line at a time from left to right */
	for(y = 0; y != last_y; y++, buf += _screenWidth * (8 - 1))
		for(x=0; x != last_x; x++, buf += 8) {
			uint32 a = x;
			uint32 b = y;

			DOLINE(0); ROL(a,13); ROL(b,11);
			DOLINE(2); ROL(a,13); ROL(b,11);
			DOLINE(4); ROL(a,13); ROL(b,11);
			DOLINE(6); ROL(a,13); ROL(b,11);

			a *= 0xDEADBEEF;
			b *= 0xBAADF00D;

			DOLINE(1); ROL(a,13); ROL(b,11);
			DOLINE(3); ROL(a,13); ROL(b,11);
			DOLINE(5); ROL(a,13); ROL(b,11);
			DOLINE(7); ROL(a,13); ROL(b,11);

			/* output the checksum for this block */
			*sums++ = a + b;
		}
}
#undef DOLINE
#undef ROL

void OSystem_SDL_Common::add_dirty_rgn_auto(const byte *buf) {
	assert(((long)buf & 3) == 0);
	
	/* generate a table of the checksums */
	mk_checksums(buf);

	if (!cksum_valid) {
		_forceFull = true;
		cksum_valid = true;
	}

	/* go through the checksum list, compare it with the previous checksums,
		 and add all dirty rectangles to a list. try to combine small rectangles
		 into bigger ones in a simple way */
	if (!_forceFull) {
		int x,y,w;
		uint32 *ck = _dirty_checksums;
		
		for(y = 0; y!=_screenHeight / 8; y++) {
			for(x = 0; x!=_screenWidth / 8; x++,ck++) {
				if (ck[0] != ck[CKSUM_NUM]) {
					/* found a dirty 8x8 block, now go as far to the right as possible,
						 and at the same time, unmark the dirty status by setting old to new. */
					w=0;
					do {
						ck[w + CKSUM_NUM] = ck[w];
						w++;
					} while (x + w != _screenWidth / 8 && ck[w] != ck[w + CKSUM_NUM]);

					add_dirty_rect(x * 8, y * 8, w * 8, 8);

					if (_forceFull)
						goto get_out;
				}
			}
		}
	} else {
		get_out:;
		/* Copy old checksums to new */
		memcpy(_dirty_checksums + CKSUM_NUM, _dirty_checksums, CKSUM_NUM * sizeof(uint32));
	}
}

void OSystem_SDL_Common::kbd_mouse() {
	uint32 time = get_msecs();
	if (time >= km.last_time + km.delay_time) {
		km.last_time = time;
		if (km.x_down_count == 1) {
			km.x_down_time = time;
			km.x_down_count = 2;
		}
		if (km.y_down_count == 1) {
			km.y_down_time = time;
			km.y_down_count = 2;
		}

		if (km.x_vel || km.y_vel) {
			if (km.x_down_count) {
				if (time > km.x_down_time + km.delay_time * 12) {
					if (km.x_vel > 0)
						km.x_vel++;
					else
						km.x_vel--;
				} else if (time > km.x_down_time + km.delay_time * 8) {
					if (km.x_vel > 0)
						km.x_vel = 5;
					else
						km.x_vel = -5;
				}
			}
			if (km.y_down_count) {
				if (time > km.y_down_time + km.delay_time * 12) {
					if (km.y_vel > 0)
						km.y_vel++;
					else
						km.y_vel--;
				} else if (time > km.y_down_time + km.delay_time * 8) {
					if (km.y_vel > 0)
						km.y_vel = 5;
					else
						km.y_vel = -5;
				}
			}

			km.x += km.x_vel;
			km.y += km.y_vel;

			if (km.x < 0) {
				km.x = 0;
				km.x_vel = -1;
				km.x_down_count = 1;
			} else if (km.x > km.x_max) {
				km.x = km.x_max;
				km.x_vel = 1;
				km.x_down_count = 1;
			}

			if (km.y < 0) {
				km.y = 0;
				km.y_vel = -1;
				km.y_down_count = 1;
			} else if (km.y > km.y_max) {
				km.y = km.y_max;
				km.y_vel = 1;
				km.y_down_count = 1;
			}

			SDL_WarpMouse(km.x, km.y);
		}
	}
}

bool OSystem_SDL_Common::show_mouse(bool visible) {
	if (_mouseVisible == visible)
		return visible;
	
	bool last = _mouseVisible;
	_mouseVisible = visible;

	if (visible)
		draw_mouse();
	else
		undraw_mouse();

	return last;
}

void OSystem_SDL_Common::set_mouse_pos(int x, int y) {
	if (x != _mouseCurState.x || y != _mouseCurState.y) {
		_mouseCurState.x = x;
		_mouseCurState.y = y;
		undraw_mouse();
		update_screen();
	}
}

void OSystem_SDL_Common::warp_mouse(int x, int y) {
	if (_mouseCurState.x != x || _mouseCurState.y != y) {
		SDL_WarpMouse(x * _scaleFactor, y * _scaleFactor);

		// SDL_WarpMouse() generates a mouse movement event, so
		// set_mouse_pos() would be called eventually. However, the
		// cannon script in CoMI calls this function twice each time
		// the cannon is reloaded. Unless we update the mouse position
		// immediately the second call is ignored, causing the cannon
		// to change its aim.

		set_mouse_pos(x, y);
	}
}
	
void OSystem_SDL_Common::set_mouse_cursor(const byte *buf, uint w, uint h, int hotspot_x, int hotspot_y) {
	assert(w <= MAX_MOUSE_W);
	assert(h <= MAX_MOUSE_H);
	_mouseCurState.w = w;
	_mouseCurState.h = h;

	_mouseHotspotX = hotspot_x;
	_mouseHotspotY = hotspot_y;

	_mouseData = buf;

	undraw_mouse();
}

void OSystem_SDL_Common::set_shake_pos(int shake_pos) {
	_newShakePos = shake_pos;
}

uint32 OSystem_SDL_Common::get_msecs() {
	return SDL_GetTicks();	
}

void OSystem_SDL_Common::delay_msecs(uint msecs) {
	SDL_Delay(msecs);
}

static int mapKey(SDLKey key, SDLMod mod, Uint16 unicode)
{
	if (key >= SDLK_F1 && key <= SDLK_F9) {
		return key - SDLK_F1 + 315;
	} else if (key >= SDLK_KP0 && key <= SDLK_KP9) {
		return key - SDLK_KP0 + '0';
	} else if (key >= SDLK_UP && key <= SDLK_PAGEDOWN) {
		return key;
	} else if (unicode) {
		return unicode;
	} else if (key >= 'a' && key <= 'z' && mod & KMOD_SHIFT) {
		return key & ~0x20;
	} else if (key >= SDLK_NUMLOCK && key <= SDLK_EURO) {
		return 0;
	}
	return key;
}

void OSystem_SDL_Common::fillMouseEvent(Event &event, int x, int y) {
	event.mouse.x = x;
	event.mouse.y = y;
	
	// FIXME: HACK HACK HACK. This works around an odd problem in the OpenGL
	// variant of the SDL backend, where the mouse y coordinates are reversed.
	// Since the OpenGL variants is quite hackish anyway, we have to hard code
	// here a screen height of 480).
	if (_mode_flags & DF_REVERSE_Y)
		event.mouse.y = 480 - event.mouse.y;
	
	// Update the "keyboard mouse" coords
	km.x = event.mouse.x;
	km.y = event.mouse.y;

	// Adjust for the screen scaling
	event.mouse.x /= _scaleFactor;
	event.mouse.y /= _scaleFactor;

	// Optionally perform aspect ratio adjusting
	if (_adjustAspectRatio)
		event.mouse.y = aspect2Real(event.mouse.y);
}

bool OSystem_SDL_Common::poll_event(Event *event) {
	SDL_Event ev;
	int axis;
	byte b = 0;
	
	kbd_mouse();
	
	// If the screen mode changed, send an EVENT_SCREEN_CHANGED
	if (_modeChanged) {
		_modeChanged = false;
		event->event_code = EVENT_SCREEN_CHANGED;
		return true;
	}

	while(SDL_PollEvent(&ev)) {
		switch(ev.type) {
		case SDL_KEYDOWN:
			if (ev.key.keysym.mod & KMOD_SHIFT)
				b |= KBD_SHIFT;
			if (ev.key.keysym.mod & KMOD_CTRL)
				b |= KBD_CTRL;
			if (ev.key.keysym.mod & KMOD_ALT)
				b |= KBD_ALT;
			event->kbd.flags = b;

			// Alt-Return toggles full screen mode				
			if (b == KBD_ALT && ev.key.keysym.sym == SDLK_RETURN) {
				property(PROP_TOGGLE_FULLSCREEN, NULL);
				break;
			}

			if (b == KBD_ALT && ev.key.keysym.sym == 's') {
				char filename[20];

				for (int n = 0;; n++) {
					SDL_RWops *file;

					sprintf(filename, "scummvm%05d.bmp", n);
					file = SDL_RWFromFile(filename, "r");
					if (!file)
						break;
					SDL_RWclose(file);
				}
				if (save_screenshot(filename))
					printf("Saved '%s'\n", filename);
				else
					printf("Could not save screenshot!\n");
				break;
			}

#ifdef MACOSX
			// On Macintosh', Cmd-Q quits
			if ((ev.key.keysym.mod & KMOD_META) && ev.key.keysym.sym=='q') {
				event->event_code = EVENT_QUIT;
				return true;
			}
#else
			// Ctrl-z and Alt-X quit
			if ((b == KBD_CTRL && ev.key.keysym.sym=='z') || (b == KBD_ALT && ev.key.keysym.sym=='x')) {
				event->event_code = EVENT_QUIT;
				return true;
			}
#endif

			// Ctr-Alt-<key> will change the GFX mode
			if (b == (KBD_CTRL|KBD_ALT)) {
				const char keys[] = "1234567890cd";
				char *ptr;

				ptr = strchr(keys, ev.key.keysym.sym);
				if (ptr != NULL) {
					Property prop;

					prop.gfx_mode = ptr - keys;
					property(PROP_SET_GFX_MODE, &prop);
					break;
				}
			}

			// Ctr-Alt-a will change aspect ratio
			if (b == (KBD_CTRL|KBD_ALT) && ev.key.keysym.sym=='a') {
				property(PROP_TOGGLE_ASPECT_RATIO, NULL);
				break;
			}

			// Ctr-Alt-b will change bilinear filtering in OpenGL backend
			if (b == (KBD_CTRL|KBD_ALT) && ev.key.keysym.sym=='b') {
				Property prop;
				prop.gfx_mode = GFX_BILINEAR;
				property(PROP_SET_GFX_MODE, &prop);
				break;
			}

#ifdef QTOPIA
			// quit on fn+backspace on zaurus
			if (ev.key.keysym.sym == 127) {
				event->event_code = EVENT_QUIT;
				return true;
			}

			// map menu key (f11) to f5 (scumm menu)
			if (ev.key.keysym.sym == SDLK_F11) {
				event->event_code = EVENT_KEYDOWN;
				event->kbd.keycode = SDLK_F5;
				event->kbd.ascii = mapKey(SDLK_F5, ev.key.keysym.mod, 0);
			}
			// map center (space) to tab (default action )
			// I wanted to map the calendar button but the calendar comes up
			//
			else if (ev.key.keysym.sym == SDLK_SPACE) {
				event->event_code = EVENT_KEYDOWN;
				event->kbd.keycode = SDLK_TAB;
				event->kbd.ascii = mapKey(SDLK_TAB, ev.key.keysym.mod, 0);
			}
			// since we stole space (pause) above we'll rebind it to the tab key on the keyboard
			else if (ev.key.keysym.sym == SDLK_TAB) {
				event->event_code = EVENT_KEYDOWN;
				event->kbd.keycode = SDLK_SPACE;
				event->kbd.ascii = mapKey(SDLK_SPACE, ev.key.keysym.mod, 0);
			} else {
			// let the events fall through if we didn't change them, this may not be the best way to
			// set it up, but i'm not sure how sdl would like it if we let if fall through then redid it though.
			// and yes i have an huge terminal size so i dont wrap soon enough.
				event->event_code = EVENT_KEYDOWN;
				event->kbd.keycode = ev.key.keysym.sym;
				event->kbd.ascii = mapKey(ev.key.keysym.sym, ev.key.keysym.mod, ev.key.keysym.unicode);
			}
#else
			event->event_code = EVENT_KEYDOWN;
			event->kbd.keycode = ev.key.keysym.sym;
			event->kbd.ascii = mapKey(ev.key.keysym.sym, ev.key.keysym.mod, ev.key.keysym.unicode);
#endif
			
			switch(ev.key.keysym.sym) {
			case SDLK_LEFT:
				km.x_vel = -1;
				km.x_down_count = 1;
				break;
			case SDLK_RIGHT:
				km.x_vel =  1;
				km.x_down_count = 1;
				break;
			case SDLK_UP:
				km.y_vel = -1;
				km.y_down_count = 1;
				break;
			case SDLK_DOWN:
				km.y_vel =  1;
				km.y_down_count = 1;
				break;
			default:
				break;
			}

			return true;
	
		case SDL_KEYUP:
			event->event_code = EVENT_KEYUP;
			event->kbd.keycode = ev.key.keysym.sym;
			event->kbd.ascii = mapKey(ev.key.keysym.sym, ev.key.keysym.mod, ev.key.keysym.unicode);

			switch(ev.key.keysym.sym) {
			case SDLK_LEFT:
				if (km.x_vel < 0) {
					km.x_vel = 0;
					km.x_down_count = 0;
				}
				break;
			case SDLK_RIGHT:
				if (km.x_vel > 0) {
					km.x_vel = 0;
					km.x_down_count = 0;
				}
				break;
			case SDLK_UP:
				if (km.y_vel < 0) {
					km.y_vel = 0;
					km.y_down_count = 0;
				}
				break;
			case SDLK_DOWN:
				if (km.y_vel > 0) {
					km.y_vel = 0;
					km.y_down_count = 0;
				}
				break;
			default:
				break;
			}
			return true;

		case SDL_MOUSEMOTION:
			event->event_code = EVENT_MOUSEMOVE;
			fillMouseEvent(*event, ev.motion.x, ev.motion.y);
			
			set_mouse_pos(event->mouse.x, event->mouse.y);
			return true;

		case SDL_MOUSEBUTTONDOWN:
			if (ev.button.button == SDL_BUTTON_LEFT)
				event->event_code = EVENT_LBUTTONDOWN;
			else if (ev.button.button == SDL_BUTTON_RIGHT)
				event->event_code = EVENT_RBUTTONDOWN;
#if defined(SDL_BUTTON_WHEELUP) && defined(SDL_BUTTON_WHEELDOWN)
			else if (ev.button.button == SDL_BUTTON_WHEELUP)
				event->event_code = EVENT_WHEELUP;
			else if (ev.button.button == SDL_BUTTON_WHEELDOWN)
				event->event_code = EVENT_WHEELDOWN;
#endif
			else
				break;

			fillMouseEvent(*event, ev.button.x, ev.button.y);

			return true;

		case SDL_MOUSEBUTTONUP:
			if (ev.button.button == SDL_BUTTON_LEFT)
				event->event_code = EVENT_LBUTTONUP;
			else if (ev.button.button == SDL_BUTTON_RIGHT)
				event->event_code = EVENT_RBUTTONUP;
			else
				break;
			fillMouseEvent(*event, ev.button.x, ev.button.y);

			return true;

		case SDL_JOYBUTTONDOWN:
			if (ev.jbutton.button == JOY_BUT_LMOUSE) {
				event->event_code = EVENT_LBUTTONDOWN;
			} else if (ev.jbutton.button == JOY_BUT_RMOUSE) {
				event->event_code = EVENT_RBUTTONDOWN;
			} else {
				event->event_code = EVENT_KEYDOWN;
				switch (ev.jbutton.button) {
					case JOY_BUT_ESCAPE:
						event->kbd.keycode = SDLK_ESCAPE;
						event->kbd.ascii = mapKey(SDLK_ESCAPE, ev.key.keysym.mod, 0);
						break;
					case JOY_BUT_PERIOD:
						event->kbd.keycode = SDLK_PERIOD;
						event->kbd.ascii = mapKey(SDLK_PERIOD, ev.key.keysym.mod, 0);
						break;
					case JOY_BUT_SPACE:
						event->kbd.keycode = SDLK_SPACE;
						event->kbd.ascii = mapKey(SDLK_SPACE, ev.key.keysym.mod, 0);
						break;
					case JOY_BUT_F5:
						event->kbd.keycode = SDLK_F5;
						event->kbd.ascii = mapKey(SDLK_F5, ev.key.keysym.mod, 0);
						break; 
				}
			}
			return true;

		case SDL_JOYBUTTONUP:
			if (ev.jbutton.button == JOY_BUT_LMOUSE) {
				event->event_code = EVENT_LBUTTONUP;
			} else if (ev.jbutton.button == JOY_BUT_RMOUSE) {
				event->event_code = EVENT_RBUTTONUP;
			} else {
				event->event_code = EVENT_KEYUP;
				switch (ev.jbutton.button) {
					case JOY_BUT_ESCAPE:
						event->kbd.keycode = SDLK_ESCAPE;
						event->kbd.ascii = mapKey(SDLK_ESCAPE, ev.key.keysym.mod, 0);
						break;
					case JOY_BUT_PERIOD:
						event->kbd.keycode = SDLK_PERIOD;
						event->kbd.ascii = mapKey(SDLK_PERIOD, ev.key.keysym.mod, 0);
						break;
					case JOY_BUT_SPACE:
						event->kbd.keycode = SDLK_SPACE;
						event->kbd.ascii = mapKey(SDLK_SPACE, ev.key.keysym.mod, 0);
						break;
					case JOY_BUT_F5:
						event->kbd.keycode = SDLK_F5;
						event->kbd.ascii = mapKey(SDLK_F5, ev.key.keysym.mod, 0);
						break;
				} 
			}
			return true;

		case SDL_JOYAXISMOTION:
			axis = ev.jaxis.value;
			if ( axis > JOY_DEADZONE) {
				axis -= JOY_DEADZONE;
				event->event_code = EVENT_MOUSEMOVE;
			} else if ( axis < -JOY_DEADZONE ) {
				axis += JOY_DEADZONE;
				event->event_code = EVENT_MOUSEMOVE;
			} else
				axis = 0;

			if ( ev.jaxis.axis == JOY_XAXIS) { 
#ifdef JOY_ANALOG
				km.x_vel = axis/2000;
				km.x_down_count = 0;
#else
				if (axis != 0) {
					km.x_vel = (axis > 0) ? 1:-1;
					km.x_down_count = 1;
				} else {
					km.x_vel = 0;
					km.x_down_count = 0;
				}
#endif

			} else if (ev.jaxis.axis == JOY_YAXIS) { 
#ifndef JOY_INVERT_Y
				axis = -axis;
#endif
#ifdef JOY_ANALOG
				km.y_vel = -axis / 2000;
				km.y_down_count = 0;
#else
				if (axis != 0) {
					km.y_vel = (-axis > 0) ? 1: -1;
					km.y_down_count = 1;
				} else {
					km.y_vel = 0;
					km.y_down_count = 0;
				}
#endif
			}
			event->mouse.x = km.x;
			event->mouse.y = km.y;
			event->mouse.x /= _scaleFactor;
			event->mouse.y /= _scaleFactor;

			if (_adjustAspectRatio)
				event->mouse.y = aspect2Real(event->mouse.y);

			return true;

		case SDL_VIDEOEXPOSE:
			_forceFull = true;
			break;

		case SDL_QUIT:
			event->event_code = EVENT_QUIT;
			return true;
		}
	}
	return false;
}

bool OSystem_SDL_Common::set_sound_proc(SoundProc proc, void *param, SoundFormat format) {
	SDL_AudioSpec desired;

	memset(&desired, 0, sizeof(desired));

	/* only one format supported at the moment */
	desired.freq = SAMPLES_PER_SEC;
	desired.format = AUDIO_S16SYS;
	desired.channels = 2;
	desired.samples = 2048;
	desired.callback = proc;
	desired.userdata = param;
	if (SDL_OpenAudio(&desired, NULL) != 0) {
		return false;
	}
	SDL_PauseAudio(0);
	return true;
}

void OSystem_SDL_Common::clear_sound_proc() {
	SDL_CloseAudio();
}

uint32 OSystem_SDL_Common::property(int param, Property *value) {
	switch(param) {

	case PROP_WANT_RECT_OPTIM:
		_mode_flags |= DF_WANT_RECT_OPTIM;
		break;

	case PROP_GET_FULLSCREEN:
		return _full_screen;

	case PROP_SET_WINDOW_CAPTION:
		SDL_WM_SetCaption(value->caption, value->caption);
		return 1;

	case PROP_OPEN_CD:
		if (SDL_InitSubSystem(SDL_INIT_CDROM) == -1)
			_cdrom = NULL;
		else {
			_cdrom = SDL_CDOpen(value->cd_num);
			/* Did if open? Check if _cdrom is NULL */
			if (!_cdrom) {
				warning("Couldn't open drive: %s", SDL_GetError());
			} else {
				cd_num_loops = 0;
				cd_stop_time = 0;
				cd_end_time = 0;
			}
		}
		break;

	case PROP_SHOW_DEFAULT_CURSOR:
		SDL_ShowCursor(value->show_cursor ? SDL_ENABLE : SDL_DISABLE);		
		break;

	case PROP_GET_SAMPLE_RATE:
		return SAMPLES_PER_SEC;
	}

	return 0;
}

void OSystem_SDL_Common::quit() {
	if(_cdrom) {
		SDL_CDStop(_cdrom);
		SDL_CDClose(_cdrom);
	}
	unload_gfx_mode();

	SDL_ShowCursor(SDL_ENABLE);
	SDL_Quit();

	exit(0);
}

void OSystem_SDL_Common::draw_mouse() {
	if (_mouseDrawn || !_mouseVisible)
		return;

	int x = _mouseCurState.x - _mouseHotspotX;
	int y = _mouseCurState.y - _mouseHotspotY;
	int w = _mouseCurState.w;
	int h = _mouseCurState.h;
	byte color;
	const byte *src = _mouseData;		// Image representing the mouse

	// clip the mouse rect, and addjust the src pointer accordingly
	if (x < 0) {
		w += x;
		src -= x;
		x = 0;
	}
	if (y < 0) {
		h += y;
		src -= y * _mouseCurState.w;
		y = 0;
	}

	if (w > _screenWidth - x)
		w = _screenWidth - x;
	if (h > _screenHeight - y)
		h = _screenHeight - y;

	// Quick check to see if anything has to be drawn at all
	if (w <= 0 || h <= 0)
		return;

	// Store the bounding box so that undraw mouse can restore the area the
	// mouse currently covers to its original content.
	_mouseOldState.x = x;
	_mouseOldState.y = y;
	_mouseOldState.w = w;
	_mouseOldState.h = h;

	// Draw the mouse cursor; backup the covered area in "bak"
	if (SDL_LockSurface(_overlayVisible ? _tmpscreen : _screen) == -1)
		error("SDL_LockSurface failed: %s.\n", SDL_GetError());

	// Mark as dirty
	add_dirty_rect(x, y, w, h);

	if (!_overlayVisible) {
		byte *bak = _mouseBackup;		// Surface used to backup the area obscured by the mouse
		byte *dst;					// Surface we are drawing into
	
		dst = (byte *)_screen->pixels + y * _screenWidth + x;
		while (h > 0) {
			int width = w;
			while (width > 0) {
				*bak++ = *dst;
				color = *src++;
				if (color != 0xFF)	// 0xFF = transparent, don't draw
					*dst = color;
				dst++;
				width--;
			}
			src += _mouseCurState.w - w;
			bak += MAX_MOUSE_W - w;
			dst += _screenWidth - w;
			h--;
		}
	
	} else {
		uint16 *bak = (uint16 *)_mouseBackup;	// Surface used to backup the area obscured by the mouse
		uint16 *dst;					// Surface we are drawing into
	
		dst = (uint16 *)_tmpscreen->pixels + (y + 1) * _tmpScreenWidth + (x + 1);
		while (h > 0) {
			int width = w;
			while (width > 0) {
				*bak++ = *dst;
				color = *src++;
				if (color != 0xFF)	// 0xFF = transparent, don't draw
					*dst = RGBToColor(_currentPalette[color].r, _currentPalette[color].g, _currentPalette[color].b);
				dst++;
				width--;
			}
			src += _mouseCurState.w - w;
			bak += MAX_MOUSE_W - w;
			dst += _tmpScreenWidth - w;
			h--;
		}
	}

	SDL_UnlockSurface(_overlayVisible ? _tmpscreen : _screen);

	// Finally, set the flag to indicate the mouse has been drawn
	_mouseDrawn = true;
}

void OSystem_SDL_Common::undraw_mouse() {
	if (!_mouseDrawn)
		return;
	_mouseDrawn = false;

	if (SDL_LockSurface(_overlayVisible ? _tmpscreen : _screen) == -1)
		error("SDL_LockSurface failed: %s.\n", SDL_GetError());

	const int old_mouse_x = _mouseOldState.x;
	const int old_mouse_y = _mouseOldState.y;
	const int old_mouse_w = _mouseOldState.w;
	const int old_mouse_h = _mouseOldState.h;
	int x, y;

	if (!_overlayVisible) {
		byte *dst, *bak = _mouseBackup;

		// No need to do clipping here, since draw_mouse() did that already
		dst = (byte *)_screen->pixels + old_mouse_y * _screenWidth + old_mouse_x;
		for (y = 0; y < old_mouse_h; ++y, bak += MAX_MOUSE_W, dst += _screenWidth) {
			for (x = 0; x < old_mouse_w; ++x) {
				dst[x] = bak[x];
			}
		}
	
	} else {

		uint16 *dst, *bak = (uint16 *)_mouseBackup;
	
		// No need to do clipping here, since draw_mouse() did that already
		dst = (uint16 *)_tmpscreen->pixels + (old_mouse_y + 1) * _tmpScreenWidth + (old_mouse_x + 1);
		for (y = 0; y < old_mouse_h; ++y, bak += MAX_MOUSE_W, dst += _tmpScreenWidth) {
			for (x = 0; x < old_mouse_w; ++x) {
				dst[x] = bak[x];
			}
		}
	}

	add_dirty_rect(old_mouse_x, old_mouse_y, old_mouse_w, old_mouse_h);

	SDL_UnlockSurface(_overlayVisible ? _tmpscreen : _screen);
}

void OSystem_SDL_Common::stop_cdrom() {	/* Stop CD Audio in 1/10th of a second */
	cd_stop_time = SDL_GetTicks() + 100;
	cd_num_loops = 0;
}

void OSystem_SDL_Common::play_cdrom(int track, int num_loops, int start_frame, int duration) {
	if (!num_loops && !start_frame)
		return;

	if (!_cdrom)
		return;
	
	if (duration > 0)
		duration += 5;

	cd_track = track;
	cd_num_loops = num_loops;
	cd_start_frame = start_frame;

	SDL_CDStatus(_cdrom);
	if (start_frame == 0 && duration == 0)
		SDL_CDPlayTracks(_cdrom, track, 0, 1, 0);
	else
		SDL_CDPlayTracks(_cdrom, track, start_frame, 0, duration);
	cd_duration = duration;
	cd_stop_time = 0;
	cd_end_time = SDL_GetTicks() + _cdrom->track[track].length * 1000 / CD_FPS;
}

bool OSystem_SDL_Common::poll_cdrom() {
	if (!_cdrom)
		return false;

	return (cd_num_loops != 0 && (SDL_GetTicks() < cd_end_time || SDL_CDStatus(_cdrom) != CD_STOPPED));
}

void OSystem_SDL_Common::update_cdrom() {
	if (!_cdrom)
		return;

	if (cd_stop_time != 0 && SDL_GetTicks() >= cd_stop_time) {
		SDL_CDStop(_cdrom);
		cd_num_loops = 0;
		cd_stop_time = 0;
		return;
	}

	if (cd_num_loops == 0 || SDL_GetTicks() < cd_end_time)
		return;

	if (cd_num_loops != 1 && SDL_CDStatus(_cdrom) != CD_STOPPED) {
		// Wait another second for it to be done
		cd_end_time += 1000;
		return;
	}

	if (cd_num_loops > 0)
		cd_num_loops--;

	if (cd_num_loops != 0) {
		if (cd_start_frame == 0 && cd_duration == 0)
			SDL_CDPlayTracks(_cdrom, cd_track, 0, 1, 0);
		else
			SDL_CDPlayTracks(_cdrom, cd_track, cd_start_frame, 0, cd_duration);
		cd_end_time = SDL_GetTicks() + _cdrom->track[cd_track].length * 1000 / CD_FPS;
	}
}

void OSystem_SDL_Common::setup_icon() {
	int w, h, ncols, nbytes, i;
	unsigned int rgba[256], icon[32 * 32];
	unsigned char mask[32][4];

	sscanf(scummvm_icon[0], "%d %d %d %d", &w, &h, &ncols, &nbytes);
	if ((w != 32) || (h != 32) || (ncols > 255) || (nbytes > 1)) {
		warning("Could not load the icon (%d %d %d %d)", w, h, ncols, nbytes);
		return;
	}
	for (i = 0; i < ncols; i++) {
		unsigned char code;
		char color[32];
		unsigned int col;
		sscanf(scummvm_icon[1 + i], "%c c %s", &code, color);
		if (!strcmp(color, "None"))
			col = 0x00000000;
		else if (!strcmp(color, "black"))
			col = 0xFF000000;
		else if (color[0] == '#') {
			sscanf(color + 1, "%06x", &col);
			col |= 0xFF000000;
		} else {
			warning("Could not load the icon (%d %s - %s) ", code, color, scummvm_icon[1 + i]);
			return;
		}
		
		rgba[code] = col;
	}
	memset(mask, 0, sizeof(mask));
	for (h = 0; h < 32; h++) {
		const char *line = scummvm_icon[1 + ncols + h];
		for (w = 0; w < 32; w++) {
			icon[w + 32 * h] = rgba[(int)line[w]];
			if (rgba[(int)line[w]] & 0xFF000000) {
				mask[h][w >> 3] |= 1 << (7 - (w & 0x07));
			}
		}
	}

	SDL_Surface *sdl_surf = SDL_CreateRGBSurfaceFrom(icon, 32, 32, 32, 32 * 4, 0xFF0000, 0x00FF00, 0x0000FF, 0xFF000000);
	SDL_WM_SetIcon(sdl_surf, (unsigned char *) mask);
	SDL_FreeSurface(sdl_surf);
}

OSystem::MutexRef OSystem_SDL_Common::create_mutex(void) {
	return (MutexRef) SDL_CreateMutex();
}

void OSystem_SDL_Common::lock_mutex(MutexRef mutex) {
	SDL_mutexP((SDL_mutex *) mutex);
}

void OSystem_SDL_Common::unlock_mutex(MutexRef mutex) {
	SDL_mutexV((SDL_mutex *) mutex);
}

void OSystem_SDL_Common::delete_mutex(MutexRef mutex) {
	SDL_DestroyMutex((SDL_mutex *) mutex);
}

void OSystem_SDL_Common::show_overlay() {
	// hide the mouse
	undraw_mouse();

	_overlayVisible = true;
	clear_overlay();
}

void OSystem_SDL_Common::hide_overlay() {
	// hide the mouse
	undraw_mouse();

	_overlayVisible = false;
	_forceFull = true;
}

void OSystem_SDL_Common::clear_overlay() {
	if (!_overlayVisible)
		return;
	
	StackLock lock(_graphicsMutex, this);	// Lock the mutex until this function ends
	
	// hide the mouse
	undraw_mouse();

	// Clear the overlay by making the game screen "look through" everywhere.
	SDL_Rect src, dst;
	src.x = src.y = 0;
	dst.x = dst.y = 1;
	src.w = dst.w = _screenWidth;
	src.h = dst.h = _screenHeight;
	if (SDL_BlitSurface(_screen, &src, _tmpscreen, &dst) != 0)
		error("SDL_BlitSurface failed: %s", SDL_GetError());

	_forceFull = true;
}

int16 OSystem_SDL_Common::get_height() {
	return _screenHeight;
}

int16 OSystem_SDL_Common::get_width() {
	return _screenWidth;
}

void OSystem_SDL_Common::grab_overlay(int16 *buf, int pitch) {
	if (!_overlayVisible)
		return;

	if (_tmpscreen == NULL)
		return;

	// hide the mouse
	undraw_mouse();

	if (SDL_LockSurface(_tmpscreen) == -1)
		error("SDL_LockSurface failed: %s.\n", SDL_GetError());

	int16 *src = (int16 *)_tmpscreen->pixels + _tmpScreenWidth + 1;
	int h = _screenHeight;
	do {
		memcpy(buf, src, _screenWidth*2);
		src += _tmpScreenWidth;
		buf += pitch;
	} while (--h);

	SDL_UnlockSurface(_tmpscreen);
}

void OSystem_SDL_Common::copy_rect_overlay(const int16 *buf, int pitch, int x, int y, int w, int h) {
	if (!_overlayVisible)
		return;

	if (_tmpscreen == NULL)
		return;

	// Clip the coordinates
	if (x < 0) {
		w += x;
		buf -= x;
		x = 0;
	}

	if (y < 0) {
		h += y; buf -= y * pitch;
		y = 0;
	}

	if (w > _screenWidth - x) {
		w = _screenWidth - x;
	}

	if (h > _screenHeight-y) {
		h = _screenHeight - y;
	}

	if (w <= 0 || h <= 0)
		return;

	// Mark the modified region as dirty
	cksum_valid = false;
	add_dirty_rect(x, y, w, h);

	/* FIXME: undraw mouse only if the draw rect intersects with the mouse rect */
	undraw_mouse();

	if (SDL_LockSurface(_tmpscreen) == -1)
		error("SDL_LockSurface failed: %s.\n", SDL_GetError());

	int16 *dst = (int16 *)_tmpscreen->pixels + (y + 1) * _tmpScreenWidth + (x + 1);
	do {
		memcpy(dst, buf, w * 2);
		dst += _tmpScreenWidth;
		buf += pitch;
	} while (--h);

	SDL_UnlockSurface(_tmpscreen);
}

void OSystem_SDL_Common::set_palette(const byte *colors, uint start, uint num) {
	const byte *b = colors;
	uint i;
	SDL_Color *base = _currentPalette + start;
	for (i = 0; i < num; i++) {
		base[i].r = b[0];
		base[i].g = b[1];
		base[i].b = b[2];
		b += 4;
	}

	if (start < _paletteDirtyStart)
		_paletteDirtyStart = start;

	if (start + num > _paletteDirtyEnd)
		_paletteDirtyEnd = start + num;
}

int16 OSystem_SDL_Common::RGBToColor(uint8 r, uint8 g, uint8 b) {
	return SDL_MapRGB(_tmpscreen->format, r, g, b);
}

void OSystem_SDL_Common::colorToRGB(int16 color, uint8 &r, uint8 &g, uint8 &b) {
	SDL_GetRGB(color, _tmpscreen->format, &r, &g, &b);
}
