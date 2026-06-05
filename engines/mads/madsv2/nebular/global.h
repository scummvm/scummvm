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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef MADS_NEBULAR_GLOBAL_H
#define MADS_NEBULAR_GLOBAL_H

#include "common/serializer.h"
#include "mads/madsv2/core/vocabh.h"

namespace MADS {
namespace MADSV2 {
namespace RexNebular {

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

enum {
	SEX_MALE = 0, SEX_UNKNOWN = 1, SEX_FEMALE = 2
};

extern void global_section_constructor();
extern void sync_room(Common::Serializer &s);

} // namespace RexNebular
} // namespace MADSV2
} // namespace MADS

#endif
