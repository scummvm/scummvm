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
#include "engines/nancy/sound.h"
#include "engines/nancy/resource.h"
#include "engines/nancy/util.h"
#include "engines/nancy/input.h"
#include "engines/nancy/graphics.h"
#include "engines/nancy/puzzledata.h"
#include "engines/nancy/state/scene.h"

#include "engines/nancy/action/riddlepuzzle.h"

#include "common/random.h"

namespace Nancy {
namespace Action {

void RiddlePuzzle::init() {
	_drawSurface.create(_screenPosition.width(), _screenPosition.height(), g_nancy->_graphicsManager->getInputPixelFormat());
	_drawSurface.clear(g_nancy->_graphicsManager->getTransColor());

	setTransparent(true);
	setVisible(true);

	RenderObject::init();
}

void RiddlePuzzle::readData(Common::SeekableReadStream &stream) {
	_puzzleState = (RiddlePuzzleData *)NancySceneState.getPuzzleData(RiddlePuzzleData::getTag());
	assert(_puzzleState);

	_viewportTextFontID = stream.readUint16LE();
	_textboxTextFontID = stream.readUint16LE();
	_cursorBlinkTime = stream.readUint16LE();
	readRect(stream, _screenPosition);
	_typeSound.readData(stream, SoundDescription::kNormal);
	_eraseSound.readData(stream, SoundDescription::kNormal);
	_enterSound.readData(stream, SoundDescription::kNormal);
	_successSceneChange.readData(stream);
	_successSound.readData(stream, SoundDescription::kNormal);
	_exitSceneChange.readData(stream);
	_exitSound.readData(stream, SoundDescription::kNormal);
	readRect(stream, _exitHotspot);

	_riddles.resize(stream.readUint16LE()) ;
	stream.skip(4);

	char buf[128];
	for (uint i = 0; i < _riddles.size(); ++i) {
		Riddle &riddle = _riddles[i];

		stream.read(buf, 128);
		buf[127] = '\0';
		riddle.text = buf;
		riddle.sound.readData(stream, SoundDescription::kNormal);

		for (uint j = 0; j < 8; ++j) {
			stream.read(buf, 20);
			buf[19] = '\0';
			Common::String answer = buf;
			if (!answer.empty()) {
				riddle.answers.push_back(answer);
			}
		}

		riddle.sceneIncorrect.readData(stream);
		riddle.soundIncorrect.readData(stream, SoundDescription::kNormal);
		riddle.sceneCorrect.readData(stream);
		riddle.soundCorrect.readData(stream, SoundDescription::kNormal);
	}
}

void RiddlePuzzle::execute() {
	switch (_state) {
	case kBegin: {
		init();
		registerGraphics();
		_nextBlinkTime = g_nancy->getTotalPlayTime() + _cursorBlinkTime;

		g_nancy->_sound->loadSound(_typeSound);
		g_nancy->_sound->loadSound(_eraseSound);
		g_nancy->_sound->loadSound(_enterSound);

		// Make a list of non-answered riddle IDs
		Common::Array<byte> availableIDs;
		for (uint i = 0; i < _riddles.size(); ++i) {
			bool isAlreadySolved = false;
			for (auto id : _puzzleState->solvedRiddleIDs) {
				if (i == id) {
					isAlreadySolved = true;
					break;
				}
			}

			if (!isAlreadySolved) {
				availableIDs.push_back(i);
			}
		}

		if (availableIDs.size() == 0) {
			_solveState = kSolvedAll;
			_state = kRun;
			break;
		} else {
			if (_puzzleState->incorrectRiddleID != -1) {
				_riddleID = _puzzleState->incorrectRiddleID;
			} else {
				_riddleID = availableIDs[g_nancy->_randomSource->getRandomNumber(availableIDs.size() - 1)];
			}
		}

		g_nancy->_sound->loadSound(_riddles[_riddleID].sound);
		g_nancy->_sound->playSound(_riddles[_riddleID].sound);
		NancySceneState.getTextbox().clear();
		NancySceneState.getTextbox().overrideFontID(_textboxTextFontID);
		NancySceneState.getTextbox().addTextLine(_riddles[_riddleID].text);
		NancySceneState.setShouldClearTextbox(false);

		_state = kRun;
	}
		// fall through
	case kRun:
		switch (_solveState) {
		case kWaitForSound: 
			if (!g_nancy->_sound->isSoundPlaying(_riddles[_riddleID].sound)) {
				_solveState = kNotSolved;
				g_system->setFeatureState(OSystem::kFeatureVirtualKeyboard, true);

			}

			break;
		case kNotSolved: {
			Time currentTime = g_nancy->getTotalPlayTime();

			if (_playerHasHitReturn) {
				_playerHasHitReturn = false;

				if (_playerInput.lastChar() == '-') {
					_playerInput.deleteLastChar();
					drawText();
				}

				if (g_nancy->_sound->isSoundPlaying(_enterSound)) {
					break;
				}

				for (Common::String &answer : _riddles[_riddleID].answers) {
					if (_playerInput.equalsIgnoreCase(answer)) {
						// Solved a riddle
						_puzzleState->solvedRiddleIDs.push_back(_riddleID);

						if (_puzzleState->solvedRiddleIDs.size() == _riddles.size()) {
							// Solved all riddles
							g_nancy->_sound->loadSound(_successSound);
							g_nancy->_sound->playSound(_successSound);
							_solveState = kSolvedAll;
							_state = kActionTrigger;

							break;
						} else {
							// Still have riddles to solve
							g_nancy->_sound->loadSound(_riddles[_riddleID].soundCorrect);
							g_nancy->_sound->playSound(_riddles[_riddleID].soundCorrect);
							_solveState = kSolvedOne;
							_state = kActionTrigger;

							break;
						}
					}
				}

				if (_solveState == kNotSolved) {
					// Did not solve a riddle
					g_nancy->_sound->loadSound(_riddles[_riddleID].soundIncorrect);
					g_nancy->_sound->playSound(_riddles[_riddleID].soundIncorrect);
					_solveState = kFailed;
					_state = kActionTrigger;
				}
			} else if (currentTime >= _nextBlinkTime) {
				_nextBlinkTime = currentTime + _cursorBlinkTime;

				if (_playerInput.size() && _playerInput.lastChar() == '-') {
					_playerInput.deleteLastChar();
				} else {
					_playerInput += '-';
				}

				drawText();
			}

			break;
		}
		default:
			break;
		}

		break;
	case kActionTrigger: {
		SoundDescription *sound = nullptr;
		SceneChangeWithFlag *sceneChange = nullptr;
		_puzzleState->incorrectRiddleID = -1;

		switch (_solveState) {
		case kNotSolved:
			sound = &_exitSound;
			sceneChange = &_exitSceneChange;

			break;
		case kFailed:
			sound = &_riddles[_riddleID].soundIncorrect;
			sceneChange = &_riddles[_riddleID].sceneIncorrect;
			_puzzleState->incorrectRiddleID = _riddleID;

			break;
		case kSolvedOne:
			sound = &_riddles[_riddleID].soundCorrect;
			sceneChange = &_riddles[_riddleID].sceneCorrect;

			break;
		case kSolvedAll:
			sound = &_successSound;
			sceneChange = &_successSceneChange;

			break;
		default:
			return;	
		}

		if (g_nancy->_sound->isSoundPlaying(*sound)) {
			return;
		}

		g_nancy->_sound->stopSound(*sound);
		g_nancy->_sound->stopSound(_typeSound);
		g_nancy->_sound->stopSound(_eraseSound);
		g_nancy->_sound->stopSound(_enterSound);

		sceneChange->execute();
		g_system->setFeatureState(OSystem::kFeatureVirtualKeyboard, false);
		NancySceneState.setShouldClearTextbox(true);
		finishExecution();
	}
	}
}

void RiddlePuzzle::handleInput(NancyInput &input) {
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
		if (key.keycode == Common::KEYCODE_BACKSPACE) {
			if (_playerInput.size() && _playerInput.lastChar() == '-' ? _playerInput.size() > 1 : true) {
				if (_playerInput.lastChar() == '-') {
					_playerInput.deleteChar(_playerInput.size() - 2);
				} else {
					_playerInput.deleteLastChar();
				}

				g_nancy->_sound->playSound(_eraseSound);

				drawText();
			}
		} else if (key.keycode == Common::KEYCODE_RETURN) {
			if (_playerInput.size() == 0 ||
				(_playerInput.size() == 1 && _playerInput.lastChar() == '-')) {
					continue;
				}

				_playerHasHitReturn = true;
				g_nancy->_sound->playSound(_enterSound);
		} else if (Common::isAlnum(key.ascii) || Common::isSpace(key.ascii)) {
			if (_playerInput.size() && _playerInput.lastChar() == '-') {
				if (_playerInput.size() <= 16) {
					_playerInput.deleteLastChar();
					_playerInput += key.ascii;
					_playerInput += '-';
					g_nancy->_sound->playSound(_typeSound);
					drawText();
				}
			} else {
				if (_playerInput.size() <= 15) {
					_playerInput += key.ascii;
					g_nancy->_sound->playSound(_typeSound);
					drawText();
				}
			}
		}
	}
}

void RiddlePuzzle::drawText() {
	_drawSurface.clear(g_nancy->_graphicsManager->getTransColor());
	const Graphics::Font *font = g_nancy->_graphicsManager->getFont(_viewportTextFontID);

	Common::Rect bounds = getBounds();
	Common::Point destPoint(bounds.left, bounds.bottom - font->getFontHeight());
	font->drawString(&_drawSurface, _playerInput, destPoint.x, destPoint.y, bounds.width(), 0);

	_needsRedraw = true;
}

} // End of namespace Action
} // End of namespace Nancy
