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

#include "engines/nancy/util.h"
#include "engines/nancy/nancy.h"
#include "engines/nancy/graphics.h"
#include "engines/nancy/resource.h"
#include "engines/nancy/sound.h"
#include "engines/nancy/input.h"
#include "engines/nancy/cursor.h"
#include "engines/nancy/puzzledata.h"
#include "engines/nancy/state/scene.h"

#include "engines/nancy/action/puzzle/turningpuzzle.h"

namespace Nancy {
namespace Action {

void TurningPuzzle::init() {
	Common::Rect screenBounds = NancySceneState.getViewport().getBounds();
	_drawSurface.create(screenBounds.width(), screenBounds.height(), g_nancy->_graphics->getInputPixelFormat());
	_drawSurface.clear(g_nancy->_graphics->getTransColor());
	setTransparent(true);
	setVisible(true);
	moveTo(screenBounds);

	g_nancy->_resource->loadImage(_imageName, _image);
	registerGraphics();
}

void TurningPuzzle::updateGraphics() {
	if (_state == kBegin) {
		return;
	}

	if (g_nancy->getGameType() >= kGameTypeNancy13) {
		if (_objectCurrentlyTurning == -1 || g_nancy->getTotalPlayTime() <= _nextTurnTime) {
			return;
		}

		uint framesPerTurn = framesPerTurnOf(_objectCurrentlyTurning);
		++_turnFrameID;

		if (_turnFrameID > framesPerTurn) {
			// The turn is over: commit it, then redraw everything on its new face.
			turnLogic(_objectCurrentlyTurning);
			_objectCurrentlyTurning = -1;
			_turnFrameID = 0;
			_nextTurnTime = 0;
			drawAllObjects();
			return;
		}

		// Step the turning object and everything linked to it through the in-between frames.
		_nextTurnTime = g_nancy->getTotalPlayTime() + (_turnDelay / (framesPerTurn + 1));
		drawObject(_objectCurrentlyTurning, _currentOrder[_objectCurrentlyTurning], _turnFrameID);

		for (uint j = 0; j < _links[_objectCurrentlyTurning].size(); ++j) {
			uint linkedID = _links[_objectCurrentlyTurning][j] - 1;
			if (linkedID < _currentOrder.size()) {
				drawObject(linkedID, _currentOrder[linkedID], _turnFrameID);
			}
		}

		return;
	}

	if (_solveState == kWaitForAnimation) {
		if (g_nancy->getTotalPlayTime() > _nextTurnTime) {
			_nextTurnTime = g_nancy->getTotalPlayTime() + (_solveDelayBetweenTurns * 1000 / _numFramesPerTurn);

			if (	(_turnFrameID == 0 && _solveAnimFace == 0) ||
					(_turnFrameID == 1 && _solveAnimFace > 0 && (int)_solveAnimFace < _numFaces - 1)) {
				g_nancy->_sound->playSound(_turnSound);
			}

			if (_turnFrameID >= _numFramesPerTurn) {
				++_solveAnimFace;
				_turnFrameID = 0;
				_nextTurnTime += 1000 * _solveDelayBetweenTurns;
			}

			for (uint i = 0; i < _currentOrder.size(); ++i) {
				uint faceID = _currentOrder[i] + _solveAnimFace;
				if (faceID >= _numFaces) {
					faceID -= _numFaces;
				}

				drawObject(i, faceID, _turnFrameID);
			}

			if ((int)_solveAnimFace >= _numFaces - 1) {
				_solveAnimFace = 0;
				++_solveAnimLoop;

				if (_solveAnimLoop >= _solveAnimationNumRepeats) {
					_solveState = kWaitBeforeSound;
					_objectCurrentlyTurning = -1;
				}
			}

			++_turnFrameID;
		}

		return;
	}

	if (_objectCurrentlyTurning != -1) {
		if (g_nancy->getTotalPlayTime() > _nextTurnTime) {
			_nextTurnTime = g_nancy->getTotalPlayTime() + (_solveDelayBetweenTurns * 1000 / _numFramesPerTurn);
			++_turnFrameID;

			uint faceID = _currentOrder[_objectCurrentlyTurning];
			uint frameID = _turnFrameID;

			if (frameID == _numFramesPerTurn && (int)faceID == _numFaces - 1) {
				faceID = frameID = 0;
			}

			// Draw clicked spindle
			drawObject(_objectCurrentlyTurning, faceID, frameID);

			// Draw linked spindles
			for (uint i = 0; i < _links[_objectCurrentlyTurning].size(); ++i) {
				faceID = _currentOrder[_links[_objectCurrentlyTurning][i] - 1];
				frameID = _turnFrameID;

				if (frameID == _numFramesPerTurn && (int)faceID == _numFaces - 1) {
					faceID = frameID = 0;
				}

				drawObject(_links[_objectCurrentlyTurning][i] - 1, faceID, frameID);
			}

			if (_turnFrameID >= _numFramesPerTurn) {
				turnLogic(_objectCurrentlyTurning);
				_objectCurrentlyTurning = -1;
				_turnFrameID = 0;
				_nextTurnTime = 0;
			}
		}
	}
}

void TurningPuzzle::readDataNancy13(Common::SeekableReadStream &stream) {
	// 47-byte header
	readFilename(stream, _imageName);			// 0x00
	_turnDelay = stream.readUint16LE();			// 0x21
	_hoverCursorType = stream.readUint16LE();	// 0x23
	_hitInset = stream.readUint16LE();			// 0x25
	_turnFlagLabel = stream.readSint16LE();		// 0x27
	_turnFlagValue = stream.readByte();			// 0x29
	stream.skip(5);								// 0x2a - not yet identified

	// A count-prefixed array of 23-byte hotspot records (as in PegsPuzzle); the first is
	// the "give up" hotspot, and its scene doubles as the one shown once solved.
	int16 numZones = stream.readSint16LE();
	for (int16 i = 0; i < numZones; ++i) {
		Common::Rect r;
		readRect(stream, r);
		uint16 cursorType = stream.readUint16LE();
		uint16 sceneID = stream.readUint16LE();
		uint16 frameID = stream.readUint16LE();
		stream.skip(1);

		if (i == 0) {
			_exitHotspot = r;
			_exitCursorType = cursorType;
			_exitScene._sceneChange.sceneID = sceneID;
			// 0xffff means "no frame" - the target scene may be a video.
			_exitScene._sceneChange.frameID = (frameID == 0xffff) ? 0 : frameID;
			_solveScene._sceneChange = _exitScene._sceneChange;
		}
	}

	uint16 numTypes = stream.readUint16LE();
	_pieceTypes.resize(numTypes);
	for (uint i = 0; i < numTypes; ++i) {
		PieceType &t = _pieceTypes[i];
		t.numFaces = stream.readByte();
		t.framesPerTurn = stream.readSint16LE();
		t.gap = stream.readSint16LE();
		t.cellW = stream.readSint16LE();
		t.cellH = stream.readSint16LE();
		t.srcStartX = stream.readSint16LE();
		t.srcStartY = stream.readSint16LE();
	}

	uint16 numObjects = stream.readUint16LE();
	_links.resize(numObjects);
	_pieceTypeIDs.resize(numObjects);
	_startPositions.resize(numObjects);
	_destRects.resize(numObjects);
	_correctOrders.resize(3);
	for (uint n = 0; n < 3; ++n) {
		_correctOrders[n].resize(numObjects);
	}

	for (uint i = 0; i < numObjects; ++i) {
		uint16 numLinks = stream.readUint16LE();
		for (uint16 j = 0; j < numLinks; ++j) {
			byte link = stream.readByte();
			if (link != 0) {
				// 1-based, as in the older games
				_links[i].push_back(link);
			}
		}

		_pieceTypeIDs[i] = stream.readUint16LE();
		_startPositions[i] = stream.readUint16LE();
		for (uint n = 0; n < 3; ++n) {
			_correctOrders[n][i] = stream.readUint16LE();
		}
		readRect(stream, _destRects[i]);
	}

	// Only the middle of an object is clickable.
	_hotspots = _destRects;
	for (uint i = 0; i < _hotspots.size(); ++i) {
		_hotspots[i].grow(-(int16)_hitInset);
	}

	_turnSoundBlock.readData(stream);
	_solveSoundBlock.readData(stream);
}

uint TurningPuzzle::numFacesOf(uint objectID) const {
	if (g_nancy->getGameType() < kGameTypeNancy13) {
		return _numFaces;
	}
	return _pieceTypes[_pieceTypeIDs[objectID]].numFaces;
}

uint TurningPuzzle::framesPerTurnOf(uint objectID) const {
	if (g_nancy->getGameType() < kGameTypeNancy13) {
		return _numFramesPerTurn;
	}
	return _pieceTypes[_pieceTypeIDs[objectID]].framesPerTurn;
}

bool TurningPuzzle::isSolved() const {
	if (g_nancy->getGameType() < kGameTypeNancy13) {
		return _currentOrder == _correctOrder;
	}

	// Any one of the (up to three) alternative orders solves it. Unused solutions are
	// filled with 0xffff, which no face can ever match.
	for (uint n = 0; n < _correctOrders.size(); ++n) {
		bool match = true;
		for (uint i = 0; i < _currentOrder.size(); ++i) {
			if (_currentOrder[i] != _correctOrders[n][i]) {
				match = false;
				break;
			}
		}

		if (match) {
			return true;
		}
	}

	return false;
}

void TurningPuzzle::drawAllObjects() {
	for (uint i = 0; i < _currentOrder.size(); ++i) {
		drawObject(i, _currentOrder[i], 0);
	}
}

SoundDescription TurningPuzzle::playSoundBlock(const RandomSoundBlock &block) {
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

void TurningPuzzle::readData(Common::SeekableReadStream &stream) {
	if (g_nancy->getGameType() >= kGameTypeNancy13) {
		readDataNancy13(stream);
		return;
	}

	readFilename(stream, _imageName);
	uint numSpindles = stream.readUint16LE();
	_numFaces = stream.readUint16LE();
	_numFramesPerTurn = stream.readUint16LE();
	if (_numFramesPerTurn == 0) {
		error("TurningPuzzle::readData(): _numFramesPerTurn is 0");
	}

	_startPositions.resize(numSpindles);
	for (uint i = 0; i < numSpindles; ++i) {
		_startPositions[i] = stream.readUint16LE();
	}
	stream.skip((16 - numSpindles) * 2);

	readRectArray(stream, _destRects, numSpindles, 16);
	readRectArray(stream, _hotspots, numSpindles, 16);

	_separateRows = stream.readByte();

	_startPos.x = stream.readSint32LE();
	_startPos.y = stream.readSint32LE();
	_srcIncrement.x = stream.readSint16LE();
	_srcIncrement.y = stream.readSint16LE();

	_links.resize(numSpindles);
	for (uint i = 0; i < numSpindles; ++i) {
		for (uint j = 0; j < 4; ++j) {
			uint16 val = stream.readUint16LE();
			if (val == 0) {
				break;
			}

			_links[i].push_back(val);
		}

		if (_links[i].size() < 4) {
			stream.skip((4 - _links[i].size() - 1) * 2);
		}
	}

	stream.skip((16 - numSpindles) * 4 * 2);

	_solveDelayBetweenTurns = stream.readUint16LE();
	_solveAnimate = stream.readByte();
	_solveAnimationNumRepeats = stream.readUint16LE();

	_turnSound.readNormal(stream);

	_correctOrder.resize(numSpindles);
	for (uint i = 0; i < numSpindles; ++i) {
		_correctOrder[i] = stream.readUint16LE();
	}
	stream.skip((16 - numSpindles) * 2);

	if (g_nancy->getGameType() >= kGameTypeNancy12) {
		// Nancy 12 inserts 3 bytes here (zero in the samples seen); purpose unknown.
		stream.skip(3);
	}

	_solveScene.readData(stream);
	_solveSoundDelay = stream.readUint16LE();
	_solveSound.readNormal(stream);

	_exitScene.readData(stream);
	readRect(stream, _exitHotspot);

	if (g_nancy->getGameType() >= kGameTypeNancy12) {
		// Nancy 12 appends a uint16 here (5 in the sample seen); purpose unknown.
		stream.skip(2);
	}
}

void TurningPuzzle::execute() {
	switch (_state) {
	case kBegin :
		init();
		if (g_nancy->getGameType() < kGameTypeNancy13) {
			// Nancy13 picks its turn sound out of a random block on every turn instead.
			g_nancy->_sound->loadSound(_turnSound);
		}
		_currentOrder = _startPositions;
		drawAllObjects();

		NancySceneState.setNoHeldItem();

		_state = kRun;
		// fall through
	case kRun :
		if (_objectCurrentlyTurning != -1) {
			return;
		}

		if (isSolved()) {
			_state = kActionTrigger;
			if (g_nancy->getGameType() >= kGameTypeNancy13) {
				_solveSound = playSoundBlock(_solveSoundBlock);
				_solveState = kWaitForSound;
			} else if (_solveAnimate) {
				_solveState = kWaitForAnimation;
			} else {
				_solveState = kWaitForSound;
				NancySceneState.setEventFlag(_solveScene._flag);
			}
			_objectCurrentlyTurning = -1;
			_turnFrameID = 0;
		}

		break;
	case kActionTrigger :
		switch (_solveState) {
		case kWaitForAnimation :
			if (_nextTurnTime == 0) {
				_solveState = kWaitForSound;
			}
			return;
		case kWaitBeforeSound :
			if (_solveSoundDelayTime == 0) {
				_solveSoundDelayTime = g_nancy->getTotalPlayTime() + (_solveSoundDelay * 1000);
			} else if (g_nancy->getTotalPlayTime() > _solveSoundDelayTime) {
				g_nancy->_sound->loadSound(_solveSound);
				g_nancy->_sound->playSound(_solveSound);
				NancySceneState.setEventFlag(_solveScene._flag);
				_solveState = kWaitForSound;
			}

			return;
		case kWaitForSound :
			if (g_nancy->_sound->isSoundPlaying(_solveSound) || g_nancy->_sound->isSoundPlaying(_turnSound)) {
				return;
			}

			NancySceneState.changeScene(_solveScene._sceneChange);
			break;
		case kNotSolved :
			_exitScene.execute();
		}

		g_nancy->_sound->stopSound(_turnSound);
		g_nancy->_sound->stopSound(_solveSound);
		finishExecution();
	}
}

void TurningPuzzle::handleInput(NancyInput &input) {
	const bool isNancy13 = g_nancy->getGameType() >= kGameTypeNancy13;

	if (NancySceneState.getViewport().convertViewportToScreen(_exitHotspot).contains(input.mousePos)) {
		if (isNancy13)
			g_nancy->_cursor->setCursorType((CursorManager::CursorType)_exitCursorType, true);
		else
			g_nancy->_cursor->setCursorType(g_nancy->_cursor->_puzzleExitCursor);

		if (input.input & NancyInput::kLeftMouseButtonUp)
			_state = kActionTrigger;

		return;
	}

	for (uint i = 0; i < _hotspots.size(); ++i) {
		if (!NancySceneState.getViewport().convertViewportToScreen(_hotspots[i]).contains(input.mousePos))
			continue;

		if (isNancy13)
			g_nancy->_cursor->setCursorType((CursorManager::CursorType)_hoverCursorType, true);
		else
			g_nancy->_cursor->setCursorType(CursorManager::kHotspot);

		if (_objectCurrentlyTurning != -1)
			break;

		if (input.input & NancyInput::kLeftMouseButtonUp) {
			if (isNancy13) {
				// The original sets this flag the first time the player turns anything.
				if (_turnFlagLabel != -1 && !_turnFlagSet) {
					NancySceneState.setEventFlag(_turnFlagLabel,
						_turnFlagValue ? g_nancy->_true : g_nancy->_false);
					_turnFlagSet = true;
				}

				// Nancy13 picks a fresh turn sound out of a random block each time.
				_turnSound = playSoundBlock(_turnSoundBlock);

				// Start the turn animation from a clean frame counter. (Older games
				// leave these untouched here - they are already 0 when a spindle is
				// clickable, and resetting them would disturb the solve animation).
				_turnFrameID = 0;
				_nextTurnTime = 0;
			} else {
				g_nancy->_sound->playSound(_turnSound);
			}

			_objectCurrentlyTurning = i;
		}

		// fixes nancy4 scene 4308
		input.eatMouseInput();
		return;
	}
}

void TurningPuzzle::drawObject(uint objectID, uint faceID, uint frameID) {
	if (g_nancy->getGameType() >= kGameTypeNancy13) {
		// The strip is a grid: one row per object type, with its faces laid out along X,
		// each face taking (framesPerTurn + 1) frames.
		const PieceType &t = _pieceTypes[_pieceTypeIDs[objectID]];
		int stride = t.gap + t.cellW;
		int step = (t.framesPerTurn + 1) * (int)faceID + (int)frameID;

		Common::Rect srcRect;
		srcRect.left = t.srcStartX + stride * step;
		srcRect.top = t.srcStartY;
		srcRect.right = srcRect.left + t.cellW;
		srcRect.bottom = srcRect.top + t.cellH;

		// Clear the object's cell first: unlike the older path (opaque sprites that fully
		// cover their cell), the Nancy13 pipe sprites are transparent, so the previous
		// frame would otherwise show through underneath.
		_drawSurface.fillRect(_destRects[objectID], _drawSurface.getTransparentColor());
		_drawSurface.blitFrom(_image, srcRect, _destRects[objectID]);
		_needsRedraw = true;
		return;
	}

	Common::Rect srcRect = _destRects[objectID];
	srcRect.moveTo(_startPos);
	Common::Point inc(_srcIncrement.x == 1 ? srcRect.width() : _srcIncrement.x, _srcIncrement.y == -2 ? srcRect.height() : _srcIncrement.y);
	srcRect.translate(	inc.x * frameID + inc.x * _numFramesPerTurn * faceID,
						_separateRows ? inc.y * objectID : 0);

	_drawSurface.blitFrom(_image, srcRect, _destRects[objectID]);
	_needsRedraw = true;
}

void TurningPuzzle::turnLogic(uint objectID) {
	++_currentOrder[objectID];
	if (_currentOrder[objectID] >= numFacesOf(objectID)) {
		_currentOrder[objectID] = 0;
	}

	for (uint j = 0; j < _links[objectID].size(); ++j) {
		uint linkedID = _links[objectID][j] - 1;
		if (linkedID >= _currentOrder.size()) {
			continue;
		}

		++_currentOrder[linkedID];
		if (_currentOrder[linkedID] >= numFacesOf(linkedID)) {
			_currentOrder[linkedID] = 0;
		}
	}
}

} // End of namespace Action
} // End of namespace Nancy
