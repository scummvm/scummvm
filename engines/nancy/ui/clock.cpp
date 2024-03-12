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

#include "engines/nancy/cursor.h"
#include "engines/nancy/input.h"
#include "engines/nancy/nancy.h"
#include "engines/nancy/graphics.h"
#include "engines/nancy/sound.h"
#include "engines/nancy/time.h"
#include "engines/nancy/util.h"

#include "engines/nancy/state/scene.h"

#include "engines/nancy/ui/clock.h"

namespace Nancy {
namespace UI {

Clock::Clock() : 	RenderObject(g_nancy->getGameType() == kGameTypeVampire ? 11 : 10),
					_animation(g_nancy->getGameType() == kGameTypeVampire ? 10 : 11, this),
					_staticImage(9),
					_clockData(nullptr),
					_locked(false) {}

void Clock::init() {
	Graphics::ManagedSurface &object0 = g_nancy->_graphics->_object0;

	_clockData = (const CLOK *)g_nancy->getEngineData("CLOK");
	assert(_clockData);

	// Calculate the size and location of the surface we'll need to draw the clock hands,
	// since their dest rects are in absolute screen space
	Common::Rect clockSurfaceScreenBounds;

	for (const Common::Rect &r : _clockData->hoursHandDests) {
		clockSurfaceScreenBounds.extend(r);
	}

	for (const Common::Rect &r : _clockData->minutesHandDests) {
		clockSurfaceScreenBounds.extend(r);
	}

	_drawSurface.create(clockSurfaceScreenBounds.width(), clockSurfaceScreenBounds.height(), g_nancy->_graphics->getInputPixelFormat());
	moveTo(clockSurfaceScreenBounds);

	_staticImage._drawSurface.create(object0, _clockData->staticImageSrc);
	_staticImage.moveTo(_clockData->staticImageDest);
	_staticImage.setVisible(false);
	_staticImage.setTransparent(g_nancy->getGameType() == kGameTypeVampire);

	_animation.setTransparent(true);
	_animation.setVisible(false);
	if (g_nancy->getGameType() == kGameTypeVampire) {
		GraphicsManager::loadSurfacePalette(_drawSurface, "OBJECT0");
	}

	setTransparent(true);

	_animation.init();
}

void Clock::registerGraphics() {
	_staticImage.registerGraphics();
	_animation.registerGraphics();
	RenderObject::registerGraphics();
}

void Clock::updateGraphics() {
	setVisible(_animation.getCurrentFrame() >= (g_nancy->getGameType() == kGameTypeVampire ? 5 : 1));

	if (_isVisible) {
		Time newPlayerTime = NancySceneState.getPlayerTime();

		if (newPlayerTime == _playerTime ||
			newPlayerTime.getMinutes() / 15 != _playerTime.getMinutes() / 15 ||
			newPlayerTime.getHours() != _playerTime.getHours()) {

			_playerTime = newPlayerTime;
			drawClockHands();
		}
	}
}

void Clock::handleInput(NancyInput &input) {
	if (!_locked) {
		_animation.handleInput(input);
	}
}

void Clock::drawClockHands() {
	Graphics::ManagedSurface &object0 = g_nancy->_graphics->_object0;
	uint hours = _playerTime.getHours();
	if (hours >= 12) {
		hours -= 12;
	}
	uint minutesHand = _playerTime.getMinutes() / 15;

	Common::Rect hoursDest = _clockData->hoursHandDests[hours];
	Common::Rect minutesDest = _clockData->minutesHandDests[minutesHand];

	hoursDest.translate(-_screenPosition.left, -_screenPosition.top);
	minutesDest.translate(-_screenPosition.left, -_screenPosition.top);

	_drawSurface.clear(g_nancy->_graphics->getTransColor());
	_drawSurface.blitFrom(object0, _clockData->hoursHandSrcs[hours], hoursDest);
	_drawSurface.blitFrom(object0, _clockData->minutesHandSrcs[minutesHand], minutesDest);
}

void Clock::ClockAnim::init() {
	auto *bootSummary = GetEngineData(BSUM);
	assert(bootSummary);

	_srcRects = _owner->_clockData->animSrcs;
	_destRects = _owner->_clockData->animDests;
	_highlightSrcRect = bootSummary->clockHighlightSrc;
	_highlightDestRect = bootSummary->extraButtonHighlightDest;

	if (_destRects.size()) {
		moveTo(bootSummary->extraButtonHotspot);
	} else {
		moveTo(_owner->_clockData->screenPosition);
	}

	_timeToKeepOpen = _owner->_clockData->timeToKeepOpen;
	_frameTime = _owner->_clockData->frameTime;

	_alwaysHighlightCursor = true;
	_hotspot = _screenPosition;
}

void Clock::ClockAnim::updateGraphics() {
	AnimatedButton::updateGraphics();
	if (_isOpen && !isPlaying() && (g_nancy->getTotalPlayTime() > _closeTime || _owner->_locked) && _isVisible) {
		setOpen(false);
		if (g_nancy->getGameType() == kGameTypeVampire) {
			_owner->_staticImage.setVisible(false);
		}
		g_nancy->_sound->playSound("GLOB");
	}
}

void Clock::ClockAnim::onClick() {
	if (!isPlaying()) {
		setOpen(!_isOpen);

		if (!_isOpen) {
			if (g_nancy->getGameType() == kGameTypeVampire) {
				_owner->_staticImage.setVisible(false);
			}
		} else if (g_nancy->getGameType() != kGameTypeVampire) {
			_owner->_staticImage.setVisible(true);
		}

		_owner->_playerTime = NancySceneState.getPlayerTime();
		g_nancy->_sound->playSound("GLOB");
	}
}

void Clock::ClockAnim::onTrigger() {
	if (_isOpen) {
		_closeTime = g_nancy->getTotalPlayTime() + _timeToKeepOpen;
		if (g_nancy->getGameType() == kGameTypeVampire) {
			_owner->_staticImage.setVisible(true);
		}
	} else {
		_owner->setVisible(false);
		_owner->_staticImage.setVisible(false);
	}
}

void Nancy5Clock::init() {
	_clockData = GetEngineData(CLOK);
	assert(_clockData);

	setVisible(true);
}

void Nancy5Clock::updateGraphics() {
	// Show current day
	if (_currentDay < 3) {
		if (NancySceneState.getEventFlag(59, true) && _currentDay == 1) {
			_currentDay = 2;
			_drawSurface.create(g_nancy->_graphics->_object0, _clockData->daySrcs[2]);
			moveTo(_clockData->staticImageDest);
			setVisible(true);
			setTransparent(true);
		} else if (NancySceneState.getEventFlag(58, true) && _currentDay == 0) {
			_currentDay = 1;
			_drawSurface.create(g_nancy->_graphics->_object0, _clockData->daySrcs[1]);
			moveTo(_clockData->staticImageDest);
			setVisible(true);
			setTransparent(true);
		} else if (NancySceneState.getEventFlag(57, true) && _currentDay == -1) {
			_currentDay = 0;
			_drawSurface.create(g_nancy->_graphics->_object0, _clockData->daySrcs[0]);
			moveTo(_clockData->staticImageDest);
			setVisible(true);
			setTransparent(true);
		}
	}

	// Show demolition countdown
	if (NancySceneState.getEventFlag(320, true)) {
		_currentDay = 3;
		Time timerTime = NancySceneState.getTimerTime();
		int32 countdownFrameID = MIN<int32>((uint32)timerTime / (_clockData->countdownTime / 12), 13);
		if (countdownFrameID != _countdownProgress) {
			_countdownProgress = countdownFrameID;

			_drawSurface.create(g_nancy->_graphics->_object0, _clockData->countdownSrcs[_countdownProgress]);
			moveTo(_clockData->staticImageDest);
			setVisible(true);
		}
	}
}

} // End of namespace UI
} // End of namespace Nancy
