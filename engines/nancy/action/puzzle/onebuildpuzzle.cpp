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

	// Post-placement animation atlas (e.g. music-box handle "GHO_SlnMBoxHandle_OVL"
	// in scene 3637). Loaded only when the puzzle defines _animRectA;
	// positioning of the overlay is deferred to startFinalAnimation() so the
	// viewport offset is known.
	if (_hasFinalAnim && !_extraSoundName.empty() && _extraSoundName != "NO_FILE") {
		g_nancy->_resource->loadImage(Common::Path(_extraSoundName), _animImage);

		// Use the engine's canonical transparent color so blitFrom and the
		// renderer agree on which pixels are see-through. (The puzzle's own
		// _drawSurface is never explicitly made transparent in init().)
		const uint32 transColor = g_nancy->_graphics->getTransColor();
		_animImage.setTransparentColor(transColor);

		const int w = _animRectA.width();
		const int h = _animRectA.height();
		_finalAnimOverlay._drawSurface.create(w, h, _animImage.format);
		_finalAnimOverlay._drawSurface.setTransparentColor(transColor);
		_finalAnimOverlay._drawSurface.clear(transColor);
		_finalAnimOverlay.setTransparent(true);
		_finalAnimOverlay.setVisible(false);
	}

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

		p.useAltSurface = false;
		if (!p.altSrcRect.isEmpty() && !p.isPreRotated) {
			int aw = p.altSrcRect.width();
			int ah = p.altSrcRect.height();
			p.altSurface.create(aw, ah, _image.format);
			p.altSurface.setTransparentColor(_drawSurface.getTransparentColor());
			p.altSurface.blitFrom(_image, p.altSrcRect, Common::Point(0, 0));
			p.useAltSurface = true;
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
			p.placed = false;

			// Nancy12 puzzles may ship pieces with an empty home rect
			// (top == bottom), which means "start scattered": the original
			// init picks a random spot inside the home-scatter zone. Without
			// this, such pieces get a zero-height rect and are invisible.
			if (g_nancy->getGameType() >= kGameTypeNancy12 && p.homeRect.top == p.homeRect.bottom)
				scatterPiece(p);
			else
				p.gameRect = p.homeRect;
		}

		updatePieceRender(i);
		p.setVisible(true);
		p.setTransparent(true);
		p.setZ(_z + (uint16)i + 1);
	}

	_isInitialized = true;
}

void OneBuildPuzzle::registerGraphics() {
	if (!_isInitialized)
		return;

	for (uint i = 0; i < _pieces.size(); ++i)
		_pieces[i].registerGraphics();

	if (_hasFinalAnim)
		_finalAnimOverlay.registerGraphics();
}

