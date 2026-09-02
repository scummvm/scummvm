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
#include "engines/nancy/cursor.h"
#include "engines/nancy/graphics.h"
#include "engines/nancy/input.h"
#include "engines/nancy/resource.h"
#include "engines/nancy/sound.h"
#include "engines/nancy/util.h"

#include "engines/nancy/enginedata.h"
#include "engines/nancy/state/scene.h"

#include "engines/nancy/action/puzzle/buildpuzzle.h"

namespace Nancy {
namespace Action {

// Reads one of the puzzle's grouped sound blocks into a plain SoundDescription,
// keeping only the first of the random alternatives.
static void readSoundBlock(Common::SeekableReadStream &stream, SoundDescription &out) {
	RandomSoundBlock block;
	block.readData(stream);

	out.name = block.names.empty() ? "NO SOUND" : block.names[0];
	out.channelID = block.channel;
	out.numLoops = block.numLoops;
	out.volume = block.volume;
}

void BuildPuzzle::readData(Common::SeekableReadStream &stream) {
	readFilename(stream, _imageName);
	readFilename(stream, _altImageName);

	_trayImageMode = stream.readByte();
	stream.skip(3);							// 0x46: the difficulty gate's flag
	_requiredPlaced = stream.readUint16LE();
	_usePlacedGate = stream.readByte();
	_stateItemID = stream.readUint16LE();
	stream.skip(32);						// 0x4e: two overlay rects
	SoundDescription unused;
	readSoundBlock(stream, unused);
	stream.skip(32);						// 0x6e: two more overlay rects
	readSoundBlock(stream, unused);
	stream.skip(32);						// 0x8e: the "done" overlay gate and its rect

	readFilename(stream, _anim1Name);
	readRect(stream, _anim1Rect);
	readFilename(stream, _anim2Name);
	readRect(stream, _anim2Rect);
	_pieceCursorType = stream.readSint16LE();
	_heldPieceCursorType = stream.readSint16LE();
	stream.skip(3);							// 0xe4: a third cursor type and a flag

	uint16 numZones = stream.readUint16LE();
	_zones.resize(numZones);
	for (uint i = 0; i < numZones; ++i) {
		Zone &zone = _zones[i];
		readRect(stream, zone.hotspot);
		zone.capacity = stream.readUint16LE();
		stream.skip(1);
		zone.fill = (ZoneFill)stream.readByte();
		zone.marksPlaced = stream.readByte();

		uint16 numIngredients = stream.readUint16LE();
		zone.ingredients.resize(numIngredients);
		for (uint j = 0; j < numIngredients; ++j) {
			Ingredient &ingredient = zone.ingredients[j];
			ingredient.pieceID = stream.readSint16LE();
			ingredient.quantity = stream.readByte();
			ingredient.mode = stream.readByte();
		}
	}

	uint16 numHolds = stream.readUint16LE();
	_holds.resize(numHolds);
	for (uint i = 0; i < numHolds; ++i) {
		HoldSlot &hold = _holds[i];
		readRect(stream, hold.srcRect);
		readRect(stream, hold.destRect);
		readRect(stream, hold.fillSrcRect1);
		readRect(stream, hold.fillSrcRect2);
		hold.amount = stream.readByte();
	}

	// Only present when the puzzle has hold slots.
	if (numHolds > 0)
		readSoundBlock(stream, _holdSound);

	uint16 numPieces = stream.readUint16LE();
	_pieces.resize(numPieces);
	for (uint i = 0; i < numPieces; ++i) {
		Piece &piece = _pieces[i];
		readRect(stream, piece.srcRect);
		readRect(stream, piece.destRect);
		readRect(stream, piece.dragSrcRect);
		readRect(stream, piece.placedSrcRect);
		readRect(stream, piece.closeupSrcRect);
		readRect(stream, piece.closeupDestRect);
		readRect(stream, piece.placedDestRect);

		piece.kind = stream.readByte();
		if (piece.kind == 3)
			readFilename(stream, piece.imageName);
		else
			piece.zoneID = stream.readSint16LE();

		piece.itemID = stream.readSint16LE();

		// The list names the scoops this piece can be taken with, and the last
		// entry picks which of a scoop's two full images to show while carrying it.
		int16 numValues = stream.readSint16LE();
		if (numValues > 0) {
			piece.holds.resize(numValues - 1);
			for (int16 j = 0; j < numValues - 1; ++j) {
				piece.holds[j] = stream.readSint16LE();
			}

			piece.fillVariant = stream.readSint16LE();
		}
	}

	_counterItemID = stream.readByte();
	if (_counterItemID != 255) {
		for (uint i = 0; i < kNumDigits; ++i)
			readRect(stream, _digitSrcRects[i]);

		_counterPos.x = (int16)stream.readSint32LE();
		_counterPos.y = (int16)stream.readSint32LE();
		_counterSpacing = stream.readSint32LE();
	}

	readSoundBlock(stream, _pickupSound);
	readSoundBlock(stream, _dropSound);
	readSoundBlock(stream, _notebookSound);
	readSoundBlock(stream, _resetSound);

	_wrongIngredientFlag = stream.readSint16LE();
	_solvedFlag = stream.readSint16LE();
	_solveScene.sceneID = stream.readUint16LE();
	_solveScene.frameID = stream.readUint16LE();
	_solveScene.continueSceneSound = kContinueSceneSound;
	_solveFlag.label = stream.readSint16LE();
	_solveFlag.flag = stream.readByte();

	readSoundBlock(stream, unused);

	_failScene.sceneID = stream.readUint16LE();
	_failScene.frameID = stream.readUint16LE();
	_failScene.continueSceneSound = kContinueSceneSound;
	_failFlag.label = stream.readSint16LE();
	_failFlag.flag = stream.readByte();

	readSoundBlock(stream, unused);

	// The count-prefixed 23-byte hotspot records shared by the later puzzles.
	int16 numExitZones = stream.readSint16LE();
	for (int16 i = 0; i < numExitZones; ++i) {
		Common::Rect zone;
		readRect(stream, zone);
		uint16 cursorType = stream.readUint16LE();
		uint16 sceneID = stream.readUint16LE();
		int16 flagLabel = stream.readSint16LE();
		byte flagValue = stream.readByte();

		if (i == 0) {
			_exitHotspot = zone;
			_exitCursorType = cursorType;
			_exitScene.sceneID = sceneID;
			_exitScene.frameID = 0;
			_exitScene.continueSceneSound = kContinueSceneSound;
			_exitFlag.label = flagLabel;
			_exitFlag.flag = flagValue;
		}
	}
}

void BuildPuzzle::setFlagOnChange(int16 label, bool value, int8 &last) {
	if (label == -1 || last == (int8)value) {
		return;
	}

	last = (int8)value;
	NancySceneState.setEventFlag(label, value ? g_nancy->_true : g_nancy->_false);
}

void BuildPuzzle::setPieceCursor(bool isHeld) {
	int16 cursorType = isHeld && _heldPieceCursorType != 0 ? _heldPieceCursorType : _pieceCursorType;

	if (cursorType != 0) {
		g_nancy->_cursor->setCursorType((CursorManager::CursorType)cursorType, true, true);
	} else {
		g_nancy->_cursor->setCursorType(CursorManager::kHotspot);
	}
}

void BuildPuzzle::init() {
	const uint32 transColor = g_nancy->_graphics->getTransColor();

	g_nancy->_resource->loadImage(_imageName, _image);
	_image.setTransparentColor(transColor);

	// A puzzle without a second image draws everything from the first one.
	g_nancy->_resource->loadImage(_altImageName.empty() ? _imageName : _altImageName, _altImage);
	_altImage.setTransparentColor(transColor);

	_numDefined = _pieces.size();

	// Every piece but a kind 1 is copied when it is dropped, so the array needs
	// room for as many copies as the recipes can ask for. It is grown once, here,
	// because the pieces are render objects and must not move afterwards.
	uint numSpare = 0;
	for (uint i = 0; i < _zones.size(); ++i) {
		for (uint j = 0; j < _zones[i].ingredients.size(); ++j) {
			numSpare += _zones[i].ingredients[j].quantity;
		}
	}

	_pieces.resize(_numDefined + numSpare);

	for (uint i = 0; i < _zones.size(); ++i) {
		_zones[i].counts.resize(_zones[i].ingredients.size());
	}

	for (uint i = 0; i < _pieces.size(); ++i) {
		Piece &piece = _pieces[i];

		// A piece's art defaults down the chain when a rect is left empty.
		if (piece.dragSrcRect.isEmpty()) {
			piece.dragSrcRect = piece.srcRect;
		}

		if (piece.placedSrcRect.isEmpty()) {
			piece.placedSrcRect = piece.dragSrcRect;
		}

		piece.inUse = (i < _numDefined);
		piece.sourceID = (int16)i;
		piece.liveRect = piece.destRect;
		piece.setZ(_z + (uint16)i + 1);
		updatePieceRender((int16)i);
	}

	for (uint i = 0; i < _holds.size(); ++i) {
		HoldSlot &hold = _holds[i];
		if (hold.srcRect.isEmpty()) {
			continue;
		}

		hold._drawSurface.create(_altImage, hold.srcRect);
		hold.setTransparent(true);
		hold.moveTo(hold.destRect);
		hold.setZ(_z + (uint16)i + 1);
		hold.setVisible(true);
	}

	_cursorItem.setTransparent(true);
	_cursorItem.setVisible(false);

	_isInitialized = true;
}

void BuildPuzzle::registerGraphics() {
	if (!_isInitialized) {
		return;
	}

	for (uint i = 0; i < _pieces.size(); ++i) {
		_pieces[i].registerGraphics();
	}

	for (uint i = 0; i < _holds.size(); ++i) {
		_holds[i].registerGraphics();
	}

	_cursorItem.registerGraphics();
}

// The graphics manager keeps its object list sorted as objects are inserted, so
// a new z only takes effect once the piece is registered again.
byte BuildPuzzle::carriedAmount() const {
	return _activeHold != -1 ? MAX<byte>(_holds[_activeHold].amount, 1) : 1;
}

void BuildPuzzle::updateCursorItem(const Common::Point &mouseVP) {
	// A scoop stays on the cursor while an ingredient is picked up with it, so
	// the scoop's own art wins over the ingredient's.
	const Common::Rect *src = nullptr;

	if (_activeHold != -1) {
		const HoldSlot &hold = _holds[_activeHold];

		// A scoop carrying an ingredient shows itself full.
		if (_heldPiece != -1) {
			src = _pieces[_heldPiece].fillVariant == 0 ? &hold.fillSrcRect1 : &hold.fillSrcRect2;
		}

		if (!src || src->isEmpty()) {
			src = &hold.srcRect;
		}
	} else if (_heldPiece != -1) {
		src = &_pieces[_heldPiece].dragSrcRect;
	}

	if (!src || src->isEmpty()) {
		_cursorItem.setVisible(false);
		return;
	}

	int width = src->width();
	int height = src->height();
	Common::Rect dest((int16)(mouseVP.x - width / 2), (int16)(mouseVP.y - height / 2),
						(int16)(mouseVP.x - width / 2 + width), (int16)(mouseVP.y - height / 2 + height));

	_cursorItem._drawSurface.create(_altImage, *src);
	_cursorItem.setTransparent(true);
	_cursorItem.moveTo(dest);
	_cursorItem.setVisible(true);
	_cursorItem.registerGraphics();
}

void BuildPuzzle::setPieceZ(int16 pieceIdx, uint16 z) {
	_pieces[pieceIdx].setZ(z);
	_pieces[pieceIdx].registerGraphics();
}

void BuildPuzzle::updatePieceRender(int16 pieceIdx) {
	Piece &piece = _pieces[pieceIdx];

	if (!piece.inUse || piece.liveRect.isEmpty()) {
		piece.setVisible(false);
		return;
	}

	// Each of the three states has its own art, and only a piece resting at home
	// is drawn from the image the puzzle selects; the other two always come from
	// the alt one.
	// A carried piece lives on the cursor instead of on the board.
	if (pieceIdx == _heldPiece) {
		piece.setVisible(false);
		return;
	}

	bool isPlaced = piece.assignedZone != -1;

	const Common::Rect *src = &piece.srcRect;
	Graphics::ManagedSurface *surf = _trayImageMode == 1 ? &_image : &_altImage;

	if (isPlaced) {
		src = &piece.placedSrcRect;
		surf = &_altImage;
	}

	if (src->isEmpty()) {
		piece.setVisible(false);
		return;
	}

	piece._drawSurface.create(*surf, *src);
	piece.setTransparent(true);
	piece.moveTo(piece.liveRect);
	piece.setVisible(true);
}

int16 BuildPuzzle::clonePiece(int16 pieceIdx) {
	for (uint i = _numDefined; i < _pieces.size(); ++i) {
		if (_pieces[i].inUse) {
			continue;
		}

		Piece &clone = _pieces[i];
		const Piece &original = _pieces[pieceIdx];

		clone.srcRect = original.srcRect;
		clone.destRect = original.destRect;
		clone.dragSrcRect = original.dragSrcRect;
		clone.placedSrcRect = original.placedSrcRect;
		clone.closeupSrcRect = original.closeupSrcRect;
		clone.closeupDestRect = original.closeupDestRect;
		clone.placedDestRect = original.placedDestRect;
		clone.kind = original.kind;
		clone.zoneID = original.zoneID;
		clone.holds = original.holds;
		clone.fillVariant = original.fillVariant;
		clone.liveRect = original.liveRect;
		clone.sourceID = original.sourceID;
		clone.assignedZone = -1;
		clone.inUse = true;

		return (int16)i;
	}

	return -1;
}

void BuildPuzzle::adjustZone(int16 zoneIdx, int16 pieceID, int8 delta) {
	Zone &zone = _zones[zoneIdx];

	for (uint i = 0; i < zone.ingredients.size(); ++i) {
		if (zone.ingredients[i].pieceID != pieceID) {
			continue;
		}

		zone.counts[i] = (byte)(zone.counts[i] + delta);
		zone.numHeld += delta;
		return;
	}

	// Nothing in the recipe wanted this piece.
	zone.numWrong += delta;
	zone.numHeld += delta;

	int16 totalWrong = 0;
	for (uint i = 0; i < _zones.size(); ++i) {
		totalWrong += _zones[i].numWrong;
	}

	setFlagOnChange(_wrongIngredientFlag, totalWrong > 0, _lastWrongFlag);
}

bool BuildPuzzle::checkSolved() const {
	for (uint i = 0; i < _zones.size(); ++i) {
		const Zone &zone = _zones[i];

		if (zone.numWrong != 0) {
			return false;
		}

		for (uint j = 0; j < zone.ingredients.size(); ++j) {
			const Ingredient &ingredient = zone.ingredients[j];

			if (ingredient.mode == 2) {
				if (zone.counts[j] < ingredient.quantity) {
					return false;
				}
			} else if (ingredient.mode == 0) {
				if (zone.counts[j] != ingredient.quantity) {
					return false;
				}
			} else {
				return false;
			}
		}
	}

	return true;
}

void BuildPuzzle::openCloseup(int16 pieceIdx) {
	_closeupPiece = pieceIdx;

	Piece &piece = _pieces[pieceIdx];
	Common::Rect dest = piece.closeupDestRect;

	// An empty destination means the close-up is centered in the viewport.
	if (dest.isEmpty()) {
		const VIEW *viewData = GetEngineData(VIEW);
		if (viewData) {
			int width = piece.closeupSrcRect.width();
			int height = piece.closeupSrcRect.height();
			int left = (viewData->screenPosition.width() - width) / 2;
			int top = (viewData->screenPosition.height() - height) / 2;
			dest = Common::Rect((int16)left, (int16)top, (int16)(left + width), (int16)(top + height));
		}
	}

	// A piece carrying its own filename keeps its close-up art in that image.
	Graphics::ManagedSurface *surf = _trayImageMode == 1 ? &_image : &_altImage;
	if (!piece.imageName.empty()) {
		if (_pieceImageName != Common::Path(piece.imageName)) {
			_pieceImageName = Common::Path(piece.imageName);
			g_nancy->_resource->loadImage(_pieceImageName, _pieceImage);
			_pieceImage.setTransparentColor(g_nancy->_graphics->getTransColor());
		}

		surf = &_pieceImage;
	}

	piece._drawSurface.create(*surf, piece.closeupSrcRect);
	piece.setTransparent(true);
	piece.moveTo(dest);
	piece.setVisible(true);
	setPieceZ(pieceIdx, (uint16)(_z + _pieces.size() + 2));
}

void BuildPuzzle::closeCloseup() {
	if (_closeupPiece == -1) {
		return;
	}

	int16 pieceIdx = _closeupPiece;
	_closeupPiece = -1;
	setPieceZ(pieceIdx, (uint16)(_z + pieceIdx + 1));
	updatePieceRender(pieceIdx);
}

void BuildPuzzle::pickUpPiece(int16 pieceIdx) {
	Piece &piece = _pieces[pieceIdx];

	// Taking a piece back out of a zone undoes its contribution. A copy only
	// exists while it is in a zone, so it goes away rather than onto the cursor.
	if (piece.assignedZone != -1) {
		adjustZone(piece.assignedZone, piece.sourceID, -1);
		piece.assignedZone = -1;

		if (pieceIdx >= (int16)_numDefined) {
			piece.inUse = false;
			piece.setVisible(false);
			return;
		}
	}

	_heldPiece = pieceIdx;
	setPieceZ(pieceIdx, (uint16)(_z + _pieces.size() + 1));

	g_nancy->_sound->loadSound(_pickupSound);
	g_nancy->_sound->playSound(_pickupSound);
	updatePieceRender(pieceIdx);
}

void BuildPuzzle::returnPiece(int16 pieceIdx) {
	Piece &piece = _pieces[pieceIdx];
	piece.liveRect = piece.destRect;
	piece.assignedZone = -1;
	_heldPiece = -1;
	setPieceZ(pieceIdx, (uint16)(_z + pieceIdx + 1));
	updatePieceRender(pieceIdx);
}

void BuildPuzzle::placePiece(int16 pieceIdx, int16 zoneIdx, const Common::Point &dropPos) {
	Zone &zone = _zones[zoneIdx];
	int16 placedIdx = pieceIdx;

	// A zone with a fill mode shows what went into it, so it needs something to
	// keep: the piece itself when its kind is consumed, otherwise a copy, which
	// leaves the original on the shelf to be used again.
	if (zone.fill != kFillAbsorb) {
		if (_pieces[pieceIdx].kind != kConsumedKind) {
			placedIdx = clonePiece(pieceIdx);
			returnPiece(pieceIdx);

			if (placedIdx == -1) {
				return;
			}
		}

		_pieces[placedIdx].assignedZone = zoneIdx;
	}

	Piece &piece = _pieces[placedIdx];
	int width = piece.placedSrcRect.width();
	int height = piece.placedSrcRect.height();

	if (!piece.placedDestRect.isEmpty()) {
		// The piece names its own spot, whatever the zone would have done.
		piece.liveRect = piece.placedDestRect;
	} else {
		switch (zone.fill) {
		case kFillCentered: {
			int left = dropPos.x - width / 2;
			int top = dropPos.y - height / 2;
			piece.liveRect = Common::Rect((int16)left, (int16)top,
											(int16)(left + width), (int16)(top + height));
			break;
		}
		case kFillTopLeft:
			// Left aligned to the zone, sitting on its bottom edge.
			piece.liveRect = Common::Rect(zone.hotspot.left, (int16)(zone.hotspot.bottom - height),
											(int16)(zone.hotspot.left + width), zone.hotspot.bottom);
			break;
		default:
			// Absorbed: a consumed piece is gone, anything else goes back home.
			piece.liveRect = piece.kind == kConsumedKind ? Common::Rect() : piece.destRect;
			break;
		}
	}

	adjustZone(zoneIdx, piece.sourceID, (int8)carriedAmount());

	g_nancy->_sound->loadSound(_dropSound);
	g_nancy->_sound->playSound(_dropSound);

	_heldPiece = -1;
	setPieceZ(placedIdx, (uint16)(_z + placedIdx + 1));
	updatePieceRender(placedIdx);

	bool solved = checkSolved();
	setFlagOnChange(_solvedFlag, solved, _lastSolvedFlag);

	if (solved) {
		_isSolved = true;
		_state = kActionTrigger;
	}
}

void BuildPuzzle::execute() {
	switch (_state) {
	case kBegin:
		init();
		registerGraphics();
		_state = kRun;
		break;
	case kRun:
		break;
	case kActionTrigger:
		if (_isSolved) {
			NancySceneState.setEventFlag(_solveFlag);
			NancySceneState.changeScene(_solveScene);
		} else {
			NancySceneState.setEventFlag(_exitFlag);
			NancySceneState.changeScene(_exitScene);
		}

		finishExecution();
		break;
	}
}

void BuildPuzzle::handleInput(NancyInput &input) {
	if (_state != kRun || _isSolved) {
		return;
	}

	const VIEW *viewData = GetEngineData(VIEW);
	if (!viewData || !viewData->screenPosition.contains(input.mousePos)) {
		return;
	}

	Common::Point mouseVP(input.mousePos.x - viewData->screenPosition.left,
							input.mousePos.y - viewData->screenPosition.top);
	bool clicked = (input.input & NancyInput::kLeftMouseButtonUp) != 0;

	updateCursorItem(mouseVP);

	// A close-up covers the board; clicking it takes the piece, except for a
	// piece that is only ever there to be looked at.
	if (_closeupPiece != -1) {
		setPieceCursor(false);

		if (clicked) {
			int16 pieceIdx = _closeupPiece;
			closeCloseup();

			if (_pieces[pieceIdx].kind != 3) {
				pickUpPiece(pieceIdx);
			}
		}

		return;
	}

	// Carrying an ingredient: release it over a zone, or anywhere else to send
	// it home.
	if (_heldPiece != -1) {
		setPieceCursor(true);

		if (clicked) {
			// Stacked zones overlap, so the release only says whether the drop is
			// over them at all; which one it lands in is the first with room, so a
			// glass fills from the bottom rather than leaving a gap under a layer.
			int16 target = -1;
			int16 over = -1;

			for (uint i = 0; i < _zones.size(); ++i) {
				const Piece &held = _pieces[_heldPiece];

				// A piece can be restricted to a single zone.
				if (held.zoneID != -1 && held.zoneID != (int16)i) {
					continue;
				}

				if (over == -1 && _zones[i].hotspot.contains(mouseVP)) {
					over = (int16)i;
				}

				if (target == -1 && (_zones[i].capacity == 0 || _zones[i].numHeld < _zones[i].capacity)) {
					target = (int16)i;
				}
			}

			if (over == -1) {
				target = -1;
			} else if (target == -1) {
				target = over;
			}

			if (target != -1) {
				placePiece(_heldPiece, target, mouseVP);

				// The scoop is emptied by the drop and goes back to its place.
				if (_activeHold != -1) {
					_holds[_activeHold].setVisible(true);
					_holds[_activeHold].registerGraphics();
					_activeHold = -1;
				}
			} else {
				returnPiece(_heldPiece);
			}

			updateCursorItem(mouseVP);
		}

		return;
	}

	// Topmost piece under the cursor.
	int16 hovered = -1;
	for (uint i = 0; i < _pieces.size(); ++i) {
		const Piece &piece = _pieces[i];
		if (!piece.inUse || !piece.liveRect.contains(mouseVP)) {
			continue;
		}

		if (hovered == -1 || piece.getZOrder() > _pieces[hovered].getZOrder()) {
			hovered = (int16)i;
		}
	}

	if (hovered != -1) {
		const Piece &piece = _pieces[hovered];

		// An ingredient that has no art of its own is scooped rather than carried,
		// and a piece that names its scoops can only be taken with one of those.
		bool needsScoop = piece.dragSrcRect.isEmpty() || !piece.holds.empty();
		bool scoopFits = _activeHold != -1 &&
							(piece.holds.empty() ||
							Common::find(piece.holds.begin(), piece.holds.end(), _activeHold) != piece.holds.end());

		if (!piece.closeupSrcRect.isEmpty() || !needsScoop || scoopFits) {
			setPieceCursor(false);

			if (clicked) {
				if (!piece.closeupSrcRect.isEmpty()) {
					openCloseup(hovered);
				} else {
					pickUpPiece(hovered);
					updateCursorItem(mouseVP);
				}
			}

			return;
		}
	}

	// The scoops themselves: one click takes it, another puts it back.
	for (uint i = 0; i < _holds.size(); ++i) {
		if (!_holds[i].destRect.contains(mouseVP)) {
			continue;
		}

		setPieceCursor(false);

		if (clicked) {
			if (_activeHold == (int16)i) {
				_holds[i].setVisible(true);
				_holds[i].registerGraphics();
				_activeHold = -1;
			} else {
				if (_activeHold != -1) {
					_holds[_activeHold].setVisible(true);
					_holds[_activeHold].registerGraphics();
				}

				_activeHold = (int16)i;
				_holds[i].setVisible(false);
			}

			g_nancy->_sound->loadSound(_pickupSound);
			g_nancy->_sound->playSound(_pickupSound);
			updateCursorItem(mouseVP);
		}

		return;
	}

	if (_exitHotspot.isEmpty()) {
		return;
	}

	if (NancySceneState.getViewport().convertViewportToScreen(_exitHotspot).contains(input.mousePos)) {
		if (_exitCursorType != 0) {
			g_nancy->_cursor->setCursorType((CursorManager::CursorType)_exitCursorType, true, true);
		} else {
			g_nancy->_cursor->setCursorType(g_nancy->_cursor->_puzzleExitCursor);
		}

		if (clicked) {
			_state = kActionTrigger;
		}
	}
}

} // End of namespace Action
} // End of namespace Nancy
