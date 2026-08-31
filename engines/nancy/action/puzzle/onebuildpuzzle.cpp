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

// Does `inner` sit inside `outer`, allowing `tolerance` of overhang per side?
static bool rectFitsIn(const Common::Rect &inner, const Common::Rect &outer, int16 tolerance) {
	return inner.left >= outer.left - tolerance && inner.top >= outer.top - tolerance &&
			inner.right <= outer.right + tolerance && inner.bottom <= outer.bottom + tolerance;
}

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

	// A Nancy 12 puzzle can put more pieces on screen than it describes: every
	// piece past the described ones is a copy of a randomly picked description,
	// and always starts scattered (see scatterPiece()).
	if (!_pieces.empty() && _pieces.size() < _totalPieces) {
		uint numDescribed = _pieces.size();
		_pieces.resize(_totalPieces);

		for (uint i = numDescribed; i < _totalPieces; ++i) {
			Piece &p = _pieces[i];
			const Piece &copied = _pieces[g_nancy->_randomSource->getRandomNumber(numDescribed - 1)];
			p.srcRect = copied.srcRect;
			p.altSrcRect = copied.altSrcRect;
			p.slotRect = copied.slotRect;
		}
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

		// Rotations 1-3: only needed if pieces can rotate, or if this one doesn't
		// start upright. Pre-placed pieces never rotate and stay at rotation 0.
		if ((_canRotateAll || p.defaultRotation != 0) && !p.isPreRotated) {
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
			if (g_nancy->getGameType() >= kGameTypeNancy12 && p.homeRect.top == p.homeRect.bottom) {
				scatterPiece(p);
			} else {
				p.gameRect = p.homeRect;

				// A piece that already starts inside its own slot, at the
				// rotation the slot calls for, counts as placed from the outset.
				if (g_nancy->getGameType() >= kGameTypeNancy13 &&
						p.defaultRotation == p.requiredRotation &&
						rectFitsIn(p.homeRect, p.slotRect, _slotTolerance))
					p.placed = true;
			}
		}

		p.setVisible(true);
		p.setTransparent(true);
		p.setZ(_z + (uint16)i + 1);
		updatePieceRender(i);
	}

	if (_countMode != kCountAllPieces)
		updateCounter();

	_closeupDisplay.setVisible(false);

	_isInitialized = true;
}

void OneBuildPuzzle::registerGraphics() {
	if (!_isInitialized)
		return;

	for (uint i = 0; i < _pieces.size(); ++i)
		_pieces[i].registerGraphics();

	if (_hasFinalAnim)
		_finalAnimOverlay.registerGraphics();

	if (_countMode != kCountAllPieces)
		_counterDisplay.registerGraphics();

	if (g_nancy->getGameType() >= kGameTypeNancy13)
		_closeupDisplay.registerGraphics();
}

