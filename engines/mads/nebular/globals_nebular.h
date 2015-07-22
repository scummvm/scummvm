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

#ifndef MADS_GLOBALS_NEBULAR_H
#define MADS_GLOBALS_NEBULAR_H

#include "common/scummsys.h"
#include "common/array.h"
#include "mads/game.h"
#include "mads/resources.h"

namespace MADS {

namespace Nebular {

enum GlobalId {
	kSexOfRex				= 0,
	kOldSexOfRex			= 1,
	kWalkerTiming           = 2,
//	kWalkerTiming0			= 3,
	kTalkInanimateCount		= 4,
	kCopyProtectFailed		= 5,

	/* Section #1 variables */
	kNeedToStandUp			= 10,
	kTurkeyExploded			= 11,
	kMedicineCabinetOpen	= 12,
	kMedicineCabinetVirgin	= 13,
	kWatchedViewScreen		= 14,
	kHoovicAlive            = 15,
	kHoovicSated            = 16,
	kHoovicFishEaten        = 17,
	kWaterInAPuddle			= 18,
	kFishIn105				= 19,
	kFishIn107				= 20,
	kFishIn108				= 21,
	kRandomNumber           = 29,

	/* Section #2 variables */
	kCurtainOpen            = 30,
	kLadderBroken           = 31,
	kBone202Status			= 32,
	kMeteorologistStatus    = 33,
	kRhotundaStatus         = 34,
	kLeavesStatus           = 35,
	kMonkeyStatus           = 36,
	kMeteorologistEverSeen  = 37,
	kMeteorologistWatch     = 38,
	kTeleporterCommand      = 39,

	kTeleporterDestination  = 40,
	kTeleporterUnderstood   = 41,
	kConv205                = 42,
	kChickenPermitted      = 43,
	kTwinklesStatus         = 44,
	kTwinklesApproached     = 45,
	kConvTwinkles1          = 46,
	kConvTwinkles2          = 47,
	kConvTwinkles3          = 48,
	kConvTwinkles5			= 49,

	kConvTwinkles6          = 50,
	kConvTwinkles7          = 51,
	kConvTwinkles8          = 52,
	kBlowgunStatus          = 53,

	/* Section #3 Variables */
	kAfterHavoc             = 60,
	kHaveYourStuff          = 61,
	kRightView320			= 62,
	kConvBuddy1				= 63,
	kConvBuddy2				= 64,
	kMetBuddyBeast          = 65,
	kKnowsBuddyBeast        = 66,
	kConvSlache1			= 67,
	kConvSlache2			= 68,
	kConvSlache3			= 69,

	kRexHasMetSlache		= 70,
	kConvIntern             = 71,
	kHasSeenProfPyro		= 72,
	kKickedIn391Grate		= 73,

	/* Section #4 Variables */
	kArmoryDoorOpen			= 80,
	kStorageDoorOpen		= 81,
	kNextIngredient         = 82,
	kIngredientList         = 83,
	kIngredientList1		= 84,
	kIngredientList2		= 85,
	kIngredientList3		= 86,
	kIngredientQuantity     = 87,
	kIngredientQuantity1	= 88,
	kIngredientQuantity2	= 89,

	kIngredientQuantity3	= 90,
	kconvPyro_1             = 91,
	kconvPyro_2             = 92,
	kconvPyro_3             = 93,
	kconvPyro_4             = 94,
	kBadFirstIngredient     = 95,
	kConvBartender1         = 96,
	kConvBartender2			= 97,
	kConvBartender3			= 98,
	kConvBartender4			= 99,

	kHasPurchased			= 100,
	kBeenThruHelgaScene		= 101,
	kHasSaidBinocs			= 102,
	kHasSaidTimer			= 103,
	kBottleDisplayed		= 104,
	kHasBeenScanned			= 105,
	kSomeoneHasExploded		= 106,

	/* Section #5 Variables */
	kBoatRaised				= 110,
	kCarStatus				= 111,
	kCityFlooded			= 112,
	kLaserOn				= 113,
	kLaserHoleIsThere		= 114,
	kCarIsGone				= 115,
	kRegisterOpen			= 116,
	kSafeStatus				= 117,
	kDogStatus              = 118,
	kLineStatus             = 119,

	kHoverCarLocation		= 120,
	kHoverCarDestination	= 121,

	/* Section #6 Variables */
	kConvHermit1			= 130,
	kConvHermit2			= 131,
	kHasTalkedToHermit		= 132,
	kHermitWantsBatteries	= 133,
	kHandsetCellStatus		= 134,
	kBeenInVideoStore		= 135,
	kDurafailRecharged		= 136,
	kPenlightCellStatus		= 137,
	kTimebombStatus			= 138,
	kCheckDaemonTimebomb	= 140,

	kResurrectRoom			= 141,

	/* Section #6 Time-Bomb Variables */
	kTimebombClock			= 142,
//	kTimebombClock0			= 143,
	kTimebombTimer			= 144,
//	kTimebombTimer0			= 145,
	kWarnedFloodCity		= 146,

	/* Section #7 Variables */
	kBottleStatus			= 150,
	kMonsterAlive			= 151,
	kConvBottleFillNode		= 152,
	kBoatStatus				= 153,

	/* Section #8 Variables */
	kAntigravClock			= 160,
//	kAntigravClock0			= 161,
	kAntigravTiming			= 162,
//	kAntigravTiming0		= 163,
	kWindowFixed			= 164,
	kInSpace                = 165,
	kReturnFromCut			= 166,
	kBeamIsUp               = 167,
	kForceBeamDown          = 168,
	kCameFromCut            = 169,

