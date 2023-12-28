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

#include "engines/nancy/action/puzzle/telephone.h"

#include "engines/nancy/state/scene.h"

namespace Nancy {
namespace Action {

void Telephone::init() {
	_drawSurface.create(_screenPosition.width(), _screenPosition.height(), g_nancy->_graphicsManager->getInputPixelFormat());
	_drawSurface.clear(g_nancy->_graphicsManager->getTransColor());

	setTransparent(true);

	g_nancy->_resource->loadImage(_imageName, _image);
}

void Telephone::readData(Common::SeekableReadStream &stream) {
	readFilename(stream, _imageName);

	readRectArray(stream, _srcRects, 12);
	readRectArray(stream, _destRects, 12);

	_screenPosition = _destRects[0];
	for (uint i = 0; i < 12; ++i) {
		_screenPosition.extend(_destRects[i]);
	}

	_genericDialogueSound.readNormal(stream);
	_genericButtonSound.readNormal(stream);
	_ringSound.readNormal(stream);
	_dialToneSound.readNormal(stream);
	_dialAgainSound.readNormal(stream);
	_hangUpSound.readNormal(stream);

	readFilenameArray(stream, _buttonSoundNames, 12);

	char textBuf[200];
	stream.read(textBuf, 200);
	textBuf[199] = '\0';
	_addressBookString = textBuf;
	stream.read(textBuf, 200);
	textBuf[199] = '\0';
	_dialAgainString = textBuf;
	_reloadScene.readData(stream);
	stream.skip(1);
	_exitScene.readData(stream);
	stream.skip(1);
	readRect(stream, _exitHotspot);

	uint numCalls = stream.readUint16LE();

	_calls.resize(numCalls);
	for (uint i = 0; i < numCalls; ++i) {
		PhoneCall &call = _calls[i];

		call.phoneNumber.resize(11);
		for (uint j = 0; j < 11; ++j) {
			call.phoneNumber[j] = stream.readByte();
		}

		readFilename(stream, call.soundName);
		stream.read(textBuf, 200);
		textBuf[199] = '\0';
		call.text = textBuf;
		call.sceneChange.readData(stream);
		stream.skip(1);
	}
}

void Telephone::execute() {
	switch (_state) {
	case kBegin:
		init();
		registerGraphics();
		g_nancy->_sound->loadSound(_dialToneSound);
		g_nancy->_sound->playSound(_dialToneSound);
		NancySceneState.getTextbox().clear();
		NancySceneState.getTextbox().addTextLine(_addressBookString);
		_state = kRun;
		// fall through
	case kRun:
		switch (_callState) {
		case kWaiting:
			if (_checkNumbers) {
				// Pressed a new button, check all numbers for match
				// We do this before going to the ringing state to support nancy4's voice mail system,
				// where call numbers can be 1 digit long
				for (uint i = 0; i < _calls.size(); ++i) {
					bool invalid = false;

					for (uint j = 0; j < _calledNumber.size(); ++j) {
						if (_calledNumber[j] != _calls[i].phoneNumber[j]) {
							// Invalid number, move onto next
							invalid = true;
							break;
						}
					}

					// We do not want to check for a terminator if the dialed number is of
					// appropriate size (7 digits, or 11 when the number starts with '1')
					bool checkNextDigit = true;
					if (_calledNumber.size() >= 11 || (_calledNumber.size() >= 7 && (_calledNumber[0] != 1))) {
						checkNextDigit = false;
					}

					if (!invalid && checkNextDigit) {
						// Check if the next digit in the phone number is '10' (star). Presumably, that will never
						// be contained in a valid phone number
						if (_calls[i].phoneNumber[_calledNumber.size()] != 10) {
							invalid = true;
						}
					}

					if (!invalid) {
						_selected = i;
						break;
					}
				}

				bool shouldRing = false;

				if (_selected == -1) {
					// Did not find a suitable match, check if the dialed number is above allowed size
					if (_calledNumber.size() >= 11 || (_calledNumber.size() >= 7 && (_calledNumber[0] != 1))) {
						shouldRing = true;
					}
				} else {
					shouldRing = true;
				}

				if (shouldRing) {
					if (_ringSound.name == "NO SOUND") {
						// Will not ring, so skip text
						_callState = kRinging;
					} else {
						NancySceneState.getTextbox().clear();
						NancySceneState.getTextbox().addTextLine(g_nancy->getStaticData().ringingText);
						g_nancy->_sound->loadSound(_ringSound);
						g_nancy->_sound->playSound(_ringSound);
						_callState = kRinging;
					}
				}

				_checkNumbers = false;
			}

			break;
		case kButtonPress:
			if (!g_nancy->_sound->isSoundPlaying(_genericButtonSound)) {
				g_nancy->_sound->stopSound(_genericButtonSound);
				undrawButton(_buttonLastPushed);
				_buttonLastPushed = -1;
				_callState = kWaiting;
			}

			break;
		case kRinging:
			if (!g_nancy->_sound->isSoundPlaying(_ringSound)) {
				g_nancy->_sound->stopSound(_ringSound);

				if (_selected != -1) {
					// Called a valid number
					NancySceneState.getTextbox().clear();
					NancySceneState.getTextbox().addTextLine(_calls[_selected].text);
					
					_genericDialogueSound.name = _calls[_selected].soundName;
					g_nancy->_sound->loadSound(_genericDialogueSound);
					g_nancy->_sound->playSound(_genericDialogueSound);
					_callState = kCall;
				} else {
					// Called an invalid number
					NancySceneState.getTextbox().clear();
					NancySceneState.getTextbox().addTextLine(_dialAgainString);

					g_nancy->_sound->loadSound(_dialAgainSound);
					g_nancy->_sound->playSound(_dialAgainSound);
					_callState = kBadNumber;
				}

				return;
			}

			break;
		case kBadNumber:
			if (!g_nancy->_sound->isSoundPlaying(_dialAgainSound)) {
				_state = kActionTrigger;
			}

			break;
		case kCall:
			if (!g_nancy->_sound->isSoundPlaying(_genericDialogueSound)) {
				_state = kActionTrigger;
			}

			break;
		case kHangUp:
			if (!g_nancy->_sound->isSoundPlaying(_hangUpSound)) {
				_state = kActionTrigger;
			}

			break;
		}

		break;
	case kActionTrigger:
		switch (_callState) {
		case kBadNumber:
			_reloadScene.execute();
			_calledNumber.clear();
			_state = kRun;
			_callState = kWaiting;

			break;
		case kCall: {
			PhoneCall &call = _calls[_selected];

			// Make sure we don't get stuck here. Happens in nancy3 when calling George's number
			// Check ignored in nancy1 since the HintSystem AR is in the same scene as the Telephone
			if (call.sceneChange._sceneChange.sceneID == kNoScene && g_nancy->getGameType() != kGameTypeNancy1) {
				call.sceneChange._sceneChange = NancySceneState.getSceneInfo();
			}

			call.sceneChange.execute();

			break;
		}
		case kHangUp:
			_exitScene.execute();

			break;
		default:
			break;
		}

				g_nancy->_sound->stopSound(_hangUpSound);
				g_nancy->_sound->stopSound(_genericDialogueSound);
				g_nancy->_sound->stopSound(_genericButtonSound);
				g_nancy->_sound->stopSound(_dialAgainSound);
				g_nancy->_sound->stopSound(_ringSound);
				g_nancy->_sound->stopSound(_dialToneSound);

		finishExecution();
	}
}

void Telephone::handleInput(NancyInput &input) {
	int buttonNr = -1;
	// Cursor gets changed regardless of state
	for (uint i = 0; i < 12; ++i) {
		if (NancySceneState.getViewport().convertViewportToScreen(_destRects[i]).contains(input.mousePos)) {
			g_nancy->_cursorManager->setCursorType(CursorManager::kHotspot);
			buttonNr = i;
			break;
		}
	}

	if (_callState != kWaiting && _callState != kRinging) {
		return;
	}

	if (NancySceneState.getViewport().convertViewportToScreen(_exitHotspot).contains(input.mousePos)) {
		g_nancy->_cursorManager->setCursorType(g_nancy->_cursorManager->_puzzleExitCursor);

		if (input.input & NancyInput::kLeftMouseButtonUp) {
			g_nancy->_sound->loadSound(_hangUpSound);
			g_nancy->_sound->playSound(_hangUpSound);

			_callState = kHangUp;
		}

		return;
	}

	if (_callState != kWaiting) {
		return;
	}

	if (buttonNr != -1) {
		if (input.input & NancyInput::kLeftMouseButtonUp) {
			if (g_nancy->_sound->isSoundPlaying(_dialToneSound)) {
				g_nancy->_sound->stopSound(_dialToneSound);
			}

			_calledNumber.push_back(buttonNr);
			_genericButtonSound.name = _buttonSoundNames[buttonNr];
			g_nancy->_sound->loadSound(_genericButtonSound);
			g_nancy->_sound->playSound(_genericButtonSound);

			drawButton(buttonNr);

			_buttonLastPushed = buttonNr;
			_checkNumbers = true;
			_callState = kButtonPress;
		}
	}
}

void Telephone::drawButton(uint id) {
	Common::Point destPoint(_destRects[id].left - _screenPosition.left, _destRects[id].top - _screenPosition.top);
	_drawSurface.blitFrom(_image, _srcRects[id], destPoint);

	_needsRedraw = true;
}

void Telephone::undrawButton(uint id) {
	Common::Rect bounds = _destRects[id];
	bounds.translate(-_screenPosition.left, -_screenPosition.top);

	_drawSurface.fillRect(bounds, g_nancy->_graphicsManager->getTransColor());
	_needsRedraw = true;
}

} // End of namespace Action
} // End of namespace Nancy
