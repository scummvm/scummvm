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

Clock::Clock() : 	RenderObject(11),
					_globe(10, this),
					_gargoyleEyes(9) {}

void Clock::init() {
	Common::SeekableReadStream *chunk = g_nancy->getBootChunkStream("CLOK");
	Graphics::ManagedSurface &object0 = g_nancy->_graphicsManager->_object0;

	_globe.init();

	if (chunk) {
		chunk->seek(0x80);

		_hoursHandSrcRects.resize(12);
		for (uint i = 0; i < 12; ++i) {
			readRect(*chunk, _hoursHandSrcRects[i]);
		}

		_minutesHandSrcRects.resize(4);
		for (uint i = 0; i < 4; ++i) {
			readRect(*chunk, _minutesHandSrcRects[i]);
		}

		Common::Rect dest;
		readRect(*chunk, dest);
		_drawSurface.create(dest.width(), dest.height(), g_nancy->_graphicsManager->getInputPixelFormat());
		moveTo(dest);

		_hoursHandDestRects.resize(12);
		for (uint i = 0; i < 12; ++i) {
			readRect(*chunk, _hoursHandDestRects[i]);
		}

		_minutesHandDestRects.resize(4);
		for (uint i = 0; i < 4; ++i) {
			readRect(*chunk, _minutesHandDestRects[i]);
		}

		Common::Rect gargoyleEyesSrcRect;
		Common::Rect gargoyleEyesDestRect;

		readRect(*chunk, gargoyleEyesSrcRect);
		readRect(*chunk, gargoyleEyesDestRect);
		_gargoyleEyes._drawSurface.create(object0, gargoyleEyesSrcRect);
		_gargoyleEyes.moveTo(gargoyleEyesDestRect);
		_gargoyleEyes.setVisible(false);

		_gargoyleEyes.setTransparent(true);
		_globe.setTransparent(true);
		GraphicsManager::loadSurfacePalette(_drawSurface, "OBJECT0");
		setTransparent(true);
	}
}

void Clock::registerGraphics() {
	_gargoyleEyes.registerGraphics();
	_globe.registerGraphics();
	RenderObject::registerGraphics();
}

void Clock::updateGraphics() {
	setVisible(_globe.getCurrentFrame() >= 5);

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
	if (!_globe.isPlaying()) {
		_globe.handleInput(input);
	}
}

void Clock::drawClockHands() {
	Graphics::ManagedSurface &object0 = g_nancy->_graphicsManager->_object0;
	uint hours = _playerTime.getHours();
	if (hours >= 12) {
		hours -= 12;
	}
	uint minutesHand = _playerTime.getMinutes() / 15;

	Common::Rect hoursDest = _hoursHandDestRects[hours];
	Common::Rect minutesDest = _minutesHandDestRects[minutesHand];

	hoursDest.translate(-_screenPosition.left, -_screenPosition.top);
	minutesDest.translate(-_screenPosition.left, -_screenPosition.top);

	_drawSurface.clear(g_nancy->_graphicsManager->getTransColor());
	_drawSurface.blitFrom(object0, _hoursHandSrcRects[hours], hoursDest);
	_drawSurface.blitFrom(object0, _minutesHandSrcRects[minutesHand], minutesDest);
}

void Clock::ClockGlobe::init() {
	Common::SeekableReadStream *chunk = g_nancy->getBootChunkStream("CLOK");

	if (chunk) {
		chunk->seek(0);
		_srcRects.resize(8);
		for (uint i = 0; i < 8; ++i) {
			readRect(*chunk, _srcRects[i]);
		}

		chunk->seek(0x180);
		Common::Rect screenDest;
		readRect(*chunk, screenDest);
		moveTo(screenDest);

		chunk->seek(0x2B0);
		_timeToKeepOpen = chunk->readUint32LE();
		_frameTime = chunk->readUint16LE();

		_alwaysHighlightCursor = true;
		_hotspot = _screenPosition;
	}
}

void Clock::ClockGlobe::updateGraphics() {
	AnimatedButton::updateGraphics();
	if (_isOpen && !isPlaying() && g_nancy->getTotalPlayTime() > _closeTime) {
		setOpen(false);
		_owner->_gargoyleEyes.setVisible(false);
		g_nancy->_sound->playSound("GLOB");
	}
}

void Clock::ClockGlobe::onClick() {
	if (!isPlaying()) {
		setOpen(!_isOpen);
		if (!_isOpen) {
			_owner->_gargoyleEyes.setVisible(false);
		}
		_owner->_playerTime = NancySceneState.getPlayerTime();
		g_nancy->_sound->playSound("GLOB");
	}
}

void Clock::ClockGlobe::onTrigger() {
	if (_isOpen) {
		_closeTime = g_nancy->getTotalPlayTime() + _timeToKeepOpen;
		_owner->_gargoyleEyes.setVisible(true);
	} else {
		_owner->setVisible(false);
	}
}

} // End of namespace UI
} // End of namespace Nancy
