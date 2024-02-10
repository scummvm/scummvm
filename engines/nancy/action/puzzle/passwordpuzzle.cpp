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

#include "engines/nancy/action/puzzle/passwordpuzzle.h"

#include "engines/nancy/state/scene.h"

namespace Nancy {
namespace Action {

PasswordPuzzle::~PasswordPuzzle() {
	g_nancy->_input->setVKEnabled(false);
}

void PasswordPuzzle::init() {
	_drawSurface.create(_screenPosition.width(), _screenPosition.height(), g_nancy->_graphics->getInputPixelFormat());
	_drawSurface.clear(g_nancy->_graphics->getTransColor());

	setTransparent(true);

	RenderObject::init();
}

void PasswordPuzzle::readData(Common::SeekableReadStream &stream) {
	Common::Serializer s(&stream, nullptr);
	s.setVersion(g_nancy->getGameType());

	s.syncAsUint16LE(_fontID);
	s.syncAsUint16LE(_cursorBlinkTime);
	readRect(s, _nameBounds);
	readRect(s, _passwordBounds);
	readRect(s, _screenPosition);

	uint numNames = 1;
	uint numPasswords = 1;
	char buf[33];
	uint fieldSize = s.getVersion() <= kGameTypeNancy5 ? 20 : 33; // nancy6 changed the size of text fields to 33

	s.syncAsUint16LE(numNames, kGameTypeNancy4);
	_names.resize(numNames);
	for (uint i = 0; i < numNames; ++i) {
		stream.read(buf, fieldSize);
		buf[fieldSize - 1] = '\0';
		_names[i] = buf;
	}
	s.skip((5 - numNames) * fieldSize, kGameTypeNancy4);

	s.syncAsUint16LE(numPasswords, kGameTypeNancy4);
	_passwords.resize(numPasswords);
	for (uint i = 0; i < numPasswords; ++i) {
		stream.read(buf, fieldSize);
		buf[19] = '\0';
		_passwords[i] = buf;
	}
	s.skip((5 - numPasswords) * fieldSize, kGameTypeNancy4);

	_maxStringLength = g_nancy->getGameType() < kGameTypeNancy6 ? 12 : 31;

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
		g_nancy->_input->setVKEnabled(true);
		_nextBlinkTime = g_nancy->getTotalPlayTime() + _cursorBlinkTime;
		_state = kRun;
		// fall through
	case kRun:
		switch (_solveState) {
		case kNotSolved: {
			Common::String &activeField = _passwordFieldIsActive ? _playerPasswordInput : _playerNameInput;
			Common::Array<Common::String> &correctAnswers = _passwordFieldIsActive ? _passwords : _names;
			Time currentTime = g_nancy->getTotalPlayTime();

			if (_playerHasHitReturn) {
				_playerHasHitReturn = false;

				if (activeField.lastChar() == '-') {
					activeField.deleteLastChar();
					drawText();
				}

				bool solvedCurrentInput = false;
				if (correctAnswers.size()) {
					for (uint i = 0; i < correctAnswers.size(); ++i) {
						if (activeField.equalsIgnoreCase(correctAnswers[i])) {
							solvedCurrentInput = true;
							break;
						}
					}
				} else {
					solvedCurrentInput = true;
				}

				if (solvedCurrentInput) {
					if (_passwordFieldIsActive || _passwords.size() == 0) {
						g_nancy->_sound->loadSound(_solveSound);
						g_nancy->_sound->playSound(_solveSound);
						_solveState = kSolved;
					} else {
						_passwordFieldIsActive = true;
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

		g_nancy->_input->setVKEnabled(false);
		finishExecution();
	}
}

void PasswordPuzzle::onPause(bool paused) {
	g_nancy->_input->setVKEnabled(!paused);
	RenderActionRecord::onPause(paused);
}

void PasswordPuzzle::handleInput(NancyInput &input) {
	if (_solveState != kNotSolved) {
		return;
	}

	if (NancySceneState.getViewport().convertViewportToScreen(_exitHotspot).contains(input.mousePos)) {
		g_nancy->_cursor->setCursorType(g_nancy->_cursor->_puzzleExitCursor);

		if (input.input & NancyInput::kLeftMouseButtonUp) {
			_state = kActionTrigger;
		}
	}

	for (uint i = 0; i < input.otherKbdInput.size(); ++i) {
		Common::KeyState &key = input.otherKbdInput[i];
		Common::String &activeField = _passwordFieldIsActive ? _playerPasswordInput : _playerNameInput;
		if (key.keycode == Common::KEYCODE_BACKSPACE) {
			if (activeField.size() && activeField.lastChar() == '-' ? activeField.size() > 1 : true) {
				if (activeField.lastChar() == '-') {
					activeField.deleteChar(activeField.size() - 2);
				} else {
					activeField.deleteLastChar();
				}

				drawText();
			}
		} else if (key.keycode == Common::KEYCODE_RETURN || key.keycode == Common::KEYCODE_KP_ENTER) {
			_playerHasHitReturn = true;
		} else if (Common::isAlnum(key.ascii) || Common::isSpace(key.ascii)) {
			if (activeField.size() && activeField.lastChar() == '-') {
				if (activeField.size() <= _maxStringLength + 1) {
					activeField.deleteLastChar();
					activeField += key.ascii;
					activeField += '-';
				}
			} else {
				if (activeField.size() <= _maxStringLength) {
					activeField += key.ascii;
				}
			}

			drawText();
		}
	}
}

void PasswordPuzzle::drawText() {
	_drawSurface.clear(g_nancy->_graphics->getTransColor());
	const Graphics::Font *font = g_nancy->_graphics->getFont(_fontID);

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
