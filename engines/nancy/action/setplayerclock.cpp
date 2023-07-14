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

#include "engines/nancy/action/setplayerclock.h"
#include "engines/nancy/state/scene.h"

#include "engines/nancy/nancy.h"
#include "engines/nancy/input.h"
#include "engines/nancy/graphics.h"
#include "engines/nancy/util.h"
#include "engines/nancy/resource.h"
#include "engines/nancy/sound.h"

namespace Nancy {
namespace Action {

void SetPlayerClock::init() {
	Common::Rect screenBounds = NancySceneState.getViewport().getBounds();
	_drawSurface.create(screenBounds.width(), screenBounds.height(), g_nancy->_graphicsManager->getInputPixelFormat());
	_drawSurface.clear(g_nancy->_graphicsManager->getTransColor());
	setTransparent(true);
	setVisible(true);
	moveTo(screenBounds);

	g_nancy->_resource->loadImage(_imageName, _image);
	_image.setTransparentColor(_drawSurface.getTransparentColor());
}

void SetPlayerClock::readData(Common::SeekableReadStream &stream) {
	readFilename(stream, _imageName);

	readRect(stream, _minutesDest);
	readRect(stream, _hoursDest);
	readRect(stream, _AMPMDest);
	readRect(stream, _timeButtonDest);
	readRect(stream, _alarmButtonDest);
	readRect(stream, _setButtonDest);
	readRect(stream, _cancelButtonDest);
	readRect(stream, _upButtonDest);
	readRect(stream, _downButtonDest);
	readRect(stream, _modeLightDest);

	readRectArray(stream, _minutesSrc, 4);
	readRectArray(stream, _hoursSrc, 12);

	readRect(stream, _AMSrc);
	readRect(stream, _PMSrc);
	readRect(stream, _timeButtonSrc);
	readRect(stream, _alarmButtonSrc);
	readRect(stream, _setButtonSrc);
	readRect(stream, _cancelButtonSrc);
	readRect(stream, _upButtonSrc);
	readRect(stream, _downButtonSrc);
	readRect(stream, _timeLightSrc);
	readRect(stream, _alarmLightSrc);

	stream.skip(2);

	_buttonSound.readNormal(stream);
	_alarmSetScene.readData(stream);
	_alarmSoundDelay = stream.readUint16LE();
	_alarmRingSound.readNormal(stream);
	_exitScene.readData(stream);
}

void SetPlayerClock::execute() {
	switch (_state) {
	case kBegin:
		init();
		registerGraphics();

		g_nancy->_sound->loadSound(_buttonSound);
		g_nancy->_sound->loadSound(_alarmRingSound);

		_alarmHours = NancySceneState.getPlayerTime().getHours();

		_state = kRun;
		// fall through
	case kRun:
		if (_alarmState == kTimeMode) {
			Time currentTime = NancySceneState.getPlayerTime();
			int8 hours = currentTime.getHours();
			int8 minutes = currentTime.getMinutes();

			if (_clearButton && !g_nancy->_sound->isSoundPlaying(_buttonSound)) {
				_drawSurface.fillRect(_timeButtonDest, _drawSurface.getTransparentColor());
				_drawSurface.fillRect(_modeLightDest, _drawSurface.getTransparentColor());
				_drawSurface.blitFrom(_image, _timeLightSrc, _modeLightDest);
				_clearButton = false;
				_needsRedraw = true;
			}

			if (_lastDrawnHours != hours || _lastDrawnMinutes / 15 != minutes / 15) {
				drawTime(currentTime.getHours(), currentTime.getMinutes());
				_lastDrawnHours = hours;
				_lastDrawnMinutes = minutes;
			}
		} else {
			if (_clearButton && !g_nancy->_sound->isSoundPlaying(_buttonSound)) {
				_drawSurface.fillRect(_alarmButtonDest, _drawSurface.getTransparentColor());
				_drawSurface.fillRect(_upButtonDest, _drawSurface.getTransparentColor());
				_drawSurface.fillRect(_downButtonDest, _drawSurface.getTransparentColor());
				_drawSurface.fillRect(_modeLightDest, _drawSurface.getTransparentColor());
				_drawSurface.blitFrom(_image, _alarmLightSrc, _modeLightDest);
				drawTime(_alarmHours, 0);
				_clearButton = false;
				_needsRedraw = true;
			}
		}

		break;
	case kActionTrigger:
		if (g_nancy->_sound->isSoundPlaying(_buttonSound)) {
			return;
		}

		if (_clearButton) {
			g_nancy->_sound->stopSound(_buttonSound);
			_drawSurface.fillRect(_setButtonDest, _drawSurface.getTransparentColor());
			_clearButton = false;
		}

		if (_alarmState == kWait) {
			// Alarm has been set, wait for timer
			if (g_system->getMillis() > _sceneChangeTime) {
				NancySceneState.setPlayerTime(_alarmHours * 3600000, false);
				_alarmSetScene.execute();
				finishExecution();
			}
		} else {
			// Cancel button pressed, go to exit scene
			_exitScene.execute();
			finishExecution();
		}
	}
}

void SetPlayerClock::handleInput(NancyInput &input) {
	if (_alarmState == kWait) {
		return;
	}

	// Cancel button is active in both time and alarm mode
	if (NancySceneState.getViewport().convertViewportToScreen(_cancelButtonDest).contains(input.mousePos)) {
		g_nancy->_cursorManager->setCursorType(CursorManager::kHotspot);

		if (input.input & NancyInput::kLeftMouseButtonUp) {
			// Cancel button pressed
			_drawSurface.blitFrom(_image, _cancelButtonSrc, _cancelButtonDest);
			_needsRedraw = true;

			g_nancy->_sound->playSound(_buttonSound);
			
			_state = kActionTrigger;
			return;
		}
	}

	if (_alarmState == kTimeMode) {
		// Alarm button is active only in time mode
		if (NancySceneState.getViewport().convertViewportToScreen(_alarmButtonDest).contains(input.mousePos)) {
			g_nancy->_cursorManager->setCursorType(CursorManager::kHotspot);

			if (input.input & NancyInput::kLeftMouseButtonUp) {
				// Alarm button pressed
				_drawSurface.blitFrom(_image, _alarmButtonSrc, _alarmButtonDest);
				_needsRedraw = true;

				g_nancy->_sound->playSound(_buttonSound);
				_lastDrawnHours = _lastDrawnMinutes = -1;
				
				_alarmState = kAlarmMode;
				_clearButton = true;
				return;
			}
		}
	} else {
		if (NancySceneState.getViewport().convertViewportToScreen(_timeButtonDest).contains(input.mousePos)) {
			// Time button is active only in alarm mode
			g_nancy->_cursorManager->setCursorType(CursorManager::kHotspot);

			if (input.input & NancyInput::kLeftMouseButtonUp) {
				// Alarm button pressed
				_drawSurface.blitFrom(_image, _timeButtonSrc, _timeButtonDest);
				_needsRedraw = true;

				g_nancy->_sound->playSound(_buttonSound);
				
				_alarmState = kTimeMode;
				_clearButton = true;
				return;
			}
		} else if (NancySceneState.getViewport().convertViewportToScreen(_upButtonDest).contains(input.mousePos)) {
			// Up button is active only in alarm mode
			g_nancy->_cursorManager->setCursorType(CursorManager::kHotspot);

			if (input.input & NancyInput::kLeftMouseButtonUp) {
				// Up button pressed
				_drawSurface.blitFrom(_image, _upButtonSrc, _upButtonDest);
				_needsRedraw = true;

				g_nancy->_sound->playSound(_buttonSound);
				_alarmHours = _alarmHours + 1 > 23 ? 0 : _alarmHours + 1;
				
				_clearButton = true;
				return;
			}
		} else if (NancySceneState.getViewport().convertViewportToScreen(_downButtonDest).contains(input.mousePos)) {
			// Down button is active only in alarm mode
			g_nancy->_cursorManager->setCursorType(CursorManager::kHotspot);

			if (input.input & NancyInput::kLeftMouseButtonUp) {
				// Down button pressed
				_drawSurface.blitFrom(_image, _downButtonSrc, _downButtonDest);
				_needsRedraw = true;

				g_nancy->_sound->playSound(_buttonSound);
				_alarmHours = _alarmHours - 1 < 0 ? 23 : _alarmHours - 1;
				
				_clearButton = true;
				return;
			}
		} else if (NancySceneState.getViewport().convertViewportToScreen(_setButtonDest).contains(input.mousePos)) {
			// Set button is active only in alarm mode
			g_nancy->_cursorManager->setCursorType(CursorManager::kHotspot);

			if (input.input & NancyInput::kLeftMouseButtonUp) {
				// Down button pressed
				_drawSurface.blitFrom(_image, _setButtonSrc, _setButtonDest);
				_needsRedraw = true;

				g_nancy->_sound->playSound(_buttonSound);
				
				_clearButton = true;
				_state = kActionTrigger;
				_alarmState = kWait;
				_sceneChangeTime = g_system->getMillis() + (_alarmSoundDelay * 1000);
				return;
			}
		}
	}
}

void SetPlayerClock::drawTime(uint16 hours, uint16 minutes) {
	_drawSurface.fillRect(_hoursDest, _drawSurface.getTransparentColor());
	_drawSurface.fillRect(_minutesDest, _drawSurface.getTransparentColor());
	_drawSurface.fillRect(_AMPMDest, _drawSurface.getTransparentColor());

	_drawSurface.blitFrom(_image, _hoursSrc[(hours - 1 < 0 ? 11 : hours - 1) % 12], _hoursDest);
	_drawSurface.blitFrom(_image, _minutesSrc[minutes / 15], _minutesDest);
	_drawSurface.blitFrom(_image, hours / 12 == 0 ? _AMSrc : _PMSrc, _AMPMDest);

	_needsRedraw = true;
}

} // End of namespace Action
} // End of namespace Nancy