// Nancy12 (AR 166) reworked OneBuildPuzzle onto the shared PuzzleBase loader:
// a fixed 513-byte header blob, followed by six variable-length "random sound"
// blocks, then a variable-count array of the same 66-byte piece records used by
// the older games. The piece array is no longer a fixed 20-slot block.
// Nancy13 shrinks the blob to 470 bytes (the exit hotspot and cancel scene move
// into the shared hotspot block that follows), appends a seventh sound block for
// the close-up, and grows the piece records to 99 bytes.
void OneBuildPuzzle::readDataNancy12(Common::SeekableReadStream &stream) {
	const bool isNancy13 = g_nancy->getGameType() >= kGameTypeNancy13;

	// --- PuzzleBase header blob (513 bytes; 470 in Nancy13) ---
	readFilename(stream, _imageName);       // 0x00
	_freePlacement = stream.readByte();     // 0x21
	_canRotateAll = stream.readByte();      // 0x22
	stream.skip(5);                         // 0x23: rotation/grab-zone config
	_dropMode = (DropMode)stream.readByte(); // 0x28
	_slotTolerance = stream.readSint16LE(); // 0x29

	// The plain drop mode ignores the stored tolerance and demands an exact fit.
	if (_dropMode == kDropNormal)
		_slotTolerance = 0;

	// Only Nancy13 allows extra slop on top of the tolerance.
	_dropSlop = isNancy13 ? kDropSlop : 0;

	_placementMode = (PlacementMode)stream.readByte(); // 0x2b
	_countMode = (CountMode)stream.readByte();         // 0x2c
	stream.skip(1);                                    // 0x2d: percentage flag

	for (uint i = 0; i < kNumDigits; ++i)   // 0x2e: counter digit sprites
		readRect(stream, _digitSrcRects[i]);

	_counterPos.x = (int16)stream.readSint32LE(); // 0xce
	_counterPos.y = (int16)stream.readSint32LE(); // 0xd2
	_counterSpacing = stream.readSint16LE();      // 0xd6

	stream.skip(0xe8 - 0xd8);                   // 0xd8: final-animation centering rect
	_requiredPieces = stream.readSint16LE();    // 0xe8

	// 0xea: home-scatter zone. Pieces whose stored home rect is empty are
	// scattered to a random spot inside this rect at init (see scatterPiece()).
	readRect(stream, _scatterZone);         // 0xea..0xf9

	readRect(stream, _placementZone);       // 0xfa: a piece may only be released in here

	if (!isNancy13)
		readRect(stream, _exitHotspot);     // 0x10a

	_pieceCursorType = stream.readSint16LE();     // 0x11a (0x10a in Nancy13)
	_heldPieceCursorType = stream.readSint16LE(); // 0x11c (0x10c in Nancy13)

	if (!isNancy13)
		stream.skip(2);                     // 0x11e: exit cursor, always _puzzleExitCursor

	readFilename(stream, _extraSoundName);  // 0x120: final-animation atlas image
	readRect(stream, _animRectA);           // 0x141
	readRect(stream, _animRectB);           // 0x151
	for (uint i = 0; i < 6; ++i)            // 0x161
		_animLayout[i] = stream.readSint16LE();
	_animSound1.readNormal(stream);         // 0x16d
	_animSound2.readNormal(stream);         // 0x19e
	_hasFinalAnim = !_animRectA.isEmpty();
	_hasCrank = !_animRectB.isEmpty();

	_solveScene.readData(stream);           // 0x1cf (0x1bd in Nancy13, where it ends the blob)

	if (isNancy13) {
		// Shared hotspot records; the first is the give-up hotspot, which replaces
		// the header's cancel scene.
		int16 numZones = stream.readSint16LE();
		for (int16 i = 0; i < numZones; ++i) {
			Common::Rect zone;
			readRect(stream, zone);
			uint16 cursorType = stream.readUint16LE();
			uint16 sceneID = stream.readUint16LE();
			int16 flagLabel = stream.readSint16LE();
			byte flagValue = stream.readByte();

			if (i == 0) {
				_exitHotspot = zone;
				_exitCursorType = cursorType;
				_cancelScene._sceneChange.sceneID = sceneID;
				// The field after the scene id is an event-flag label, not a frame.
				_cancelScene._sceneChange.frameID = 0;
				_cancelScene._sceneChange.continueSceneSound = kContinueSceneSound;
				_cancelScene._flag.label = flagLabel;
				_cancelScene._flag.flag = flagValue;
			}
		}
	} else {
		_cancelScene.readData(stream);      // 0x1e8 (ends the 513-byte blob)
	}

	// --- Random-sound blocks: pickup, rotate, drop, good, bad, completion, close-up ---
	const uint numSoundBlocks = isNancy13 ? kNumSoundsNancy13 : kNumSounds;
	RandomSoundBlock blocks[kNumSoundsNancy13];
	for (uint i = 0; i < numSoundBlocks; ++i)
		blocks[i].readData(stream);

	SoundDescription *sounds[kNumSoundsNancy13] = { &_pickupSound, &_rotateSound, &_dropSound,
													&_goodPlacementSound, &_badPlacementSound,
													&_completionSound, &_closeupSound };
	for (uint i = 0; i < numSoundBlocks; ++i) {
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
	_totalPieces = stream.readUint16LE();
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

		if (isNancy13) {
			// Close-up view: the source region, and where it is drawn.
			readRect(stream, p.closeupSrcRect);
			readRect(stream, p.closeupDestRect);
			p.hasCloseupSound = stream.readByte() != 0;
		}

		p.defaultRotation = stream.readByte();
		p.requiredRotation = stream.readByte();
		p.isPreRotated = p.requiredRotation == kPrePlacedRotation;
	}

	// Optional placement-order arrays, each present only when its flag is set.
	_orderedPlacement = stream.readByte() != 0;
	if (_orderedPlacement) {
		_placementOrder.resize(_numPieces);
		for (uint i = 0; i < _numPieces; ++i)
			_placementOrder[i] = stream.readSint16LE();
	}

	if (stream.readByte() != 0) {
		_preplacedZOrder.resize(_numPieces);
		for (uint i = 0; i < _numPieces; ++i)
			_preplacedZOrder[i] = stream.readSint16LE();
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
	_totalPieces = _numPieces;
	_freePlacement = stream.readByte();
	_canRotateAll = stream.readByte();
	stream.skip(6); // rotationMode, zoneHeight, zoneWidth, mouse-clamping flag
	_slotTolerance = stream.readSint16LE();

	_orderedPlacement = stream.readByte() != 0;

	_placementOrder.resize(20);
	for (uint i = 0; i < 20; ++i)
		_placementOrder[i] = stream.readSint16LE();

	if (isNancy10) {
		stream.readByte(); // Set when the puzzle stacks its pre-placed pieces
		_preplacedZOrder.resize(20);
		for (uint i = 0; i < 20; ++i)
			_preplacedZOrder[i] = stream.readSint16LE();
	}

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
		// Up to Nancy 11 this byte is a plain pre-placed flag, and a piece only
		// ever fits its slot upright; requiredRotation stays at 0.
		p.isPreRotated = stream.readByte() != 0;
	}

	if (isNancy10) {
		// The 32-byte post-piece block holds two rects. The first is a
		// bad-placement check region (unused here); the second is the region
		// forks may be dragged onto and released in.
		stream.skip(16);
		readRect(stream, _placementZone);
		readFilename(stream, _extraSoundName);
		readRect(stream, _animRectA);
		readRect(stream, _animRectB);
		for (uint i = 0; i < 6; ++i)
			_animLayout[i] = stream.readSint16LE();
		_animSound1.readNormal(stream);
		_animSound2.readNormal(stream);
		_hasFinalAnim = !_animRectA.isEmpty();
		_hasCrank = !_animRectB.isEmpty();
	}

	_pickupSound.readNormal(stream);
	_rotateSound.readNormal(stream);
	_dropSound.readNormal(stream);
	readFilename(stream, _dropAlt1Filename);
	readFilename(stream, _dropAlt2Filename);

	_goodPlacementSound.readNormal(stream);
	readFilename(stream, _goodAlt1Filename);
	readFilename(stream, _goodAlt2Filename);
	readPlacementTexts(stream, _goodTexts);

	_badPlacementSound.readNormal(stream);
	readFilename(stream, _badAlt1Filename);
	readFilename(stream, _badAlt2Filename);
	readPlacementTexts(stream, _badTexts);

	// Piece hover/drag cursor, then exit cursor (handled via _puzzleExitCursor).
	_pieceCursorType = stream.readSint16LE();
	stream.skip(2);
	_solveScene.readData(stream);
	_completionSound.readNormal(stream);

	// Completion caption. Only an AUTOTEXT key produces a textbox caption; the
	// trailing inline string is a sound subtitle (e.g. "High pitched sound" for
	// the tuning-fork puzzle), which the original never writes to the textbox.
	// It is still read to keep the stream aligned.
	Common::String completionKey;
	char textBuf[200];
	readFilename(stream, completionKey);
	stream.read(textBuf, 200);
	_completionText = resolveSubtitleText(completionKey);

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
		if (g_nancy->getGameType() >= kGameTypeNancy13)
			g_nancy->_sound->loadSound(_closeupSound);
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
					// Crank puzzles never solve by placement alone; the player
					// must turn the crank to finish (see finishCrankTurn()).
					if (!_hasCrank)
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
			showSubtitle(_completionText);
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

	if (_isDragging) {
		// Always update drag position while carrying a piece
		updateDragPosition(mouseVP);

		// The held fork shows the hotspot hand cursor while over the placement
		// region, and the plain magnifying glass everywhere else.
		if (_placementZone.isEmpty() || _placementZone.contains(mouseVP))
			setPieceCursor(true);
		else
			g_nancy->_cursor->setCursorType(CursorManager::kNormal);

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
			// A fork can only be released inside the contraption region; a
			// click outside it is ignored and the piece stays on the cursor.
			if (!_placementZone.isEmpty() && !_placementZone.contains(mouseVP))
				return;

			Piece &piece = _pieces[_pickedUpPiece];

			// Swap mode needs a target: released over empty space the click is
			// ignored and the piece stays on the cursor. The target is chosen
			// before the drop is judged, so a correct placement displaces the
			// occupant too.
			int16 target = -1;
			int16 displaced = -1;
			bool targetIsPiece = false;
			if (_dropMode == kDropSwap) {
				target = findDropTarget(piece.gameRect, targetIsPiece);
				if (target == -1)
					return;

				if (targetIsPiece)
					displaced = target;
			}

			Common::Rect slot = piece.slotRect;

			// Bounding-box must fit within slot +- tolerance.
			bool nearSlot = rectFitsIn(piece.gameRect, slot, _slotTolerance + _dropSlop);

			// A piece only fits at the orientation its slot calls for; a
			// 180-degree flip keeps the same bounding box, so proximity alone
			// would accept an upside-down piece.
			bool rotationOk = (piece.curRotation == piece.requiredRotation);

			bool orderOk = !_orderedPlacement ||
				(_piecesPlaced < (uint16)_placementOrder.size() &&
				 _placementOrder[_piecesPlaced] == (int16)(_pickedUpPiece + 1));

			if (nearSlot && rotationOk && orderOk) {
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

				bool restorePosition = true;

				// The piece takes over the target's spot, aligned by its left and
				// bottom edges; the occupant is handed to the cursor in exchange.
				if (_dropMode == kDropSwap && !_freePlacement) {
					const Common::Rect &anchor = targetIsPiece ? _pieces[target].gameRect
															   : _pieces[target].slotRect;
					piece.gameRect.left = anchor.left;
					piece.gameRect.bottom = anchor.bottom;
					piece.gameRect.right = piece.gameRect.left + _pickedUpWidth;
					piece.gameRect.top = piece.gameRect.bottom - _pickedUpHeight;
					restorePosition = false;
				} else if (_placementMode == kPlacementCounter || _dropMode == kDropAnySlot) {
					int16 slotIdx = findSlotAt(piece.gameRect);

					if (slotIdx != -1) {
						restorePosition = false;

						if (_placementMode == kPlacementCounter) {
							// A slot swallows whatever is dropped into it, so a wrong
							// one costs a mistake; too many and the puzzle is lost.
							piece.placed = true;
							++_mistakes;

							if (_mistakes > _totalPieces - _requiredPieces) {
								_isCancelled = true;
								_state = kActionTrigger;
							}
						} else {
							// Otherwise it snaps into the slot it landed in, without
							// counting as placed.
							piece.gameRect = _pieces[slotIdx].slotRect;
						}
					}
				}

				if (restorePosition) {
					if (!_freePlacement) {
						piece.gameRect = _prevDragGameRect;
					} else {
						piece.curRotation = piece.defaultRotation;
						piece.gameRect = piece.homeRect;
					}
				}
			}

			if (displaced != -1) {
				// The occupant has been pushed out of its spot.
				Piece &other = _pieces[displaced];
				if (other.placed && _placementMode == kPlacementNormal) {
					other.placed = false;
					if (_piecesPlaced > 0)
						--_piecesPlaced;
				}
			}

			updateCounter();

			// Re-arm at-home art when the piece lands back on homeRect
			if (!piece.altSurface.empty() && !piece.placed &&
					piece.gameRect == piece.homeRect) {
				piece.useAltSurface = true;
			}

			updatePieceRender(_pickedUpPiece);
			playDropSound();

			if (displaced != -1) {
				// Handed to the cursor; the drop sound above covers the swap.
				pickUpPiece(displaced, false);
			} else {
				_isDragging = false;
				_pickedUpPiece = -1;
			}
		}
		return;
	}

	// Crank hotspot: on puzzles solved by a crank it can be turned at any time
	// before the puzzle is solved. Turning it plays the winding animation, then
	// either solves the puzzle or (if the forks aren't all correctly placed)
	// makes a bad noise so the player can try again. See finishCrankTurn().
	if (_hasCrank && _solveState == kIdle && _animRectB.contains(mouseVP)) {
		g_nancy->_cursor->setCursorType(CursorManager::kPuzzleArrow);
		if (input.input & NancyInput::kLeftMouseButtonUp)
			startFinalAnimation();
		return;
	}

	// A close-up swallows the whole viewport: a click anywhere dismisses it and
	// picks up the piece that opened it, not whatever lies under the cursor.
	if (_closeupPiece != -1) {
		setPieceCursor();

		if (_solveState != kIdle)
			return;

		if (input.input & (NancyInput::kLeftMouseButtonUp | NancyInput::kRightMouseButtonUp)) {
			int16 pieceIdx = _closeupPiece;
			bool rightClick = (input.input & NancyInput::kRightMouseButtonUp);
			playRotateSoundAndStartTimer();
			closeCloseup();
			pickUpPiece(pieceIdx, rightClick);
		}
		return;
	}

	// Not dragging: find the topmost piece under the cursor. The hover cursor is
	// refreshed even while a drop/placement sound plays (non-idle) so a piece put
	// down off-target keeps the piece cursor; only clicks are gated on kIdle.
	int16 topmostUnplaced = -1;
	int16 topmostAny = -1;

	for (uint i = 0; i < _pieces.size(); ++i) {
		Piece &p = _pieces[i];
		if (!p.gameRect.contains(mouseVP))
			continue;

		// A piece that has dropped into its slot in counter mode is gone for
		// good, so it doesn't react to the cursor any more. Everywhere else a
		// placed piece can still be picked back up.
		if (p.placed && _placementMode == kPlacementCounter)
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
			setPieceCursor();

		// Clicks are only processed when idle
		if (_solveState != kIdle)
			return;

		// Left click on an unplaced piece: pick it up
		// Right click: pick it up and rotate it
		bool leftClick = (input.input & NancyInput::kLeftMouseButtonUp);
		bool rightClick = (input.input & NancyInput::kRightMouseButtonUp);
		if ((leftClick || rightClick) && topmostUnplaced != -1) {
			playRotateSoundAndStartTimer();

			// A piece with a close-up opens it instead of being picked up.
			if (!_pieces[topmostUnplaced].closeupDestRect.isEmpty())
				openCloseup(topmostUnplaced);
			else
				pickUpPiece(topmostUnplaced, rightClick);
		}
		return;
	}

	// Nothing else is interactive while a drop/placement sound plays
	if (_solveState != kIdle)
		return;

	// Check exit hotspot
	Common::Rect exitScreen = NancySceneState.getViewport().convertViewportToScreen(_exitHotspot);
	if (exitScreen.contains(input.mousePos)) {
		if (_exitCursorType != 0)
			g_nancy->_cursor->setCursorType((CursorManager::CursorType)_exitCursorType, true, false);
		else
			g_nancy->_cursor->setCursorType(g_nancy->_cursor->_puzzleExitCursor);
		if (input.input & NancyInput::kLeftMouseButtonUp) {
			_isCancelled = true;
			_state = kActionTrigger;
		}
	}
}