// Nancy12 (AR 166) reworked OneBuildPuzzle onto the shared PuzzleBase loader:
// a fixed 513-byte header blob, followed by six variable-length "random sound"
// blocks, then a variable-count array of the same 66-byte piece records used by
// the older games. The piece array is no longer a fixed 20-slot block.
void OneBuildPuzzle::readDataNancy12(Common::SeekableReadStream &stream) {
	// --- PuzzleBase header blob (513 bytes) ---
	readFilename(stream, _imageName);       // 0x00
	_freePlacement = stream.readByte();     // 0x21
	_canRotateAll = stream.readByte();      // 0x22
	stream.skip(6);                         // 0x23: rotation/zone config + placement-mode byte
	_slotTolerance = stream.readSint16LE(); // 0x29

	// 0x2b..0xe9: placement-mode byte, final-animation centering rect and filler
	// count. None are needed by this port.
	stream.skip(0xea - 0x2b);

	// 0xea: home-scatter zone. Pieces whose stored home rect is empty are
	// scattered to a random spot inside this rect at init (see scatterPiece()).
	readRect(stream, _scatterZone);         // 0xea..0xf9

	// 0xfa..0x11f: misc config, unused by this port.
	stream.skip(0x120 - 0xfa);

	readFilename(stream, _extraSoundName);  // 0x120: final-animation atlas image
	readRect(stream, _animRectA);           // 0x141
	readRect(stream, _animRectB);           // 0x151
	for (uint i = 0; i < 6; ++i)            // 0x161
		_animLayout[i] = stream.readSint16LE();
	_animSound1.readNormal(stream);         // 0x16d
	_animSound2.readNormal(stream);         // 0x19e
	_hasFinalAnim = !_animRectA.isEmpty();

	_solveScene.readData(stream);           // 0x1cf
	_cancelScene.readData(stream);          // 0x1e8 (ends the 513-byte blob)

	// --- Random-sound blocks: pickup, rotate, drop, good, bad, completion ---
	RandomSoundBlock blocks[kNumSounds];
	for (uint i = 0; i < kNumSounds; ++i)
		blocks[i].readData(stream);

	SoundDescription *sounds[kNumSounds] = { &_pickupSound, &_rotateSound, &_dropSound,
											 &_goodPlacementSound, &_badPlacementSound, &_completionSound };
	for (uint i = 0; i < kNumSounds; ++i) {
		SoundDescription &s = *sounds[i];
		s.name = blocks[i].names.empty() ? "NO SOUND" : blocks[i].names[0];
		s.channelID = blocks[i].channel;
		s.numLoops = blocks[i].numLoops;
		s.volume = blocks[i].volume;
	}

	// The drop/good/bad sounds pick randomly between their alternatives, which are
	// now the extra entries of the corresponding block.
	Common::String *dropAlts[2] = { &_dropAlt1Filename, &_dropAlt2Filename };
	Common::String *goodAlts[2] = { &_goodAlt1Filename, &_goodAlt2Filename };
	Common::String *badAlts[2]  = { &_badAlt1Filename,  &_badAlt2Filename };
	Common::String **altSets[3] = { dropAlts, goodAlts, badAlts };
	const uint altBlocks[3] = { kDropSound, kGoodSound, kBadSound };
	for (uint i = 0; i < 3; ++i) {
		for (uint a = 0; a < 2; ++a) {
			const RandomSoundBlock &b = blocks[altBlocks[i]];
			*altSets[i][a] = (b.names.size() > a + 1) ? b.names[a + 1] : "NO SOUND";
		}
	}

	// Nancy12 puzzles no longer carry inline good/bad/completion text.
	_goodTexts.resize(3);
	_badTexts.resize(3);

	// --- Piece array (variable count) ---
	stream.readSint16LE(); // Secondary piece count (matches numPieces in practice)
	_numPieces = stream.readUint16LE();

	_pieces.resize(_numPieces);
	for (uint i = 0; i < _numPieces; ++i) {
		Piece &p = _pieces[i];

		// Two source rects: altSrc = at-home art, srcRect = active art.
		Common::Rect altSrc;
		readRect(stream, altSrc);
		readRect(stream, p.srcRect);
		if (p.srcRect.isEmpty())
			p.srcRect = altSrc;
		else if (!altSrc.isEmpty())
			p.altSrcRect = altSrc;

		readRect(stream, p.slotRect);
		readRect(stream, p.homeRect);
		p.defaultRotation = stream.readByte();
		// A piece is pre-placed only when this marker is exactly 10.
		p.isPreRotated = stream.readByte() == 10;
	}

	// Optional placement-order arrays, each present only when its flag is set.
	_orderedPlacement = stream.readByte() != 0;
	if (_orderedPlacement) {
		_placementOrder.resize(_numPieces);
		for (uint i = 0; i < _numPieces; ++i)
			_placementOrder[i] = stream.readSint16LE();
	}

	if (stream.readByte() != 0) {
		_legacyPlacementOrder.resize(_numPieces);
		for (uint i = 0; i < _numPieces; ++i)
			_legacyPlacementOrder[i] = stream.readSint16LE();
	}
}

