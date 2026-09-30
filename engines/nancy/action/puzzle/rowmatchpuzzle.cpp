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
#include "engines/nancy/sound.h"
#include "engines/nancy/input.h"

#include "engines/nancy/state/scene.h"
#include "engines/nancy/action/puzzle/rowmatchpuzzle.h"

namespace Nancy {
namespace Action {

void RowMatchPuzzle::readData(Common::SeekableReadStream &stream) {
	readFilename(stream, _imageName);

	_tileSrcs.resize(stream.readSint16LE());
	for (Common::Rect &src : _tileSrcs) {
		readRect(stream, src);
	}

	readRect(stream, _spareDest);
	_spareSolutionTile = getTile(stream.readSint16LE());

	_columns.resize(stream.readSint16LE());
	for (Column &column : _columns) {
		stream.skip(1);

		// The count includes both of the ids in each slot
		column.slots.resize(MAX<int16>(stream.readSint16LE() / 2, 0));
		if (column.slots.empty()) {
			continue;
		}

		for (Slot &slot : column.slots) {
			slot.startTile = getTile(stream.readSint16LE());
			slot.solutionTile = stream.readSint16LE();
			readRect(stream, slot.dest);
		}

		readRect(stream, column.insertHotspot);
		readRect(stream, column.droppedTileDest);
	}

	_insertCursor = stream.readUint16LE();
	_takeCursor = stream.readUint16LE();
	_heldTileCursor = stream.readUint16LE();

	_insertSound.readData(stream);
	_takeSound.readData(stream);

	_solveScene._sceneChange.sceneID = stream.readUint16LE();
	_solveScene._sceneChange.frameID = stream.readUint16LE();
	_solveScene._sceneChange.continueSceneSound = kContinueSceneSound;
	_solveScene._flag.label = stream.readSint16LE();
	_solveScene._flag.flag = stream.readByte();
	_solveSoundBlock.readData(stream);

	// Unlike most puzzles, this one has more than one way out
	readExitHotspots(stream, _exits);
}

void RowMatchPuzzle::init() {
	initViewportSurface();
	loadImage();

	for (Column &column : _columns) {
		for (Slot &slot : column.slots) {
			slot.tile = slot.startTile;
		}

		column.droppedTile = kNoTile;
	}

	_spareTile = _spareSolutionTile;

	NancySceneState.setNoHeldItem();
	redraw();
}

bool RowMatchPuzzle::isSolved() const {
	for (const Column &column : _columns) {
		for (const Slot &slot : column.slots) {
			if (slot.tile == kNoTile || slot.tile != slot.solutionTile) {
				return false;
			}
		}
	}

	return _spareTile != kNoTile && _spareTile == _spareSolutionTile;
}

void RowMatchPuzzle::holdTile(int tile) {
	_heldTile = tile;
}

void RowMatchPuzzle::releaseTile() {
	_heldTile = kNoTile;
}

void RowMatchPuzzle::setCursor(const Common::Point &pos) {
	if (_heldTile != kNoTile) {
		for (const Column &column : _columns) {
			if (column.insertHotspot.contains(pos)) {
				setDataCursor(_insertCursor);
				return;
			}
		}

		if (_spareTile == kNoTile && _spareDest.contains(pos)) {
			setDataCursor(_insertCursor);
		} else {
			setDataCursor(_heldTileCursor, false);
		}

		return;
	}

	for (const Column &column : _columns) {
		if (column.droppedTile != kNoTile && column.droppedTileDest.contains(pos)) {
			setDataCursor(_takeCursor);
			return;
		}
	}

	if (_spareTile != kNoTile && _spareDest.contains(pos)) {
		setDataCursor(_takeCursor);
	}
}

void RowMatchPuzzle::drawTile(int tile, const Common::Point &pos) {
	if (tile != kNoTile) {
		_drawSurface.blitFrom(_image, _tileSrcs[tile], pos);
	}
}

void RowMatchPuzzle::redraw() {
	_drawSurface.clear(_drawSurface.getTransparentColor());

	for (const Column &column : _columns) {
		for (const Slot &slot : column.slots) {
			drawTile(slot.tile, slot.dest.origin());
		}

		drawTile(column.droppedTile, column.droppedTileDest.origin());
	}

	drawTile(_spareTile, _spareDest.origin());

	if (_heldTile != kNoTile) {
		drawTile(_heldTile, getHeldPiecePos(_mousePos, _tileSrcs[_heldTile], Common::Point()));
	}

	_needsRedraw = true;
}

void RowMatchPuzzle::execute() {
	switch (_state) {
	case kBegin:
		init();
		registerGraphics();
		_state = kRun;
		// fall through
	case kRun:
		if (_exitRequested) {
			_state = kActionTrigger;
			break;
		}

		if (_isSolved) {
			if (!isSolveSoundPlaying()) {
				_state = kActionTrigger;
			}

			break;
		}

		if (isSolved()) {
			_isSolved = true;
			if (_heldTile != kNoTile) {
				releaseTile();
			}

			_solveSound = playSoundBlock(_solveSoundBlock);
		}

		redraw();
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

void RowMatchPuzzle::handleInput(NancyInput &input) {
	if (_state != kRun || _isSolved || _exitRequested) {
		return;
	}

	Common::Rect viewportBounds = NancySceneState.getViewport().getBounds();
	if (!NancySceneState.getViewport().convertViewportToScreen(viewportBounds).contains(input.mousePos)) {
		return;
	}

	Common::Rect mouseRect = NancySceneState.getViewport().convertScreenToViewport(
		Common::Rect(input.mousePos.x, input.mousePos.y, input.mousePos.x + 1, input.mousePos.y + 1));
	_mousePos = mouseRect.origin();

	setCursor(_mousePos);

	if (_heldTile == kNoTile) {
		for (const ExitHotspot &exit : _exits) {
			if (!exit.hotspot.contains(_mousePos)) {
				continue;
			}

			setDataCursor(exit.cursorType);
			if (input.input & NancyInput::kLeftMouseButtonUp) {
				_exitScene._sceneChange = exit.scene;
				_exitScene._flag = exit.flag;
				_exitRequested = true;
				input.eatMouseInput();
			}

			return;
		}
	}

	if (!(input.input & NancyInput::kLeftMouseButtonUp)) {
		return;
	}

	if (_heldTile != kNoTile) {
		// Pushing the tile in at the top shifts the column down, and drops out its bottom tile
		for (Column &column : _columns) {
			if (!column.insertHotspot.contains(_mousePos)) {
				continue;
			}

			int carried = _heldTile;
			for (Slot &slot : column.slots) {
				SWAP(carried, slot.tile);
			}

			column.droppedTile = carried;
			playSoundBlock(_insertSound);
			releaseTile();
			input.eatMouseInput();
			return;
		}

		if (_spareTile == kNoTile && _spareDest.contains(_mousePos)) {
			_spareTile = _heldTile;
			releaseTile();
		}
	} else {
		for (Column &column : _columns) {
			if (column.droppedTile != kNoTile && column.droppedTileDest.contains(_mousePos)) {
				holdTile(column.droppedTile);
				column.droppedTile = kNoTile;
				playSoundBlock(_takeSound);
				input.eatMouseInput();
				return;
			}
		}

		if (_spareTile != kNoTile && _spareDest.contains(_mousePos)) {
			holdTile(_spareTile);
			_spareTile = kNoTile;
			playSoundBlock(_takeSound);
		}
	}

	input.eatMouseInput();
}

} // End of namespace Action
} // End of namespace Nancy
