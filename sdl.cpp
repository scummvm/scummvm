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
 * Change Log:
 * $Log$
 * Revision 1.7.2.1  2001/10/12 07:25:32  yazoo
 * Patched for indy4
 *
 * Revision 1.7  2001/10/11 11:49:51  strigeus
 * Determine caption from file name.
 *
 * Revision 1.6  2001/10/11 10:15:58  strigeus
 * no SDL cursor
 *
 * Revision 1.5  2001/10/10 11:53:39  strigeus
 * smoother mouse + bug fix
 *
 * Revision 1.4  2001/10/10 10:02:33  strigeus
 * alternative mouse cursor
 * basic save&load
 *
 * Revision 1.3  2001/10/09 19:02:28  strigeus
 * command line parameter support
 *
 * Revision 1.2  2001/10/09 17:38:20  strigeus
 * Autodetection of endianness.
 *
 * Revision 1.1.1.1  2001/10/09 14:30:13  strigeus
 * initial revision
 *
 */

#define NEED_SDL_HEADERS

#include "stdafx.h"
#include "scumm.h"

#define SCALEUP_2x2

Scumm scumm;

static SDL_Surface *screen;

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

void waitForTimer(Scumm *s) {
	SDL_Event event;
	byte dontPause = true;
	
	do {
		while (SDL_PollEvent(&event)) {
			switch(event.type) {
			case SDL_KEYDOWN:
				s->_keyPressed = event.key.keysym.sym;
				if (event.key.keysym.sym >= '0' && event.key.keysym.sym<='9') {
					s->_saveLoadSlot = event.key.keysym.sym - '0';
					if (event.key.keysym.mod&KMOD_SHIFT)
						s->_saveLoadFlag = 1;
					else if (event.key.keysym.mod&KMOD_CTRL)
						s->_saveLoadFlag = 2;
				}
				if (event.key.keysym.sym=='z' && event.key.keysym.mod&KMOD_CTRL) {
					exit(1);
				}
				if (event.key.keysym.sym=='f' && event.key.keysym.mod&KMOD_CTRL) {
					s->_fastMode ^= 1;
				}
				
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
#if 0
			case SDL_ACTIVEEVENT:
				if (event.active.state & SDL_APPINPUTFOCUS) {
					dontPause = event.active.gain;
				}
				break;
#endif
			case SDL_QUIT:
				exit(1);
				break;
			}
		}
		SDL_Delay(dontPause ? 10 : 100);
	} while (!dontPause);

	s->_scummTimer+=3;
}

#define MAX_DIRTY_RECTS 40
SDL_Rect dirtyRects[MAX_DIRTY_RECTS];
int numDirtyRects;
bool fullRedraw;

int old_mouse_x, old_mouse_y;
bool has_mouse,hide_mouse;
byte old_backup[24*16*2];


void addDirtyRect(int x, int y, int w, int h) {
	SDL_Rect *r;
	if (numDirtyRects==MAX_DIRTY_RECTS)
		fullRedraw = true;
	else if (!fullRedraw) {
		r = &dirtyRects[numDirtyRects++];
		r->x = x*2;
		r->y = y*2;
		r->w = w*2;
		r->h = h*2;
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
		dst += 640;
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

void drawMouse(Scumm *s, int xdraw, int ydraw, int color, byte *mask, bool visible) {
	int x,y;
	uint32 bits;
	byte *dst,*bak;

	if (hide_mouse)
		visible = false;

	if (SDL_LockSurface(screen)==-1)
		error("SDL_LockSurface failed: %s.\n", SDL_GetError());

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

void initGraphics(Scumm *s) {
	if (SDL_Init(SDL_INIT_VIDEO)==-1) {
		error("Could not initialize SDL: %s.\n", SDL_GetError());
    exit(1);
	}

	/* Clean up on exit */
  	atexit(SDL_Quit);

	char buf[512], *gameName;
	
	sprintf(buf, "ScummVM - %s", gameName = s->getGameName());
	free(gameName);
	
	SDL_WM_SetCaption(buf,buf);
	SDL_ShowCursor(SDL_DISABLE);

#if !defined(SCALEUP_2x2)
	screen = SDL_SetVideoMode(320, 200, 8, SDL_SWSURFACE);
#else
	screen = SDL_SetVideoMode(640, 400, 8, SDL_SWSURFACE);
#endif

	printf("%d %d, %d %d, %d %d %d, %d %d %d %d %d %d\n", 
		sizeof(int8), sizeof(uint8),
		sizeof(int16), sizeof(uint16),
		sizeof(int32), sizeof(uint32),
		sizeof(void*),
		sizeof(Box), sizeof(MouseCursor),sizeof(CodeHeader),
		sizeof(ImageHeader),
		&((CodeHeader*)0)->unk4,
		sizeof(Scumm)
	);


}

#undef main
int main(int argc, char* argv[]) {
	scumm._exe_name = "atlantis";
	scumm._videoMode = 0x13;
	scumm.scummMain(argc, argv);
	return 0;
}

