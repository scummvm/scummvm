/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
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

#define NEED_SDL_HEADERS

#include "stdafx.h"
#include "scumm.h"
#include "gui.h"
#include "sound.h"
#include "SDL_thread.h"

static unsigned int scale;

Scumm scumm;
ScummDebugger debugger;
Gui gui;

SoundEngine sound;
SOUND_DRIVER_TYPE snd_driv;

static SDL_Surface *screen;
static SDL_CD      *cdrom;

/* For 2xSAI */
static SDL_Surface *sdl_hwscreen;
static SDL_Surface *sdl_tmpscreen;
int Init_2xSaI (uint32 BitFormat);	
void _2xSaI (uint8 *srcPtr, uint32 srcPitch, uint8 *deltaPtr, uint8 *dstPtr, uint32 dstPitch, int width, int height);
void Super2xSaI (uint8 *srcPtr, uint32 srcPitch, uint8 *deltaPtr, uint8 *dstPtr, uint32 dstPitch, int width, int height);
void SuperEagle(uint8 *srcPtr, uint32 srcPitch, uint8 *deltaPtr, uint8 *dstPtr, uint32 dstPitch, int width, int height);


static int current_shake_pos;

void resetCursor(void) {
	SDL_ShowCursor(SDL_ENABLE);
}

void updateScreen(Scumm *s);

void updatePalette(Scumm *s) {
	SDL_Color colors[256];
	int first = s->_palDirtyMin;
	int num = s->_palDirtyMax - first + 1;
	int i;
	byte *data = s->_currentPalette;
	
	data += first*3;
	for (i=0; i<num; i++,data+=3) {
		colors[i].r = data[0];
		colors[i].g = data[1];
		colors[i].b = data[2];
		colors[i].unused = 0;
	}
	
	SDL_SetColors(screen, colors, first, num);
	
	s->_palDirtyMax = -1;
	s->_palDirtyMin = 0x3E8;
}

int mapKey(int key, byte mod) {
	if (key>=SDLK_F1 && key<=SDLK_F9) {
		return key - SDLK_F1 + 315;
	} else if (key>='a' && key<='z' && mod&KMOD_SHIFT) {
		key&=~0x20;
	} else if (key>=SDLK_NUMLOCK && key<=SDLK_EURO)
		return 0;
	return key;
}

void waitForTimer(Scumm *s, int msec_delay) {
	SDL_Event event;
	uint32 start_time;

	if (s->_fastMode&2)
		msec_delay = 0;
	else if (s->_fastMode&1)
		msec_delay = 10;

	start_time = SDL_GetTicks();

	do {
		while (SDL_PollEvent(&event)) {
			switch(event.type) {
			case SDL_KEYDOWN:
				s->_keyPressed = mapKey(event.key.keysym.sym, event.key.keysym.mod);
				if (event.key.keysym.sym >= '0' && event.key.keysym.sym<='9') {
					s->_saveLoadSlot = event.key.keysym.sym - '0';
					if (event.key.keysym.mod&KMOD_SHIFT) {
						sprintf(s->_saveLoadName, "Quicksave %d", s->_saveLoadSlot);
						s->_saveLoadFlag = 1;
					} else if (event.key.keysym.mod&KMOD_CTRL)
						s->_saveLoadFlag = 2;
					s->_saveLoadCompatible = false;
				} else if (event.key.keysym.sym=='z' && event.key.keysym.mod&KMOD_CTRL) {
					exit(1);
				} else if (event.key.keysym.sym=='f' && event.key.keysym.mod&KMOD_CTRL) {
					s->_fastMode ^= 1;
				} else if (event.key.keysym.sym=='g' && event.key.keysym.mod&KMOD_CTRL) {
					s->_fastMode ^= 2;
				} else if (event.key.keysym.sym=='d' && event.key.keysym.mod&KMOD_CTRL) {
					debugger.attach(s);
				} else if (event.key.keysym.sym=='s' && event.key.keysym.mod&KMOD_CTRL) {
					s->resourceStats();
				} else if (event.key.keysym.sym==SDLK_RETURN && event.key.keysym.mod&KMOD_ALT) {
					if(!SDL_WM_ToggleFullScreen(screen))
						warning("Full screen failed");
				}

	#if defined(__APPLE__) || defined(MACOS)
				if (event.key.keysym.sym=='q' && event.key.keysym.mod&KMOD_LMETA) {
					exit(1);
				} 
	#endif
				break;
			case SDL_MOUSEMOTION: {
				int newx,newy;
	if (scale == 3)
	{
				newx = event.motion.x/3;
				newy = event.motion.y/3;
	} else
	if (scale == 2)
	{
				newx = event.motion.x>>1;
				newy = event.motion.y>>1;
	} else
	{
				newx = event.motion.x;
				newy = event.motion.y;
	}

				if (newx != s->mouse.x || newy != s->mouse.y) {
					s->mouse.x = newx;
					s->mouse.y = newy;
					s->drawMouse();
					updateScreen(s);
				}
				break;
				}
			case SDL_MOUSEBUTTONDOWN:
				if (event.button.button==SDL_BUTTON_LEFT)
					s->_leftBtnPressed |= msClicked|msDown;
				else if (event.button.button==SDL_BUTTON_RIGHT)
					s->_rightBtnPressed |= msClicked|msDown;
				break;
			case SDL_MOUSEBUTTONUP:
				if (event.button.button==SDL_BUTTON_LEFT)
					s->_leftBtnPressed &= ~msDown;
				else if (event.button.button==SDL_BUTTON_RIGHT)
					s->_rightBtnPressed &= ~msDown;
				break;

			case SDL_QUIT:
				exit(1);
				break;
			}
		}

		if (SDL_GetTicks() >= start_time + msec_delay)
			break;
		SDL_Delay(10);
	} while (1);
}

#define MAX_DIRTY_RECTS 40
SDL_Rect dirtyRects[MAX_DIRTY_RECTS];
int numDirtyRects;
bool fullRedraw;

int old_mouse_x, old_mouse_y;
int old_mouse_h, old_mouse_w;
bool has_mouse,hide_mouse;

#define BAK_WIDTH 40
#define BAK_HEIGHT 40
byte old_backup[BAK_WIDTH*BAK_HEIGHT*2];


void addDirtyRect(int x, int y, int w, int h) {
	SDL_Rect *r;
	if (numDirtyRects==MAX_DIRTY_RECTS)
		fullRedraw = true;
	else if (!fullRedraw) {
		r = &dirtyRects[numDirtyRects++];
	if (scale == 3)
	{
		r->x = x*3;
		r->y = y*3;
		r->w = w*3;
		r->h = h*3;
	} else
	if (scale == 2)
	{
		r->x = x*2;
		r->y = y*2;
		r->w = w*2;
		r->h = h*2;
	} else
	{
		r->x = x;
		r->y = y;
		r->w = w;
		r->h = h;
	}
	}
}

void addDirtyRectClipped(int x, int y, int w, int h) {
	if (x<0) { w += x; x=0; }
	if (y<0) { h += y; y=0; }
	if (w >= 320-x) w = 320-x;
	if (h >= 200-y) h = 200-y;
	if (w>0 && h>0)
		addDirtyRect(x,y,w,h);
}

#define MAX(a,b) (((a)<(b)) ? (b) : (a))
#define MIN(a,b) (((a)>(b)) ? (b) : (a))

