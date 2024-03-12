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
#include "engines/nancy/font.h"

#include "engines/nancy/action/puzzle/telephone.h"

#include "engines/nancy/state/scene.h"

namespace Nancy {
namespace Action {

void Telephone::init() {
	Common::Rect screenBounds = NancySceneState.getViewport().getBounds();
	_drawSurface.create(screenBounds.width(), screenBounds.height(), g_nancy->_graphics->getInputPixelFormat());
	_drawSurface.clear(g_nancy->_graphics->getTransColor());
	setTransparent(true);
	setVisible(true);
	moveTo(screenBounds);

	g_nancy->_resource->loadImage(_imageName, _image);
	g_nancy->_resource->loadImage(_displayAnimName, _animImage);

	if (_isNewPhone) {
		_font = g_nancy->_graphics->getFont(_displayFont);
	}
}

void Telephone::readData(Common::SeekableReadStream &stream) {
	readFilename(stream, _imageName);

	uint16 numButtons = 12;
	uint16 maxNumButtons = _isNewPhone ? 20 : 12;

	if (_isNewPhone) {
		_hasDisplay = stream.readByte();
		_displayFont = stream.readUint16LE();
		readFilename(stream, _displayAnimName);
		_displayAnimFrameTime = stream.readUint32LE();
		uint16 numFrames = stream.readUint16LE();
		readRectArray(stream, _displaySrcs, numFrames, 10);
		readRect(stream, _displayDest);
		_dialAutomatically = stream.readByte();

		numButtons = stream.readUint16LE();
	}

	readRectArray(stream, _srcRects, numButtons, maxNumButtons);
	readRectArray(stream, _destRects, numButtons, maxNumButtons);

	if (_isNewPhone) {
		readRect(stream, _dirHighlightSrc);
		readRect(stream, _dialHighlightSrc);

		_upDirButtonID = stream.readUint16LE();
		_downDirButtonID = stream.readUint16LE();
		_dialButtonID = stream.readUint16LE();
		_dirButtonID = stream.readUint16LE();

		readRect(stream, _displayDialingSrc);
	}

	if (!_isNewPhone) {
		_genericDialogueSound.readNormal(stream);
		_genericButtonSound.readNormal(stream);
		_ringSound.readNormal(stream);
		_dialToneSound.readNormal(stream);
		_dialAgainSound.readNormal(stream);
		_hangUpSound.readNormal(stream);
	} else {
		_ringSound.readNormal(stream);
		_dialToneSound.readNormal(stream);
		_preCallSound.readNormal(stream);
		_hangUpSound.readNormal(stream);
		_genericButtonSound.readNormal(stream);
	}

	readFilenameArray(stream, _buttonSoundNames, numButtons);
	stream.skip(33 * (maxNumButtons - numButtons));

	char textBuf[200];
	if (!_isNewPhone) {
		stream.read(textBuf, 200);
		textBuf[199] = '\0';
		_addressBookString = textBuf;
	} else {
		_dialAgainSound.readNormal(stream);
	}

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

		if (_isNewPhone) {
			call.eventFlagCondition = stream.readSint16LE();
		}

		call.phoneNumber.resize(11);
		for (uint j = 0; j < 11; ++j) {
			call.phoneNumber[j] = stream.readByte();
		}

		if (!_isNewPhone) {
			readFilename(stream, call.soundName);
			stream.read(textBuf, 200);
			textBuf[199] = '\0';
			call.text = textBuf;
		} else {
			readRect(stream, call.displaySrc);
		}

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
			if (_isNewPhone && !_animIsStopped) {
				if (g_nancy->getTotalPlayTime() > _displayAnimEnd) {
					if (_displayAnimEnd == 0) {
						_displayAnimEnd = g_nancy->getTotalPlayTime() + _displayAnimFrameTime;
					} else {
						_displayAnimEnd += _displayAnimFrameTime;
					}

					_drawSurface.blitFrom(_animImage, _displaySrcs[_displayAnimFrame], _displayDest);
					_needsRedraw = true;
					++_displayAnimFrame;

					if (_displayAnimFrame >= _displaySrcs.size()) {
						_displayAnimFrame = 0;
					}
				}
			}

			if (_checkNumbers) {
				// Pressed a new button, check all numbers for match
				// We do this before going to the ringing state to support nancy4's voice mail system,
				// where call numbers can be 1 digit long
				for (uint i = 0; i < _calls.size(); ++i) {
					// Do not evaluate phone calls whose condition isn't met
					if (_calls[i].eventFlagCondition != kEvNoEvent) {
						if (NancySceneState.getEventFlag(_calls[i].eventFlagCondition, g_nancy->_false)) {
							continue;
						}
					}

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
					if (_ringSound.name != "NO SOUND") {
						if (_hasDisplay) {
							_drawSurface.blitFrom(_image, _displayDialingSrc, _displayDest);
							_needsRedraw = true;
						} else {
							NancySceneState.getTextbox().clear();
							NancySceneState.getTextbox().addTextLine(g_nancy->getStaticData().ringingText);
						}

						g_nancy->_sound->loadSound(_ringSound);
						g_nancy->_sound->playSound(_ringSound);
					}

					_callState = kRinging;
				}

				_checkNumbers = false;
			}

			break;
		case kButtonPress:
			if (!g_nancy->_sound->isSoundPlaying(_genericButtonSound)) {
				g_nancy->_sound->stopSound(_genericButtonSound);
				_drawSurface.fillRect(_destRects[_buttonLastPushed], g_nancy->_graphics->getTransColor());
				_needsRedraw = true;

				if (_isShowingDirectory) {
					_drawSurface.blitFrom(_image, _dirHighlightSrc, _destRects[_dirButtonID]);
					_drawSurface.blitFrom(_image, _calls[_displayedDirectory].displaySrc, _displayDest);
				} else if (_dirButtonID != -1) {
					_drawSurface.fillRect(_destRects[_dirButtonID], _drawSurface.getTransparentColor());
				}

				_buttonLastPushed = -1;
				_callState = kWaiting;
			}

			break;
		case kRinging:
			if (!g_nancy->_sound->isSoundPlaying(_ringSound)) {
				g_nancy->_sound->stopSound(_ringSound);

				if (_selected != -1) {
					// Called a valid number

					if (_preCallSound.name == "NO SOUND") {
						// Old phone, go directly to call
						NancySceneState.getTextbox().clear();
						NancySceneState.getTextbox().addTextLine(_calls[_selected].text);

						_genericDialogueSound.name = _calls[_selected].soundName;
						g_nancy->_sound->loadSound(_genericDialogueSound);
						g_nancy->_sound->playSound(_genericDialogueSound);
						_callState = kCall;
					} else {
						// New phone, play a short sound of phone being picked up
						g_nancy->_sound->loadSound(_preCallSound);
						g_nancy->_sound->playSound(_preCallSound);
						_callState = kPreCall;
					}
				} else {
					// Called an invalid number
					NancySceneState.getTextbox().clear();
					NancySceneState.getTextbox().addTextLine(_dialAgainString);

					if (_hasDisplay) {
						_drawSurface.fillRect(_displayDest, _drawSurface.getTransparentColor());
						_needsRedraw = true;
					}

					if (_dialButtonID != -1) {
						_drawSurface.fillRect(_destRects[_dialButtonID], _drawSurface.getTransparentColor());
						_needsRedraw = true;
					}

					_calledNumber.clear();

					g_nancy->_sound->loadSound(_dialAgainSound);
					g_nancy->_sound->playSound(_dialAgainSound);
					_callState = kBadNumber;
				}

				return;
			}

			break;
		case kPreCall:
			if (!g_nancy->_sound->isSoundPlaying(_preCallSound)) {
				g_nancy->_sound->stopSound(_preCallSound);

				if (!_calls[_selected].text.empty()) {
					NancySceneState.getTextbox().clear();
					NancySceneState.getTextbox().addTextLine(_calls[_selected].text);
				}

				_genericDialogueSound.name = _calls[_selected].soundName;
				g_nancy->_sound->loadSound(_genericDialogueSound);
				g_nancy->_sound->playSound(_genericDialogueSound);
				_callState = kCall;
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
	for (int i = 0; i < (int)_destRects.size(); ++i) {
		// Dial button is an exception
		if (i == _dialButtonID && !_calledNumber.size() && !_isShowingDirectory) {
			continue;
		}

		if (NancySceneState.getViewport().convertViewportToScreen(_destRects[i]).contains(input.mousePos)) {
			g_nancy->_cursor->setCursorType(CursorManager::kHotspot);
			buttonNr = i;
			break;
		}
	}

	if (_callState != kWaiting && _callState != kRinging) {
		return;
	}

	if (NancySceneState.getViewport().convertViewportToScreen(_exitHotspot).contains(input.mousePos)) {
		g_nancy->_cursor->setCursorType(g_nancy->_cursor->_puzzleExitCursor);

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

			// Handle non-digit numbers
			bool directorySwitch = false;
			bool changeDirectoryEntry = false;
			int dirEntryDelta = 1;
			if (_dialButtonID != -1 && buttonNr == _dialButtonID) {
				_calledNumber = _calls[_displayedDirectory].phoneNumber;
				while (_calledNumber.back() == 10) {
					_calledNumber.pop_back();
				}

				_checkNumbers = true;

				// Dial button doesn't make sound, and doesn't get pressed down
				_drawSurface.blitFrom(_image, _dialHighlightSrc, _destRects[_dialButtonID]);

				if (_dirButtonID != -1) {
					_drawSurface.fillRect(_destRects[_dirButtonID], _drawSurface.getTransparentColor());
				}

				return;
			} else if (_upDirButtonID != -1 && buttonNr == _upDirButtonID) {
				if (!_isShowingDirectory) {
					directorySwitch = true;
				} else {
					++_displayedDirectory;
					changeDirectoryEntry = true;
				}
			} else if (_downDirButtonID != -1 && buttonNr == _downDirButtonID) {
				if (!_isShowingDirectory) {
					directorySwitch = true;
				} else {
					--_displayedDirectory;
					dirEntryDelta = -1;
					changeDirectoryEntry = true;
				}
			} else if (_dirButtonID != -1 && buttonNr == _dirButtonID) {
				if (!_isShowingDirectory) {
					directorySwitch = true;
				}
			} else {
				if (_isShowingDirectory || !_calledNumber.size()) {
					_isShowingDirectory = false;
					_displayedDirectory = 0;
					_drawSurface.fillRect(_displayDest, _drawSurface.getTransparentColor());
				}

				_calledNumber.push_back(buttonNr);
				_checkNumbers = _dialAutomatically;
				_animIsStopped = true;

				if (_calledNumber.size() > 11) {
					_calledNumber.clear();

					if (_hasDisplay) {
						_drawSurface.fillRect(_displayDest, _drawSurface.getTransparentColor());
					} else if (_isNewPhone) {
						NancySceneState.getTextbox().clear();
					}

					_checkNumbers = false;
				}

				if (_isNewPhone && _calledNumber.size()) {
					Common::String numberString;
					for (uint j = 0; j < _calledNumber.size(); ++j) {
						numberString += '0' + _calledNumber[j];
					}

					if (_hasDisplay) {
						_font->drawString(&_drawSurface, numberString, _displayDest.left + 19, _displayDest.top + 21 - _font->getFontHeight(),
							_displayDest.width() - 20, 0);
					} else {
						NancySceneState.getTextbox().clear();
						NancySceneState.getTextbox().addTextLine(numberString);
					}
				}
			}

			if (directorySwitch) {
				// Handle switch to directory mode
				_isShowingDirectory = true;
				changeDirectoryEntry = true;
				_calledNumber.clear();
			}

			if (changeDirectoryEntry) {
				int start = _displayedDirectory;

				do {
					if (_displayedDirectory >= (int)_calls.size()) {
						_displayedDirectory = 0;
					} else if (_displayedDirectory < 0) {
						_displayedDirectory = _calls.size() - 1;
					}

					if (_calls[_displayedDirectory].eventFlagCondition == kEvNoEvent) {
						break;
					}

					if (NancySceneState.getEventFlag(_calls[_displayedDirectory].eventFlagCondition, g_nancy->_true)) {
						break;
					}

					_displayedDirectory += dirEntryDelta;
				} while (_displayedDirectory != start);
			}

			_genericButtonSound.name = _buttonSoundNames[buttonNr];
			g_nancy->_sound->loadSound(_genericButtonSound);
			g_nancy->_sound->playSound(_genericButtonSound);

			_drawSurface.blitFrom(_image, _srcRects[buttonNr], _destRects[buttonNr]);
			_needsRedraw = true;

			_displayAnimEnd = 0;
			_displayAnimFrame = 0;

			_buttonLastPushed = buttonNr;
			_callState = kButtonPress;
		}
	}
}

} // End of namespace Action
} // End of namespace Nancy
