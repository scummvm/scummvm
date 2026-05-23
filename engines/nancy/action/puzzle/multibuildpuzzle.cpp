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

		p.curRotation = 0;
		p.gameRect = p.homeRect;
		p.isPlaced = false;

		updatePieceRender(i);
		p.setVisible(true);
		p.setTransparent(true);
		p.setZ((uint16)(_z + i + 1));
	}

	_isInitialized = true;
}

void MultiBuildPuzzle::registerGraphics() {
	if (!_isInitialized)
		return;

	for (uint i = 0; i < _pieces.size(); ++i)
		_pieces[i].registerGraphics();
}

void MultiBuildPuzzle::readData(Common::SeekableReadStream &stream) {
	readFilename(stream, _primaryImageName);

	Common::String secName;
	readFilename(stream, secName);
	_closeupImageName = Common::Path(secName);
	_hasCloseupImage = (secName != "NO_FILE" && !secName.empty());

	_numPieces = stream.readUint16LE();
	_requiredPieces = stream.readUint16LE();

	_autoSolveOnDrop = stream.readByte() != 0;
	_canRotateAll = stream.readByte() != 0;

	_useRotationHotspot = stream.readByte() != 0;
	_rotHotspotHeight   = stream.readSint16LE();
	_rotHotspotWidth    = stream.readSint16LE();

	readRect(stream, _targetZone);

	_allowAltZoneSnap = stream.readByte() != 0;
	_checkOverlapOnDrop = stream.readByte() != 0;

	// Pieces: data file always has 20 × 67-byte slots; only _numPieces are used.
	// Reserve up-front so counter-spawn push_back doesn't reallocate (pieces are
	// RenderObjects already registered with the graphics manager).
	_pieces.reserve(80);
	_pieces.resize(_numPieces);
	for (uint i = 0; i < 20; ++i) {
		if (i < _numPieces) {
			Piece &p = _pieces[i];
			readRect(stream, p.srcRect);
			readRect(stream, p.homeRect);
			readRect(stream, p.altSrcRect);
			readRect(stream, p.cuSrcRect);
			p.counterByte  = stream.readByte();
			p.mustPlace    = stream.readByte();
			p.mustNotPlace = stream.readByte();
		} else {
			stream.skip(67);
		}
	}

	_rotationSound.readNormal(stream);
	_pickupSound.readNormal(stream);
	_dropSound.readNormal(stream);

	_dragCursorID  = stream.readSint16LE();
	_exitCursorID1 = stream.readSint16LE();
	_exitCursorID2 = stream.readSint16LE();

	_solveScene.readData(stream);
	_solveSound.readNormal(stream);

	readFilename(stream, _solveTextKey);
	char textBuf[200];
	stream.read(textBuf, 200);
	assembleTextLine(textBuf, _solveText, 200);

	_cancelScene.readData(stream);

	readRect(stream, _exitHotspot);
	readRect(stream, _exitHotspot2);
}

