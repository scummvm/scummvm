#include "stdafx.h"
#include "scumm.h"
#include "SDL_thread.h"
#include "gameDetector.h"

#include "cdmusic.h"
#include "mp3_cd.h"
#include <SDL.h>



class OSystem_SDL : public OSystem {
public:
	// Set colors of the palette
	void set_palette(const byte *colors, uint start, uint num);

	// Set the size of the video bitmap.
	// Typically, 320x200
	void init_size(uint w, uint h, byte sound);

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
	void *create_thread(ThreadProc *proc, void *param);
	
	// Get the next event.
	// Returns true if an event was retrieved.	
	bool poll_event(Event *event);
	
	// Set function that generates samples 
	void set_sound_proc(void *param, SoundProc *proc);
		
	// Quit
	void quit();

	// Set a parameter
	uint32 set_param(int param, uint32 value);

	static OSystem *create(int gfx_mode, bool full_screen);

private:
	typedef void TwoXSaiProc(uint8 *srcPtr, uint32 srcPitch, uint8 *deltaPtr,
								uint8 *dstPtr, uint32 dstPitch, int width, int height);

	SDL_Surface *sdl_screen;
	SDL_Surface *sdl_hwscreen;
	SDL_Surface *sdl_tmpscreen;
	SDL_CD *cdrom;

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

		MAX_MOUSE_W = 40,
		MAX_MOUSE_H = 40,
		MAX_SCALING = 3,

		TMP_SCREEN_OFFS = 320*2 + 8,
	};

	SDL_Rect *dirty_rect_list;
	int num_dirty_rects;
	uint32 *dirty_checksums;

	int scaling;

	SoundProc *_sound_proc;
	void *_sound_param;

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

	TwoXSaiProc *_sai_func;

	SDL_Color *_cur_pal;

	void add_dirty_rgn_auto(const byte *buf);
	void mk_checksums(const byte *buf);

	static void fill_sound(void *userdata, Uint8 * stream, int len);
	
	void add_dirty_rect(int x, int y, int w, int h);

	void draw_mouse();
	void undraw_mouse();

	void load_gfx_mode();
	void unload_gfx_mode();

	void hotswap_gfx_mode();

	void get_320x200_image(byte *buf);
};

int Init_2xSaI (uint32 BitFormat);
void _2xSaI(uint8 *srcPtr, uint32 srcPitch, uint8 *deltaPtr, uint8 *dstPtr,
						uint32 dstPitch, int width, int height);
void Super2xSaI(uint8 *srcPtr, uint32 srcPitch, uint8 *deltaPtr,
								uint8 *dstPtr, uint32 dstPitch, int width, int height);
void SuperEagle(uint8 *srcPtr, uint32 srcPitch, uint8 *deltaPtr,
								uint8 *dstPtr, uint32 dstPitch, int width, int height);


void atexit_proc() {
	SDL_ShowCursor(SDL_ENABLE);
	SDL_Quit();
}

OSystem *OSystem_SDL::create(int gfx_mode, bool full_screen) {
	OSystem_SDL *syst = new OSystem_SDL();
	syst->_mode = gfx_mode;
	syst->_full_screen = full_screen;

	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) ==-1) {
		error("Could not initialize SDL: %s.\n", SDL_GetError());
	}

	SDL_ShowCursor(SDL_DISABLE);

	/* Clean up on exit */
 	atexit(atexit_proc);

	return syst;
}

OSystem *OSystem_SDL_create(int gfx_mode, bool full_screen) {
	return OSystem_SDL::create(gfx_mode, full_screen);
}

void OSystem_SDL::set_palette(const byte *colors, uint start, uint num) {
	const byte *b = colors;
	uint i;
	SDL_Color *base = _cur_pal + start;
	for(i=0;i!=num;i++) {
		base[i].r = b[0];
		base[i].g = b[1];
		base[i].b = b[2];
		b += 4;
	}

	if (_mode_flags & DF_FORCE_FULL_ON_PALETTE)
		force_full = true;
	SDL_SetColors(sdl_screen, base, start, num);
}

