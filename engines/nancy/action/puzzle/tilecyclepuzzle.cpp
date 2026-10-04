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

#include "engines/nancy/nancy.h"
#include "engines/nancy/graphics.h"
#include "engines/nancy/input.h"
#include "engines/nancy/util.h"

#include "engines/nancy/state/scene.h"
#include "engines/nancy/action/puzzle/tilecyclepuzzle.h"

namespace Nancy {
namespace Action {

void TileCyclePuzzle::readData(Common::SeekableReadStream &stream) {
	stream.skip(33);	// The name the tiles get saved under; empty in the game data
	_hoverCursor = stream.readUint16LE();
	_turnTime = (uint32)(stream.readDoubleLE() * 1000);
	readFilename(stream, _imageName);

	_templates.resize(stream.readUint16LE());
	for (Template &tileTemplate : _templates) {
		readFilename(stream, tileTemplate.name);

		tileTemplate.states.resize(stream.readUint16LE());
		for (Common::Array<Common::Rect> &frames : tileTemplate.states) {
			frames.resize(stream.readUint16LE());
			for (Common::Rect &frame : frames) {
				readRect(stream, frame);
			}
		}
	}

	_tiles.resize(stream.readUint16LE());
	for (Tile &tile : _tiles) {
		readFilename(stream, tile.name);

		Common::String templateName;
		readFilename(stream, templateName);
		for (uint i = 0; i < _templates.size(); ++i) {
			if (_templates[i].name.equalsIgnoreCase(templateName)) {
				tile.templateID = i;
				break;
			}
		}

		if (tile.templateID == -1) {
			error("TileCyclePuzzle: tile %s has unknown template %s", tile.name.c_str(), templateName.c_str());
		}

		tile.linkedTiles.resize(stream.readUint16LE());
		for (Common::String &linkedTile : tile.linkedTiles) {
			readFilename(stream, linkedTile);
		}

		readRect(stream, tile.dest);
		tile.state = stream.readUint32LE();
		tile.presentFlag = stream.readSint16LE();
	}

	_clickSound.readData(stream);
	_turnedSound.readData(stream);

	_goals.resize(stream.readUint16LE());
	for (Goal &goal : _goals) {
		goal.tileStates.resize(stream.readUint16LE());
		for (TileState &tileState : goal.tileStates) {
			readFilename(stream, tileState.tileName);
			tileState.state = stream.readSint32LE();
		}

		goal.needsAll = stream.readByte();
		goal.flag.label = stream.readSint16LE();
		goal.flag.flag = stream.readByte();
	}

	readExitHotspot(stream);
}

void TileCyclePuzzle::init() {
	initViewportSurface();
	loadImage();
	redraw();
}

bool TileCyclePuzzle::isTilePresent(const Tile &tile) const {
	return tile.presentFlag == -1 || NancySceneState.getEventFlag(tile.presentFlag, g_nancy->_true);
}

TileCyclePuzzle::Tile *TileCyclePuzzle::findTile(const Common::String &name) {
	for (Tile &tile : _tiles) {
		if (tile.name.equalsIgnoreCase(name)) {
			return &tile;
		}
	}

	return nullptr;
}

void TileCyclePuzzle::redraw() {
	_drawSurface.clear(_drawSurface.getTransparentColor());

	for (const Tile &tile : _tiles) {
		if (!isTilePresent(tile)) {
			continue;
		}

		// The game data only has states of a single frame
		const Common::Array<Common::Array<Common::Rect> > &states = _templates[tile.templateID].states;
		if (tile.state < states.size() && !states[tile.state].empty()) {
			_drawSurface.blitFrom(_image, states[tile.state][0], tile.dest.origin());
		}
	}

	_needsRedraw = true;
}

void TileCyclePuzzle::finishTurn() {
	_isTurning = false;
	playSoundBlock(_turnedSound);

	for (Tile &tile : _tiles) {
		if (tile.isTurning) {
			tile.isTurning = false;
			tile.state = (tile.state + 1) % _templates[tile.templateID].states.size();
		}
	}

	updateGoals();
	redraw();
}

void TileCyclePuzzle::updateGoals() {
	for (const Goal &goal : _goals) {
		bool isMet = goal.needsAll;

		for (const TileState &tileState : goal.tileStates) {
			const Tile *tile = findTile(tileState.tileName);
			bool isInState = tile && (int32)tile->state == tileState.state && isTilePresent(*tile);

			if (goal.needsAll) {
				isMet = isMet && isInState;
			} else {
				isMet = isMet || isInState;
			}
		}

		const bool value = isMet == (goal.flag.flag != 0);
		NancySceneState.setEventFlag(goal.flag.label, value ? g_nancy->_true : g_nancy->_false);
	}
}

void TileCyclePuzzle::execute() {
	switch (_state) {
	case kBegin:
		init();
		registerGraphics();
		_state = kRun;
		// fall through
	case kRun:
		if (_exitRequested) {
			_state = kActionTrigger;
		} else if (_isTurning && g_nancy->getTotalPlayTime() >= _turnEndTime) {
			finishTurn();
		}

		break;
	case kActionTrigger:
		_exitScene.execute();
		finishExecution();
		break;
	}
}

void TileCyclePuzzle::handleInput(NancyInput &input) {
	if (_state != kRun || _exitRequested) {
		return;
	}

	Common::Rect viewportBounds = NancySceneState.getViewport().getBounds();
	if (!NancySceneState.getViewport().convertViewportToScreen(viewportBounds).contains(input.mousePos)) {
		return;
	}

	const bool clicked = input.input & NancyInput::kLeftMouseButtonUp;

	if (hoverExitHotspot(input)) {
		if (clicked) {
			_exitRequested = true;
			input.eatMouseInput();
		}

		return;
	}

	// The tiles can't be clicked while some are turning
	if (_isTurning) {
		return;
	}

	Common::Rect mouseRect = NancySceneState.getViewport().convertScreenToViewport(
		Common::Rect(input.mousePos.x, input.mousePos.y, input.mousePos.x + 1, input.mousePos.y + 1));
	Common::Point pos = mouseRect.origin();

	for (Tile &tile : _tiles) {
		if (!tile.dest.contains(pos) || !isTilePresent(tile)) {
			continue;
		}

		setDataCursor(_hoverCursor);

		if (clicked) {
			playSoundBlock(_clickSound);

			tile.isTurning = true;
			for (const Common::String &name : tile.linkedTiles) {
				Tile *linkedTile = findTile(name);
				if (linkedTile) {
					linkedTile->isTurning = true;
				}
			}

			_isTurning = true;
			_turnEndTime = g_nancy->getTotalPlayTime() + _turnTime;
			input.eatMouseInput();
		}

		return;
	}
}

} // End of namespace Action
} // End of namespace Nancy