void setShakePos(Scumm *s, int shake_pos) {
	int old_shake_pos = current_shake_pos;
	int dirty_height, dirty_blackheight;
	int dirty_top, dirty_blacktop;

	if (shake_pos != old_shake_pos) {
		current_shake_pos = shake_pos;
		fullRedraw = true;
		
		/* Old shake pos was current_shake_pos, new is shake_pos.
		 * Move the screen up or down to account for the change.
		 */
	if (scale == 3)
	{
		SDL_Rect dstr = {0,shake_pos*3,960,600};
		SDL_Rect srcr = {0,old_shake_pos*3,960,600};
		SDL_BlitSurface(screen, &srcr, screen, &dstr);
	} else
	if (scale == 2)
	{
		SDL_Rect dstr = {0,shake_pos*2,640,400};
		SDL_Rect srcr = {0,old_shake_pos*2,640,400};
		SDL_BlitSurface(screen, &srcr, screen, &dstr);
	} else
	{
		SDL_Rect dstr = {0,shake_pos,320,200};
		SDL_Rect srcr = {0,old_shake_pos,320,200};
		SDL_BlitSurface(screen, &srcr, screen, &dstr);
	}
		
		/* Also adjust the mouse pointer backup Y coordinate.
		 * There is a minor mouse glitch when the mouse is moved
		 * at the blackness of the shake area, but it's hardly noticable */
		old_mouse_y += shake_pos - old_shake_pos;

		/* Refresh either the upper part of the screen,
		 * or the lower part */
		if (shake_pos > old_shake_pos) {
			dirty_height = MIN(shake_pos, 0) - MIN(old_shake_pos,0);
			dirty_top = -MIN(shake_pos,0);
			dirty_blackheight = MAX(shake_pos,0) - MAX(old_shake_pos,0);
			dirty_blacktop = MAX(old_shake_pos,0);
		} else {
			dirty_height = MAX(old_shake_pos,0) - MAX(shake_pos, 0);
			dirty_top = 200 - MAX(old_shake_pos,0);
			dirty_blackheight = MIN(old_shake_pos,0) - MIN(shake_pos,0);
			dirty_blacktop = 200 + MIN(shake_pos,0);
		}

		/* Fill the dirty area with blackness or the scumm image */
		{
		if (scale == 3)
		{
			SDL_Rect blackrect = {0, dirty_blacktop*3, 960, dirty_blackheight*3};
			SDL_FillRect(screen, &blackrect, 0);
		} else
		if (scale == 2)
		{
			SDL_Rect blackrect = {0, dirty_blacktop*2, 640, dirty_blackheight*2};
			SDL_FillRect(screen, &blackrect, 0);
		} else
		{
			SDL_Rect blackrect = {0, dirty_blacktop, 320, dirty_blackheight};
			SDL_FillRect(screen, &blackrect, 0);
		}

			s->redrawLines(dirty_top, dirty_top + dirty_height);
		}
	}
}

/* Copy part of bitmap */
void blitToScreen(Scumm *s, byte *src,int x, int y, int w, int h) {
	byte *dst;
	int i;

	hide_mouse = true;
	if (has_mouse) {
		s->drawMouse();
	}

	/* Account for the shaking and do Y clipping */
	y += current_shake_pos;
	if (y < 0 ) {	h += y; src -= y*320; y = 0; }
	if (h > 200 - y) { h = 200 - y; }
	if (h<=0)	return;

	if (SDL_LockSurface(screen)==-1)
		error("SDL_LockSurface failed: %s.\n", SDL_GetError());

	if (scale == 3)
	{
	dst = (byte*)screen->pixels + y*960*3 + x*3;
	addDirtyRect(x,y,w,h);
#ifdef DEBUG_CODE
	byte black = GetAsyncKeyState(VK_SHIFT)<0 ? 0 : 0xFF;
	do {
		i=0;
		do {
			dst[i*3] = dst[i*3+1] = dst[i*3+2] = src[i] & black;
		} while (++i!=w);
		memcpy(dst+960, dst, w*3);
		memcpy(dst+960+960, dst, w*3);
		dst += 960*3;
		src += 320;
	} while (--h);
#else
	do {
		i=0;
		do {
			dst[i*3] = dst[i*3+1] = dst[i*3+2] = src[i];
		} while (++i!=w);
		memcpy(dst+960, dst, w*3);
		memcpy(dst+960+960, dst, w*3);
		dst += 960*3;
		src += 320;
	} while (--h);
#endif
	} else
	if (scale == 2)
	{
	dst = (byte*)screen->pixels + y*640*2 + x*2;
	addDirtyRect(x,y,w,h);		
#ifdef DEBUG_CODE
	byte black = GetAsyncKeyState(VK_SHIFT)<0 ? 0 : 0xFF;
	do {
		i=0;
		do {
			dst[i*2] = dst[i*2+1] = src[i] & black;
		} while (++i!=w);
		memcpy(dst+640, dst, w*2);
		dst += 640*2;
		src += 320;
	} while (--h);
#else
	do {
		i=0;
		do {
			dst[i*2] = dst[i*2+1] = src[i];
		} while (++i!=w);
		memcpy(dst+640, dst, w*2);
		dst += 640*2;
		src += 320;
	} while (--h);
#endif
	} else
	{
	dst = (byte*)screen->pixels + y*320 + x;
	addDirtyRect(x,y,w,h);
	do {
		memcpy(dst, src, w);
		dst += 320;
		src += 320;
	} while (--h);
	}

	SDL_UnlockSurface(screen);
}

void Draw2xSaI(SDL_Rect *r, int vidmode) {
	if (SDL_BlitSurface(screen, r, sdl_tmpscreen, r) != 0)
		error("SDL_BlitSurface failed");

	SDL_LockSurface(sdl_tmpscreen);
	SDL_LockSurface(sdl_hwscreen);

	switch(vidmode) {
		case VIDEO_2XSAI:
				_2xSaI((byte*)sdl_tmpscreen->pixels + r->x*2 + r->y*640, 640, NULL, 
						(byte*)sdl_hwscreen->pixels + r->x*4 + r->y*640*4, 640*2, r->w, r->h);
				break;
		case VIDEO_SUPERSAI:
				Super2xSaI((byte*)sdl_tmpscreen->pixels + r->x*2 + r->y*640, 640, NULL, 
						(byte*)sdl_hwscreen->pixels + r->x*4 + r->y*640*4, 640*2, r->w, r->h);
				break;
		case VIDEO_SUPEREAGLE:
				SuperEagle((byte*)sdl_tmpscreen->pixels + r->x*2 + r->y*640, 640, NULL, 
						(byte*)sdl_hwscreen->pixels + r->x*4 + r->y*640*4, 640*2, r->w, r->h);
				break;
		default:
				error("Unknown graphics mode %d", vidmode);
				break;
	}
	
	/* scale the rect to fit in SDL_UpdateRects */
	r->x <<= 1;
	r->y <<= 1;
	r->w <<= 1;
	r->h <<= 1;
	
	SDL_UnlockSurface(sdl_tmpscreen);
	SDL_UnlockSurface(sdl_hwscreen);
	SDL_UpdateRect(sdl_hwscreen, r->x, r->y, r->w, r->h);
}

