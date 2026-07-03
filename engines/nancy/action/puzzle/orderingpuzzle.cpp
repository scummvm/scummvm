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

#include "common/serializer.h"

#include "engines/nancy/nancy.h"
#include "engines/nancy/graphics.h"
#include "engines/nancy/resource.h"
#include "engines/nancy/input.h"
#include "engines/nancy/sound.h"
#include "engines/nancy/util.h"

#include "engines/nancy/action/puzzle/orderingpuzzle.h"

#include "engines/nancy/state/scene.h"

namespace Nancy {
namespace Action {

void OrderingPuzzle::init() {
	for (uint i = 0; i < _destRects.size(); ++i) {
		if (i == 0) {
			_screenPosition = _destRects[i];
		} else {
			_screenPosition.extend(_destRects[i]);
		}
	}

	for (uint i = 0; i < _overlayDests.size(); ++i) {
		_screenPosition.extend(_overlayDests[i]);
	}

	if (!_checkButtonDest.isEmpty()) {
		_screenPosition.extend(_checkButtonDest);
	}

	g_nancy->_resource->loadImage(_imageName, _image);
	_drawSurface.create(_screenPosition.width(), _screenPosition.height(), g_nancy->_graphics->getInputPixelFormat());

	if (_image.hasPalette()) {
		uint8 palette[256 * 3];
		_image.grabPalette(palette, 0, 256);
		_drawSurface.setPalette(palette, 0, 256);
	}

	setTransparent(true);
	_drawSurface.clear(_drawSurface.getTransparentColor());
	setVisible(true);

	RenderObject::init();
}

void OrderingPuzzle::readData(Common::SeekableReadStream &stream) {
	bool isPiano = _puzzleType == kPiano;
	bool isOrderItems = _puzzleType == kOrderItems;
	bool isKeypad = _puzzleType == kKeypad || _puzzleType == kKeypadTerse;
	readFilename(stream, _imageName);
	Common::Serializer ser(&stream, nullptr);
	ser.setVersion(g_nancy->getGameType());

	uint16 numElements = 5;
	uint16 maxNumElements = 15;
	if (ser.getVersion() == kGameTypeVampire) {
		// Hardcoded in The Vampire Diaries
		numElements = maxNumElements = 5;
	} else {
		ser.syncAsUint16LE(numElements);
	}

	// Nancy 11 bumped the base ordering puzzle's element cap from 15 to 16
	if (_puzzleType == kOrdering && g_nancy->getGameType() >= kGameTypeNancy11) {
		maxNumElements = 16;
	}

	switch (_puzzleType) {
	case kOrderItems :
		ser.syncAsByte(_hasSecondState);
		ser.syncAsByte(_itemsStayDown);
		break;
	case kPiano :
		_itemsStayDown = false;
		break;
	case kKeypadTerse:
		// fall through
	case kKeypad :
		ser.syncAsByte(_itemsStayDown);
		ser.syncAsByte(_needButtonToCheckSuccess);
		readRect(ser, _checkButtonSrc);
		readRect(ser, _checkButtonDest);
		maxNumElements = 30;
		break;
	default:
		break;
	}

	// nancy7 moved the keypad rects at the end
	if (g_nancy->getGameType() <= kGameTypeNancy6 || !isKeypad) {
		readRectArray(ser, _down1Rects, numElements, maxNumElements);

		if (isOrderItems) {
			readRectArray(stream, _up2Rects, numElements, maxNumElements);
			readRectArray(stream, _down2Rects, numElements, maxNumElements);
		}

		readRectArray(ser, _destRects, numElements, maxNumElements);

		if (isPiano) {
			readRectArray(stream, _hotspots, numElements, maxNumElements);
		} else {
			_hotspots = _destRects;
		}
	}

	if (isPiano && g_nancy->getGameType() >= kGameTypeNancy8) {
		_specialCursor1Id = stream.readUint16LE();
		readRect(stream, _specialCursor1Dest);
		_specialCursor2Id = stream.readUint16LE();
		readRect(stream, _specialCursor2Dest);
	}

	uint sequenceLength = 5;
	ser.syncAsUint16LE(sequenceLength, kGameTypeNancy1);

	if (isKeypad) {
		ser.syncAsByte(_checkOrder, kGameTypeNancy7);
	}

	_correctSequence.resize(sequenceLength);
	uint sizeElem = 1;
	for (uint i = 0; i < sequenceLength; ++i) {
		switch (_puzzleType) {
		case kKeypadTerse:
			// fall through
		case kKeypad :
			// fall through
		case kOrdering:
			ser.syncAsByte(_correctSequence[i]);
			sizeElem = 1;
			break;
		case kPiano:
			ser.syncAsUint16LE(_correctSequence[i]);
			sizeElem = 2;
			break;
		case kOrderItems:
			// For some reason, OrderItems labels starting from 1
			ser.syncAsUint16LE(_correctSequence[i]);
			--_correctSequence[i];
			sizeElem = 2;
			break;
		default:
			error("OrderingPuzzle::readData(): Unsupported puzzle type %d", _puzzleType);
		}
	}
	ser.skip((maxNumElements - sequenceLength) * sizeElem, kGameTypeNancy1);

	if (isOrderItems) {
		uint numOverlays = 0;
		ser.syncAsUint16LE(_state2InvItem);
		ser.syncAsUint16LE(numOverlays);

		readRectArray(ser, _overlaySrcs, numOverlays);
		readRectArray(ser, _overlayDests, numOverlays);
	} else if (isPiano && g_nancy->getGameType() >= kGameTypeNancy8) {
		if (g_nancy->getGameType() >= kGameTypeNancy11) {
			// Nancy 11 stores two interleaved sound names per key (e.g. a press and
			// a release sound), padded to maxNumElements pairs
			_pianoSoundNames.resize(numElements);
			_pianoReleaseSoundNames.resize(numElements);
			for (uint i = 0; i < numElements; ++i) {
				readFilename(stream, _pianoSoundNames[i]);
				readFilename(stream, _pianoReleaseSoundNames[i]);
			}
			stream.skip((maxNumElements - numElements) * 2 * 33);
		} else {
			readFilenameArray(stream, _pianoSoundNames, numElements);
			stream.skip((maxNumElements - numElements) * 33);
		}
	}

	if (ser.getVersion() > kGameTypeVampire) {
		_pushDownSound.readNormal(stream);

		if (isOrderItems) {
			_itemSound.readNormal(stream);
			_popUpSound.readNormal(stream);
		}
	}

	if (ser.getVersion() == kGameTypeVampire) {
		_solveExitScene._sceneChange.readData(stream, true);
		ser.skip(2); // shouldStopRendering
		ser.syncAsSint16LE(_solveExitScene._flag.label);
		ser.syncAsByte(_solveExitScene._flag.flag);
	} else {
		_solveExitScene.readData(stream);
	}

	ser.syncAsUint16LE(_solveSoundDelay);
	_solveSound.readNormal(stream);

	if (ser.getVersion() == kGameTypeVampire) {
		_exitScene._sceneChange.readData(stream, true);
		ser.skip(2); // shouldStopRendering
		ser.syncAsSint16LE(_exitScene._flag.label);
		ser.syncAsByte(_exitScene._flag.flag);
	} else {
		_exitScene.readData(stream);
	}

	readRect(stream, _exitHotspot);

	if (isKeypad && g_nancy->getGameType() >= kGameTypeNancy7) {
		if (_puzzleType == kKeypad) {
			if (g_nancy->getGameType() >= kGameTypeNancy12) {
				// Nancy 12 reworked the keypad block: grid dimensions and the per-stage
				// codes (the win still uses the sequence read above), then three button
				// rect arrays - sprite source rects, on-screen dest positions, and tighter
				// per-button hotspots. The dest rects double as the hotspots here.
				stream.skip(0x309 - 0x10c); // grid + per-stage codes (not modeled)
				readRectArray(stream, _down1Rects, numElements, 30); // button sprite source rects
				readRectArray(stream, _destRects, numElements, 30);  // on-screen button positions
				stream.skip(30 * 16);       // tighter per-button hotspots (we use _destRects)
			} else {
				if (g_nancy->getGameType() >= kGameTypeNancy11) {
					// Nancy 11 multi-stage keypad: a stage count, per-stage display rects, the codes
					// for stages 1+, a final code matrix and an alternate scene, then the button rects.
					_numStages = stream.readUint16LE();
					stream.skip(20 * 16 + 1); // per-stage display rects + blink flag

					_stageSequences.resize(4);
					_stageCheckOrder.resize(4);
					for (uint s = 0; s < 4; ++s) {
						uint16 len = stream.readUint16LE();
						_stageCheckOrder[s] = (stream.readByte() != 0);
						len = MIN<uint16>(len, 30);
						_stageSequences[s].resize(len);
						for (uint16 k = 0; k < len; ++k)
							_stageSequences[s][k] = stream.readByte();
						stream.skip(30 - len);
					}

					stream.skip(25 + 25); // final code matrix + alternate scene (unused by the sequential model)
				}
				readRectArray(ser, _down1Rects, numElements, maxNumElements);
				readRectArray(ser, _destRects, numElements, maxNumElements);
			}
		} else if (_puzzleType == kKeypadTerse) {
			// Terse elements are the same size & placed on a grid (in the source image AND on screen)
			uint16 columns = stream.readUint16LE();

			// The Nancy 11 multi-stage terse keypad chains scenes: each scene holds one stage and
			// the grid is preceded by the scene to advance to once this stage's code is entered.
			// Standalone/final keypads omit it. A grid column count can never exceed maxNumElements,
			// so a larger leading value is that scene id - it overrides the solve scene's target
			// (the rest of the solve scene change is reused), then the real column count follows.
			if (g_nancy->getGameType() >= kGameTypeNancy11 && columns > maxNumElements) {
				_solveExitScene._sceneChange.sceneID = columns;
				columns = stream.readUint16LE();
			}

			uint16 rows = stream.readUint16LE();

			uint16 width = stream.readUint16LE();
			uint16 height = stream.readUint16LE();

			// Nancy 11 stores the element count as 0 and derives it from the grid dimensions
			if (g_nancy->getGameType() >= kGameTypeNancy11) {
				numElements = columns * rows;
			}

			// Guard against malformed dimensions (columns is a divisor below)
			if (columns == 0) {
				columns = 1;
			}
			numElements = MIN<uint16>(numElements, maxNumElements);

			_down1Rects.resize(numElements);
			_destRects.resize(numElements);

			Common::Point srcStartPos, srcDist, destStartPos, destDist;

			srcStartPos.x = stream.readUint16LE();
			srcStartPos.y = stream.readUint16LE();
			srcDist.x = stream.readUint16LE();
			srcDist.y = stream.readUint16LE();

			destStartPos.x = stream.readUint16LE();
			destStartPos.y = stream.readUint16LE();
			destDist.x = stream.readUint16LE();
			destDist.y = stream.readUint16LE();

			for (uint i = 0; i < numElements; ++i) {
				uint x = i % columns;
				uint y = i / columns;
				Common::Rect &src = _down1Rects[i];
				src.left	= srcStartPos.x + (x * srcDist.x) + (width * x);
				src.top		= srcStartPos.y + (y * srcDist.y) + (height * y);
				src.setWidth(width + 1);
				src.setHeight(height + 1);

				Common::Rect &dest = _destRects[i];
				dest.left	= destStartPos.x + (x * destDist.x) + (width * x);
				dest.top	= destStartPos.y + (y * destDist.y) + (height * y);
				dest.setWidth(width + 1);
				dest.setHeight(height + 1);
			}
		}

		if (g_nancy->getGameType() >= kGameTypeNancy12 && _puzzleType == kKeypadTerse) {
			// Nancy 12 keypad-terse grew by 4 bytes (exact layout not yet mapped).
			stream.skip(4);
		}

		_hotspots = _destRects;
	}

	_downItems.resize(numElements, false);
	_secondStateItems.resize(numElements, false);
}

void OrderingPuzzle::execute() {
	switch (_state) {
	case kBegin:
		init();
		registerGraphics();
		if (g_nancy->getGameType() > kGameTypeVampire) {
			g_nancy->_sound->loadSound(_pushDownSound);
			if (_puzzleType == kOrderItems) {
				g_nancy->_sound->loadSound(_itemSound);
				g_nancy->_sound->loadSound(_popUpSound);
			}
		}

		NancySceneState.setNoHeldItem();

		_state = kRun;
		// fall through
	case kRun:
		switch (_solveState) {
		case kNotSolved: {
			if (!_itemsStayDown) {
				// Clear the pushed item
				if (g_nancy->_sound->isSoundPlaying(_pushDownSound)) {
					return;
				}

				for (uint i = 0; i < _downItems.size(); ++i) {
					if (_downItems[i]) {
						popUp(i);
					}
				}
			}

			bool solved = true;

			if (_puzzleType != kPiano) {
				if (_clickedSequence.size() >= _correctSequence.size()) {
					bool equal = true;
					if (_checkOrder) {
						equal = (_clickedSequence == _correctSequence);
					} else {
						for (uint i = 0; i < _correctSequence.size(); ++i) {
							bool found = false;
							for (uint j = 0; j < _clickedSequence.size(); ++j) {
								if (_correctSequence[i] == _clickedSequence[j]) {
									found = true;
									break;
								}
							}

							if (!found) {
								// Couldn't find one of the items in the correct sequence
								equal = false;
								break;
							}
						}
					}

					// Check the pressed sequence. If its length is above a certain number,
					// clear it and start anew
					if (!equal) {
						if (_puzzleType != kOrderItems) {
							uint maxNumPressed = 4;
							if (g_nancy->getGameType() > kGameTypeVampire) {
								if (_puzzleType == kKeypad || _puzzleType == kKeypadTerse) {
									maxNumPressed = _correctSequence.size();
								} else {
									maxNumPressed = _correctSequence.size() + 1;
								}
							}

							if (_clickedSequence.size() > maxNumPressed) {
								clearAllElements();
								return;
							}

							// In the clam puzzle with flags in Nancy9, test for the exact
							// number of flag buttons pressed. Keeping this with a game
							// version check for now, to avoid regressions in other puzzle
							// types in other games.
							// TODO: Merge with the above check once all the puzzle types
							// have been retested
							if (g_nancy->getGameType() == kGameTypeNancy9 && _puzzleType == kOrdering) {
								if (_clickedSequence.size() >= _correctSequence.size()) {
									clearAllElements();
									return;
								}
							}
						} else {
							// OrderItems has a slight delay, after which it actually clears
							if (_clickedSequence.size() == _correctSequence.size()) {
								if (_solveSoundPlayTime == 0) {
									_solveSoundPlayTime = g_nancy->getTotalPlayTime() + 500;
								} else {
									if (g_nancy->getTotalPlayTime() > _solveSoundPlayTime) {
										clearAllElements();
										_solveSoundPlayTime = 0;
										return;
									}
								}
							}
						}

						solved = false;
					}
				} else {
					solved = false;
				}
			} else {
				// Piano puzzle checks only the last few elements
				if (_clickedSequence.size() < _correctSequence.size()) {
					return;
				}

				// Arbitrary number
				if (_clickedSequence.size() > 30) {
					_clickedSequence.erase(&_clickedSequence[0], &_clickedSequence[_clickedSequence.size() - _correctSequence.size()]);
				}

				for (uint i = 0; i < _correctSequence.size(); ++i) {
					if (_clickedSequence[_clickedSequence.size() - _correctSequence.size() + i] != (int16)_correctSequence[i]) {
						return;
					}
				}
			}

			if (_puzzleType == kKeypad && _numStages > 1) {
				// Nancy 11 multi-stage keypad: every stage's code must be entered in turn.
				// The check button confirms the current code; a wrong code resets the stage.
				if (!_checkButtonPressed) {
					return;
				}

				if (g_nancy->_sound->isSoundPlaying(_pushDownSound)) {
					return;
				}

				_checkButtonPressed = false;

				if (!solved) {
					clearAllElements();
					return;
				}

				if (_currentStage + 1 < (int)_numStages) {
					++_currentStage;
					_correctSequence = _stageSequences[_currentStage - 1];
					_checkOrder = _stageCheckOrder[_currentStage - 1];
					clearAllElements();
					return;
				}

				// Final stage solved
				NancySceneState.setEventFlag(_solveExitScene._flag);
				_currentStage = 0;
			} else if (_puzzleType == kKeypad && _needButtonToCheckSuccess) {
				// KeypadPuzzle moves to the "success" scene regardless whether the puzzle was solved or not,
				// provided the check button is pressed.
				if (_checkButtonPressed) {
					if (!g_nancy->_sound->isSoundPlaying(_pushDownSound)) {
						if (solved) {
							NancySceneState.setEventFlag(_solveExitScene._flag);
						}
					} else {
						return;
					}
				} else {
					return;
				}
			} else {
				if (solved) {
					if (_puzzleType == kOrderItems) {
						if (!g_nancy->_sound->isSoundPlaying(_pushDownSound)) {
							// Draw some overlays when solved correctly (OrderItems only)
							for (uint i = 0; i < _overlaySrcs.size(); ++i) {
								Common::Rect destRect = _overlayDests[i];
								destRect.translate(-_screenPosition.left, -_screenPosition.top);

								_drawSurface.blitFrom(_image, _overlaySrcs[i], destRect);
								_needsRedraw = true;
							}
						} else {
							return;
						}
					}

					NancySceneState.setEventFlag(_solveExitScene._flag);
				} else {
					return;
				}
			}

			_solveSoundPlayTime = g_nancy->getTotalPlayTime() + _solveSoundDelay * 1000;
			_solveState = kPlaySound;
		}
			// fall through
		case kPlaySound:
			if (g_nancy->getTotalPlayTime() <= _solveSoundPlayTime) {
				break;
			}

			g_nancy->_sound->loadSound(_solveSound);
			g_nancy->_sound->playSound(_solveSound);
			_solveState = kWaitForSound;
			break;
		case kWaitForSound:
			if (!g_nancy->_sound->isSoundPlaying(_solveSound)) {
				_state = kActionTrigger;
			}

			break;
		}

		break;
	case kActionTrigger:
		if (g_nancy->getGameType() == kGameTypeVampire) {
			g_nancy->_sound->stopSound("BUOK");
		} else {
			g_nancy->_sound->stopSound(_pushDownSound);
		}

		g_nancy->_sound->stopSound(_solveSound);

		if (_solveState == kNotSolved) {
			_exitScene.execute();
		} else {
			NancySceneState.changeScene(_solveExitScene._sceneChange);
		}

		finishExecution();
		break;
	}
}

void OrderingPuzzle::handleInput(NancyInput &input) {
	if (_solveState != kNotSolved) {
		return;
	}

	bool canClick = true;
	if (_itemsStayDown && g_nancy->_sound->isSoundPlaying(_pushDownSound)) {
		canClick = false;
	} else if (_puzzleType == kPiano) {
		for (uint i = 0; i < _hotspots.size(); ++i) {
			if (_downItems[i]) {
				canClick = false;
				break;
			}
		}
	}

	if (NancySceneState.getViewport().convertViewportToScreen(_exitHotspot).contains(input.mousePos)) {
		g_nancy->_cursor->setCursorType(g_nancy->_cursor->_puzzleExitCursor);

		if (canClick && input.input & NancyInput::kLeftMouseButtonUp) {
			_state = kActionTrigger;
		}
		return;
	}

	if (_needButtonToCheckSuccess && NancySceneState.getViewport().convertViewportToScreen(_checkButtonDest).contains(input.mousePos)) {
		g_nancy->_cursor->setCursorType(CursorManager::kHotspot);

		if (canClick && input.input & NancyInput::kLeftMouseButtonUp) {
			_checkButtonPressed = true;
			g_nancy->_sound->loadSound(_pushDownSound);
			g_nancy->_sound->playSound(_pushDownSound);
			Common::Rect destRect = _checkButtonDest;
			destRect.translate(-_screenPosition.left, -_screenPosition.top);

			_drawSurface.blitFrom(_image, _checkButtonSrc, destRect);
			_needsRedraw = true;
		}
	}

	for (int i = 0; i < (int)_hotspots.size(); ++i) {
		if (NancySceneState.getViewport().convertViewportToScreen(_hotspots[i]).contains(input.mousePos)) {
			// Set the custom cursor for nancy8+ PianoPuzzle
			if (NancySceneState.getViewport().convertViewportToScreen(_specialCursor1Dest).contains(input.mousePos)) {
				g_nancy->_cursor->setCursorType((CursorManager::CursorType)_specialCursor1Id, true);
			} else if (NancySceneState.getViewport().convertViewportToScreen(_specialCursor2Dest).contains(input.mousePos)) {
				g_nancy->_cursor->setCursorType((CursorManager::CursorType)_specialCursor2Id, true);
			} else {
				g_nancy->_cursor->setCursorType(CursorManager::kHotspot);
			}

			if (canClick && input.input & NancyInput::kLeftMouseButtonUp) {
				if (_puzzleType == kOrderItems) {
					if (_itemsStayDown && _downItems[i]) {
						// Button is pressed, OrderItems does not allow for depressing
						return;
					}

					if (NancySceneState.getHeldItem() == _state2InvItem) {
						// We are holding the correct inventory, set the button to its alternate (dusted) state
						setToSecondState(i);
						return;
					}
				}

				if (_puzzleType == kPiano) {
					// Set the correct sound name for every piano key
					if (g_nancy->getGameType() <= kGameTypeNancy7) {
						// In earlier games, the sound name is the base sound + a number
						if (Common::isDigit(_pushDownSound.name.lastChar())) {
							_pushDownSound.name.deleteLastChar();
						}

						_pushDownSound.name.insertChar('0' + i, _pushDownSound.name.size());
					} else {
						// Later games added an array of sound names
						_pushDownSound.name = _pianoSoundNames[i];
					}

					g_nancy->_sound->loadSound(_pushDownSound);
				}

				if (_puzzleType == kOrdering || _puzzleType == kKeypad || _puzzleType == kKeypadTerse) {
					// OrderingPuzzle and KeypadPuzzle allow for depressing buttons after they're pressed.
					// If the button is the last one the player pressed, it is removed from the order.
					// If not, the sequence is kept wrong and will be reset after enough buttons are pressed
					for (uint j = 0; j < _clickedSequence.size(); ++j) {
						if (_clickedSequence[j] == i && _downItems[i] == true) {
							popUp(i);
							if (_clickedSequence.back() == i) {
								_clickedSequence.pop_back();
							}

							return;
						}
					}
				}

				_clickedSequence.push_back(i);
				pushDown(i);
			}

			return;
		}
	}
}

Common::String OrderingPuzzle::getRecordTypeName() const {
	switch (_puzzleType) {
	case kPiano:
		return "PianoPuzzle";
	case kOrderItems:
		return "OrderItemsPuzzle";
	case kKeypad:
		return "KeypadPuzzle";
	case kKeypadTerse:
		return "KeypadTersePuzzle";
	default:
		return "OrderingPuzzle";
	}
}

void OrderingPuzzle::pushDown(uint id) {
	if (g_nancy->getGameType() == kGameTypeVampire) {
		g_nancy->_sound->playSound("BUOK");
	} else {
		g_nancy->_sound->loadSound(_pushDownSound);
		g_nancy->_sound->playSound(_pushDownSound);
	}

	_downItems[id] = true;
	Common::Rect destRect = _destRects[id];
	destRect.translate(-_screenPosition.left, -_screenPosition.top);
	_drawSurface.blitFrom(_image, _secondStateItems[id] ? _down2Rects[id] : _down1Rects[id], destRect);

	_needsRedraw = true;
}

void OrderingPuzzle::setToSecondState(uint id) {
	g_nancy->_sound->loadSound(_itemSound);
	g_nancy->_sound->playSound(_itemSound);

	_secondStateItems[id] = true;
	Common::Rect destRect = _destRects[id];
	destRect.translate(-_screenPosition.left, -_screenPosition.top);
	_drawSurface.blitFrom(_image, _downItems[id] ? _down2Rects[id] : _up2Rects[id], destRect);

	_needsRedraw = true;
}

void OrderingPuzzle::popUp(uint id) {
	if (_itemsStayDown) {
		// Make sure we only play the sound when the buttons don't auto-depress
		if (g_nancy->getGameType() == kGameTypeVampire) {
			g_nancy->_sound->playSound("BUOK");
		} else {
			if (!_popUpSound.name.empty() && _popUpSound.name != "NO SOUND") {
				g_nancy->_sound->playSound(_popUpSound);
			} else {
				g_nancy->_sound->playSound(_pushDownSound);
			}
		}
	}

	_downItems[id] = false;
	Common::Rect destRect = _destRects[id];
	destRect.translate(-_screenPosition.left, -_screenPosition.top);

	if (_secondStateItems[id] == false || _up2Rects.size() == 0) {
		_drawSurface.fillRect(destRect, _drawSurface.getTransparentColor());
	} else {
		_drawSurface.blitFrom(_image, _up2Rects[id], destRect);
	}

	_needsRedraw = true;
}

void OrderingPuzzle::clearAllElements() {
	for (uint id = 0; id < _downItems.size(); ++id) {
		popUp(id);
	}

	_clickedSequence.clear();
	return;
}

} // End of namespace Action
} // End of namespace Nancy