// --- Internal helpers ---

void OneBuildPuzzle::readPlacementTexts(Common::SeekableReadStream &stream, Common::Array<Common::String> &out) {
	Common::String keys[3];
	for (uint i = 0; i < 3; ++i)
		readFilename(stream, keys[i]);

	char textBuf[200];
	out.resize(3);
	for (uint i = 0; i < 3; ++i) {
		stream.read(textBuf, 200);
		Common::String literal;
		assembleTextLine(textBuf, literal, 200);
		out[i] = resolveSubtitleText(keys[i], literal);
	}
}

void OneBuildPuzzle::setPieceCursor(bool isHeld) {
	if (g_nancy->getGameType() >= kGameTypeNancy10) {
		// Nancy 12 carries a second cursor for a piece that's on the cursor;
		// the older games use the same one for hovering and carrying.
		int16 cursorType = (isHeld && _heldPieceCursorType != 0) ? _heldPieceCursorType : _pieceCursorType;

		// The piece hand uses the hotspot variant (blue hand with an outline).
		g_nancy->_cursor->setCursorType((CursorManager::CursorType)cursorType, true, true);
	} else {
		g_nancy->_cursor->setCursorType(CursorManager::kCustom1);
	}
}

void OneBuildPuzzle::pickUpPiece(int16 pieceIdx, bool rotate) {
	_pickedUpPiece = pieceIdx;

	Piece &pp = _pieces[_pickedUpPiece];
	pp.useAltSurface = false;

	if (rotate)
		rotatePiece(_pickedUpPiece);

	_isDragging = true;
	_pickedUpWidth  = pp.rotateSurfaces[pp.curRotation].w;
	_pickedUpHeight = pp.rotateSurfaces[pp.curRotation].h;
	pp.setZ((uint16)(_z + (int)_pieces.size() * 2));
	pp.registerGraphics();
}

