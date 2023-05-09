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

#include "engines/nancy/action/telephone.h"

#include "engines/nancy/state/scene.h"

namespace Nancy {
namespace Action {

void Telephone::init() {
	_drawSurface.create(_screenPosition.width(), _screenPosition.height(), g_nancy->_graphicsManager->getInputPixelFormat());
	_drawSurface.clear(g_nancy->_graphicsManager->getTransColor());

	setTransparent(true);

	g_nancy->_resource->loadImage(_imageName, _image);

	NancySceneState.setShouldClearTextbox(false);
}

void Telephone::readData(Common::SeekableReadStream &stream) {
	readFilename(stream, _imageName);

	_srcRects.reserve(12);
	for (uint i = 0; i < 12; ++i) {
		_srcRects.push_back(Common::Rect());
		readRect(stream, _srcRects.back());
	}

	_destRects.reserve(12);
	for (uint i = 0; i < 12; ++i) {
		_destRects.push_back(Common::Rect());
		readRect(stream, _destRects.back());

		if (i == 0) {
			_screenPosition = _destRects.back();
		} else {
			_screenPosition.extend(_destRects.back());
		}
	}

	_genericDialogueSound.readNormal(stream);
	_genericButtonSound.readNormal(stream);
	_ringSound.readNormal(stream);
	_dialToneSound.readNormal(stream);
	_dialAgainSound.readNormal(stream);
	_hangUpSound.readNormal(stream);

	_buttonSoundNames.reserve(12);
	for (uint i = 0; i < 12; ++i) {
		Common::String buttonSoundName;
		readFilename(stream, buttonSoundName);
		_buttonSoundNames.push_back(buttonSoundName);
	}

	char textBuf[200];
	stream.read(textBuf, 200);
	textBuf[199] = '\0';
	_addressBookString = textBuf;
	stream.read(textBuf, 200);
	textBuf[199] = '\0';
	_dialAgainString = textBuf;
	_reloadScene.readData(stream);
	_exitScene.readData(stream);
	readRect(stream, _exitHotspot);

	uint numCalls = stream.readUint16LE();

	_calls.reserve(numCalls);
	for (uint i = 0; i < numCalls; ++i) {
		_calls.push_back(PhoneCall());
		PhoneCall &call = _calls.back();

		call.phoneNumber.reserve(11);
		for (uint j = 0; j < 11; ++j) {
			call.phoneNumber.push_back(stream.readByte());
		}

		readFilename(stream, call.soundName);
		stream.read(textBuf, 200);
		textBuf[199] = '\0';
		call.text = textBuf;
		call.sceneChange.readData(stream);
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
			// Long phone numbers start with 1
			if (_calledNumber.size() >= 11 || (_calledNumber.size() >= 7 && (_calledNumber[0] != 1))) {
				NancySceneState.getTextbox().clear();
				NancySceneState.getTextbox().addTextLine("ringing...<n><e>"); // Hardcoded in the original engine
				g_nancy->_sound->loadSound(_ringSound);
				g_nancy->_sound->playSound(_ringSound);
				_callState = kRinging;
			}

			break;
		case kButtonPress:
			if (!g_nancy->_sound->isSoundPlaying(_genericButtonSound)) {
				g_nancy->_sound->stopSound(_genericButtonSound);
				undrawButton(_selected);
				_callState = kWaiting;
			}

			break;
		case kRinging:
			if (!g_nancy->_sound->isSoundPlaying(_ringSound)) {
				g_nancy->_sound->stopSound(_ringSound);
				uint numberLength = _calledNumber[0] == 1 ? 11 : 7;

				for (uint i = 0; i < _calls.size(); ++i) {
					bool invalid = false;

					for (uint j = 0; j < numberLength; ++j) {
						if (_calledNumber[j] != _calls[i].phoneNumber[j]) {
							// Invalid number, move onto next
							invalid = true;
							break;
						}
					}

					if (invalid) {
						continue;
					}

					NancySceneState.getTextbox().clear();
					NancySceneState.getTextbox().addTextLine(_calls[i].text);

					_genericDialogueSound.name = _calls[i].soundName;
					g_nancy->_sound->loadSound(_genericDialogueSound);
					g_nancy->_sound->playSound(_genericDialogueSound);
					_selected = i;
					_callState = kCall;

					return;
				}

				NancySceneState.getTextbox().clear();
				NancySceneState.getTextbox().addTextLine(_dialAgainString);

				g_nancy->_sound->loadSound(_dialAgainSound);
				g_nancy->_sound->playSound(_dialAgainSound);
				_callState = kBadNumber;
				return;
			}

			break;
		case kBadNumber:
			if (!g_nancy->_sound->isSoundPlaying(_dialAgainSound)) {
				g_nancy->_sound->stopSound(_dialAgainSound);

				_state = kActionTrigger;
			}

			break;
		case kCall:
			if (!g_nancy->_sound->isSoundPlaying(_genericDialogueSound)) {
				g_nancy->_sound->stopSound(_genericDialogueSound);

				_state = kActionTrigger;
			}

			break;
		case kHangUp:
			if (!g_nancy->_sound->isSoundPlaying(_hangUpSound)) {
				g_nancy->_sound->stopSound(_hangUpSound);

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
			call.sceneChange.execute();

			break;
		}
		case kHangUp:
			_exitScene.execute();

			break;
		default:
			break;
		}

		finishExecution();
		NancySceneState.setShouldClearTextbox(true);
		NancySceneState.getTextbox().clear();
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

	if (_callState != kWaiting) {
		return;
	}

	if (NancySceneState.getViewport().convertViewportToScreen(_exitHotspot).contains(input.mousePos)) {
		g_nancy->_cursorManager->setCursorType(CursorManager::kExit);

		if (input.input & NancyInput::kLeftMouseButtonUp) {
			g_nancy->_sound->loadSound(_hangUpSound);
			g_nancy->_sound->playSound(_hangUpSound);

			_callState = kHangUp;
		}

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

			_selected = buttonNr;

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