void MultiBuildPuzzle::execute() {
	switch (_state) {
	case kBegin:
		init();
		registerGraphics();
		g_nancy->_sound->loadSound(_rotationSound);
		g_nancy->_sound->loadSound(_pickupSound);
		g_nancy->_sound->loadSound(_dropSound);
		g_nancy->_sound->loadSound(_solveSound);
		_state = kRun;
		// fall through
	case kRun:
		switch (_solveState) {
		case kIdle:
			break;

		case kWaitTimer:
			// Short debounce after placing a piece.
			if (g_system->getMillis() >= _timerEnd)
				_solveState = kIdle;
			break;

		case kPlaySolveSound:
			// Solve sound + caption are now played synchronously inside
			// checkIfSolved(); this state shouldn't normally be reached.
			_solveState = kWaitSolveSound;
			break;

		case kWaitSolveSound:
			if (!g_nancy->_sound->isSoundPlaying(_solveSound)) {
				g_nancy->_sound->stopSound(_solveSound);
				_state = kActionTrigger;
			}
			break;
		}
		break;

	case kActionTrigger:
		g_nancy->_sound->stopSound(_rotationSound);
		g_nancy->_sound->stopSound(_pickupSound);
		g_nancy->_sound->stopSound(_dropSound);
		g_nancy->_sound->stopSound(_solveSound);
		if (_isCancelled) {
			NancySceneState.changeScene(_cancelScene._sceneChange);
			// Cancel flag is only set if at least one piece was placed (or
			// spawned). For sandwich (all counter pieces) the spawn delta is
			// what trips the gate when a bad ingredient was placed.
			if (_cancelScene._flag.label != kFlagNoLabel) {
				uint16 count = 0;
				for (uint i = 0; i < _numPieces; ++i) {
					if (_pieces[i].isPlaced && _pieces[i].counterByte == 0)
						++count;
				}
				count += (uint16)(_pieces.size() - _numPieces);
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

CursorManager::CursorType MultiBuildPuzzle::cursorFromDataID(int16 id, CursorManager::CursorType fallback) const {
	if (id < 0 || id > 21)
		return fallback;
	return (CursorManager::CursorType)id;
}

bool MultiBuildPuzzle::altZoneSnapValid() const {
	// Valid when the dragged piece sits atop a moved piece, within an X
	// tolerance of 20 px and a Y tolerance of 4 px. Used for sand-castle
	// stacking when _allowAltZoneSnap is set.
	if (_pickedUpPiece < 0)
		return false;

	const Piece &pp = _pieces[_pickedUpPiece];
	int halfW = (pp.gameRect.width()  - 1) / 2;
	int halfH = (pp.gameRect.height() - 1) / 2;
	int cx = pp.gameRect.left + halfW;
	int cy = pp.gameRect.top  + halfH;

	for (uint i = 0; i < _pieces.size(); ++i) {
		if ((int)i == _pickedUpPiece)
			continue;
		const Piece &other = _pieces[i];
		if (other.gameRect == other.homeRect)
			continue;
		if (other.gameRect.top  - 4 < cy + halfH &&
		    cy < other.gameRect.top &&
		    other.gameRect.left - 20 < cx - halfW &&
		    cx + halfW < (other.gameRect.right - 1) + 20)
			return true;
	}
	return false;
}

void MultiBuildPuzzle::spawnCounterPiece(int srcIdx) {
	if (srcIdx < 0 || srcIdx >= (int)_pieces.size())
		return;
	// Defensive cap to avoid runaway memory use (sand castle can spawn endlessly).
	if (_pieces.size() >= 80)
		return;

	const Piece &src = _pieces[srcIdx];
	// All clones share surfaces with the original piece at typeIdx.
	int sharedType = (src.typeIdx >= 0) ? src.typeIdx : srcIdx;
	const Piece &surf = _pieces[sharedType];

	_pieces.push_back(Piece());
	Piece &np = _pieces.back();
	np.srcRect      = src.srcRect;
	np.homeRect     = src.homeRect;
	np.altSrcRect   = src.altSrcRect;
	np.cuSrcRect    = src.cuSrcRect;
	np.counterByte  = src.counterByte;
	np.mustPlace    = src.mustPlace;
	np.mustNotPlace = src.mustNotPlace;
	np.typeIdx      = sharedType;
	np.curRotation  = 0;
	np.gameRect     = np.homeRect;
	np.isPlaced     = false;

	// Each clone needs its own ManagedSurface since it's an independent RenderObject.
	for (int r = 0; r < 4; ++r) {
		if (!surf.hasSurface[r])
			continue;
		int w = surf.rotateSurfaces[r].w;
		int h = surf.rotateSurfaces[r].h;
		np.rotateSurfaces[r].create(w, h, surf.rotateSurfaces[r].format);
		np.rotateSurfaces[r].setTransparentColor(surf.rotateSurfaces[r].getTransparentColor());
		np.rotateSurfaces[r].blitFrom(surf.rotateSurfaces[r], Common::Point(0, 0));
		np.hasSurface[r] = true;
	}

	int newIdx = (int)_pieces.size() - 1;
	updatePieceRender(newIdx);
	_pieces[newIdx].setVisible(true);
	_pieces[newIdx].setTransparent(true);
	_pieces[newIdx].setZ((uint16)(_z + newIdx + 1));
	_pieces[newIdx].registerGraphics();
}

bool MultiBuildPuzzle::isValidDrop() const {
	if (_pickedUpPiece < 0)
		return false;

	const Piece &pp = _pieces[_pickedUpPiece];

	// Bounding-box-inside test. Left/top are strict; right/bottom allow up to
	// kEdgeTolerance px of overflow (matches the design tolerance the engine
	// uses for the overlap check below).
	const int kEdgeTolerance = 3;
	bool inTargetZone =
		!_targetZone.isEmpty() &&
		_targetZone.left   < pp.gameRect.left &&
		pp.gameRect.right  <= _targetZone.right  + kEdgeTolerance &&
		_targetZone.top    < pp.gameRect.top &&
		pp.gameRect.bottom <= _targetZone.bottom + kEdgeTolerance;

	if (!inTargetZone) {
		if (!_allowAltZoneSnap || !altZoneSnapValid())
			return false;
	}

	// Overlap check with 3-px tolerance: pieces are considered overlapping
	// only if their bounding boxes overlap by more than 3 pixels (so adjacent
	// or barely-touching placements are accepted).
	if (_checkOverlapOnDrop) {
		int ppLeft   = pp.gameRect.left;
		int ppTop    = pp.gameRect.top;
		int ppRight  = pp.gameRect.right  - 1;
		int ppBottom = pp.gameRect.bottom - 1;

		for (uint i = 0; i < _pieces.size(); ++i) {
			if ((int)i == _pickedUpPiece)
				continue;
			if (!_pieces[i].isPlaced)
				continue;
			const Piece &other = _pieces[i];
			int otherLeft   = other.gameRect.left;
			int otherTop    = other.gameRect.top;
			int otherRight  = other.gameRect.right  - 1;
			int otherBottom = other.gameRect.bottom - 1;

			bool xOverlap = false;
			if (ppLeft < otherLeft)
				xOverlap = (otherLeft + 3 <= ppRight);
			else if (ppLeft <= otherRight - 3)
				xOverlap = true;

			if (!xOverlap)
				continue;

			bool yOverlap = false;
			if (ppTop < otherTop)
				yOverlap = (otherTop + 3 <= ppBottom);
			else if (ppTop <= otherBottom - 3)
				yOverlap = true;

			if (yOverlap)
				return false;
		}
	}

	return true;
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

	CursorManager::CursorType dragCursor = cursorFromDataID(_dragCursorID, CursorManager::kCustom1);

	if (_isDragging) {
		// Centre dragged piece on cursor. Offset uses (width-1)/2 to match
		// the original's inclusive-coordinate delta arithmetic.
		Piece &pp = _pieces[_pickedUpPiece];
		int newLeft = mouseVP.x - (_pickedUpWidth  - 1) / 2;
		int newTop  = mouseVP.y - (_pickedUpHeight - 1) / 2;
		pp.gameRect.left   = newLeft;
		pp.gameRect.top    = newTop;
		pp.gameRect.right  = newLeft + _pickedUpWidth;
		pp.gameRect.bottom = newTop  + _pickedUpHeight;
		updatePieceRender(_pickedUpPiece);
		bool validDrop = isValidDrop();

		g_nancy->_cursor->setCursorType(validDrop ? dragCursor : CursorManager::kNormal);

		// Right click: rotate the carried piece
		if ((input.input & NancyInput::kRightMouseButtonUp) && pp.hasSurface[1]) {
			pp.curRotation = (pp.curRotation + 1) % 4;
			_pickedUpWidth  = pp.rotateSurfaces[pp.curRotation].w;
			_pickedUpHeight = pp.rotateSurfaces[pp.curRotation].h;
			g_nancy->_sound->playSound(_rotationSound);
			updatePieceRender(_pickedUpPiece);
			return;
		}

		// Left click: drop. Invalid drop returns the piece to its home rect.
		if (input.input & NancyInput::kLeftMouseButtonUp) {
			// Clear drag state before updatePieceRender so the correct visual is chosen.
			_isDragging = false;
			int placedIdx = _pickedUpPiece;
			_pickedUpPiece = -1;

			if (validDrop) {
				pp.isPlaced = true;
				g_nancy->_sound->playSound(_dropSound);

				// Counter pieces respawn at home for unlimited supply.
				if (pp.counterByte != 0)
					spawnCounterPiece(placedIdx);
			} else {
				pp.gameRect = pp.homeRect;
			}

			updatePieceRender(placedIdx);

			// Solve check runs on drop for puzzles with _autoSolveOnDrop, and
			// also once piece count grows past the original's auto-solve trigger.
			if (_autoSolveOnDrop || _pieces.size() > 79)
				checkIfSolved();

			if (_solveState == kIdle) {
				_solveState = kWaitTimer;
				_timerEnd = g_system->getMillis() + 300;
			}
		}
		return;
	}

	if (_selectedPiece != -1) {
		g_nancy->_cursor->setCursorType(dragCursor);

		if (input.input & NancyInput::kLeftMouseButtonUp) {
			int sel = _selectedPiece;
			_selectedPiece = -1;
			Piece &pp = _pieces[sel];
			_pickedUpPiece = sel;
			_isDragging = true;
			pp.curRotation = 0;
			_pickedUpWidth  = pp.rotateSurfaces[0].w;
			_pickedUpHeight = pp.rotateSurfaces[0].h;
			int newLeft = mouseVP.x - (_pickedUpWidth  - 1) / 2;
			int newTop  = mouseVP.y - (_pickedUpHeight - 1) / 2;
			pp.gameRect = Common::Rect(newLeft, newTop,
			                          newLeft + _pickedUpWidth, newTop + _pickedUpHeight);
			updatePieceRender(sel);
			g_nancy->_sound->playSound(_pickupSound);
		}
		return;
	}

	// Find a piece to pick up. First pass detects the rotation hotspot
	// (a small rect at each piece's top-left) when rotation is enabled.
	int16 topmostRot = -1;
	int16 topmost = -1;
	bool rotationsEnabled = _canRotateAll && _useRotationHotspot &&
	                        _rotHotspotWidth > 0 && _rotHotspotHeight > 0;
	for (int i = (int)_pieces.size() - 1; i >= 0; --i) {
		Piece &p = _pieces[i];
		if (!p.gameRect.contains(mouseVP))
			continue;
		// Placed counter pieces are locked (can't re-grab the placed ingredient).
		if (p.isPlaced && p.counterByte != 0)
			continue;
		if (rotationsEnabled) {
			Common::Rect rotRect(p.gameRect.left, p.gameRect.top,
			                     p.gameRect.left + _rotHotspotWidth,
			                     p.gameRect.top  + _rotHotspotHeight);
			if (rotRect.contains(mouseVP)) {
				if (topmostRot == -1 || p.getZOrder() > _pieces[topmostRot].getZOrder())
					topmostRot = (int16)i;
				continue;
			}
		}
		if (topmost == -1 || p.getZOrder() > _pieces[topmost].getZOrder())
			topmost = (int16)i;
	}

	if (topmostRot != -1) {
		// Hovering rotation hotspot: click rotates 90° and picks up.
		g_nancy->_cursor->setCursorType(CursorManager::kRotateCW);
		if (input.input & NancyInput::kLeftMouseButtonUp) {
			Piece &pp = _pieces[topmostRot];
			pp.isPlaced = false;
			pp.curRotation = (pp.curRotation + 1) % 4;
			if (!pp.hasSurface[pp.curRotation])
				pp.curRotation = 0;
			pp.setZ((uint16)(_z + (int)_pieces.size() * 2));
			pp.registerGraphics();
			_isDragging = true;
			_pickedUpPiece = topmostRot;
			_pickedUpWidth  = pp.rotateSurfaces[pp.curRotation].w;
			_pickedUpHeight = pp.rotateSurfaces[pp.curRotation].h;
			g_nancy->_sound->playSound(_rotationSound);
			updatePieceRender(topmostRot);
		}
		return;
	}

	if (topmost != -1) {
		g_nancy->_cursor->setCursorType(dragCursor);

		if (input.input & NancyInput::kLeftMouseButtonUp) {
			Piece &pp = _pieces[topmost];
			pp.isPlaced = false;
			pp.curRotation = 0;
			pp.setZ((uint16)(_z + (int)_pieces.size() * 2));
			pp.registerGraphics();

			if (_hasCloseupImage && !pp.cuSrcRect.isEmpty()) {
				// First click shows the closeup view centred on the piece.
				_selectedPiece = topmost;
				const int cuW = pp.cuSrcRect.width();
				const int cuH = pp.cuSrcRect.height();
				const int pieceW = pp.rotateSurfaces[pp.curRotation].w;
				const int pieceH = pp.rotateSurfaces[pp.curRotation].h;
				const int centerX = pp.gameRect.left + pieceW / 2;
				const int centerY = pp.gameRect.top  + pieceH / 2;
				int cuLeft = centerX - cuW / 2;
				int cuTop  = centerY - cuH / 2;
				cuLeft = CLIP<int>(cuLeft, 0, MAX(0, vpScreen.width()  - cuW));
				cuTop  = CLIP<int>(cuTop,  0, MAX(0, vpScreen.height() - cuH));
				pp.gameRect = Common::Rect(cuLeft, cuTop, cuLeft + cuW, cuTop + cuH);
			} else {
				// Direct drag on first click.
				_isDragging = true;
				_pickedUpPiece = topmost;
				_pickedUpWidth  = pp.rotateSurfaces[0].w;
				_pickedUpHeight = pp.rotateSurfaces[0].h;
				g_nancy->_sound->playSound(_pickupSound);
			}
			updatePieceRender(topmost);
		}
		return;
	}

	// Exit hotspots: a click in either fires the exit path. Each uses its own data cursor id.
	if (!checkExitHotspot(_exitHotspot, _exitCursorID1, input))
		checkExitHotspot(_exitHotspot2, _exitCursorID2, input);
}

bool MultiBuildPuzzle::checkExitHotspot(const Common::Rect &hot, int16 cursorID, const NancyInput &input) {
	if (hot.isEmpty())
		return false;
	Common::Rect exitScreen = NancySceneState.getViewport().convertViewportToScreen(hot);
	if (!exitScreen.contains(input.mousePos))
		return false;
	g_nancy->_cursor->setCursorType(cursorFromDataID(cursorID, g_nancy->_cursor->_puzzleExitCursor));
	if (input.input & NancyInput::kLeftMouseButtonUp)
		checkIfSolvedOnExit();
	return true;
}

void MultiBuildPuzzle::checkIfSolvedOnExit() {
	// Run the solve check; if it doesn't mark the puzzle solved, cancel out.
	checkIfSolved();
	if (!_isSolved) {
		_isCancelled = true;
		_state = kActionTrigger;
	}
}

void MultiBuildPuzzle::checkIfSolved() {
	// Count = placed pieces with counterByte == 0, plus the spawn delta
	// (so counter-piece puzzles like sandwich count each placement).
	uint16 count = 0;
	for (uint i = 0; i < _numPieces; ++i) {
		if (_pieces[i].isPlaced && _pieces[i].counterByte == 0)
			++count;
	}
	count += (uint16)(_pieces.size() - _numPieces);

	if (count < _requiredPieces)
		return;

	// Bail without solving on any constraint failure.
	for (uint i = 0; i < _numPieces; ++i) {
		if (_pieces[i].isPlaced && _pieces[i].mustNotPlace)
			return;
		if (!_pieces[i].isPlaced && _pieces[i].mustPlace)
			return;
	}

	// Play sound + caption inline (rather than parking in kPlaySolveSound for
	// execute() to pick up) so the transition is deterministic regardless of
	// action-record processing order.
	_isSolved = true;
	g_nancy->_sound->playSound(_solveSound);

	// Caption: prefer the CONVO lookup of the text key. An empty lookup
	// result means audio-only — keep the textbox empty. Only fall back to
	// the raw _solveText when the key isn't in CONVO.
	Common::String textToShow;
	bool useLookup = false;
	if (!_solveTextKey.empty()) {
		const CVTX *convo = (const CVTX *)g_nancy->getEngineData("CONVO");
		if (convo && convo->texts.contains(_solveTextKey)) {
			textToShow = convo->texts[_solveTextKey];
			useLookup = true;
		}
	}
	if (!useLookup)
		textToShow = _solveText;
	if (!textToShow.empty()) {
		NancySceneState.getTextbox().clear();
		NancySceneState.getTextbox().addTextLine(textToShow);
	}

	_solveState = kWaitSolveSound;
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
		// Zoomed closeup.
		int w = p.cuSrcRect.width();
		int h = p.cuSrcRect.height();
		p._drawSurface.create(w, h, _closeupImage.format);
		p._drawSurface.setTransparentColor(_closeupImage.getTransparentColor());
		p._drawSurface.blitFrom(_closeupImage, p.cuSrcRect, Common::Point(0, 0));
	} else {
		// At rest on the shelf: show srcRect from primary image.
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