void OSystem_SDL::fill_sound(void *userdata, Uint8 * stream, int len) {
	OSystem_SDL *os = (OSystem_SDL*)userdata;
	if (os->_sound_proc)
		os->_sound_proc(os->_sound_param, (int16*)stream, len>>1);
	else
		memset(stream, 0x0, len);
}

void OSystem_SDL::load_gfx_mode() {
	force_full = true;
	scaling = 1;
	_internal_scaling = 1;
	_mode_flags = 0;

	_sai_func = NULL;
	switch(_mode) {
	case GFX_2XSAI:
		_sai_func = _2xSaI;
		break;
	case GFX_SUPER2XSAI:
		_sai_func = Super2xSaI;
		break;
	case GFX_SUPEREAGLE:
		_sai_func = SuperEagle;
		break;

	case GFX_DOUBLESIZE:
		if (_full_screen) {
			warning("full screen in useless in doublesize mode, reverting to normal mode");
			goto normal_mode;
		}
		scaling = 2;
		_internal_scaling = 2;
		_mode_flags = DF_WANT_RECT_OPTIM;
		
		sdl_hwscreen = sdl_screen = SDL_SetVideoMode(640, 400, 8, SDL_SWSURFACE);
		if (sdl_screen == NULL)
			error("sdl_screen failed");
		break;

	case GFX_TRIPLESIZE:
		if (_full_screen) {
			warning("full screen in useless in triplesize mode, reverting to normal mode");
			goto normal_mode;
		}
		scaling = 3;
		_internal_scaling = 3;
		_mode_flags = DF_WANT_RECT_OPTIM;

		sdl_hwscreen = sdl_screen = SDL_SetVideoMode(960, 600, 8, 
			_full_screen ? (SDL_FULLSCREEN|SDL_SWSURFACE) : SDL_SWSURFACE
		);
		if (sdl_screen == NULL)
			error("sdl_screen failed");


		break;

	case GFX_NORMAL:
normal_mode:;
		_mode_flags = DF_WANT_RECT_OPTIM;
		sdl_hwscreen = sdl_screen = SDL_SetVideoMode(320, 200, 8, 
			_full_screen ? (SDL_FULLSCREEN|SDL_SWSURFACE) : SDL_SWSURFACE
		);
		if (sdl_screen == NULL)
			error("sdl_screen failed");
		break;
	}

	if (_sai_func) {
		uint16 *tmp_screen = (uint16*)calloc(320*204 + 16,sizeof(uint16));
		_mode_flags = DF_FORCE_FULL_ON_PALETTE | DF_WANT_RECT_OPTIM | DF_2xSAI | DF_SEPARATE_HWSCREEN | DF_UPDATE_EXPAND_1_PIXEL;

		Init_2xSaI(565);
		sdl_screen = SDL_CreateRGBSurface(SDL_SWSURFACE, 320, 200, 8, 0, 0, 0, 0);
		if (sdl_screen == NULL)
			error("SDL_CreateRGBSurface(SDL_SWSURFACE, 320, 200, 8, 0, 0, 0, 0) failed");

		sdl_hwscreen = SDL_SetVideoMode(640, 400, 16, 
			_full_screen ? (SDL_FULLSCREEN|SDL_SWSURFACE) : SDL_SWSURFACE
		);
		if (sdl_hwscreen == NULL)
			error("sdl_hwscreen failed");

		/* Need some extra bytes around when using 2XSAI */
		sdl_tmpscreen = SDL_CreateRGBSurfaceFrom(tmp_screen + TMP_SCREEN_OFFS, 320, 200, 16,
			320*2, 0,0,0,0);
		if (sdl_tmpscreen == NULL)
			error("sdl_tmpscreen failed");

		scaling = 2;		
	}
}