// Pieces at their current positions first, at the drop test's slop, then the
// slots at the plain tolerance. The carried piece never matches itself.
int16 OneBuildPuzzle::findDropTarget(const Common::Rect &dropRect, bool &isPiece) const {
	for (uint i = 0; i < _pieces.size(); ++i) {
		if ((int16)i == _pickedUpPiece)
			continue;

		if (rectFitsIn(dropRect, _pieces[i].gameRect, _slotTolerance + _dropSlop)) {
			isPiece = true;
			return (int16)i;
		}
	}

	for (uint i = 0; i < _pieces.size(); ++i) {
		if (_pieces[i].slotRect.isEmpty())
			continue;

		if (rectFitsIn(dropRect, _pieces[i].slotRect, _slotTolerance)) {
			isPiece = false;
			return (int16)i;
		}
	}

	isPiece = false;
	return -1;
}

void OneBuildPuzzle::openCloseup(int16 pieceIdx) {
	const Piece &p = _pieces[pieceIdx];

	_closeupDisplay._drawSurface.create(_image, p.closeupSrcRect);
	_closeupDisplay.setTransparent(false);

	Common::Rect dest = p.closeupDestRect;
	const VIEW *viewData = GetEngineData(VIEW);
	if (viewData)
		dest.translate(viewData->screenPosition.left, viewData->screenPosition.top);

	_closeupDisplay.moveTo(dest);
	_closeupDisplay.setVisible(true);
	_closeupPiece = pieceIdx;

	// Flagged pieces also get a spoken remark, keyed by the sound name.
	if (p.hasCloseupSound) {
		g_nancy->_sound->playSound(_closeupSound);

		Common::String text = resolveSubtitleText(_closeupSound.name, Common::String(), "AUTOTEXT");
		if (text.empty())
			text = resolveSubtitleText(_closeupSound.name, Common::String(), "CONVO");
		showSubtitle(text);
	}
}

