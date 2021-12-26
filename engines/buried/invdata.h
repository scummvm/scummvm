/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1995 Presto Studios, Inc.
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

#ifndef BURIED_INVDATA_H
#define BURIED_INVDATA_H

#include "common/scummsys.h"

namespace Buried {

enum {
	kItemBalconyKey = 0,
	kItemBioChipAI = 1,
	kItemBioChipBlank = 2,
	kItemBioChipCloak = 3,
	kItemBioChipEvidence = 4,
	kItemBioChipFiles = 5,
	kItemBioChipInterface = 6,
	kItemBioChipJump = 7,
	kItemBioChipTranslate = 8,
	kItemBloodyArrow = 9,
	kItemBurnedLetter = 10,
	kItemBurnedOutCore = 11,
	kItemCeramicBowl = 12,
	kItemCheeseGirl = 13,
	kItemClassicGamesCart = 14, // Unused!
	kItemCodexAtlanticus = 15,
	kItemCoilOfRope = 16,
	kItemCopperKey = 17,
	kItemCopperMedallion = 18,
	kItemCreditChip = 19, // Unused!
	kItemEnvironCart = 20,
	kItemExplosiveCharge = 21,
	kItemDriveAssembly = 22,
	kItemGeneratorCore = 23,
	kItemGenoSingleCart = 24,
	kItemGoldCoins = 25,
	kItemGrapplingHook = 26,
	kItemHammer = 27,
	kItemInteractiveSculpture = 28,
	kItemJadeBlock = 29,
	kItemLensFilter = 30,
	kItemLimestoneBlock = 31,
	kItemMayanPuzzleBox = 32,
	kItemMetalBar = 33,
	kItemObsidianBlock = 34,
	kItemPreservedHeart = 35,
	kItemRemoteControl = 36,
	kItemRichardsSword = 37,
	kItemSiegeCycle = 38,
	kItemCavernSkull = 39,
	kItemEntrySkull = 40,
	kItemSpearSkull = 41,
	kItemWaterCanEmpty = 42,
	kItemWaterCanFull = 43,
	kItemWheelAssembly = 44,
	kItemWoodenPegs = 45
};

struct InventoryElement {
	int16 itemID;
	int32 firstDragID;
	int32 dragIDCount;
};

} // End of namespace Buried

#endif
