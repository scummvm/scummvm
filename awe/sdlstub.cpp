/* AWE - Another World Engine
 * Copyright (C) 2004 Gregory Montoir
 * Copyright (C) 2004 The ScummVM project
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
 */

#include "stdafx.h"
#include "common/util.h"

#include <SDL.h>

#include "awe.h"
#include "systemstub.h"
#include "util.h"

namespace Awe {

struct SDLStub : SystemStub {
	typedef void (SDLStub::*ScaleProc)(uint16 *dst, uint16 dstPitch, const uint16 *src, uint16 srcPitch, uint16 w, uint16 h);

	enum {
		SCREEN_W = 320,
		SCREEN_H = 200
	};

	struct Scaler {
		const char *name;
		ScaleProc proc;
		uint8 factor;
	};
	
	static const Scaler _scalers[];

	uint8 *_offscreen;
	SDL_Surface *_screen;
	SDL_Surface *_sclscreen;
	bool _fullscreen;
	uint8 _scaler;
	uint16 _pal[16];

	virtual ~SDLStub() {}
	virtual void init(const char *title);
	virtual void destroy();
	virtual void setPalette(uint8 s, uint8 n, const uint8 *buf);
	virtual void copyRect(uint16 x, uint16 y, uint16 w, uint16 h, const uint8 *buf, uint32 pitch);
	virtual void processEvents();
	virtual void sleep(uint32 duration);
	virtual uint32 getTimeStamp();

	void prepareGfxMode();
	void cleanupGfxMode();
	void switchGfxMode(bool fullscreen, uint8 scaler);

	void point1x(uint16 *dst, uint16 dstPitch, const uint16 *src, uint16 srcPitch, uint16 w, uint16 h);
	void point2x(uint16 *dst, uint16 dstPitch, const uint16 *src, uint16 srcPitch, uint16 w, uint16 h);
	void point3x(uint16 *dst, uint16 dstPitch, const uint16 *src, uint16 srcPitch, uint16 w, uint16 h);
};

const SDLStub::Scaler SDLStub::_scalers[] = {
	{ "Point1x", &SDLStub::point1x, 1 },
	{ "Point2x", &SDLStub::point2x, 2 },
	{ "Point3x", &SDLStub::point3x, 3 }
};

SystemStub *SystemStub_SDL_create() {
	return new SDLStub();
}

void SDLStub::init(const char *title) {
	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER);
	SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);
	SDL_ShowCursor(SDL_DISABLE);
	SDL_WM_SetCaption(title, NULL);
	memset(&_pi, 0, sizeof(_pi));
	_offscreen = (uint8 *)malloc(SCREEN_W * SCREEN_H * 2);
	if (!_offscreen) {
		::error("Unable to allocate offscreen buffer");
	}
	_fullscreen = false;
	_scaler = 1;
	prepareGfxMode();
}

void SDLStub::destroy() {
	cleanupGfxMode();
}

void SDLStub::setPalette(uint8 s, uint8 n, const uint8 *buf) {
	assert(s + n <= 16);
	for (int i = s; i < s + n; ++i) {
		uint8 c[3];
		for (int j = 0; j < 3; ++j) {
			uint8 col = buf[i * 3 + j];
			c[j] =  (col << 2) | (col & 3);
		}
		_pal[i] = SDL_MapRGB(_screen->format, c[0], c[1], c[2]);
	}	
}

void SDLStub::copyRect(uint16 x, uint16 y, uint16 w, uint16 h, const uint8 *buf, uint32 pitch) {
	buf += y * pitch + x;
	uint16 *p = (uint16 *)_offscreen;
	while (h--) {
		for (int i = 0; i < w / 2; ++i) {
			uint8 p1 = *(buf + i) >> 4;
			uint8 p2 = *(buf + i) & 0xF;
			*(p + i * 2 + 0) = _pal[p1];
			*(p + i * 2 + 1) = _pal[p2];
		}
		p += SCREEN_W;
		buf += pitch;
	}
	SDL_LockSurface(_sclscreen);
	(this->*_scalers[_scaler].proc)((uint16 *)_sclscreen->pixels, _sclscreen->pitch, (uint16 *)_offscreen, SCREEN_W, SCREEN_W, SCREEN_H);
	SDL_UnlockSurface(_sclscreen);
	SDL_BlitSurface(_sclscreen, NULL, _screen, NULL);
	SDL_UpdateRect(_screen, 0, 0, 0, 0);
}

