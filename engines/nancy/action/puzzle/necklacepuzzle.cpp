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
#include "common/util.h"

#include "graphics/font.h"

#include "engines/nancy/nancy.h"
#include "engines/nancy/graphics.h"
#include "engines/nancy/resource.h"
#include "engines/nancy/sound.h"
#include "engines/nancy/input.h"
#include "engines/nancy/cursor.h"
#include "engines/nancy/util.h"
#include "engines/nancy/font.h"
#include "engines/nancy/puzzledata.h"

#include "engines/nancy/state/scene.h"
#include "engines/nancy/action/puzzle/necklacepuzzle.h"

namespace Nancy {
namespace Action {

NecklacePuzzle::~NecklacePuzzle() {
	refundBeads();
}

void NecklacePuzzle::readData(Common::SeekableReadStream &stream) {
	_refundOnExit = stream.readByte();
	_overlapPlaced = stream.readSint32LE();
	_overlapFalling = stream.readSint32LE();
	_dropFlag.label = stream.readSint16LE();
	_dropFlag.flag = stream.readByte();
	_heldCursorType = stream.readUint16LE();
	_hoverCursorType = stream.readUint16LE();
	_heldOffsetX = stream.readSint32LE();
	_heldOffsetY = stream.readSint32LE();
	_countFontID = stream.readUint16LE();
	_unknownA0 = stream.readUint16LE();
	_countTextExtraY = stream.readUint16LE();
	_countTextX = stream.readSint32LE();
	_countTextY = stream.readSint32LE();

	uint16 numTypes = stream.readUint16LE();
	_beadTypes.resize(numTypes);
	for (uint16 i = 0; i < numTypes; ++i) {
		BeadType &type = _beadTypes[i];
		type.key = stream.readSint32LE();
		type.overlaps = stream.readByte();

		Common::Path names[3];
		for (uint j = 0; j < 3; ++j) {
			readFilename(stream, names[j]);
		}

		// The three sheets repeat across kinds, so each distinct one is loaded once.
		int *slots[3] = { &type.cursorImage, &type.strandImage, &type.boxImage };
		for (uint j = 0; j < 3; ++j) {
			uint k = 0;
			for (; k < _imageNames.size(); ++k) {
				if (_imageNames[k] == names[j]) {
					break;
				}
			}

			if (k == _imageNames.size()) {
				_imageNames.push_back(names[j]);
			}

			*slots[j] = (int)k;
		}
	}

	uint16 numBeads = stream.readUint16LE();
	_beads.resize(numBeads);
	for (uint16 i = 0; i < numBeads; ++i) {
		Bead &bead = _beads[i];
		bead.type = stream.readByte();
		readRect(stream, bead.cursorSrc);
		readRect(stream, bead.strandSrc);
		readRect(stream, bead.boxSrc);
		readRect(stream, bead.boxDest);
		bead.tableIndex = stream.readSint16LE();
	}

	uint16 numStrands = stream.readUint16LE();
	_strands.resize(numStrands);
	for (uint16 i = 0; i < numStrands; ++i) {
		Strand &strand = _strands[i];
		strand.grabByTopBead = stream.readByte();
		strand.allowSwap = stream.readByte();

		uint16 numTypesAllowed = stream.readUint16LE();
		strand.allowedTypes.resize(numTypesAllowed);
		for (uint16 j = 0; j < numTypesAllowed; ++j) {
			strand.allowedTypes[j] = stream.readByte();
		}

		uint16 numSolution = stream.readUint16LE();
		strand.solution.resize(numSolution);
		for (uint16 j = 0; j < numSolution; ++j) {
			strand.solution[j] = stream.readUint16LE();
		}

		readRect(stream, strand.needleHotspot);
		readRect(stream, strand.strandRect);

		strand.dropSeconds = stream.readFloatLE();
	}

	_pickUpSound.readData(stream);
	_putDownSound.readData(stream);
	_unknownSound.readData(stream);

	_solveScene._sceneChange.sceneID = stream.readUint16LE();
	_solveScene._sceneChange.frameID = stream.readUint16LE();
	int16 solvedOffset = stream.readSint16LE();
	_solveScene._sceneChange.verticalOffset = solvedOffset >= 0 ? solvedOffset : 0;
	_solveScene._sceneChange.continueSceneSound = stream.readByte();
	_solvedSound.readData(stream);

	_unsolvedScene.sceneID = stream.readUint16LE();
	_unsolvedScene.frameID = stream.readUint16LE();
	int16 unsolvedOffset = stream.readSint16LE();
	_unsolvedScene.verticalOffset = unsolvedOffset >= 0 ? unsolvedOffset : 0;
	_unsolvedScene.continueSceneSound = stream.readByte();
	_unsolvedSound.readData(stream);

	readExitHotspot(stream);
}

void NecklacePuzzle::init() {
	initViewportSurface();

	_images.resize(_imageNames.size());
	for (uint i = 0; i < _imageNames.size(); ++i) {
		g_nancy->_resource->loadImage(_imageNames[i], _images[i]);
		_images[i].setTransparentColor(_drawSurface.getTransparentColor());
	}

	NancySceneState.setNoHeldItem();

	redraw();
}

void NecklacePuzzle::registerGraphics() {
	RenderObject::registerGraphics();
	_heldBeadObject.registerGraphics();
}

int NecklacePuzzle::findBeadType(byte key) const {
	for (uint i = 0; i < _beadTypes.size(); ++i) {
		if (_beadTypes[i].key == (int32)key) {
			return (int)i;
		}
	}

	return -1;
}

bool NecklacePuzzle::strandAccepts(const Strand &strand, byte type) const {
	for (uint i = 0; i < strand.allowedTypes.size(); ++i) {
		if (strand.allowedTypes[i] == type) {
			return true;
		}
	}

	return false;
}

int NecklacePuzzle::stackTop(const Strand &strand) const {
	if (strand.placed.empty()) {
		return strand.strandRect.bottom;
	}

	return strand.placed.back().rect.top;
}

int NecklacePuzzle::stackTopBelow(const Strand &strand) const {
	if (strand.placed.size() < 2) {
		return strand.strandRect.bottom;
	}

	return strand.placed[strand.placed.size() - 2].rect.top;
}

int16 NecklacePuzzle::beadCount(const Bead &bead) const {
	if (bead.tableIndex == kNoTableIndex) {
		return 0;
	}

	TableData *table = (TableData *)NancySceneState.getPuzzleData(TableData::getTag());
	if (!table) {
		return 0;
	}

	int16 value = table->getValue(bead.tableIndex);
	return value == kNoTableValue ? 0 : value;
}

void NecklacePuzzle::adjustBeadCount(const Bead &bead, int16 delta) {
	if (bead.tableIndex == kNoTableIndex) {
		return;
	}

	TableData *table = (TableData *)NancySceneState.getPuzzleData(TableData::getTag());
	if (!table) {
		return;
	}

	int16 value = table->getValue(bead.tableIndex);
	if (value == kNoTableValue) {
		value = 0;
	}

	table->setValue(bead.tableIndex, value + delta);
}

NecklacePuzzle::Hover NecklacePuzzle::hitTest(const Common::Point &mousePos) const {
	Hover hover;
	const bool holding = _heldBead != kNoBead;

	for (uint i = 0; i < _strands.size(); ++i) {
		const Strand &strand = _strands[i];

		if (holding && strand.needleHotspot.contains(mousePos)) {
			const Bead &held = _beads[_heldBead];
			const int extent = held.strandSrc.height() - _overlapFalling;

			if (strandAccepts(strand, held.type)) {
				if (stackTop(strand) - extent > strand.strandRect.top) {
					hover.strand = (int)i;
					return hover;
				}

				// The strand is full, but it can take this shell in place of the one
				// on top, which goes back on the cursor.
				if (strand.allowSwap && stackTopBelow(strand) - extent > strand.strandRect.top) {
					hover.strand = (int)i;
					hover.swapTop = true;
					return hover;
				}
			}

			continue;
		}

		if (holding || strand.placed.empty()) {
			continue;
		}

		// Some strands hand the top shell back when the needle is clicked, the rest
		// when the shell itself is.
		const Common::Rect &grabRect = strand.grabByTopBead ? strand.placed.back().rect : strand.needleHotspot;
		if (grabRect.contains(mousePos)) {
			hover.strand = (int)i;
			return hover;
		}
	}

	for (uint i = 0; i < _beads.size(); ++i) {
		if (_beads[i].boxDest.contains(mousePos) && beadCount(_beads[i]) > 0) {
			hover.bead = (int)i;
			return hover;
		}
	}

	return hover;
}

void NecklacePuzzle::holdBead(int16 bead, NancyInput *input) {
	_heldBead = bead;

	if (_heldBead == kNoBead) {
		_heldBeadObject.setVisible(false);
		_heldBeadObject.putDown();
		return;
	}

	const Bead &held = _beads[_heldBead];
	const int typeIndex = findBeadType(held.type);
	const int image = typeIndex >= 0 ? _beadTypes[typeIndex].cursorImage : -1;

	if (image >= 0 && _images[image].getBounds().contains(held.cursorSrc)) {
		_heldBeadObject._drawSurface.create(_images[image], held.cursorSrc);
		_heldBeadObject.setTransparent(true);
		_heldBeadObject.setVisible(true);
		_heldBeadObject.pickUp();
		if (input) {
			_heldBeadObject.handleInput(*input);
		}
	} else {
		_heldBeadObject.setVisible(false);
		_heldBeadObject.putDown();
	}
}

// Takes the shell off the cursor without putting it anywhere.
void NecklacePuzzle::releaseBead() {
	playSoundBlock(_putDownSound);
	holdBead(kNoBead, nullptr);
}

void NecklacePuzzle::dropBeadOn(int strandIndex) {
	Strand &strand = _strands[strandIndex];
	const Bead &held = _beads[_heldBead];
	const int typeIndex = findBeadType(held.type);
	const bool overlaps = typeIndex >= 0 && _beadTypes[typeIndex].overlaps;

	// The shell always starts at the top of the strand, centered across it.
	int centeredWidth = held.strandSrc.width() - (overlaps ? _overlapPlaced : 0);
	PlacedBead placed;
	placed.bead = _heldBead;
	placed.rect.left = strand.strandRect.left + (strand.strandRect.width() - centeredWidth) / 2;
	placed.rect.top = strand.strandRect.top;
	placed.rect.right = placed.rect.left + held.strandSrc.width();
	placed.rect.bottom = placed.rect.top + held.strandSrc.height();

	// Sliding the full length of the strand takes dropSeconds; this one only has to
	// cover the gap down to whatever it lands on.
	const int restOn = stackTop(strand);
	const int fullDrop = strand.strandRect.height();
	uint32 duration = 0;
	if (fullDrop > 0) {
		duration = (uint32)(strand.dropSeconds * (float)(restOn - strand.strandRect.top) * 1000.0f / (float)fullDrop);
	}

	strand.placed.push_back(placed);

	_fallingStrand = strandIndex;
	_fallingStartY = placed.rect.top;
	_fallingEndY = restOn - held.strandSrc.height() + (overlaps ? _overlapFalling : 0);
	_fallStartTime = g_nancy->getTotalPlayTime();
	_fallDuration = MAX<uint32>(duration, 1);

	NancySceneState.setEventFlag(_dropFlag);
}

void NecklacePuzzle::checkSolved() {
	for (uint i = 0; i < _strands.size(); ++i) {
		const Strand &strand = _strands[i];
		if (strand.placed.size() != strand.solution.size()) {
			return;
		}

		// The strand is filled bottom-up, so the design reads from the last shell back.
		for (uint j = 0; j < strand.solution.size(); ++j) {
			const PlacedBead &placed = strand.placed[strand.placed.size() - 1 - j];
			if (_beads[placed.bead].tableIndex != (int16)strand.solution[j]) {
				return;
			}
		}
	}

	_solved = true;
}

// Everything on the strands, and anything on the cursor, goes back in the box when
// the puzzle is left unfinished.
void NecklacePuzzle::refundBeads() {
	if (!_refundOnExit || _solved || !g_nancy) {
		return;
	}

	for (uint i = 0; i < _strands.size(); ++i) {
		Strand &strand = _strands[i];
		for (uint j = 0; j < strand.placed.size(); ++j) {
			adjustBeadCount(_beads[strand.placed[j].bead], 1);
		}
		strand.placed.clear();
	}

	if (_heldBead != kNoBead) {
		adjustBeadCount(_beads[_heldBead], 1);
		_heldBead = kNoBead;
	}

	_fallingStrand = -1;
}

void NecklacePuzzle::execute() {
	switch (_state) {
	case kBegin:
		init();
		registerGraphics();
		_state = kRun;
		// fall through

	case kRun:
		if (_exitRequested || _solved) {
			_state = kActionTrigger;
			break;
		}

		if (_fallingStrand >= 0) {
			Strand &strand = _strands[_fallingStrand];
			PlacedBead &falling = strand.placed.back();

			uint32 elapsed = g_nancy->getTotalPlayTime() - _fallStartTime;
			int y = _fallingEndY;
			if (elapsed < _fallDuration) {
				y = _fallingStartY + (int)((int64)(_fallingEndY - _fallingStartY) * elapsed / _fallDuration);
			}

			if (y != falling.rect.top) {
				falling.rect.moveTo(falling.rect.left, y);
				redraw();
			}

			if (elapsed >= _fallDuration) {
				_fallingStrand = -1;
				checkSolved();
				redraw();
			}
		}

		break;

	case kActionTrigger:
		refundBeads();

		if (_solved) {
			playSoundBlock(_solvedSound);
			if (_solveScene._sceneChange.sceneID != kNoScene) {
				NancySceneState.changeScene(_solveScene._sceneChange);
			}
		} else {
			playSoundBlock(_unsolvedSound);
			NancySceneState.setEventFlag(_exitScene._flag);
			NancySceneState.changeScene(_unsolvedScene.sceneID != kNoScene ? _unsolvedScene : _exitScene._sceneChange);
		}

		finishExecution();
		break;
	}
}

void NecklacePuzzle::handleInput(NancyInput &input) {
	if (_state != kRun || _solved || _exitRequested) {
		return;
	}

	_heldBeadObject.handleInput(input);

	// Nothing can be clicked while a shell is still sliding down a strand.
	if (_fallingStrand >= 0) {
		return;
	}

	Common::Rect vpScreen = NancySceneState.getViewport().getScreenPosition();
	Common::Point mouseVP = input.mousePos - Common::Point(vpScreen.left, vpScreen.top);
	const bool click = (input.input & NancyInput::kLeftMouseButtonUp) != 0;

	Hover hover = hitTest(mouseVP);

	if (_heldBead != kNoBead) {
		g_nancy->_cursor->setCursorType((CursorManager::CursorType)_heldCursorType, true,
			hover.strand >= 0 || hover.bead >= 0);
	} else if (hover.strand >= 0 || hover.bead >= 0) {
		g_nancy->_cursor->setCursorType((CursorManager::CursorType)_hoverCursorType, true);
	}

	if (click && hover.strand >= 0) {
		Strand &strand = _strands[hover.strand];

		if (_heldBead != kNoBead) {
			// A swap takes the shell that was on top and hands it straight back.
			int16 displaced = kNoBead;
			if (hover.swapTop) {
				displaced = strand.placed.back().bead;
				strand.placed.pop_back();
			}

			dropBeadOn(hover.strand);
			releaseBead();

			if (displaced != kNoBead) {
				holdBead(displaced, &input);
			}
		} else {
			// Take the top shell back off the strand.
			int16 bead = strand.placed.back().bead;
			strand.placed.pop_back();
			holdBead(bead, &input);
		}

		redraw();
		input.eatMouseInput();
		return;
	}

	if (click && hover.bead >= 0) {
		const int16 wasHeld = _heldBead;

		if (wasHeld != kNoBead) {
			adjustBeadCount(_beads[wasHeld], 1);
			releaseBead();
		}

		// Clicking the shell already on the cursor just puts it back.
		if (wasHeld != hover.bead) {
			adjustBeadCount(_beads[hover.bead], -1);
			holdBead((int16)hover.bead, &input);
			playSoundBlock(_pickUpSound);
		}

		redraw();
		input.eatMouseInput();
		return;
	}

	if (click && _heldBead != kNoBead) {
		adjustBeadCount(_beads[_heldBead], 1);
		releaseBead();
		redraw();
		input.eatMouseInput();
		return;
	}

	if (hoverExitHotspot(input)) {
		if (click) {
			_exitRequested = true;
		}
	}
}

void NecklacePuzzle::redraw() {
	_drawSurface.clear(_drawSurface.getTransparentColor());

	for (uint i = 0; i < _beads.size(); ++i) {
		const Bead &bead = _beads[i];
		const int typeIndex = findBeadType(bead.type);
		if (typeIndex < 0) {
			continue;
		}

		const int image = _beadTypes[typeIndex].boxImage;
		if (image >= 0 && !bead.boxSrc.isEmpty() && _images[image].getBounds().contains(bead.boxSrc)) {
			_drawSurface.blitFrom(_images[image], bead.boxSrc, Common::Point(bead.boxDest.left, bead.boxDest.top));
		}

		drawBeadCount(bead);
	}

	for (uint i = 0; i < _strands.size(); ++i) {
		const Strand &strand = _strands[i];
		for (uint j = 0; j < strand.placed.size(); ++j) {
			const PlacedBead &placed = strand.placed[j];
			const Bead &bead = _beads[placed.bead];
			const int typeIndex = findBeadType(bead.type);
			if (typeIndex < 0) {
				continue;
			}

			const int image = _beadTypes[typeIndex].strandImage;
			if (image >= 0 && !bead.strandSrc.isEmpty() && _images[image].getBounds().contains(bead.strandSrc)) {
				_drawSurface.blitFrom(_images[image], bead.strandSrc, Common::Point(placed.rect.left, placed.rect.top));
			}
		}
	}

	_needsRedraw = true;
}

// How many of this shell are still in the box, drawn next to its box slot.
void NecklacePuzzle::drawBeadCount(const Bead &bead) {
	if (bead.tableIndex == kNoTableIndex) {
		return;
	}

	const Graphics::Font *font = g_nancy->_graphics->getFont(_countFontID);
	if (!font) {
		font = g_nancy->_graphics->getFont(0);
	}
	if (!font) {
		return;
	}

	Common::String str = Common::String::format("%d", beadCount(bead));
	int w = font->getStringWidth(str);
	if (w <= 0) {
		return;
	}

	font->drawString(&_drawSurface, str, bead.boxDest.left + _countTextX,
		bead.boxDest.top + _countTextY + _countTextExtraY, w, 0);
}

} // End of namespace Action
} // End of namespace Nancy
