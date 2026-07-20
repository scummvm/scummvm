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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "common/random.h"
#include "common/util.h"

#include "engines/nancy/nancy.h"
#include "engines/nancy/graphics.h"
#include "engines/nancy/resource.h"
#include "engines/nancy/sound.h"
#include "engines/nancy/input.h"
#include "engines/nancy/cursor.h"
#include "engines/nancy/util.h"

#include "engines/nancy/state/scene.h"
#include "engines/nancy/action/puzzle/blockspuzzle.h"

namespace Nancy {
namespace Action {

void BlocksPuzzle::readData(Common::SeekableReadStream &stream) {
	// 111-byte header (bulk-copied by the original into a sub-object).
	readFilename(stream, _imageName);			// 0x00
	_turnDuration = stream.readUint16LE();		// 0x21
	_carryCursorType = stream.readUint16LE();	// 0x23
	_turnCursorType = stream.readUint16LE();	// 0x25
	_startFlag.label = stream.readSint16LE();	// 0x27
	_startFlag.flag = stream.readByte();		// 0x29

	readRect(stream, _overlaySrc);			// 0x2a
	readRect(stream, _overlayDest);			// 0x3a
	readRect(stream, _turntableDest);		// 0x4a
	readRect(stream, _turntableHotspot);	// 0x5a

	// The scene change and flag applied once the board comes out solved. The frame is
	// always the scene's first, and its sound carries over.
	_solveScene.sceneID = stream.readUint16LE();	// 0x6a
	_solveScene.continueSceneSound = kContinueSceneSound;
	_solveFlag.label = stream.readSint16LE();		// 0x6c
	_solveFlag.flag = stream.readByte();			// 0x6e

	// A count-prefixed array of fixed 23-byte hotspot records:
	// {rect, u16 cursorType, u16 sceneID, u16 frameID, byte}. The sample carries one - the
	// "give up / exit" hotspot (leave the puzzle unsolved), with the exit cursor type.
	int16 numZones = stream.readSint16LE();
	for (int16 i = 0; i < numZones; ++i) {
		Common::Rect r;
		readRect(stream, r);
		uint16 cursorType = stream.readUint16LE();
		uint16 sceneID = stream.readUint16LE();
		int16 exitFlagLabel = stream.readSint16LE();
		byte exitFlagValue = stream.readByte();

		if (i == 0) {
			_exitHotspot = r;
			_exitCursorType = cursorType;
			_exitScene.sceneID = sceneID;
			// The field after the scene id is a flag label (set on give-up), not a frame.
			_exitScene.frameID = 0;
			_exitFlag.label = exitFlagLabel;
			_exitFlag.flag = exitFlagValue;
		}
	}

	// The block shapes, each a 13-byte descriptor of its row in the atlas image.
	int16 numBlocks = stream.readSint16LE();
	_blocks.resize(numBlocks);
	for (int16 i = 0; i < numBlocks; ++i) {
		Block &block = _blocks[i];
		block.numRotations = stream.readByte();
		block.numTweenFrames = stream.readSint16LE();
		block.gap = stream.readSint16LE();
		block.width = stream.readSint16LE();
		block.height = stream.readSint16LE();
		block.atlasX = stream.readSint16LE();
		block.atlasY = stream.readSint16LE();
	}

	// The board cells, each a 39-byte record holding both its starting and its target
	// contents.
	int16 numCells = stream.readSint16LE();
	_cells.resize(numCells);
	for (int16 i = 0; i < numCells; ++i) {
		Cell &cell = _cells[i];
		cell.block = stream.readSint16LE();
		cell.rotation = stream.readByte();
		cell.targetBlock = stream.readSint16LE();
		cell.targetRotation = stream.readUint16LE();
		readRect(stream, cell.dest);
		readRect(stream, cell.hotspot);
	}

	// Three random-sound blocks: [0] turn, [1] pick up / put down, [2] success.
	for (uint i = 0; i < kNumSounds; ++i) {
		_sounds[i].readData(stream);
	}
}

void BlocksPuzzle::init() {
	Common::Rect vpBounds = NancySceneState.getViewport().getBounds();
	_drawSurface.create(vpBounds.width(), vpBounds.height(), g_nancy->_graphics->getInputPixelFormat());
	_drawSurface.clear(g_nancy->_graphics->getTransColor());
	setTransparent(true);
	setVisible(true);
	moveTo(vpBounds);

	g_nancy->_resource->loadImage(_imageName, _image);
	_image.setTransparentColor(_drawSurface.getTransparentColor());

	_hasTurntable = !_turntableDest.isEmpty();

	NancySceneState.setEventFlag(_startFlag);
	NancySceneState.setNoHeldItem();

	redraw();
	registerGraphics();
}

Common::Rect BlocksPuzzle::blockSrc(int16 block, byte rotation, int16 frame) const {
	const Block &b = _blocks[block];
	int16 left = b.atlasX + ((b.numTweenFrames + 1) * rotation + frame) * (b.gap + b.width);
	return Common::Rect(left, b.atlasY, left + b.width, b.atlasY + b.height);
}

int16 BlocksPuzzle::cellAtCursor(const Common::Point &mousePos, bool occupiedOnly) const {
	for (uint i = 0; i < _cells.size(); ++i) {
		const Cell &cell = _cells[i];
		if (occupiedOnly && cell.block == kNoBlock) {
			continue;
		}

		if (NancySceneState.getViewport().convertViewportToScreen(cell.hotspot).contains(mousePos)) {
			return (int16)i;
		}
	}

	return kNoBlock;
}

bool BlocksPuzzle::isSolved() const {
	for (uint i = 0; i < _cells.size(); ++i) {
		const Cell &cell = _cells[i];
		if (cell.block == kNoBlock || cell.block != cell.targetBlock || cell.rotation != cell.targetRotation) {
			return false;
		}
	}

	return true;
}

void BlocksPuzzle::pickUp(int16 cell) {
	playSoundBlock(_sounds[kHandleSound]);

	if (cell == kTurntableCell) {
		_carriedBlock = _turnBlock;
		_carriedRotation = _turnRotation;
		_carriedSrc = _turnSrc;
		_turnBlock = kNoBlock;
	} else {
		Cell &c = _cells[cell];
		_carriedBlock = c.block;
		_carriedRotation = c.rotation;
		_carriedSrc = blockSrc(c.block, c.rotation, 0);
		c.block = kNoBlock;
	}

	redraw();
}

void BlocksPuzzle::drop(int16 cell) {
	playSoundBlock(_sounds[kHandleSound]);

	// Putting a block down where one already sits swaps them, so the displaced block ends
	// up in hand and the board never loses a piece.
	if (cell == kTurntableCell) {
		if (_turnBlock != kNoBlock) {
			SWAP(_turnBlock, _carriedBlock);
			SWAP(_turnRotation, _carriedRotation);
			SWAP(_turnSrc, _carriedSrc);
		} else {
			_turnBlock = _carriedBlock;
			_turnRotation = _carriedRotation;
			_turnSrc = _carriedSrc;
			_carriedBlock = kNoBlock;
		}
	} else {
		Cell &c = _cells[cell];
		if (c.block != kNoBlock) {
			SWAP(c.block, _carriedBlock);
			SWAP(c.rotation, _carriedRotation);
			_carriedSrc = blockSrc(_carriedBlock, _carriedRotation, 0);
		} else {
			c.block = _carriedBlock;
			c.rotation = _carriedRotation;
			_carriedBlock = kNoBlock;
		}
	}

	redraw();
}

void BlocksPuzzle::startTurn() {
	// Without a turntable the carried block is turned where it is, on the cursor.
	if (!_hasTurntable) {
		_turnBlock = _carriedBlock;
		_turnRotation = _carriedRotation;
		_turnSrc = _carriedSrc;
	}

	playSoundBlock(_sounds[kTurnSound]);
	_puzzleState = kTurnFrame;
	_turnFrame = 1;
}

void BlocksPuzzle::redraw() {
	_drawSurface.clear(g_nancy->_graphics->getTransColor());

	for (uint i = 0; i < _cells.size(); ++i) {
		const Cell &cell = _cells[i];
		if (cell.block == kNoBlock) {
			continue;
		}

		_drawSurface.blitFrom(_image, blockSrc(cell.block, cell.rotation, 0),
			Common::Point(cell.dest.left, cell.dest.top));
	}

	if (!_overlayDest.isEmpty()) {
		_drawSurface.blitFrom(_image, _overlaySrc, Common::Point(_overlayDest.left, _overlayDest.top));
	}

	if (_hasTurntable && _turnBlock != kNoBlock) {
		_drawSurface.blitFrom(_image, _turnSrc, Common::Point(_turntableDest.left, _turntableDest.top));
	}

	// The carried block rides the cursor. While it is being turned in hand its sprite is
	// the animation's current frame.
	if (_carriedBlock != kNoBlock) {
		_drawSurface.blitFrom(_image, _carriedSrc,
			Common::Point(_dragPos.x - _carriedSrc.width() / 2, _dragPos.y - _carriedSrc.height() / 2));
	}

	_needsRedraw = true;
}

void BlocksPuzzle::setDataCursor(uint16 cursorType) const {
	// The ids in the AR data are raw Nancy13 cursor types, which is exactly what the
	// "set from script" path expects.
	g_nancy->_cursor->setCursorType((CursorManager::CursorType)cursorType, true);
}

SoundDescription BlocksPuzzle::playSoundBlock(const RandomSoundBlock &block) {
	SoundDescription desc;
	if (block.names.empty()) {
		return desc;
	}

	uint idx = block.names.size() == 1 ? 0 : g_nancy->_randomSource->getRandomNumber(block.names.size() - 1);
	const Common::String &name = block.names[idx];
	if (name.empty() || name == "NO SOUND") {
		return desc;
	}

	desc.name = name;
	desc.channelID = block.channel;
	desc.numLoops = block.numLoops > 0 ? block.numLoops : 1;
	desc.volume = block.volume;

	g_nancy->_sound->loadSound(desc);
	g_nancy->_sound->playSound(desc);
	return desc;
}

void BlocksPuzzle::execute() {
	switch (_state) {
	case kBegin:
		init();
		_state = kRun;
		// fall through
	case kRun:
		if (_exitRequested) {
			_state = kActionTrigger;
			break;
		}

		switch (_puzzleState) {
		case kPlaying:
			if (isSolved()) {
				_solved = true;
				_puzzleState = kStartSolved;
			}

			break;
		case kStartSolved:
			_solveSound = playSoundBlock(_sounds[kSuccessSound]);
			_puzzleState = kWaitSolved;

			if (_solveSound.name.empty()) {
				_state = kActionTrigger;
			}

			break;
		case kTurnFrame: {
			const Block &block = _blocks[_turnBlock];
			int16 lastFrame = block.numTweenFrames + 1;

			if (_turnFrame == lastFrame) {
				if (_turnRotation == block.numRotations - 1) {
					_turnSrc = blockSrc(_turnBlock, 0, 0);
					_turnRotation = 0;
				} else {
					// One past a rotation's last tween frame is the next rotation's
					// resting frame, so the animation lands on it before stepping over.
					_turnSrc = blockSrc(_turnBlock, _turnRotation, _turnFrame);
					++_turnRotation;
				}

				_puzzleState = kPlaying;
				_turnFrame = 0;

				if (!_hasTurntable) {
					// The block is back in hand, turned one step further.
					_carriedBlock = _turnBlock;
					_carriedRotation = _turnRotation;
					_turnBlock = kNoBlock;
				}
			} else {
				_turnSrc = blockSrc(_turnBlock, _turnRotation, _turnFrame);
				++_turnFrame;
				_puzzleState = kTurnDelay;
				_nextFrameTime = g_nancy->getTotalPlayTime() + _turnDuration / (block.numTweenFrames + 1);
			}

			_carriedSrc = _turnSrc;
			redraw();
			break;
		}
		case kTurnDelay:
			if (g_nancy->getTotalPlayTime() >= _nextFrameTime) {
				_puzzleState = kTurnFrame;
			}

			break;
		case kWaitSolved:
			if (!g_nancy->_sound->isSoundPlaying(_solveSound)) {
				_state = kActionTrigger;
			}

			break;
		}

		break;
	case kActionTrigger:
		if (_exitRequested) {
			NancySceneState.setEventFlag(_exitFlag);
			if (_exitScene.sceneID != kNoScene) {
				NancySceneState.changeScene(_exitScene);
			}
		} else {
			if (_solveScene.sceneID != kNoScene) {
				NancySceneState.changeScene(_solveScene);
			}

			NancySceneState.setEventFlag(_solveFlag);
		}

		finishExecution();
		break;
	}
}

void BlocksPuzzle::handleInput(NancyInput &input) {
	if (_state != kRun || _solved) {
		return;
	}

	// The carried block keeps tracking the cursor even while it is being turned.
	if (_carriedBlock != kNoBlock) {
		Common::Rect screenPt(input.mousePos.x, input.mousePos.y, input.mousePos.x + 1, input.mousePos.y + 1);
		Common::Rect vpPt = NancySceneState.getViewport().convertScreenToViewport(screenPt);
		_dragPos = Common::Point(vpPt.left, vpPt.top);
		redraw();
	}

	if (_puzzleState != kPlaying) {
		return;
	}

	const bool click = (input.input & NancyInput::kLeftMouseButtonUp) != 0;

	// -- Carrying a block: it follows the cursor until it is put down. --
	if (_carriedBlock != kNoBlock) {
		// Boards with no turntable turn the block in hand instead.
		if (!_hasTurntable && (input.input & NancyInput::kRightMouseButtonUp)) {
			startTurn();
			input.eatMouseInput();
			return;
		}

		int16 cell = cellAtCursor(input.mousePos, false);
		if (cell == kNoBlock && _hasTurntable &&
				NancySceneState.getViewport().convertViewportToScreen(_turntableDest).contains(input.mousePos)) {
			cell = kTurntableCell;
		}

		if (cell != kNoBlock) {
			setDataCursor(_carryCursorType);
			if (click) {
				drop(cell);
			}

			input.eatMouseInput();
		}

		return;
	}

	// -- Empty-handed: pick a block up, turn the one on the turntable, or leave. --
	int16 cell = cellAtCursor(input.mousePos, true);
	if (cell != kNoBlock) {
		setDataCursor(_carryCursorType);
		if (click) {
			pickUp(cell);
		}

		input.eatMouseInput();
		return;
	}

	if (_hasTurntable && _turnBlock != kNoBlock) {
		if (NancySceneState.getViewport().convertViewportToScreen(_turntableHotspot).contains(input.mousePos)) {
			setDataCursor(_turnCursorType);
			if (click) {
				startTurn();
			}

			input.eatMouseInput();
			return;
		}

		if (NancySceneState.getViewport().convertViewportToScreen(_turntableDest).contains(input.mousePos)) {
			setDataCursor(_carryCursorType);
			if (click) {
				pickUp(kTurntableCell);
			}

			input.eatMouseInput();
			return;
		}
	}

	if (!_exitHotspot.isEmpty() &&
			NancySceneState.getViewport().convertViewportToScreen(_exitHotspot).contains(input.mousePos)) {
		setDataCursor(_exitCursorType);
		if (click) {
			_exitRequested = true;
		}
	}
}

} // End of namespace Action
} // End of namespace Nancy