void OneBuildPuzzle::closeCloseup() {
	if (_closeupPiece == -1)
		return;

	if (_pieces[_closeupPiece].hasCloseupSound)
		NancySceneState.getTextbox().clear();

	_closeupDisplay.setVisible(false);
	_closeupPiece = -1;
}

void OneBuildPuzzle::updatePieceRender(int pieceIdx) {
	Piece &p = _pieces[pieceIdx];

	// In counter mode the slot rect is a container the piece is dropped into
	// (a drawer, in the Nancy 12 nuts and bolts puzzle) and is much larger than
	// the piece itself, so a placed piece is hidden instead of drawn in it.
	if (p.placed && _placementMode == kPlacementCounter) {
		p.setVisible(false);
		return;
	}

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

	if (p.isPreRotated || (!_canRotateAll && p.defaultRotation == 0))
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

	// The scattered spot becomes the piece's home, so a piece dropped away from
	// its slot returns there instead of to the empty rect it was loaded with.
	p.homeRect = p.gameRect;
}

int16 OneBuildPuzzle::findSlotAt(const Common::Rect &rect) const {
	for (uint i = 0; i < _pieces.size(); ++i) {
		const Common::Rect &slot = _pieces[i].slotRect;
		if (slot.isEmpty())
			continue;

		if (rectFitsIn(rect, slot, _slotTolerance))
			return (int16)i;
	}

	return -1;
}

