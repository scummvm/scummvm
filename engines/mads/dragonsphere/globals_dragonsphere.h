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
 */

#ifndef MADS_GLOBALS_DRAGONSPHERE_H
#define MADS_GLOBALS_DRAGONSPHERE_H

#include "common/scummsys.h"
#include "common/array.h"
#include "mads/game.h"
#include "mads/resources.h"

namespace MADS {

namespace Dragonsphere {

enum GlobalId {
	// Global variables

	kWalkerTiming           = 0,
	kWalkerTiming2			= 1,
	kPlayerPersona = 10,
	kPlayerScore = 11,
	kDragonHighScene = 12,
	kDragonMyScene = 13,
	kNoLoadWalker = 14,
	kPlayerScoreFlags = 15,
	kSorcerorDefeated = 16,
	kPreRoom = 17,
	kPerformDisplacements = 18,

	kKingStatus = 20,
	kWardStatus = 21,
	kTalkedToSoptus = 22,
	kTalkedToShifter = 23,
	kAmuletStatus = 24,
	kBooksStatus = 25,
	kTapestryStatus = 26,
	kTalkedToMerchant = 27,
	kCanViewCrownHole = 28,
	kTalkedToStatus = 29,

	kGuardPidStatus = 30,
	kCrawledOutOfBed101 = 31,
	kStatueIsOnStairway114 = 32,
	kDogIsAsleep = 33,
	kCrystalBallDead = 34,
	kThrewBone = 35,
	kWoodenDoorOpen = 36,
	kKingIsInStairwell = 37,
	kLlanieStatus = 38,
	kNoTalkToGuard = 39,

	kObjectGiven201 = 40,
	kKingGotStabbed = 41,
	kGivenObjectBefore = 42,
	kGuardsAreAsleep = 43,
	kDomeUp = 44,
	kTalkedToWise = 45,
	kDollGiven = 46,
	kResetConv = 47,
	kTalkedToGreta = 48,
	kSlimeHealed = 49,

	kDanceMusicOn = 50,
	kPlayerIsSeal = 51,
	kVinesHavePlayer = 52,
	kEndOfGame = 53,
	kResetConv2 = 54,
	kInvokedFrom111 = 55,

	kSaveWinsInDesert = 60,
	kBubblesUpIn301 = 61,
	kBubbleWontAttack = 62,
	kPidLookedAtDoll = 63,

	kOasis = 80,
	kFireHoles = 81,
	kDesertRoom = 82,
	kFromDirection = 83,
	kDesertCounter = 84,
	kMoveDirection409 = 85,
	kHealVerbsVisible = 86,
	kGrapesHaveGrown = 87,
	kPidTalkShamon = 88,
	kPidJustDied = 89,

	kGrapesAreDead = 90,
	kRocIsChewingDates = 91,
	kWinsInDesert = 92,
	kWinsTillPrize = 93,
	kPidHasBeenHealedSop = 94,
	kGamePoints = 95,
	kDancePoints = 96,
	kCluePoints = 97,
	kPrizesOwedToPlayer = 98,
	kObjectFlags = 99,

	kWaterfallDiverted = 100,
	kShakStatus = 101,
	kMaxGridValue = 102,
	kMoveDirection510 = 103,
	kShak506Angry = 104,
	kMonsterIsDead = 105,
	kDoneTalkingLani502 = 106,
	kFoundLani504 = 107,
	kSaidUseSwordShak = 108,
	kGobletFilledSoporific = 109,

	kBeenIn504AsPid = 110,
	kSeenLaniDeadFirstTime = 111,
	kSaidPoemIn504 = 112,
	kTriedToHealLlanie504 = 113,
	kPutBundleOnLlanie504 = 114,
	kMake504Empty = 115,
	kHasTakenMud = 116,
	kPlatformClicked606 = 117,
	kHadSpiritBundle = 118,

	kMudIsInEye603 = 120,
	kRopeIsAlive = 121,
	kRatCageIsOpen = 122,
	kFlaskOnPlate = 123,
	kFluidIsDripping = 124,
	kHoleIsIn607 = 125,
	kRopeIsHangingIn607 = 126,
	kObjectIsInFreezer605 = 127,
	kObjectImitated = 128,
	kHasRed = 129,

	kHasYellow = 130,
	kHasBlue = 131,
	kWizardDead = 132,
	kVineWillGrab = 133,
	kFloorIsCool = 134,
	kRatMelted = 135,
	kDoorIsCool = 136,
	kUsedElevator = 137,
	kBeenOnTopFloor = 138,
	kTorchIsIn609 = 139,

	kGridPosition = 140
	};

class DragonsphereGlobals : public Globals {
public:
	SynchronizedList _spriteIndexes;
	SynchronizedList _sequenceIndexes;
	SynchronizedList _animationIndexes;
public:
	/**
	 * Constructor
	 */
	DragonsphereGlobals();

	/**
	* Synchronize the globals data
	*/
	virtual void synchronize(Common::Serializer &s);
};

} // End of namespace Dragonsphere

} // End of namespace MADS

#endif /* MADS_GLOBALS_DRAGONSPHERE_H */
