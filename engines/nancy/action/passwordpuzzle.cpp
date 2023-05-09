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

#include "engines/nancy/graphics.h"
#include "engines/nancy/nancy.h"
#include "engines/nancy/sound.h"
#include "engines/nancy/input.h"
#include "engines/nancy/util.h"

#include "engines/nancy/action/passwordpuzzle.h"

#include "engines/nancy/state/scene.h"

namespace Nancy {
namespace Action {

void PasswordPuzzle::init() {
	_drawSurface.create(_screenPosition.width(), _screenPosition.height(), g_nancy->_graphicsManager->getInputPixelFormat());
	_drawSurface.clear(g_nancy->_graphicsManager->getTransColor());

	setTransparent(true);

	RenderObject::init();
}

void PasswordPuzzle::readData(Common::SeekableReadStream &stream) {
	_fontID = stream.readUint16LE();
	_cursorBlinkTime = stream.readUint16LE();
	readRect(stream, _nameBounds);
	readRect(stream, _passwordBounds);
	readRect(stream, _screenPosition);

	char buf[20];
	stream.read(buf, 20);
	buf[19] = '\0';
	_name = buf;
	stream.read(buf, 20);
	buf[19] = '\0';
	_password = buf;
	_solveExitScene.readData(stream);
	_solveSound.readNormal(stream);
	_failExitScene.readData(stream);
	_failSound.readNormal(stream);
	_exitScene.readData(stream);
	readRect(stream, _exitHotspot);
}

void PasswordPuzzle::execute() {
	switch (_state) {
	case kBegin:
		init();
		registerGraphics();
		g_system->setFeatureState(OSystem::kFeatureVirtualKeyboard, true);
		_nextBlinkTime = g_nancy->getTotalPlayTime() + _cursorBlinkTime;
		_state = kRun;
		// fall through
	case kRun:
		switch (_solveState) {
		case kNotSolved: {
			Common::String &activeField = _passwordFieldIsActive ? _playerPasswordInput : _playerNameInput;
			Common::String &correctField = _passwordFieldIsActive ? _password : _name;
			Time currentTime = g_nancy->getTotalPlayTime();

			if (_playerHasHitReturn) {
				_playerHasHitReturn = false;

				if (activeField.lastChar() == '-') {
					activeField.deleteLastChar();
					drawText();
				}

				if (activeField.equalsIgnoreCase(correctField)) {
					if (!_passwordFieldIsActive) {
						_passwordFieldIsActive = true;
					} else {
						g_nancy->_sound->loadSound(_solveSound);
						g_nancy->_sound->playSound(_solveSound);
						_solveState = kSolved;
					}
				} else {
					g_nancy->_sound->loadSound(_failSound);
					g_nancy->_sound->playSound(_failSound);
					_solveState = kFailed;
				}


			} else if (currentTime >= _nextBlinkTime) {
				_nextBlinkTime = currentTime + _cursorBlinkTime;

				if (activeField.size() && activeField.lastChar() == '-') {
					activeField.deleteLastChar();
				} else {
					activeField += '-';
				}

				drawText();
			}

			break;
		}
		case kFailed:
			if (!g_nancy->_sound->isSoundPlaying(_failSound)) {
				g_nancy->_sound->stopSound(_failSound);
				_state = kActionTrigger;
			}

			break;
		case kSolved:
			if (!g_nancy->_sound->isSoundPlaying(_solveSound)) {
				g_nancy->_sound->stopSound(_solveSound);
				_state = kActionTrigger;
			}

			break;
		}

		break;
	case kActionTrigger:
		switch (_solveState) {
		case kNotSolved:
			_exitScene.execute();
			break;
		case kFailed:
			_failExitScene.execute();
			break;
		case kSolved:
			_solveExitScene.execute();
			break;
		}
		
		g_system->setFeatureState(OSystem::kFeatureVirtualKeyboard, false);
		finishExecution();
	}
}

void PasswordPuzzle::handleInput(NancyInput &input) {
	if (_solveState != kNotSolved) {
		return;
	}

	if (NancySceneState.getViewport().convertViewportToScreen(_exitHotspot).contains(input.mousePos)) {
		g_nancy->_cursorManager->setCursorType(CursorManager::kExit);

		if (input.input & NancyInput::kLeftMouseButtonUp) {
			_state = kActionTrigger;
		}
	}

	for (uint i = 0; i < input.otherKbdInput.size(); ++i) {
		Common::KeyState &key = input.otherKbdInput[i];
		Common::String &activeField = _passwordFieldIsActive ? _playerPasswordInput : _playerNameInput;
		Common::String &correctField = _passwordFieldIsActive ? _password : _name;
		if (key.keycode == Common::KEYCODE_BACKSPACE) {
			if (activeField.size() && activeField.lastChar() == '-' ? activeField.size() > 1 : true) {
				if (activeField.lastChar() == '-') {
					activeField.deleteChar(activeField.size() -2);
				} else {
					activeField.deleteLastChar();
				}

				drawText();
			}
		} else if (key.keycode == Common::KEYCODE_RETURN) {
			_playerHasHitReturn = true;
		} else if (Common::isAlnum(key.ascii) || Common::isSpace(key.ascii)) {
			if (activeField.size() && activeField.lastChar() == '-') {
				if (activeField.size() <= correctField.size() + 2) {
					activeField.deleteLastChar();
					activeField += key.ascii;
					activeField += '-';
				}
			} else {
				if (activeField.size() <= correctField.size() + 1) {
					activeField += key.ascii;
				}
			}

			drawText();
		}
	}
}

void PasswordPuzzle::drawText() {
	_drawSurface.clear(g_nancy->_graphicsManager->getTransColor());
	const Graphics::Font *font = g_nancy->_graphicsManager->getFont(_fontID);

	Common::Rect bounds = _nameBounds;
	bounds = NancySceneState.getViewport().convertViewportToScreen(bounds);
	bounds = convertToLocal(bounds);
	Common::Point destPoint(bounds.left, bounds.bottom + 1 - font->getFontHeight());
	font->drawString(&_drawSurface, _playerNameInput, destPoint.x, destPoint.y, bounds.width(), 0);

	bounds = _passwordBounds;
	bounds = NancySceneState.getViewport().convertViewportToScreen(bounds);
	bounds = convertToLocal(bounds);
	destPoint.x = bounds.left;
	destPoint.y = bounds.bottom + 1 - font->getFontHeight();
	font->drawString(&_drawSurface, _playerPasswordInput, destPoint.x, destPoint.y, bounds.width(), 0);

	_needsRedraw = true;
}

} // End of namespace Action
} // End of namespace Nancy