void OneBuildPuzzle::updateCounter() {
	if (_countMode == kCountAllPieces)
		return;

	uint16 value;
	if (_countMode == kCountPlacements)
		value = _piecesPlaced;
	else if (_placementMode == kPlacementCounter)
		value = _mistakes;
	else
		value = _totalPieces - _piecesPlaced;

	Common::String digits = Common::String::format("%u", (uint)value);

	int width = 0;
	int height = 0;
	for (uint i = 0; i < digits.size(); ++i) {
		const Common::Rect &digit = _digitSrcRects[digits[i] - '0'];
		width += digit.width() + (i ? _counterSpacing : 0);
		height = MAX<int>(height, digit.height());
	}

	if (width == 0 || height == 0)
		return;

	_counterDisplay._drawSurface.create(width, height, _image.format);
	_counterDisplay.setTransparent(true);

	// Clear to the transparent color first so the gaps between the digits stay
	// see-through.
	_counterDisplay._drawSurface.clear(g_nancy->_graphics->getTransColor());

	int destX = 0;
	for (uint i = 0; i < digits.size(); ++i) {
		const Common::Rect &digit = _digitSrcRects[digits[i] - '0'];
		_counterDisplay._drawSurface.blitFrom(_image, digit, Common::Point(destX, 0));
		destX += digit.width() + _counterSpacing;
	}

	Common::Rect dest(_counterPos.x, _counterPos.y, _counterPos.x + width, _counterPos.y + height);
	const VIEW *viewData = GetEngineData(VIEW);
	if (viewData)
		dest.translate(viewData->screenPosition.left, viewData->screenPosition.top);

	_counterDisplay.moveTo(dest);
	_counterDisplay.setVisible(true);
	_counterDisplay.setNeedsRedraw(true);
}

