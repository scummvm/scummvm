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
#include "common/system.h"

#include "engines/nancy/nancy.h"
#include "engines/nancy/graphics.h"
#include "engines/nancy/resource.h"
#include "engines/nancy/sound.h"
#include "engines/nancy/input.h"
#include "engines/nancy/util.h"

#include "engines/nancy/enginedata.h"
#include "engines/nancy/state/scene.h"
#include "engines/nancy/action/puzzle/onebuildpuzzle.h"

namespace Nancy {
namespace Action {

void OneBuildPuzzle::init() {
	g_nancy->_resource->loadImage(_imageName, _image);
	_image.setTransparentColor(_drawSurface.getTransparentColor());

	for (uint i = 0; i < _pieces.size(); ++i) {
		Piece &p = _pieces[i];
		int w = p.srcRect.width();
		int h = p.srcRect.height();

		// Rotation 0: blit from source image
		p.rotateSurfaces[0].create(w, h, _image.format);
		p.rotateSurfaces[0].setTransparentColor(_drawSurface.getTransparentColor());
		p.rotateSurfaces[0].blitFrom(_image, p.srcRect, Common::Point(0, 0));
		p.hasSurface[0] = true;

		// Rotations 1-3: only needed if pieces can rotate
		if (_canRotateAll || p.isPreRotated) {
			for (int r = 1; r < 4; ++r) {
				rotateSurface90CW(p.rotateSurfaces[r - 1], p.rotateSurfaces[r]);
				p.rotateSurfaces[r].setTransparentColor(_drawSurface.getTransparentColor());
				p.hasSurface[r] = true;
			}
		}

		// Initial position and rotation
		if (p.isPreRotated) {
			// Pre-rotated pieces start at their slot and are already placed
			p.curRotation = 0;
			p.gameRect = p.slotRect;
			p.placed = true;
		} else {
			// Normal pieces start at home with defaultRotation
			p.curRotation = p.defaultRotation;
			p.gameRect = p.homeRect;
			p.placed = false;
		}

		updatePieceRender(i);
		p.setVisible(true);
		p.setTransparent(true);
		p.setZ(_z + (uint16)i + 1);
	}
}

void OneBuildPuzzle::registerGraphics() {
	for (uint i = 0; i < _pieces.size(); ++i)
		_pieces[i].registerGraphics();
}

void OneBuildPuzzle::readData(Common::SeekableReadStream &stream) {
	readFilename(stream, _imageName);

	_numPieces = stream.readUint16LE();
	_freePlacement = stream.readByte();
	_canRotateAll = stream.readByte();
	stream.skip(6); // rotationMode, zoneHeight, zoneWidth, mouse-clamping flag
	_slotTolerance = stream.readSint16LE();
	_orderedPlacement = stream.readByte();

	_placementOrder.resize(20);
	for (uint i = 0; i < 20; ++i)
		_placementOrder[i] = stream.readSint16LE();

	_pieces.resize(_numPieces);
	for (uint i = 0; i < 20; ++i) {
		if (i < _numPieces) {
			Piece &p = _pieces[i];
			readRect(stream, p.srcRect);
			readRect(stream, p.slotRect);
			readRect(stream, p.homeRect);
			p.defaultRotation = stream.readByte();
			p.isPreRotated = stream.readByte();
		} else {
			stream.skip(50);
		}
	}

	_pickupSound.readNormal(stream);  // +0x43e: played when rotating a placed piece
	_rotateSound.readNormal(stream);  // +0x46f: played when picking up an unplaced piece
	_dropSound.readNormal(stream);    // +0x4a0: played when dropping a piece
	readFilename(stream, _dropAlt1Filename);
	readFilename(stream, _dropAlt2Filename);

	_goodPlacementSound.readNormal(stream); // +0x513
	readFilename(stream, _goodAlt1Filename);
	readFilename(stream, _goodAlt2Filename);

	_goodTexts.resize(3);
	Common::String unusedKey;
	for (uint i = 0; i < 3; ++i)
		readFilename(stream, unusedKey);
	char textBuf[200];
	for (uint i = 0; i < 3; ++i) {
		stream.read(textBuf, 200);
		assembleTextLine(textBuf, _goodTexts[i], 200);
	}

	_badPlacementSound.readNormal(stream);  // +0x841
	readFilename(stream, _badAlt1Filename);
	readFilename(stream, _badAlt2Filename);

	_badTexts.resize(3);
	for (uint i = 0; i < 3; ++i)
		readFilename(stream, unusedKey);
	for (uint i = 0; i < 3; ++i) {
		stream.read(textBuf, 200);
		assembleTextLine(textBuf, _badTexts[i], 200);
	}

	stream.skip(4); // unknown bytes at +0xb6f
	_solveScene.readData(stream);
	_completionSound.readNormal(stream);
	readFilename(stream, unusedKey);
	stream.read(textBuf, 200);
	assembleTextLine(textBuf, _completionText, 200);

	_cancelScene.readData(stream);
	readRect(stream, _exitHotspot);
}

void OneBuildPuzzle::execute() {
	switch (_state) {
	case kBegin:
		init();
		registerGraphics();
		g_nancy->_sound->loadSound(_pickupSound);
		g_nancy->_sound->loadSound(_rotateSound);
		g_nancy->_sound->loadSound(_dropSound);
		g_nancy->_sound->loadSound(_goodPlacementSound);
		g_nancy->_sound->loadSound(_badPlacementSound);
		g_nancy->_sound->loadSound(_completionSound);
		_state = kRun;
		// fall through
	case kRun:
		switch (_solveState) {
		case kIdle:
			// Normal interaction; handleInput drives piece movement
			break;
		case kWaitTimer:
			// Post-drop/pickup delay (300ms) before deciding outcome
			if (g_system->getMillis() >= _timerEnd) {
				g_nancy->_sound->stopSound(_currentSound);
				if (!_isDropSound) {
					// Pickup/rotate sound finished; return to idle (piece still dragging)
					_solveState = kIdle;
				} else if (_correctlyPlaced) {
					playGoodPlacementSound();
					checkAllPlaced();
				} else {
					// Wrong drop: play bad placement feedback
					playBadPlacementSound();
				}
			}
			break;
		case kWaitPlaceSound:
			// Waiting for good/bad placement sound to finish OR 1s minimum display time
			if (!g_nancy->_sound->isSoundPlaying(_currentSound) || g_system->getMillis() >= _timerEnd) {
				g_nancy->_sound->stopSound(_currentSound);
				NancySceneState.getTextbox().clear();
				_solveState = kIdle;
			}
			break;
		case kWaitCompletion:
			// Waiting for completion sound to finish before scene change
			if (!g_nancy->_sound->isSoundPlaying(_completionSound)) {
				_state = kActionTrigger;
			}
			break;
		case kTriggerCompletion:
			// Play completion sound/text, then wait for it to finish
			g_nancy->_sound->playSound(_completionSound);
			if (!_completionText.empty()) {
				NancySceneState.getTextbox().clear();
				NancySceneState.getTextbox().addTextLine(_completionText);
			}
			_solveState = kWaitCompletion;
			break;
		}
		break;
	case kActionTrigger:
		if (_isCancelled) {
			_cancelScene.execute();
		} else {
			NancySceneState.setEventFlag(_solveScene._flag);
			NancySceneState.changeScene(_solveScene._sceneChange);
		}
		break;
	}
}

void OneBuildPuzzle::handleInput(NancyInput &input) {
	if (_state != kRun || _isSolved || _isCancelled)
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
		// Always update drag position while carrying a piece
		updateDragPosition(mouseVP);
		g_nancy->_cursor->setCursorType(CursorManager::kCustom1);

		if (_solveState != kIdle)
			return;

		// Right click while dragging: rotate the carried piece
		if (input.input & NancyInput::kRightMouseButtonUp) {
			rotatePiece(_pickedUpPiece);
			Piece &pp = _pieces[_pickedUpPiece];
			_pickedUpWidth  = pp.rotateSurfaces[pp.curRotation].w;
			_pickedUpHeight = pp.rotateSurfaces[pp.curRotation].h;
			playPickupSound();
			return;
		}

		// Left click while dragging: attempt to place
		if (input.input & NancyInput::kLeftMouseButtonUp) {
			Piece &piece = _pieces[_pickedUpPiece];

			Common::Rect slot = piece.slotRect;

			// Correct placement: bounding-box must fit within slot +- tolerance
			// (piece is rotation 0, same size as slot)
			bool nearSlot = (piece.gameRect.left >= slot.left - _slotTolerance &&
							 piece.gameRect.top  >= slot.top  - _slotTolerance &&
							 piece.gameRect.right  <= slot.right  + _slotTolerance &&
							 piece.gameRect.bottom <= slot.bottom + _slotTolerance);

			bool correctRotation = (piece.curRotation == 0);
			bool orderOk = !_orderedPlacement ||
				(_piecesPlaced < (uint16)_placementOrder.size() &&
				 _placementOrder[_piecesPlaced] == (int16)(_pickedUpPiece + 1));

			if (nearSlot && correctRotation && orderOk) {
				piece.gameRect = piece.slotRect;
				piece.placed = true;
				_correctlyPlaced = true;
				++_piecesPlaced;
			} else {
				_correctlyPlaced = false;
				if (!_freePlacement) {
					piece.gameRect = _prevDragGameRect;
				} else {
					piece.curRotation = piece.defaultRotation;
					piece.gameRect = piece.homeRect;
				}
			}

			updatePieceRender(_pickedUpPiece);
			_isDragging = false;
			_pickedUpPiece = -1;
			playDropSound();
		}
		return;
	}

