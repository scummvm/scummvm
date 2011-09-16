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

#include "common/macresman.h"
#include "common/stream.h"

#include "pegasus/elements.h"
#include "pegasus/graphics.h"

namespace Pegasus {

DisplayElement::DisplayElement(const tDisplayElementID id) : IDObject(id) {
	_elementIsDisplaying = false;
	_elementIsVisible = false;
	_elementOrder = 0;
	_triggeredElement = this;
	_nextElement = 0;
}

DisplayElement::~DisplayElement() {
	if (isDisplaying())
		((PegasusEngine *)g_engine)->_gfx->removeDisplayElement(this);
}

void DisplayElement::setDisplayOrder(const tDisplayOrder order) {
	if (_elementOrder != order) {
		_elementOrder = order;
		if (isDisplaying()) {
			((PegasusEngine *)g_engine)->_gfx->removeDisplayElement(this);
			((PegasusEngine *)g_engine)->_gfx->addDisplayElement(this);
			triggerRedraw();
		}
	}
}

void DisplayElement::startDisplaying() {
	if (!isDisplaying()) {
		((PegasusEngine *)g_engine)->_gfx->addDisplayElement(this);
		triggerRedraw();
	}
}

void DisplayElement::stopDisplaying() {
	if (isDisplaying()) {
		triggerRedraw();
		((PegasusEngine *)g_engine)->_gfx->removeDisplayElement(this);
	}
}

void DisplayElement::setBounds(const tCoordType left, const tCoordType top, const tCoordType right, const tCoordType bottom) {
	_bounds = Common::Rect(left, top, right, bottom);
}

void DisplayElement::getBounds(Common::Rect &r) const {
	r = _bounds;
}

void DisplayElement::sizeElement(const tCoordType h, const tCoordType v) {
	_bounds.right = _bounds.left + h;
	_bounds.bottom = _bounds.top + v;
}

void DisplayElement::moveElementTo(const tCoordType h, const tCoordType v) {
	_bounds.moveTo(h, v);
}

void DisplayElement::moveElement(const tCoordType dh, const tCoordType dv) {
	_bounds.translate(dh, dv);
}

void DisplayElement::getLocation(tCoordType &h, tCoordType &v) const {
	h = _bounds.left;
	v = _bounds.top;
}

void DisplayElement::centerElementAt(const tCoordType h, const tCoordType v) {
	_bounds.moveTo(h - (_bounds.width() / 2), v - (_bounds.height() / 2));
}

void DisplayElement::getCenter(tCoordType &h, tCoordType &v) const {
	h = (_bounds.left + _bounds.right) / 2;
	v = (_bounds.top + _bounds.bottom) / 2;
}

void DisplayElement::setBounds(const Common::Rect &r) {
	if (r != _bounds) {
		triggerRedraw();
		_bounds = r;
		triggerRedraw();
	}
}

void DisplayElement::hide() {
	if (_elementIsVisible) {
		triggerRedraw();
		_elementIsVisible = false;
	}
}

void DisplayElement::show() {
	if (!_elementIsVisible) {
		_elementIsVisible = true;
		triggerRedraw();
	}
}

//	Only invalidates this element's bounding rectangle if all these conditions are true:
//	--	The triggered element is this element.
//	--	The element is displaying on the display list.
//	--	The element is visible.
//	--	The element is part of the active layer OR is one of the reserved items.
void DisplayElement::triggerRedraw() {
	GraphicsManager *gfx = ((PegasusEngine *)g_engine)->_gfx;

	if (_triggeredElement == this) {
		if (validToDraw(gfx->getBackOfActiveLayer(), gfx->getFrontOfActiveLayer()))
			gfx->invalRect(_bounds);
	} else {
		_triggeredElement->triggerRedraw();
	}
}

void DisplayElement::setTriggeredElement(DisplayElement *element) {
	if (element)
		_triggeredElement = element;
	else
		_triggeredElement = this;
}

bool DisplayElement::validToDraw(tDisplayOrder backLayer, tDisplayOrder frontLayer) {
	return	isDisplaying() && _elementIsVisible &&
			(getObjectID() <= kHighestReservedElementID ||
			(getDisplayOrder() >= backLayer &&
			getDisplayOrder() <= frontLayer));
}

DropHighlight::DropHighlight(const tDisplayElementID id) : DisplayElement(id) {
	_highlightColor = g_system->getScreenFormat().RGBToColor(0x48, 0x80, 0xD8);
	_thickness = 2;
	_cornerDiameter = 0;
}

void DropHighlight::draw(const Common::Rect &) {
	Graphics::Surface *screen = ((PegasusEngine *)g_engine)->_gfx->getWorkArea();

	// Since this is only used in two different ways, I'm only
	// going to implement it in those two ways. Deal with it.

	Common::Rect rect = _bounds;
	screen->frameRect(rect, _highlightColor);
	rect.grow(1);
	screen->frameRect(rect, _highlightColor);

	if (_cornerDiameter == 8 && _thickness == 4) {
		rect.grow(1);
		screen->frameRect(rect, _highlightColor);
		screen->hLine(rect.left + 1, rect.top - 1, rect.right - 2, _highlightColor);
		screen->hLine(rect.left + 1, rect.bottom, rect.right - 2, _highlightColor);
		screen->vLine(rect.left - 1, rect.top + 1, rect.bottom - 2, _highlightColor);
		screen->vLine(rect.right, rect.top + 1, rect.bottom - 2, _highlightColor);
	}
}

EnergyBar::EnergyBar(const tDisplayElementID id) : DisplayElement(id) {
	_maxEnergy = 0;
	_energyLevel = 0;
	_barColor = 0;
}

void EnergyBar::checkRedraw() {	
	if (_elementIsVisible) {
		Common::Rect r;
		calcLevelRect(r);
		if (r != _levelRect)
			triggerRedraw();
	}
}

void EnergyBar::setEnergyLevel(const uint32 newLevel) {
	if (_energyLevel != newLevel) {
		_energyLevel = newLevel;
		checkRedraw();
	}
}

void EnergyBar::setMaxEnergy(const uint32 newMax) {
	if (_maxEnergy != newMax) {
		if (_energyLevel > newMax)
			_energyLevel = newMax;
		_maxEnergy = newMax;
		checkRedraw();
	}
}

void EnergyBar::setBounds(const Common::Rect &r) {
	DisplayElement::setBounds(r);
	checkRedraw();
}

void EnergyBar::draw(const Common::Rect &r) {
	Common::Rect levelRect;
	calcLevelRect(levelRect);

	if (r.intersects(levelRect))
		((PegasusEngine *)g_engine)->_gfx->getWorkArea()->fillRect(levelRect, _barColor);
}

void EnergyBar::calcLevelRect(Common::Rect &r) const {
	if (_maxEnergy > 0) {
		r = _bounds;
		r.left = r.right - _bounds.width() * _energyLevel / _maxEnergy;
	} else {
		r = Common::Rect(0, 0, 0, 0);
	}
}

IdlerAnimation::IdlerAnimation(const tDisplayElementID id) : Animation(id) {
	_lastTime = 0xffffffff;
}

void IdlerAnimation::startDisplaying() {
	if (!isDisplaying()) {
		Animation::startDisplaying();
		startIdling();
	}
}

void IdlerAnimation::stopDisplaying() {
	if (isDisplaying()) {
		Animation::stopDisplaying();
		stopIdling();
	}
}

void IdlerAnimation::useIdleTime() {
	uint32 currentTime = getTime();

	if (currentTime != _lastTime) {
		_lastTime = currentTime;
		timeChanged(_lastTime);
	}
}

void IdlerAnimation::timeChanged(const TimeValue) {
	triggerRedraw();
}

FrameSequence::FrameSequence(const tDisplayElementID id) : IdlerAnimation(id) {
	_duration = 0;
	_currentFrameNum = 0;
	_resFork = new Common::MacResManager();
}

FrameSequence::~FrameSequence() {
	delete _resFork;
}

void FrameSequence::useFileName(const Common::String &fileName) {
	_resFork->open(fileName);
}

void FrameSequence::openFrameSequence() {
	if (!_resFork->hasResFork())
		return;

	Common::SeekableReadStream *res = _resFork->getResource(MKTAG('P', 'F', 'r', 'm'), 0x80);

	if (!res)
		return;

	uint32 scale = res->readUint32BE();
	_bounds.top = res->readUint16BE();
	_bounds.left = res->readUint16BE();
	_bounds.bottom = res->readUint16BE();
	_bounds.right = res->readUint16BE();
	_numFrames = res->readUint16BE();
	_duration = 0;

	_frameTimes.clear();
	for (uint32 i = 0; i < _numFrames; i++) {
		TimeValue time = res->readUint32BE();
		_duration += time;
		_frameTimes.push_back(_duration);
	}

	setScale(scale);
	setSegment(0, _duration);
	setTime(0);
	_currentFrameNum = 0;
	newFrame(_currentFrameNum);
	triggerRedraw();

	delete res;
}

void FrameSequence::closeFrameSequence() {
	stop();
	_resFork->close();
	_duration = 0;
	_numFrames = 0;
	_frameTimes.clear();
}

void FrameSequence::timeChanged(const TimeValue time) {
	int16 frameNum = 0;
	for (int16 i = _numFrames - 1; i >= 0; i--) {
		if (_frameTimes[i] < time) {
			frameNum = i;
			break;
		}
	}

	if (frameNum != _currentFrameNum) {
		_currentFrameNum = frameNum;
		newFrame(_currentFrameNum);
		triggerRedraw();
	}
}

void FrameSequence::setFrameNum(const int16 frameNum) {
	int16 f = CLIP<int>(frameNum, 0, _numFrames);

	if (_currentFrameNum != f) {
		_currentFrameNum = f;
		setTime(_frameTimes[f]);
		newFrame(f);
		triggerRedraw();
	}
}

bool FrameSequence::isSequenceOpen() const {
	return _numFrames != 0;
}

} // End of namespace Pegasus
