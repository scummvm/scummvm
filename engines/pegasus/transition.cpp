/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1995-1997 Presto Studios, Inc.
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
 */

#include "common/system.h"

#include "pegasus/transition.h"

namespace Pegasus {

ScreenFader::ScreenFader() {
	_fadeTarget = getBlack();
	// Initially, assume screens are on at full brightness.
	Fader::setFaderValue(100);
}

void ScreenFader::doFadeOutSync(const TimeValue duration, const TimeValue scale, const uint32 fadeTarget) {
	_fadeTarget = fadeTarget;

	FaderMoveSpec spec;
	spec.makeTwoKnotFaderSpec(scale, 0, getFaderValue(), duration, 0);
	startFaderSync(spec);
}

void ScreenFader::doFadeInSync(const TimeValue duration, const TimeValue scale, const uint32 fadeTarget) {
	_fadeTarget = fadeTarget;

	FaderMoveSpec spec;
	spec.makeTwoKnotFaderSpec(scale, 0, getFaderValue(), duration, 100);
	startFaderSync(spec);
}

void ScreenFader::setFaderValue(const int32 value) {
	if (value != getFaderValue()) {
		Fader::setFaderValue(value);

		// TODO: Gamma fading
	}
}

uint32 ScreenFader::getBlack() {
	return g_system->getScreenFormat().RGBToColor(0, 0, 0);
}

Transition::Transition(const DisplayElementID id) : FaderAnimation(id) {
	_outPicture = 0;
	_inPicture = 0;
}

void Transition::setBounds(const Common::Rect &r) {
	FaderAnimation::setBounds(r);
	_boundsWidth = _bounds.width();
	_boundsHeight = _bounds.height();
}

void Transition::setInAndOutElements(DisplayElement *inElement, DisplayElement *outElement) {
	_inPicture = inElement;
	_outPicture = outElement;

	Common::Rect r;

	if (_outPicture)
		_outPicture->getBounds(r);
	else if (_inPicture)
		_inPicture->getBounds(r);

	setBounds(r);
}

void Slide::draw(const Common::Rect &r) {
	Common::Rect oldBounds, newBounds;
	
	adjustSlideRects(oldBounds, newBounds);
	drawElements(r, oldBounds, newBounds);
}

void Slide::adjustSlideRects(Common::Rect &oldBounds, Common::Rect &newBounds) {
	oldBounds = _bounds;
	newBounds = _bounds;
}

void Slide::drawElements(const Common::Rect &drawRect, const Common::Rect &oldBounds, const Common::Rect &newBounds) {
	drawSlideElement(drawRect, oldBounds, _outPicture);
	drawSlideElement(drawRect, newBounds, _inPicture);
}

void Slide::drawSlideElement(const Common::Rect &drawRect, const Common::Rect &oldBounds, DisplayElement *picture) {
	if (picture && drawRect.intersects(oldBounds)) {
		picture->moveElementTo(oldBounds.left, oldBounds.top);
		picture->draw(drawRect.findIntersectingRect(oldBounds));
	}
}

void Push::adjustSlideRects(Common::Rect &oldBounds, Common::Rect &newBounds) {
	switch (_direction & kSlideHorizMask) {
		case kSlideLeftMask:
			newBounds.left = oldBounds.right = _bounds.right - pegasusRound(getFaderValue() * _boundsWidth, kTransitionRange);
			newBounds.right = newBounds.left + _boundsWidth;
			oldBounds.left = oldBounds.right - _boundsWidth;
			break;
		case kSlideRightMask:
			oldBounds.left = newBounds.right = _bounds.left + pegasusRound(getFaderValue() * _boundsWidth, kTransitionRange);
			oldBounds.right = oldBounds.left + _boundsWidth;
			newBounds.left = newBounds.right - _boundsWidth;
			break;
		default:
			newBounds.left = oldBounds.left = _bounds.left;
			newBounds.right = oldBounds.right = _bounds.right;
			break;
	}
	switch (_direction & kSlideVertMask) {
		case kSlideDownMask:
			oldBounds.top = newBounds.bottom = _bounds.top + pegasusRound(getFaderValue() * _boundsHeight, kTransitionRange);
			oldBounds.bottom = oldBounds.top + _boundsHeight;
			newBounds.top = newBounds.bottom - _boundsHeight;
			break;
		case kSlideUpMask:
			newBounds.top = oldBounds.bottom = _bounds.bottom - pegasusRound(getFaderValue() * _boundsHeight, kTransitionRange);
			newBounds.bottom = newBounds.top + _boundsHeight;
			oldBounds.top = oldBounds.bottom - _boundsHeight;
			break;
		default:
			newBounds.top = oldBounds.top = _bounds.top;
			newBounds.bottom = oldBounds.bottom = _bounds.bottom;
			break;
	}
}

} // End of namespace Pegasus