void updateScreen2xSaI(Scumm *s) {
	SDL_Rect r;

	if (s->_fastMode&2)
		return;

	if (hide_mouse) {
		hide_mouse = false;
		s->drawMouse();
	}

	if(s->_palDirtyMax != -1)
		updatePalette(s);
	
	if (fullRedraw) {
		r.x = 0;
		r.y = 0;
		r.w = 320;
		r.h = 200;
		Draw2xSaI(&r, s->_videoMode);
		fullRedraw = false;

		return;
	} else if (numDirtyRects) {
		SDL_Rect *dr;
		int i;
		
		for (i = 0; i <= numDirtyRects; i++) {
			dr = &dirtyRects[i];
			Draw2xSaI(dr, s->_videoMode);
		}
	}

	numDirtyRects = 0;
}


void updateScreenScale(Scumm *s) {
	if (fullRedraw) {
		SDL_UpdateRect(screen, 0,0,0,0);
		fullRedraw = false;
	} else if (numDirtyRects) {
		SDL_UpdateRects(screen, numDirtyRects, dirtyRects);	
	}

	numDirtyRects = 0;
}

void updateScreen(Scumm *s) {
	if (s->_fastMode&2)
		return;

	if (hide_mouse) {
		hide_mouse = false;
		s->drawMouse();
	}

	if(s->_palDirtyMax != -1) {
		updatePalette(s);
	}

	if (s->_videoMode == VIDEO_SCALE)
		updateScreenScale(s);
	else
		updateScreen2xSaI(s);	
}

void drawMouse(Scumm *s, int xdraw, int ydraw, int w, int h, byte *buf, bool visible) {
	int x,y;
	byte *dst,*bak;
	byte color;

	if (hide_mouse)
		visible = false;

	assert(w<=BAK_WIDTH && h<=BAK_HEIGHT);

	if (SDL_LockSurface(screen)==-1)
		error("SDL_LockSurface failed: %s.\n", SDL_GetError());

	if (scale == 3)
	{

	if (has_mouse) {
		dst = (byte*)screen->pixels + old_mouse_y*960*3 + old_mouse_x*3;
		bak = old_backup;

		for (y=0; y<old_mouse_h; y++,bak+=BAK_WIDTH*3,dst+=960*3) {
			if ( (uint)(old_mouse_y + y) < 200) {
				for (x=0; x<old_mouse_w; x++) {
					if ((uint)(old_mouse_x + x) < 320) {
						dst[x*3+960] = dst[x*3+960+960] = dst[x*3] = bak[x*3];
						dst[x*3+960+1] = dst[x*3+960+960+1] = dst[x*3+1] = bak[x*3+1];
						dst[x*3+960+2] = dst[x*3+960+960+2] = dst[x*3+2] = bak[x*3+2];
					}
				}
			}
		}
	}

	if (visible) {
		ydraw += current_shake_pos;

		dst = (byte*)screen->pixels + ydraw*960*3 + xdraw*3;
		bak = old_backup;

		for (y=0; y<h; y++,dst+=960*3,bak+=BAK_WIDTH*3,buf+=w) {
			if ((uint)(ydraw+y)<200) {
				for (x=0; x<w; x++) {
					if ((uint)(xdraw+x)<320) {
						bak[x*3] = dst[x*3];
						bak[x*3+1] = dst[x*3+1];
						bak[x*3+2] = dst[x*3+2];
						if ((color=buf[x])!=0xFF) {
							dst[x*3] = color;
							dst[x*3+1] = color;
							dst[x*3+2] = color;
							dst[x*3+960] = color;
							dst[x*3+1+960] = color;
							dst[x*3+2+960] = color;
							dst[x*3+960+960] = color;
							dst[x*3+1+960+960] = color;
							dst[x*3+2+960+960] = color;
						}
					}
				}
			}
		}
	}
	} else
	if (scale == 2)
	{

	if (has_mouse) {
		dst = (byte*)screen->pixels + old_mouse_y*640*2 + old_mouse_x*2;
		bak = old_backup;

		for (y=0; y<old_mouse_h; y++,bak+=BAK_WIDTH*2,dst+=640*2) {
			if ( (uint)(old_mouse_y + y) < 200) {
				for (x=0; x<old_mouse_w; x++) {
					if ((uint)(old_mouse_x + x) < 320) {
						dst[x*2+640] = dst[x*2] = bak[x*2];
						dst[x*2+640+1] = dst[x*2+1] = bak[x*2+1];
					}
				}
			}
		}
	}

	if (visible) {
		ydraw += current_shake_pos;
		
		dst = (byte*)screen->pixels + ydraw*640*2 + xdraw*2;
		bak = old_backup;

		for (y=0; y<h; y++,dst+=640*2,bak+=BAK_WIDTH*2,buf+=w) {
			if ((uint)(ydraw+y)<200) {
				for (x=0; x<w; x++) {
					if ((uint)(xdraw+x)<320) {
						bak[x*2] = dst[x*2];
						bak[x*2+1] = dst[x*2+1];
						if ((color=buf[x])!=0xFF) {
							dst[x*2] = color;
							dst[x*2+1] = color;
							dst[x*2+640] = color;
							dst[x*2+1+640] = color;
						}
					}
				}
			}
		}
	}
	} else
	{
	if (has_mouse) {
		dst = (byte*)screen->pixels + old_mouse_y*320 + old_mouse_x;
		bak = old_backup;

		for (y=0; y<old_mouse_h; y++,bak+=BAK_WIDTH,dst+=320) {
			if ( (uint)(old_mouse_y + y) < 200) {
				for (x=0; x<old_mouse_w; x++) {
					if ((uint)(old_mouse_x + x) < 320) {
						dst[x] = bak[x];
					}
				}
			}
		}
	}
	if (visible) {
		ydraw += current_shake_pos;
		
		dst = (byte*)screen->pixels + ydraw*320 + xdraw;
		bak = old_backup;

		for (y=0; y<h; y++,dst+=320,bak+=BAK_WIDTH,buf+=w) {
			if ((uint)(ydraw+y)<200) {
				for (x=0; x<w; x++) {
					if ((uint)(xdraw+x)<320) {
						bak[x] = dst[x];
						if ((color=buf[x])!=0xFF) {
							dst[x] = color;
						}
					}
				}
			}
		}
	}
	}

	SDL_UnlockSurface(screen);

	if (has_mouse) {
		has_mouse = false;
		addDirtyRectClipped(old_mouse_x, old_mouse_y, old_mouse_w, old_mouse_h);
	}

	if (visible) {
		has_mouse = true;
		addDirtyRectClipped(xdraw, ydraw, w, h);
		old_mouse_x = xdraw;
		old_mouse_y = ydraw;
		old_mouse_w = w;
		old_mouse_h = h;
	}
}

void fill_sound(void *userdata, Uint8 *stream, int len) {
	scumm.mixWaves((int16*)stream, len>>1);
}

void cd_playtrack(int track, int offset, int delay) {
        SDL_CDStatus(cdrom);
        SDL_CDPlayTracks(cdrom, track, (int)((offset * 7.5) - 22650), 0, (int)(delay * 7.5));
}

int music_thread(Scumm *s) {
	int old_time, cur_time;

	old_time = SDL_GetTicks();

	do {
		SDL_Delay(10);
		
		cur_time = SDL_GetTicks();
		while (old_time < cur_time) {
			old_time += 10;
			sound.on_timer();	
		}
	} while (1);
	
	return 0;
}


