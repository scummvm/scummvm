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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#if defined(__ANDROID__)

#include "common/fs.h"
#include "common/stream.h"
#include "common/archive.h"
#include "graphics/decoders/tga.h"

#include "backends/platform/android/events.h"
#include "backends/platform/android/texture.h"
#include "backends/platform/android/touchcontrols.h"

struct CardinalSwipe {
	CardinalSwipe(int dX, int dY);
	uint16 distance;
	enum Direction {
		kDirectionLeft,
		kDirectionUp,
		kDirectionRight,
		kDirectionDown
	} direction;
};

CardinalSwipe::CardinalSwipe(int dX, int dY) {
	if (abs(dX) > abs(dY)) {
		if (dX > 0) {
			distance = dX;
			direction = kDirectionRight;
		} else {
			distance = abs(dX);
			direction = kDirectionLeft;
		}
	} else {
		if (dY > 0) {
			distance = dY;
			direction = kDirectionDown;
		} else {
			distance = abs(dY);
			direction = kDirectionUp;
		}
	}
}

static Common::Rect clipFor(const CardinalSwipe &cs) {
	switch (cs.direction) {
	case CardinalSwipe::kDirectionLeft:
		return Common::Rect(0, 128, 128, 256);
	case CardinalSwipe::kDirectionUp:
		return Common::Rect(0, 0, 128, 128);
	case CardinalSwipe::kDirectionRight:
		return Common::Rect(128, 128, 256, 256);
	case CardinalSwipe::kDirectionDown:
		return Common::Rect(128, 0, 256, 128);
	default: // unreachable
		return Common::Rect(0, 0, 1, 1);
	}
}

TouchControls::TouchControls() :
	_arrows_texture(NULL),
	_joystickPressing(Common::KEYCODE_INVALID),
	_key_receiver(NULL),
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

static Common::KeyCode determineKey(int dX, int dY) {
	if (dX * dX + dY * dY < 50 * 50)
		return Common::KEYCODE_INVALID;

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
	Graphics::TGADecoder dec;
	dec.loadStream(*str);
	void *pixels = dec.getSurface()->pixels;

	GLES8888Texture *ret = new GLES8888Texture();
	uint16 w = dec.getSurface()->w;
	uint16 h = dec.getSurface()->h;
	uint16 pitch = dec.getSurface()->pitch;
	ret->allocBuffer(w, h);
	ret->updateBuffer(0, 0, w, h, pixels, pitch);

	delete str;
	return ret;
}

void TouchControls::init(KeyReceiver *kr, int width, int height) {
	_arrows_texture = loadBuiltinTexture("arrows.tga");
	_screen_width = width;
	_screen_height = height;
	_key_receiver = kr;
}

void TouchControls::draw() {
	int joyPtr = pointerFor(kTouchAreaJoystick);
	if (joyPtr != -1) {
		Pointer &joy = _pointers[joyPtr];
		CardinalSwipe cs(joy.currentX - joy.startX, joy.currentY - joy.startY);

		if (cs.distance >= 50) {
			Common::Rect clip = clipFor(cs);
			_arrows_texture->drawTexture(2 * _screen_width / 10, _screen_height / 2, 64, 64, clip);
		}
	}

	int centerPtr = pointerFor(kTouchAreaCenter);
	if (centerPtr != -1) {
		Pointer &center = _pointers[centerPtr];
		CardinalSwipe cs(center.currentX - center.startX, center.currentY - center.startY);

		if (cs.distance >= 100) {
			Common::Rect clip = clipFor(cs);
			_arrows_texture->drawTexture(_screen_width / 2, _screen_height / 2, 64, 64, clip);
		}
	}

	int rightPtr = pointerFor(kTouchAreaRight);
	if (rightPtr != -1) {
		Pointer &right = _pointers[rightPtr];
		CardinalSwipe cs(right.currentX - right.startX, right.currentY - right.startY);

		if (cs.distance >= 100) {
			if (   cs.direction == CardinalSwipe::kDirectionDown
			    || cs.direction == CardinalSwipe::kDirectionUp) {
				Common::Rect clip = clipFor(cs);
				_arrows_texture->drawTexture( 8 * _screen_width / 10, _screen_height / 2, 64, 64, clip);
			}
		}
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
		}
		return;

	case JACTION_MOVE:
		_pointers[ptr].currentX = x;
		_pointers[ptr].currentY = y;
		if (_pointers[ptr].function == kTouchAreaJoystick) {
			int dX = x - _pointers[ptr].startX;
			int dY = y - _pointers[ptr].startY;
			Common::KeyCode newPressing = determineKey(dX, dY);
			if (newPressing != _joystickPressing) {
				_key_receiver->keyPress(_joystickPressing, KeyReceiver::UP);
				_key_receiver->keyPress(newPressing, KeyReceiver::DOWN);
				_joystickPressing = newPressing;
			}
		}
		return;

	case JACTION_UP:
	case JACTION_POINTER_UP: {
		int dX = _pointers[ptr].currentX - _pointers[ptr].startX;
		int dY = _pointers[ptr].currentY - _pointers[ptr].startY;
		struct CardinalSwipe cs(dX, dY);

		switch (_pointers[ptr].function) {
		case kTouchAreaCenter: {
			pointerFor(kTouchAreaCenter) = -1;
			if (cs.distance < 100) {
				_key_receiver->keyPress(Common::KEYCODE_RETURN);
				break;
			}
			switch (cs.direction) {
			case CardinalSwipe::kDirectionLeft:
				_key_receiver->keyPress(Common::KEYCODE_LEFT);
				break;
			case CardinalSwipe::kDirectionUp:
				_key_receiver->keyPress(Common::KEYCODE_UP);
				break;
			case CardinalSwipe::kDirectionRight:
				_key_receiver->keyPress(Common::KEYCODE_RIGHT);
				break;
			case CardinalSwipe::kDirectionDown:
				_key_receiver->keyPress(Common::KEYCODE_DOWN);
				break;
			}
			break;
		}

		case kTouchAreaJoystick:
			pointerFor(kTouchAreaJoystick) = -1;
			if (_joystickPressing != Common::KEYCODE_INVALID) {
				_key_receiver->keyPress(_joystickPressing, KeyReceiver::UP);
			}
			break;

		case kTouchAreaRight:
			pointerFor(kTouchAreaRight) = -1;
			if (   cs.direction == CardinalSwipe::kDirectionLeft
			    || cs.direction == CardinalSwipe::kDirectionRight
			    || cs.distance < 100) {
				_key_receiver->keyPress(Common::KEYCODE_i);
			} else {
				if (cs.direction == CardinalSwipe::kDirectionUp) {
					_key_receiver->keyPress(Common::KEYCODE_PAGEUP);
				} else {
					_key_receiver->keyPress(Common::KEYCODE_PAGEDOWN);
				}
			}
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

#endif