void OSystem_SDL::unload_gfx_mode() {
	SDL_Surface *surf;

	surf=sdl_screen; sdl_screen=NULL; SDL_FreeSurface(surf);

	if (_mode_flags & DF_SEPARATE_HWSCREEN) {
		surf=sdl_hwscreen; sdl_hwscreen=NULL; SDL_FreeSurface(surf);
	}

	surf = sdl_tmpscreen; sdl_tmpscreen=NULL;
	if(surf) { 
		free((uint16*)surf->pixels - (int)TMP_SCREEN_OFFS);
		SDL_FreeSurface(surf);
	}
}

void OSystem_SDL::init_size(uint w, uint h, byte sound) {
	SDL_AudioSpec desired;

	if (w != SCREEN_WIDTH && h != SCREEN_HEIGHT)
		error("320x200 is the only game resolution supported");

	/* init sound */
	if (sound != SOUND_NONE) {
		desired.freq = SAMPLES_PER_SEC;
		desired.format = sound==SOUND_8BIT ? AUDIO_U8 : AUDIO_S16SYS;
		desired.channels = 1;
		desired.samples = 2048;
		desired.callback = fill_sound;
		desired.userdata = this;
		SDL_OpenAudio(&desired, NULL);
		SDL_PauseAudio(0);
	}

	/* allocate palette, it needs to be persistent across
	 * driver changes, so i'll alloc it here */
	_cur_pal = (SDL_Color*)calloc(sizeof(SDL_Color), 256);

	dirty_rect_list = (SDL_Rect*)calloc(NUM_DIRTY_RECT, sizeof(SDL_Rect));
	_ms_backup = (byte*)malloc(MAX_MOUSE_W * MAX_MOUSE_H * MAX_SCALING);
	dirty_checksums = (uint32*)calloc(CKSUM_NUM*2, sizeof(uint32));

	load_gfx_mode();
}

void OSystem_SDL::copy_rect(const byte *buf, int pitch, int x, int y, int w, int h) {
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
		if (w >= SCREEN_WIDTH-x) { w = SCREEN_WIDTH - x; }
		if (h >= SCREEN_HEIGHT-y) { h = SCREEN_HEIGHT - y; }
			
		if (w<=0 || h<=0)
			return;

		/* FIXME: undraw mouse only if the draw rect intersects with the mouse rect */
		if (_mouse_drawn)
			undraw_mouse();
			
		cksum_valid = false;

		add_dirty_rect(x, y, w, h);
	}

	if (SDL_LockSurface(sdl_screen) == -1)
		error("SDL_LockSurface failed: %s.\n", SDL_GetError());

	byte *dst;

	switch(_internal_scaling) {
	case 1:
		dst = (byte *)sdl_screen->pixels + y * 320 + x;
		do {
			memcpy(dst, buf, w);
			dst += 320;
			buf += pitch;
		} while (--h);
		break;

	case 2:
		dst = (byte *)sdl_screen->pixels + y * 640 * 2 + x * 2;
		do {
			int i = 0;
			do {
				dst[i * 2] = dst[i * 2 + 1] = buf[i];
			} while (++i != w);
			memcpy(dst + 640, dst, w * 2);
			dst += 640 * 2;
			buf += pitch;
		} while (--h);
		break;

	case 3:
		dst = (byte *)sdl_screen->pixels + y * 960 * 3 + x * 3;
		do {
			int i = 0;
			do {
				dst[i * 3] = dst[i * 3 + 1] = dst[i * 3 + 2] = buf[i];
			} while (++i != w);
			memcpy(dst + 960, dst, w * 3);
			memcpy(dst + 960 + 960, dst, w * 3);
			dst += 960 * 3;
			buf += pitch;
		} while (--h);
		break;
	}	

	SDL_UnlockSurface(sdl_screen);
}


void OSystem_SDL::add_dirty_rect(int x, int y, int w, int h) {
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
		if (x<0) { w+=x; x=0; }
		if (y<0) { h+=y; y=0; }
		if (w>=SCREEN_WIDTH-x) { w=SCREEN_WIDTH-x; }
		if (h>=SCREEN_HEIGHT-y) { h=SCREEN_HEIGHT-y; }
	
		if (_internal_scaling != 1) {
			x *= _internal_scaling;
			y *= _internal_scaling;
			w *= _internal_scaling;
			h *= _internal_scaling;
		}

		r->x = x;
		r->y = y;
		r->w = w;
		r->h = h;
	}
}

