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

#include "sdl-common.h"
#include "sound/mididrv.h"
#include "gameDetector.h"
#include "scaler.h"
#include "scumm.h"	// Only #included for error() and warning()

#include "scummvm.xpm"


bool atexit_proc_installed = false;
void atexit_proc() {
	SDL_ShowCursor(SDL_ENABLE);
	SDL_Quit();
}

OSystem *OSystem_SDL_create(int gfx_mode, bool full_screen) {
	return OSystem_SDL_Common::create(gfx_mode, full_screen);
}

OSystem *OSystem_SDL_Common::create(int gfx_mode, bool full_screen) {
	OSystem_SDL_Common *syst = OSystem_SDL_Common::create();
	syst->_mode = gfx_mode;
	syst->_full_screen = full_screen;

	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER) ==-1) {
		error("Could not initialize SDL: %s.\n", SDL_GetError());
	}

#ifdef WIN32				  // Use waveout on win32, not
	SDL_AudioInit("waveout"); // dsound - unfortunatly dsound
#endif						  // doesn't do COOPERATIVE mode

	SDL_ShowCursor(SDL_DISABLE);

	// Setup the icon
	syst->setup_icon();

#ifndef MACOSX		// Work around a bug in OS X
	// Clean up on exit
	atexit_proc_installed = true;
 	atexit(atexit_proc);
#endif

	return syst;
}

void OSystem_SDL_Common::set_timer(int timer, int (*callback)(int)) {
	SDL_SetTimer(timer, (SDL_TimerCallback) callback);
}

void OSystem_SDL_Common::init_size(uint w, uint h) {
	//if (w != SCREEN_WIDTH && h != SCREEN_HEIGHT)
	//	error("320x200 is the only game resolution supported");

	SCREEN_WIDTH = w;
	SCREEN_HEIGHT = h;
	CKSUM_NUM = (SCREEN_WIDTH*SCREEN_HEIGHT/(8*8));
	/* allocate palette, it needs to be persistent across
	 * driver changes, so i'll alloc it here */
	_cur_pal = (SDL_Color*)calloc(sizeof(SDL_Color), 256);

	dirty_rect_list = (SDL_Rect*)calloc(NUM_DIRTY_RECT, sizeof(SDL_Rect));
	_mouse_backup = (byte*)malloc(MAX_MOUSE_W * MAX_MOUSE_H * MAX_SCALING);
	dirty_checksums = (uint32*)calloc(CKSUM_NUM*2, sizeof(uint32));

	load_gfx_mode();

#ifdef MACOSX		// Work around a bug in OS X 10.1 related to OpenGL in windowed mode
	if (!atexit_proc_installed) {
		atexit_proc_installed = true;
		atexit(atexit_proc);
	}
#endif
}

void OSystem_SDL_Common::copy_rect(const byte *buf, int pitch, int x, int y, int w, int h) {
	if (sdl_screen == NULL)
		return;

	if (pitch == SCREEN_WIDTH && x==0 && y==0 && w==SCREEN_WIDTH && h==SCREEN_HEIGHT && _mode_flags&DF_WANT_RECT_OPTIM) {
		/* Special, optimized case for full screen updates.
		 * It tries to determine what areas were actually changed,
		 * and just updates those, on the actual display. */
		add_dirty_rgn_auto(buf);
	} else {
		/* Clip the coordinates */
		if (x < 0) { w+=x; buf-=x; x = 0; }
		if (y < 0) { h+=y; buf-=y*pitch; y = 0; }
		if (w > SCREEN_WIDTH-x) { w = SCREEN_WIDTH - x; }
		if (h > SCREEN_HEIGHT-y) { h = SCREEN_HEIGHT - y; }
			
		if (w <= 0 || h <= 0)
			return;

		cksum_valid = false;
		add_dirty_rect(x, y, w, h);
	}

	/* FIXME: undraw mouse only if the draw rect intersects with the mouse rect */
	if (_mouse_drawn)
		undraw_mouse();

	if (SDL_LockSurface(sdl_screen) == -1)
		error("SDL_LockSurface failed: %s.\n", SDL_GetError());

	byte *dst = (byte *)sdl_screen->pixels + y * SCREEN_WIDTH + x;
	do {
		memcpy(dst, buf, w);
		dst += SCREEN_WIDTH;
		buf += pitch;
	} while (--h);

	SDL_UnlockSurface(sdl_screen);
}


