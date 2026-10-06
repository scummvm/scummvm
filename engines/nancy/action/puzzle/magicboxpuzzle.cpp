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
#include "engines/nancy/cursor.h"
#include "engines/nancy/util.h"

#include "engines/nancy/state/scene.h"
#include "engines/nancy/action/puzzle/magicboxpuzzle.h"

namespace Nancy {
namespace Action {

static void readRectList(Common::SeekableReadStream &stream, Common::Array<Common::Rect> &dst) {
	int16 num = stream.readSint16LE();
	dst.resize(num > 0 ? num : 0);
	for (int16 i = 0; i < num; ++i) {
		readRect(stream, dst[i]);
	}
}

void MagicBoxPuzzle::readData(Common::SeekableReadStream &stream) {
	_gridFlowType = stream.readUint16LE();

	_numCols = stream.readSint32LE();
	_numRows = stream.readSint32LE();
	_subgridCols = stream.readSint32LE();
	_subgridRows = stream.readSint32LE();

	_allCellsBlank = stream.readByte();
	_unknown13 = stream.readByte();
	_allowTakeBack = stream.readByte();

	uint numCells = (uint)MAX<int32>(0, _numRows) * (uint)MAX<int32>(0, _numCols);
	_cellValues.resize(numCells);
	for (uint i = 0; i < numCells; ++i) {
		_cellValues[i] = stream.readSint32LE();
	}

	readRectList(stream, _slotDests);

	uint16 numPieces = stream.readUint16LE();
	_pieceValues.resize(numPieces);
	for (uint16 i = 0; i < numPieces; ++i) {
		_pieceValues[i] = stream.readSint32LE();
	}

	readFilename(stream, _tileImageName);
	readRectList(stream, _tileSrcs);

	readRectList(stream, _trayDests);

	readFilename(stream, _indicatorImageName);
	readRectList(stream, _indicatorSrcs);
	readRectList(stream, _indicatorDests);

	_rowTargets.resize(MAX<int32>(0, _numRows));
	for (uint i = 0; i < _rowTargets.size(); ++i) {
		_rowTargets[i] = stream.readSint32LE();
	}
	_colTargets.resize(MAX<int32>(0, _numCols));
	for (uint i = 0; i < _colTargets.size(); ++i) {
		_colTargets[i] = stream.readSint32LE();
	}

	_hoverCursorType = stream.readUint16LE();
	_dragCursorType = stream.readUint16LE();
	stream.skip(8); // tile-slide animation speed and step

	for (uint i = 0; i < kNumSounds; ++i) {
		_sounds[i].readData(stream);
	}

	_solveScene._sceneChange.sceneID = stream.readUint16LE();
	_solveScene._sceneChange.frameID = stream.readUint16LE();
	_solveScene._sceneChange.continueSceneSound = kContinueSceneSound;
	_solveScene._flag.label = stream.readSint16LE();
	_solveScene._flag.flag = stream.readByte();
	_solveSoundBlock.readData(stream);

	_failScene.sceneID = stream.readUint16LE();
	_failScene.frameID = stream.readUint16LE();
	_failScene.continueSceneSound = kContinueSceneSound;
	_failFlag.label = stream.readSint16LE();
	_failFlag.flag = stream.readByte();
	_failSound.readData(stream);

	readExitHotspot(stream);
	_exitScene._sceneChange.continueSceneSound = kContinueSceneSound;
}

void MagicBoxPuzzle::init() {
	initViewportSurface();

	g_nancy->_resource->loadImage(_tileImageName, _tileImage);
	_tileImage.setTransparentColor(_drawSurface.getTransparentColor());

	if (_indicatorImageName != _tileImageName) {
		g_nancy->_resource->loadImage(_indicatorImageName, _indicatorImage);
		_indicatorImage.setTransparentColor(_drawSurface.getTransparentColor());
	}

	// Slots are the blank cells, in the same order as their dest rects.
	_slotCells.clear();
	for (uint i = 0; i < _cellValues.size(); ++i) {
		if (_allCellsBlank || _cellValues[i] == 0) {
			_slotCells.push_back(i);
		}
	}
	_slotCells.resize(MIN(_slotCells.size(), _slotDests.size()));

	_slotContents.clear();
	_slotContents.resize(_slotCells.size(), -1);
	_piecePlacement.clear();
	_piecePlacement.resize(_pieceValues.size(), -1);

	_carriedPiece = -1;
	_solved = false;
	_exitRequested = false;

	NancySceneState.setNoHeldItem();

	redraw();
	registerGraphics();
	_carriedObject.registerGraphics();
}

Common::Rect MagicBoxPuzzle::tileSrc(int32 value) const {
	if (value >= 1 && (uint)value <= _tileSrcs.size()) {
		return _tileSrcs[value - 1];
	}
	return Common::Rect();
}

int MagicBoxPuzzle::slotAtCursor(const Common::Point &mousePos) const {
	for (uint i = 0; i < _slotCells.size(); ++i) {
		if (NancySceneState.getViewport().convertViewportToScreen(_slotDests[i]).contains(mousePos)) {
			return i;
		}
	}
	return -1;
}

int MagicBoxPuzzle::trayPieceAtCursor(const Common::Point &mousePos) const {
	for (uint i = 0; i < _piecePlacement.size(); ++i) {
		if (_piecePlacement[i] != -1 || i >= _trayDests.size()) {
			continue;
		}
		if (NancySceneState.getViewport().convertViewportToScreen(_trayDests[i]).contains(mousePos)) {
			return i;
		}
	}
	return -1;
}

int32 MagicBoxPuzzle::rowSum(int row) const {
	int32 sum = 0;
	for (int col = 0; col < _numCols; ++col) {
		sum += _cellValues[row * _numCols + col];
	}
	for (uint i = 0; i < _slotContents.size(); ++i) {
		if (_slotContents[i] != -1 && _slotCells[i] / _numCols == row) {
			sum += _pieceValues[_slotContents[i]];
		}
	}
	return sum;
}

int32 MagicBoxPuzzle::colSum(int col) const {
	int32 sum = 0;
	for (int row = 0; row < _numRows; ++row) {
		sum += _cellValues[row * _numCols + col];
	}
	for (uint i = 0; i < _slotContents.size(); ++i) {
		if (_slotContents[i] != -1 && _slotCells[i] % _numCols == col) {
			sum += _pieceValues[_slotContents[i]];
		}
	}
	return sum;
}

bool MagicBoxPuzzle::isSolved() const {
	for (int row = 0; row < _numRows; ++row) {
		if (rowSum(row) != _rowTargets[row]) {
			return false;
		}
	}
	for (int col = 0; col < _numCols; ++col) {
		if (colSum(col) != _colTargets[col]) {
			return false;
		}
	}
	return true;
}

void MagicBoxPuzzle::carryPiece(int piece, NancyInput &input) {
	_carriedPiece = piece;

	Common::Rect src = (piece >= 0) ? tileSrc(_pieceValues[piece]) : Common::Rect();
	if (!src.isEmpty() && _tileImage.getBounds().contains(src)) {
		_carriedObject._drawSurface.create(_tileImage, src);
		_carriedObject.setTransparent(true);
		_carriedObject.setVisible(true);
		_carriedObject.pickUp();
		_carriedObject.handleInput(input);
	} else {
		_carriedObject.setVisible(false);
		_carriedObject.putDown();
	}
}

void MagicBoxPuzzle::redraw() {
	_drawSurface.clear(g_nancy->_graphics->getTransColor());

	// The fixed cells are part of the scene background.
	for (uint i = 0; i < _piecePlacement.size(); ++i) {
		if (i == (uint)_carriedPiece) {
			continue;
		}

		Common::Rect src = tileSrc(_pieceValues[i]);
		if (src.isEmpty()) {
			continue;
		}

		int slot = _piecePlacement[i];
		if (slot == -1 && i >= _trayDests.size()) {
			continue;
		}
		const Common::Rect &dest = (slot == -1) ? _trayDests[i] : _slotDests[slot];
		_drawSurface.blitFrom(_tileImage, src, Common::Point(dest.left, dest.top));
	}

	// A marker lights up beside every row and column that already adds up.
	if (!_indicatorSrcs.empty() && !_indicatorSrcs[0].isEmpty()) {
		const Graphics::ManagedSurface &image = _indicatorImage.empty() ? _tileImage : _indicatorImage;
		for (uint i = 0; i < _indicatorDests.size(); ++i) {
			bool lit = (i < (uint)_numRows) ? (rowSum(i) == _rowTargets[i]) :
				(colSum(i - _numRows) == _colTargets[i - _numRows]);
			if (lit) {
				_drawSurface.blitFrom(image, _indicatorSrcs[0],
					Common::Point(_indicatorDests[i].left, _indicatorDests[i].top));
			}
		}
	}

	_needsRedraw = true;
}

void MagicBoxPuzzle::execute() {
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

		if (_solved) {
			if (_endSound.name.empty() || !g_nancy->_sound->isSoundPlaying(_endSound)) {
				_state = kActionTrigger;
			}
		}

		break;
	case kActionTrigger:
		if (_exitRequested) {
			_exitScene.execute();
		} else {
			_solveScene.execute();
		}

		finishExecution();
		break;
	}
}

void MagicBoxPuzzle::handleInput(NancyInput &input) {
	if (_state != kRun || _solved || _exitRequested) {
		return;
	}

	const bool click = (input.input & NancyInput::kLeftMouseButtonUp) != 0;

	// Carrying a tile: drop it into a free slot, or put it back in the tray.
	if (_carriedPiece >= 0) {
		setDataCursor(_dragCursorType);
		_carriedObject.handleInput(input);

		if (click) {
			int slot = slotAtCursor(input.mousePos);
			if (slot >= 0 && _slotContents[slot] == -1) {
				_slotContents[slot] = _carriedPiece;
				_piecePlacement[_carriedPiece] = slot;
			} else {
				_piecePlacement[_carriedPiece] = -1;
			}

			playSoundBlock(_sounds[3]);
			carryPiece(-1, input);
			redraw();

			if (isSolved()) {
				_solved = true;
				_endSound = playSoundBlock(_solveSoundBlock);
			}
		}

		input.eatMouseInput();
		return;
	}

	// Not carrying: pick a tile up from the tray or off the board.
	int piece = trayPieceAtCursor(input.mousePos);
	if (piece == -1 && _allowTakeBack) {
		int slot = slotAtCursor(input.mousePos);
		if (slot >= 0) {
			piece = _slotContents[slot];
		}
	}

	if (piece >= 0) {
		setDataCursor(_hoverCursorType);
		if (click) {
			int slot = _piecePlacement[piece];
			if (slot != -1) {
				_slotContents[slot] = -1;
				_piecePlacement[piece] = -1;
			}
			carryPiece(piece, input);
			playSoundBlock(_sounds[1]);
			redraw();
		}
		input.eatMouseInput();
		return;
	}

	if (hoverExitHotspot(input)) {
		if (click) {
			_exitRequested = true;
		}
	}
}

} // End of namespace Action
} // End of namespace Nancy