void SDLStub::processEvents() {
	SDL_Event ev;
	while(SDL_PollEvent(&ev)) {
		switch (ev.type) {
		case SDL_QUIT:
			exit(0);
			break;
		case SDL_KEYUP:
			switch(ev.key.keysym.sym) {
			case SDLK_LEFT:
				_pi.dirMask &= ~PlayerInput::DIR_LEFT;
				break;
			case SDLK_RIGHT:
				_pi.dirMask &= ~PlayerInput::DIR_RIGHT;
				break;
			case SDLK_UP:
				_pi.dirMask &= ~PlayerInput::DIR_UP;
				break;
			case SDLK_DOWN:
				_pi.dirMask &= ~PlayerInput::DIR_DOWN;
				break;
			case SDLK_SPACE:
			case SDLK_RETURN:
				_pi.button = false;
				break;
			default:
				break;
			}
			break;
		case SDL_KEYDOWN:
			if (ev.key.keysym.mod & KMOD_ALT) {
				if (ev.key.keysym.sym == SDLK_RETURN) {
					switchGfxMode(!_fullscreen, _scaler);
				} else if (ev.key.keysym.sym == SDLK_KP_PLUS) {
					uint8 s = _scaler + 1;
					if (s < ARRAYSIZE(_scalers)) {
						switchGfxMode(_fullscreen, s);
					}
				} else if (ev.key.keysym.sym == SDLK_KP_MINUS) {
					int8 s = _scaler - 1;
					if (_scaler > 0) {
						switchGfxMode(_fullscreen, s);
					}
				} else if (ev.key.keysym.sym == SDLK_x) {
					_pi.quit = true;
				}
				break;
			} else if (ev.key.keysym.mod & KMOD_CTRL) {
				if (ev.key.keysym.sym == SDLK_s) {
					_pi.save = true;
				} else if (ev.key.keysym.sym == SDLK_l) {
					_pi.load = true;
				} else if (ev.key.keysym.sym == SDLK_f) {
					_pi.fastMode = true;
				} else if (ev.key.keysym.sym == SDLK_KP_PLUS) {
					_pi.stateSlot = 1;
				} else if (ev.key.keysym.sym == SDLK_KP_MINUS) {
					_pi.stateSlot = -1;
				}
				break;
			}
			_pi.lastChar = ev.key.keysym.sym;
			switch(ev.key.keysym.sym) {
			case SDLK_LEFT:
				_pi.dirMask |= PlayerInput::DIR_LEFT;
				break;
			case SDLK_RIGHT:
				_pi.dirMask |= PlayerInput::DIR_RIGHT;
				break;
			case SDLK_UP:
				_pi.dirMask |= PlayerInput::DIR_UP;
				break;
			case SDLK_DOWN:
				_pi.dirMask |= PlayerInput::DIR_DOWN;
				break;
			case SDLK_SPACE:
			case SDLK_RETURN:
				_pi.button = true;
				break;
			case SDLK_c:
				_pi.code = true;
				break;
			case SDLK_p:
				_pi.pause = true;
				break;
			default:
				break;
			}
			break;
		default:
			break;
		}
	}
}

void SDLStub::sleep(uint32 duration) {
	SDL_Delay(duration);
}

uint32 SDLStub::getTimeStamp() {
	return SDL_GetTicks();	
}

void SDLStub::prepareGfxMode() {
	int w = SCREEN_W * _scalers[_scaler].factor;
	int h = SCREEN_H * _scalers[_scaler].factor;
	_screen = SDL_SetVideoMode(w, h, 16, _fullscreen ? (SDL_FULLSCREEN | SDL_HWSURFACE) : SDL_HWSURFACE);
	if (!_screen) {
		::error("SDLStub::prepareGfxMode() unable to allocate _screen buffer");
	}
	_sclscreen = SDL_CreateRGBSurface(SDL_SWSURFACE, w, h, 16,
						_screen->format->Rmask,
						_screen->format->Gmask,
						_screen->format->Bmask,
						_screen->format->Amask);
	if (!_sclscreen) {
		::error("SDLStub::prepareGfxMode() unable to allocate _sclscreen buffer");
	}
}

void SDLStub::cleanupGfxMode() {
	if (_offscreen) {
		free(_offscreen);
		_offscreen = 0;
	}
	if (_sclscreen) {
		SDL_FreeSurface(_sclscreen);
		_sclscreen = 0;
	}
	if (_screen) {
		SDL_FreeSurface(_screen);
		_screen = 0;
	}
}

void SDLStub::switchGfxMode(bool fullscreen, uint8 scaler) {
	SDL_Surface *prev_sclscreen = _sclscreen;
	SDL_FreeSurface(_screen); 	
	_fullscreen = fullscreen;
	_scaler = scaler;
	prepareGfxMode();
	SDL_BlitSurface(prev_sclscreen, NULL, _sclscreen, NULL);
	SDL_FreeSurface(prev_sclscreen);
}

void SDLStub::point1x(uint16 *dst, uint16 dstPitch, const uint16 *src, uint16 srcPitch, uint16 w, uint16 h) {
	dstPitch >>= 1;
	while (h--) {
		memcpy(dst, src, w * 2);
		dst += dstPitch;
		src += dstPitch;
	}
}

void SDLStub::point2x(uint16 *dst, uint16 dstPitch, const uint16 *src, uint16 srcPitch, uint16 w, uint16 h) {
	dstPitch >>= 1;
	while (h--) {
		uint16 *p = dst;
		for (int i = 0; i < w; ++i, p += 2) {
			uint16 c = *(src + i);
			*(p + 0) = c;
			*(p + 1) = c;
			*(p + 0 + dstPitch) = c;
			*(p + 1 + dstPitch) = c;
		}
		dst += dstPitch * 2;
		src += srcPitch;
	}
}

void SDLStub::point3x(uint16 *dst, uint16 dstPitch, const uint16 *src, uint16 srcPitch, uint16 w, uint16 h) {
	dstPitch >>= 1;
	while (h--) {
		uint16 *p = dst;
		for (int i = 0; i < w; ++i, p += 3) {
			uint16 c = *(src + i);
			*(p + 0) = c;
			*(p + 1) = c;
			*(p + 2) = c;
			*(p + 0 + dstPitch) = c;
			*(p + 1 + dstPitch) = c;
			*(p + 2 + dstPitch) = c;
			*(p + 0 + dstPitch * 2) = c;
			*(p + 1 + dstPitch * 2) = c;
			*(p + 2 + dstPitch * 2) = c;
		}
		dst += dstPitch * 3;
		src += srcPitch;
	}
}

}