void OSystem_SDL_Common::add_dirty_rect(int x, int y, int w, int h) {
	if (force_full)
		return;

	if (num_dirty_rects == NUM_DIRTY_RECT)
		force_full = true;
	else {
		SDL_Rect *r = &dirty_rect_list[num_dirty_rects++];
		
		/* Update the dirty region by 1 pixel for graphics drivers
		 * that "smear" the screen */
		if (_mode_flags & DF_UPDATE_EXPAND_1_PIXEL) {
			x--;
			y--;
			w+=2;
			h+=2;
		}

		/* clip */
		if (x < 0) { w+=x; x=0; }
		if (y < 0) { h+=y; y=0; }
		if (w > SCREEN_WIDTH-x) { w = SCREEN_WIDTH - x; }
		if (h > SCREEN_HEIGHT-y) { h = SCREEN_HEIGHT - y; }
	
		r->x = x;
		r->y = y;
		r->w = w;
		r->h = h;
	}
}

#define ROL(a,n) a = (a<<(n)) | (a>>(32-(n)))
#define DOLINE(x) a ^= ((uint32*)buf)[0+(x)*(SCREEN_WIDTH/4)]; b ^= ((uint32*)buf)[1+(x)*(SCREEN_WIDTH/4)]
void OSystem_SDL_Common::mk_checksums(const byte *buf) {
	uint32 *sums = dirty_checksums;
	uint x,y;
	const uint last_x = (uint)SCREEN_WIDTH/8;
	const uint last_y = (uint)SCREEN_HEIGHT/8;

	/* the 8x8 blocks in buf are enumerated starting in the top left corner and
	 * reading each line at a time from left to right */
	for(y=0; y != last_y; y++, buf+=SCREEN_WIDTH*(8-1))
		for(x=0; x != last_x; x++, buf+=8) {
			uint32 a = x;
			uint32 b = y;

			DOLINE(0); ROL(a,13); ROL(b,11);
			DOLINE(2); ROL(a,13); ROL(b,11);
			DOLINE(4); ROL(a,13); ROL(b,11);
			DOLINE(6); ROL(a,13); ROL(b,11);

			a*=0xDEADBEEF;
			b*=0xBAADF00D;

			DOLINE(1); ROL(a,13); ROL(b,11);
			DOLINE(3); ROL(a,13); ROL(b,11);
			DOLINE(5); ROL(a,13); ROL(b,11);
			DOLINE(7); ROL(a,13); ROL(b,11);

			/* output the checksum for this block */
			*sums++=a+b;
		}
}
#undef DOLINE
#undef ROL