void initGraphics(Scumm *s, bool fullScreen, unsigned int scaleFactor) {
	SDL_AudioSpec desired;

	scale = scaleFactor;

	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO)==-1) {
		error("Could not initialize SDL: %s.\n", SDL_GetError());
	    exit(1);
	}

    if (SDL_InitSubSystem(SDL_INIT_CDROM) == -1)
        cdrom = NULL;
    else
        cdrom = SDL_CDOpen(0);

	/* Clean up on exit */
 	atexit(SDL_Quit);
	atexit(resetCursor);

	char buf[512], *gameName;
	
	sprintf(buf, "ScummVM - %s", gameName = s->getGameName());
	free(gameName);

	desired.freq = SAMPLES_PER_SEC;
	desired.format = AUDIO_S16SYS;
	desired.channels = 1;
	desired.samples = 2048;
	desired.callback = fill_sound;
	SDL_OpenAudio(&desired, NULL);
	SDL_PauseAudio(0);

	SDL_WM_SetCaption(buf,buf);
	SDL_ShowCursor(SDL_DISABLE);

	if (!snd_driv.wave_based()) {
		/* Create Music Thread */
		SDL_CreateThread((int (*)(void *))&music_thread, &scumm);
	}

	if (s->_videoMode == VIDEO_SCALE) {
				screen = SDL_SetVideoMode(320 * scale, 200 * scale, 8, fullScreen ? (SDL_SWSURFACE | SDL_FULLSCREEN) : (SDL_SWSURFACE | SDL_DOUBLEBUF));
	} else {
		uint16 *tmp_screen = (uint16*)calloc(320*202 + 8,sizeof(uint16));
		Init_2xSaI(565);

		screen = SDL_CreateRGBSurface(SDL_SWSURFACE, 320, 200, 8, 0, 0, 0, 0);
		sdl_hwscreen = SDL_SetVideoMode(640, 400, 16, fullScreen ? (SDL_SWSURFACE | SDL_FULLSCREEN) : (SDL_SWSURFACE | SDL_DOUBLEBUF));
		sdl_tmpscreen = SDL_CreateRGBSurfaceFrom(tmp_screen + 320 + 4, 320, 200, 16, 320*2, 0,0,0,0);
		if (sdl_tmpscreen == NULL)
			error("sdl_tmpscreen failed");

		scale = 1;
	}

// SDL_SWSURFACE 	0x00000000	/* Surface is in system memory */
// SDL_HWSURFACE 	0x00000001	/* Surface is in video memory */
// SDL_ASYNCBLIT 	0x00000004	/* Use asynchronous blits if possible */
// SDL_ANYFORMAT 	0x10000000	/* Allow any video depth/pixel-format */
// SDL_HWPALETTE 	0x20000000	/* Surface has exclusive palette */
// SDL_DOUBLEBUF 	0x40000000	/* Set up double-buffered video mode */
// SDL_FULLSCREEN	0x80000000	/* Surface is a full screen display */
// SDL_OPENGL    	0x00000002	/* Create an OpenGL rendering context */
// SDL_OPENGLBLIT	0x0000000A	/* Create an OpenGL rendering context and use it for blitting */
// SDL_RESIZABLE 	0x00000010	/* This video mode may be resized */
// SDL_NOFRAME   	0x00000020	/* No window caption or edge frame */



	printf("%d %d, %d %d, %d %d %d, %d %d %d %d %d\n", 
		sizeof(int8), sizeof(uint8),
		sizeof(int16), sizeof(uint16),
		sizeof(int32), sizeof(uint32),
		sizeof(void*),
		sizeof(Box), sizeof(MouseCursor),sizeof(CodeHeader),
		sizeof(ImageHeader),
		sizeof(Scumm)
	);
}


#if !defined(__APPLE__)
#undef main
#endif


void launcherLoop() {
	int last_time, new_time;
	int delta = 0;
	last_time = SDL_GetTicks();

	gui.saveLoadDialog();
	do {
		updateScreen(&scumm);

		new_time = SDL_GetTicks();
		waitForTimer(&scumm, delta * 15 + last_time - new_time);
		last_time = SDL_GetTicks();

		if (gui._active) {
			gui.loop();
			delta = 5;
		} else
			error("gui closed!");
	} while(1);

};

int main(int argc, char* argv[]) {
	int delta;
	int last_time, new_time;
	
#if defined(MACOS) 
	/* support for config file on macos */
	
	char *argitem;
	char *argstr;
	FILE *argf;
	
	if (( argf = fopen("configuration.macos", "r")) == NULL) {
		error("Can't open configuration file.\n");
		exit(1);
	}
	
	argc=0;
	argstr = (char *) malloc(64);
	argstr = fgets(argstr, 64, argf);
	if ((argitem = strchr(argstr, '\n'))!=NULL)
		*argitem = '\0';
	
	argitem = strtok(argstr, " ");
	
	while (argitem!=NULL) {
		argv = (char**) realloc(argv, (argc+1)*8);
		argv[argc] = (char *) malloc(64);
		strcpy(argv[argc], argitem);
		argc++;	
		
		argitem = strtok(NULL, " ");
	}
	
	free(argstr);
	fclose(argf);
	
#endif

	scumm._gui = &gui;
	gui.init(&scumm);
	sound.initialize(&scumm, &snd_driv);
    scumm.scummMain(argc, argv);
	gui.init(&scumm);	/* Reinit GUI after loading a game */

	last_time = SDL_GetTicks();
	delta = 0;
	do {
		updateScreen(&scumm);

		new_time = SDL_GetTicks();
		waitForTimer(&scumm, delta * 15 + last_time - new_time);
		last_time = SDL_GetTicks();

		if (gui._active) {
			gui.loop();
			delta = 5;
		} else {
			delta = scumm.scummLoop(delta);
		}
	} while(1);

	return 0;
}

