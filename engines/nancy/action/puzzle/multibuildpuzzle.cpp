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

// Nancy 13 stores several flags outside a SceneChangeWithFlag.
static void readFlag(Common::SeekableReadStream &stream, FlagDescription &flag) {
	flag.label = stream.readSint16LE();
	flag.flag = stream.readByte();
}

MultiBuildPuzzle::~MultiBuildPuzzle() {
	for (uint i = 0; i < _pieces.size(); ++i)
		delete _pieces[i];
}

void MultiBuildPuzzle::init() {
	g_nancy->_resource->loadImage(_primaryImageName, _primaryImage);
	_primaryImage.setTransparentColor(_drawSurface.getTransparentColor());

	if (_hasCloseupImage) {
		g_nancy->_resource->loadImage(_closeupImageName, _closeupImage);
		_closeupImage.setTransparentColor(_drawSurface.getTransparentColor());
	}

	if (_hasAnimImage) {
		_animRender._drawSurface.create(_animRect.width(), _animRect.height(),
				g_nancy->_graphics->getInputPixelFormat());
		_animRender.setTransparent(true);
		_animRender._drawSurface.clear(g_nancy->_graphics->getTransColor());
		_animRender.moveTo(_animRect);
		_animRender.setZOrder((uint16)(_z + 80 + 10));
		_animRender.setVisible(true);

		g_nancy->_resource->loadImage(_animImageName, _animImage);
		_animImage.setTransparentColor(g_nancy->_graphics->getTransColor());

		_animSurfaceReady = true;
	}

	for (uint i = 0; i < _pieces.size(); ++i) {
		Piece &p = *_pieces[i];

		if (!p.imageName.empty()) {
			// Nancy 13: the piece brings its own closeup image.
			g_nancy->_resource->loadImage(p.imageName, p.image);
			p.image.setTransparentColor(_drawSurface.getTransparentColor());
		}

		// Fixtures are never dragged, so they get no drag sprites.
		if (p.kind == kPieceIngredient) {
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
		}

		p.curRotation = 0;
		p.gameRect = p.homeRect;
		p.isPlaced = false;

		updatePieceRender(i);
		p.setVisible(true);
		p.setTransparent(true);
		p.setZOrder((uint16)(_z + i + 1));
	}

	_isInitialized = true;
}

void MultiBuildPuzzle::registerGraphics() {
	if (!_isInitialized)
		return;

	if (_animSurfaceReady)
		_animRender.registerGraphics();

	for (uint i = 0; i < _pieces.size(); ++i)
		_pieces[i]->registerGraphics();
}

