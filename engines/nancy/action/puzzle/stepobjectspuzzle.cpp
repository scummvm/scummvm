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
#include "engines/nancy/action/puzzle/stepobjectspuzzle.h"

namespace Nancy {
namespace Action {

// Footprints are ghosted so they read as a trail rather than as more objects
static const byte kFootprintAlpha = 179;

// A footstep sound holds the board for this long before the step is judged
static const uint32 kStepSoundDelay = 300;

void StepObjectsPuzzle::readData(Common::SeekableReadStream &stream) {
	readFilename(stream, _imageName);
	_cursorType = stream.readUint16LE();
	_numRows = stream.readUint16LE();
	_numCols = stream.readUint16LE();
	_pitchY = stream.readUint16LE();
	_pitchX = stream.readUint16LE();
	_solveScene.sceneID = stream.readUint16LE();
	_solveFlag.label = stream.readSint16LE();
	_solveFlag.flag = stream.readByte();
	_numSteps = stream.readUint16LE();

	_solution.resize(_numSteps);
	for (uint i = 0; i < _numSteps; ++i) {
		_solution[i].objectID = stream.readByte();
		_solution[i].row = stream.readSint16LE();
		_solution[i].col = stream.readSint16LE();
	}

	uint16 numObjects = stream.readUint16LE();
	_objects.resize(numObjects);
	for (uint i = 0; i < numObjects; ++i) {
		StepObject &object = _objects[i];
		object.undoWrongStep = stream.readByte() != 0;
		readRect(stream, object.srcRect);
		readRect(stream, object.footprintSrcRect);

		// Only the top left corner is used; the cells are sized from the sprite
		Common::Rect originRect;
		readRect(stream, originRect);
		object.gridOrigin = Common::Point(originRect.left, originRect.top);

		object.startRow = stream.readUint16LE();
		object.startCol = stream.readUint16LE();
	}

	int16 numZones = stream.readSint16LE();
	for (int16 i = 0; i < numZones; ++i) {
		Common::Rect zone;
		readRect(stream, zone);
		uint16 cursorType = stream.readUint16LE();
		uint16 sceneID = stream.readUint16LE();
		int16 exitFlagLabel = stream.readSint16LE();
		byte exitFlagValue = stream.readByte();

		if (i == 0) {
			_exitHotspot = zone;
			_exitCursorType = cursorType;
			_exitScene.sceneID = sceneID;
			// The field after the scene id is a flag label (set on give-up), not a frame.
			_exitScene.frameID = 0;
			_exitFlag.label = exitFlagLabel;
			_exitFlag.flag = exitFlagValue;
		}
	}

	_sounds.resize(kNumSounds);
	for (uint i = 0; i < kNumSounds; ++i) {
		_sounds[i].readData(stream);
	}
}

void StepObjectsPuzzle::init() {
	Common::Rect vpBounds = NancySceneState.getViewport().getBounds();

	// Footprints are drawn with partial alpha, so the surface carries an alpha
	// channel instead of a transparent color key
	_drawSurface.create(vpBounds.width(), vpBounds.height(), g_nancy->_graphics->getTransparentPixelFormat());
	setVisible(true);
	moveTo(vpBounds);

	g_nancy->_resource->loadImage(_imageName, _image);

	resetBoard();

	NancySceneState.setNoHeldItem();

	redraw();
	registerGraphics();
}

Common::Rect StepObjectsPuzzle::getCellRect(const StepObject &object, int row, int col) const {
	int16 left = object.gridOrigin.x + _pitchX * col;
	int16 top = object.gridOrigin.y + _pitchY * row;
	return Common::Rect(left, top, left + object.srcRect.width(), top + object.srcRect.height());
}

bool StepObjectsPuzzle::isHovered(const Common::Rect &viewportRect, const Common::Point &mousePos) const {
	return !viewportRect.isEmpty() &&
		NancySceneState.getViewport().convertViewportToScreen(viewportRect).contains(mousePos);
}

bool StepObjectsPuzzle::cellAtCursor(const StepObject &object, const Common::Point &mousePos, int &outRow, int &outCol) const {
	for (int row = 0; row < _numRows; ++row) {
		for (int col = 0; col < _numCols; ++col) {
			if (isHovered(getCellRect(object, row, col), mousePos)) {
				outRow = row;
				outCol = col;
				return true;
			}
		}
	}

	return false;
}

bool StepObjectsPuzzle::canStepOn(uint objectID, int row, int col) const {
	const StepObject &object = _objects[objectID];

	// An object may always return to the cell it started from; doing so clears the routine
	if (row == object.startRow && col == object.startCol) {
		return true;
	}

	for (uint i = 0; i < _trail.size(); ++i) {
		if (_trail[i].objectID == objectID && _trail[i].row == row && _trail[i].col == col) {
			return false;
		}
	}

	return true;
}

bool StepObjectsPuzzle::isSolutionMatched() const {
	if (_playerSteps.size() != _solution.size()) {
		return false;
	}

	for (uint i = 0; i < _solution.size(); ++i) {
		if (_playerSteps[i].objectID != _solution[i].objectID ||
				_playerSteps[i].row != _solution[i].row ||
				_playerSteps[i].col != _solution[i].col) {
			return false;
		}
	}

	return true;
}

void StepObjectsPuzzle::resetBoard() {
	_trail.clear();
	_playerSteps.clear();
	_carriedID = -1;

	for (uint i = 0; i < _objects.size(); ++i) {
		_objects[i].row = _objects[i].startRow;
		_objects[i].col = _objects[i].startCol;
	}
}

void StepObjectsPuzzle::beginStepSound(SoundID sound, bool isDrop) {
	playSoundBlock(_sounds[sound]);
	_lastStepWasDrop = isDrop;
	_stepSoundEnd = g_nancy->getTotalPlayTime() + kStepSoundDelay;
	_puzzleState = kStepping;
}

void StepObjectsPuzzle::pickUp(uint objectID) {
	StepObject &object = _objects[objectID];

	// The cell being vacated keeps a footprint, so it cannot be used again
	Step step;
	step.objectID = objectID;
	step.row = object.row;
	step.col = object.col;
	_trail.push_back(step);

	_carriedID = objectID;
	_carriedRect = getCellRect(object, object.row, object.col);

	beginStepSound(kSoundPickUp, false);
}

void StepObjectsPuzzle::drop(int row, int col) {
	StepObject &object = _objects[_carriedID];
	int16 prevRow = object.row;
	int16 prevCol = object.col;

	object.row = row;
	object.col = col;

	Step step;
	step.objectID = _carriedID;
	step.row = row;
	step.col = col;
	_playerSteps.push_back(step);

	uint index = _playerSteps.size() - 1;
	bool correct = index < _solution.size() &&
		_solution[index].objectID == step.objectID &&
		_solution[index].row == step.row &&
		_solution[index].col == step.col;

	if (!correct) {
		if (object.undoWrongStep) {
			_playerSteps.pop_back();
			_trail.pop_back();
			object.row = prevRow;
			object.col = prevCol;
		} else if (row == object.startRow && col == object.startCol) {
			playSoundBlock(_sounds[kSoundReset]);
			resetBoard();
			redraw();
			return;
		}
	}

	_carriedID = -1;
	_lastStepCorrect = correct;
	beginStepSound(kSoundStep, true);
	redraw();
}

void StepObjectsPuzzle::drawSprite(const Common::Rect &srcRect, const Common::Point &destPos, byte alpha) {
	if (srcRect.isEmpty() || !_image.getBounds().contains(srcRect)) {
		return;
	}

	// The color key is matched on RGB alone, since an image may carry an alpha channel
	byte tr, tg, tb;
	g_nancy->_graphics->getInputPixelFormat().colorToRGB(g_nancy->_graphics->getTransColor(), tr, tg, tb);

	for (int y = 0; y < srcRect.height(); ++y) {
		int destY = destPos.y + y;
		if (destY < 0 || destY >= _drawSurface.h) {
			continue;
		}

		for (int x = 0; x < srcRect.width(); ++x) {
			int destX = destPos.x + x;
			if (destX < 0 || destX >= _drawSurface.w) {
				continue;
			}

			// An image without an alpha channel reports every pixel opaque and keys on
			// the color instead; one with an alpha channel masks the sprite out with it
			byte a, r, g, b;
			_image.format.colorToARGB(_image.getPixel(srcRect.left + x, srcRect.top + y), a, r, g, b);
			if (a == 0 || (r == tr && g == tg && b == tb)) {
				continue;
			}

			_drawSurface.setPixel(destX, destY, _drawSurface.format.ARGBToColor(a * alpha / 255, r, g, b));
		}
	}
}

void StepObjectsPuzzle::redraw() {
	_drawSurface.clear(0);

	for (uint i = 0; i < _trail.size(); ++i) {
		const Step &step = _trail[i];
		if (step.objectID >= _objects.size()) {
			continue;
		}

		const StepObject &object = _objects[step.objectID];
		Common::Rect cell = getCellRect(object, step.row, step.col);

		if (object.footprintSrcRect.isEmpty()) {
			drawSprite(object.srcRect, Common::Point(cell.left, cell.top), kFootprintAlpha);
		} else {
			drawSprite(object.footprintSrcRect, Common::Point(cell.left, cell.top), 255);
		}
	}

	for (uint i = 0; i < _objects.size(); ++i) {
		if ((int)i == _carriedID) {
			continue;
		}

		const StepObject &object = _objects[i];
		Common::Rect cell = getCellRect(object, object.row, object.col);
		drawSprite(object.srcRect, Common::Point(cell.left, cell.top), 255);
	}

	if (_carriedID >= 0) {
		drawSprite(_objects[_carriedID].srcRect, Common::Point(_carriedRect.left, _carriedRect.top), 255);
	}

	_needsRedraw = true;
}

void StepObjectsPuzzle::setDataCursor(uint16 cursorType) const {
	// The ids in the AR data are raw Nancy13 cursor types, which is exactly what the
	// "set from script" path expects.
	g_nancy->_cursor->setCursorType((CursorManager::CursorType)cursorType, true);
}

SoundDescription StepObjectsPuzzle::playSoundBlock(const RandomSoundBlock &block) {
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

void StepObjectsPuzzle::execute() {
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
		case kIdle:
			if (!_solved && _playerSteps.size() == _numSteps && isSolutionMatched()) {
				_solved = true;
				_solveSound = playSoundBlock(_sounds[kSoundSolved]);
				_puzzleState = kSolved;
			}

			break;
		case kStepping:
			if (g_nancy->getTotalPlayTime() >= _stepSoundEnd) {
				if (_lastStepWasDrop) {
					playSoundBlock(_sounds[_lastStepCorrect ? kSoundCorrectStep : kSoundWrongStep]);
				}

				_puzzleState = kIdle;
			}

			break;
		case kSolved:
			if (_solveSound.name.empty() || !g_nancy->_sound->isSoundPlaying(_solveSound)) {
				_state = kActionTrigger;
			}

			break;
		}

		break;
	case kActionTrigger:
		if (_solved) {
			NancySceneState.setEventFlag(_solveFlag);

			if (_solveScene.sceneID != kNoScene) {
				NancySceneState.changeScene(_solveScene);
			}
		} else {
			NancySceneState.setEventFlag(_exitFlag);
			if (_exitScene.sceneID != kNoScene) {
				NancySceneState.changeScene(_exitScene);
			}
		}

		finishExecution();
		break;
	}
}

void StepObjectsPuzzle::handleInput(NancyInput &input) {
	if (_state != kRun || _solved) {
		return;
	}

	// A footstep sound holds the board: a carried object still follows the
	// cursor, but no further step may be taken until the sound finishes
	const bool click = _puzzleState == kIdle && (input.input & NancyInput::kLeftMouseButtonUp) != 0;

	if (_carriedID >= 0) {
		const StepObject &object = _objects[_carriedID];
		setDataCursor(_cursorType);

		// The carried sprite is centered on the cursor and kept inside the viewport
		Common::Rect vpBounds = NancySceneState.getViewport().getBounds();
		Common::Rect screenPt(input.mousePos.x, input.mousePos.y, input.mousePos.x + 1, input.mousePos.y + 1);
		Common::Rect vpPt = NancySceneState.getViewport().convertScreenToViewport(screenPt);
		int16 w = object.srcRect.width();
		int16 h = object.srcRect.height();
		int16 left = CLIP<int16>(vpPt.left - w / 2, vpBounds.left, vpBounds.right - w);
		int16 top = CLIP<int16>(vpPt.top - h / 2, vpBounds.top, vpBounds.bottom - h);
		_carriedRect = Common::Rect(left, top, left + w, top + h);
		redraw();

		if (click) {
			int row, col;
			if (cellAtCursor(object, input.mousePos, row, col) && canStepOn(_carriedID, row, col)) {
				drop(row, col);
			}
		}

		input.eatMouseInput();
		return;
	}

	for (uint i = 0; i < _objects.size(); ++i) {
		const StepObject &object = _objects[i];

		if (isHovered(getCellRect(object, object.row, object.col), input.mousePos)) {
			setDataCursor(_cursorType);
			if (click) {
				pickUp(i);
				redraw();
			}

			input.eatMouseInput();
			return;
		}

		// Clicking the cell an object started from, once it has moved away, clears the routine
		if (isHovered(getCellRect(object, object.startRow, object.startCol), input.mousePos)) {
			setDataCursor(_cursorType);
			if (click) {
				playSoundBlock(_sounds[kSoundReset]);
				resetBoard();
				redraw();
			}

			input.eatMouseInput();
			return;
		}
	}

	if (isHovered(_exitHotspot, input.mousePos)) {
		setDataCursor(_exitCursorType);
		if (click) {
			_exitRequested = true;
		}
	}
}

} // End of namespace Action
} // End of namespace Nancy