/************ ENDER: Temporary debug code for boxen **************/
int hlineColor (SDL_Surface *dst, Sint16 x1, Sint16 x2, Sint16 y, Uint32 color) {
 Sint16 left,right,top,bottom;
 Uint8 *pixel,*pixellast;
 int dx;
 int pixx, pixy;
 Sint16 w;
 Sint16 xtmp;
 int result=-1;
 Uint8 *colorptr;
 
 /* Get clipping boundary */
 left = dst->clip_rect.x;
 right = dst->clip_rect.x+dst->clip_rect.w-1;
 top = dst->clip_rect.y;
 bottom = dst->clip_rect.y+dst->clip_rect.h-1;

 /* Swap x1, x2 if required */
 if (x1>x2) {
  xtmp=x1; x1=x2; x2=xtmp;
 }

 /* Visible */
 if ((x1>right) || (x2<left) || (y<top) || (y>bottom)) {
  return(0);
 }
 
 /* Clip x */
 if (x1<left) { 
  x1=left;
 } 
 if (x2>right) {
  x2=right;
 }
  
 /* Calculate width */
 w=x2-x1;
 
 /* Sanity check on width */
 if (w<0) {
  return(0);
 }

  /* Setup color */
  colorptr=(Uint8 *)&color;
  if (SDL_BYTEORDER == SDL_BIG_ENDIAN) {
   color=SDL_MapRGBA(dst->format, colorptr[0], colorptr[1], colorptr[2], colorptr[3]);
  } else {
   color=SDL_MapRGBA(dst->format, colorptr[3], colorptr[2], colorptr[1], colorptr[0]);
  }
  
  /* Lock surface */
  SDL_LockSurface(dst);

  /* More variable setup */
  dx=w;
  pixx = dst->format->BytesPerPixel;
  pixy = dst->pitch;
  pixel = ((Uint8*)dst->pixels) + pixx * (int)x1 + pixy * (int)y;
  
  /* Draw */
  switch(dst->format->BytesPerPixel) {
   case 1:
    memset (pixel, color, dx);
    break;
   case 2:
    pixellast = pixel + dx + dx;
    for (; pixel<=pixellast; pixel += pixx) {
     *(Uint16*)pixel = color;
    }
    break;
   case 3:
    pixellast = pixel + dx + dx + dx;
    for (; pixel<=pixellast; pixel += pixx) {
     if (SDL_BYTEORDER == SDL_BIG_ENDIAN) {
      pixel[0] = (color >> 16) & 0xff;
      pixel[1] = (color >> 8) & 0xff;
      pixel[2] = color & 0xff;
     } else {
      pixel[0] = color & 0xff;
      pixel[1] = (color >> 8) & 0xff;
      pixel[2] = (color >> 16) & 0xff;
     }
    }
    break;
   default: /* case 4*/
    dx = dx + dx;
    pixellast = pixel + dx + dx;
    for (; pixel<=pixellast; pixel += pixx) {
     *(Uint32*)pixel = color;
    }
    break;
  }

  /* Unlock surface */
  SDL_UnlockSurface(dst);

  /* Set result code */
  result=0;

 return(result);
}

int gfxPrimitivesCompareInt(const void *a, const void *b);

static int *gfxPrimitivesPolyInts=NULL;
static int gfxPrimitivesPolyAllocated=0;

int filledPolygonColor (SDL_Surface *dst, Sint16 *vx, Sint16 *vy, int n, int color)
{
	int result;
	int i;
	int y;
	int miny, maxy;
	int x1, y1;
	int x2, y2;
	int ind1, ind2;
	int ints;
	
	/* Sanity check */
	if (n<3) {
	 return -1;
	}
	
	/* Allocate temp array, only grow array */
	if (!gfxPrimitivesPolyAllocated) {
	 gfxPrimitivesPolyInts = (int *) malloc(sizeof(int) * n);
	 gfxPrimitivesPolyAllocated = n;
	} else {
	 if (gfxPrimitivesPolyAllocated<n) {
 	  gfxPrimitivesPolyInts = (int *) realloc(gfxPrimitivesPolyInts, sizeof(int) * n);
	  gfxPrimitivesPolyAllocated = n;
	 }
	}		

	/* Determine Y maxima */
	miny = vy[0];
	maxy = vy[0];
	for (i=1; (i < n); i++) {
		if (vy[i] < miny) {
		 miny = vy[i];
		} else if (vy[i] > maxy) {
		 maxy = vy[i];
		}
	}
	
	/* Draw, scanning y */
	result=0;
	for (y=miny; (y <= maxy); y++) {
		ints = 0;
		for (i=0; (i < n); i++) {
			if (!i) {
				ind1 = n-1;
				ind2 = 0;
			} else {
				ind1 = i-1;
				ind2 = i;
			}
			y1 = vy[ind1];
			y2 = vy[ind2];
			if (y1 < y2) {
				x1 = vx[ind1];
				x2 = vx[ind2];
			} else if (y1 > y2) {
				y2 = vy[ind1];
				y1 = vy[ind2];
				x2 = vx[ind1];
				x1 = vx[ind2];
			} else {
				continue;
			}
			if ((y >= y1) && (y < y2)) {
				gfxPrimitivesPolyInts[ints++] = (y-y1) * (x2-x1) / (y2-y1) + x1;
			} else if ((y == maxy) && (y > y1) && (y <= y2)) {
				gfxPrimitivesPolyInts[ints++] = (y-y1) * (x2-x1) / (y2-y1) + x1;
			}
		}
		qsort(gfxPrimitivesPolyInts, ints, sizeof(int), gfxPrimitivesCompareInt);

		for (i=0; (i<ints); i+=2) {
			result |= hlineColor(dst, gfxPrimitivesPolyInts[i], gfxPrimitivesPolyInts[i+1], y, color);
		}
	}
	
 return (result);
}

int gfxPrimitivesCompareInt(const void *a, const void *b)
{
 return (*(const int *)a) - (*(const int *)b);
}

void BoxTest(int num) {
	BoxCoords box;
	Sint16 rx1[4], ry1[4];
	
	scumm.getBoxCoordinates(num,  &box);
	rx1[0] = box.ul.x*2; ry1[0] = box.ul.y*2+32;
	rx1[1] = box.ur.x*2; ry1[1] = box.ur.y*2+32;
	rx1[2] = box.ll.x*2; ry1[2] = box.ll.y*2+32;
	rx1[3] = box.lr.x*2; ry1[3] = box.lr.y*2+32;
	
	filledPolygonColor(screen, &rx1[0], &ry1[0], 4, 255);
	SDL_UpdateRect(screen, 0,0,0,0);
	
}




		/********** 2XSAI Filter *****************/
static uint32 colorMask = 0xF7DEF7DE;
static uint32 lowPixelMask = 0x08210821;
static uint32 qcolorMask = 0xE79CE79C;
static uint32 qlowpixelMask = 0x18631863;
static uint32 redblueMask = 0xF81F;
static uint32 greenMask = 0x7E0;

int Init_2xSaI (uint32 BitFormat)
{
    if (BitFormat == 565)
    {
	colorMask = 0xF7DEF7DE;
	lowPixelMask = 0x08210821;
	qcolorMask = 0xE79CE79C;
	qlowpixelMask = 0x18631863;
	redblueMask = 0xF81F;
	greenMask = 0x7E0;
    }
    else if (BitFormat == 555)
    {
	colorMask = 0x7BDE7BDE;
	lowPixelMask = 0x04210421;
	qcolorMask = 0x739C739C;
	qlowpixelMask = 0x0C630C63;
	redblueMask = 0x7C1F;
	greenMask = 0x3E0;
    }
    else
    {
	return 0;
    }

    return 1;
}

static inline int GetResult1 (uint32 A, uint32 B, uint32 C, uint32 D,
			      uint32 /* E */)
{
    int x = 0;
    int y = 0;
    int r = 0;

    if (A == C)
	x += 1;
    else if (B == C)
	y += 1;
    if (A == D)
	x += 1;
    else if (B == D)
	y += 1;
    if (x <= 1)
	r += 1;
    if (y <= 1)
	r -= 1;
    return r;
}

static inline int GetResult2 (uint32 A, uint32 B, uint32 C, uint32 D,
			      uint32 /* E */)
{
    int x = 0;
    int y = 0;
    int r = 0;

    if (A == C)
	x += 1;
    else if (B == C)
	y += 1;
    if (A == D)
	x += 1;
    else if (B == D)
	y += 1;
    if (x <= 1)
	r -= 1;
    if (y <= 1)
	r += 1;
    return r;
}