void MultiBuildPuzzle::readData(Common::SeekableReadStream &stream) {
	const bool isNancy10 = g_nancy->getGameType() >= kGameTypeNancy10;
	const bool isNancy13 = g_nancy->getGameType() >= kGameTypeNancy13;

	readFilename(stream, _primaryImageName);

	Common::String secName;
	readFilename(stream, secName);
	_closeupImageName = Common::Path(secName);
	_hasCloseupImage = (secName != "NO_FILE" && !secName.empty());

	if (isNancy10) {
		_retainState = stream.readByte() != 0;	// TODO: state isn't saved yet
	}

	_numPieces = stream.readUint16LE();
	_requiredPieces = stream.readUint16LE();

	_autoSolveOnDrop = stream.readByte() != 0;
	_canRotateAll = stream.readByte() != 0;

	_useRotationHotspot = stream.readByte() != 0;
	_rotHotspotHeight   = stream.readSint16LE();
	_rotHotspotWidth    = stream.readSint16LE();

	readRect(stream, _targetZone);

	_altZoneSnapMode = stream.readByte();
	_allowAltZoneSnap = _altZoneSnapMode != 0;
	_checkOverlapOnDrop = stream.readByte() != 0;

	if (isNancy10) {
		// Anim block: name + rect + 4 layout shorts (cols/framesPerStep/spacing/totalRows)
		Common::String animName;
		readFilename(stream, animName);
		_animImageName = Common::Path(animName);
		_hasAnimImage = (animName != "NO_FILE" && !animName.empty());
		readRect(stream, _animRect);
		for (uint i = 0; i < 4; ++i)
			_animLayout[i] = stream.readSint16LE();
	}

	// Pieces: the data file always has 20 slots; only _numPieces are used.
	// Slot size: 67 bytes in Nancy 9, 83 in Nancy 10, 116 in Nancy 13.
	const uint pieceSize = isNancy13 ? 116 : (isNancy10 ? 83 : 67);
	for (uint i = 0; i < _numPieces; ++i)
		_pieces.push_back(new Piece());
	for (uint i = 0; i < 20; ++i) {
		if (i >= _numPieces) {
			stream.skip(pieceSize);
			continue;
		}

		Piece &p = *_pieces[i];
		// srcRect is empty when the unplaced piece is baked into the scene
		// overlay (cake mixing) and non-empty when it must be rendered live at
		// rest (plant potting).
		readRect(stream, p.srcRect);
		readRect(stream, p.homeRect);
		readRect(stream, p.altSrcRect);
		readRect(stream, p.cuSrcRect);

		if (isNancy10) {
			readRect(stream, p.placedDstRect);
		}

		if (isNancy13) {
			Common::String pieceImage;
			readFilename(stream, pieceImage);
			if (pieceImage != "NO_FILE" && !pieceImage.empty())
				p.imageName = Common::Path(pieceImage);
		}

		p.counterByte = stream.readByte();
		p.mustPlace = stream.readByte();

		if (isNancy13) {
			// The count byte doubles as a piece kind; fixtures take no
			// part in the recipe check.
			if (p.mustPlace >= kPieceInspect) {
				p.kind = p.mustPlace;
				p.mustPlace = 0;
			}
			p.compare = stream.readByte();
		} else {
			// Older games only distinguish "must be placed" from "must not be".
			p.compare = stream.readByte() != 0 ? kDoNotPlace : kPlaceExactly;
		}
	}

	_rotationSound.readNormal(stream);
	_pickupSound.readNormal(stream);
	_dropSound.readNormal(stream);

	if (isNancy10) {
		// "Missed" feedback when an ingredient is thrown away: a sound plus a
		// caption laid out exactly like the solve caption (33-byte CONVO key +
		// 200-byte raw fallback).
		_missedSound.readNormal(stream);
		readFilename(stream, _missedTextKey);
		char missedBuf[200];
		stream.read(missedBuf, 200);
		assembleTextLine(missedBuf, _missedText, 200);
	}

	char textBuf[200];

	if (isNancy13) {
		// Flags now precede their scene, the exit cursors sit next to their
		// hotspots, and the submit scene/sound/caption are new.
		_dragCursorID = stream.readSint16LE();

		readFlag(stream, _minCountFlag);
		_solveScene._sceneChange.readData(stream);
		stream.skip(2); // shouldStopRendering

		_solveSound.readNormal(stream);
		readFilename(stream, _solveTextKey);
		stream.read(textBuf, 200);
		assembleTextLine(textBuf, _solveText, 200);

		readFlag(stream, _solveScene._flag);
		_submitScene._sceneChange.readData(stream);
		stream.skip(2);

		_submitSound.readNormal(stream);
		readFilename(stream, _submitTextKey);
		stream.read(textBuf, 200);
		assembleTextLine(textBuf, _submitText, 200);

		readRect(stream, _exitHotspot);
		_exitCursorID1 = stream.readSint16LE();
		readFlag(stream, _submitScene._flag);

		_cancelScene._sceneChange.readData(stream);
		stream.skip(2);

		readRect(stream, _exitHotspot2);
		_exitCursorID2 = stream.readSint16LE();
		readFlag(stream, _cancelScene._flag);
		return;
	}

	_dragCursorID  = stream.readSint16LE();
	_exitCursorID1 = stream.readSint16LE();
	_exitCursorID2 = stream.readSint16LE();

	_solveScene.readData(stream);
	_solveSound.readNormal(stream);

	readFilename(stream, _solveTextKey);
	stream.read(textBuf, 200);
	assembleTextLine(textBuf, _solveText, 200);

	_cancelScene.readData(stream);

	readRect(stream, _exitHotspot);
	readRect(stream, _exitHotspot2);

	_minCountFlag = _cancelScene._flag;
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
		g_nancy->_sound->loadSound(_missedSound);
		g_nancy->_sound->loadSound(_submitSound);
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

		case kWaitSubmitSound:
			if (!g_nancy->_sound->isSoundPlaying(_submitSound)) {
				g_nancy->_sound->stopSound(_submitSound);
				_state = kActionTrigger;
			}
			break;

		case kResetButtonHeld:
			// The button shows its pressed sprite briefly, then clears the board.
			if (g_system->getMillis() >= _timerEnd) {
				_solveState = kIdle;
				resetPuzzle();
			}
			break;

		case kAnimStep: {
			if (!_hasAnimImage) {
				if (g_system->getMillis() >= _timerEnd)
					_solveState = kIdle;
				break;
			}

			_animActive = true;

			if (_dropSound.name != "NO SOUND" &&
					!g_nancy->_sound->isSoundPlaying(_dropSound)) {
				g_nancy->_sound->playSound(_dropSound);
			}

			if (_animLayout[1] > 0 &&
					_animFrameCounter / _animLayout[1] >= _animLayout[0]) {
				_animFrameCounter = 0;
				_animRowCounter++;
			}

			if (_animRowCounter < _animLayout[3] && _animLayout[1] > 0) {
				// pdW/pdH match the raw inclusive `right - left` (our exclusive width - 1)
				int pdW = _animRect.width()  - 1;
				int pdH = _animRect.height() - 1;
				int spacing = _animLayout[2];
				int col = _animFrameCounter % _animLayout[1];
				int row = _animFrameCounter / _animLayout[1];
				int srcL = (pdW + spacing) * col + 1;
				int srcT = (pdH + spacing) * row + 1;
				_animSrcRect = Common::Rect(srcL, srcT, srcL + pdW, srcT + pdH);
				_animFrameCounter++;
				renderAnimFrame();
				_animFrameWaitEnd = g_system->getMillis() + 100;
				_solveState = kAnimWaitFrame;
			} else {
				_animRowCounter = 0;
				_animFrameCounter = 0;
				_animEnded = true;
				_animActive = false;
				clearAnimFrame();
				_solveState = kIdle;
			}
			break;
		}

		case kAnimWaitFrame:
			if (g_system->getMillis() >= _animFrameWaitEnd)
				_solveState = kAnimStep;
			break;
		}
		break;

	case kActionTrigger:
		g_nancy->_sound->stopSound(_rotationSound);
		g_nancy->_sound->stopSound(_pickupSound);
		g_nancy->_sound->stopSound(_dropSound);
		g_nancy->_sound->stopSound(_solveSound);
		g_nancy->_sound->stopSound(_missedSound);
		g_nancy->_sound->stopSound(_submitSound);
		if (_isSubmitted) {
			NancySceneState.setEventFlag(_submitScene._flag);
			NancySceneState.changeScene(_submitScene._sceneChange);
		} else if (_isCancelled) {
			NancySceneState.changeScene(_cancelScene._sceneChange);
			// Cancel flag is only set if at least one piece was placed (or
			// spawned). For sandwich (all counter pieces) the spawn delta is
			// what trips the gate when a bad ingredient was placed.
			if (_cancelScene._flag.label != kFlagNoLabel) {
				uint16 count = 0;
				for (uint i = 0; i < _numPieces; ++i) {
					if (_pieces[i]->isPlaced && _pieces[i]->counterByte == 0)
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
	// Nancy 13 ids reach past the older games' cursor range.
	const int16 maxID = g_nancy->getGameType() >= kGameTypeNancy13
		? (int16)g_nancy->getStaticData().numCursorTypes - 1 : 21;
	if (id < 0 || id > maxID)
		return fallback;
	return (CursorManager::CursorType)id;
}

bool MultiBuildPuzzle::altZoneSnapValid() const {
	// Valid when the dragged piece sits atop a moved piece, within an X
	// tolerance of 20 px and a Y tolerance of 4 px. Used for sand-castle
	// stacking when _allowAltZoneSnap is set.
	if (_pickedUpPiece < 0)
		return false;

	const Piece &pp = *_pieces[_pickedUpPiece];
	int halfW = (pp.gameRect.width()  - 1) / 2;
	int halfH = (pp.gameRect.height() - 1) / 2;
	int cx = pp.gameRect.left + halfW;
	int cy = pp.gameRect.top  + halfH;

	for (uint i = 0; i < _pieces.size(); ++i) {
		if ((int)i == _pickedUpPiece)
			continue;
		const Piece &other = *_pieces[i];
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

	const Piece &src = *_pieces[srcIdx];
	// All clones share surfaces with the original piece at typeIdx.
	int sharedType = (src.typeIdx >= 0) ? src.typeIdx : srcIdx;
	const Piece &surf = *_pieces[sharedType];

	_pieces.push_back(new Piece());
	Piece &np = *_pieces.back();
	np.srcRect      = src.srcRect;
	np.homeRect     = src.homeRect;
	np.altSrcRect   = src.altSrcRect;
	np.cuSrcRect    = src.cuSrcRect;
	np.placedDstRect = src.placedDstRect;
	np.counterByte  = src.counterByte;
	np.mustPlace    = src.mustPlace;
	np.compare      = src.compare;
	np.kind         = src.kind;
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
	_pieces[newIdx]->setVisible(true);
	_pieces[newIdx]->setTransparent(true);
	_pieces[newIdx]->setZOrder((uint16)(_z + newIdx + 1));
	_pieces[newIdx]->registerGraphics();
}

bool MultiBuildPuzzle::isValidDrop() const {
	if (_pickedUpPiece < 0)
		return false;

	const Piece &pp = *_pieces[_pickedUpPiece];

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
			if (!_pieces[i]->isPlaced)
				continue;
			const Piece &other = *_pieces[i];
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
	if (_state != kRun || _solveState != kIdle || _isSolved || _isCancelled || _isSubmitted)
		return;

	const VIEW *viewData = GetEngineData(VIEW);
	if (!viewData)
		return;
	Common::Rect vpScreen = viewData->screenPosition;

	// Exit hotspots can sit below the viewport (cake mixing). They remain
	// reachable while an ingredient closeup is showing, so the player can back
	// away at the bottom of the screen to cancel adding it.
	if (!_isDragging && !vpScreen.contains(input.mousePos)) {
		if (!checkExitHotspot(_exitHotspot, _exitCursorID1, true, input))
			checkExitHotspot(_exitHotspot2, _exitCursorID2, false, input);
		return;
	}

	if (!vpScreen.contains(input.mousePos))
		return;

	Common::Point mouseVP(input.mousePos.x - vpScreen.left,
	                      input.mousePos.y - vpScreen.top);

	CursorManager::CursorType dragCursor = cursorFromDataID(_dragCursorID, CursorManager::kCustom1);

	if (_isDragging) {
		// Centre dragged piece on cursor. Offset uses (width-1)/2 to match
		// the original's inclusive-coordinate delta arithmetic.
		Piece &pp = *_pieces[_pickedUpPiece];
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

			const bool isNancy10 = g_nancy->getGameType() >= kGameTypeNancy10;
			// "Add ingredient" mode (cake mixing): a valid drop just bumps the
			// ingredient's placement count and returns the piece to the shelf;
			// there is a single piece per ingredient (no counter-spawn), so the
			// count is tracked purely by placeCount.
			const bool addMode = isNancy10 && _altZoneSnapMode == 2;

			if (validDrop) {
				int srcIdx = (pp.typeIdx >= 0) ? pp.typeIdx : placedIdx;
				if (isNancy10 && _pieces[srcIdx]->placeCount < 255)
					_pieces[srcIdx]->placeCount++;

				if (addMode) {
					// Ingredient goes back to its shelf slot, still pickable.
					pp.isPlaced = false;
					pp.gameRect = pp.homeRect;
				} else {
					pp.isPlaced = true;
					g_nancy->_sound->playSound(_dropSound);
					// Counter pieces respawn at home for unlimited supply.
					if (pp.counterByte != 0)
						spawnCounterPiece(placedIdx);
				}

				if (isNancy10)
					updateSolveFlags();
			} else {
				// Missed: the ingredient was thrown away. Return it to the shelf
				// and play the "Missed." feedback (sound + caption).
				pp.isPlaced = false;
				pp.gameRect = pp.homeRect;
				if (isNancy10 && _missedSound.name != "NO SOUND") {
					g_nancy->_sound->playSound(_missedSound);
					showSubtitle(resolveSubtitleText(_missedTextKey, _missedText, "CONVO"));
					if (_solveState == kIdle) {
						_solveState = kWaitTimer;
						_timerEnd = g_system->getMillis() + 200;
					}
				}
			}

			updatePieceRender(placedIdx);

			if (g_nancy->getGameType() >= kGameTypeNancy13 && _pieces.size() > 79) {
				// Safety valve: the piece array is full, so hand the order over.
				submitPuzzle();
			} else if (_autoSolveOnDrop || _pieces.size() > 79) {
				checkIfSolved();
			}

			if (validDrop && _hasAnimImage && _solveState != kAnimStep &&
					_solveState != kAnimWaitFrame) {
				_animFrameCounter = 0;
				_animRowCounter = 0;
				_animActive = true;
				_animEnded = false;
				_solveState = kAnimStep;
			} else if (_solveState == kIdle) {
				_solveState = kWaitTimer;
				_timerEnd = g_system->getMillis() + 300;
			}
		}
		return;
	}

	if (_selectedPiece != -1) {
		Piece &pp = *_pieces[_selectedPiece];

		// Only the closeup itself is interactive: clicking it picks it up to
		// drag. Anywhere else the exit hotspots stay live, so the player can back
		// away at the bottom of the screen to cancel adding the ingredient.
		if (!pp.gameRect.contains(mouseVP)) {
			if (!checkExitHotspot(_exitHotspot, _exitCursorID1, true, input))
				checkExitHotspot(_exitHotspot2, _exitCursorID2, false, input);
			return;
		}

		g_nancy->_cursor->setCursorType(dragCursor, true);

		if ((input.input & NancyInput::kLeftMouseButtonUp) && pp.kind != kPieceIngredient) {
			// Fixtures are only there to be looked at; clicking puts them away.
			int sel = _selectedPiece;
			_selectedPiece = -1;
			pp.gameRect = pp.homeRect;
			updatePieceRender(sel);
			return;
		}

		if (input.input & NancyInput::kLeftMouseButtonUp) {
			int sel = _selectedPiece;
			_selectedPiece = -1;
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
		Piece &p = *_pieces[i];
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
				if (topmostRot == -1 || p.getZOrder() > _pieces[topmostRot]->getZOrder())
					topmostRot = (int16)i;
				continue;
			}
		}
		if (topmost == -1 || p.getZOrder() > _pieces[topmost]->getZOrder())
			topmost = (int16)i;
	}

	if (topmostRot != -1) {
		// Hovering rotation hotspot: click rotates 90° and picks up.
		g_nancy->_cursor->setCursorType(CursorManager::kRotateCW);
		if (input.input & NancyInput::kLeftMouseButtonUp) {
			Piece &pp = *_pieces[topmostRot];
			pp.isPlaced = false;
			pp.curRotation = (pp.curRotation + 1) % 4;
			if (!pp.hasSurface[pp.curRotation])
				pp.curRotation = 0;
			pp.setZOrder((uint16)(_z + (int)_pieces.size() * 2));
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
		g_nancy->_cursor->setCursorType(dragCursor, true);

		if (input.input & NancyInput::kLeftMouseButtonUp) {
			Piece &pp = *_pieces[topmost];
			pp.isPlaced = false;
			pp.curRotation = 0;
			pp.setZOrder((uint16)(_z + (int)_pieces.size() * 2));
			pp.registerGraphics();

			if ((_hasCloseupImage || !pp.image.empty()) && !pp.cuSrcRect.isEmpty()) {
				// First click shows the closeup view.
				openCloseup(topmost, vpScreen);
				if (pp.kind == kPieceReset) {
					// The reset button clears the board on its own.
					_solveState = kResetButtonHeld;
					_timerEnd = g_system->getMillis() + 200;
				}
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
	if (!checkExitHotspot(_exitHotspot, _exitCursorID1, true, input))
		checkExitHotspot(_exitHotspot2, _exitCursorID2, false, input);
}

void MultiBuildPuzzle::openCloseup(int pieceIdx, const Common::Rect &viewportScreen) {
	// A piece with a fixed closeup destination uses it (cake mixing, all of
	// Nancy 13); otherwise the closeup is centred on the piece (plant potting).
	Piece &p = *_pieces[pieceIdx];
	_selectedPiece = (int16)pieceIdx;

	const int cuW = p.cuSrcRect.width();
	const int cuH = p.cuSrcRect.height();
	int cuLeft;
	int cuTop;
	if (!p.placedDstRect.isEmpty()) {
		cuLeft = p.placedDstRect.left;
		cuTop  = p.placedDstRect.top;
	} else {
		const int pieceW = p.rotateSurfaces[p.curRotation].w;
		const int pieceH = p.rotateSurfaces[p.curRotation].h;
		cuLeft = p.gameRect.left + pieceW / 2 - cuW / 2;
		cuTop  = p.gameRect.top  + pieceH / 2 - cuH / 2;
		cuLeft = CLIP<int>(cuLeft, 0, MAX(0, viewportScreen.width()  - cuW));
		cuTop  = CLIP<int>(cuTop,  0, MAX(0, viewportScreen.height() - cuH));
	}
	p.gameRect = Common::Rect(cuLeft, cuTop, cuLeft + cuW, cuTop + cuH);
}

bool MultiBuildPuzzle::checkExitHotspot(const Common::Rect &hot, int16 cursorID, bool isSubmit, const NancyInput &input) {
	if (hot.isEmpty())
		return false;
	Common::Rect exitScreen = NancySceneState.getViewport().convertViewportToScreen(hot);
	if (!exitScreen.contains(input.mousePos))
		return false;
	g_nancy->_cursor->setCursorType(cursorFromDataID(cursorID, g_nancy->_cursor->_puzzleExitCursor),
		g_nancy->getGameType() >= kGameTypeNancy13);
	if (input.input & NancyInput::kLeftMouseButtonUp) {
		if (isSubmit && g_nancy->getGameType() >= kGameTypeNancy13)
			submitPuzzle();
		else
			checkIfSolvedOnExit();
	}
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

void MultiBuildPuzzle::submitPuzzle() {
	// The order is handed over as-is; the two flags say what was in it.
	updateSolveFlags();
	_isSubmitted = true;
	g_nancy->_sound->playSound(_submitSound);
	showSubtitle(resolveSubtitleText(_submitTextKey, _submitText, "CONVO"));
	_solveState = kWaitSubmitSound;
}

void MultiBuildPuzzle::resetPuzzle() {
	// Counter clones live past _numPieces; dropping them deregisters their
	// render objects.
	while (_pieces.size() > _numPieces) {
		delete _pieces.back();
		_pieces.pop_back();
	}

	_selectedPiece = -1;
	_pickedUpPiece = -1;
	_isDragging = false;

	for (uint i = 0; i < _pieces.size(); ++i) {
		Piece &p = *_pieces[i];
		p.placeCount = 0;
		p.isPlaced = false;
		p.curRotation = 0;
		p.gameRect = p.homeRect;
		updatePieceRender(i);
	}

	// The flags are rewritten on the next board change.
	_minCountFlagLastValue = -1;
	_solveFlagLastValue = -1;
}

bool MultiBuildPuzzle::updateSolveFlags() {
	uint16 total = 0;
	for (uint i = 0; i < _numPieces; ++i) {
		if (_pieces[i]->counterByte == 0 || _allowAltZoneSnap)
			total += _pieces[i]->placeCount;
	}
	total += (uint16)(_pieces.size() - _numPieces);

	// The cancel scene's flag is the "enough ingredients" flag that enables the
	// BAKE option in the neighbouring scene. It is raised at/above the threshold.
	// Below the threshold, Nancy 10 leaves it latched (so backing away from the
	// counter and stepping back up keeps BAKE available), while Nancy 11+ clears
	// it. Written only on a value change (see member comment).
	if (_minCountFlag.label != kFlagNoLabel) {
		const bool enough = total >= _requiredPieces;
		if (enough || g_nancy->getGameType() >= kGameTypeNancy11) {
			byte want = enough ? _minCountFlag.flag
				: (_minCountFlag.flag == g_nancy->_false ? g_nancy->_true : g_nancy->_false);
			if ((int)want != _minCountFlagLastValue) {
				NancySceneState.setEventFlag(_minCountFlag.label, want);
				_minCountFlagLastValue = want;
			}
		}
	}

	if (total < _requiredPieces)
		return false;

	// The solve scene's flag marks an exact recipe match. It is cleared until
	// every ingredient count matches; only then is it raised to its true value.
	bool exact = true;
	for (uint i = 0; i < _numPieces; ++i) {
		const Piece &p = *_pieces[i];
		if (p.placeCount > 0 && p.compare == kDoNotPlace) {
			exact = false;
			break;
		}
		// A zero mustPlace means no count requirement (cake cooking).
		if (p.mustPlace > 0) {
			if (p.compare == kPlaceExactly && p.placeCount != p.mustPlace) {
				exact = false;
				break;
			}
			if (p.compare == kPlaceAtLeast && p.placeCount < p.mustPlace) {
				exact = false;
				break;
			}
		}
	}

	if (_solveScene._flag.label != kFlagNoLabel) {
		byte want = exact ? _solveScene._flag.flag
			: (_solveScene._flag.flag == g_nancy->_false ? g_nancy->_true : g_nancy->_false);
		if ((int)want != _solveFlagLastValue) {
			NancySceneState.setEventFlag(_solveScene._flag.label, want);
			_solveFlagLastValue = want;
		}
	}

	return exact;
}

void MultiBuildPuzzle::checkIfSolved() {
	const bool isNancy10 = g_nancy->getGameType() >= kGameTypeNancy10;

	if (isNancy10) {
		// Exact-count match; solveScene == kNoScene means the dialog handles
		// the transition via the solve flag set in updateSolveFlags.
		bool exactMatch = updateSolveFlags();
		if (!exactMatch || _solveScene._sceneChange.sceneID == kNoScene)
			return;
	} else {
		// Nancy 9: bool placement semantics, no per-drop flag setting.
		uint16 count = 0;
		for (uint i = 0; i < _numPieces; ++i) {
			if (_pieces[i]->isPlaced && _pieces[i]->counterByte == 0)
				++count;
		}
		count += (uint16)(_pieces.size() - _numPieces);

		if (count < _requiredPieces)
			return;

		for (uint i = 0; i < _numPieces; ++i) {
			if (_pieces[i]->isPlaced && _pieces[i]->compare == kDoNotPlace)
				return;
			if (!_pieces[i]->isPlaced && _pieces[i]->mustPlace)
				return;
		}
	}

	_isSolved = true;
	g_nancy->_sound->playSound(_solveSound);
	// A CONVO key that resolves to an empty string means audio-only; otherwise fall
	// back to the raw caption when the key isn't in CONVO.
	showSubtitle(resolveSubtitleText(_solveTextKey, _solveText, "CONVO"));
	_solveState = kWaitSolveSound;
}

void MultiBuildPuzzle::renderAnimFrame() {
	if (!_animSurfaceReady || _animImage.empty())
		return;

	_animRender._drawSurface.clear(g_nancy->_graphics->getTransColor());
	_animRender._drawSurface.blitFrom(_animImage, _animSrcRect, Common::Point(0, 0));
	_animRender.setNeedsRedraw(true);
}

void MultiBuildPuzzle::clearAnimFrame() {
	if (!_animSurfaceReady)
		return;
	_animRender._drawSurface.clear(g_nancy->_graphics->getTransColor());
	_animRender.setNeedsRedraw(true);
}

void MultiBuildPuzzle::updatePieceRender(int pieceIdx) {
	Piece &p = *_pieces[pieceIdx];
	bool isSelected = (!_isDragging && pieceIdx == _selectedPiece);
	bool isDragging  = (_isDragging  && pieceIdx == _pickedUpPiece);

	// Nancy 10+: a piece with no source rect is baked into the scene overlay
	// at rest (cake mixing) and must stay hidden. Placed pieces also hide when
	// a completion animation will cover them. Pieces that do carry a source
	// rect (plant potting) render normally at rest.
	if (g_nancy->getGameType() >= kGameTypeNancy10 && !isDragging && !isSelected) {
		bool bakedAtRest = !p.isPlaced && p.srcRect.isEmpty();
		if (bakedAtRest || (p.isPlaced && _hasAnimImage)) {
			p.setVisible(false);
			p.moveTo(p.gameRect);
			return;
		}
	}

	p.setVisible(true);

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
	} else if (isSelected && (_hasCloseupImage || !p.image.empty()) && !p.cuSrcRect.isEmpty()) {
		// Zoomed closeup, drawn from the piece's own image when it has one.
		const Graphics::ManagedSurface &source = p.image.empty() ? _closeupImage : p.image;
		int w = p.cuSrcRect.width();
		int h = p.cuSrcRect.height();
		p._drawSurface.create(w, h, source.format);
		p._drawSurface.setTransparentColor(source.getTransparentColor());
		p._drawSurface.blitFrom(source, p.cuSrcRect, Common::Point(0, 0));
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