void OneBuildPuzzle::checkAllPlaced() {
	if (_countMode != kCountAllPieces) {
		// Counter puzzles end as soon as enough pieces have gone into the right
		// slot, even when a few are still lying around.
		if (_piecesPlaced < _requiredPieces)
			return;
	} else {
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
	showSubtitle(_goodTexts[idx]);
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
	showSubtitle(_badTexts[idx]);
	_solveState = kWaitPlaceSound;
	_timerEnd = g_system->getMillis() + 1000;
}

void OneBuildPuzzle::startFinalAnimation() {
	_finalAnimDone = true;
	_animFrameCounter = 0;
	_animRowCounter = 0;

	// Without an animation image to step through, resolve the crank turn now.
	if (_animImage.w == 0) {
		if (_animSound1.name != "NO SOUND" && !_animSound1.name.empty()) {
			g_nancy->_sound->loadSound(_animSound1);
			g_nancy->_sound->playSound(_animSound1);
		}
		finishCrankTurn();
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

	// Animation finished: solve the puzzle or make the bad noise.
	finishCrankTurn();
}

void OneBuildPuzzle::finishCrankTurn() {
	_finalAnimOverlay.setVisible(false);

	// checkAllPlaced() sets _isSolved and moves to kTriggerCompletion once every
	// required fork is in place.
	checkAllPlaced();
	if (_isSolved)
		return;

	// The forks aren't all correctly placed yet: the contraption makes a bad
	// noise and the player can turn the crank again.
	if (_animSound2.name != "NO SOUND" && !_animSound2.name.empty()) {
		g_nancy->_sound->loadSound(_animSound2);
		g_nancy->_sound->playSound(_animSound2);
		_currentSound = _animSound2;
		_timerEnd = g_system->getMillis() + 800;
		_solveState = kWaitPlaceSound;
	} else {
		_solveState = kIdle;
	}
	_finalAnimDone = false;
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