static inline int GetResult (uint32 A, uint32 B, uint32 C, uint32 D)
{
    int x = 0;
    int y = 0;
    int r = 0;

    if (A == C)
	x += 1;
    else if (B == C)
	y += 1;
    if (A == D)
	x += 1;
    else if (B == D)
	y += 1;
    if (x <= 1)
	r += 1;
    if (y <= 1)
	r -= 1;
    return r;
}

static inline uint32 INTERPOLATE (uint32 A, uint32 B)
{
    if (A != B)
    {
	return (((A & colorMask) >> 1) + ((B & colorMask) >> 1) +
		(A & B & lowPixelMask));
    }
    else
	return A;
}

static inline uint32 Q_INTERPOLATE (uint32 A, uint32 B, uint32 C, uint32 D)
{
    register uint32 x = ((A & qcolorMask) >> 2) +
	((B & qcolorMask) >> 2) +
	((C & qcolorMask) >> 2) + ((D & qcolorMask) >> 2);
    register uint32 y = (A & qlowpixelMask) +
	(B & qlowpixelMask) + (C & qlowpixelMask) + (D & qlowpixelMask);

    y = (y >> 2) & qlowpixelMask;
    return x + y;
}

#define BLUE_MASK565 0x001F001F
#define RED_MASK565 0xF800F800
#define GREEN_MASK565 0x07E007E0

#define BLUE_MASK555 0x001F001F
#define RED_MASK555 0x7C007C00
#define GREEN_MASK555 0x03E003E0

void Super2xSaI (uint8 *srcPtr, uint32 srcPitch,
		 uint8 *deltaPtr, uint8 *dstPtr, uint32 dstPitch,
		 int width, int height)
{
    uint16 *bP;
    uint8  *dP;
    uint32 inc_bP;

    {
        uint32 Nextline = srcPitch >> 1;
	inc_bP = 1;

	while (height--)
	{
	    bP = (uint16 *) srcPtr;
	    dP = (uint8 *) dstPtr;

	    for (uint32 finish = width; finish; finish -= inc_bP)
	    {
		uint32 color4, color5, color6;
		uint32 color1, color2, color3;
		uint32 colorA0, colorA1, colorA2, colorA3,
		    colorB0, colorB1, colorB2, colorB3, colorS1, colorS2;
		uint32 product1a, product1b, product2a, product2b;

//---------------------------------------    B1 B2
//                                         4  5  6 S2
//                                         1  2  3 S1
//                                           A1 A2

		colorB0 = *(bP - Nextline - 1);
		colorB1 = *(bP - Nextline);
		colorB2 = *(bP - Nextline + 1);
		colorB3 = *(bP - Nextline + 2);

		color4 = *(bP - 1);
		color5 = *(bP);
		color6 = *(bP + 1);
		colorS2 = *(bP + 2);

		color1 = *(bP + Nextline - 1);
		color2 = *(bP + Nextline);
		color3 = *(bP + Nextline + 1);
		colorS1 = *(bP + Nextline + 2);

		colorA0 = *(bP + Nextline + Nextline - 1);
		colorA1 = *(bP + Nextline + Nextline);
		colorA2 = *(bP + Nextline + Nextline + 1);
		colorA3 = *(bP + Nextline + Nextline + 2);

//--------------------------------------
		if (color2 == color6 && color5 != color3)
		{
		    product2b = product1b = color2;
		}
		else if (color5 == color3 && color2 != color6)
		{
		    product2b = product1b = color5;
		}
		else if (color5 == color3 && color2 == color6)
		{
		    register int r = 0;

		    r += GetResult (color6, color5, color1, colorA1);
		    r += GetResult (color6, color5, color4, colorB1);
		    r += GetResult (color6, color5, colorA2, colorS1);
		    r += GetResult (color6, color5, colorB2, colorS2);

		    if (r > 0)
			product2b = product1b = color6;
		    else if (r < 0)
			product2b = product1b = color5;
		    else
		    {
			product2b = product1b = INTERPOLATE (color5, color6);
		    }
		}
		else
		{
		    if (color6 == color3 && color3 == colorA1
			    && color2 != colorA2 && color3 != colorA0)
			product2b =
			    Q_INTERPOLATE (color3, color3, color3, color2);
		    else if (color5 == color2 && color2 == colorA2
			     && colorA1 != color3 && color2 != colorA3)
			product2b =
			    Q_INTERPOLATE (color2, color2, color2, color3);
		    else
			product2b = INTERPOLATE (color2, color3);

		    if (color6 == color3 && color6 == colorB1
			    && color5 != colorB2 && color6 != colorB0)
			product1b =
			    Q_INTERPOLATE (color6, color6, color6, color5);
		    else if (color5 == color2 && color5 == colorB2
			     && colorB1 != color6 && color5 != colorB3)
			product1b =
			    Q_INTERPOLATE (color6, color5, color5, color5);
		    else
			product1b = INTERPOLATE (color5, color6);
		}

		if (color5 == color3 && color2 != color6 && color4 == color5
			&& color5 != colorA2)
		    product2a = INTERPOLATE (color2, color5);
		else
		    if (color5 == color1 && color6 == color5
			&& color4 != color2 && color5 != colorA0)
		    product2a = INTERPOLATE (color2, color5);
		else
		    product2a = color2;

		if (color2 == color6 && color5 != color3 && color1 == color2
			&& color2 != colorB2)
		    product1a = INTERPOLATE (color2, color5);
		else
		    if (color4 == color2 && color3 == color2
			&& color1 != color5 && color2 != colorB0)
		    product1a = INTERPOLATE (color2, color5);
		else
		    product1a = color5;

		product1a = product1a | (product1b << 16);
		product2a = product2a | (product2b << 16);

		*((uint32 *) dP) = product1a;
		*((uint32 *) (dP + dstPitch)) = product2a;

		bP += inc_bP;
		dP += sizeof (uint32);
	    }			// end of for ( finish= width etc..)

	    srcPtr   += srcPitch;
	    dstPtr   += dstPitch * 2;
	    deltaPtr += srcPitch;
	}			// while (height--)
    }
}

