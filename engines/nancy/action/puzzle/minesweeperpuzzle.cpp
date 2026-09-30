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
#include "engines/nancy/font.h"
#include "engines/nancy/util.h"

#include "engines/nancy/state/scene.h"
#include "engines/nancy/action/puzzle/minesweeperpuzzle.h"

namespace Nancy {
namespace Action {

void MinesweeperPuzzle::readTool(Common::SeekableReadStream &stream, Tool &tool) {
	readRect(stream, tool.idleSrc);
	readRect(stream, tool.activeSrc);
	tool.cursor = stream.readUint16LE();
	tool.hotspot.x = stream.readSint32LE();
	tool.hotspot.y = stream.readSint32LE();
	tool.pickUpSound.readData(stream);
	tool.putDownSound.readData(stream);
}

void MinesweeperPuzzle::readEnding(Common::SeekableReadStream &stream, Ending &ending) {
	ending.scene._sceneChange.sceneID = stream.readUint16LE();
	ending.scene._sceneChange.frameID = stream.readUint16LE();
	ending.scene._sceneChange.continueSceneSound = kContinueSceneSound;
	ending.scene._flag.label = stream.readSint16LE();
	ending.scene._flag.flag = stream.readByte();
	ending.sound.readData(stream);
}

void MinesweeperPuzzle::readData(Common::SeekableReadStream &stream) {
	stream.skip(33);
	_maxCracks = stream.readUint32LE();

	readFilename(stream, _toolImageName);
	readTool(stream, _shovel);
	readTool(stream, _cone);
	_hoverCursor = stream.readUint16LE();

	readRect(stream, _shovelHotspot);
	readRect(stream, _coneHotspot);
	readRect(stream, _coneCountDest);
	_coneCountColor = stream.readUint32BE();
	readFilename(stream, _coneCountFontName);

	_digSound.readData(stream);
	_crackSound.readData(stream);
	_digNotAllowedSound.readData(stream);
	_coneNotAllowedSound.readData(stream);

	readEnding(stream, _solveEnding);
	readEnding(stream, _failEnding);

	for (uint i = 0; i < ARRAYSIZE(_tileSrcs); ++i) {
		readRect(stream, _tileSrcs[i]);
	}

	readRect(stream, _coneSrc);
	readFilename(stream, _boardImageName);

	// Clicks are tested without the cells' right and bottom edges, so neighbors don't overlap
	Common::Rect firstCell;
	readExclusiveRect(stream, firstCell);
	int32 stepX = firstCell.width() + 1 + stream.readSint32LE();
	int32 stepY = firstCell.height() + 1 + stream.readSint32LE();

	// The board is drawn as rows of '-' for solid ice and '*' for thin ice
	uint16 numRows = stream.readUint16LE();
	Common::Array<Common::String> rows(numRows);
	for (Common::String &row : rows) {
		readFilename(stream, row);
	}

	_height = numRows;
	_width = numRows ? rows[0].size() : 0;
	if (_width == 0) {
		error("MinesweeperPuzzle: empty board");
	}

	_cells.resize(_width * _height);
	for (uint y = 0; y < _height; ++y) {
		if (rows[y].size() != _width) {
			error("MinesweeperPuzzle: board rows have different lengths");
		}

		for (uint x = 0; x < _width; ++x) {
			Cell &cell = _cells[y * _width + x];
			cell.dest = firstCell;
			cell.dest.translate(x * stepX, y * stepY);
			cell.type = rows[y][x] == '*' ? kThinIce : kClear;
		}
	}

	for (uint i = 0; i < _cells.size(); ++i) {
		if (_cells[i].type == kThinIce) {
			continue;
		}

		Common::Array<int> neighbors;
		getNeighbors(i, neighbors);
		for (int neighbor : neighbors) {
			if (_cells[neighbor].type == kThinIce) {
				++_cells[i].type;
			}
		}
	}

	readExitHotspot(stream);
}

void MinesweeperPuzzle::init() {
	initViewportSurface();

	g_nancy->_resource->loadImage(_toolImageName, _toolImage);
	_toolImage.setTransparentColor(_drawSurface.getTransparentColor());
	g_nancy->_resource->loadImage(_boardImageName, _boardImage);
	_boardImage.setTransparentColor(_drawSurface.getTransparentColor());

	// There are exactly as many cones as there are patches of thin ice
	_conesLeft = 0;
	for (const Cell &cell : _cells) {
		if (cell.type == kThinIce) {
			++_conesLeft;
		}
	}

	NancySceneState.setNoHeldItem();
	redraw();
}

int MinesweeperPuzzle::cellAt(const Common::Point &pos) const {
	for (uint i = 0; i < _cells.size(); ++i) {
		if (_cells[i].dest.contains(pos)) {
			return i;
		}
	}

	return -1;
}

void MinesweeperPuzzle::getNeighbors(int cell, Common::Array<int> &neighbors) const {
	int x = cell % _width;
	int y = cell / _width;

	for (int dy = -1; dy <= 1; ++dy) {
		for (int dx = -1; dx <= 1; ++dx) {
			if (dx == 0 && dy == 0) {
				continue;
			}

			int nx = x + dx;
			int ny = y + dy;
			if (nx >= 0 && ny >= 0 && nx < (int)_width && ny < (int)_height) {
				neighbors.push_back(ny * _width + nx);
			}
		}
	}
}

// Cells on the edge of the pond can always be reached; the others only
// once one of their neighbors has been cleared
bool MinesweeperPuzzle::isReachable(int cell) const {
	Common::Array<int> neighbors;
	getNeighbors(cell, neighbors);
	if (neighbors.size() != 8) {
		return true;
	}

	for (int neighbor : neighbors) {
		if (isRevealed(neighbor)) {
			return true;
		}
	}

	return false;
}

MinesweeperPuzzle::DigResult MinesweeperPuzzle::dig(const Common::Point &pos) {
	int id = cellAt(pos);
	if (id == -1) {
		return kOffBoard;
	}

	if (!isReachable(id)) {
		return kNotAllowed;
	}

	Cell &cell = _cells[id];
	if (cell.shownType == kThinIce) {
		return kFellThrough;
	}

	if (cell.hasCone || isRevealed(id)) {
		return kNotAllowed;
	}

	cell.shownType = cell.type;
	return cell.type == kThinIce ? kCracked : kDug;
}

MinesweeperPuzzle::ConeResult MinesweeperPuzzle::toggleCone(const Common::Point &pos) {
	int id = cellAt(pos);
	if (id == -1) {
		return kConeOffBoard;
	}

	if (!isReachable(id)) {
		return kConeNotAllowed;
	}

	Cell &cell = _cells[id];
	if (cell.shownType == kCovered || cell.shownType == kThinIce) {
		cell.hasCone = !cell.hasCone;
		return cell.hasCone ? kConePlaced : kConeRemoved;
	}

	if (!cell.hasCone) {
		return kConeNotAllowed;
	}

	cell.hasCone = false;
	return kConeRemoved;
}

bool MinesweeperPuzzle::removeCone(const Common::Point &pos) {
	int id = cellAt(pos);
	if (id == -1 || !_cells[id].hasCone) {
		return false;
	}

	_cells[id].hasCone = false;
	return true;
}

// Solved once every patch of thin ice has a cone on it, at which point the rest of the ice is cleared
bool MinesweeperPuzzle::isSolved() {
	for (const Cell &cell : _cells) {
		if (cell.type == kThinIce && !cell.hasCone) {
			return false;
		}
	}

	for (Cell &cell : _cells) {
		if (cell.shownType != cell.type && !cell.hasCone) {
			cell.shownType = cell.type;
			cell.hasCone = false;
		}
	}

	return true;
}

void MinesweeperPuzzle::handleDigResult(DigResult result) {
	switch (result) {
	case kNotAllowed:
		playSoundBlock(_digNotAllowedSound);
		break;
	case kFellThrough:
		startEnding(_failEnding);
		break;
	case kCracked:
		if (++_cracks > _maxCracks) {
			startEnding(_failEnding);
		} else {
			playSoundBlock(_crackSound);
		}

		break;
	case kDug:
		if (isSolved()) {
			startEnding(_solveEnding);
		} else {
			playSoundBlock(_digSound);
		}

		break;
	default:
		break;
	}
}

void MinesweeperPuzzle::handleConeResult(ConeResult result) {
	switch (result) {
	case kConePlaced:
		playSoundBlock(_cone.putDownSound);
		if (--_conesLeft == 0) {
			if (_cone.isHeld) {
				putDownTool(_cone);
			}

			if (isSolved()) {
				startEnding(_solveEnding);
			}
		}

		break;
	case kConeRemoved:
		playSoundBlock(_cone.pickUpSound);
		++_conesLeft;
		break;
	case kConeNotAllowed:
		playSoundBlock(_coneNotAllowedSound);
		break;
	default:
		break;
	}
}

void MinesweeperPuzzle::pickUpTool(Tool &tool) {
	tool.isHeld = true;
	playSoundBlock(tool.pickUpSound);
}

void MinesweeperPuzzle::putDownTool(Tool &tool) {
	tool.isHeld = false;
	playSoundBlock(tool.putDownSound);
}

void MinesweeperPuzzle::startEnding(const Ending &ending) {
	_isEnding = true;
	_endScene = ending.scene;
	_endSound = playSoundBlock(ending.sound);

	if (_shovel.isHeld) {
		putDownTool(_shovel);
	}

	if (_cone.isHeld) {
		putDownTool(_cone);
	}
}

void MinesweeperPuzzle::setCursor(const Common::Point &pos) {
	if (_shovel.isHeld) {
		setDataCursor(_shovel.cursor);
		return;
	}

	if (_cone.isHeld) {
		setDataCursor(_cone.cursor);
		return;
	}

	int id = cellAt(pos);
	if ((id != -1 && _cells[id].hasCone) || _shovelHotspot.contains(pos) ||
			(_coneHotspot.contains(pos) && _conesLeft > 0)) {
		setDataCursor(_hoverCursor);
	}
}

// Some tiles lie partly or wholly outside the board image, e.g. the covered one; only
// the part inside it gets drawn
void MinesweeperPuzzle::drawTile(const Common::Rect &src, const Common::Point &dest) {
	Common::Rect clipped = src.findIntersectingRect(Common::Rect(_boardImage.w, _boardImage.h));
	if (clipped.isEmpty()) {
		return;
	}

	_drawSurface.blitFrom(_boardImage, clipped, dest + (clipped.origin() - src.origin()));
}

void MinesweeperPuzzle::redraw() {
	_drawSurface.clear(_drawSurface.getTransparentColor());

	if (!_shovel.isHeld) {
		_drawSurface.blitFrom(_toolImage, _shovel.idleSrc, _shovelHotspot.origin());
	}

	if (!_cone.isHeld && _conesLeft > 0) {
		_drawSurface.blitFrom(_toolImage, _cone.idleSrc, _coneHotspot.origin());
	}

	const Font *countFont = g_nancy->_graphics->getFont(_coneCountFontName);
	if (countFont && countFont->getSystemFont()) {
		uint32 color = argbToColor(_drawSurface.format, _coneCountColor);
		countFont->getSystemFont()->drawString(_drawSurface.surfacePtr(), Common::String::format("%d", _conesLeft),
			_coneCountDest.left, _coneCountDest.top, _coneCountDest.width(), color);
	}

	for (const Cell &cell : _cells) {
		drawTile(_tileSrcs[cell.shownType], cell.dest.origin());
		if (cell.hasCone) {
			drawTile(_coneSrc, cell.dest.origin());
		}
	}

	// A held tool takes the place of the cursor, and changes when it can be used where it points
	const Tool *heldTool = _shovel.isHeld ? &_shovel : (_cone.isHeld ? &_cone : nullptr);
	if (heldTool) {
		bool usable = false;
		int id = cellAt(_mousePos);
		if (id != -1 && isReachable(id)) {
			usable = !isRevealed(id) || (heldTool == &_cone && _cells[id].type == kThinIce);
		}

		const Common::Rect &src = usable ? heldTool->activeSrc : heldTool->idleSrc;
		_drawSurface.blitFrom(_toolImage, src, getHeldPiecePos(_mousePos, src, heldTool->hotspot));
	}

	_needsRedraw = true;
}

void MinesweeperPuzzle::execute() {
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

		if (_isEnding && (_endSound.name.empty() || !g_nancy->_sound->isSoundPlaying(_endSound))) {
			_state = kActionTrigger;
		}

		redraw();
		break;
	case kActionTrigger:
		if (_exitRequested) {
			_exitScene.execute();
		} else {
			_endScene.execute();
		}

		finishExecution();
		break;
	}
}

void MinesweeperPuzzle::handleInput(NancyInput &input) {
	if (_state != kRun || _isEnding || _exitRequested) {
		return;
	}

	Common::Rect viewportBounds = NancySceneState.getViewport().getBounds();
	if (!NancySceneState.getViewport().convertViewportToScreen(viewportBounds).contains(input.mousePos)) {
		return;
	}

	Common::Rect mouseRect = NancySceneState.getViewport().convertScreenToViewport(
		Common::Rect(input.mousePos.x, input.mousePos.y, input.mousePos.x + 1, input.mousePos.y + 1));
	Common::Point pos = mouseRect.origin();
	_mousePos = pos;

	setCursor(pos);

	// Right clicking uses the tool that isn't held
	if (input.input & NancyInput::kRightMouseButtonUp) {
		if (_cone.isHeld) {
			DigResult result = dig(pos);
			if (result != kOffBoard) {
				handleDigResult(result);
			}
		} else if (_shovel.isHeld) {
			if (_conesLeft > 0) {
				ConeResult result = toggleCone(pos);
				if (result != kConeOffBoard) {
					handleConeResult(result);
				}
			} else if (removeCone(pos)) {
				playSoundBlock(_cone.pickUpSound);
				++_conesLeft;
			}
		}

		input.eatMouseInput();
		return;
	}

	if (input.input & NancyInput::kLeftMouseButtonUp) {
		bool shovelWasHeld = _shovel.isHeld;
		bool coneWasHeld = _cone.isHeld;

		if (shovelWasHeld) {
			DigResult result = dig(pos);
			if (result != kOffBoard) {
				handleDigResult(result);
				input.eatMouseInput();
				return;
			}

			putDownTool(_shovel);
		}

		if (coneWasHeld) {
			ConeResult result = toggleCone(pos);
			if (result != kConeOffBoard) {
				handleConeResult(result);
				input.eatMouseInput();
				return;
			}

			putDownTool(_cone);
		}

		if (_shovelHotspot.contains(pos) && !shovelWasHeld) {
			pickUpTool(_shovel);
		} else if (_coneHotspot.contains(pos) && !coneWasHeld && _conesLeft > 0) {
			pickUpTool(_cone);
		} else if (removeCone(pos)) {
			// Picking up a cone from the ice
			++_conesLeft;
			pickUpTool(_cone);
		} else if (!shovelWasHeld && !coneWasHeld && hoverExitHotspot(input)) {
			_exitRequested = true;
		}

		input.eatMouseInput();
		return;
	}

	if (!_shovel.isHeld && !_cone.isHeld) {
		hoverExitHotspot(input);
	}
}

} // End of namespace Action
} // End of namespace Nancy