	// Not dragging: only process when idle
	if (_solveState != kIdle)
		return;

	// Find topmost piece under cursor (separately tracking unplaced vs any)
	int16 topmostUnplaced = -1;
	int16 topmostAny = -1;

	for (uint i = 0; i < _pieces.size(); ++i) {
		Piece &p = _pieces[i];
		if (!p.gameRect.contains(mouseVP))
			continue;
		if (topmostAny == -1 || p.getZOrder() > _pieces[topmostAny].getZOrder())
			topmostAny = (int16)i;
		if (!p.placed) {
			if (topmostUnplaced == -1 || p.getZOrder() > _pieces[topmostUnplaced].getZOrder())
				topmostUnplaced = (int16)i;
		}
	}

	if (topmostAny != -1) {
		g_nancy->_cursor->setCursorType(CursorManager::kHotspot);

		// Right click: rotate topmost piece under cursor
		if (input.input & NancyInput::kRightMouseButtonUp) {
			rotatePiece(topmostAny);
			playPickupSound();
			return;
		}

		// Left click on an unplaced piece: pick it up
		if ((input.input & NancyInput::kLeftMouseButtonUp) && topmostUnplaced != -1) {
			Piece &pp = _pieces[topmostUnplaced];
			_pickedUpPiece = topmostUnplaced;
			_isDragging = true;
			_pickedUpWidth  = pp.rotateSurfaces[pp.curRotation].w;
			_pickedUpHeight = pp.rotateSurfaces[pp.curRotation].h;
			pp.setZ((uint16)(_z + (int)_pieces.size() * 2));
			pp.registerGraphics();
			playRotateSoundAndStartTimer();
		}
		return;
	}

