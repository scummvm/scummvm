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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "asylum/system/cursor.h"

#include "asylum/system/graphics.h"

#include "asylum/asylum.h"

#include "graphics/cursorman.h"

namespace Asylum {

const uint32 CURSOR_UPDATE_TICKS = 100;

Cursor::Cursor(AsylumEngine *engine) : _vm(engine),
	_state(0), _cursorRes(nullptr), _nextTick(0), _frameStep(0),
	_graphicResourceId(kResourceNone), _currentFrame(0), _lastFrameIndex(0), _counter(0), _animation(kCursorAnimationNone),
	_forceHide(false) {
}

Cursor::~Cursor() {
	delete _cursorRes;
}

void Cursor::hide() const {
	CursorMan.showMouse(false);
}

void Cursor::show() const {
	if (!_forceHide)
		CursorMan.showMouse(true);
}

void Cursor::set(ResourceId resourceId, int32 cnt, CursorAnimation anim, int32 frames) {
	bool wasHidden = isHidden();

	hide();

	delete _cursorRes;
	_cursorRes = new GraphicResource(_vm, resourceId);

	// Set next update
	_nextTick = _vm->getTick() + CURSOR_UPDATE_TICKS;

	// Get frame count
	if (frames >= 0)
		_lastFrameIndex = (uint32)frames;
	else
		_lastFrameIndex = _cursorRes->count() - 1;

	this->_graphicResourceId = resourceId;
	this->_animation = anim;
	this->_counter = cnt;
	_currentFrame = 0;
	_frameStep = 1;

	// Do not animate if no frames (and the other way around)
	if (_lastFrameIndex == 0 || anim == kCursorAnimationNone) {
		_lastFrameIndex = 0;
		_animation = kCursorAnimationNone;
	}

	if (_lastFrameIndex >= _cursorRes->count())
		_lastFrameIndex = _cursorRes->count() - 1;

	update();

	if (!wasHidden)
		show();
}

void Cursor::update() {
	if (!_cursorRes)
		error("[Cursor::update] Cursor resources not initialized properly!");

	Common::Point hotspot = getHotspot(_currentFrame);

	GraphicFrame *frame = _cursorRes->getFrame(_currentFrame);
	CursorMan.replaceCursor((byte *)frame->surface.getPixels(),
	                        frame->surface.w,
	                        frame->surface.h,
	                        hotspot.x,
	                        hotspot.y,
	                        0);
}

void Cursor::setState(const Common::Event &evt) {
	switch (evt.type) {
	default:
		break;

	case Common::EVENT_LBUTTONDOWN:
		_state |= kCursorStateLeft;
		break;

	case Common::EVENT_RBUTTONDOWN:
		_state |= kCursorStateRight;
		break;

	case Common::EVENT_MBUTTONDOWN:
		_state |= kCursorMiddle;
		break;

	case Common::EVENT_LBUTTONUP:
		_state &= ~kCursorStateLeft;
		break;

	case Common::EVENT_RBUTTONUP:
		_state &= ~kCursorStateRight;
		break;

	case Common::EVENT_MBUTTONUP:
		_state &= ~kCursorMiddle;
		break;
	}
}

void Cursor::animate() {
	if (isHidden() || !_animation || _nextTick > _vm->getTick())
		return;

	bool notifyHandler = false;
	int32 frame = 0;

	if (_animation == kCursorAnimationLinear) {
		if (_currentFrame == _lastFrameIndex) {
			_currentFrame = frame = 0;
		} else {
			_currentFrame += _frameStep;
			frame = _currentFrame;
		}
	} else if (_animation == kCursorAnimationMirror) {
		_currentFrame += _frameStep;
		frame = _currentFrame;

		if (_currentFrame == 0 || _currentFrame == _lastFrameIndex)
			_frameStep = -_frameStep;
	}

	if (frame == 0) {
		if (_counter != -1) {
			--_counter;

			if (!_counter) {
				_animation = kCursorAnimationNone;
				notifyHandler = true;
			}
		}
	}

	_nextTick = _vm->getTick() + CURSOR_UPDATE_TICKS;

	if (notifyHandler)
		_vm->notify(EVENT_ASYLUM_CURSOR);

	update();
}

Common::Point Cursor::getHotspot(uint32 frameIndex) {
	if (!_cursorRes)
		error("[Cursor::getHotspot] Cursor resource not initialized properly");

	Common::Point point;
	uint32 resFlags = _cursorRes->getData().flags;

	if (BYTE1(resFlags) & 0x10) {
		// XXX removed a check for frameIndex >= 0 as it will always
		// evaluate to true since frameIndex is unsigned
		if (frameIndex > _cursorRes->count()) {
			GraphicFrame *frame = _cursorRes->getFrame(_currentFrame);

			point.x = frame->x;
			point.y = frame->y;
		}
	}

	return point;
}

bool Cursor::isHidden() const {
	return !CursorMan.isVisible();
}

const Common::Point Cursor::position() const {
	return g_system->getEventManager()->getMousePos();
}

} // end of namespace Asylum
