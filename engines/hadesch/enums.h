/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * Copyright 2020 Google
 *
 */
#ifndef HADESCH_ENUMS_H
#define HADESCH_ENUMS_H

namespace Hadesch {
enum {
	kHadeschDebugGeneral = 1 << 0,
	kHadeschDebugResources = 1 << 1,
	kHadeschDebugMessagingSystem = 1 << 2,
	kHadeschDebugDialogs = 1 << 3
};

enum Gender {
	kFemale = 0,
	kMale = 1,
	// Make it 2, rather than -1, so that we can serialize it in one 1
	// byte.
	kUnknown = 2
};

enum Quest {
	    kNoQuest,
	    kCreteQuest,
	    kTroyQuest,
	    kMedusaQuest,
	    kRescuePhilQuest,
	    kEndGame,
	    kNumQuests
};

enum RoomId {
	kInvalidRoom = 0,
	kIntroRoom = 1,
	kOlympusRoom = 2,
	kWallOfFameRoom = 3,
	kSeriphosRoom = 4,
	kAthenaRoom = 5,
	kMedIsleRoom = 6,
	kMedusaPuzzle = 7,
	kArgoRoom = 8,
	kTroyRoom = 9,
	kCatacombsRoom = 10,
	kPriamRoom = 11,
	kTrojanHorsePuzzle = 12,
	kCreteRoom = 13,
	kMinosPalaceRoom = 14,
	kDaedalusRoom = 15,
	kMinotaurPuzzle = 16,
	kVolcanoRoom = 17,
	kRiverStyxRoom = 18,
	kHadesThroneRoom = 19,
	kFerrymanPuzzle = 20,
	kMonsterPuzzle = 21,
	kQuiz = 22,
	kCreditsRoom = 23,
	kOptionsRoom = 24,
	kNumRooms
};

enum StatueId {
	kBacchusStatue = 0,
	kHermesStatue = 1,
	kZeusStatue = 2,
	kPoseidonStatue = 3,
	kAresStatue = 4,
	kAphroditeStatue = 5,
	kApolloStatue = 6,
	kArtemisStatue = 7,
	kDemeterStatue = 8,
	kAthenaStatue = 9,
	kHeraStatue = 10,
	kHephaestusStatue = 11,
	kNumStatues
};

enum InventoryItem {
	kNone = 0,
	kStraw = 2,
	kStone = 3,
	kBricks = 4,
	kMessage = 5,
	kKey = 6,
	kDecree = 7,
	kWood = 8,
	kHornlessStatue1 = 9,
	kHornlessStatue2 = 10,
	kHornlessStatue3 = 11,
	kHornlessStatue4 = 12,
	kHornedStatue = 13,
	kCoin = 14,
	kPotion = 15,
	kShield = 16,
	kSword = 17,
	kBag = 18,
	kHelmet = 19,
	kSandals = 20,
	kTorch = 21
};

// Also includes InventoryItem - 1
enum HeroBeltFrame {
	kLightning1 = 21,
	kLightning2 = 22,
	kLightning3 = 23,
	kNumberI = 24,
	kNumberII = 25,
	kNumberIII = 26,
	kQuestScroll = 27,
	kQuestScrollHighlighted = 28,
	kHadesScroll = 29,
	kHadesScrollHighlighted = 30,
	kOptionsButton = 31,
	kInactiveHints = 32,
	kActiveHints = 33,
	kBranchOfLife = 34,
	kReturnToWall = 35,
	kPowerOfWisdom = 38,
	kPowerOfStrength = 39,
	kPowerOfStealth = 40
};

enum FateId {
	kLachesis,
	kAtropos,
	kClotho,
	kNumFates
};

enum CatacombsPosition {
	kCatacombsLeft = 0,
	kCatacombsCenter = 1,
	kCatacombsRight = 2
};

enum CatacombsPath {
	kCatacombsHelen = 0,
	kCatacombsGuards = 1,
	kCatacombsPainAndPanic = 2
};

enum CatacombsLevel {
	kCatacombLevelSign,
	kCatacombLevelTorch,
	kCatacombLevelMusic
};

enum HeroPower {
	kPowerNone = -1,
	kPowerStrength = 0,
	kPowerStealth = 1,
	kPowerWisdom = 2
};
}

#endif