	// Check exit hotspot
	Common::Rect exitScreen = NancySceneState.getViewport().convertViewportToScreen(_exitHotspot);
	if (exitScreen.contains(input.mousePos)) {
		g_nancy->_cursor->setCursorType(g_nancy->_cursor->_puzzleExitCursor);
		if (input.input & NancyInput::kLeftMouseButtonUp) {
			_isCancelled = true;
			_state = kActionTrigger;
		}
	}
}

// --- Internal helpers ---

void OneBuildPuzzle::updatePieceRender(int pieceIdx) {
	Piece &p = _pieces[pieceIdx];
	int rot = p.curRotation;
	if (!p.hasSurface[rot])
		rot = 0;
	if (!p.hasSurface[rot])
		return;
	p._drawSurface.create(p.rotateSurfaces[rot], p.rotateSurfaces[rot].getBounds());
	p.setTransparent(true);
	p.moveTo(p.gameRect);
}

void OneBuildPuzzle::rotatePiece(int pieceIdx) {
	Piece &p = _pieces[pieceIdx];
	int oldRot = p.curRotation;
	int oldW = p.rotateSurfaces[oldRot].w;
	int oldH = p.rotateSurfaces[oldRot].h;

	int newRot = (oldRot + 1) % 4;
	p.curRotation = newRot;

	int newW = p.hasSurface[newRot] ? p.rotateSurfaces[newRot].w : oldW;
	int newH = p.hasSurface[newRot] ? p.rotateSurfaces[newRot].h : oldH;

	// Preserve center point when changing dimensions
	int cx = p.gameRect.left + oldW / 2;
	int cy = p.gameRect.top  + oldH / 2;
	p.gameRect.left   = cx - newW / 2;
	p.gameRect.top    = cy - newH / 2;
	p.gameRect.right  = p.gameRect.left + newW;
	p.gameRect.bottom = p.gameRect.top  + newH;

	clampRectToViewport(p.gameRect);
	updatePieceRender(pieceIdx);
}

void OneBuildPuzzle::updateDragPosition(Common::Point mouseVP) {
	if (_pickedUpPiece == -1)
		return;

	Piece &p = _pieces[_pickedUpPiece];

	// Save current position as "previous" before updating (for freePlacement restore)
	_prevDragGameRect = p.gameRect;

	int newLeft = mouseVP.x - _pickedUpWidth / 2;
	int newTop  = mouseVP.y - _pickedUpHeight / 2;

	p.gameRect.left   = newLeft;
	p.gameRect.top    = newTop;
	p.gameRect.right  = newLeft + _pickedUpWidth;
	p.gameRect.bottom = newTop  + _pickedUpHeight;

	clampRectToViewport(p.gameRect);
	updatePieceRender(_pickedUpPiece);
}

void OneBuildPuzzle::clampRectToViewport(Common::Rect &rect) {
	const VIEW *viewData = GetEngineData(VIEW);
	if (!viewData)
		return;
	int vpW = viewData->screenPosition.width();
	int vpH = viewData->screenPosition.height();
	int w = rect.width();
	int h = rect.height();

	if (rect.top < 0) {
		rect.top = 0;
		rect.bottom = h;
	}
	if (rect.bottom > vpH) {
		rect.bottom = vpH;
		rect.top = vpH - h;
	}
	if (rect.left < 0) {
		rect.left = 0;
		rect.right = w;
	}
	if (rect.right > vpW) {
		rect.right = vpW;
		rect.left = vpW - w;
	}
}

void OneBuildPuzzle::checkAllPlaced() {
	for (uint i = 0; i < _pieces.size(); ++i) {
		if (!_pieces[i].placed)
			return;
	}
	_isSolved = true;
	_solveState = kTriggerCompletion;
}

void OneBuildPuzzle::playPickupSound() {
	_currentSound = _pickupSound;
	g_nancy->_sound->loadSound(_currentSound);
	g_nancy->_sound->playSound(_currentSound);
	_timerEnd = g_system->getMillis() + 300;
	_isDropSound = false;
}

void OneBuildPuzzle::playRotateSoundAndStartTimer() {
	_currentSound = _rotateSound;
	g_nancy->_sound->loadSound(_currentSound);
	g_nancy->_sound->playSound(_currentSound);
	_solveState = kWaitTimer;
	_timerEnd = g_system->getMillis() + 300;
	_isDropSound = false;
}

void OneBuildPuzzle::playDropSound() {
	_currentSound = _dropSound;
	int roll = g_nancy->_randomSource->getRandomNumber(2);
	if (roll == 1 && _dropAlt1Filename != "NO SOUND" && !_dropAlt1Filename.empty())
		_currentSound.name = _dropAlt1Filename;
	else if (roll == 2 && _dropAlt2Filename != "NO SOUND" && !_dropAlt2Filename.empty())
		_currentSound.name = _dropAlt2Filename;
	g_nancy->_sound->loadSound(_currentSound);
	g_nancy->_sound->playSound(_currentSound);
	_solveState = kWaitTimer;
	_timerEnd = g_system->getMillis() + 300;
	_isDropSound = true;
}

void OneBuildPuzzle::playGoodPlacementSound() {
	int idx = g_nancy->_randomSource->getRandomNumber(2);
	_currentSound = _goodPlacementSound;
	if (idx == 1 && _goodAlt1Filename != "NO SOUND" && !_goodAlt1Filename.empty())
		_currentSound.name = _goodAlt1Filename;
	else if (idx == 2 && _goodAlt2Filename != "NO SOUND" && !_goodAlt2Filename.empty())
		_currentSound.name = _goodAlt2Filename;
	else
		idx = 0;
	g_nancy->_sound->loadSound(_currentSound);
	g_nancy->_sound->playSound(_currentSound);
	if (!_goodTexts[idx].empty()) {
		NancySceneState.getTextbox().clear();
		NancySceneState.getTextbox().addTextLine(_goodTexts[idx]);
	}
	_solveState = kWaitPlaceSound;
	_timerEnd = g_system->getMillis() + 1000;
}

void OneBuildPuzzle::playBadPlacementSound() {
	int idx = g_nancy->_randomSource->getRandomNumber(2);
	_currentSound = _badPlacementSound;
	if (idx == 1 && _badAlt1Filename != "NO SOUND" && !_badAlt1Filename.empty())
		_currentSound.name = _badAlt1Filename;
	else if (idx == 2 && _badAlt2Filename != "NO SOUND" && !_badAlt2Filename.empty())
		_currentSound.name = _badAlt2Filename;
	else
		idx = 0;
	g_nancy->_sound->loadSound(_currentSound);
	g_nancy->_sound->playSound(_currentSound);
	if (!_badTexts[idx].empty()) {
		NancySceneState.getTextbox().clear();
		NancySceneState.getTextbox().addTextLine(_badTexts[idx]);
	}
	_solveState = kWaitPlaceSound;
	_timerEnd = g_system->getMillis() + 1000;
}

// static
void OneBuildPuzzle::rotateSurface90CW(const Graphics::ManagedSurface &src, Graphics::ManagedSurface &dst) {
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