	kCutX                   = 170,
	kCutY                   = 171,
	kCutFacing				= 172,
	kDontRepeat             = 173,
	kHoppyDead              = 174,
	kHasWatchedAntigrav		= 175,
	kRemoteSequenceRan		= 176,
	kRemoteOnGround			= 177,
	kFromCockpit            = 178,
	kExitShip               = 179,

	kBetweenRooms           = 180,
	kTopButtonPushed		= 181,
	kTargetModInstalled		= 182,
	kShieldModInstalled		= 183,
	kUpBecauseOfRemote		= 184,

	kTeleporterRoom         = 190,
	kTeleporterCode         = 200
};

// Enums used for specific individual globals
// Section #1
// Rex's sex/swimming state
enum { REX_MALE = 0, REX_MALE_SWIMMER = 1, REX_FEMALE = 2 };

// Section #2
// State of Meteorologist in the outpost
enum { METEOROLOGIST_ABSENT = 0, METEOROLOGIST_PRESENT = 1, METEOROLOGIST_GONE = 2 };

// State of watching the Meteorologist
enum { METEOROLOGIST_NORMAL = 0, METEOROLOGIST_GROUND = 1, METEOROLOGIST_TOWER = 2 };

// The fat bouncy lady that can squish you on the plains
enum { RHOTUNDA_HUNGRY = 0, RHOTUNDA_STUCK = 1, RHOTUNDA_GONE = 2 };

// Flags for the bones you can take
enum { BONE_202_LEFT_GONE = 1, BONE_202_RIGHT_GONE = 2 };

// Leaves used to cover the trap
enum { LEAVES_ON_GROUND = 0, LEAVES_WITH_PLAYER = 1, LEAVES_ON_TRAP = 2 };

// Monkey ambush state
enum { MONKEY_AMBUSH_READY = 0, MONKEY_HAS_BINOCULARS = 1, MONKEY_IS_GONE = 2 };

// Teleporter status flags
enum {
	TELEPORTER_NONE = 0, TELEPORTER_BEAM_IN = 1, TELEPORTER_BEAM_OUT = 2,
	TELEPORTER_STEP_OUT = 3, TELEPORTER_WRONG = 4
};

// Twinkles status
enum { TWINKLES_AT_HOME = 0, TWINKLES_GONE = 1 };

// Section #4
// Status of the explosives
enum { EXPLOSIVES_INSIDE = 3 };

// Section #5
// Dog status (room 602, outside Abdul's garage)
enum { DOG_PRESENT = 1, DOG_LEFT = 2, DOG_GONE = 3, DOG_DEAD = 4 };

// Dog status (room 608, inside Abdul's garage)
enum { CAR_UP = 0, CAR_SQUASHES_DOG = 1, CAR_SQUASHES_DOG_AGAIN = 2, CAR_DOWN = 3, CAR_DOWN_ON_SQUASHED_DOG = 4 };

enum { SAFE_CLOSED = 0, SAFE_OPENED = 1, SAFE_CLOSED_WITH_HOLE = 2, SAFE_OPENED_WITH_HOLE = 3 };

enum { LINE_NOT_DROPPED = 1, LINE_DROPPED = 2, LINE_TIED = 3, LINE_NOW_UNTIED = 4 };

// Section #6
enum {
	NO_CELLS = 0,				// Handset doesn't contain any cells
	CHARGED_DURAFAIL = 1,		// Handset has charged durafail cells
	UNCHARGED_DURAFAIL = 2,		// Handset has uncharged durafail cells */
	PHONE_CELLS = 3,			// Handset has already charged phone cells
	FIRST_TIME_PHONE_CELLS = 4,	// First time phone cells are in the handset
	FIRST_TIME_UNCHARGED_DURAFAIL = 5,	// First time uncharged cells are in penlight
	FIRST_TIME_CHARGED_DURAFAIL = 6		// First time charged cells are in penlight
};

// Time bomb status
enum {
	TIMEBOMB_DEACTIVATED = 0, TIMEBOMB_ACTIVATED = 1,
	TIMEBOMB_BLOW_UP = 2, TIMEBOMB_DEAD = 3
};

/* Section #7 */
// Status of the the bottle
enum {
	BOTTLE_EMPTY = 0, BOTTLE_ONE_QUARTER_FULL = 1, BOTTLE_HALF_FULL = 2,
	BOTTLE_THREE_QUARTERS_FULL = 3, BOTTLE_FULL = 4
};

// Status of the boat
enum {
	BOAT_UNFLOODED = 0, BOAT_ADRIFT = 1, BOAT_TIED_FLOATING = 2,
	BOAT_TIED = 3, BOAT_GONE = 4
};


/* Miscellaneous defines */
#define TELEPORTER_COUNT 10      // Total number of teleporters
#define TELEPORTER_WORK_COUNT 6  // Total number that actually work


class NebularGlobals : public Globals {
public:
	SynchronizedList _spriteIndexes;
	SynchronizedList _sequenceIndexes;

	int _timebombClock, _timebombTimer;
public:
	/**
	 * Constructor
	 */
	NebularGlobals();

	/**
	* Synchronize the globals data
	*/
	virtual void synchronize(Common::Serializer &s);
};

} // End of namespace Nebular

} // End of namespace MADS

#endif /* MADS_GLOBALS_NEBULAR_H */
