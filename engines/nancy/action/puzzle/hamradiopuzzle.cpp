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

#include "common/random.h"
#include "common/config-manager.h"

#include "engines/nancy/nancy.h"
#include "engines/nancy/graphics.h"
#include "engines/nancy/resource.h"
#include "engines/nancy/sound.h"
#include "engines/nancy/input.h"
#include "engines/nancy/util.h"

#include "engines/nancy/state/scene.h"

#include "engines/nancy/action/puzzle/hamradiopuzzle.h"

namespace Nancy {
namespace Action {

static const char *morseCodeTable[] = {
	".-", "-...", "-.-.", "-..",	// a, b, c, d
	".", "..-.", "--.", "....",		// e, f, g, h
	"..", ".---", "-.-", ".-..",	// i, j, k, l
	"--", "-.", "---", ".--.",		// m, n, o, p
	"--.-", ".-.", "...", "-",		// q, r, s, t
	"..-", "...-", ".--", "-..-",	// u, v, w, x
	"-.--", "--.."					// y, z
};

void HamRadioPuzzle::init() {
	Common::Rect screenBounds = NancySceneState.getViewport().getBounds();
	_drawSurface.create(screenBounds.width(), screenBounds.height(), g_nancy->_graphics->getInputPixelFormat());
	_drawSurface.clear(g_nancy->_graphics->getTransColor());
	setTransparent(true);
	setVisible(true);
	moveTo(screenBounds);

	g_nancy->_resource->loadImage(_imageName, _image);
	_image.setTransparentColor(_drawSurface.getTransparentColor());
}

void HamRadioPuzzle::updateGraphics() {
	if (_digitsRolling) {
		uint32 curTime = g_nancy->getTotalPlayTime();
		bool allDigitsCorrect = true;

		for (uint i = 0; i < _numDigits; ++i) {
			if (curTime > _nextDigitFrameTimes[i]) {
				uint targetFrame = (_curDigits[i] == 0 ? (10 - 1) * 3 : (_curDigits[i] - 1) * 3);

				if (_displayedDigitFrames[i] == targetFrame) {
					continue;
				}

				if (++_displayedDigitFrames[i] >= 10 * 3) {
					_displayedDigitFrames[i] = 0;
				}

				// Have we arrived at the correct digit?
				if (_displayedDigitFrames[i] != targetFrame) {
					// If not, set the next frame time depending on how far away the next digit is
					// This way, the animation slows down as we approach the correct digit
					int frameDifference = targetFrame - _displayedDigitFrames[i];
					if (frameDifference < 0) {
						frameDifference += 10 * 3;
					}

					if (_nextDigitFrameTimes[i] == 0) {
						_nextDigitFrameTimes[i] = curTime;
					}

					switch (frameDifference) {
					case 1:
						_nextDigitFrameTimes[i] += 300; break;
					case 2:
						// fall through
					case 3:
						_nextDigitFrameTimes[i] += 200; break;
					case 4:
						// fall through
					case 5:
						_nextDigitFrameTimes[i] += 100; break;
					default:
						_nextDigitFrameTimes[i] += 50; break;
					}

					// Mark digits as incorrect
					allDigitsCorrect = false;
				}

				// Play the rolling sound
				g_nancy->_sound->loadSound(_digitRollSound, nullptr, true);
				g_nancy->_sound->playSound(_digitRollSound);

				// Finally, change the digit graphic
				_drawSurface.blitFrom(_image, _digitSrcs[_displayedDigitFrames[i]], _digitDests[i]);
				_needsRedraw = true;
			} else {
				// Still animating a digit, so we can't be at the correct one yet
				allDigitsCorrect = false;
			}
		}

		if (allDigitsCorrect) {
			// We've arrived at the correct digits, end the animation state
			_digitsRolling = false;
			Common::fill(_nextDigitFrameTimes.begin(), _nextDigitFrameTimes.end(), 0);
		}
	}
}

void HamRadioPuzzle::setFrequency(const Common::Array<uint16> &freq) {
	_isOnCorrectFrequency = false;
	_curMorseString.clear();
	_curCharString.clear();

	if (freq == _startFreq.frequency) {
		// Check start frequency
		_startFreq.sound.loadAndPlay();
		NancySceneState.setEventFlag(_startFreq.flag);
	} else if (freq == _correctFreq.frequency) {
		// Check correct transmission frequency
		_correctFreq.sound.loadAndPlay();
		NancySceneState.setEventFlag(_correctFreq.flag);
		_isOnCorrectFrequency = true;
	} else {
		// Check other frequencies
		for (auto &otherFreq : _otherFrequencies) {
			if (freq == otherFreq.frequency) {
				otherFreq.sound.loadAndPlay();
				NancySceneState.setEventFlag(otherFreq.flag);
				return;
			}
		}

		// No frequency found, pick random "bad" sound
		// This is re-rolled every time a bad frequency is connected to, even if the player
		// hasn't inputted any new digits
		_badFrequencySounds[g_nancy->_randomSource->getRandomNumber(_badFrequencySounds.size() - 1)].loadAndPlay();
	}
}

void HamRadioPuzzle::CCSound::readData(Common::SeekableReadStream &stream) {
	char buf[100];
	stream.read(buf, 100);
	assembleTextLine(buf, text, 100);
	sound.readNormal(stream);
}

void HamRadioPuzzle::CCSound::loadAndPlay() {
	g_nancy->_sound->loadSound(sound);
	g_nancy->_sound->playSound(sound);

	if (text.size() && ConfMan.getBool("subtitles")) {
		NancySceneState.getTextbox().clear();
		NancySceneState.getTextbox().addTextLine(text);
		NancySceneState.getTextbox().drawTextbox();
	}
}

void HamRadioPuzzle::Frequency::readData(Common::SeekableReadStream &stream, uint16 numDigits) {
	frequency.resize(numDigits);
	for (uint i = 0; i < numDigits; ++i) {
		frequency[i] = stream.readUint16LE();
	}
	stream.skip((8 - numDigits) * 2);

	sound.readData(stream);
	flag.label = stream.readUint16LE();
	flag.flag = stream.readByte();
}

void HamRadioPuzzle::readData(Common::SeekableReadStream &stream) {
	readFilename(stream, _imageName);

	_numDigits = stream.readUint16LE();

	_startFreq.readData(stream, _numDigits);
	_correctFreq.readData(stream, _numDigits);

	_passwordMaxSize = stream.readUint16LE();
	readFilename(stream, _password); // not a filename
	_passwordFlag.label = stream.readUint16LE();
	_passwordFlag.flag = stream.readByte();
	readFilename(stream, _codeWord); // not a filename

	stream.skip(2);

	readRectArray(stream, _digitDests, _numDigits, 8);
	readRectArray(stream, _buttonDests, 10 + 6);

	readRectArray(stream, _digitSrcs, 10 * 3); // digits 0-9, plus 2 inbetweens per digit
	readRectArray(stream, _buttonSrcs, 10 + 6);

	_digitRollSound.readNormal(stream);
	_frequencyButtonSound.readData(stream);
	_connectButtonSound.readData(stream);
	_dotButtonSound.readData(stream);
	_dashButtonSound.readData(stream);
	_sendButtonSound.readData(stream);
	_deleteButtonSound.readData(stream);
	_resetButtonSound.readData(stream);
	_badLetterSound.readData(stream);
	_longMorseOtherSound.readData(stream);
	_goodPasswordSound.readData(stream);
	_longMorseSound.readData(stream);

	_badFrequencySounds.resize(3);
	for (uint i = 0; i < 3; ++i) {
		_badFrequencySounds[i].readData(stream);
	}

	_solveScene.readData(stream);
	_solveSoundDelay = stream.readUint16LE();
	_solveSound.readData(stream);

	readRect(stream, _exitButtonDest);
	readRect(stream, _exitButtonSrc);

	_exitScene.readData(stream);
	_exitSoundDelay = stream.readUint16LE();
	_exitSound.readNormal(stream);

	uint16 numOtherFreqs = stream.readUint16LE();
	_otherFrequencies.resize(numOtherFreqs);
	for (uint i = 0; i < numOtherFreqs; ++i) {
		_otherFrequencies[i].readData(stream, _numDigits);
	}

	_curDigits.resize(_numDigits, 0);
	_displayedDigitFrames.resize(_numDigits, 0);
	_nextDigitFrameTimes.resize(_numDigits, 0);
}

void HamRadioPuzzle::execute() {
	switch (_state) {
	case kBegin :
		init();
		registerGraphics();

		g_nancy->_sound->loadSound(_digitRollSound);
		setFrequency(_startFreq.frequency);
		_curDigits = _startFreq.frequency;
		_state = kRun;
		// fall through
	case kRun :
		if (_pressedButton != kNone && g_nancy->getTotalPlayTime() > _buttonEndTime) {
			bool isDot = false;

			// Check for button presses
			switch (_pressedButton) {
			case kConnect:
				setFrequency(_curDigits);
				break;
			case kDot:
				isDot = true;
				// fall through
			case kDash:
				_curMorseString += isDot ? '.' : '-'; // Original engine uses the captions inside the dot and dash sounds

				if (_curMorseString.size() > 4) {
					_curMorseString.clear();
					_badLetterSound.loadAndPlay();
				} else {
					if (ConfMan.getBool("subtitles")) {
						NancySceneState.getTextbox().clear();
						NancySceneState.getTextbox().addTextLine(_curMorseString);
						NancySceneState.getTextbox().setOverrideFont(3); // Original engine pushes <f3> tag instead
						NancySceneState.getTextbox().drawTextbox();
					}
				}

				break;
			case kSend: {
				bool foundCorrect = false;

				if (_curMorseString.size()) {
					for (uint i = 0; i < ARRAYSIZE(morseCodeTable); ++i) {
						if (_curMorseString == morseCodeTable[i]) {
							foundCorrect = true;
							_curCharString += ('a' + i);
							break;
						}
					}
				}

				// Check if above maximum length string
				if (_curMorseString.size() > 10) {
					_longMorseSound.loadAndPlay();
					_curCharString.clear();
					_curMorseString.clear();
					break;
				}

				// Morse code is incorrect
				if (!foundCorrect) {
					_badLetterSound.loadAndPlay();
				}
			}

				// fall through
			case kDelete:
				_curMorseString.clear();

				if (_curCharString.size() > 10) {
					// Password is above max size, clear
					_curCharString.clear();
					NancySceneState.getTextbox().clear();
					_longMorseSound.loadAndPlay();
				} else if (_solvedPassword && _curCharString.size() > _passwordMaxSize) {
					// Password is above max size, clear
					_curCharString.clear();
					NancySceneState.getTextbox().clear();
					_longMorseOtherSound.loadAndPlay();
				}

				if (ConfMan.getBool("subtitles")) {
					NancySceneState.getTextbox().clear();
					NancySceneState.getTextbox().addTextLine(_curCharString);
					NancySceneState.getTextbox().drawTextbox();
				}

				if (_isOnCorrectFrequency) {
					// When transmitting on right frequency, check password/code word
					if (!_solvedPassword) {
						// Password not solved, check against it
						if (_curCharString == _password) {
							_solvedPassword = true;
							NancySceneState.setEventFlag(_passwordFlag);
							_curCharString.clear();
							_goodPasswordSound.loadAndPlay();
						}
					} else {
						// Password solved, check against codeword
						if (_curCharString == _codeWord) {
							_solvedCodeword = true;
							_curCharString.clear();

							_solveSound.loadAndPlay(); // Sound delay is ignored
						}
					}
				}

				break;
			case kReset:
				_curCharString.clear();
				_curMorseString.clear();
				NancySceneState.getTextbox().clear();

				break;
			default:
				// Number digit
				for (int i = 0; i < _numDigits - 1; ++i) {
					_curDigits[i] = _curDigits[i + 1];
				}

				_curDigits.back() = (_pressedButton == 9 ? 0 : _pressedButton + 1);
				_digitsRolling = true;

				break;
			}

			_drawSurface.fillRect(_buttonDests[_pressedButton], _drawSurface.getTransparentColor());
			_needsRedraw = true;

			_pressedButton = kNone;
		}

		break;
	case kActionTrigger:
		if (_digitsRolling) {
			return;
		}

		if (_solvedCodeword) {
			_solveScene.execute();
		} else {
			// Fail sound is ignored
			_exitScene.execute();
		}

		finishExecution();
		break;
	}
}

void HamRadioPuzzle::handleInput(NancyInput &input) {
	if (_digitsRolling || _state != kRun || _pressedButton != kNone) {
		return;
	}

	// Handle exit button
	if (NancySceneState.getViewport().convertViewportToScreen(_exitButtonDest).contains(input.mousePos)) {
		g_nancy->_cursor->setCursorType(CursorManager::kHotspot);

		if (input.input & NancyInput::kLeftMouseButtonUp) {
			_state = kActionTrigger;
			for (uint i = 0; i < _curDigits.size(); ++i) {
				_curDigits[i] = 0;
			}
			_digitsRolling = true;
			_drawSurface.blitFrom(_image, _exitButtonSrc, _exitButtonDest);
			_needsRedraw = true;
		}
		return;
	}

	// Handle other buttons
	for (uint i = 0; i < _buttonDests.size(); ++i) {
		if (NancySceneState.getViewport().convertViewportToScreen(_buttonDests[i]).contains(input.mousePos)) {
			if (i >= 10 || _pressedButton == kNone) {
				g_nancy->_cursor->setCursorType(CursorManager::kHotspot);

				if (input.input & NancyInput::kLeftMouseButtonUp) {
					_pressedButton = i;
					_drawSurface.blitFrom(_image, _buttonSrcs[i], _buttonDests[i]);
					_needsRedraw = true;

					CCSound *soundToPlay = nullptr;

					switch (i) {
					case kConnect:
						soundToPlay = &_connectButtonSound; break;
					case kDot:
						soundToPlay = &_dotButtonSound; break;
					case kDash:
						soundToPlay = &_dashButtonSound; break;
					case kSend:
						soundToPlay = &_sendButtonSound; break;
					case kDelete:
						soundToPlay = &_deleteButtonSound; break;
					case kReset:
						soundToPlay = &_resetButtonSound; break;
					default:
						soundToPlay = &_frequencyButtonSound; break;
					}

					// Do NOT use the loadAndPlaySound() function, since the dot/dash sounds have ./- captions
					g_nancy->_sound->loadSound(soundToPlay->sound, nullptr, true);
					g_nancy->_sound->playSound(soundToPlay->sound);
				}
			}

			break;
		}
	}

	if (_pressedButton != kNone) {
		_buttonEndTime = g_nancy->getTotalPlayTime() + 250;
	}
}

} // End of namespace Action
} // End of namespace Nancy
