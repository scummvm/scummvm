/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#if defined(__ANDROID__)

// Allow use of stuff in <time.h>
#define FORBIDDEN_SYMBOL_EXCEPTION_time_h

//
// Disable printf override in common/forbidden.h to avoid
// clashes with log.h from the Android SDK.
// That header file uses
//   __attribute__ ((format(printf, 3, 4)))
// which gets messed up by our override mechanism; this could
// be avoided by either changing the Android SDK to use the equally
// legal and valid
//   __attribute__ ((format(printf, 3, 4)))
// or by refining our printf override to use a varadic macro
// (which then wouldn't be portable, though).
// Anyway, for now we just disable the printf override globally
// for the Android port
#define FORBIDDEN_SYMBOL_EXCEPTION_printf

#include "common/fs.h"
#include "common/stream.h"
#include "common/archive.h"
#include "image/tga.h"

#include "backends/platform/android3d/android.h"
#include "backends/platform/android3d/events.h"
#include "backends/platform/android3d/texture.h"
#include "backends/platform/android3d/touchcontrols.h"

static Common::Rect clipFor(const Common::KeyCode &cs) {
	switch (cs) {
	case Common::KEYCODE_UP:
	case Common::KEYCODE_PAGEUP:
		return Common::Rect(0, 0, 128, 128);
	case Common::KEYCODE_RIGHT:
		return Common::Rect(128, 0, 256, 128);
	case Common::KEYCODE_DOWN:
	case Common::KEYCODE_PAGEDOWN:
		return Common::Rect(256, 0, 384, 128);
	case Common::KEYCODE_LEFT:
		return Common::Rect(384, 0, 512, 128);
	case Common::KEYCODE_i:
		return Common::Rect(0, 128, 128, 256);
	case Common::KEYCODE_p:
		return Common::Rect(128, 128, 256, 256);
	case Common::KEYCODE_u:
		return Common::Rect(256, 128, 384, 256);
	case Common::KEYCODE_e:
	case Common::KEYCODE_l:
		return Common::Rect(384, 128, 512, 256);
	default: // unreachable
		return Common::Rect(0, 0, 1, 1);
	}
}

TouchControls::TouchControls() :
	_arrows_texture(NULL),
	_joystickPressing(Common::KEYCODE_INVALID),
	_centerPressing(Common::KEYCODE_INVALID),
	_rightPressing(Common::KEYCODE_INVALID),
	_screen_width(0),
	_screen_height(0) {

	for (int p = 0; p < kNumPointers; ++p) {
		Pointer &pp = _pointers[p];
		pp.currentX = pp.currentY = pp.startX = pp.startY = 0;
		pp.active = false;
		pp.function = kTouchAreaNone;
	}

	for (int i = 0; i < 4; ++i)
		_activePointers[i] = -1;
}

TouchControls::~TouchControls() {
	if (_arrows_texture) {
		delete _arrows_texture;
		_arrows_texture = 0;
	}
}

uint16 TouchControls::getTouchArea(int x, int y) {
	float xPercent = float(x) / _screen_width;

	if (xPercent < 0.3)
		return kTouchAreaJoystick;
	else if (xPercent < 0.8)
		return kTouchAreaCenter;
	else
		return kTouchAreaRight;
}

static Common::KeyCode determineKey(int dX, int dY, Common::KeyCode def = Common::KEYCODE_INVALID) {
	if (dX * dX + dY * dY < 50 * 50)
		return def;

	if (dY > abs(dX))
		return Common::KEYCODE_DOWN;
	if (dX > abs(dY))
		return Common::KEYCODE_RIGHT;
	if (-dY > abs(dX))
		return Common::KEYCODE_UP;
	if (-dX > abs(dY))
		return Common::KEYCODE_LEFT;

	return Common::KEYCODE_INVALID;
}

static GLES8888Texture *loadBuiltinTexture(const char *filename) {
	Common::ArchiveMemberPtr member = SearchMan.getMember(filename);
	Common::SeekableReadStream *str = member->createReadStream();
	Image::TGADecoder dec;
	dec.loadStream(*str);
	const void *pixels = dec.getSurface()->getPixels();

	GLES8888Texture *ret = new GLES8888Texture();
	uint16 w = dec.getSurface()->w;
	uint16 h = dec.getSurface()->h;
	uint16 pitch = dec.getSurface()->pitch;
	ret->allocBuffer(w, h);
	ret->updateBuffer(0, 0, w, h, pixels, pitch);

	delete str;
	return ret;
}

void TouchControls::init(int width, int height) {
	_arrows_texture = loadBuiltinTexture("arrows.tga");
	_screen_width = width;
	_screen_height = height;
}

const uint _numRightKeycodes = 4;
const Common::KeyCode _rightKeycodes[] = { Common::KEYCODE_i, Common::KEYCODE_p, Common::KEYCODE_u, Common::KEYCODE_e };

