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

#include "engines/nancy/nancy.h"
#include "engines/nancy/graphics.h"
#include "engines/nancy/resource.h"
#include "engines/nancy/sound.h"
#include "engines/nancy/input.h"
#include "engines/nancy/util.h"

#include "engines/nancy/state/scene.h"
#include "engines/nancy/action/puzzle/mindpuzzle.h"

namespace Nancy {
namespace Action {

void MindPuzzle::readData(Common::SeekableReadStream &stream) {
	// 608-byte base header (fixed-size fields; the color palette is a fixed-size
	// array and the count fields sit at the end).
	readFilename(stream, _imageName);		// 0x00, 33 bytes

	stream.skip(0x25 - 0x21);				// 0x21: two count fields
	readRect(stream, _exitHotspot);			// 0x25: give-up / leave hotspot
	_winScene.readData(stream);				// 0x35: scene reached on a solved code (25 bytes)
	_numGuesses = stream.readUint16LE();	// 0x4e

	_loseScene.readData(stream);			// 0x50: out-of-guesses / exit scene (25 bytes)
	stream.skip(0x79 - 0x69);				// 0x69: unused rect
	stream.skip(16);						// 0x79: tall right-edge rect (answer-reveal area), not a hotspot

	stream.skip(0xd9 - 0x89);				// answer-reveal positions (unused)
	readRect(stream, _feedbackSrcRects[0]);	// 0xd9, right-color-wrong-slot peg (plain pole)
	readRect(stream, _feedbackSrcRects[1]);	// 0xe9, right-color-and-slot peg (flag pole)

	// The golf-club submit button's screen rect (the actual hotspot) is at 0x109;
	// the rect at 0x79 used previously sat above the button, so clicks missed it.
	stream.skip(0x109 - 0xf9);				// 0xf9: another (unused) sprite rect
	readRect(stream, _submitButtonRect);	// 0x109

	readRectArray(stream, _ballSrcRects, kMaxColors);	// 0x119, ball sprite sources
	readRectArray(stream, _ballHitRects, kMaxColors);	// 0x1b9, bottom-row ball positions

	_allowDuplicates = stream.readByte();	// 0x259
	_numColors = stream.readUint16LE();		// 0x25a
	_codeLength = stream.readUint16LE();	// 0x25c
	_numRows = stream.readUint16LE();		// 0x25e

	_ballSrcRects.resize(_numColors);
	_ballHitRects.resize(_numColors);

	// Row records - 10 slot rects each (guess pegs 0..codeLength-1, feedback pegs 5..).
	_rows.resize(_numRows);
	for (uint r = 0; r < _numRows; ++r) {
		for (int s = 0; s < 10; ++s) {
			readRect(stream, _rows[r].slotRects[s]);
		}
	}

	for (uint i = 0; i < kNumSounds; ++i) {
		_sounds[i].readData(stream);
	}
}

void MindPuzzle::generateSecret() {
	_secret.resize(_codeLength);
	_secretColorCount.clear();
	_secretColorCount.resize(_numColors, 0);

	for (uint i = 0; i < _codeLength; ++i) {
		int16 color;
		if (_allowDuplicates == 0) {
			// Reject colors already used in the code.
			bool dup;
			do {
				dup = false;
				color = g_nancy->_randomSource->getRandomNumber(_numColors - 1);
				for (uint j = 0; j < i; ++j) {
					if (_secret[j] == color) {
						dup = true;
						break;
					}
				}
			} while (dup);
		} else {
			color = g_nancy->_randomSource->getRandomNumber(_numColors - 1);
		}
		_secret[i] = color;
		++_secretColorCount[color];
	}
}

void MindPuzzle::scoreRow(int row) {
	// Standard Mastermind scoring with duplicate-aware white pegs.
	Common::Array<int16> matched(_numColors, 0);

	// Black pegs: exact color + position.
	for (uint s = 0; s < _codeLength; ++s) {
		if (_guess[row][s] == _secret[s]) {
			++matched[_secret[s]];
		}
	}

	// White pegs: right color in a wrong slot, limited by how many of that color
	// remain unmatched in the code.
	for (uint s = 0; s < _codeLength; ++s) {
		if (_guess[row][s] == _secret[s]) {
			continue;
		}
		int16 color = _guess[row][s];
		for (uint t = 0; t < _codeLength; ++t) {
			if (_secret[t] == color && _guess[row][t] != _secret[t]) {
				if (matched[color] < _secretColorCount[color]) {
					++matched[color];
				}
				break;
			}
		}
	}

	// Lay out the feedback pegs: blacks first, then whites, then empties.
	int blacks = 0;
	for (uint s = 0; s < _codeLength; ++s) {
		if (_guess[row][s] == _secret[s]) {
			++blacks;
		}
	}
	int whites = 0;
	for (int16 c = 0; c < (int16)_numColors; ++c) {
		whites += matched[c];
	}
	whites -= blacks;

	for (uint s = 0; s < _codeLength; ++s) {
		if ((int)s < blacks) {
			_feedback[row][s] = kFeedbackBlack;
		} else if ((int)s < blacks + whites) {
			_feedback[row][s] = kFeedbackWhite;
		} else {
			_feedback[row][s] = kFeedbackNone;
		}
	}

	if (blacks == (int)_codeLength) {
		_solved = true;
	}
}

void MindPuzzle::drawPeg(int color, const Common::Rect &dest) {
	if (color < 0 || color >= (int)_ballSrcRects.size() || dest.isEmpty()) {
		return;
	}
	_drawSurface.blitFrom(_image, _ballSrcRects[color], dest);
}

void MindPuzzle::redraw() {
	_drawSurface.clear(g_nancy->_graphics->getTransColor());

	for (uint r = 0; r < _rows.size(); ++r) {
		for (uint s = 0; s < _codeLength; ++s) {
			if (_guess[r][s] >= 0) {
				drawPeg(_guess[r][s], _rows[r].slotRects[s]);
			}
			// Feedback pegs sit in the second half of the row's slots: a plain pole
			// for a right color in the wrong slot, a flag pole for a right color + slot.
			if (_feedback[r][s] != kFeedbackNone) {
				const Common::Rect &dest = _rows[r].slotRects[5 + s];
				if (!dest.isEmpty()) {
					_drawSurface.blitFrom(_image, _feedbackSrcRects[_feedback[r][s] == kFeedbackBlack ? 1 : 0], dest);
				}
			}
		}
	}

	// The ball being dragged follows the cursor, centered on it.
	if (_heldColor >= 0 && _heldColor < (int)_ballSrcRects.size()) {
		const Common::Rect &src = _ballSrcRects[_heldColor];
		int w = src.width();
		int h = src.height();
		Common::Rect dst(_heldDrawPos.x - w / 2, _heldDrawPos.y - h / 2,
			_heldDrawPos.x - w / 2 + w, _heldDrawPos.y - h / 2 + h);
		_drawSurface.blitFrom(_image, src, dst);
	}

	_needsRedraw = true;
}

int MindPuzzle::paletteHit(const Common::Point &mouseVP) const {
	for (uint i = 0; i < _ballHitRects.size(); ++i) {
		if (_ballHitRects[i].contains(mouseVP)) {
			return i;
		}
	}
	return -1;
}

bool MindPuzzle::slotHit(const Common::Point &mouseVP, int &slot) const {
	if (_currentRow >= (int)_rows.size()) {
		return false;
	}
	for (uint s = 0; s < _codeLength; ++s) {
		if (_rows[_currentRow].slotRects[s].contains(mouseVP)) {
			slot = s;
			return true;
		}
	}
	return false;
}

void MindPuzzle::init() {
	Common::Rect vpBounds = NancySceneState.getViewport().getBounds();
	_drawSurface.create(vpBounds.width(), vpBounds.height(),
		g_nancy->_graphics->getInputPixelFormat());
	_drawSurface.clear(g_nancy->_graphics->getTransColor());
	setTransparent(true);
	setVisible(true);
	moveTo(vpBounds);

	g_nancy->_resource->loadImage(_imageName, _image);
	_image.setTransparentColor(_drawSurface.getTransparentColor());

	for (uint r = 0; r < kMaxRows; ++r) {
		for (uint s = 0; s < kSlotsPerRow; ++s) {
			_guess[r][s] = -1;
			_feedback[r][s] = kFeedbackNone;
		}
	}

	_currentRow = 0;
	_heldColor = -1;
	_remainingGuesses = _numGuesses;
	_solved = false;

	generateSecret();
	redraw();
}

void MindPuzzle::execute() {
	switch (_state) {
	case kBegin:
		init();
		registerGraphics();
		_state = kRun;
		// fall through
	case kRun:
		if (_solved) {
			_state = kActionTrigger;
		}
		break;
	case kActionTrigger:
		if (_solved) {
			// Win: play the applause cue, wait for it to finish, then move to the win scene.
			if (!_outcomeStarted) {
				_outcomeStarted = true;
				const RandomSoundBlock &applause = _sounds[kApplauseSound];
				if (!applause.names.empty()) {
					_outcomeSound.name = applause.names[g_nancy->_randomSource->getRandomNumber(applause.names.size() - 1)];
					_outcomeSound.channelID = applause.channel;
					_outcomeSound.numLoops = applause.numLoops;
					_outcomeSound.volume = applause.volume;
					g_nancy->_sound->loadSound(_outcomeSound);
					g_nancy->_sound->playSound(_outcomeSound);
				}
			}

			if (g_nancy->_sound->isSoundPlaying(_outcomeSound)) {
				return;
			}

			g_nancy->_sound->stopSound(_outcomeSound);
			NancySceneState.setEventFlag(_winScene._flag);
			NancySceneState.changeScene(_winScene._sceneChange);
		} else {
			// Out of guesses, or the player left via the exit hotspot.
			NancySceneState.setEventFlag(_loseScene._flag);
			NancySceneState.changeScene(_loseScene._sceneChange);
		}

		finishExecution();
		break;
	}
}

void MindPuzzle::handleInput(NancyInput &input) {
	if (_state != kRun || _solved) {
		return;
	}

	Common::Rect vpScreen = NancySceneState.getViewport().getScreenPosition();
	Common::Point mouseVP = input.mousePos - Common::Point(vpScreen.left, vpScreen.top);

	// A held ball follows the cursor; release it into a slot (swapping with any
	// ball already there) or anywhere else to send it back to the palette.
	if (_heldColor != -1) {
		g_nancy->_cursor->setCursorType(CursorManager::kDragHand);

		if (_heldDrawPos != mouseVP) {
			_heldDrawPos = mouseVP;
			redraw();
		}

		if (input.input & NancyInput::kLeftMouseButtonUp) {
			int slot = -1;
			if (slotHit(mouseVP, slot)) {
				int16 previous = _guess[_currentRow][slot];
				_guess[_currentRow][slot] = _heldColor;
				_heldColor = previous;	// keep dragging the displaced ball, if any
			} else {
				_heldColor = -1;
			}
			redraw();
		}
		return;
	}

	// Pick up a ball from the bottom palette.
	int color = paletteHit(mouseVP);
	if (color != -1) {
		g_nancy->_cursor->setCursorType(CursorManager::kDragHand);
		if (input.input & NancyInput::kLeftMouseButtonUp) {
			_heldColor = color;
			_heldDrawPos = mouseVP;
			redraw();
		}
		return;
	}

	// Pick a ball back up from a filled slot in the current row.
	int slot = -1;
	if (slotHit(mouseVP, slot) && _guess[_currentRow][slot] != -1) {
		g_nancy->_cursor->setCursorType(CursorManager::kDragHand);
		if (input.input & NancyInput::kLeftMouseButtonUp) {
			_heldColor = _guess[_currentRow][slot];
			_guess[_currentRow][slot] = -1;
			_heldDrawPos = mouseVP;
			redraw();
		}
		return;
	}

	// Submit the row via the golf-club button, once every slot is filled.
	if (_submitButtonRect.contains(mouseVP)) {
		bool full = true;
		for (uint s = 0; s < _codeLength; ++s) {
			if (_guess[_currentRow][s] < 0) {
				full = false;
				break;
			}
		}

		g_nancy->_cursor->setCursorType(CursorManager::kHotspot);
		if (full && (input.input & NancyInput::kLeftMouseButtonUp)) {
			scoreRow(_currentRow);
			redraw();
			if (!_solved) {
				++_currentRow;
				--_remainingGuesses;
				if (_currentRow >= (int)_rows.size() || _remainingGuesses <= 0) {
					// Out of guesses.
					_state = kActionTrigger;
				}
			}
		}
		return;
	}

	// Leave the puzzle (give up) via the exit hotspot; this reports a loss.
	if (_exitHotspot.contains(mouseVP)) {
		g_nancy->_cursor->setCursorType(g_nancy->_cursor->_puzzleExitCursor);
		if (input.input & NancyInput::kLeftMouseButtonUp) {
			_state = kActionTrigger;
		}
	}
}

} // End of namespace Action
} // End of namespace Nancy