void OneBuildPuzzle::readData(Common::SeekableReadStream &stream) {
	if (g_nancy->getGameType() >= kGameTypeNancy12) {
		readDataNancy12(stream);
		return;
	}

	const bool isNancy10 = g_nancy->getGameType() >= kGameTypeNancy10;

	readFilename(stream, _imageName);

	_numPieces = stream.readUint16LE();
	_freePlacement = stream.readByte();
	_canRotateAll = stream.readByte();
	stream.skip(6); // rotationMode, zoneHeight, zoneWidth, mouse-clamping flag
	_slotTolerance = stream.readSint16LE();

	if (isNancy10) {
		// TODO: purpose of this duplicate placement-order block is unknown.
		_legacyOrderedFlag = stream.readByte() != 0;
		_legacyPlacementOrder.resize(20);
		for (uint i = 0; i < 20; ++i)
			_legacyPlacementOrder[i] = stream.readSint16LE();
	}

	_orderedPlacement = stream.readByte();

	_placementOrder.resize(20);
	for (uint i = 0; i < 20; ++i)
		_placementOrder[i] = stream.readSint16LE();

	// Nancy 10 piece records add an alternative source rect at the front.
	const uint pieceSize = isNancy10 ? 66 : 50;

	_pieces.resize(_numPieces);
	for (uint i = 0; i < 20; ++i) {
		if (i >= _numPieces) {
			stream.skip(pieceSize);
			continue;
		}

		Piece &p = _pieces[i];
		if (isNancy10) {
			// Two rects: altSrc = at-home art, srcRect = active art
			Common::Rect altSrc;
			readRect(stream, altSrc);
			readRect(stream, p.srcRect);
			if (p.srcRect.isEmpty()) {
				p.srcRect = altSrc;
			} else if (!altSrc.isEmpty()) {
				p.altSrcRect = altSrc;
			}
		} else {
			readRect(stream, p.srcRect);
		}
		readRect(stream, p.slotRect);
		readRect(stream, p.homeRect);
		p.defaultRotation = stream.readByte();
		p.isPreRotated = stream.readByte();
	}

	if (isNancy10) {
		stream.skip(32); // TODO: 32 post-piece bytes, layout undecoded.
		readFilename(stream, _extraSoundName);
		readRect(stream, _animRectA);
		readRect(stream, _animRectB);
		for (uint i = 0; i < 6; ++i)
			_animLayout[i] = stream.readSint16LE();
		_animSound1.readNormal(stream);
		_animSound2.readNormal(stream);
		_hasFinalAnim = !_animRectA.isEmpty();
	}

	_pickupSound.readNormal(stream);
	_rotateSound.readNormal(stream);
	_dropSound.readNormal(stream);
	readFilename(stream, _dropAlt1Filename);
	readFilename(stream, _dropAlt2Filename);

	_goodPlacementSound.readNormal(stream);
	readFilename(stream, _goodAlt1Filename);
	readFilename(stream, _goodAlt2Filename);

	_goodTexts.resize(3);

	const CVTX *autotext = (const CVTX *)g_nancy->getEngineData("AUTOTEXT");
	assert(autotext);

	Common::String unusedKey;
	char textBuf[200];

	for (uint i = 0; i < 3; ++i) {
		stream.read(textBuf, 200);
		assembleTextLine(textBuf, _goodTexts[i], 200);
		if (!_goodTexts[i].empty() && autotext->texts.contains(_goodTexts[i]))
			_goodTexts[i] = autotext->texts[_goodTexts[i]];
	}
	for (uint i = 0; i < 3; ++i)
		readFilename(stream, unusedKey);

	_badPlacementSound.readNormal(stream);
	readFilename(stream, _badAlt1Filename);
	readFilename(stream, _badAlt2Filename);

	_badTexts.resize(3);
	for (uint i = 0; i < 3; ++i) {
		stream.read(textBuf, 200);
		assembleTextLine(textBuf, _badTexts[i], 200);
		if (!_badTexts[i].empty() && autotext->texts.contains(_badTexts[i]))
			_badTexts[i] = autotext->texts[_badTexts[i]];
	}
	for (uint i = 0; i < 3; ++i)
		readFilename(stream, unusedKey);

	stream.skip(4); // TODO: 4 bytes before solveScene, unknown.
	_solveScene.readData(stream);
	_completionSound.readNormal(stream);
	readFilename(stream, unusedKey);
	stream.read(textBuf, 200);
	assembleTextLine(textBuf, _completionText, 200);
	if (!_completionText.empty() && autotext->texts.contains(_completionText))
		_completionText = autotext->texts[_completionText];

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
					checkAllPlaced();
					if (!_isSolved)
						playGoodPlacementSound();
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
			g_nancy->_sound->loadSound(_completionSound);
			g_nancy->_sound->playSound(_completionSound);
			if (!_completionText.empty()) {
				NancySceneState.getTextbox().clear();
				NancySceneState.getTextbox().addTextLine(_completionText);
			}
			_solveState = kWaitCompletion;
			break;
		case kAnimateFinal:
			// 100ms per frame, matches original case 3/4 tick rate
			if (g_system->getMillis() >= _timerEnd)
				stepFinalAnimation();
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

	// Post-placement final-animation stage: once all pieces are placed on a
	// puzzle that defines _animRectA, the puzzle waits here until the user
	// clicks the hotspot (e.g. winding a music-box crank, throwing a lever).
	if (_waitingForFinalAnim && _solveState == kIdle) {
		if (_animRectA.contains(mouseVP)) {
			g_nancy->_cursor->setCursorType(CursorManager::kPuzzleArrow);
			if (input.input & NancyInput::kLeftMouseButtonUp)
				startFinalAnimation();
			return;
		}
		// Fall through so the exit hotspot still works while waiting.
	}

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

			// Bounding-box must fit within slot +- tolerance. The original
			// engine doesn't check rotation separately; a rotated piece's
			// dimensions are reflected in gameRect, so a non-fitting rotation
			// is rejected by the rect inequalities below.
			bool nearSlot = (piece.gameRect.left >= slot.left - _slotTolerance &&
							 piece.gameRect.top  >= slot.top  - _slotTolerance &&
							 piece.gameRect.right  <= slot.right  + _slotTolerance &&
							 piece.gameRect.bottom <= slot.bottom + _slotTolerance);

			bool orderOk = !_orderedPlacement ||
				(_piecesPlaced < (uint16)_placementOrder.size() &&
				 _placementOrder[_piecesPlaced] == (int16)(_pickedUpPiece + 1));

			if (nearSlot && orderOk) {
				piece.gameRect = piece.slotRect;
				piece.placed = true;
				_correctlyPlaced = true;
				++_piecesPlaced;

				// Skip pre-placed pieces
				if (_piecesPlaced < _placementOrder.size() && (uint)(_placementOrder[_piecesPlaced] - 1) < _pieces.size())
					if (_pieces[_placementOrder[_piecesPlaced] - 1].isPreRotated)
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

			// Re-arm at-home art when the piece lands back on homeRect
			if (!piece.altSurface.empty() && !piece.placed &&
					piece.gameRect == piece.homeRect) {
				piece.useAltSurface = true;
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
		if (topmostUnplaced != -1)
			g_nancy->_cursor->setCursorType(CursorManager::kCustom1);

		// Left click on an unplaced piece: pick it up
		// Right click: pick it up and rotate it
		bool leftClick = (input.input & NancyInput::kLeftMouseButtonUp);
		bool rightClick = (input.input & NancyInput::kRightMouseButtonUp);
		if ((leftClick || rightClick) && topmostUnplaced != -1) {
			_pickedUpPiece = topmostUnplaced;

			Piece &pp = _pieces[_pickedUpPiece];
			pp.useAltSurface = false;

			if (rightClick)
				rotatePiece(_pickedUpPiece);

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
	if (p.useAltSurface && !p.altSurface.empty()) {
		p._drawSurface.create(p.altSurface, p.altSurface.getBounds());
	} else {
		int rot = p.curRotation;
		if (!p.hasSurface[rot])
			rot = 0;
		if (!p.hasSurface[rot])
			return;
		p._drawSurface.create(p.rotateSurfaces[rot], p.rotateSurfaces[rot].getBounds());
	}
	p.setTransparent(true);
	p.moveTo(p.gameRect);
}

void OneBuildPuzzle::rotatePiece(int pieceIdx) {
	Piece &p = _pieces[pieceIdx];

	if (!_canRotateAll && !p.isPreRotated)
		return;

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

void OneBuildPuzzle::scatterPiece(Piece &p) {
	// Piece display size at its starting rotation. Fall back to rotation 0 when
	// the rotated surface wasn't generated (non-rotatable pieces).
	int rot = p.hasSurface[p.curRotation] ? p.curRotation : 0;
	int w = p.rotateSurfaces[rot].w;
	int h = p.rotateSurfaces[rot].h;

	// The scatter zone comes from the puzzle data; if it's degenerate the
	// original engine falls back to the full viewport (as kBegin does).
	Common::Rect zone = _scatterZone;
	if (zone.isEmpty()) {
		const VIEW *viewData = GetEngineData(VIEW);
		if (viewData)
			zone = Common::Rect(viewData->screenPosition.width(), viewData->screenPosition.height());
	}

	int maxLeft = MAX<int>(zone.left, zone.right - w);
	int maxTop  = MAX<int>(zone.top, zone.bottom - h);
	int left = zone.left + (int)g_nancy->_randomSource->getRandomNumber(MAX(0, maxLeft - zone.left));
	int top  = zone.top  + (int)g_nancy->_randomSource->getRandomNumber(MAX(0, maxTop - zone.top));

	p.gameRect = Common::Rect((int16)left, (int16)top, (int16)(left + w), (int16)(top + h));
}

void OneBuildPuzzle::checkAllPlaced() {
	for (uint i = 0; i < _pieces.size(); ++i) {
		if (_pieces[i].placed)
			continue;

		// Nancy 10: pieces with an empty slotRect (top == 0 && bottom == 0)
		// are filler — they don't need to be placed for the puzzle to solve.
		const Common::Rect &slot = _pieces[i].slotRect;
		if (slot.top == 0 && slot.bottom == 0)
			continue;

		return;
	}

	// Puzzles with a post-placement animation (e.g. scene 3637's music-box
	// crank) require the player to click _animRectA before the puzzle solves.
	if (_hasFinalAnim && !_finalAnimDone) {
		_waitingForFinalAnim = true;
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

void OneBuildPuzzle::startFinalAnimation() {
	_finalAnimDone = true;       // one-shot guard
	_waitingForFinalAnim = false;
	_animFrameCounter = 0;
	_animRowCounter = 0;

	// Without an animation image to step through, fall straight into completion.
	if (_animImage.w == 0) {
		if (_animSound1.name != "NO SOUND" && !_animSound1.name.empty()) {
			g_nancy->_sound->loadSound(_animSound1);
			g_nancy->_sound->playSound(_animSound1);
		}
		_isSolved = true;
		_solveState = kTriggerCompletion;
		return;
	}

	// Position the overlay at _animRectA, translated into screen coords.
	const VIEW *viewData = GetEngineData(VIEW);
	Common::Rect dst = _animRectA;
	if (viewData)
		dst.translate(viewData->screenPosition.left, viewData->screenPosition.top);
	_finalAnimOverlay.moveTo(dst);
	_finalAnimOverlay.setVisible(true);

	if (_animSound1.name != "NO SOUND" && !_animSound1.name.empty()) {
		g_nancy->_sound->loadSound(_animSound1);
		g_nancy->_sound->playSound(_animSound1);
	}

	_solveState = kAnimateFinal;
	_timerEnd = g_system->getMillis();   // fire immediately on first tick
}

void OneBuildPuzzle::stepFinalAnimation() {
	// animLayout = {cols, framesPerStep, baseX, baseY, spacing, totalRows}.
	// Counter wraps to the next row when (counter / framesPerStep) >= cols.
	const int16 cols          = _animLayout[0];
	const int16 framesPerStep = _animLayout[1] ? _animLayout[1] : 1;
	const int16 baseX         = _animLayout[2];
	const int16 baseY         = _animLayout[3];
	const int16 spacing       = _animLayout[4];
	const int16 totalRows     = _animLayout[5];

	if (_animFrameCounter / framesPerStep >= cols) {
		_animFrameCounter = 0;
		++_animRowCounter;
	}

	if (_animRowCounter < totalRows) {
		// Source rect on the atlas. Original engine uses inclusive width/height
		// (right_raw - left_raw), so width()-1 / height()-1 in our convention.
		const int cellW = _animRectA.width()  - 1;
		const int cellH = _animRectA.height() - 1;
		const int srcLeft = baseX + (cellW + spacing) * (_animFrameCounter % framesPerStep);
		const int srcTop  = baseY + (cellH + spacing) * (_animFrameCounter / framesPerStep);
		Common::Rect src(srcLeft, srcTop, srcLeft + _animRectA.width(), srcTop + _animRectA.height());

		// Clear to transparent first so any pixels not covered by the source
		// (or skipped by source-transparency) stay see-through, not garbage.
		_finalAnimOverlay._drawSurface.clear(g_nancy->_graphics->getTransColor());
		_finalAnimOverlay._drawSurface.blitFrom(_animImage, src, Common::Point(0, 0));
		_finalAnimOverlay.setVisible(true);
		_finalAnimOverlay.setNeedsRedraw(true);

		++_animFrameCounter;
		_timerEnd = g_system->getMillis() + 100;
		return;
	}

	// Animation finished: hide overlay and run the standard completion flow.
	_finalAnimOverlay.setVisible(false);
	_isSolved = true;
	_solveState = kTriggerCompletion;
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