#define ROL(a,n) a = (a<<(n)) | (a>>(32-(n)))
#define DOLINE(x) a ^= ((uint32*)buf)[0+(x)*(SCREEN_WIDTH/4)]; b ^= ((uint32*)buf)[1+(x)*(SCREEN_WIDTH/4)]
void OSystem_SDL::mk_checksums(const byte *buf) {
	uint32 *sums = dirty_checksums;
	uint x,y;

	/* the 8x8 blocks in buf are enumerated starting in the top left corner and
	 * reading each line at a time from left to right */
	for(y=0; y!=SCREEN_HEIGHT/8; y++,buf+=SCREEN_WIDTH*(8-1))
		for(x=0; x!=SCREEN_WIDTH/8; x++,buf+=8) {
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


void OSystem_SDL::add_dirty_rgn_auto(const byte *buf) {
	if (_mouse_drawn)
		undraw_mouse();

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
		uint x,y,w;
		uint32 *ck = dirty_checksums;
		SDL_Rect *dr = dirty_rect_list;
		
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

					/* add this rect to the dirty list. */
					if(dr==&dirty_rect_list[NUM_DIRTY_RECT-1]) {
						force_full=true;
						goto get_out;
					}
										
					dr->x = x*8;
					dr->y = y*8;
					dr->w = w*8;
					dr->h = 1*8;
					dr++;
				}
			}
		}
		num_dirty_rects = dr - dirty_rect_list;
	} else {
		get_out:;
		/* Copy old checksums to new */
		memcpy(dirty_checksums + CKSUM_NUM, dirty_checksums, CKSUM_NUM * sizeof(uint32));
	}
}

void OSystem_SDL::update_screen() {
	/* First make sure the mouse is drawn, if it should be drawn. */
	draw_mouse();
	
	/* force a full redraw, accomplish that by adding one big rect to the dirty
	 * rect list */
	if (force_full) {
		SDL_Rect *dr = dirty_rect_list;
		dr->x = 0;
		dr->y = 0;
		dr->w = SCREEN_WIDTH;
		dr->h = SCREEN_HEIGHT;
		num_dirty_rects = 1;
		force_full = false;
	}

	if (num_dirty_rects == 0 || sdl_hwscreen == NULL)
		return;
		
	if (_mode_flags & DF_2xSAI) {
		SDL_Rect *r;
		uint32 area = 0;

		SDL_Rect *dr = dirty_rect_list + num_dirty_rects;

		/* Convert appropriate parts of image into 16bpp */
		for(r=dirty_rect_list; r!=dr; r++) {
			if (SDL_BlitSurface(sdl_screen, r, sdl_tmpscreen, r) != 0)
				error("SDL_BlitSurface failed: %s", SDL_GetError());
		}

		SDL_LockSurface(sdl_tmpscreen);
		SDL_LockSurface(sdl_hwscreen);
	
		for(r=dirty_rect_list; r!=dr; r++) {
			/* Apply the 2xsai algorithm */
			_sai_func((byte*)sdl_tmpscreen->pixels + r->x*2 + r->y*640, 640, NULL, 
				(byte*)sdl_hwscreen->pixels + r->x*4 + r->y*640*4, 640*2, r->w, r->h);

			/* Calculate area */
			area += r->w * r->h;
			
			/* scale the rect to fit in SDL_UpdateRects */
			r->x <<= 1;
			r->y <<= 1;
			r->w <<= 1;
			r->h <<= 1;
		}

		SDL_UnlockSurface(sdl_tmpscreen);
		SDL_UnlockSurface(sdl_hwscreen);

		/* Call SDL update on the affected regions */
		SDL_UpdateRects(sdl_hwscreen, num_dirty_rects, dirty_rect_list);
#ifdef WIN32
		if (GetAsyncKeyState(VK_SHIFT)<0)
			printf("Update area %d pixels. %d%%\n", area, (area+(320*2)/2) / (320*2));
#endif
	} else {
		/* Call SDL update on the affected regions */
		SDL_UpdateRects(sdl_hwscreen, num_dirty_rects, dirty_rect_list);
	}
	
	num_dirty_rects = 0;
}

