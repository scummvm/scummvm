/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "common/system.h"
#include "common/events.h"
#include "graphics/paletteman.h"
#include "graphics/screen.h"
#include "awe/awe.h"
#include "awe/system_stub.h"
#include "awe/util.h"

namespace Awe {

struct ScummVMStub : SystemStub {
	typedef void (ScummVMStub::*ScaleProc)(uint16 *dst, uint16 dstPitch, const uint16 *src, uint16 srcPitch, uint16 w, uint16 h);

	enum {
		SCREEN_W = 320,
		SCREEN_H = 200
	};

	uint8 *_offscreen;
	Graphics::Screen *_screen;
	byte _pal[16];

	virtual ~ScummVMStub() {}
	virtual void init(const char *title);
	virtual void destroy();
	virtual void setPalette(uint8 s, uint8 n, const uint8 *buf);
	virtual void copyRect(uint16 x, uint16 y, uint16 w, uint16 h, const uint8 *buf, uint32 pitch);
	virtual void processEvents();
	virtual void sleep(uint32 duration);
	virtual uint32 getTimeStamp();

	void prepareGfxMode();
	void cleanupGfxMode();
};

SystemStub *SystemStub_create() {
	return new ScummVMStub();
}

void ScummVMStub::init(const char *title) {
	memset(&_pi, 0, sizeof(_pi));
	_offscreen = (uint8 *)malloc(SCREEN_W * SCREEN_H * 2);
	if (!_offscreen) {
		error("Unable to allocate offscreen buffer");
	}

	prepareGfxMode();
}

void ScummVMStub::destroy() {
	cleanupGfxMode();
}

void ScummVMStub::setPalette(uint8 s, uint8 n, const uint8 *buf) {
	assert(s + n <= 16);
	for (int i = s; i < s + n; ++i) {
		uint8 c[3];
		for (int j = 0; j < 3; ++j) {
			uint8 col = buf[i * 3 + j];
			c[j] =  (col << 2) | (col & 3);
		}

		g_system->getPaletteManager()->setPalette(
			c, i, 1);
		_pal[i] = i;
	}	
}

void ScummVMStub::copyRect(uint16 x, uint16 y, uint16 w, uint16 h, const uint8 *buf, uint32 pitch) {
	buf += y * pitch + x;
	byte *p = (byte *)_offscreen;
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

	_screen->copyRectToSurface(_offscreen, SCREEN_W,
		0, 0, SCREEN_W, SCREEN_H);
	_screen->update();
}

void ScummVMStub::processEvents() {
	Common::Event ev;

	while (!g_engine->shouldQuit() &&
			g_system->getEventManager()->pollEvent(ev)) {
		switch (ev.type) {
		case Common::EVENT_KEYUP:
			switch(ev.kbd.keycode) {
			case Common::KEYCODE_LEFT:
				_pi.dirMask &= ~PlayerInput::DIR_LEFT;
				break;
			case Common::KEYCODE_RIGHT:
				_pi.dirMask &= ~PlayerInput::DIR_RIGHT;
				break;
			case Common::KEYCODE_UP:
				_pi.dirMask &= ~PlayerInput::DIR_UP;
				break;
			case Common::KEYCODE_DOWN:
				_pi.dirMask &= ~PlayerInput::DIR_DOWN;
				break;
			case Common::KEYCODE_SPACE:
			case Common::KEYCODE_RETURN:
				_pi.button = false;
				break;
			default:
				break;
			}
			break;
		case Common::EVENT_KEYDOWN:
			if (ev.kbd.flags & Common::KBD_ALT) {
				if (ev.kbd.keycode == Common::KEYCODE_x) {
					_pi.quit = true;
				}
				break;
			} else if (ev.kbd.flags & Common::KBD_CTRL) {
				if (ev.kbd.keycode == Common::KEYCODE_s) {
					_pi.save = true;
				} else if (ev.kbd.keycode == Common::KEYCODE_l) {
					_pi.load = true;
				} else if (ev.kbd.keycode == Common::KEYCODE_f) {
					_pi.fastMode = true;
				} else if (ev.kbd.keycode == Common::KEYCODE_KP_PLUS) {
					_pi.stateSlot = 1;
				} else if (ev.kbd.keycode == Common::KEYCODE_KP_MINUS) {
					_pi.stateSlot = -1;
				}
				break;
			}
			_pi.lastChar = ev.kbd.keycode;
			switch(ev.kbd.keycode) {
			case Common::KEYCODE_LEFT:
				_pi.dirMask |= PlayerInput::DIR_LEFT;
				break;
			case Common::KEYCODE_RIGHT:
				_pi.dirMask |= PlayerInput::DIR_RIGHT;
				break;
			case Common::KEYCODE_UP:
				_pi.dirMask |= PlayerInput::DIR_UP;
				break;
			case Common::KEYCODE_DOWN:
				_pi.dirMask |= PlayerInput::DIR_DOWN;
				break;
			case Common::KEYCODE_SPACE:
			case Common::KEYCODE_RETURN:
				_pi.button = true;
				break;
			case Common::KEYCODE_c:
				_pi.code = true;
				break;
			case Common::KEYCODE_p:
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

void ScummVMStub::sleep(uint32 duration) {
	g_system->delayMillis(duration);
}

uint32 ScummVMStub::getTimeStamp() {
	return g_system->getMillis();	
}

void ScummVMStub::prepareGfxMode() {
//	int w = SCREEN_W * _scalers[_scaler].factor;
//	int h = SCREEN_H * _scalers[_scaler].factor;
	_screen = new Graphics::Screen();
}

void ScummVMStub::cleanupGfxMode() {
	if (_offscreen) {
		free(_offscreen);
		_offscreen = nullptr;
	}

	if (_screen) {
		delete _screen;
		_screen = nullptr;
	}
}

} // namespace Awe
