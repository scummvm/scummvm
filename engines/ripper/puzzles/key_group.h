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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef RIPPER_PUZZLES_KEY_GROUP_H
#define RIPPER_PUZZLES_KEY_GROUP_H

#include "common/rect.h"

#include "ripper/display.h"
#include "ripper/resources.h"

namespace Ripper {

class RipperEngine;

class KeyGroupPuzzle {
public:
	enum Result {
		kExited,
		kSolved,
		kLoadFailed
	};

	explicit KeyGroupPuzzle(RipperEngine *engine);

	Result run(uint completionFlag);

private:
	enum SlotBank {
		kNoSlotBank,
		kSourceSlotBank,
		kTargetSlotBank
	};

	struct SlotHit {
		SlotBank bank;
		int index;

		SlotHit() : bank(kNoSlotBank), index(-1) {}
		SlotHit(SlotBank slotBank, int slotIndex) :
			bank(slotBank), index(slotIndex) {}
	};

	bool loadAssets();
	bool loadBackground();
	bool loadPiece(uint piece);
	void loadPersistentState();
	void storePersistentState() const;
	void applyPalette() const;
	void drawFrame(byte *screen, uint pitch, const BitmapAssetFrame &frame,
		int x, int y) const;
	bool render() const;
	Common::Point slotOrigin(SlotBank bank, uint index) const;
	SlotHit findSlot(const Common::Point &point) const;
	int &slotValue(const SlotHit &slot);
	int slotValue(const SlotHit &slot) const;
	bool pickUp(const SlotHit &slot, const Common::Point &point);
	bool putDown(const SlotHit &slot);
	void updateHeldPosition(const Common::Point &point);
	void updateCursor(const Common::Point &point);
	bool isSolved() const;
	bool complete(uint completionFlag, const char *source);
	bool serviceKeyword(uint16 command);
	bool setNavigationFlags(bool previous, bool next, bool cancelled);
	Common::String targetStateString() const;

	RipperEngine *_engine;
	BitmapAssetFrame _background;
	BitmapAssetFrame _pieces[20];
	IndexedDisplaySnapshot _incomingDisplay;
	int _sourceSlots[20];
	int _targetSlots[14];
	uint _maxPieceWidth;
	uint _maxPieceHeight;
	int _heldPiece;
	Common::Point _heldPosition;
	Common::Point _heldOffset;
	SlotHit _hoveredSlot;
	uint _keywordIndex;
};

} // End of namespace Ripper

#endif // RIPPER_PUZZLES_KEY_GROUP_H
