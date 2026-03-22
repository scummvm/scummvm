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
#include "engines/nancy/util.h"

#include "engines/nancy/enginedata.h"
#include "engines/nancy/state/scene.h"
#include "engines/nancy/action/puzzle/multibuildpuzzle.h"

namespace Nancy {
namespace Action {

void MultiBuildPuzzle::init() {
	g_nancy->_resource->loadImage(_primaryImageName, _primaryImage);
	_primaryImage.setTransparentColor(_drawSurface.getTransparentColor());

	if (_hasCloseupImage) {
		g_nancy->_resource->loadImage(_closeupImageName, _closeupImage);
		_closeupImage.setTransparentColor(_drawSurface.getTransparentColor());
	}

	for (uint i = 0; i < _pieces.size(); ++i) {
		Piece &p = _pieces[i];

		const Common::Rect &spriteSrc = !p.altSrcRect.isEmpty() ? p.altSrcRect : p.srcRect;

		int w = spriteSrc.width();
		int h = spriteSrc.height();

		// Rotation 0: blit from primary image using sprite source rect
		p.rotateSurfaces[0].create(w, h, _primaryImage.format);
		p.rotateSurfaces[0].setTransparentColor(_primaryImage.getTransparentColor());
		p.rotateSurfaces[0].blitFrom(_primaryImage, spriteSrc, Common::Point(0, 0));
		p.hasSurface[0] = true;

		// Rotations 1-3: created if canRotateAll or piece has a valid altSrcRect
		if (_canRotateAll || !p.altSrcRect.isEmpty()) {
			for (int r = 1; r < 4; ++r) {
				rotateSurface90CW(p.rotateSurfaces[r - 1], p.rotateSurfaces[r]);
				p.rotateSurfaces[r].setTransparentColor(_primaryImage.getTransparentColor());
				p.hasSurface[r] = true;
			}
		}

		// All pieces start at their homeRect (slot) in unplaced visual state
		p.curRotation = 0;
		p.gameRect = p.homeRect;
		p.isPlaced = false;

		updatePieceRender(i);
		p.setVisible(true);
		p.setTransparent(true);
		p.setZ((uint16)(_z + i + 1));
	}

	if (_hasCloseupImage) {
		_shelfSlots.resize(_pieces.size());
		for (uint i = 0; i < _pieces.size(); ++i) {
			Piece &slot = _shelfSlots[i];
			int w = _pieces[i].srcRect.width();
			int h = _pieces[i].srcRect.height();
			slot._drawSurface.create(w, h, _primaryImage.format);
			slot._drawSurface.setTransparentColor(_primaryImage.getTransparentColor());
			slot._drawSurface.blitFrom(_primaryImage, _pieces[i].srcRect, Common::Point(0, 0));
			slot.moveTo(_pieces[i].homeRect);
			slot.setTransparent(true);
			slot.setVisible(true);
			slot.setZ(_z);  // Below all active pieces (_z+1 and up)
		}
	}
}

void MultiBuildPuzzle::registerGraphics() {
	if (_hasCloseupImage) {
		for (uint i = 0; i < _shelfSlots.size(); ++i)
			_shelfSlots[i].registerGraphics();
	}
	for (uint i = 0; i < _pieces.size(); ++i)
		_pieces[i].registerGraphics();
}

void MultiBuildPuzzle::readData(Common::SeekableReadStream &stream) {
	// 0x00: primary image name (33 bytes)
	readFilename(stream, _primaryImageName);

	// 0x21: closeup image name (33 bytes)
	Common::String secName;
	readFilename(stream, secName);
	_closeupImageName = Common::Path(secName);
	_hasCloseupImage = (secName != "NO_FILE" && !secName.empty());

	// 0x42: numPieces, requiredPieces
	_numPieces = stream.readUint16LE();
	_requiredPieces = stream.readUint16LE();

	// 0x46: 1 unknown byte, 0x47: canRotateAll
	stream.skip(1);
	_canRotateAll = stream.readByte() != 0;

	// 0x48-0x5e: 23 unknown bytes
	stream.skip(23);

	// 0x5f: pieces (always 20 × 67 bytes in data, only numPieces are valid)
	_pieces.resize(_numPieces);
	for (uint i = 0; i < 20; ++i) {
		if (i < _numPieces) {
			Piece &p = _pieces[i];
			readRect(stream, p.srcRect);     // 0x00, 16 bytes
			readRect(stream, p.homeRect);    // 0x10, 16 bytes
			readRect(stream, p.altSrcRect);  // 0x20, 16 bytes
			readRect(stream, p.cuSrcRect);   // 0x30, 16 bytes
			p.counterByte  = stream.readByte();  // 0x40
			p.mustPlace    = stream.readByte();  // 0x41
			p.mustNotPlace = stream.readByte();  // 0x42
		} else {
			stream.skip(67);
		}
	}

	// 0x59b: three SoundDescriptions (0x31 bytes each)
	_sounds[0].readNormal(stream);
	_sounds[1].readNormal(stream);
	_sounds[2].readNormal(stream);

	// 0x62e: 6 unknown bytes
	stream.skip(6);

	// 0x634: solve scene, 25 bytes
	_solveScene.readData(stream);

	// 0x64d: solve sound (49 bytes)
	_solveSound.readNormal(stream);

	// 0x67e: solve text key (33 bytes, looked up in string table)
	Common::String solveTextKey;
	readFilename(stream, solveTextKey);

	// 0x69f: solve raw text (200 bytes)
	char textBuf[200];
	stream.read(textBuf, 200);
	assembleTextLine(textBuf, _solveText, 200);

	// 0x767: cancel scene, 25 bytes
	_cancelScene.readData(stream);

	// 0x780: exit hotspot rect (16 bytes)
	readRect(stream, _exitHotspot);

	// 0x790: target drop-zone rect — pieces must be within this area to count as validly placed
	readRect(stream, _targetZone);
}

void MultiBuildPuzzle::execute() {
	switch (_state) {
	case kBegin:
		init();
		registerGraphics();
		g_nancy->_sound->loadSound(_sounds[0]);
		g_nancy->_sound->loadSound(_sounds[1]);
		g_nancy->_sound->loadSound(_sounds[2]);
		g_nancy->_sound->loadSound(_solveSound);
		_state = kRun;
		// fall through
	case kRun:
		switch (_solveState) {
		case kIdle:
			// Normal interaction; handleInput drives piece movement.
			break;

		case kWaitTimer:
			// Short animation after placing a piece.
			if (g_system->getMillis() >= _timerEnd) {
				_solveState = kIdle;
			}
			break;

		case kPlaySolveSound:
			// Play solve sound and show solve text, then wait for it to finish.
			g_nancy->_sound->playSound(_solveSound);
			if (!_solveText.empty()) {
				NancySceneState.getTextbox().clear();
				NancySceneState.getTextbox().addTextLine(_solveText);
			}
			_solveState = kWaitSolveSound;
			break;

		case kWaitSolveSound:
			// Wait until solve sound has finished, then trigger scene change.
			if (!g_nancy->_sound->isSoundPlaying(_solveSound)) {
				g_nancy->_sound->stopSound(_solveSound);
				_state = kActionTrigger;
			}
			break;
		}
		break;

	case kActionTrigger:
		g_nancy->_sound->stopSound(_sounds[0]);
		g_nancy->_sound->stopSound(_sounds[1]);
		g_nancy->_sound->stopSound(_sounds[2]);
		g_nancy->_sound->stopSound(_solveSound);
		if (_isCancelled) {
			// Change to cancel scene unconditionally, but only set the cancel flag if
			// at least one counterByte==0 piece was placed.
			NancySceneState.changeScene(_cancelScene._sceneChange);
			if (_cancelScene._flag.label != kFlagNoLabel) {
				uint16 count = 0;
				for (uint i = 0; i < _pieces.size(); ++i) {
					if (_pieces[i].isPlaced && _pieces[i].counterByte == 0)
						++count;
				}
				if (count > 0)
					NancySceneState.setEventFlag(_cancelScene._flag);
			}
		} else {
			NancySceneState.setEventFlag(_solveScene._flag);
			NancySceneState.changeScene(_solveScene._sceneChange);
		}
		finishExecution();
		break;
	}
}

void MultiBuildPuzzle::handleInput(NancyInput &input) {
	if (_state != kRun || _solveState != kIdle || _isSolved || _isCancelled)
		return;

	const VIEW *viewData = GetEngineData(VIEW);
	if (!viewData)
		return;
	Common::Rect vpScreen = viewData->screenPosition;
	if (!vpScreen.contains(input.mousePos))
		return;

	Common::Point mouseVP(input.mousePos.x - vpScreen.left,
	                      input.mousePos.y - vpScreen.top);

	if (_isDragging) {
		// Update dragged piece to center on cursor
		Piece &pp = _pieces[_pickedUpPiece];
		int newLeft = mouseVP.x - _pickedUpWidth / 2;
		int newTop  = mouseVP.y - _pickedUpHeight / 2;
		pp.gameRect.left   = newLeft;
		pp.gameRect.top    = newTop;
		pp.gameRect.right  = newLeft + _pickedUpWidth;
		pp.gameRect.bottom = newTop  + _pickedUpHeight;
		updatePieceRender(_pickedUpPiece);

		g_nancy->_cursor->setCursorType(CursorManager::kCustom1);

		// Right click: rotate the carried piece
		if ((input.input & NancyInput::kRightMouseButtonUp) && pp.hasSurface[1]) {
			pp.curRotation = (pp.curRotation + 1) % 4;
			_pickedUpWidth  = pp.rotateSurfaces[pp.curRotation].w;
			_pickedUpHeight = pp.rotateSurfaces[pp.curRotation].h;
			g_nancy->_sound->playSound(_sounds[0]);
			updatePieceRender(_pickedUpPiece);
			return;
		}

		// Left click: drop the piece wherever the cursor is.
		// For non-closeup puzzles: reject if the drop center is outside the target zone or
		// the piece overlaps an already-placed piece; piece returns to shelf on rejection.
		// For closeup puzzles: no geometric checks — free-form placement.
		if (input.input & NancyInput::kLeftMouseButtonUp) {
			bool validDrop = true;

			// Geometric checks apply only to non-closeup puzzles with a win condition (e.g. books).
			// Sand castle (no closeup image, _requiredPieces=0) allows free stacking.
			// Sandwich puzzle (has closeup image) allows free-form placement.
			if (!_hasCloseupImage && _requiredPieces > 0) {
				// Boundary check: drop center must be inside the target zone
				Common::Point dropCenter((pp.gameRect.left + pp.gameRect.right) / 2,
				                        (pp.gameRect.top  + pp.gameRect.bottom) / 2);
				if (!_targetZone.isEmpty() && !_targetZone.contains(dropCenter))
					validDrop = false;

				// Overlap check: piece must not overlap any already-placed piece
				if (validDrop) {
					for (uint i = 0; i < _pieces.size(); ++i) {
						if ((int)i != _pickedUpPiece && _pieces[i].isPlaced &&
						        pp.gameRect.intersects(_pieces[i].gameRect)) {
							validDrop = false;
							break;
						}
					}
				}
			}

			// Clear drag state BEFORE updatePieceRender so the correct visual is chosen:
			// - valid drop:   isPlaced=true,  isDragging=false -> shows rotation surface at drop pos
			// - invalid drop: isPlaced=false, isDragging=false -> shows shelf srcRect at homeRect
			_isDragging = false;
			int placedIdx = _pickedUpPiece;
			_pickedUpPiece = -1;

			if (validDrop) {
				pp.isPlaced = true;
				g_nancy->_sound->playSound(_sounds[1]);
			} else {
				// Return piece to its shelf position
				pp.gameRect = pp.homeRect;
			}

			updatePieceRender(placedIdx);

			// After placing, check if the puzzle is now solved (FUN_0046da47)
			checkIfSolved();

			// Brief debounce before next interaction (only if not transitioning to solve)
			if (_solveState == kIdle) {
				_solveState = kWaitTimer;
				_timerEnd = g_system->getMillis() + 300;
			}
		}
		return;
	}

	if (_selectedPiece != -1) {
		if (input.input & NancyInput::kLeftMouseButtonUp) {
			int sel = _selectedPiece;
			_selectedPiece = -1;
			Piece &pp = _pieces[sel];
			_pickedUpPiece = sel;
			_isDragging = true;
			pp.curRotation = 0;
			_pickedUpWidth  = pp.rotateSurfaces[0].w;
			_pickedUpHeight = pp.rotateSurfaces[0].h;
			// Centre the drag rect on the cursor now that we leave the CU display position
			int newLeft = mouseVP.x - _pickedUpWidth / 2;
			int newTop  = mouseVP.y - _pickedUpHeight / 2;
			pp.gameRect = Common::Rect(newLeft, newTop,
			                          newLeft + _pickedUpWidth, newTop + _pickedUpHeight);
			updatePieceRender(sel);
		}
		return;
	}

	// Not dragging and nothing selected: look for a piece to pick up / select
	int16 topmost = -1;
	for (int i = (int)_pieces.size() - 1; i >= 0; --i) {
		Piece &p = _pieces[i];
		if (!p.gameRect.contains(mouseVP))
			continue;
		if (topmost == -1 || p.getZOrder() > _pieces[topmost].getZOrder())
			topmost = (int16)i;
	}

	if (topmost != -1) {
		g_nancy->_cursor->setCursorType(CursorManager::kHotspot);

		if (input.input & NancyInput::kLeftMouseButtonUp) {
			Piece &pp = _pieces[topmost];
			pp.isPlaced = false;
			pp.curRotation = 0;
			pp.setZ((uint16)(_z + (int)_pieces.size() * 2));
			pp.registerGraphics();
			g_nancy->_sound->playSound(_sounds[0]);

			if (_hasCloseupImage && !pp.cuSrcRect.isEmpty()) {
				// First click shows close-up view, centered in the viewport.
				_selectedPiece = topmost;
				int cuW = pp.cuSrcRect.width();
				int cuH = pp.cuSrcRect.height();
				int cuLeft = (vpScreen.width()  - cuW) / 2;
				int cuTop  = (vpScreen.height() - cuH) / 2;
				pp.gameRect = Common::Rect(cuLeft, cuTop, cuLeft + cuW, cuTop + cuH);
			} else {
				// Direct drag: first click immediately starts dragging
				_isDragging = true;
				_pickedUpPiece = topmost;
				_pickedUpWidth  = pp.rotateSurfaces[0].w;
				_pickedUpHeight = pp.rotateSurfaces[0].h;
			}
			updatePieceRender(topmost);
		}
		return;
	}

	// Check exit hotspot
	if (!_exitHotspot.isEmpty()) {
		Common::Rect exitScreen = NancySceneState.getViewport().convertViewportToScreen(_exitHotspot);
		if (exitScreen.contains(input.mousePos)) {
			g_nancy->_cursor->setCursorType(g_nancy->_cursor->_puzzleExitCursor);
			if (input.input & NancyInput::kLeftMouseButtonUp) {
				checkIfSolvedOnExit();
			}
		}
	}
}

void MultiBuildPuzzle::checkIfSolvedOnExit() {
	if (_requiredPieces == 1) {
		// Check constraints again
		bool placedBadPiece = false;
		bool placedPiece = false;

		for (uint i = 0; i < _pieces.size(); ++i) {
			if (_pieces[i].isPlaced) {
				if (_pieces[i].mustNotPlace) {
					NancySceneState.setEventFlag(_cancelScene._flag);
					placedBadPiece = true;
					break;
				}
				placedPiece = true;
			}
		}

		if (!placedPiece || placedBadPiece) {
			// Player hasn't placed any pieces, or has placed
			// at least one bad piece -> retry scene or lose,
			// depending on the cancelScene flag.
			_isCancelled = true;
			_state = kActionTrigger;
		} else {
			// Player has placed only good pieces -> player won.
			_isSolved = true;
			_solveState = kPlaySolveSound;
		}
	} else {
		_isCancelled = true;
		_state = kActionTrigger;
	}
}

void MultiBuildPuzzle::checkIfSolved() {
	// Non-CU puzzles (no secondary image) with _requiredPieces == 0 have no win condition;
	// the player exits manually (e.g. sand castle). CU puzzles (sandwich) with
	// _requiredPieces == 0 still use mustPlace/mustNotPlace constraints to determine solve.
	if (_requiredPieces == 0 && !_hasCloseupImage)
		return;

	// Count correctly placed pieces (those with counterByte == 0)
	uint16 count = 0;
	for (uint i = 0; i < _pieces.size(); ++i) {
		if (_pieces[i].isPlaced && _pieces[i].counterByte == 0)
			++count;
	}

	if (count < _requiredPieces)
		return;

	// Check constraints: no placed mustNotPlace piece, no unplaced mustPlace piece
	for (uint i = 0; i < _pieces.size(); ++i) {
		if (_pieces[i].isPlaced && _pieces[i].mustNotPlace)
			return;
		if (!_pieces[i].isPlaced && _pieces[i].mustPlace)
			return;
	}

	// Solved!
	_isSolved = true;
	_solveState = kPlaySolveSound;
}

void MultiBuildPuzzle::updatePieceRender(int pieceIdx) {
	Piece &p = _pieces[pieceIdx];
	bool isSelected = (!_isDragging && pieceIdx == _selectedPiece);
	bool isDragging  = (_isDragging  && pieceIdx == _pickedUpPiece);

	if (p.isPlaced || isDragging) {
		// Placed or being dragged: show rotation sprite.
		int rot = p.curRotation;
		if (!p.hasSurface[rot])
			rot = 0;
		if (p.hasSurface[rot]) {
			int w = p.rotateSurfaces[rot].w;
			int h = p.rotateSurfaces[rot].h;
			p._drawSurface.create(w, h, p.rotateSurfaces[rot].format);
			p._drawSurface.setTransparentColor(p.rotateSurfaces[rot].getTransparentColor());
			p._drawSurface.blitFrom(p.rotateSurfaces[rot], Common::Point(0, 0));
		}
	} else if (isSelected && _hasCloseupImage && !p.cuSrcRect.isEmpty()) {
		// Show zoomed close-up
		int w = p.cuSrcRect.width();
		int h = p.cuSrcRect.height();
		p._drawSurface.create(w, h, _closeupImage.format);
		p._drawSurface.setTransparentColor(_closeupImage.getTransparentColor());
		p._drawSurface.blitFrom(_closeupImage, p.cuSrcRect, Common::Point(0, 0));
	} else {
		// Unplaced and at rest on the shelf (or selected with no close-up available):
		// show srcRect from primary image.
		int w = p.srcRect.width();
		int h = p.srcRect.height();
		p._drawSurface.create(w, h, _primaryImage.format);
		p._drawSurface.setTransparentColor(_primaryImage.getTransparentColor());
		p._drawSurface.blitFrom(_primaryImage, p.srcRect, Common::Point(0, 0));
	}

	p.setTransparent(true);
	p.moveTo(p.gameRect);
}

void MultiBuildPuzzle::rotateSurface90CW(const Graphics::ManagedSurface &src, Graphics::ManagedSurface &dst) {
	int srcW = src.w;
	int srcH = src.h;
	dst.create(srcH, srcW, src.format);

	for (int y = 0; y < srcH; ++y) {
		for (int x = 0; x < srcW; ++x) {
			uint32 pixel = src.getPixel(x, y);
			dst.setPixel(srcH - 1 - y, x, pixel);
		}
	}
}

} // End of namespace Action
} // End of namespace Nancy