void TouchControls::draw() {
	if (_joystickPressing != Common::KEYCODE_INVALID) {
		Common::Rect clip = clipFor(_joystickPressing);
		_arrows_texture->drawTexture(2 * _screen_width / 10, _screen_height / 2, 64, 64, clip);
	}

	if (_centerPressing != Common::KEYCODE_INVALID) {
		Common::Rect clip = clipFor(_centerPressing);
		_arrows_texture->drawTexture(_screen_width / 2, _screen_height / 2, 64, 64, clip);
	}

	if (_rightPressing != Common::KEYCODE_INVALID) {
		Common::Rect clip = clipFor(_rightPressing);
		_arrows_texture->drawTexture( 8 * _screen_width / 10, _screen_height / 2, 64, 64, clip);
	}
}

void TouchControls::update(int ptr, int action, int x, int y) {
	if (ptr > kNumPointers)
		return;

	TouchArea touchArea = (TouchArea) getTouchArea(x, y);

	switch (action) {
	case JACTION_POINTER_DOWN:
	case JACTION_DOWN:
		if (touchArea > kTouchAreaNone && -1 == pointerFor(touchArea)) {
			pointerFor(touchArea) = ptr;
			_pointers[ptr].active = true;
			_pointers[ptr].function = touchArea;
			_pointers[ptr].startX = _pointers[ptr].currentX = x;
			_pointers[ptr].startY = _pointers[ptr].currentY = y;
			// fall through to move case to initialize _{joy,center,right}Pressing
		} else {
			return;
		}

	case JACTION_MULTIPLE: {
		_pointers[ptr].currentX = x;
		_pointers[ptr].currentY = y;
		int dX = x - _pointers[ptr].startX;
		int dY = y - _pointers[ptr].startY;

		switch (_pointers[ptr].function) {
		case kTouchAreaJoystick: {
			Common::KeyCode newPressing = determineKey(dX, dY);
			if (newPressing != _joystickPressing) {
				keyUp(_joystickPressing);
				keyDown(newPressing);
				_joystickPressing = newPressing;
			} else if(abs(dY) > 150) {
			   keyDown(Common::KEYCODE_LSHIFT);
			} else if(abs(dY) <= 150){
			   keyUp(Common::KEYCODE_LSHIFT);
			}
			return;
		}

		case kTouchAreaCenter:
			_centerPressing = determineKey(dX, dY, Common::KEYCODE_RETURN);
			return;

		case kTouchAreaRight:
			_rightPressing = determineKey(dX, dY, Common::KEYCODE_i);
			switch (_rightPressing) {
			case Common::KEYCODE_LEFT:
			case Common::KEYCODE_RIGHT:
				_rightPressing = _rightKeycodes[abs(dX / 100) % _numRightKeycodes];
				break;

			case Common::KEYCODE_UP:
				_rightPressing = Common::KEYCODE_PAGEUP;
				break;

			case Common::KEYCODE_DOWN:
				_rightPressing = Common::KEYCODE_PAGEDOWN;
				break;

			default:
				break;
			}

		default:
			return;
		}
		return;
	}

	case JACTION_UP:
	case JACTION_POINTER_UP: {
		switch (_pointers[ptr].function) {
		case kTouchAreaJoystick:
			pointerFor(kTouchAreaJoystick) = -1;
			if (_joystickPressing != Common::KEYCODE_INVALID) {
				keyUp(_joystickPressing);
				_joystickPressing = Common::KEYCODE_INVALID;
				keyUp(Common::KEYCODE_LSHIFT);
			}
			break;

		case kTouchAreaCenter:
			pointerFor(kTouchAreaCenter) = -1;
			keyPress(_centerPressing);
			_centerPressing = Common::KEYCODE_INVALID;
			break;

		case kTouchAreaRight:
			pointerFor(kTouchAreaRight) = -1;
			keyPress(_rightPressing);
			_rightPressing = Common::KEYCODE_INVALID;
			break;

		case kTouchAreaNone:
		default:
			break;
		}
		_pointers[ptr].active = false;
		_pointers[ptr].function = kTouchAreaNone;
		return;
	}
	}
}

int &TouchControls::pointerFor(TouchArea ta) {
	return _activePointers[ta - kTouchAreaNone];
}

void TouchControls::keyDown(Common::KeyCode kc) {
	Common::Event ev;
	ev.type = Common::EVENT_KEYDOWN;
	ev.kbd.keycode = kc;
	dynamic_cast<OSystem_Android *>(g_system)->pushEvent(ev);
}

void TouchControls::keyUp(Common::KeyCode kc) {
	Common::Event ev;
	ev.type = Common::EVENT_KEYUP;
	ev.kbd.keycode = kc;
	dynamic_cast<OSystem_Android *>(g_system)->pushEvent(ev);
}

void TouchControls::keyPress(Common::KeyCode kc) {
	Common::Event ev;
	ev.kbd.keycode = kc;
	dynamic_cast<OSystem_Android *>(g_system)->pushKeyPressEvent(ev);
}

#endif
