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
#include "engines/nancy/cursor.h"
#include "engines/nancy/util.h"

#include "engines/nancy/state/scene.h"
#include "engines/nancy/action/puzzle/boardgamepuzzle.h"

namespace Nancy {
namespace Action {

void BoardGamePuzzle::readData(Common::SeekableReadStream &stream) {
	readFilename(stream, _imageName);		// 0x00
	stream.skip(2);							// int16 card count (10)
	readRectArray(stream, _cardBlueSrcs, kNumButtons);	// 0x23, available-card sprites
	readRectArray(stream, _cardWhiteSrcs, kNumButtons);	// 0xc3, active-card sprites
	readRectArray(stream, _buttonRects, kNumButtons);	// 0x163, on-screen card positions
	readRect(stream, _resetPressedSrc);		// 0x203, pressed reset sprite
	readRect(stream, _resetButtonRect);		// 0x213, on-screen reset position
	readFilename(stream, _movieName);		// 0x223, the board-game movie
	readRect(stream, _movieRect);			// 0x244
	readRect(stream, _boardRect);			// 0x254

	_framesPerPosition = stream.readSint16LE();	// 0x264
	_winTarget = stream.readSint16LE();			// 0x266
	stream.skip(2);								// 0x268

	_moves.resize(kNumMoves);
	for (uint i = 0; i < kNumMoves; ++i) {		// 0x26a, 12 x 10-byte records
		_moves[i].amount = stream.readSint16LE();
		_moves[i].jumpFrom = stream.readSint16LE();
		_moves[i].jumpTo = stream.readSint16LE();
		_moves[i].jumpFrameStart = stream.readSint16LE();
		_moves[i].jumpFrameEnd = stream.readSint16LE();
	}

	_winScene.readData(stream);				// 0x2e2, SceneChangeWithFlag
	_loseScene.readData(stream);			// 0x2fb, SceneChangeWithFlag

	// Six random-sound blocks (button/click/clank/slide/key/beep).
	for (uint i = 0; i < kNumSounds; ++i) {
		_sounds[i].readData(stream);
	}
}

void BoardGamePuzzle::init() {
	Common::Rect vpBounds = NancySceneState.getViewport().getBounds();
	_drawSurface.create(vpBounds.width(), vpBounds.height(),
		g_nancy->_graphics->getInputPixelFormat());
	_drawSurface.clear(g_nancy->_graphics->getTransColor());
	setTransparent(true);
	setVisible(true);
	moveTo(vpBounds);

	g_nancy->_resource->loadImage(_imageName, _image);
	_image.setTransparentColor(_drawSurface.getTransparentColor());

	_moviePlayer.loadFile(_movieName);
	_buttonUsed.clear();
	_buttonUsed.resize(kNumButtons, false);
	_position = 0;
	_solved = false;
	_lost = false;
	_boardState = kBoardWaiting;
	_moviePlayer.goToFrame(framePosition(_position));

	redraw();
}

void BoardGamePuzzle::drawCard(uint index) {
	if (index >= _buttonRects.size() || _buttonRects[index].isEmpty()) {
		return;
	}

	const Common::Rect *src;
	if ((int)index == _activeCard) {
		src = &_cardWhiteSrcs[index];	// currently being played
	} else if (!_buttonUsed[index]) {
		src = &_cardBlueSrcs[index];	// available
	} else {
		return;							// used -> dark card from the background shows
	}

	const Common::Rect &dst = _buttonRects[index];
	_drawSurface.blitFrom(_image, *src, Common::Point(dst.left, dst.top));
}

void BoardGamePuzzle::redraw() {
	_drawSurface.clear(g_nancy->_graphics->getTransColor());

	// The board movie over the board region. Everything else (board frame, the
	// reset button, and the dark "used" cards) lives on the scene background and
	// shows through the transparent surface. _image is a sprite sheet holding
	// only the blue/white card variants + the pressed reset button.
	_moviePlayer.drawFrame(_drawSurface, Common::Point(_boardRect.left, _boardRect.top));

	// The move cards: blue when available, white while being played.
	for (uint i = 0; i < _buttonRects.size(); ++i) {
		drawCard(i);
	}

	// The reset button only appears (its pressed sprite) while it is held.
	if (_resetPressed && !_resetButtonRect.isEmpty()) {
		_drawSurface.blitFrom(_image, _resetPressedSrc,
			Common::Point(_resetButtonRect.left, _resetButtonRect.top));
	}

	_needsRedraw = true;
}

void BoardGamePuzzle::playSoundBlock(uint index) {
	if (index >= kNumSounds) {
		return;
	}

	const RandomSoundBlock &block = _sounds[index];
	if (block.names.empty() || block.names[0].empty() || block.names[0] == "NO SOUND") {
		return;
	}

	SoundDescription desc;
	desc.name = block.names[0];
	desc.channelID = block.channel;
	desc.numLoops = block.numLoops > 0 ? block.numLoops : 1;
	desc.volume = block.volume;

	g_nancy->_sound->loadSound(desc);
	g_nancy->_sound->playSound(desc);
}

void BoardGamePuzzle::resolveMove(int button) {
	// Mirrors the original move-resolution rules: advance the token along the
	// track, warp on a jump square, win by landing on the target exactly, lose
	// by overshooting it. Positions map to movie frames via framePosition().
	const MoveRecord &m = _moves[button];
	int oldPos = _position;
	int startFrame, endFrame;

	if (oldPos == 0) {
		startFrame = 0;
		_position = m.amount;
		endFrame = framePosition(_position);
	} else if (oldPos == m.jumpFrom) {
		startFrame = m.jumpFrameStart;
		endFrame = m.jumpFrameEnd;
		_position = m.jumpTo;
	} else if (m.amount + oldPos > _winTarget) {
		_lost = true;
		startFrame = framePosition(oldPos);
		endFrame = startFrame;
	} else {
		startFrame = framePosition(oldPos);
		_position += m.amount;
		endFrame = framePosition(_position);
		if (_position == _winTarget) {
			_solved = true;
		}
	}

	_buttonUsed[button] = true;
	_activeCard = button;			// shows the white sprite while the move plays
	_boardState = kBoardAnimating;
	playSoundBlock(kSlideSound);
	_moviePlayer.playRange(startFrame, endFrame);

	// Full redraw once at the start so the played card turns white and the first
	// frame is shown; the animation then only re-blits the movie region.
	redraw();
}

void BoardGamePuzzle::execute() {
	switch (_state) {
	case kBegin:
		init();
		registerGraphics();
		_state = kRun;
		// fall through
	case kRun:
		if (_resetPressed && g_nancy->getTotalPlayTime() - _resetPressedTime > 200) {
			_resetPressed = false;
			redraw();
		}

		if (_boardState == kBoardAnimating) {
			if (_moviePlayer.update()) {
				// Only the board (movie) region changes each frame; re-blitting
				// the whole overlay + re-darkening buttons here would be far too
				// slow and make playback drop frames. The static chrome stays.
				_moviePlayer.drawFrame(_drawSurface, Common::Point(_boardRect.left, _boardRect.top));
				_needsRedraw = true;
			}
			if (!_moviePlayer.isRangePlaying()) {
				// The played card is now used: a full redraw turns it dark.
				_activeCard = -1;
				redraw();

				if (_solved) {
					playSoundBlock(kWinSound);
					_resultTime = g_nancy->getTotalPlayTime();
					_boardState = kBoardResult;
				} else if (_lost) {
					playSoundBlock(kLoseSound);
					_resultTime = g_nancy->getTotalPlayTime();
					_boardState = kBoardResult;
				} else {
					playSoundBlock(kLandSound);
					_boardState = kBoardWaiting;
				}
			}
		} else if (_boardState == kBoardResult) {
			// Hold on the finished board briefly before the scene change.
			if (g_nancy->getTotalPlayTime() - _resultTime > 1500) {
				_state = kActionTrigger;
			}
		}
		break;
	case kActionTrigger:
		if (_solved) {
			_winScene.execute();
		} else if (_lost) {
			_loseScene.execute();
		}
		finishExecution();
		break;
	}
}

void BoardGamePuzzle::handleInput(NancyInput &input) {
	if (_state != kRun || _boardState != kBoardWaiting) {
		return;
	}

	// Reset returns the token to the start and re-enables every move button.
	if (!_resetButtonRect.isEmpty() &&
			NancySceneState.getViewport().convertViewportToScreen(_resetButtonRect).contains(input.mousePos)) {
		g_nancy->_cursor->setCursorType(CursorManager::kHotspot);
		if (input.input & NancyInput::kLeftMouseButtonUp) {
			_position = 0;
			for (uint i = 0; i < _buttonUsed.size(); ++i) {
				_buttonUsed[i] = false;
			}
			_resetPressed = true;
			_resetPressedTime = g_nancy->getTotalPlayTime();
			playSoundBlock(kResetSound);
			_moviePlayer.goToFrame(framePosition(_position));
			redraw();
		}
		return;
	}

	// A move button advances the token; each may be used once.
	for (uint i = 0; i < _buttonRects.size(); ++i) {
		if (_buttonUsed[i] || _buttonRects[i].isEmpty()) {
			continue;
		}
		if (!NancySceneState.getViewport().convertViewportToScreen(_buttonRects[i]).contains(input.mousePos)) {
			continue;
		}
		g_nancy->_cursor->setCursorType(CursorManager::kHotspot);
		if (input.input & NancyInput::kLeftMouseButtonUp) {
			playSoundBlock(kButtonSound);
			resolveMove(i);
		}
		return;
	}
}

} // End of namespace Action
} // End of namespace Nancy
