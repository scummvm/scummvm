/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#include "asylum/system/cursor.h"

#include "asylum/system/graphics.h"

#include "asylum/asylum.h"

#include "common/events.h"
#include "common/system.h"
#include "graphics/cursorman.h"

namespace Asylum {

const uint32 CURSOR_UPDATE_TICKS = 100;

Cursor::Cursor(AsylumEngine *engine) : _vm(engine),
	graphicResourceId(kResourceNone), currentFrame(0), lastFrameIndex(0), counter(0), animation(kCursorAnimationNone),
	_cursorRes(NULL), _nextTick(0), _frameStep(0), _state(0) {

}

Cursor::~Cursor() {
	delete _cursorRes;

	// Zero-out passed pointers
	_vm = NULL;
}

void Cursor::hide() const {
	CursorMan.showMouse(false);
}

void Cursor::show() const {
	CursorMan.showMouse(true);
}

void Cursor::set(ResourceId resourceId, int32 counter, CursorAnimation animation, int32 frames) {
	hide();

	delete _cursorRes;
	_cursorRes = new GraphicResource(_vm, resourceId);

	// Set next update
	_nextTick = _vm->getTick() + 100;

	// Get frame count
	if (frames >= 0)
		lastFrameIndex = frames;
	else
		lastFrameIndex = _cursorRes->count() - 1;

	this->animation = animation;
	this->counter = counter;
	currentFrame = 0;
	_frameStep = 1;

	// Do not animate if no frames (and the other way around)
	if (lastFrameIndex == 0 || animation == kCursorAnimationNone) {
		lastFrameIndex = 0;
		animation = kCursorAnimationNone;
	}

	if (lastFrameIndex >= _cursorRes->count())
		lastFrameIndex = _cursorRes->count() - 1;

	update();

	show();
}

void Cursor::update() {
	if (!_cursorRes)
		error("[Cursor::update] Cursor resources not initialized properly!");

	Common::Point hotspot = getHotspot(currentFrame);

	GraphicFrame *frame = _cursorRes->getFrame(currentFrame);
	CursorMan.replaceCursor((byte *)frame->surface.pixels,
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
	if (isHidden() || !animation || _nextTick > _vm->getTick())
		return;

	bool notifyHandler = false;
	int32 frame = 0;

	if (animation == kCursorAnimationLinear) {
		if (currentFrame == lastFrameIndex) {
			currentFrame = frame = 0;
		} else {
			currentFrame += _frameStep;
			frame = currentFrame;
		}
	} else if (animation == kCursorAnimationMirror) {
		currentFrame += _frameStep;
		frame = currentFrame;

		if (currentFrame == 0 || currentFrame == lastFrameIndex)
			_frameStep = -_frameStep;
	}

	if (frame == 0) {
		if (counter != -1) {
			--counter;

			if (!counter) {
				animation = kCursorAnimationNone;
				notifyHandler = true;
			}
		}
	}

	_nextTick = _vm->getTick() + 100;

	if (notifyHandler)
		_vm->notify(EVENT_ASYLUM_CURSOR);

	update();
}

Common::Point Cursor::getHotspot(uint32 frameIndex) {
	Common::Point point;

	uint32 resFlags = _cursorRes->getData().flags;

	if (BYTE1(resFlags) & 0x10) {
		if (frameIndex >= 0 && frameIndex > _cursorRes->count()) {
			GraphicFrame *frame = _cursorRes->getFrame(currentFrame);

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
