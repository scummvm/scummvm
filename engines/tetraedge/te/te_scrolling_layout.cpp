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

#include "tetraedge/te/te_scrolling_layout.h"

#include "tetraedge/tetraedge.h"
#include "tetraedge/te/te_input_mgr.h"

#include "common/math.h"

namespace Tetraedge {

TeScrollingLayout::TeScrollingLayout() : _contentLayout(nullptr),
	_enclose(true), _mouseControl(true), _autoScrollLoop(-1), _autoScrollDelay(1500),
	_autoScrollAnimation1Enabled(true), _autoScrollAnimation2Enabled(true),
	_autoScrollAnimation1Speed(0.1f), _autoScrollAnimation2Speed(0.1f),
	_autoScrollAnimation1Delay(1000), _autoScrollAnimation2Delay(1000),
	_currentScrollLoopNo(0), _inertiaAnimationDuration(500),
	_mouseMoveThreshold(30.0), _insideMouseThreshold(true),
	_direction(0.0, 1.0, 0.0)
{
	setSizeType(RELATIVE_TO_PARENT);
	setSize(TeVector3f32(1.0, 1.0, 1.0));
	// TODO: original does addChild on something here.
	// FIXME: This doesn't seem right...
	//onWorldTransformationMatrixChanged().add(this, &TeScrollingLayout::onSlideButtonDown);
	Common::Array<float> curve;
	curve.push_back(0.0f);
	curve.push_back(1.0f);
	_autoScrollAnimation1Curve = curve;
	_autoScrollAnimation2Curve = curve;
	_autoScrollDelayTimer.alarmSignal().add(this, &TeScrollingLayout::onAutoScrollDelayTimer);
	_autoScrollAnimation1Timer.alarmSignal().add(this, &TeScrollingLayout::onAutoScrollAnimation1DelayTimer);
	_autoScrollAnimation2Timer.alarmSignal().add(this, &TeScrollingLayout::onAutoScrollAnimation2DelayTimer);
	_autoScrollAnimation1.onFinished().add(this, &TeScrollingLayout::onAutoScrollAnimation1Finished);
	_autoScrollAnimation2.onFinished().add(this, &TeScrollingLayout::onAutoScrollAnimation2Finished);
	Common::Array<float> curve2;
	curve2.push_back(0.0f);
	curve2.push_back(0.35f);
	curve2.push_back(0.68f);
	curve2.push_back(0.85f);
	curve2.push_back(0.93f);
	curve2.push_back(0.97f);
	curve2.push_back(1.0f);
	_inertiaAnimationCurve = curve2;
	_scrollTimer.start();
	playAutoScroll();
}

TeScrollingLayout::~TeScrollingLayout() {
	TeInputMgr *inputmgr = g_engine->getInputMgr();
	inputmgr->_mouseMoveSignal.remove<TeScrollingLayout>(this, &TeScrollingLayout::onMouseMove);
	inputmgr->_mouseLUpSignal.remove<TeScrollingLayout>(this, &TeScrollingLayout::onMouseLeftUp);
}

void TeScrollingLayout::setContentLayout(TeLayout *layout) {
	if (_contentLayout) {
		removeChild(_contentLayout);
	}
	_contentLayout = layout;
	if (layout) {
		_contentLayoutUserPos = layout->userPosition();
		// TODO: original seems to call addChildBefore(layout, this) which doesn't make sense?
		addChild(_contentLayout);
	}
}

void TeScrollingLayout::setSpeed(const TeVector3f32 &speed) {
	_speed = speed;
	TeVector3f32 newpos = scrollPosition() + _speed * (float)(_scrollTimer.timeElapsed() / 1000000.0);
	setScrollPosition(newpos);
}

void TeScrollingLayout::setScrollPosition(const TeVector3f32 &scrPos) {
	if (!_contentLayout)
		return;

	TeVector3f32 pos = scrPos;
	pos.x() = CLIP(pos.x(), 0.0f, 1.0f);
	pos.y() = CLIP(pos.y(), 0.0f, 1.0f);

	const TeVector3f32 thisSize(xSize(), ySize(), 1.0);
	const TeVector3f32 contentSize(_contentLayout->xSize(), _contentLayout->ySize(), 1.0);
	TeVector3f32 sizeRatio;

	if (thisSize.x() == 0.0 || thisSize.y() == 0.0) {
		sizeRatio = TeVector3f32(1.0, 1.0, 1.0);
	} else {
		sizeRatio = contentSize / thisSize;
	}

	TeVector3f32 posToSet = _contentLayout->userPosition();
	const TeVector3f32 contentAnchor = _contentLayout->anchor();
	if (!_enclose) {
		if (thisSize.x() < contentSize.x()) {
			float offset = (sizeRatio.x() + 1.0) * pos.x();
			posToSet.x() = contentAnchor.x() * sizeRatio.x() + (1.0 - offset);
		}
		if (thisSize.y() < contentSize.y()) {
			float offset = (sizeRatio.y() + 1.0) * pos.y();
			posToSet.y() = contentAnchor.y() * sizeRatio.y() + (1.0 - offset);
		}
	} else {
		if (thisSize.x() < contentSize.x()) {
			float offset = (sizeRatio.x() - 1.0) * pos.x();
			posToSet.x() = contentAnchor.x() * sizeRatio.x() - offset;
		}
		if (thisSize.y() < contentSize.y()) {
			float offset = (sizeRatio.y() - 1.0) * pos.y();
			posToSet.y() = contentAnchor.y() * sizeRatio.y() - offset;
		}
	}

	_contentLayout->setPosition(posToSet);
	_posUpdatedSignal.call();
}

TeVector3f32 TeScrollingLayout::scrollPosition() {
	if (!_contentLayout)
		return TeVector3f32();

	const TeVector3f32 thisSize(xSize(), ySize(), 1.0);
	const TeVector3f32 contentSize(_contentLayout->xSize(), _contentLayout->ySize(), 1.0);

	TeVector3f32 sizeRatio;
	if (thisSize.x() == 0.0 || thisSize.y() == 0.0) {
		sizeRatio = TeVector3f32(1.0, 1.0, 1.0);
	} else {
		sizeRatio = contentSize / thisSize;
	}

	TeVector3f32 result(0.0, 0.0, 0.0);
	if (_enclose) {
		TeVector3f32 contentAnchor = _contentLayout->anchor();
		TeVector3f32 contentPos = _contentLayout->userPosition();
		if (sizeRatio.x() > 1.0) {
			result.x() = (-(int)(contentPos.x() - contentAnchor.x() * sizeRatio.x())) / (sizeRatio.x() - 1.0);
		}
		if (sizeRatio.y() > 1.0) {
			result.y() = (-(int)(contentPos.y() - contentAnchor.y() * sizeRatio.y())) / (sizeRatio.y() - 1.0);
		}
	} else {
		TeVector3f32 offsetPos = _contentLayout->position() - TeVector3f32(1.0, 1.0, 1.0);
		result = ((_contentLayout->anchor() * sizeRatio) - offsetPos) / (sizeRatio + TeVector3f32(1.0, 1.0, 1.0));
	}
	return result;
}

bool TeScrollingLayout::onAutoScrollDelayTimer() {
	_autoScrollDelayTimer.stop();
	playAutoScrollAnimation1();
	return false;
}

bool TeScrollingLayout::onAutoScrollAnimation1DelayTimer() {
	_autoScrollAnimation1Timer.stop();
	_autoScrollAnimation1.setCurve(_autoScrollAnimation1Curve);
	const TeVector3f32 startPos = scrollPosition();
	_autoScrollAnimation1._startVal = startPos;
	TeVector3f32 endPos = startPos + TeVector3f32(1.0, 1.0, 0.0) * _direction;
	endPos.x() = CLIP(endPos.x(), 0.0f, 1.0f);
	endPos.y() = CLIP(endPos.y(), 0.0f, 1.0f);
	_autoScrollAnimation1._endVal = endPos;

	TeVector3f32 sizeRatio(1.0, 1.0, 0.0);
	if (_contentLayout) {
		sizeRatio = _contentLayout->userSize() / size();
	}

	float duration = 0.0;
	if (_autoScrollAnimation1Speed != 0.0) {
		const TeVector3f32 dist = endPos - startPos;
		if (_enclose) {
			sizeRatio = dist * (sizeRatio - TeVector3f32(1.0, 1.0, 0.0));
		} else {
			sizeRatio = dist * (sizeRatio + TeVector3f32(1.0, 1.0, 0.0));
		}
		duration = (sizeRatio * _direction).length() / (_autoScrollAnimation1Speed / 1000.0);
	}

	_autoScrollAnimation1._duration = duration;
	_autoScrollAnimation1._callbackObj = this;
	_autoScrollAnimation1._callbackMethod = &TeScrollingLayout::setScrollPosition;
	_autoScrollAnimation1.play();
	return false;
}

bool TeScrollingLayout::onAutoScrollAnimation2DelayTimer() {
	_autoScrollAnimation2Timer.stop();
	_autoScrollAnimation2.setCurve(_autoScrollAnimation2Curve);
	const TeVector3f32 startPos = scrollPosition();
	_autoScrollAnimation2._startVal = startPos;
	// Note: this is the only real difference between this and the "1" version
	// of the function.. - instead of +
	TeVector3f32 endPos = startPos - TeVector3f32(1.0, 1.0, 0.0) * _direction;
	endPos.x() = CLIP(endPos.x(), 0.0f, 1.0f);
	endPos.y() = CLIP(endPos.y(), 0.0f, 1.0f);
	_autoScrollAnimation2._endVal = endPos;

	TeVector3f32 sizeRatio(1.0, 1.0, 0.0);
	if (_contentLayout) {
		sizeRatio = _contentLayout->size() / size();
	}

	float duration = 0.0;
	if (_autoScrollAnimation2Speed != 0.0) {
		const TeVector3f32 dist = endPos - startPos;
		if (_enclose) {
			sizeRatio = dist * (sizeRatio - TeVector3f32(1.0, 1.0, 0.0));
		} else {
			sizeRatio = dist * (sizeRatio + TeVector3f32(1.0, 1.0, 0.0));
		}
		duration = (sizeRatio * _direction).length() / (_autoScrollAnimation2Speed / 1000.0);
	}

	_autoScrollAnimation2._duration = duration;
	_autoScrollAnimation2._callbackObj = this;
	_autoScrollAnimation2._callbackMethod = &TeScrollingLayout::setScrollPosition;
	_autoScrollAnimation2.play();
	return false;
}

bool TeScrollingLayout::onAutoScrollAnimation1Finished() {
	playAutoScrollAnimation2();
	return false;
}

bool TeScrollingLayout::onAutoScrollAnimation2Finished() {
	_currentScrollLoopNo++;
	playAutoScrollAnimation1();
	return false;
}

bool TeScrollingLayout::onMouseMove(const Common::Point &pt) {
	_inertiaAnimation.stop();
	const TeVector3f32 scrollPos = scrollPosition();
	TeVector3f32 offset;
	TeInputMgr *inputmgr = g_engine->getInputMgr();
	if (_contentLayout) {
		const TeVector3f32 thisUserSz = userSize();
		const TeVector3f32 contentUserSz = _contentLayout->userSize();
		if (contentUserSz.y() <= thisUserSz.y())
			return false;

		const TeVector2s32 lastMouse = inputmgr->lastMousePos();
		if (!_enclose) {
			offset.x() = (-(int)(_direction.x() * (lastMouse._x - _slideDownMousePos._x))) /
						(xSize() + _contentLayout->xSize());
			offset.y() = ((lastMouse._y - _slideDownMousePos._y) * _direction.y()) / (ySize() + _contentLayout->ySize());
		} else {
			float xdiff = xSize() - _contentLayout->xSize();
			if (xdiff)
				offset.x() = (-(int)(_direction.x() * (lastMouse._x - _slideDownMousePos._x))) / xdiff;
			float ydiff = ySize() - _contentLayout->ySize();
			if (ydiff)
				offset.y() = ((lastMouse._y - _slideDownMousePos._y) * _direction.y()) / ydiff;
		}
	}

	setScrollPosition(scrollPos + offset);
	_slideDownMousePos = inputmgr->lastMousePos();
	TeVector3f32 nowMousePos(inputmgr->lastMousePos());
	_insideMouseThreshold = (_lastMouseDownPos - nowMousePos).length() <= _mouseMoveThreshold;
	uint64 elapsed = _scrollTimer.timeElapsed();
	if (elapsed > 0) {
		_speed = offset / (float)(elapsed / 1000000.0);
	}
	return false;
}

bool TeScrollingLayout::onMouseLeftUp(const Common::Point &pt) {
	_inertiaAnimation.stop();
	if (_contentLayout) {
		_inertiaAnimation.setCurve(_inertiaAnimationCurve);
		_inertiaAnimation._duration = _inertiaAnimationDuration;
		_inertiaAnimation._startVal = _speed;
		_inertiaAnimation._endVal = TeVector3f32(0, 0, 0);
		_inertiaAnimation._callbackObj = this;
		_inertiaAnimation._callbackMethod = &TeScrollingLayout::setSpeed;
		_inertiaAnimation.play();
	}

	TeInputMgr *inputmgr = g_engine->getInputMgr();
	inputmgr->_mouseMoveSignal.remove<TeScrollingLayout>(this, &TeScrollingLayout::onMouseMove);
	inputmgr->_mouseLUpSignal.remove<TeScrollingLayout>(this, &TeScrollingLayout::onMouseLeftUp);

	if (_autoScrollLoop == -1 || _currentScrollLoopNo < _autoScrollLoop) {
		_autoScrollDelayTimer.start();
		_autoScrollDelayTimer.setAlarmIn(_autoScrollDelay * 1000);
	}
	return false;
}

bool TeScrollingLayout::onSlideButtonDown() {
	_currentScrollLoopNo = 0;
	_inertiaAnimation.stop();
	_autoScrollDelayTimer.stop();
	_autoScrollAnimation1Timer.stop();
	_autoScrollAnimation2Timer.stop();
	_autoScrollAnimation1.stop();
	_autoScrollAnimation2.stop();

	_slideDownMousePos = g_engine->getInputMgr()->lastMousePos();

	_lastMouseDownPos = TeVector3f32(_slideDownMousePos);
	_insideMouseThreshold = true;

	TeInputMgr *inputmgr = g_engine->getInputMgr();

	Common::SharedPtr<TeCallback1Param<TeScrollingLayout, const Common::Point &>> callback;

	inputmgr->_mouseMoveSignal.remove(this, &TeScrollingLayout::onMouseMove);
	callback.reset(new TeCallback1Param<TeScrollingLayout, const Common::Point &>(
						this, &TeScrollingLayout::onMouseMove, FLT_MAX));
	inputmgr->_mouseMoveSignal.push_back(callback);

	inputmgr->_mouseLUpSignal.remove(this, &TeScrollingLayout::onMouseLeftUp);
	callback.reset(new TeCallback1Param<TeScrollingLayout, const Common::Point &>(
						this, &TeScrollingLayout::onMouseLeftUp, FLT_MAX));
	inputmgr->_mouseLUpSignal.push_back(callback);
	return false;
}

void TeScrollingLayout::playAutoScrollAnimation1() {
	if (!_autoScrollAnimation1Enabled) {
		playAutoScrollAnimation2();
		return;
	}

	if (_autoScrollLoop != -1 && _currentScrollLoopNo >= _autoScrollLoop)
		return;

	_autoScrollAnimation1Timer.start();
	_autoScrollAnimation1Timer.setAlarmIn(_autoScrollAnimation1Delay * 1000);
	return;
}

void TeScrollingLayout::playAutoScrollAnimation2() {
	if (!_autoScrollAnimation2Enabled) {
		_currentScrollLoopNo++;
		playAutoScrollAnimation1();
		return;
	}

	if (_autoScrollLoop != -1 && _currentScrollLoopNo >= _autoScrollLoop)
		return;

	_autoScrollAnimation2Timer.start();
	_autoScrollAnimation2Timer.setAlarmIn(_autoScrollAnimation2Delay * 1000);
}

void TeScrollingLayout::resetScrollPosition() {
	if (!_contentLayout)
		return;

	_inertiaAnimation.stop();
	_autoScrollDelayTimer.stop();
	_autoScrollAnimation1Timer.stop();
	_autoScrollAnimation2Timer.stop();
	_autoScrollAnimation1.stop();
	_autoScrollAnimation2.stop();
	_contentLayout->setPosition(_contentLayoutUserPos);
	_posUpdatedSignal.call();
}

void TeScrollingLayout::playAutoScroll() {
	_currentScrollLoopNo = 0;
	if (_autoScrollLoop < 1 && _autoScrollLoop != -1)
		return;
	_inertiaAnimation.stop();
	_autoScrollDelayTimer.stop();
	_autoScrollAnimation1Timer.stop();
	_autoScrollAnimation2Timer.stop();
	_autoScrollAnimation1.stop();
	_autoScrollAnimation2.stop();
	_autoScrollDelayTimer.start();
	_autoScrollDelayTimer.setAlarmIn(_autoScrollDelay * 1000);
}

} // end namespace Tetraedge