bool OSystem_SDL::show_mouse(bool visible) {
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
	
void OSystem_SDL::set_mouse_pos(int x, int y) {
	if (x != _ms_cur.x || y != _ms_cur.y) {
		_ms_cur.x = x;
		_ms_cur.y = y;
		undraw_mouse();
	}
}
	
void OSystem_SDL::set_mouse_cursor(const byte *buf, uint w, uint h, int hotspot_x, int hotspot_y) {
	_ms_cur.w = w;
	_ms_cur.h = h;

	_ms_hotspot_x = hotspot_x;
	_ms_hotspot_y = hotspot_y;

	_ms_buf = (byte*)buf;

	undraw_mouse();
}
	
void OSystem_SDL::set_shake_pos(int shake_pos) {
}
		
uint32 OSystem_SDL::get_msecs() {
	return SDL_GetTicks();	
}
	
void OSystem_SDL::delay_msecs(uint msecs) {
	SDL_Delay(msecs);
}
	
void *OSystem_SDL::create_thread(ThreadProc *proc, void *param) {
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
	
bool OSystem_SDL::poll_event(Event *event) {
	SDL_Event ev;

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
					set_param(PARAM_TOGGLE_FULLSCREEN, 0);
					break;
				}

				if (b == KBD_CTRL && ev.key.keysym.sym=='z') {
					quit();
					break;
				}

				if (b == (KBD_CTRL|KBD_ALT) && 
						ev.key.keysym.sym>='1' && ev.key.keysym.sym<='6') {
					set_param(PARAM_HOTSWAP_GFX_MODE, ev.key.keysym.sym - '1');
					break;
				}


				event->event_code = EVENT_KEYDOWN;
				event->kbd.keycode = ev.key.keysym.sym;
				event->kbd.ascii = mapKey(ev.key.keysym.sym, ev.key.keysym.mod);
				return true;
			}

		case SDL_MOUSEMOTION:
			event->event_code = EVENT_MOUSEMOVE;
			event->mouse.x = ev.motion.x;
			event->mouse.y = ev.motion.y;

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
			event->mouse.x = ev.button.x;
			event->mouse.y = ev.button.y;
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
	
void OSystem_SDL::set_sound_proc(void *param, SoundProc *proc) {
	_sound_proc = proc;	
	_sound_param = param;
}


/* retrieve the 320x200 bitmap currently being displayed */
void OSystem_SDL::get_320x200_image(byte *buf) {
	/* make sure the mouse is gone */
	undraw_mouse();
	
	if (SDL_LockSurface(sdl_screen) == -1)
		error("SDL_LockSurface failed: %s.\n", SDL_GetError());

	byte *src;
	int x,y;

	switch(_internal_scaling) {
	case 1:
		memcpy(buf, sdl_screen->pixels, 320*200);
		break;

	case 2:
		src = (byte*)sdl_screen->pixels;
		for(y=0; y!=200; y++) {
			for(x=0; x!=320; x++)
				buf[x] = src[x*2];

			buf += 320;
			src += 320 * 2 * 2;
		}
		break;

	case 3:
		src = (byte*)sdl_screen->pixels;
		for(y=0; y!=200; y++) {
			for(x=0; x!=320; x++)
				buf[x] = src[x*3];

			buf += 320;
			src += 320 * 3 * 3;
		}
		break;
	}	

	SDL_UnlockSurface(sdl_screen);
}

void OSystem_SDL::hotswap_gfx_mode() {
	/* hmm, need to allocate a 320x200 bitmap
	 * which will contain the "backup" of the screen during the change.
	 * then draw that to the new screen right after it's setup.
	 */
	
	byte *bak_mem = (byte*)malloc(320*200);

	get_320x200_image(bak_mem);

	unload_gfx_mode();
	load_gfx_mode();

	/* blit image */
	OSystem_SDL::copy_rect(bak_mem, 320, 0, 0, 320, 200);
	free(bak_mem);

	/* reset palette */
	SDL_SetColors(sdl_screen, _cur_pal, 0, 256);

	force_full = true;
	OSystem_SDL::update_screen();
}

uint32 OSystem_SDL::set_param(int param, uint32 value) {
	switch(param) {

	case PARAM_TOGGLE_FULLSCREEN:
		_full_screen ^= true;
		
		if (!SDL_WM_ToggleFullScreen(sdl_hwscreen)) {
			/* if ToggleFullScreen fails, achieve the same effect with hotswap gfx mode */
			hotswap_gfx_mode();
		}
		return 1;

	case PARAM_WINDOW_CAPTION:
		SDL_WM_SetCaption((char*)value, (char*)value);
		return 1;

	case PARAM_OPEN_CD:
		if (SDL_InitSubSystem(SDL_INIT_CDROM) == -1)
			cdrom = NULL;
		else {
			cdrom = SDL_CDOpen(value);
			/* Did if open? Check if cdrom is NULL */
			if (!cdrom) {
				warning("Couldn't open drive: %s\n", SDL_GetError());
			}
		}
		break;

	case PARAM_HOTSWAP_GFX_MODE:
		if (value >= 6)
			return 0;

		_mode = value;
		hotswap_gfx_mode();

		return 1;

	case PARAM_SHOW_DEFAULT_CURSOR:
		SDL_ShowCursor(value ? SDL_ENABLE : SDL_DISABLE);		
		break;
	}

	return 0;
}
		
void OSystem_SDL::quit() {
	unload_gfx_mode();		
	exit(1);
}

void OSystem_SDL::draw_mouse() {
	if (_mouse_drawn || !_mouse_visible)
		return;
	_mouse_drawn = true;

	if (SDL_LockSurface(sdl_screen) == -1)
		error("SDL_LockSurface failed: %s.\n", SDL_GetError());

	const int ydraw = _ms_cur.y + _current_shake_pos - _ms_hotspot_x;
	const int xdraw = _ms_cur.x - _ms_hotspot_y;
	const int w = _ms_cur.w;
	const int h = _ms_cur.h;
	int x,y;
	byte color;
	byte *dst, *bak = _ms_backup;
	byte *buf = _ms_buf;

	_ms_old.w = w;
	_ms_old.h = h;
	_ms_old.x = xdraw;
	_ms_old.y = ydraw;

	switch(_internal_scaling) {
	case 1:
		dst = (byte *)sdl_screen->pixels + ydraw * 320 + xdraw;

		for (y = 0; y < h; y++, dst += 320, bak += MAX_MOUSE_W, buf += w) {
			if ((uint) (ydraw + y) < 200) {
				for (x = 0; x < w; x++) {
					if ((uint) (xdraw + x) < 320) {
						bak[x] = dst[x];
						if ((color = buf[x]) != 0xFF) {
							dst[x] = color;
						}
					}
				}
			}
		}
		break;

	case 2:
		dst = (byte *)sdl_screen->pixels + ydraw * 640 * 2 + xdraw * 2;

		for (y = 0; y < h; y++, dst += 640 * 2, bak += MAX_MOUSE_W * 2, buf += w) {
			if ((uint) (ydraw + y) < 200) {
				for (x = 0; x < w; x++) {
					if ((uint) (xdraw + x) < 320) {
						bak[x * 2] = dst[x * 2];
						bak[x * 2 + 1] = dst[x * 2 + 1];
						if ((color = buf[x]) != 0xFF) {
							dst[x * 2] = color;
							dst[x * 2 + 1] = color;
							dst[x * 2 + 640] = color;
							dst[x * 2 + 1 + 640] = color;
						}
					}
				}
			}
		}
		break;

	case 3:
		dst = (byte *)sdl_screen->pixels + ydraw * 960 * 3 + xdraw * 3;

		for (y = 0; y < h; y++, dst += 960 * 3, bak += MAX_MOUSE_W * 3, buf += w) {
			if ((uint) (ydraw + y) < 200) {
				for (x = 0; x < w; x++) {
					if ((uint) (xdraw + x) < 320) {
						bak[x * 3] = dst[x * 3];
						bak[x * 3 + 1] = dst[x * 3 + 1];
						bak[x * 3 + 2] = dst[x * 3 + 2];
						if ((color = buf[x]) != 0xFF) {
							dst[x * 3] = color;
							dst[x * 3 + 1] = color;
							dst[x * 3 + 2] = color;
							dst[x * 3 + 960] = color;
							dst[x * 3 + 1 + 960] = color;
							dst[x * 3 + 2 + 960] = color;
							dst[x * 3 + 960 + 960] = color;
							dst[x * 3 + 1 + 960 + 960] = color;
							dst[x * 3 + 2 + 960 + 960] = color;
						}
					}
				}
			}
		}
		break;
	}

	add_dirty_rect(xdraw,ydraw,w,h);

	SDL_UnlockSurface(sdl_screen);
}

void OSystem_SDL::undraw_mouse() {
	if (!_mouse_drawn)
		return;
	_mouse_drawn = false;

	if (SDL_LockSurface(sdl_screen) == -1)
		error("SDL_LockSurface failed: %s.\n", SDL_GetError());

	byte *dst, *bak = _ms_backup;
	const int old_mouse_x = _ms_old.x;
	const int old_mouse_y = _ms_old.y;
	const int old_mouse_w = _ms_old.w;
	const int old_mouse_h = _ms_old.h;
	int x,y;

	switch(_internal_scaling) {
	case 1:
		dst = (byte *)sdl_screen->pixels + old_mouse_y * 320 + old_mouse_x;

		for (y = 0; y < old_mouse_h; y++, bak += MAX_MOUSE_W, dst += 320) {
			if ((uint) (old_mouse_y + y) < 200) {
				for (x = 0; x < old_mouse_w; x++) {
					if ((uint) (old_mouse_x + x) < 320) {
						dst[x] = bak[x];
					}
				}
			}
		}
		break;

	case 2:
		dst = (byte *)sdl_screen->pixels + old_mouse_y * 640 * 2 + old_mouse_x * 2;

		for (y = 0; y < old_mouse_h; y++, bak += MAX_MOUSE_W * 2, dst += 640 * 2) {
			if ((uint) (old_mouse_y + y) < 200) {
				for (x = 0; x < old_mouse_w; x++) {
					if ((uint) (old_mouse_x + x) < 320) {
						dst[x * 2 + 640] = dst[x * 2] = bak[x * 2];
						dst[x * 2 + 640 + 1] = dst[x * 2 + 1] = bak[x * 2 + 1];
					}
				}
			}
		}
		break;

	case 3:
		dst = (byte *)sdl_screen->pixels + old_mouse_y * 960 * 3 + old_mouse_x * 3;

		for (y = 0; y < old_mouse_h; y++, bak += MAX_MOUSE_W * 3, dst += 960 * 3) {
			if ((uint) (old_mouse_y + y) < 200) {
				for (x = 0; x < old_mouse_w; x++) {
					if ((uint) (old_mouse_x + x) < 320) {
						dst[x * 3 + 960] = dst[x * 3 + 960 + 960] = dst[x * 3] =
							bak[x * 3];
						dst[x * 3 + 960 + 1] = dst[x * 3 + 960 + 960 + 1] =
							dst[x * 3 + 1] = bak[x * 3 + 1];
						dst[x * 3 + 960 + 2] = dst[x * 3 + 960 + 960 + 2] =
							dst[x * 3 + 2] = bak[x * 3 + 2];
					}
				}
			}
		}
		break;
	}

	add_dirty_rect(old_mouse_x, old_mouse_y, old_mouse_w, old_mouse_h);

	SDL_UnlockSurface(sdl_screen);
}

void cd_stop() {
}

void cd_play(Scumm *s, int track, int num_loops, int start_frame, int end_track) {
}

int cd_is_running() {
	return 0;
}

void cd_music_loop() {
}

