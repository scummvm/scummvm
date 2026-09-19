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
#include "engines/nancy/cursor.h"
#include "engines/nancy/input.h"
#include "engines/nancy/puzzledata.h"
#include "engines/nancy/sound.h"
#include "engines/nancy/util.h"

#include "engines/nancy/state/scene.h"
#include "engines/nancy/action/puzzle/lettergridpuzzle.h"

namespace Nancy {
namespace Action {

// readRect() makes rects inclusive; the cell and strip layouts step by the raw sizes
static int16 rawWidth(const Common::Rect &rect) {
	return rect.isEmpty() ? 0 : rect.width() - 1;
}

static int16 rawHeight(const Common::Rect &rect) {
	return rect.isEmpty() ? 0 : rect.height() - 1;
}

void LetterGridPuzzle::readData(Common::SeekableReadStream &stream) {
	readFilename(stream, _letterImageName);
	readFilename(stream, _markImageName);

	stream.skip(1);
	_hoverCursorType = stream.readUint16LE();

	readRect(stream, _markSrc);
	_gridOrigin.x = stream.readSint32LE();
	_gridOrigin.y = stream.readSint32LE();
	_spacingY = stream.readSint16LE();
	_spacingX = stream.readSint16LE();

	int16 numColumns = stream.readSint16LE();
	for (int16 i = 0; i < numColumns; ++i) {
		Column column;
		readRect(stream, column.letterSrc);
		int16 numRows = stream.readSint16LE();
		for (int16 j = 0; j < numRows; ++j) {
			column.rows.push_back(stream.readSint16LE());
		}
		_columns.push_back(column);
	}

	int16 numRows = stream.readSint16LE();
	for (int16 i = 0; i < numRows; ++i) {
		_targets.push_back(stream.readSint16LE());
	}

	_stripOrigin.x = stream.readSint32LE();
	_stripOrigin.y = stream.readSint32LE();
	_stripSpacing = stream.readSint16LE();

	_clickSound.readData(stream);

	_solveScene.sceneID = stream.readUint16LE();
	_solveScene.frameID = stream.readUint16LE();
	_solveScene.continueSceneSound = kContinueSceneSound;
	_solveFlag.label = stream.readSint16LE();
	_solveFlag.flag = stream.readByte();

	_solveSound.readData(stream);

	readExitHotspot(stream, _exitHotspot, _exitCursorType, _exitScene, _exitFlag);
	_exitScene.continueSceneSound = kContinueSceneSound;

	// The marks live in a fixed-size global grid
	if (_columns.size() > kMaxColumns) {
		_columns.resize(kMaxColumns);
	}
	if (_targets.size() > kMaxRows) {
		_targets.resize(kMaxRows);
	}
}

void LetterGridPuzzle::init() {
	Common::Rect vpBounds = NancySceneState.getViewport().getBounds();
	_drawSurface.create(vpBounds.width(), vpBounds.height(),
		g_nancy->_graphics->getInputPixelFormat());
	_drawSurface.clear(g_nancy->_graphics->getTransColor());
	setTransparent(true);
	setVisible(true);
	moveTo(vpBounds);

	g_nancy->_resource->loadImage(_letterImageName, _letterImage);
	_letterImage.setTransparentColor(_drawSurface.getTransparentColor());

	if (!_markImageName.empty()) {
		g_nancy->_resource->loadImage(_markImageName, _markImage);
		_markImage.setTransparentColor(_drawSurface.getTransparentColor());
	}

	GridMapPuzzleData *data = (GridMapPuzzleData *)NancySceneState.getPuzzleData(GridMapPuzzleData::getTag());
	assert(data);
	if (data->itemState.size() != kMaxRows) {
		data->itemState.resize(kMaxRows);
		for (uint i = 0; i < kMaxRows; ++i) {
			data->itemState[i] = -1;
		}
	}

	_markedColumn = data->itemState;

	_solved = false;
	_solveSoundStarted = false;
	_exitRequested = false;

	redraw();
}

Common::Rect LetterGridPuzzle::cellRect(uint column, uint row) const {
	int16 width = rawWidth(_markSrc);
	int16 height = rawHeight(_markSrc);
	int16 x = _gridOrigin.x + column * (width + _spacingX);
	int16 y = _gridOrigin.y + row * (height + _spacingY);
	return Common::Rect(x, y, x + width, y + height);
}

Common::Rect LetterGridPuzzle::stripRect(uint row) const {
	if (_columns.empty()) {
		return Common::Rect();
	}

	int16 width = rawWidth(_columns[0].letterSrc);
	int16 height = rawHeight(_columns[0].letterSrc);
	int16 x = _stripOrigin.x + row * (width + _stripSpacing);
	return Common::Rect(x, _stripOrigin.y, x + width, _stripOrigin.y + height);
}

bool LetterGridPuzzle::cellAt(const Common::Point &point, uint &column, uint &row) const {
	for (uint c = 0; c < _columns.size(); ++c) {
		for (uint r = 0; r < _targets.size(); ++r) {
			if (!cellRect(c, r).contains(point)) {
				continue;
			}

			// Only the listed cells of a column are clickable
			const Common::Array<int16> &rows = _columns[c].rows;
			for (uint i = 0; i < rows.size(); ++i) {
				if (rows[i] == (int16)r) {
					column = c;
					row = r;
					return true;
				}
			}
		}
	}

	return false;
}

void LetterGridPuzzle::toggleCell(uint column, uint row) {
	// A row holds at most one mark
	if (_markedColumn[row] == (int16)column) {
		_markedColumn[row] = -1;
	} else {
		_markedColumn[row] = column;
	}

	GridMapPuzzleData *data = (GridMapPuzzleData *)NancySceneState.getPuzzleData(GridMapPuzzleData::getTag());
	data->itemState = _markedColumn;

	playSoundBlock(_clickSound);
	checkSolved();
	redraw();
}

void LetterGridPuzzle::checkSolved() {
	for (uint row = 0; row < _targets.size(); ++row) {
		int16 target = _targets[row];
		if (target == kAnyColumn) {
			continue;
		}

		int16 marked = _markedColumn[row];
		if (marked == -1 ? target != kNoColumn : marked != target) {
			return;
		}
	}

	_solved = true;
}

void LetterGridPuzzle::playSoundBlock(const RandomSoundBlock &block) {
	if (block.names.empty()) {
		return;
	}

	uint idx = block.names.size() == 1 ? 0 : g_nancy->_randomSource->getRandomNumber(block.names.size() - 1);
	const Common::String &name = block.names[idx];
	if (name.empty() || name == "NO SOUND") {
		return;
	}

	SoundDescription desc;
	desc.name = name;
	desc.channelID = block.channel;
	desc.numLoops = block.numLoops > 0 ? block.numLoops : 1;
	desc.volume = block.volume;

	g_nancy->_sound->loadSound(desc);
	g_nancy->_sound->playSound(desc);
}

void LetterGridPuzzle::redraw() {
	_drawSurface.clear(_drawSurface.getTransparentColor());

	for (uint row = 0; row < _targets.size(); ++row) {
		int16 column = _markedColumn[row];
		if (column < 0 || column >= (int16)_columns.size()) {
			continue;
		}

		Common::Rect cell = cellRect(column, row);
		if (!_markImage.empty()) {
			Common::Rect src(_markSrc.left, _markSrc.top, _markSrc.left + cell.width(), _markSrc.top + cell.height());
			_drawSurface.blitFrom(_markImage, src, Common::Point(cell.left, cell.top));
		}

		Common::Rect strip = stripRect(row);
		const Common::Rect &letter = _columns[column].letterSrc;
		Common::Rect src(letter.left, letter.top, letter.left + strip.width(), letter.top + strip.height());
		_drawSurface.blitFrom(_letterImage, src, Common::Point(strip.left, strip.top));
	}

	_needsRedraw = true;
}

void LetterGridPuzzle::execute() {
	switch (_state) {
	case kBegin:
		init();
		registerGraphics();
		_state = kRun;
		// fall through
	case kRun:
		if (_solved || _exitRequested) {
			_state = kActionTrigger;
		}

		break;
	case kActionTrigger:
		if (_exitRequested) {
			NancySceneState.setEventFlag(_exitFlag);
			NancySceneState.changeScene(_exitScene);
			finishExecution();
			break;
		}

		// The solve sound plays out before the scene changes
		if (!_solveSoundStarted) {
			playSoundBlock(_solveSound);
			_solveSoundStarted = true;
			break;
		}

		if (!_solveSound.names.empty() && g_nancy->_sound->isSoundPlaying((uint16)_solveSound.channel)) {
			break;
		}

		NancySceneState.setEventFlag(_solveFlag);
		NancySceneState.changeScene(_solveScene);
		finishExecution();
		break;
	}
}

void LetterGridPuzzle::handleInput(NancyInput &input) {
	if (_state != kRun || _solved || _exitRequested) {
		return;
	}

	if (!_exitHotspot.isEmpty() &&
			NancySceneState.getViewport().convertViewportToScreen(_exitHotspot).contains(input.mousePos)) {
		g_nancy->_cursor->setCursorType((CursorManager::CursorType)_exitCursorType, true);
		if (input.input & NancyInput::kLeftMouseButtonUp) {
			_exitRequested = true;
		}
		input.eatMouseInput();
		return;
	}

	Common::Rect vpScreen = NancySceneState.getViewport().getScreenPosition();
	Common::Point mouseVP = input.mousePos - Common::Point(vpScreen.left, vpScreen.top);

	uint column = 0, row = 0;
	if (!cellAt(mouseVP, column, row)) {
		return;
	}

	g_nancy->_cursor->setCursorType((CursorManager::CursorType)_hoverCursorType, true);
	if (input.input & NancyInput::kLeftMouseButtonUp) {
		toggleCell(column, row);
	}
	input.eatMouseInput();
}

} // End of namespace Action
} // End of namespace Nancy
