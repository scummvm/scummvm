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

#define SCALEUP_2x2

Scumm scumm;
ScummDebugger debugger;
Gui gui;

SoundEngine sound;

static SDL_Surface *screen;

void updateScreen(Scumm *s);

void updatePalette(Scumm *s) {
	SDL_Color colors[256];
	int first = s->_palDirtyMin;
	int num = s->_palDirtyMax - first + 1;
	int i;
	byte *data = s->_currentPalette;

	data += first*3;
	for (i=0; i<num; i++,data+=3) {
		colors[i].r = data[0]<<2;
		colors[i].g = data[1]<<2;
		colors[i].b = data[2]<<2;
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
				}
				if (event.key.keysym.sym=='z' && event.key.keysym.mod&KMOD_CTRL) {
					exit(1);
				} 
				if (event.key.keysym.sym=='f' && event.key.keysym.mod&KMOD_CTRL) {
					s->_fastMode ^= 1;
				}
				if (event.key.keysym.sym=='g' && event.key.keysym.mod&KMOD_CTRL) {
					s->_fastMode ^= 2;
				}

				if (event.key.keysym.sym=='d' && event.key.keysym.mod&KMOD_CTRL) {
					debugger.attach(s);
				}
				if (event.key.keysym.sym=='s' && event.key.keysym.mod&KMOD_CTRL) {
					s->resourceStats();
				}

	#if defined(__APPLE__)
				if (event.key.keysym.sym=='q' && event.key.keysym.mod&KMOD_LMETA) {
					exit(1);
				} 
	#endif
				break;
			case SDL_MOUSEMOTION: {
				int newx,newy;
	#if !defined(SCALEUP_2x2)
				newx = event.motion.x;
				newy = event.motion.y;
	#else
				newx = event.motion.x>>1;
				newy = event.motion.y>>1;
	#endif
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
					s->_leftBtnPressed |= 1;
				else if (event.button.button==SDL_BUTTON_RIGHT)
					s->_rightBtnPressed |= 1;
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
#define BAK_HEIGHT 24
byte old_backup[BAK_WIDTH*BAK_HEIGHT*2];


void addDirtyRect(int x, int y, int w, int h) {
	SDL_Rect *r;
	if (numDirtyRects==MAX_DIRTY_RECTS)
		fullRedraw = true;
	else if (!fullRedraw) {
		r = &dirtyRects[numDirtyRects++];
#if defined(SCALEUP_2x2)
		r->x = x*2;
		r->y = y*2;
		r->w = w*2;
		r->h = h*2;
#else
		r->x = x;
		r->y = y;
		r->w = w;
		r->h = h;
#endif
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

/* Copy part of bitmap */
void blitToScreen(Scumm *s, byte *src,int x, int y, int w, int h) {
	byte *dst;
	int i;

	hide_mouse = true;
	if (has_mouse) {
		s->drawMouse();
	}

	if (SDL_LockSurface(screen)==-1)
		error("SDL_LockSurface failed: %s.\n", SDL_GetError());

#if !defined(SCALEUP_2x2)
	dst = (byte*)screen->pixels + y*320 + x;
	addDirtyRect(x,y,w,h);
	do {
		memcpy(dst, src, w);
		dst += 320;
		src += 320;
	} while (--h);
#else
	dst = (byte*)screen->pixels + y*640*2 + x*2;
	addDirtyRect(x,y,w,h);		
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
#if defined(SHOW_AREA)
		debug(2,"update area 100 %%");
#endif
	} else if (numDirtyRects) {
#if defined(SHOW_AREA)
		int area = 0,i;
		for (i=0; i<numDirtyRects; i++)
			area += (dirtyRects[i].w * dirtyRects[i].h);
		debug(2,"update area %f %%", (float)area/640);
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

	if (SDL_LockSurface(screen)==-1)
		error("SDL_LockSurface failed: %s.\n", SDL_GetError());

#if defined(SCALEUP_2x2)

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
#else
	if (has_mouse) {
		dst = (byte*)screen->pixels + old_mouse_y*320 + old_mouse_x;
		bak = old_backup;

		for (y=0; y<h; y++,bak+=BAK_WIDTH,dst+=320) {
			if ( (uint)(old_mouse_y + y) < 200) {
				for (x=0; x<w; x++) {
					if ((uint)(old_mouse_x + x) < 320) {
						dst[x] = bak[x];
					}
				}
			}
		}
	}
	if (visible) {
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


#endif	

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

void drawMouse(Scumm *s, int xdraw, int ydraw, int color, byte *mask, bool visible) {
	int x,y;
	uint32 bits;
	byte *dst,*bak;

	if (hide_mouse)
		visible = false;

	if (SDL_LockSurface(screen)==-1)
		error("SDL_LockSurface failed: %s.\n", SDL_GetError());

#if defined(SCALEUP_2x2)

	if (has_mouse) {
		dst = (byte*)screen->pixels + old_mouse_y*640*2 + old_mouse_x*2;
		bak = old_backup;

		for (y=0; y<16; y++,bak+=48,dst+=640*2) {
			if ( (uint)(old_mouse_y + y) < 200) {
				for (x=0; x<24; x++) {
					if ((uint)(old_mouse_x + x) < 320) {
						dst[x*2+640] = dst[x*2] = bak[x*2];
						dst[x*2+640+1] = dst[x*2+1] = bak[x*2+1];
					}
				}
			}
		}
	}

	if (visible) {
		dst = (byte*)screen->pixels + ydraw*640*2 + xdraw*2;
		bak = old_backup;

		for (y=0; y<16; y++,dst+=640*2,bak+=48) {
			bits = mask[3] | (mask[2]<<8) | (mask[1]<<16);
			mask += 4;
			if ((uint)(ydraw+y)<200) {
				for (x=0; x<24; x++,bits<<=1) {
					if ((uint)(xdraw+x)<320) {
						bak[x*2] = dst[x*2];
						bak[x*2+1] = dst[x*2+1];
						if (bits&(1<<23)) {
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
#else
	if (has_mouse) {
		dst = (byte*)screen->pixels + old_mouse_y*320 + old_mouse_x;
		bak = old_backup;

		for (y=0; y<16; y++,bak+=24,dst+=320) {
			if ( (uint)(old_mouse_y + y) < 200) {
				for (x=0; x<24; x++) {
					if ((uint)(old_mouse_x + x) < 320) {
						dst[x] = bak[x];
					}
				}
			}
		}
	}
	if (visible) {
		dst = (byte*)screen->pixels + ydraw*320 + xdraw;
		bak = old_backup;

		for (y=0; y<16; y++,dst+=320,bak+=24) {
			bits = mask[3] | (mask[2]<<8) | (mask[1]<<16);
			mask += 4;
			if ((uint)(ydraw+y)<200) {
				for (x=0; x<24; x++,bits<<=1) {
					if ((uint)(xdraw+x)<320) {
						bak[x] = dst[x];
						if (bits&(1<<23)) {
							dst[x] = color;
						}
					}
				}
			}
		}
	}


#endif	

	SDL_UnlockSurface(screen);

	if (has_mouse) {
		has_mouse = false;
		addDirtyRectClipped(old_mouse_x, old_mouse_y, 24, 16);
	}

	if (visible) {
		has_mouse = true;
		addDirtyRectClipped(xdraw, ydraw, 24, 16);
		old_mouse_x = xdraw;
		old_mouse_y = ydraw;
	}
}

static uint32 midi_counter;

void fill_sound(void *userdata, Uint8 *stream, int len) {
	memset(stream, 0, len);
	scumm.mixWaves((int16*)stream, len>>1);
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

void initGraphics(Scumm *s, bool fullScreen) {
	SDL_AudioSpec desired;

	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO)==-1) {
		error("Could not initialize SDL: %s.\n", SDL_GetError());
	    exit(1);
	}

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

	/* Create Music Thread */
	SDL_CreateThread((int (*)(void *))&music_thread, &scumm);

#if !defined(SCALEUP_2x2)
	screen = SDL_SetVideoMode(320, 200, 8, fullScreen ? (SDL_SWSURFACE | SDL_FULLSCREEN) : SDL_SWSURFACE);
#else
	screen = SDL_SetVideoMode(640, 400, 8, fullScreen ? (SDL_SWSURFACE | SDL_FULLSCREEN) : SDL_SWSURFACE);
#endif

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
	int delta,tmp;
	int last_time, new_time;

	sound.initialize(&scumm);
	scumm._soundDriver = &sound;

	scumm._gui = &gui;
	scumm.scummMain(argc, argv);

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

