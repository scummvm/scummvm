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

#include "engines/nancy/nancy.h"
#include "engines/nancy/graphics.h"
#include "engines/nancy/resource.h"
#include "engines/nancy/sound.h"
#include "engines/nancy/input.h"
#include "engines/nancy/util.h"
#include "engines/nancy/state/scene.h"
#include "engines/nancy/ui/viewport.h"

#include "engines/nancy/action/overridelockpuzzle.h"

#include "common/random.h"

namespace Nancy {
namespace Action {

void OverrideLockPuzzle::init() {
	Common::Rect bounds = NancySceneState.getViewport().getBounds();

	_drawSurface.create(bounds.width(), bounds.height(), g_nancy->_graphicsManager->getInputPixelFormat());
	_drawSurface.clear(g_nancy->_graphicsManager->getTransColor());

	setTransparent(true);
	setVisible(true);
	moveTo(bounds);

	g_nancy->_resource->loadImage(_imageName, _image);
	_image.setTransparentColor(_drawSurface.getTransparentColor());
}

void OverrideLockPuzzle::readData(Common::SeekableReadStream &stream) {
	readFilename(stream, _imageName);

	byte num = stream.readByte();
	_popButtons = stream.readByte();
	_randomizeLights = stream.readByte();

	readRectArray(stream, _buttonSrcs, num);
	stream.skip((10 - num) * 16);
	readRectArray(stream, _buttonDests, num);
	stream.skip((10 - num) * 16);
	readRectArray(stream, _hotspots, num);
	stream.skip((10 - num) * 16);
	readRectArray(stream, _lightSrcs, num);
	stream.skip((10 - num) * 16);
	readRectArray(stream, _lightDests, num);
	stream.skip((10 - num) * 16);

	_buttonSound.read(stream, SoundDescription::kNormal);
	_wrongSound.read(stream, SoundDescription::kNormal);

	_buttonPopTime = stream.readUint16LE();

	_solveExitScene.readData(stream);
	_solveSound.read(stream, SoundDescription::kNormal);

	_exitScene.readData(stream);
	readRect(stream, _exitHotspot);
}

void OverrideLockPuzzle::execute() {
	switch (_state) {
	case kBegin: {
		init();
		registerGraphics();
		
		// Set the order of the button presses (always random)
		// and of the lights (only random on expert difficulty)
		uint numButtons = _buttonSrcs.size();
		_buttonOrder.resize(numButtons);
		_lightsOrder.resize(numButtons);
		Common::Array<byte> buttonIDs(numButtons);
		Common::Array<byte> lightIDs(numButtons);
		for (uint i = 0; i < numButtons; ++i) {
			buttonIDs[i] = i;
			lightIDs[i] = i;
		}

		for (uint i = 0; i < numButtons; ++i) {
			_buttonOrder[i] = buttonIDs.remove_at(g_nancy->_randomSource->getRandomNumber(buttonIDs.size() - 1));

			if (_randomizeLights == kLightsRandom) {
				_lightsOrder[i] = lightIDs.remove_at(g_nancy->_randomSource->getRandomNumber(lightIDs.size() - 1));
			} else {
				_lightsOrder[i] = i;
			}
		}

		g_nancy->_sound->loadSound(_buttonSound);
		g_nancy->_sound->loadSound(_wrongSound);
		_state = kRun;
	}
		// fall through
	case kRun:
		if (_timeToPop != 0 && g_nancy->getTotalPlayTime() > _timeToPop) {
			if (_popButtons == kButtonsPopUp) {
				drawButton(_lastPushedButton, true);
			}
			
			drawLights();

			_lastPushedButton = -1;
			_timeToPop = 0;
			
			for (uint i = 0; i < _playerOrder.size(); ++i) {
				if (_playerOrder[i] != _buttonOrder[i]) {
					// Wrong order, reset
					_drawSurface.clear(_drawSurface.getTransparentColor());
					_playerOrder.clear();
					_needsRedraw = true;
					g_nancy->_sound->playSound(_wrongSound);
					return;
				}
			}

			if (_playerOrder.size() == _buttonOrder.size()) {
				// Solved the puzzle
				g_nancy->_sound->loadSound(_solveSound);
				g_nancy->_sound->playSound(_solveSound);
				_state = kActionTrigger;
				_solveState = kSolved;
			}
		}

		break;
	case kActionTrigger:
		switch (_solveState) {
		case kNotSolved:
			_exitScene.execute();
			break;
		case kSolved:
			if (g_nancy->_sound->isSoundPlaying(_solveSound)) {
				return;
			}

			_solveExitScene.execute();
			g_nancy->_sound->stopSound(_solveSound);
		}

		g_nancy->_sound->stopSound(_buttonSound);
		g_nancy->_sound->stopSound(_wrongSound);
		finishExecution();
	}
}

void OverrideLockPuzzle::handleInput(NancyInput &input) {
	if ((_state != kRun && _solveState != kNotSolved) || _timeToPop != 0) {
		return;
	}

	// Check the exit hotspot
	if (NancySceneState.getViewport().convertViewportToScreen(_exitHotspot).contains(input.mousePos)) {
		g_nancy->_cursorManager->setCursorType(CursorManager::kExit);

		if (input.input & NancyInput::kLeftMouseButtonUp) {
			_state = kActionTrigger;
		}

		return;
	}

	for (uint i = 0; i < _buttonOrder.size(); ++i) {
		bool hotspotIsInactive = false;
		for (uint j = 0; j < _playerOrder.size(); ++j) {
			if (_playerOrder[j] == i) {
				hotspotIsInactive = true;
				break;
			}
		}

		if (hotspotIsInactive) {
			continue;
		}

		if (NancySceneState.getViewport().convertViewportToScreen(_hotspots[i]).contains(input.mousePos)) {
			g_nancy->_cursorManager->setCursorType(CursorManager::kHotspot);
			
			if (input.input & NancyInput::kLeftMouseButtonUp) {
				drawButton(i, false);
				_lastPushedButton = i;
				_timeToPop = g_nancy->getTotalPlayTime() + _buttonPopTime;
				_playerOrder.push_back(i);
				g_nancy->_sound->playSound(_buttonSound);
			}

			break;
		}
	}
}

void OverrideLockPuzzle::drawButton(uint buttonID, bool clear) {
	if (clear) {
		_drawSurface.fillRect(_buttonDests[buttonID], _drawSurface.getTransparentColor());
		return;
	} else {
		_drawSurface.blitFrom(_image, _buttonSrcs[buttonID], _buttonDests[buttonID]);
	}

	_needsRedraw = true;
}

void OverrideLockPuzzle::drawLights() {
	for (uint i = 0; i < _playerOrder.size(); ++i) {
		if (_randomizeLights == kLightsCircular) {
			_drawSurface.blitFrom(_image, _lightSrcs[i], _lightDests[i]);
		} else {
			_drawSurface.blitFrom(_image, _lightSrcs[_lightsOrder[i]], _lightDests[_lightsOrder[i]]);
		}
	}

	_needsRedraw = true;
}

} // End of namespace Action
} // End of namespace Nancy