void OSystem_SDL_Common::add_dirty_rgn_auto(const byte *buf) {
	assert( ((uint32)buf & 3) == 0);
	
	/* generate a table of the checksums */
	mk_checksums(buf);

	if (!cksum_valid) {
		force_full = true;
		cksum_valid = true;
	}

	/* go through the checksum list, compare it with the previous checksums,
		 and add all dirty rectangles to a list. try to combine small rectangles
		 into bigger ones in a simple way */
	if (!force_full) {
		int x,y,w;
		uint32 *ck = dirty_checksums;
		
		for(y=0; y!=SCREEN_HEIGHT/8; y++) {
			for(x=0; x!=SCREEN_WIDTH/8; x++,ck++) {
				if (ck[0] != ck[CKSUM_NUM]) {
					/* found a dirty 8x8 block, now go as far to the right as possible,
						 and at the same time, unmark the dirty status by setting old to new. */
					w=0;
					do {
						ck[w+CKSUM_NUM] = ck[w];
						w++;
					} while (x+w != SCREEN_WIDTH/8 && ck[w] != ck[w+CKSUM_NUM]);
					
					add_dirty_rect(x*8, y*8, w*8, 8);

					if (force_full)
						goto get_out;
				}
			}
		}
	} else {
		get_out:;
		/* Copy old checksums to new */
		memcpy(dirty_checksums + CKSUM_NUM, dirty_checksums, CKSUM_NUM * sizeof(uint32));
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
				if (time > km.x_down_time + km.delay_time*12) {
					if (km.x_vel > 0)
						km.x_vel++;
					else
						km.x_vel--;
				} else if (time > km.x_down_time + km.delay_time*8) {
					if (km.x_vel > 0)
						km.x_vel = 5;
					else
						km.x_vel = -5;
				}
			}
			if (km.y_down_count) {
				if (time > km.y_down_time + km.delay_time*12) {
					if (km.y_vel > 0)
						km.y_vel++;
					else
						km.y_vel--;
				} else if (time > km.y_down_time + km.delay_time*8) {
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
	if (_mouse_visible == visible)
		return visible;
	
	bool last = _mouse_visible;
	_mouse_visible = visible;

	if (visible)
		draw_mouse();
	else
		undraw_mouse();

	return last;
}
	
void OSystem_SDL_Common::set_mouse_pos(int x, int y) {
	if (x != _mouse_cur_state.x || y != _mouse_cur_state.y) {
		_mouse_cur_state.x = x;
		_mouse_cur_state.y = y;
		undraw_mouse();
	}
}
	
void OSystem_SDL_Common::set_mouse_cursor(const byte *buf, uint w, uint h, int hotspot_x, int hotspot_y) {
	_mouse_cur_state.w = w;
	_mouse_cur_state.h = h;

	_mouse_hotspot_x = hotspot_x;
	_mouse_hotspot_y = hotspot_y;

	_mouse_data = (byte*)buf;

	undraw_mouse();
}
	
void OSystem_SDL_Common::set_shake_pos(int shake_pos) {
	_new_shake_pos = shake_pos;
}
		
uint32 OSystem_SDL_Common::get_msecs() {
	return SDL_GetTicks();	
}
	
void OSystem_SDL_Common::delay_msecs(uint msecs) {
	SDL_Delay(msecs);
}
	
void *OSystem_SDL_Common::create_thread(ThreadProc *proc, void *param) {
	return SDL_CreateThread(proc, param);
}

int mapKey(int key, byte mod)
{
	if (key >= SDLK_F1 && key <= SDLK_F9) {
		return key - SDLK_F1 + 315;
	} else if (key >= 'a' && key <= 'z' && mod & KMOD_SHIFT) {
		key &= ~0x20;
	} else if (key >= SDLK_NUMLOCK && key <= SDLK_EURO)
		return 0;
	return key;
}
	
bool OSystem_SDL_Common::poll_event(Event *event) {
	SDL_Event ev;
	kbd_mouse();

	for(;;) {
		if (!SDL_PollEvent(&ev))
			return false;

		switch(ev.type) {
		case SDL_KEYDOWN: {
				byte b = 0;
				if (ev.key.keysym.mod & KMOD_SHIFT) b |= KBD_SHIFT;
				if (ev.key.keysym.mod & KMOD_CTRL) b |= KBD_CTRL;
				if (ev.key.keysym.mod & KMOD_ALT) b |= KBD_ALT;
				event->kbd.flags = b;

				/* internal keypress? */				
				if (b == KBD_ALT && ev.key.keysym.sym==SDLK_RETURN) {
					property(PROP_TOGGLE_FULLSCREEN, NULL);
					break;
				}

				if ((b == KBD_CTRL && ev.key.keysym.sym=='z') || (b == KBD_ALT && ev.key.keysym.sym=='x')) {
					quit();
					break;
				}
				if (b == (KBD_CTRL|KBD_ALT) && 
				    (ev.key.keysym.sym>='1') && (ev.key.keysym.sym<='7')) {
					Property prop;
					prop.gfx_mode = ev.key.keysym.sym - '1';
					property(PROP_SET_GFX_MODE, &prop);
					break;
				}
				#ifdef QTOPIA
				// quit on fn+backspace on zaurus
				if (ev.key.keysym.sym==127) {
					quit();
					break;
				}
				// map menu key (f11) to f5 (scumm menu)
				if (ev.key.keysym.sym==292) {
					event->event_code = EVENT_KEYDOWN;
					event->kbd.keycode = 286;
					event->kbd.ascii = mapKey(286, ev.key.keysym.mod);
				}
				// map center (space) to tab (default action )
				// i wanted to map the calendar button but the calendar comes up
				//
				else if (ev.key.keysym.sym==32) {
					event->event_code = EVENT_KEYDOWN;
					event->kbd.keycode = 9;
					event->kbd.ascii = mapKey(9, ev.key.keysym.mod);
				}
				// since we stole space (pause) above we'll rebind it to the tab key on the keyboard
				else if (ev.key.keysym.sym==9) {
					event->event_code = EVENT_KEYDOWN;
					event->kbd.keycode = 32;
					event->kbd.ascii = mapKey(32, ev.key.keysym.mod);
				}
				else {
				// let the events fall through if we didn't change them, this may not be the best way to
				// set it up, but i'm not sure how sdl would like it if we let if fall through then redid it though.
				// and yes i have an huge terminal size so i dont wrap soon enough.
					event->event_code = EVENT_KEYDOWN;
					event->kbd.keycode = ev.key.keysym.sym;
					event->kbd.ascii = mapKey(ev.key.keysym.sym, ev.key.keysym.mod);
				}
				#endif
				#ifndef QTOPIA
				event->event_code = EVENT_KEYDOWN;
				event->kbd.keycode = ev.key.keysym.sym;
				event->kbd.ascii = mapKey(ev.key.keysym.sym, ev.key.keysym.mod);
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
			}

		case SDL_KEYUP: {
			event->event_code = EVENT_KEYUP;
			event->kbd.keycode = ev.key.keysym.sym;
			event->kbd.ascii = mapKey(ev.key.keysym.sym, ev.key.keysym.mod);

				switch(ev.key.keysym.sym){
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
			}

		case SDL_MOUSEMOTION:
			event->event_code = EVENT_MOUSEMOVE;
			km.x = event->mouse.x = ev.motion.x;
			km.y = event->mouse.y = ev.motion.y;

			event->mouse.x /= scaling;
			event->mouse.y /= scaling;

			return true;

		case SDL_MOUSEBUTTONDOWN:
			if (ev.button.button == SDL_BUTTON_LEFT)
				event->event_code = EVENT_LBUTTONDOWN;
			else if (ev.button.button == SDL_BUTTON_RIGHT)
				event->event_code = EVENT_RBUTTONDOWN;
			else
				break;
			km.x = event->mouse.x = ev.motion.x;
			km.y = event->mouse.y = ev.motion.y;
			event->mouse.x /= scaling;
			event->mouse.y /= scaling;

			return true;

		case SDL_MOUSEBUTTONUP:
			if (ev.button.button == SDL_BUTTON_LEFT)
				event->event_code = EVENT_LBUTTONUP;
			else if (ev.button.button == SDL_BUTTON_RIGHT)
				event->event_code = EVENT_RBUTTONUP;
			else
				break;
			event->mouse.x = ev.button.x;
			event->mouse.y = ev.button.y;
			event->mouse.x /= scaling;
			event->mouse.y /= scaling;
			return true;

		case SDL_QUIT:
			quit();
		}
	}
}
	
bool OSystem_SDL_Common::set_sound_proc(void *param, SoundProc *proc, byte format) {
	SDL_AudioSpec desired;

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


/* retrieve the 320x200 bitmap currently being displayed */
void OSystem_SDL_Common::get_320x200_image(byte *buf) {
	/* make sure the mouse is gone */
	undraw_mouse();
	
	if (SDL_LockSurface(sdl_screen) == -1)
		error("SDL_LockSurface failed: %s.\n", SDL_GetError());

	memcpy(buf, sdl_screen->pixels, SCREEN_WIDTH*SCREEN_HEIGHT);

	SDL_UnlockSurface(sdl_screen);
}

uint32 OSystem_SDL_Common::property(int param, Property *value) {
	switch(param) {

	case PROP_GET_FULLSCREEN:
		return _full_screen;

	case PROP_SET_WINDOW_CAPTION:
		SDL_WM_SetCaption(value->caption, value->caption);
		return 1;

	case PROP_OPEN_CD:
		if (SDL_InitSubSystem(SDL_INIT_CDROM) == -1)
			cdrom = NULL;
		else {
			cdrom = SDL_CDOpen(value->cd_num);
			/* Did if open? Check if cdrom is NULL */
			if (!cdrom) {
				warning("Couldn't open drive: %s\n", SDL_GetError());
			}
		}
		break;

	case PROP_SET_GFX_MODE:
		if (value->gfx_mode >= 7)
			return 0;

		_mode = value->gfx_mode;
		hotswap_gfx_mode();

		return 1;

	case PROP_SHOW_DEFAULT_CURSOR:
		SDL_ShowCursor(value->show_cursor ? SDL_ENABLE : SDL_DISABLE);		
		break;

	case PROP_GET_SAMPLE_RATE:
		return SAMPLES_PER_SEC;
	}

	return 0;
}
		
void OSystem_SDL_Common::quit() {
	if(cdrom) {
		SDL_CDStop(cdrom);
		SDL_CDClose(cdrom);
	}
	unload_gfx_mode();		
	exit(1);
}

void OSystem_SDL_Common::draw_mouse() {
	if (_mouse_drawn || !_mouse_visible)
		return;

	int x = _mouse_cur_state.x - _mouse_hotspot_x;
	int y = _mouse_cur_state.y - _mouse_hotspot_y;
	int w = _mouse_cur_state.w;
	int h = _mouse_cur_state.h;
	byte color;
	byte *src = _mouse_data;		// Image representing the mouse
	byte *bak = _mouse_backup;		// Surface used to backup the area obscured by the mouse
	byte *dst;					// Surface we are drawing into

	// clip the mouse rect, and addjust the src pointer accordingly
	if (x < 0) {
		w += x;
		src -= x;
		x = 0;
	}
	if (y < 0) {
		h += y;
		src -= y * _mouse_cur_state.w;
		y = 0;
	}
	if (w > SCREEN_WIDTH - x)
		w = SCREEN_WIDTH - x;
	if (h > SCREEN_HEIGHT - y)
		h = SCREEN_HEIGHT - y;

	// Store the bounding box so that undraw mouse can restore the area the
	// mouse currently covers to its original content.
	_mouse_old_state.x = x;
	_mouse_old_state.y = y;
	_mouse_old_state.w = w;
	_mouse_old_state.h = h;

	// Quick check to see if anything has to be drawn at all
	if (w <= 0 || h <= 0)
		return;

	// Draw the mouse cursor; backup the covered area in "bak"

	if (SDL_LockSurface(sdl_screen) == -1)
		error("SDL_LockSurface failed: %s.\n", SDL_GetError());

	add_dirty_rect(x, y, w, h);

	dst = (byte *)sdl_screen->pixels + y * SCREEN_WIDTH + x;
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
		src += _mouse_cur_state.w - w;
		bak += MAX_MOUSE_W - w;
		dst += SCREEN_WIDTH - w;
		h--;
	}

	SDL_UnlockSurface(sdl_screen);
	
	// Finally, set the flag to indicate the mouse has been drawn
	_mouse_drawn = true;
}

void OSystem_SDL_Common::undraw_mouse() {
	if (!_mouse_drawn)
		return;
	_mouse_drawn = false;

	if (SDL_LockSurface(sdl_screen) == -1)
		error("SDL_LockSurface failed: %s.\n", SDL_GetError());

	byte *dst, *bak = _mouse_backup;
	const int old_mouse_x = _mouse_old_state.x;
	const int old_mouse_y = _mouse_old_state.y;
	const int old_mouse_w = _mouse_old_state.w;
	const int old_mouse_h = _mouse_old_state.h;
	int x,y;

	// No need to do clipping here, since draw_mouse() did that already

	dst = (byte *)sdl_screen->pixels + old_mouse_y * SCREEN_WIDTH + old_mouse_x;
	for (y = 0; y < old_mouse_h; ++y, bak += MAX_MOUSE_W, dst += SCREEN_WIDTH) {
		for (x = 0; x < old_mouse_w; ++x) {
			dst[x] = bak[x];
		}
	}

	add_dirty_rect(old_mouse_x, old_mouse_y, old_mouse_w, old_mouse_h);

	SDL_UnlockSurface(sdl_screen);
}

void OSystem_SDL_Common::stop_cdrom() {	/* Stop CD Audio in 1/10th of a second */
	cd_stop_time = SDL_GetTicks() + 100;
	cd_num_loops = 0;

}

void OSystem_SDL_Common::play_cdrom(int track, int num_loops, int start_frame, int end_frame) {
	if (!num_loops && !start_frame)
		return;

	if (!cdrom)
		return;
	
	if (end_frame > 0)
		end_frame+=5;

	cd_track = track;
	cd_num_loops = num_loops;
	cd_start_frame = start_frame;

	SDL_CDStatus(cdrom);	
	SDL_CDPlayTracks(cdrom, track, start_frame, 0, end_frame);
	cd_end_frame = end_frame;
	cd_stop_time = 0;
	cd_end_time = SDL_GetTicks() + cdrom->track[track].length * 1000 / CD_FPS;
}

bool OSystem_SDL_Common::poll_cdrom() {
	if (!cdrom)
		return false;

	return (cd_num_loops != 0 && (SDL_GetTicks() < cd_end_time || SDL_CDStatus(cdrom) != CD_STOPPED));
}

void OSystem_SDL_Common::update_cdrom() {
	if (!cdrom)
		return;
		
	if (cd_stop_time != 0 && SDL_GetTicks() >= cd_stop_time) {
		SDL_CDStop(cdrom);
		cd_num_loops = 0;
		cd_stop_time = 0;
		return;
	}

	if (cd_num_loops == 0 || SDL_GetTicks() < cd_end_time)
		return;

	if (cd_num_loops != 1 && SDL_CDStatus(cdrom) != CD_STOPPED) {
		// Wait another second for it to be done
		cd_end_time += 1000;
		return;
	}

	if (cd_num_loops > 0)
		cd_num_loops--;

	if (cd_num_loops != 0) {
		SDL_CDPlayTracks(cdrom, cd_track, cd_start_frame, 0, cd_end_frame);
		cd_end_time = SDL_GetTicks() + cdrom->track[cd_track].length * 1000 / CD_FPS;
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
		char *line = scummvm_icon[1 + ncols + h];
		for (w = 0; w < 32; w++) {
			icon[w + 32 * h] = rgba[line[w]];
			if (rgba[line[w]] & 0xFF000000) {
				mask[h][w >> 3] |= 1 << (7 - (w & 0x07));
			}
		}
	}

	SDL_Surface *sdl_surf = SDL_CreateRGBSurfaceFrom(icon, 32, 32, 32, 32 * 4, 0xFF0000, 0x00FF00, 0x0000FF, 0xFF000000);
	SDL_WM_SetIcon(sdl_surf, (unsigned char *) mask);
}

void *OSystem_SDL_Common::create_mutex(void) {
	return (void *) SDL_CreateMutex();
}

void OSystem_SDL_Common::lock_mutex(void *mutex) {
	SDL_mutexP((SDL_mutex *) mutex);
}

void OSystem_SDL_Common::unlock_mutex(void *mutex) {
	SDL_mutexV((SDL_mutex *) mutex);
}

void OSystem_SDL_Common::delete_mutex(void *mutex) {
	SDL_DestroyMutex((SDL_mutex *) mutex);
}
