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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "common/scummsys.h"
#include "common/config-manager.h"
#include "mads/mads.h"
#include "mads/game.h"
#include "mads/screen.h"
#include "mads/msurface.h"
#include "mads/nebular/game_nebular.h"
#include "mads/nebular/dialogs_nebular.h"
#include "mads/nebular/globals_nebular.h"

namespace MADS {

namespace Nebular {

GameNebular::GameNebular(MADSEngine *vm): Game(vm) {
	_surface = new MSurface(MADS_SCREEN_WIDTH, MADS_SCENE_HEIGHT);
	_storyMode = STORYMODE_NAUGHTY;
}

ProtectionResult GameNebular::checkCopyProtection() {
	/*
	// DEBUG: Flag copy protection failure
	_globals[5] = -1;
	
	if (!ConfMan.getBool("copy_protection"))
		return true;

	* DEBUG: Disabled for now
	CopyProtectionDialog *dlg = new CopyProtectionDialog(_vm, false);
	dlg->show();
	delete dlg;
	*/

	// DEBUG: Return that copy protection failed
	return PROTECTION_SUCCEED;
}

void GameNebular::initialiseGlobals() {
	int count, count2;
	int bad;

	_globals.reset();
	_globals[kTalkInanimateCount] = 8;

	/* Section #1 variables */
	_globals[kNeedToStandUp] = true;
	_globals[kTurkeyExploded] = false;
	_globals[kMedicineCabinetOpen] = false;
	_globals[kMedicineCabinetVirgin] = true;
	_globals[kWatchedViewScreen] = false;
	_globals[kHoovicAlive] = true;
	_globals[kWaterInAPuddle] = false;

	_globals[kFishIn105] = true;
	_globals[kFishIn107] = true;
	_globals[kFishIn108] = true;

	/* Section #2 variables */
	_globals[kLadderBroken] = false;
	_globals[kBone202Status] = 0;
	_globals[kRhotundaStatus] = RHOTUNDA_HUNGRY;
	_globals[kMonkeyStatus] = MONKEY_AMBUSH_READY;
	_globals[kMeteorologistStatus] = METEOROLOGIST_PRESENT;
	_globals[kMeteorologistEverSeen] = false;
	_globals[kMeteorologistWatch] = METEOROLOGIST_NORMAL;
	_globals[kTeleporterCommand] = TELEPORTER_NONE;
	_globals[kTeleporterUnderstood] = false;
	_globals[kTwinklesStatus] = TWINKLES_AT_HOME;
	_globals[kTwinklesApproached] = 0;

	/* Section #3 variables */
	_globals[kAfterHavoc] = false;
	_globals[kKickedIn391Grate] = false;

	/* Section #4 variables */
	_globals[kBadFirstIngredient] = -1;
	_objects.setQuality(OBJ_CHARGE_CASES, EXPLOSIVES_INSIDE, nullptr);
	_globals[kHasPurchased] = false;
	_globals[kBeenThruHelgaScene] = false;
	_globals[kNextIngredient] = 0;
	_globals[kHasSaidTimer] = false;
	_globals[kHasSaidBinocs] = false;
	_globals[kBottleDisplayed] = false;
	_globals[kHasBeenScanned] = false;
	_globals[kSomeoneHasExploded] = false;

	// Generate a random ingredient list
	for (count = 0; count < 4; ++count) {
		do {
			_globals[kIngredientList + count] = _vm->getRandomNumber(3);
			bad = false;
			for (count2 = 0; count2 < count; ++count2) {
				if (_globals[kIngredientList + count] == _globals[kIngredientList + count2]) {
					bad = true;
				}
			}
		} while (bad);
	}

	// Generate random ingredient quantities
	for (count = 0; count < 4; ++count) {
		do {
			_globals[kIngredientQuantity + count] = _vm->getRandomNumber(3);
			bad = false;
			for (count2 = 0; count2 < count; ++count2) {
				if (_globals[kIngredientQuantity + count] == _globals[kIngredientQuantity + count2]) {
					bad = true;
				}
			}
		} while (bad);
	}


	/* Section #5 variables */
	_globals[kHoverCarLocation] = 501;
	_globals[kHoverCarDestination] = -1;
	_globals[kCityFlooded] = false;
	_globals[kBoatRaised] = true;
	_globals[kLaserHoleIsThere] = false;
	_globals[kLineStatus] = LINE_NOT_DROPPED;


	/* Section #6 variables */
	_globals[kHasTalkedToHermit] = false;
	_globals[kHandsetCellStatus] = FIRST_TIME_PHONE_CELLS;
	_globals[kTimebombStatus] = TIMEBOMB_DEACTIVATED;
	_globals[kWarnedFloodCity] = false;
	_globals._timebombClock = 0;
	_globals._timebombTimer = 0;


	/* Section #7 variables */
	_globals[kBottleStatus] = BOTTLE_EMPTY;
	_globals[kBoatStatus] = BOAT_UNFLOODED;


	/* Section #8 variables */
	_globals[kWindowFixed] = false;
	_globals[kInSpace] = false;
	_globals[kReturnFromCut] = false;
	_globals[kBeamIsUp] = false;
	_globals[kForceBeamDown] = false;
	_globals[kCameFromCut] = false;
	_globals[kDontRepeat] = false;
	_globals[kHoppyDead] = false;
	_globals[kHasWatchedAntigrav] = false;
	_globals[kRemoteSequenceRan] = false;
	_globals[kRemoteOnGround] = false;
	_globals[kFromCockpit] = false;
	_globals[kExitShip] = false;
	_globals[kBetweenRooms] = false;
	_globals[kTopButtonPushed] = false;
	_globals[kShieldModInstalled] = false;
	_globals[kTargetModInstalled] = false;
	_globals[kUpBecauseOfRemote] = false;


	/* Set up the game's teleporters */
	_globals[kTeleporterRoom] = 201;
	_globals[kTeleporterRoom + 1] = 301;
	_globals[kTeleporterRoom + 2] = 413;
	_globals[kTeleporterRoom + 3] = 706;
	_globals[kTeleporterRoom + 4] = 801;
	_globals[kTeleporterRoom + 5] = 551;
	_globals[kTeleporterRoom + 6] = 752;
	_globals[kTeleporterRoom + 7] = 0;
	_globals[kTeleporterRoom + 8] = 0;
	_globals[kTeleporterRoom + 9] = 0;

	for (count = 0; count < TELEPORTER_COUNT; ++count) {
		do {
			_globals[kTeleporterCode + count] = _vm->getRandomNumber(9999);
			bad = false;
			for (count2 = 0; count2 < count; ++count2) {
				if (_globals[kTeleporterCode + count] == _globals[kTeleporterCode + count2]) {
					bad = true;
				}
			}
		} while (bad);
	}

	// Final setup based on selected difficulty level
	switch (_difficulty) {
	case DIFFICULTY_HARD:
		_objects.setRoom(OBJ_PLANT_STALK, NOWHERE);
		_objects.setRoom(OBJ_PENLIGHT, NOWHERE);

		_globals[kLeavesStatus] = LEAVES_ON_TRAP;
		break;

	case DIFFICULTY_REALLY_HARD:
		_objects.setRoom(OBJ_PLANT_STALK, NOWHERE);

		_globals[kLeavesStatus] = LEAVES_ON_GROUND;
		_globals[kDurafailRecharged] = true;
		_globals[kPenlightCellStatus] = FIRST_TIME_CHARGED_DURAFAIL;
		break;

	case DIFFICULTY_IMPOSSIBLE:
		_objects.setRoom(OBJ_BLOWGUN, NOWHERE);
		_objects.setRoom(OBJ_NOTE, NOWHERE);

		_globals[kLeavesStatus] = LEAVES_ON_GROUND;
		_globals[kPenlightCellStatus] = FIRST_TIME_UNCHARGED_DURAFAIL;
		_globals[kDurafailRecharged] = false;
		break;
	}

	_player._facing = FACING_NORTH;
	_player._turnToFacing = FACING_NORTH;

	loadResourceSequence("RXM", 1);
	loadResourceSequence("ROX", 1);
}

void GameNebular::setSectionHandler() {
	delete _sectionHandler;

	switch (_sectionNumber) {
	case 1:
		_sectionHandler = new Section1Handler(_vm);
		break;
	case 2:
		_sectionHandler = new Section2Handler(_vm);
		break;
	case 3:
		_sectionHandler = new Section3Handler(_vm);
		break;
	case 4:
		_sectionHandler = new Section4Handler(_vm);
		break;
	case 5:
		_sectionHandler = new Section5Handler(_vm);
		break;
	case 6:
		_sectionHandler = new Section6Handler(_vm);
		break;
	case 7:
		_sectionHandler = new Section7Handler(_vm);
		break;
	case 8:
		_sectionHandler = new Section8Handler(_vm);
		break;
	default:
		break;
	}
}

void GameNebular::checkShowDialog() {
	if (_vm->_dialogs->_pendingDialog && _player._stepEnabled && !_globals[5]) {
		_scene._spriteSlots.releasePlayerSprites();
		_vm->_dialogs->showDialog();
		_vm->_dialogs->_pendingDialog = DIALOG_NONE;
	}
}

void GameNebular::doObjectAction() {
	warning("TODO: GameNebular::doObjectAction");
}

void GameNebular::step() {
	if (_player._visible && _player._stepEnabled && !_player._moving &&
		(_player._facing == _player._turnToFacing)) {
		if (_scene._frameStartTime >= *((uint32 *)&_globals[kWalkerTiming])) {
			if (!_player._stopWalkerIndex) {
				int randomVal = _vm->getRandomNumber(29999);;
				if (_globals[kSexOfRex] == REX_MALE) {
					switch (_player._facing) {
					case FACING_SOUTHWEST:
					case FACING_SOUTHEAST:
					case FACING_NORTHWEST:
					case FACING_NORTHEAST:
						if (randomVal < 200) {
							_player.addWalker(-1, 0);
							_player.addWalker(1, 0);
						}
						break;

					case FACING_WEST:
					case FACING_EAST:
						if (randomVal < 500) {
							for (int count = 0; count < 10; ++count) {
								_player.addWalker(1, 0);
							}
						}
						break;

					case 2:
						if (randomVal < 500) {
							for (int count = 0; count < 10; ++count) {
								_player.addWalker((randomVal < 250) ? 1 : 2, 0);
							}
						} else if (randomVal < 750) {
							for (int count = 0; count < 5; ++count) {
								_player.addWalker(1, 0);
							}

							_player.addWalker(0, 0);
							_player.addWalker(0, 0);

							for (int count = 0; count < 5; ++count) {
								_player.addWalker(2, 0);
							}
						}
						break;
					}
				}
			}

			*((uint32 *)&_globals[kWalkerTiming]) += 6;
		}
	}

	// Below is countdown to set the timebomb off in room 604
	if (_globals[kTimebombStatus] == TIMEBOMB_ACTIVATED) {
		int diff = _scene._frameStartTime - *((uint32 *)&_globals[kTimebombClock]);
		if ((diff >= 0) && (diff <= 60)) {
			*((uint32 *)&_globals[kTimebombTimer]) += diff;
		} else {
			++*((uint32 *)&_globals[kTimebombTimer]);
		}
		*((uint32 *)&_globals[kTimebombClock]) = _scene._frameStartTime;
	}
}

} // End of namespace Nebular

} // End of namespace MADS
