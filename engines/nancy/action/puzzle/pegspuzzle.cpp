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

#include "engines/nancy/nancy.h"
#include "engines/nancy/graphics.h"
#include "engines/nancy/resource.h"
#include "engines/nancy/sound.h"
#include "engines/nancy/input.h"
#include "engines/nancy/cursor.h"
#include "engines/nancy/util.h"

#include "engines/nancy/state/scene.h"
#include "engines/nancy/action/puzzle/pegspuzzle.h"

namespace Nancy {
namespace Action {

void PegsPuzzle::readData(Common::SeekableReadStream &stream) {
	// 96-byte header (bulk-copied by the original into a sub-object).
	readFilename(stream, _imageName);			// 0x00
	// The puzzle carries its own cursors as raw Nancy13 cursor type ids.
	_hoverCursorType = stream.readUint16LE();	// 0x21
	_dragCursorType = stream.readUint16LE();	// 0x23
	_startEmptyFlag = stream.readByte();	// 0x25
	_startEmptyPos = stream.readByte();		// 0x26
	_targetPegCount = stream.readByte();	// 0x27

	// Win / lose scene changes, each {uint16 sceneID, uint16 frameID, byte}. A frameID of
	// 0xffff means "no specific frame" (the target scene may be a video, so seeking to
	// 65535 must be avoided) - keep the default frame 0.
	_winScene.sceneID = stream.readUint16LE();	// 0x28
	uint16 winFrame = stream.readUint16LE();	// 0x2a
	_winScene.frameID = (winFrame == 0xffff) ? 0 : winFrame;
	stream.skip(1);								// 0x2c
	_loseScene.sceneID = stream.readUint16LE();	// 0x2d
	uint16 loseFrame = stream.readUint16LE();	// 0x2f
	_loseScene.frameID = (loseFrame == 0xffff) ? 0 : loseFrame;
	stream.skip(1);								// 0x31

	readRect(stream, _pegSrc);				// 0x32 - int32 rect
	readRect(stream, _selectedSrc);			// 0x42 - int32 rect
	_originY = stream.readUint16LE();		// 0x52
	_originX = stream.readUint16LE();		// 0x54
	_numRows = stream.readUint16LE();		// 0x56
	_numCols = stream.readUint16LE();		// 0x58
	_pitchYBias = stream.readUint16LE();	// 0x5a
	_pitchXBias = stream.readUint16LE();	// 0x5c
	_numBlocked = stream.readUint16LE();	// 0x5e

	// The blocked cells (the corners cut out of the grid).
	_blockedPositions.resize(_numBlocked);
	for (uint i = 0; i < _numBlocked; ++i) {
		_blockedPositions[i] = stream.readByte();
	}

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

	// Five random-sound blocks: [0] peg select, [1] jump, [2] selection pulse, [3] win, [4] lose.
	_sounds.resize(5);
	for (uint i = 0; i < 5; ++i) {
		_sounds[i].readData(stream);
	}
}

void PegsPuzzle::init() {
	Common::Rect vpBounds = NancySceneState.getViewport().getBounds();
	_drawSurface.create(vpBounds.width(), vpBounds.height(), g_nancy->_graphics->getInputPixelFormat());
	_drawSurface.clear(g_nancy->_graphics->getTransColor());
	setTransparent(true);
	setVisible(true);
	moveTo(vpBounds);

	g_nancy->_resource->loadImage(_imageName, _image);
	_image.setTransparentColor(_drawSurface.getTransparentColor());

	// Build the board: every hole starts with a peg, the cut-out corners are blocked,
	// and one hole (usually the centre) starts empty.
	_board.resize(64);
	for (uint i = 0; i < 64; ++i) {
		_board[i] = kPeg;
	}
	for (uint i = 0; i < _blockedPositions.size(); ++i) {
		byte pos = _blockedPositions[i];
		int col = _numRows ? (pos % _numRows) : 0;
		int row = _numCols ? (pos / _numCols) : 0;
		if (validCell(col, row)) {
			_board[cellIndex(col, row)] = kBlocked;
		}
	}
	if (_startEmptyFlag == 0) {
		int col = _numRows ? (_startEmptyPos % _numRows) : 0;
		int row = _numCols ? (_startEmptyPos / _numCols) : 0;
		if (validCell(col, row)) {
			_board[cellIndex(col, row)] = kEmpty;
		}
	}

	// Per-cell destination rects (viewport space). The pitch is the sprite size plus a
	// small per-axis bias, exactly as the original computes it.
	int pitchX = (_pitchXBias - _pegSrc.left) + _pegSrc.right;
	int pitchY = (_pitchYBias - _pegSrc.top) + _pegSrc.bottom;
	int cellW = _pegSrc.right - _pegSrc.left;
	int cellH = _pegSrc.bottom - _pegSrc.top;

	_destRects.resize(64);
	for (int row = 0; row < _numRows; ++row) {
		for (int col = 0; col < _numCols; ++col) {
			int left = _originX + pitchX * col;
			int top = _originY + pitchY * row;
			_destRects[cellIndex(col, row)] = Common::Rect((int16)left, (int16)top,
				(int16)(left + cellW), (int16)(top + cellH));
		}
	}

	NancySceneState.setNoHeldItem();

	redraw();
	registerGraphics();
}

bool PegsPuzzle::validCell(int col, int row) const {
	return col >= 0 && row >= 0 && col < _numCols && row < _numRows;
}

bool PegsPuzzle::canJump(int col, int row, int dCol, int dRow, int &destCol, int &destRow) const {
	destCol = col + 2 * dCol;
	destRow = row + 2 * dRow;
	if (!validCell(destCol, destRow)) {
		return false;
	}

	int midCol = col + dCol;
	int midRow = row + dRow;
	return _board[cellIndex(col, row)] == kPeg &&
		_board[cellIndex(midCol, midRow)] == kPeg &&
		_board[cellIndex(destCol, destRow)] == kEmpty;
}

bool PegsPuzzle::isCarriedTarget(int col, int row) const {
	if (_carriedCol < 0) {
		return false;
	}

	static const int dirs[4][2] = { { 0, -1 }, { 0, 1 }, { -1, 0 }, { 1, 0 } };
	for (int d = 0; d < 4; ++d) {
		int dc, dr;
		if (canJump(_carriedCol, _carriedRow, dirs[d][0], dirs[d][1], dc, dr) && dc == col && dr == row) {
			return true;
		}
	}
	return false;
}

bool PegsPuzzle::cellAtCursor(const Common::Point &mousePos, int &outCol, int &outRow) const {
	for (int row = 0; row < _numRows; ++row) {
		for (int col = 0; col < _numCols; ++col) {
			int idx = cellIndex(col, row);
			if (_board[idx] != kBlocked &&
					NancySceneState.getViewport().convertViewportToScreen(_destRects[idx]).contains(mousePos)) {
				outCol = col;
				outRow = row;
				return true;
			}
		}
	}
	return false;
}

bool PegsPuzzle::cellHasAnyMove(int col, int row) const {
	static const int dirs[4][2] = { { 0, -1 }, { 0, 1 }, { -1, 0 }, { 1, 0 } };
	for (int d = 0; d < 4; ++d) {
		int dc, dr;
		if (canJump(col, row, dirs[d][0], dirs[d][1], dc, dr)) {
			return true;
		}
	}
	return false;
}

bool PegsPuzzle::anyMovesLeft() const {
	for (int row = 0; row < _numRows; ++row) {
		for (int col = 0; col < _numCols; ++col) {
			if (_board[cellIndex(col, row)] == kPeg && cellHasAnyMove(col, row)) {
				return true;
			}
		}
	}
	return false;
}

int PegsPuzzle::pegCount() const {
	int count = 0;
	for (int row = 0; row < _numRows; ++row) {
		for (int col = 0; col < _numCols; ++col) {
			if (_board[cellIndex(col, row)] == kPeg) {
				++count;
			}
		}
	}
	return count;
}

void PegsPuzzle::doJump(int fromCol, int fromRow, int destCol, int destRow) {
	int midCol = (fromCol + destCol) / 2;
	int midRow = (fromRow + destRow) / 2;
	_board[cellIndex(fromCol, fromRow)] = kEmpty;
	_board[cellIndex(midCol, midRow)] = kEmpty;
	_board[cellIndex(destCol, destRow)] = kPeg;
}

Common::Point PegsPuzzle::cursorToViewport(const Common::Point &mousePos) const {
	Common::Rect screenPt(mousePos.x, mousePos.y, mousePos.x + 1, mousePos.y + 1);
	Common::Rect vpPt = NancySceneState.getViewport().convertScreenToViewport(screenPt);
	return Common::Point(vpPt.left, vpPt.top);
}

void PegsPuzzle::setDataCursor(uint16 cursorType) const {
	// The ids in the AR data are raw Nancy13 cursor types, which is exactly what the
	// "set from script" path expects.
	g_nancy->_cursor->setCursorType((CursorManager::CursorType)cursorType, true);
}

void PegsPuzzle::redraw() {
	_drawSurface.clear(g_nancy->_graphics->getTransColor());

	const bool carrying = (_carriedCol >= 0);

	for (int row = 0; row < _numRows; ++row) {
		for (int col = 0; col < _numCols; ++col) {
			int idx = cellIndex(col, row);
			if (_board[idx] != kPeg) {
				continue;
			}

			// The peg being carried is lifted off the board and drawn at the cursor instead.
			if (carrying && col == _carriedCol && row == _carriedRow) {
				continue;
			}

			const Common::Rect &dest = _destRects[idx];
			_drawSurface.blitFrom(_image, _pegSrc, Common::Point(dest.left, dest.top));
		}
	}

	// The dragged piece, following the cursor.
	if (carrying) {
		int w = _pegSrc.right - _pegSrc.left;
		int h = _pegSrc.bottom - _pegSrc.top;
		_drawSurface.blitFrom(_image, _pegSrc, Common::Point(_dragPos.x - w / 2, _dragPos.y - h / 2));
	}

	_needsRedraw = true;
}

SoundDescription PegsPuzzle::playSoundBlock(const RandomSoundBlock &block) {
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

void PegsPuzzle::execute() {
	switch (_state) {
	case kBegin:
		init();
		_state = kRun;
		// fall through
	case kRun: {
		uint32 now = g_nancy->getTotalPlayTime();

		if (_exitRequested) {
			_state = kActionTrigger;
			break;
		}

		// The original evaluates win/lose only once no legal move remains.
		if (!_ended && _carriedCol < 0 && !anyMovesLeft()) {
			_ended = true;
			_solved = (pegCount() <= _targetPegCount);
			_endSound = playSoundBlock(_solved ? _sounds[3] : _sounds[4]);
			_endTime = now;
			redraw();
		}

		if (_ended) {
			bool soundDone = _endSound.name.empty() || !g_nancy->_sound->isSoundPlaying(_endSound);
			if (soundDone && now - _endTime > 500) {
				_state = kActionTrigger;
			}
		}

		break;
	}
	case kActionTrigger: {
		if (_exitRequested) {
			NancySceneState.setEventFlag(_exitFlag);
			if (_exitScene.sceneID != kNoScene) {
				NancySceneState.changeScene(_exitScene);
			}
		} else {
			const SceneChangeDescription &sc = _solved ? _winScene : _loseScene;
			if (sc.sceneID != kNoScene) {
				NancySceneState.changeScene(sc);
			}
		}

		finishExecution();
		break;
	}
	}
}

void PegsPuzzle::handleInput(NancyInput &input) {
	if (_state != kRun || _ended) {
		return;
	}

	const bool click = (input.input & NancyInput::kLeftMouseButtonUp) != 0;

	// -- Carrying a peg: it follows the cursor; drop it on a reachable hole to jump. --
	if (_carriedCol >= 0) {
		setDataCursor(_dragCursorType);

		// The dragged piece tracks the cursor (viewport space).
		_dragPos = cursorToViewport(input.mousePos);
		redraw();

		if (click) {
			int col, row;
			if (cellAtCursor(input.mousePos, col, row) && isCarriedTarget(col, row)) {
				doJump(_carriedCol, _carriedRow, col, row);
				playSoundBlock(_sounds[1]);
			}

			// The peg is always set down, whether it jumped or not. Chaining another jump
			// is simply picking the landed peg up again.
			_carriedCol = _carriedRow = -1;
			redraw();
		}

		input.eatMouseInput();
		return;
	}

	// -- Not carrying: pick up a movable peg, or leave via the exit hotspot. --
	int col, row;
	if (cellAtCursor(input.mousePos, col, row)) {
		if (_board[cellIndex(col, row)] == kPeg && cellHasAnyMove(col, row)) {
			setDataCursor(_hoverCursorType);
			if (click) {
				_carriedCol = col;
				_carriedRow = row;
				// Seed the drag position before the first draw, otherwise the piece would
				// briefly appear at the previous drag's position.
				_dragPos = cursorToViewport(input.mousePos);
				playSoundBlock(_sounds[0]);
				redraw();
			}
			input.eatMouseInput();
		}
		return;
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
