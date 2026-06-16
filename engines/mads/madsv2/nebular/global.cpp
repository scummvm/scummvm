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

#include "mads/madsv2/nebular/global.h"
#include "mads/madsv2/nebular/mads/inventory.h"
#include "mads/madsv2/core/game.h"
#include "mads/madsv2/core/global.h"
#include "mads/madsv2/core/imath.h"
#include "mads/madsv2/core/inter.h"
#include "mads/madsv2/core/kernel.h"
#include "mads/madsv2/core/object.h"
#include "mads/madsv2/core/player.h"
#include "mads/madsv2/core/error.h"

namespace MADS {
namespace MADSV2 {
namespace RexNebular {
namespace Rooms {

// Section preloads
extern void section_1_preload();
extern void section_2_preload();
extern void section_3_preload();
extern void section_4_preload();
extern void section_5_preload();
extern void section_6_preload();
extern void section_7_preload();
extern void section_8_preload();

extern void room_101_synchronize(Common::Serializer &s);
extern void room_101_synchronize(Common::Serializer &s);
extern void room_102_synchronize(Common::Serializer &s);
extern void room_103_synchronize(Common::Serializer &s);
extern void room_104_synchronize(Common::Serializer &s);
extern void room_105_synchronize(Common::Serializer &s);
extern void room_106_synchronize(Common::Serializer &s);
extern void room_107_synchronize(Common::Serializer &s);
extern void room_108_synchronize(Common::Serializer &s);
extern void room_109_synchronize(Common::Serializer &s);
extern void room_110_synchronize(Common::Serializer &s);
extern void room_111_synchronize(Common::Serializer &s);
extern void room_112_synchronize(Common::Serializer &s);
extern void room_201_synchronize(Common::Serializer &s);
extern void room_202_synchronize(Common::Serializer &s);
extern void room_203_synchronize(Common::Serializer &s);
extern void room_205_synchronize(Common::Serializer &s);
extern void room_207_synchronize(Common::Serializer &s);
extern void room_208_synchronize(Common::Serializer &s);
extern void room_209_synchronize(Common::Serializer &s);
extern void room_210_synchronize(Common::Serializer &s);
extern void room_211_synchronize(Common::Serializer &s);
extern void room_212_synchronize(Common::Serializer &s);
extern void room_213_synchronize(Common::Serializer &s);
extern void room_214_synchronize(Common::Serializer &s);
extern void room_215_synchronize(Common::Serializer &s);
extern void room_216_synchronize(Common::Serializer &s);
extern void room_301_synchronize(Common::Serializer &s);
extern void room_302_synchronize(Common::Serializer &s);
extern void room_303_synchronize(Common::Serializer &s);
extern void room_304_synchronize(Common::Serializer &s);
extern void room_307_synchronize(Common::Serializer &s);
extern void room_308_synchronize(Common::Serializer &s);
extern void room_309_synchronize(Common::Serializer &s);
extern void room_310_synchronize(Common::Serializer &s);
extern void room_311_synchronize(Common::Serializer &s);
extern void room_313_synchronize(Common::Serializer &s);
extern void room_316_synchronize(Common::Serializer &s);
extern void room_318_synchronize(Common::Serializer &s);
extern void room_319_synchronize(Common::Serializer &s);
extern void room_320_synchronize(Common::Serializer &s);
extern void room_321_synchronize(Common::Serializer &s);
extern void room_322_synchronize(Common::Serializer &s);
extern void room_351_synchronize(Common::Serializer &s);
extern void room_352_synchronize(Common::Serializer &s);
extern void room_353_synchronize(Common::Serializer &s);
extern void room_354_synchronize(Common::Serializer &s);
extern void room_357_synchronize(Common::Serializer &s);
extern void room_358_synchronize(Common::Serializer &s);
extern void room_359_synchronize(Common::Serializer &s);
extern void room_360_synchronize(Common::Serializer &s);
extern void room_361_synchronize(Common::Serializer &s);
extern void room_366_synchronize(Common::Serializer &s);
extern void room_387_synchronize(Common::Serializer &s);
extern void room_388_synchronize(Common::Serializer &s);
extern void room_389_synchronize(Common::Serializer &s);
extern void room_390_synchronize(Common::Serializer &s);
extern void room_391_synchronize(Common::Serializer &s);
extern void room_399_synchronize(Common::Serializer &s);
extern void room_401_synchronize(Common::Serializer &s);
extern void room_402_synchronize(Common::Serializer &s);
extern void room_405_synchronize(Common::Serializer &s);
extern void room_406_synchronize(Common::Serializer &s);
extern void room_407_synchronize(Common::Serializer &s);
extern void room_408_synchronize(Common::Serializer &s);
extern void room_409_synchronize(Common::Serializer &s);
extern void room_410_synchronize(Common::Serializer &s);
extern void room_411_synchronize(Common::Serializer &s);
extern void room_413_synchronize(Common::Serializer &s);
extern void room_501_synchronize(Common::Serializer &s);
extern void room_502_synchronize(Common::Serializer &s);
extern void room_503_synchronize(Common::Serializer &s);
extern void room_504_synchronize(Common::Serializer &s);
extern void room_505_synchronize(Common::Serializer &s);
extern void room_506_synchronize(Common::Serializer &s);
extern void room_507_synchronize(Common::Serializer &s);
extern void room_508_synchronize(Common::Serializer &s);
extern void room_511_synchronize(Common::Serializer &s);
extern void room_512_synchronize(Common::Serializer &s);
extern void room_513_synchronize(Common::Serializer &s);
extern void room_515_synchronize(Common::Serializer &s);
extern void room_551_synchronize(Common::Serializer &s);
extern void room_601_synchronize(Common::Serializer &s);
extern void room_602_synchronize(Common::Serializer &s);
extern void room_603_synchronize(Common::Serializer &s);
extern void room_604_synchronize(Common::Serializer &s);
extern void room_605_synchronize(Common::Serializer &s);
extern void room_607_synchronize(Common::Serializer &s);
extern void room_608_synchronize(Common::Serializer &s);
extern void room_609_synchronize(Common::Serializer &s);
extern void room_610_synchronize(Common::Serializer &s);
extern void room_611_synchronize(Common::Serializer &s);
extern void room_612_synchronize(Common::Serializer &s);
extern void room_620_synchronize(Common::Serializer &s);
extern void room_701_synchronize(Common::Serializer &s);
extern void room_702_synchronize(Common::Serializer &s);
extern void room_703_synchronize(Common::Serializer &s);
extern void room_704_synchronize(Common::Serializer &s);
extern void room_705_synchronize(Common::Serializer &s);
extern void room_706_synchronize(Common::Serializer &s);
extern void room_707_synchronize(Common::Serializer &s);
extern void room_710_synchronize(Common::Serializer &s);
extern void room_711_synchronize(Common::Serializer &s);
extern void room_751_synchronize(Common::Serializer &s);
extern void room_752_synchronize(Common::Serializer &s);
extern void room_801_synchronize(Common::Serializer &s);
extern void room_802_synchronize(Common::Serializer &s);
extern void room_803_synchronize(Common::Serializer &s);
extern void room_804_synchronize(Common::Serializer &s);
extern void room_805_synchronize(Common::Serializer &s);
extern void room_807_synchronize(Common::Serializer &s);
extern void room_808_synchronize(Common::Serializer &s);
extern void room_810_synchronize(Common::Serializer &s);

} // namespace Rooms

void global_init_code() {
	for (int i = 0; i < 210; i++)
		global[i] = 0;

	global[kTalkInanimateCount]     = 8;
	global[kMeteorologistStatus]    = 1;

	global[kTurkeyExploded]         = 0;
	global[kMedicineCabinetOpen]    = 0;
	global[kWatchedViewScreen]      = 0;
	global[kWaterInAPuddle]         = 0;
	global[kLadderBroken]           = 0;
	global[kBone202Status]          = 0;
	global[kRhotundaStatus]         = 0;
	global[kMonkeyStatus]           = 0;
	global[kMeteorologistEverSeen]  = 0;
	global[kMeteorologistWatch]     = 0;
	global[kTeleporterCommand]      = 0;
	global[kTeleporterUnderstood]   = 0;
	global[kTwinklesStatus]         = 0;
	global[kTwinklesApproached]     = 0;
	global[kAfterHavoc]             = 0;
	global[kKickedIn391Grate]       = 0;

	global[kNeedToStandUp]          = -1;
	global[kMedicineCabinetVirgin]  = -1;
	global[kHoovicAlive]            = -1;
	global[kFishIn105]              = -1;
	global[kFishIn107]              = -1;
	global[kFishIn108]              = -1;
	global[kBadFirstIngredient]     = -1;

	object_set_quality(OBJ_CHARGE_CASES, 3, 0);

	global[kHasPurchased]           = 0;
	global[kBeenThruHelgaScene]     = 0;
	global[kNextIngredient]         = 0;
	global[kHasSaidTimer]           = 0;
	global[kHasSaidBinocs]          = 0;
	global[kBottleDisplayed]        = 0;
	global[kHasBeenScanned]         = 0;
	global[kSomeoneHasExploded]     = 0;

	// Fill kIngredientList[0..3] with 4 unique random values in [0, 3]
	for (int index = 0; index < 4; ) {
		global[kIngredientList + index] = imath_random(0, 3);
		int flag = 0;
		for (int idx2 = 0; idx2 < index; idx2++) {
			if (global[kIngredientList + idx2] == global[kIngredientList + index])
				flag = -1;
		}
		if (!flag)
			index++;
	}

	// Fill kIngredientQuantity[0..3] with 4 unique random values in [0, 3]
	for (int index = 0; index < 4; ) {
		global[kIngredientQuantity + index] = imath_random(0, 3);
		int flag = 0;
		for (int idx2 = 0; idx2 < index; idx2++) {
			if (global[kIngredientQuantity + idx2] == global[kIngredientQuantity + index])
				flag = -1;
		}
		if (!flag)
			index++;
	}

	global[kHoverCarLocation]       = 501;
	global[kHoverCarDestination]    = -1;
	global[kBoatRaised]             = -1;
	global[kLineStatus]             = 1;
	global[kHandsetCellStatus]      = 4;
	global[kCityFlooded]            = 0;
	global[kLaserHoleIsThere]       = 0;
	global[kHasTalkedToHermit]      = 0;
	global[kTimebombStatus]         = 0;
	global[kTimebombClock + 1]      = 0;
	global[kTimebombClock]          = 0;
	global[kTimebombTimer + 1]      = 0;
	global[kTimebombTimer]          = 0;

	global[kTeleporterRoom + 0]     = 201;
	global[kTeleporterRoom + 1]     = 301;
	global[kTeleporterRoom + 2]     = 413;
	global[kTeleporterRoom + 3]     = 706;
	global[kTeleporterRoom + 4]     = 801;
	global[kTeleporterRoom + 5]     = 551;
	global[kTeleporterRoom + 6]     = 752;

	global[kWarnedFloodCity]        = 0;
	global[kBottleStatus]           = 0;
	global[kBoatStatus]             = 0;
	global[kWindowFixed]            = 0;
	global[kInSpace]                = 0;
	global[kReturnFromCut]          = 0;
	global[kBeamIsUp]               = 0;
	global[kForceBeamDown]          = 0;
	global[kCameFromCut]            = 0;
	global[kDontRepeat]             = 0;
	global[kHoppyDead]              = 0;
	global[kHasWatchedAntigrav]     = 0;
	global[kRemoteSequenceRan]      = 0;
	global[kRemoteOnGround]         = 0;
	global[kFromCockpit]            = 0;
	global[kExitShip]               = 0;
	global[kBetweenRooms]           = 0;
	global[kTopButtonPushed]        = 0;
	global[kShieldModInstalled]     = 0;
	global[kTargetModInstalled]     = 0;
	global[kUpBecauseOfRemote]      = 0;
	global[kTeleporterRoom + 7]     = 0;
	global[kTeleporterRoom + 8]     = 0;
	global[kTeleporterRoom + 9]     = 0;

	// Fill kTeleporterCode[0..9] with 10 unique random values in [0, 9999]
	for (int index = 0; index < 10; ) {
		global[kTeleporterCode + index] = imath_random(0, 9999);
		int flag = 0;
		for (int idx2 = 0; idx2 < index; idx2++) {
			if (global[kTeleporterCode + idx2] == global[kTeleporterCode + index])
				flag = -1;
		}
		if (!flag)
			index++;
	}

	switch (game.difficulty) {
	case DIFFICULTY_HARD:
		global[kLeavesStatus] = 0;
		inter_move_object(OBJ_BLOWGUN, 1);
		inter_move_object(OBJ_NOTE, 1);
		global[kPenlightCellStatus] = 5;
		global[kDurafailRecharged]  = 0;
		break;
	case DIFFICULTY_MEDIUM:
		global[kLeavesStatus] = 0;
		inter_move_object(OBJ_PLANT_STALK, 1);
		global[kDurafailRecharged]  = -1;
		global[kPenlightCellStatus] = 6;
		break;
	case DIFFICULTY_EASY:
		global[kLeavesStatus] = 2;
		inter_move_object(OBJ_PLANT_STALK, 1);
		inter_move_object(OBJ_PENLIGHT, 1);
		break;
	}

	player.facing          = FACING_NORTH;
	player.turn_to_facing  = FACING_NORTH;
	player_himem_preload(player.series_name, 1);
	player_himem_preload("ROX", 1);
}

void global_section_constructor() {
	section_preload_code_pointer = NULL;
	section_room_constructor = NULL;
	section_init_code_pointer = NULL;
	section_parser_code_pointer = NULL;
	section_daemon_code_pointer = NULL;

	room_preload_code_pointer = NULL;
	room_init_code_pointer = NULL;
	room_daemon_code_pointer = NULL;
	room_pre_parser_code_pointer = NULL;
	room_parser_code_pointer = NULL;
	room_error_code_pointer = NULL;
	room_shutdown_code_pointer = NULL;

	switch (new_section) {
	case 1:
		section_preload_code_pointer = Rooms::section_1_preload;
		break;
	case 2:
		section_preload_code_pointer = Rooms::section_2_preload;
		break;
	case 3:
		section_preload_code_pointer = Rooms::section_3_preload;
		break;
	case 4:
		section_preload_code_pointer = Rooms::section_4_preload;
		break;
	case 5:
		section_preload_code_pointer = Rooms::section_5_preload;
		break;
	case 6:
		section_preload_code_pointer = Rooms::section_6_preload;
		break;
	case 7:
		section_preload_code_pointer = Rooms::section_7_preload;
		break;
	case 8:
		section_preload_code_pointer = Rooms::section_8_preload;
		break;
	}
}

void sync_room(Common::Serializer &s) {
	switch (new_room) {
	case 101: Rooms::room_101_synchronize(s);
	case 102: Rooms::room_102_synchronize(s);
	case 103: Rooms::room_103_synchronize(s);
	case 104: Rooms::room_104_synchronize(s);
	case 105: Rooms::room_105_synchronize(s);
	case 106: Rooms::room_106_synchronize(s);
	case 107: Rooms::room_107_synchronize(s);
	case 108: Rooms::room_108_synchronize(s);
	case 109: Rooms::room_109_synchronize(s);
	case 110: Rooms::room_110_synchronize(s);
	case 111: Rooms::room_111_synchronize(s);
	case 112: Rooms::room_112_synchronize(s);
	case 201: Rooms::room_201_synchronize(s);
	case 202: Rooms::room_202_synchronize(s);
	case 203: Rooms::room_203_synchronize(s);
	case 205: Rooms::room_205_synchronize(s);
	case 207: Rooms::room_207_synchronize(s);
	case 208: Rooms::room_208_synchronize(s);
	case 209: Rooms::room_209_synchronize(s);
	case 210: Rooms::room_210_synchronize(s);
	case 211: Rooms::room_211_synchronize(s);
	case 212: Rooms::room_212_synchronize(s);
	case 213: Rooms::room_213_synchronize(s);
	case 214: Rooms::room_214_synchronize(s);
	case 215: Rooms::room_215_synchronize(s);
	case 216: Rooms::room_216_synchronize(s);
	case 301: Rooms::room_301_synchronize(s);
	case 302: Rooms::room_302_synchronize(s);
	case 303: Rooms::room_303_synchronize(s);
	case 304: Rooms::room_304_synchronize(s);
	case 307: Rooms::room_307_synchronize(s);
	case 308: Rooms::room_308_synchronize(s);
	case 309: Rooms::room_309_synchronize(s);
	case 310: Rooms::room_310_synchronize(s);
	case 311: Rooms::room_311_synchronize(s);
	case 313: Rooms::room_313_synchronize(s);
	case 316: Rooms::room_316_synchronize(s);
	case 318: Rooms::room_318_synchronize(s);
	case 319: Rooms::room_319_synchronize(s);
	case 320: Rooms::room_320_synchronize(s);
	case 321: Rooms::room_321_synchronize(s);
	case 322: Rooms::room_322_synchronize(s);
	case 351: Rooms::room_351_synchronize(s);
	case 352: Rooms::room_352_synchronize(s);
	case 353: Rooms::room_353_synchronize(s);
	case 354: Rooms::room_354_synchronize(s);
	case 357: Rooms::room_357_synchronize(s);
	case 358: Rooms::room_358_synchronize(s);
	case 359: Rooms::room_359_synchronize(s);
	case 360: Rooms::room_360_synchronize(s);
	case 361: Rooms::room_361_synchronize(s);
	case 366: Rooms::room_366_synchronize(s);
	case 387: Rooms::room_387_synchronize(s);
	case 388: Rooms::room_388_synchronize(s);
	case 389: Rooms::room_389_synchronize(s);
	case 390: Rooms::room_390_synchronize(s);
	case 391: Rooms::room_391_synchronize(s);
	case 399: Rooms::room_399_synchronize(s);
	case 401: Rooms::room_401_synchronize(s);
	case 402: Rooms::room_402_synchronize(s);
	case 405: Rooms::room_405_synchronize(s);
	case 406: Rooms::room_406_synchronize(s);
	case 407: Rooms::room_407_synchronize(s);
	case 408: Rooms::room_408_synchronize(s);
	case 409: Rooms::room_409_synchronize(s);
	case 410: Rooms::room_410_synchronize(s);
	case 411: Rooms::room_411_synchronize(s);
	case 413: Rooms::room_413_synchronize(s);
	case 501: Rooms::room_501_synchronize(s);
	case 502: Rooms::room_502_synchronize(s);
	case 503: Rooms::room_503_synchronize(s);
	case 504: Rooms::room_504_synchronize(s);
	case 505: Rooms::room_505_synchronize(s);
	case 506: Rooms::room_506_synchronize(s);
	case 507: Rooms::room_507_synchronize(s);
	case 508: Rooms::room_508_synchronize(s);
	case 511: Rooms::room_511_synchronize(s);
	case 512: Rooms::room_512_synchronize(s);
	case 513: Rooms::room_513_synchronize(s);
	case 515: Rooms::room_515_synchronize(s);
	case 551: Rooms::room_551_synchronize(s);
	case 601: Rooms::room_601_synchronize(s);
	case 602: Rooms::room_602_synchronize(s);
	case 603: Rooms::room_603_synchronize(s);
	case 604: Rooms::room_604_synchronize(s);
	case 605: Rooms::room_605_synchronize(s);
	case 607: Rooms::room_607_synchronize(s);
	case 608: Rooms::room_608_synchronize(s);
	case 609: Rooms::room_609_synchronize(s);
	case 610: Rooms::room_610_synchronize(s);
	case 611: Rooms::room_611_synchronize(s);
	case 612: Rooms::room_612_synchronize(s);
	case 620: Rooms::room_620_synchronize(s);
	case 701: Rooms::room_701_synchronize(s);
	case 702: Rooms::room_702_synchronize(s);
	case 703: Rooms::room_703_synchronize(s);
	case 704: Rooms::room_704_synchronize(s);
	case 705: Rooms::room_705_synchronize(s);
	case 706: Rooms::room_706_synchronize(s);
	case 707: Rooms::room_707_synchronize(s);
	case 710: Rooms::room_710_synchronize(s);
	case 711: Rooms::room_711_synchronize(s);
	case 751: Rooms::room_751_synchronize(s);
	case 752: Rooms::room_752_synchronize(s);
	case 801: Rooms::room_801_synchronize(s);
	case 802: Rooms::room_802_synchronize(s);
	case 803: Rooms::room_803_synchronize(s);
	case 804: Rooms::room_804_synchronize(s);
	case 805: Rooms::room_805_synchronize(s);
	case 807: Rooms::room_807_synchronize(s);
	case 808: Rooms::room_808_synchronize(s);
	case 810: Rooms::room_810_synchronize(s);
	default: break;
	}
}

} // namespace RexNebular
} // namespace MADSV2
} // namespace MADS
