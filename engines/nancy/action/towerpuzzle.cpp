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

#include "engines/nancy/util.h"
#include "engines/nancy/nancy.h"
#include "engines/nancy/graphics.h"
#include "engines/nancy/resource.h"
#include "engines/nancy/sound.h"
#include "engines/nancy/input.h"
#include "engines/nancy/state/scene.h"

#include "engines/nancy/action/towerpuzzle.h"

namespace Nancy {
namespace Action {

void TowerPuzzle::init() {
	Common::Rect screenBounds = NancySceneState.getViewport().getBounds();
	_drawSurface.create(screenBounds.width(), screenBounds.height(), g_nancy->_graphicsManager->getInputPixelFormat());
	_drawSurface.clear(g_nancy->_graphicsManager->getTransColor());
	setTransparent(true);
	setVisible(true);
	moveTo(screenBounds);

	g_nancy->_resource->loadImage(_imageName, _image);
	_image.setTransparentColor(_drawSurface.getTransparentColor());
}

void TowerPuzzle::registerGraphics() {
	_heldRing.registerGraphics();
	RenderObject::registerGraphics();
}

void TowerPuzzle::readData(Common::SeekableReadStream &stream) {
	_puzzleState = NancySceneState._towerPuzzleState;
	assert(_puzzleState);

	readFilename(stream, _imageName);

	_numRingsByDifficulty.resize(3);
	for (uint i = 0; i < 3; ++i) {
		_numRingsByDifficulty[i] = stream.readUint16LE();
	}

	stream.skip(2);

	readRectArray(stream, _droppedRingSrcs, 6);
	readRectArray(stream, _heldRingSrcs, 6);

	readRectArray(stream, _hotspots, 3);

	_destRects.resize(6);
	for (uint ringID = 0; ringID < 6; ++ringID) {
		_destRects[ringID].resize(3);
		for (uint poleID = 0; poleID < 3; ++poleID) {
			// Biggest ring can only be in bottom position,
			// so it only has one rect per pole; second-biggest can
			// be in bottom-most and one position above it, so it has
			// two rects per pole, etc. Skipped data is array of 0xFF.
			readRectArray(stream, _destRects[ringID][poleID], ringID + 1);
			stream.skip((6 - ringID - 1) * 16);
		}
	}

	_takeSound.readData(stream, SoundDescription::kNormal);
	_dropSound.readData(stream, SoundDescription::kNormal);

	_solveExitScene._sceneChange.readData(stream);
	stream.skip(2);
	_solveSound.readData(stream, SoundDescription::kNormal);
	_solveExitScene._flag.label = stream.readSint16LE();
	_solveExitScene._flag.flag = stream.readByte();

	_exitScene.readData(stream);
	readRect(stream, _exitHotspot);
}

void TowerPuzzle::execute() {
	switch (_state) {
	case kBegin:
		init();
		registerGraphics();
		_numRings = _numRingsByDifficulty[NancySceneState.getDifficulty()];

		if (!_puzzleState->playerHasTriedPuzzle) {
			for (uint i = 0; i < _numRings; ++i) {
				_puzzleState->order[0][i] = i;
			}
			_puzzleState->playerHasTriedPuzzle = true;
		}

		for (uint poleID = 0; poleID < 3; ++poleID) {
			for (uint pos = 0; pos < _numRings; ++pos) {
				if (_puzzleState->order[poleID][pos] == -1) {
					continue;
				}

				drawRing(poleID, pos, _puzzleState->order[poleID][pos]);
			}
		}

		g_nancy->_sound->loadSound(_takeSound);
		g_nancy->_sound->loadSound(_dropSound);

		_state = kRun;
		// fall through
	case kRun:
		switch (_solveState) {
		case kNotSolved :
			for (uint i = 0; i < _numRings; ++i) {
				// Win condition is valid for both middle and right pole
				if (_puzzleState->order[1][i] != (int8)i && _puzzleState->order[2][i] != (int8)i) {
					return;
				}
			}

			g_nancy->_sound->loadSound(_solveSound);
			g_nancy->_sound->playSound(_solveSound);
			_solveState = kWaitForSound;
			break;
		case kWaitForSound :
			if (!g_nancy->_sound->isSoundPlaying(_solveSound)) {
				g_nancy->_sound->stopSound(_solveSound);
				_state = kActionTrigger;
			}

			break;
		}

		break;
	case kActionTrigger :
		switch (_solveState) {
		case kNotSolved:
			_exitScene.execute();
			break;
		case kWaitForSound:
			_solveExitScene.execute();
			_puzzleState->playerHasTriedPuzzle = false;
			break;
		}

		g_nancy->_sound->stopSound(_takeSound);
		g_nancy->_sound->stopSound(_dropSound);
		g_nancy->_sound->stopSound(_solveSound);
		finishExecution();
	}
}

void TowerPuzzle::handleInput(NancyInput &input) {
	if (_state != kRun && _solveState != kNotSolved) {
		return;
	}
	
	// Note: this is a click-and-drag puzzle

	// Check if mouse is above a pole hotspot
	// and change the cursor if needed
	int hoveredPoleID = -1;
	for (uint poleID = 0; poleID < 3; ++poleID) {
		if (NancySceneState.getViewport().convertViewportToScreen(_hotspots[poleID]).contains(input.mousePos)) {
			g_nancy->_cursorManager->setCursorType(CursorManager::kHotspot);
			hoveredPoleID = poleID;
			break;
		}
	}

	if (_heldRingID == -1) {
		// Not holding a ring

		// First, check the exit hotspot
		if (NancySceneState.getViewport().convertViewportToScreen(_exitHotspot).contains(input.mousePos)) {
			g_nancy->_cursorManager->setCursorType(CursorManager::kExit);

			if (input.input & NancyInput::kLeftMouseButtonUp) {
				// Player has clicked, exit
				_state = kActionTrigger;
			}

			return;
		}

		// Check if we need to pick up a ring
		// Behavior is the same as original engine, where clicking outside a hotspot
		// and dragging the mouse inside while holding the click still triggers
		if (hoveredPoleID != -1 && (input.input & NancyInput::kLeftMouseButtonHeld)) {
			// Find the position of the topmost ring
			int ringPos;
			for (ringPos = 5; ringPos > -1; --ringPos) {
				if (_puzzleState->order[hoveredPoleID][ringPos] != -1) {
					break;
				}
			}

			if (ringPos == -1) {
				// Pole contains no rings, do nothing
				return;
			}

			// Redraw so the ring isn't visible anymore
			drawRing(hoveredPoleID, ringPos, _puzzleState->order[hoveredPoleID][ringPos], true);

			if (ringPos > 0) {
				drawRing(hoveredPoleID, ringPos - 1, _puzzleState->order[hoveredPoleID][ringPos - 1]);
			}

			// Change the data
			SWAP<int8>(_heldRingID, _puzzleState->order[hoveredPoleID][ringPos]);
			_heldRingPoleID = hoveredPoleID;

			// Show the held ring
			_heldRing._drawSurface.create(_image, _heldRingSrcs[_heldRingID]);
			_heldRing.setVisible(true);
			_heldRing.setTransparent(true);

			g_nancy->_sound->playSound(_takeSound);
		}
	}

	if (_heldRingID != -1) {
		// Holding a ring, check if it has just been dropped
		if ((input.input & NancyInput::kLeftMouseButtonUp) || !(input.input & NancyInput::kLeftMouseButtonHeld)) {
			// Check if dropped over a pole hotspot
			// If not, return to old pole; if yes, move to new one
			uint returnToPole = hoveredPoleID == -1 ? _heldRingPoleID : hoveredPoleID;

			// Find the new position of the ring
			uint newPos;
			for (newPos = 0; newPos < 6; ++newPos) {
				if (_puzzleState->order[returnToPole][newPos] == -1) {
					break;
				}
			}

			// Make sure the player can't place a larger ring on top of a smaller one
			if (newPos > 0 && _puzzleState->order[returnToPole][newPos - 1] > _heldRingID) {
				returnToPole = _heldRingPoleID;

				for (newPos = 0; newPos < 6; ++newPos) {
					if (_puzzleState->order[returnToPole][newPos] == -1) {
						break;
					}
				}
			}

			// Draw the new ring in its place
			drawRing(returnToPole, newPos, _heldRingID);

			// Change the data
			SWAP<int8>(_heldRingID, _puzzleState->order[returnToPole][newPos]);
			_heldRingPoleID = -1;

			g_nancy->_sound->playSound(_dropSound);

			// Hide the held ring
			_heldRing.setVisible(false);
		} else {
			// Still holding the ring, move under mouse

			Common::Rect viewport = NancySceneState.getViewport().getScreenPosition();

			// Do not move ring if mouse is outside viewport
			if (!viewport.contains(input.mousePos)) {
				return;
			}

			Common::Rect newScreenPos = _heldRingSrcs[_heldRingID];
			newScreenPos.moveTo(input.mousePos);
			newScreenPos.translate(-newScreenPos.width() / 2, -newScreenPos.height() / 2);

			// Center of ring is at top left of cursor
			const Common::Point &cursorHotspot = g_nancy->_cursorManager->getCurrentCursorHotspot();
			newScreenPos.translate(-cursorHotspot.x, -cursorHotspot.y);

			// Clip movement so the ring stays entirely inside the viewport
			if (newScreenPos.left < viewport.left) {
				newScreenPos.translate(viewport.left - newScreenPos.left, 0);
			} else if (newScreenPos.right > viewport.right) {
				newScreenPos.translate(viewport.right - newScreenPos.right, 0);
			}

			if (newScreenPos.top < viewport.top) {
				newScreenPos.translate(0, viewport.top - newScreenPos.top);
			} else if (newScreenPos.bottom > viewport.bottom) {
				newScreenPos.translate(0, viewport.bottom - newScreenPos.bottom);
			}

			_heldRing.moveTo(newScreenPos);
		}
	}
}

void TowerPuzzle::drawRing(uint poleID, uint position, uint ringID, bool clear) {
	_needsRedraw = true;

	if (clear) {
		// Just clear the ring, leaving a hole in the surface
		// that needs to be filled by redrawing the ring below
		_drawSurface.fillRect(_destRects[ringID][poleID][position], _drawSurface.getTransparentColor());
		return;
	}

	_drawSurface.blitFrom(_image, _droppedRingSrcs[ringID], _destRects[ringID][poleID][position]);
}

} // End of namespace Action
} // End of namespace Nancy
