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
 * Revision 1.2  2001/10/09 17:38:20  strigeus
 * Autodetection of endianness.
 *
 * Revision 1.1.1.1  2001/10/09 14:30:13  strigeus
 *
 * initial revision
 *
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
				break;
			case SDL_MOUSEMOTION:
#if !defined(SCALEUP_2x2)
				s->mouse.x = event.motion.x;
				s->mouse.y = event.motion.y;
#else
				s->mouse.x = event.motion.x>>1;
				s->mouse.y = event.motion.y>>1;
#endif
				break;
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

/* Copy part of bitmap */
void blitToScreen(Scumm *s, byte *src,int x, int y, int w, int h) {
	byte *dst;
	SDL_Rect *r;
	int i;

	if (SDL_LockSurface(screen)==-1)
		error("SDL_LockSurface failed: %s.\n", SDL_GetError());

#if !defined(SCALEUP_2x2)
	dst = (byte*)screen->pixels + y*320 + x;

	if (numDirtyRects==MAX_DIRTY_RECTS)
		fullRedraw = true;
	else if (!fullRedraw) {
		r = &dirtyRects[numDirtyRects++];
		r->x = x;
		r->y = y;
		r->w = w;
		r->h = h;
	}
		
	do {
		memcpy(dst, src, w);
		dst += 640;
		src += 320;
	} while (--h);
#else
	dst = (byte*)screen->pixels + y*640*2 + x*2;

	if (numDirtyRects==MAX_DIRTY_RECTS)
		fullRedraw = true;
	else if (!fullRedraw) {
		r = &dirtyRects[numDirtyRects++];
		r->x = x*2;
		r->y = y*2;
		r->w = w*2;
		r->h = h*2;
	}
		
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

#undef main
int main(int argc, char* argv[]) {
	if (SDL_Init(SDL_INIT_VIDEO)==-1) {
		printf("Could not initialize SDL: %s.\n", SDL_GetError());
        return -1;
	}
	
	printf("%d %d, %d %d, %d %d %d, %d %d %d %d %d\n", 
		sizeof(int8), sizeof(uint8),
		sizeof(int16), sizeof(uint16),
		sizeof(int32), sizeof(uint32),
		sizeof(void*),
		sizeof(Box), sizeof(MouseCursor),sizeof(CodeHeader),
		sizeof(ImageHeader),
		&((CodeHeader*)0)->unk4
	);

	/* Clean up on exit */
    atexit(SDL_Quit);

#if !defined(SCALEUP_2x2)
	screen = SDL_SetVideoMode(320, 200, 8, SDL_SWSURFACE);
#else
	screen = SDL_SetVideoMode(640, 400, 8, SDL_SWSURFACE);
#endif
	scumm._videoMode = 0x13;
	scumm.scummMain();

	return 0;
}
