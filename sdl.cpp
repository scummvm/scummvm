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

static int current_shake_pos;

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

	#if defined(__APPLE__)
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
	if (fullRedraw) {
		SDL_UpdateRect(screen, 0,0,0,0);
		fullRedraw = false;
#if defined(SHOW_AREA)
		debug(2,"update area 100 %%");
#endif
	} else if (numDirtyRects) {
#if defined(SHOW_AREA)
		int area = 0,i;
		for (i=0; i<numDirtyRects; i++)
			area += (dirtyRects[i].w * dirtyRects[i].h);
		debug(2,"update area %f %%", (float)area/(640*400/100));
#endif

		SDL_UpdateRects(screen, numDirtyRects, dirtyRects);	
	}

	numDirtyRects = 0;
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

static uint32 midi_counter;

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
	

	if (scale == 3)
	{
		screen = SDL_SetVideoMode(960, 600, 8, fullScreen ? (SDL_SWSURFACE | SDL_FULLSCREEN) : (SDL_SWSURFACE | SDL_DOUBLEBUF));
	} else
	if (scale == 2)
	{
		screen = SDL_SetVideoMode(640, 400, 8, fullScreen ? (SDL_SWSURFACE | SDL_FULLSCREEN) : (SDL_SWSURFACE | SDL_DOUBLEBUF));
	} else
	{
		screen = SDL_SetVideoMode(320, 200, 8, fullScreen ? (SDL_SWSURFACE | SDL_FULLSCREEN) : (SDL_SWSURFACE | SDL_DOUBLEBUF));
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

int main(int argc, char* argv[]) {
	int delta;
	int last_time, new_time;

	sound.initialize(&scumm, &snd_driv);

	scumm._gui = &gui;
        scumm.scummMain(argc, argv);

        if (!(scumm._features & GF_SMALL_HEADER))
                gui.init(&scumm);

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
	rx1[2] = box.lr.x*2; ry1[2] = box.lr.y*2+32;
	rx1[3] = box.ll.x*2; ry1[3] = box.ll.y*2+32;

	filledPolygonColor(screen, &rx1[0], &ry1[0], 4, 255);
	SDL_UpdateRect(screen, 0,0,0,0);
	
}