void SuperEagle (uint8 *srcPtr, uint32 srcPitch, uint8 *deltaPtr, 
		 uint8 *dstPtr, uint32 dstPitch, int width, int height)
{
    uint8  *dP;
    uint16 *bP;
    uint32 inc_bP;

    {
	inc_bP = 1;

	uint32 Nextline = srcPitch >> 1;

	while (height--)
	{
	    bP = (uint16 *) srcPtr;
	    dP = dstPtr;
	    for (uint32 finish = width; finish; finish -= inc_bP)
	    {
		uint32 color4, color5, color6;
		uint32 color1, color2, color3;
		uint32 colorA1, colorA2, colorB1, colorB2, colorS1, colorS2;
		uint32 product1a, product1b, product2a, product2b;

		colorB1 = *(bP - Nextline);
		colorB2 = *(bP - Nextline + 1);

		color4 = *(bP - 1);
		color5 = *(bP);
		color6 = *(bP + 1);
		colorS2 = *(bP + 2);

		color1 = *(bP + Nextline - 1);
		color2 = *(bP + Nextline);
		color3 = *(bP + Nextline + 1);
		colorS1 = *(bP + Nextline + 2);

		colorA1 = *(bP + Nextline + Nextline);
		colorA2 = *(bP + Nextline + Nextline + 1);

		// --------------------------------------
		if (color2 == color6 && color5 != color3)
		{
		    product1b = product2a = color2;
		    if ((color1 == color2) || (color6 == colorB2))
		    {
			product1a = INTERPOLATE (color2, color5);
			product1a = INTERPOLATE (color2, product1a);
		    }
		    else
		    {
			product1a = INTERPOLATE (color5, color6);
		    }

		    if ((color6 == colorS2) || (color2 == colorA1))
		    {
			product2b = INTERPOLATE (color2, color3);
			product2b = INTERPOLATE (color2, product2b);
		    }
		    else
		    {
			product2b = INTERPOLATE (color2, color3);
		    }
		}
		else if (color5 == color3 && color2 != color6)
		{
		    product2b = product1a = color5;

		    if ((colorB1 == color5) || (color3 == colorS1))
		    {
			product1b = INTERPOLATE (color5, color6);
			product1b = INTERPOLATE (color5, product1b);
		    }
		    else
		    {
			product1b = INTERPOLATE (color5, color6);
		    }

		    if ((color3 == colorA2) || (color4 == color5))
		    {
			product2a = INTERPOLATE (color5, color2);
			product2a = INTERPOLATE (color5, product2a);
		    }
		    else
		    {
			product2a = INTERPOLATE (color2, color3);
		    }

		}
		else if (color5 == color3 && color2 == color6)
		{
		    register int r = 0;

		    r += GetResult (color6, color5, color1, colorA1);
		    r += GetResult (color6, color5, color4, colorB1);
		    r += GetResult (color6, color5, colorA2, colorS1);
		    r += GetResult (color6, color5, colorB2, colorS2);

		    if (r > 0)
		    {
			product1b = product2a = color2;
			product1a = product2b = INTERPOLATE (color5, color6);
		    }
		    else if (r < 0)
		    {
			product2b = product1a = color5;
			product1b = product2a = INTERPOLATE (color5, color6);
		    }
		    else
		    {
			product2b = product1a = color5;
			product1b = product2a = color2;
		    }
		}
		else
		{
		    product2b = product1a = INTERPOLATE (color2, color6);
		    product2b =
			Q_INTERPOLATE (color3, color3, color3, product2b);
		    product1a =
			Q_INTERPOLATE (color5, color5, color5, product1a);

		    product2a = product1b = INTERPOLATE (color5, color3);
		    product2a = Q_INTERPOLATE (color2, color2, color2, product2a);
		    product1b = Q_INTERPOLATE (color6, color6, color6, product1b);
		}
		product1a = product1a | (product1b << 16);
		product2a = product2a | (product2b << 16);

		*((uint32 *) dP) = product1a;
		*((uint32 *) (dP + dstPitch)) = product2a;

		bP += inc_bP;
		dP += sizeof (uint32);
	    }			// end of for ( finish= width etc..)

	    srcPtr += srcPitch;
	    dstPtr += dstPitch * 2;
	    deltaPtr += srcPitch;
	}			// endof: while (height--)
    }
}

void _2xSaI (uint8 *srcPtr, uint32 srcPitch, uint8 *deltaPtr,
	     uint8 *dstPtr, uint32 dstPitch, int width, int height)
{
    uint8  *dP;
    uint16 *bP;
    uint32 inc_bP;


    {
	inc_bP = 1;

	uint32 Nextline = srcPitch >> 1;

	while (height--)
	{
	    bP = (uint16 *) srcPtr;
	    dP = dstPtr;

	    for (uint32 finish = width; finish; finish -= inc_bP)
	    {

		register uint32 colorA, colorB;
		uint32 colorC, colorD,
		    colorE, colorF, colorG, colorH,
		    colorI, colorJ, colorK, colorL,

		    colorM, colorN, colorO, colorP;
		uint32 product, product1, product2;

//---------------------------------------
// Map of the pixels:                    I|E F|J
//                                       G|A B|K
//                                       H|C D|L
//                                       M|N O|P
		colorI = *(bP - Nextline - 1);
		colorE = *(bP - Nextline);
		colorF = *(bP - Nextline + 1);
		colorJ = *(bP - Nextline + 2);

		colorG = *(bP - 1);
		colorA = *(bP);
		colorB = *(bP + 1);
		colorK = *(bP + 2);

		colorH = *(bP + Nextline - 1);
		colorC = *(bP + Nextline);
		colorD = *(bP + Nextline + 1);
		colorL = *(bP + Nextline + 2);

		colorM = *(bP + Nextline + Nextline - 1);
		colorN = *(bP + Nextline + Nextline);
		colorO = *(bP + Nextline + Nextline + 1);
		colorP = *(bP + Nextline + Nextline + 2);

		if ((colorA == colorD) && (colorB != colorC))
		{
		    if (((colorA == colorE) && (colorB == colorL)) ||
			    ((colorA == colorC) && (colorA == colorF)
			     && (colorB != colorE) && (colorB == colorJ)))
		    {
			product = colorA;
		    }
		    else
		    {
			product = INTERPOLATE (colorA, colorB);
		    }

		    if (((colorA == colorG) && (colorC == colorO)) ||
			    ((colorA == colorB) && (colorA == colorH)
			     && (colorG != colorC) && (colorC == colorM)))
		    {
			product1 = colorA;
		    }
		    else
		    {
			product1 = INTERPOLATE (colorA, colorC);
		    }
		    product2 = colorA;
		}
		else if ((colorB == colorC) && (colorA != colorD))
		{
		    if (((colorB == colorF) && (colorA == colorH)) ||
			    ((colorB == colorE) && (colorB == colorD)
			     && (colorA != colorF) && (colorA == colorI)))
		    {
			product = colorB;
		    }
		    else
		    {
			product = INTERPOLATE (colorA, colorB);
		    }

		    if (((colorC == colorH) && (colorA == colorF)) ||
			    ((colorC == colorG) && (colorC == colorD)
			     && (colorA != colorH) && (colorA == colorI)))
		    {
			product1 = colorC;
		    }
		    else
		    {
			product1 = INTERPOLATE (colorA, colorC);
		    }
		    product2 = colorB;
		}
		else if ((colorA == colorD) && (colorB == colorC))
		{
		    if (colorA == colorB)
		    {
			product = colorA;
			product1 = colorA;
			product2 = colorA;
		    }
		    else
		    {
			register int r = 0;

			product1 = INTERPOLATE (colorA, colorC);
			product = INTERPOLATE (colorA, colorB);

			r +=
                            GetResult1 (colorA, colorB, colorG, colorE,
					colorI);
			r +=
			    GetResult2 (colorB, colorA, colorK, colorF,
					colorJ);
			r +=
			    GetResult2 (colorB, colorA, colorH, colorN,
					colorM);
			r +=
			    GetResult1 (colorA, colorB, colorL, colorO,
					colorP);

			if (r > 0)
			    product2 = colorA;
			else if (r < 0)
			    product2 = colorB;
			else
			{
			    product2 =
				Q_INTERPOLATE (colorA, colorB, colorC,
					       colorD);
			}
		    }
		}
		else
		{
		    product2 = Q_INTERPOLATE (colorA, colorB, colorC, colorD);

		    if ((colorA == colorC) && (colorA == colorF)
			    && (colorB != colorE) && (colorB == colorJ))
		    {
			product = colorA;
		    }
		    else
			if ((colorB == colorE) && (colorB == colorD)
			    && (colorA != colorF) && (colorA == colorI))
		    {
			product = colorB;
		    }
		    else
		    {
			product = INTERPOLATE (colorA, colorB);
		    }

		    if ((colorA == colorB) && (colorA == colorH)
			    && (colorG != colorC) && (colorC == colorM))
		    {
			product1 = colorA;
		    }
		    else
			if ((colorC == colorG) && (colorC == colorD)
			    && (colorA != colorH) && (colorA == colorI))
		    {
			product1 = colorC;
		    }
		    else
		    {
			product1 = INTERPOLATE (colorA, colorC);
		    }
		}

		product = colorA | (product << 16);
		product1 = product1 | (product2 << 16);
		*((int32 *) dP) = product;
		*((uint32 *) (dP + dstPitch)) = product1;

		bP += inc_bP;
		dP += sizeof (uint32);
	    }			// end of for ( finish= width etc..)

	    srcPtr += srcPitch;
	    dstPtr += dstPitch * 2;
	    deltaPtr += srcPitch;
	}			// endof: while (height--)
    }
}

static uint32 Bilinear (uint32 A, uint32 B, uint32 x)
{
    unsigned long areaA, areaB;
    unsigned long result;

    if (A == B)
	return A;

    areaB = (x >> 11) & 0x1f;	// reduce 16 bit fraction to 5 bits
    areaA = 0x20 - areaB;

    A = (A & redblueMask) | ((A & greenMask) << 16);
    B = (B & redblueMask) | ((B & greenMask) << 16);

    result = ((areaA * A) + (areaB * B)) >> 5;

    return (result & redblueMask) | ((result >> 16) & greenMask);

}

static uint32 Bilinear4 (uint32 A, uint32 B, uint32 C, uint32 D, uint32 x,
			 uint32 y)
{
    unsigned long areaA, areaB, areaC, areaD;
    unsigned long result, xy;

    x = (x >> 11) & 0x1f;
    y = (y >> 11) & 0x1f;
    xy = (x * y) >> 5;

    A = (A & redblueMask) | ((A & greenMask) << 16);
    B = (B & redblueMask) | ((B & greenMask) << 16);
    C = (C & redblueMask) | ((C & greenMask) << 16);
    D = (D & redblueMask) | ((D & greenMask) << 16);

    areaA = 0x20 + xy - x - y;
    areaB = x - xy;
    areaC = y - xy;
    areaD = xy;

    result = ((areaA * A) + (areaB * B) + (areaC * C) + (areaD * D)) >> 5;

    return (result & redblueMask) | ((result >> 16) & greenMask);
}

void Scale_2xSaI (uint8 *srcPtr, uint32 srcPitch, uint8 * /* deltaPtr */,
		  uint8 *dstPtr, uint32 dstPitch, 
		  uint32 dstWidth, uint32 dstHeight, int width, int height)
{
    uint8  *dP;
    uint16 *bP;

    uint32 w;
    uint32 h;
    uint32 dw;
    uint32 dh;
    uint32 hfinish;
    uint32 wfinish;

    uint32 Nextline = srcPitch >> 1;

    wfinish = (width - 1) << 16;	// convert to fixed point
    dw = wfinish / (dstWidth - 1);
    hfinish = (height - 1) << 16;	// convert to fixed point
    dh = hfinish / (dstHeight - 1);

    for (h = 0; h < hfinish; h += dh)
    {
	uint32 y1, y2;

	y1 = h & 0xffff;	// fraction part of fixed point
	bP = (uint16 *) (srcPtr + ((h >> 16) * srcPitch));
	dP = dstPtr;
	y2 = 0x10000 - y1;

	w = 0;

	for (; w < wfinish;)
	{
	    uint32 A, B, C, D;
	    uint32 E, F, G, H;
	    uint32 I, J, K, L;
	    uint32 x1, x2, a1, f1, f2;
	    uint32 position, product1;

	    position = w >> 16;
	    A = bP[position];	// current pixel
	    B = bP[position + 1];	// next pixel
	    C = bP[position + Nextline];
	    D = bP[position + Nextline + 1];
	    E = bP[position - Nextline];
	    F = bP[position - Nextline + 1];
	    G = bP[position - 1];
	    H = bP[position + Nextline - 1];
	    I = bP[position + 2];
	    J = bP[position + Nextline + 2];
	    K = bP[position + Nextline + Nextline];
	    L = bP[position + Nextline + Nextline + 1];

	    x1 = w & 0xffff;	// fraction part of fixed point
	    x2 = 0x10000 - x1;

	    /*0*/ 
	    if (A == B && C == D && A == C)
		product1 = A;
	    else
	    /*1*/ 
	    if (A == D && B != C)
	    {
		f1 = (x1 >> 1) + (0x10000 >> 2);
		f2 = (y1 >> 1) + (0x10000 >> 2);
		if (y1 <= f1 && A == J && A != E)	// close to B
		{
		    a1 = f1 - y1;
		    product1 = Bilinear (A, B, a1);
		}
		else if (y1 >= f1 && A == G && A != L)	// close to C
		{
		    a1 = y1 - f1;
		    product1 = Bilinear (A, C, a1);
		}
		else if (x1 >= f2 && A == E && A != J)	// close to B
		{
		    a1 = x1 - f2;
		    product1 = Bilinear (A, B, a1);
		}
		else if (x1 <= f2 && A == L && A != G)	// close to C
		{
		    a1 = f2 - x1;
		    product1 = Bilinear (A, C, a1);
		}
		else if (y1 >= x1)	// close to C
		{
		    a1 = y1 - x1;
		    product1 = Bilinear (A, C, a1);
		}
		else if (y1 <= x1)	// close to B
		{
		    a1 = x1 - y1;
		    product1 = Bilinear (A, B, a1);
		}
	    }
	    else
	    /*2*/ 
	    if (B == C && A != D)
	    {
		f1 = (x1 >> 1) + (0x10000 >> 2);
		f2 = (y1 >> 1) + (0x10000 >> 2);
		if (y2 >= f1 && B == H && B != F)	// close to A
		{
		    a1 = y2 - f1;
		    product1 = Bilinear (B, A, a1);
		}
		else if (y2 <= f1 && B == I && B != K)	// close to D
		{
		    a1 = f1 - y2;
		    product1 = Bilinear (B, D, a1);
		}
		else if (x2 >= f2 && B == F && B != H)	// close to A
		{
		    a1 = x2 - f2;
		    product1 = Bilinear (B, A, a1);
		}
		else if (x2 <= f2 && B == K && B != I)	// close to D
		{
		    a1 = f2 - x2;
		    product1 = Bilinear (B, D, a1);
		}
		else if (y2 >= x1)	// close to A
		{
		    a1 = y2 - x1;
		    product1 = Bilinear (B, A, a1);
		}
		else if (y2 <= x1)	// close to D
		{
		    a1 = x1 - y2;
		    product1 = Bilinear (B, D, a1);
		}
	    }
	    /*3*/
	    else
	    {
		product1 = Bilinear4 (A, B, C, D, x1, y1);
	    }

//end First Pixel
	    *(uint32 *) dP = product1;
	    dP += 2;
	    w += dw;
	}
	dstPtr += dstPitch;
    }
}
