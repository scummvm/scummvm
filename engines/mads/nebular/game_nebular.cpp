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
#include "mads/nebular/nebular_scenes.h"

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

	Player::preloadSequences("RXM", 1);
	Player::preloadSequences("ROX", 1);
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
		_player.releasePlayerSprites();
		_vm->_dialogs->showDialog();
		_vm->_dialogs->_pendingDialog = DIALOG_NONE;
	}
}

void GameNebular::doObjectAction() {
	Scene &scene = _scene;
	MADSAction &action = _scene._action;
	Dialogs &dialogs = *_vm->_dialogs;
	int id;

	if (action.isAction(NOUN_SMELL) && scene._currentSceneId > 103 && scene._currentSceneId < 111) {
		dialogs.show(440);
	} else if (action.isAction(NOUN_EAT) && scene._currentSceneId > 103 && scene._currentSceneId < 111) {
		dialogs.show(441);
	} else if (action.isAction(NOUN_SMELL, NOUN_BURGER)) {
		dialogs.show(442);
	} else if (action.isAction(NOUN_EAT, NOUN_BURGER)) {
		dialogs.show(443);
	} else if (action.isAction(NOUN_SMELL, NOUN_STUFFED_FISH)) {
		dialogs.show(444);
	} else if (action.isAction(NOUN_EAT, NOUN_STUFFED_FISH)) {
		dialogs.show(445);
	} else if (action.isAction(401, 289)) {
		dialogs.show(scene._currentSceneId > 103 && scene._currentSceneId < 111 ? 446 : 447);
	} else if (action.isAction(306, 371)) {
		dialogs.show(448);
	} else if (action.isAction(951, 35)) {
		dialogs.show(449);
	} else if (action.isAction(203, 276)) {
		dialogs.show(450);
	} else if (action.isAction(NOUN_EAT, 378)) {
		_objects.setRoom(OBJ_TWINKIFRUIT, PLAYER_INVENTORY);
		dialogs.show(451);
	} else if (action.isAction(153, 378)) {
		_objects.setRoom(OBJ_TWINKIFRUIT, PLAYER_INVENTORY);
		dialogs.show(452);
	} else if (action.isAction(152)) {
		dialogs.show(453);
	} else if (action.isAction(1187, OBJ_PHONE_CELLS)) {
		dialogs.show(454);
	} else if (action.isAction(1188, 92)) {
		dialogs.show(455);
	} else if (action.isAction(140, 72)) {
		dialogs.show(456);
	} else if (action.isAction(1189, OBJ_VASE)) {
		dialogs.show(457);
	} else if (action.isAction(306, 369)) {
		dialogs.show(458);
	} else if (action.isAction(1192, 774)) {
		dialogs.show(459);
	} else if (action.isAction(1191, 359)) {
		dialogs.show(460);
	} else if (action.isAction(1190)) {
		dialogs.show(461);
	} else if (action.isAction(934, 277)) {
		dialogs.show(462);
	} else if (action.isAction(329, 277)) {
		dialogs.show(465);
	} else if (action.isAction(368, 135)) {
		dialogs.show(463);
	} else if (action.isAction(25, 135)) {
		dialogs.show(463);
	} else if (action.isAction(379)) {
		dialogs.show(464);
	} else if (action.isAction(869)) {
		dialogs.show(466);
	} 	else if (action.isAction(530, 288)) {
		dialogs.show(467);
	} else if (action.isAction(NOUN_EAT, 74)) {
		dialogs.show(469);
	} else if (action.isAction(50, 381)) {
		dialogs.show(471);
	} else if (action.isAction(307, 950)) {
		dialogs.show(472);
	} else if (action.isAction(NOUN_READ, NOUN_LOG)) {
		dialogs.show(473);
	} else if (action.isAction(1189, 43)) {
		dialogs.show(474);
	} else if (action.isAction(114, 726)) {
		dialogs.show(475);
	} else if (action.isAction(114, 727)) {
		dialogs.show(476);
	} else if (action.isAction(114, 937)) {
		dialogs.show(477);
	} else if (action.isAction(VERB_PUT, 276, NOUN_PLANT_STALK) && _objects.isInRoom(OBJ_POISON_DARTS)
			&& _objects.isInRoom(OBJ_PLANT_STALK)) {
		_objects.addToInventory(OBJ_BLOWGUN);
		_objects.setRoom(OBJ_PLANT_STALK, PLAYER_INVENTORY);
		_globals[kBlowgunStatus] = 0;
		dialogs.showPicture(OBJ_BLOWGUN, 809);
	} else if (action.isAction(VERB_PUT, 276, NOUN_BLOWGUN) && _objects.isInInventory(OBJ_POISON_DARTS)
			&& _objects.isInInventory(OBJ_BLOWGUN)) {
		dialogs.show(433);
	} else if (action.isAction(1161) && action.isAction(1195) && action.isAction(1197)) {
		dialogs.show(434);
	} else if (action.isAction(1196)) {
		dialogs.show(479);
	} else if ((action.isAction(287) || action.isAction(NOUN_LOOK_AT) || action.isAction(VERB_LOOK)) &&
			action.isAction(936) && _objects.isInInventory(OBJ_NOTE)) {
		_objects.setRoom(OBJ_NOTE, PLAYER_INVENTORY);
		_objects.addToInventory(OBJ_COMBINATION);
		dialogs.showPicture(OBJ_COMBINATION, 851);
	} else if ((action.isAction(VERB_LOOK) || action.isAction(NOUN_READ)) &&
			((id = _objects.getIdFromDesc(action._activeAction._objectNameId)) > 0 ||
			(action._activeAction._indirectObjectId > 0 &&
			(id = _objects.getIdFromDesc(action._activeAction._indirectObjectId))))) {
		if (id == OBJ_REPAIR_LIST) {
			dialogs._indexList[0] = _globals[kTeleporterCode + 7];
			dialogs._indexList[1] = _globals[kTeleporterCode + 8];
			dialogs._indexList[2] = _globals[kTeleporterCode + 6];
			dialogs._indexList[3] = _globals[kTeleporterCode + 9];
			dialogs._indexList[4] = _globals[kTeleporterCode + 0];
			dialogs._indexList[5] = _globals[kTeleporterCode + 1];
			dialogs._indexList[6] = _globals[kTeleporterCode + 4];
			dialogs._indexList[7] = _globals[kTeleporterCode + 5];
			dialogs._indexList[8] = _globals[kTeleporterCode + 2];

			dialogs.showPicture(id, 402);
		} else {
			int messageId = 800 + id;
			if (id == OBJ_CHARGE_CASES) {
				error("TODO: object_get_folder - loc_29B48");
				messageId = 860;
			}
			
			if (id == OBJ_TAPE_PLAYER && _objects[OBJ_AUDIO_TAPE]._roomNumber == OBJ_TAPE_PLAYER)
				messageId = 867;

			if (id == 32 && _objects[OBJ_FISHING_LINE]._roomNumber == 3)
				messageId = 862;

			if (id == OBJ_BOTTLE && _globals[kBottleStatus] != 0)
				messageId = 862 + _globals[kBottleStatus];

			if (id == OBJ_PHONE_HANDSET && _globals[kHandsetCellStatus])
				messageId = 861;

			dialogs.showPicture(id, messageId);
		}
	} else if (action.isAction(VERB_PUT, NOUN_BURGER, NOUN_DEAD_FISH)) {
		if (_objects.isInInventory(OBJ_BURGER) || _objects.isInInventory(OBJ_DEAD_FISH)) {
			_objects.removeFromInventory(OBJ_DEAD_FISH, PLAYER_INVENTORY);
			_objects.removeFromInventory(OBJ_BURGER, PLAYER_INVENTORY);
			_objects.addToInventory(OBJ_STUFFED_FISH);
			dialogs.showPicture(OBJ_STUFFED_FISH, 803);
		}
	} else if (action.isAction(VERB_PUT, 26, 357) && _objects.isInInventory(OBJ_AUDIO_TAPE) &&
			_objects.isInInventory(OBJ_TAPE_PLAYER)) {
		_objects.setRoom(OBJ_AUDIO_TAPE, 25);
	} else if (action.isAction(118, 357) && _objects.isInInventory(OBJ_TAPE_PLAYER)) {
		if (_objects[OBJ_AUDIO_TAPE]._roomNumber == 25) {
			_objects.addToInventory(OBJ_AUDIO_TAPE);
		} else {
			dialogs.show(407);
		}
	} else if (action.isAction(108, 357)) {
		dialogs.show(408);
	} else if (action.isAction(14, 291)) {
		dialogs.show(_globals[kTopButtonPushed] ? 502 : 501);
	} else if ((action.isAction(25, 106, 72) || action.isAction(VERB_PUT, 106, 72)) &&
			_objects.isInInventory(OBJ_DETONATORS) && _objects.isInInventory(OBJ_CHARGE_CASES)) {
		error("TODO: loc_29DBA");
	} else if (action.isAction(25, 106)) {
		dialogs.show(470);
	} else if ((action.isAction(25, 371, 43) || action.isAction(7, 371, 43) || action.isAction(25, 371, 42)
			|| action.isAction(VERB_PUT, 371, 42)) && _objects.isInInventory(OBJ_TIMER_MODULE) && (
			_objects.isInInventory(OBJ_BOMBS) || _objects.isInInventory(OBJ_BOMB))) {
		if (_objects.isInInventory(OBJ_BOMBS)) {
			_objects.setRoom(OBJ_BOMBS, PLAYER_INVENTORY);
			_objects.addToInventory(OBJ_BOMB);
		} else {
			_objects.setRoom(OBJ_BOMB, PLAYER_INVENTORY);
		}

		_objects.setRoom(OBJ_TIMER_MODULE, PLAYER_INVENTORY);
		_objects.addToInventory(OBJ_TIMEBOMB);
		dialogs.showPicture(OBJ_TIMEBOMB, 404);
	} else if (action.isAction(140, 271)) {
		dialogs.show(410);
	} else if (action.isAction(119, 46)) {
		_globals[kBottleStatus] = 0;
		dialogs.show(432);
	} else if (action.isAction(108, 136)) {
		if (_objects[OBJ_FISHING_LINE]._roomNumber == 3) {
			_objects.addToInventory(OBJ_FISHING_LINE);
			dialogs.showPicture(OBJ_FISHING_LINE, 409);
		} else {
			dialogs.show(428);
		}
	} else if (action.isAction(108, 262)) {
		switch (_globals[kPenlightCellStatus]) {
		case 1:
		case 2:
			_objects.addToInventory(OBJ_DURAFAIL_CELLS);
			dialogs.showPicture(OBJ_DURAFAIL_CELLS, 412);
			break;
		case 3:
			_objects.addToInventory(OBJ_PHONE_CELLS);
			dialogs.showPicture(OBJ_DURAFAIL_CELLS, 413);
			break;
		case 5:
			_objects.addToInventory(OBJ_DURAFAIL_CELLS);
			dialogs.showPicture(OBJ_DURAFAIL_CELLS, 411);
			break;
		case 6:
			_objects.addToInventory(OBJ_DURAFAIL_CELLS);
			dialogs.showPicture(OBJ_DURAFAIL_CELLS, 429);
			break;
		default:
			dialogs.show(478);
			break;
		}
	} else if (action.isAction(108, 264)) {
		switch (_globals[kHandsetCellStatus]) {
		case 1:
			_objects.addToInventory(OBJ_DURAFAIL_CELLS);
			dialogs.showPicture(OBJ_DURAFAIL_CELLS,
				_difficulty != 1 || _globals[kDurafailRecharged] ? 415 : 414);
			break;
		case 2:
			_objects.addToInventory(OBJ_DURAFAIL_CELLS);
			if (_difficulty == 1) {
				dialogs.showPicture(OBJ_DURAFAIL_CELLS, 416);
			} else {
				_globals[kHandsetCellStatus] = 0;
			}
			break;
		case 3:
			_objects.addToInventory(OBJ_PHONE_CELLS);
			dialogs.showPicture(OBJ_PHONE_CELLS, 418);
			break;
		case 4:
			_objects.addToInventory(OBJ_PHONE_CELLS);
			dialogs.showPicture(OBJ_PHONE_CELLS, 417);
			break;
		default:
			dialogs.show(478);
			break;
		}
	} else if (action.isAction(VERB_PUT, 263, 262)) {
		if (_globals[kPenlightCellStatus] == 0) {
			_globals[kPenlightCellStatus] = 3;
			_objects.setRoom(OBJ_PHONE_CELLS, PLAYER_INVENTORY);
			dialogs.show(419);
		} else {
			dialogs.show(420);
		}
	} else if (action.isAction(VERB_PUT, 263, 264)) {
		if (_globals[kHandsetCellStatus] == 0) {
			_globals[kHandsetCellStatus] = 3;
			_objects.setRoom(OBJ_PHONE_CELLS, PLAYER_INVENTORY);
			dialogs.show(421);
		}
		else {
			dialogs.show(422);
		}
	} else if (action.isAction(VERB_PUT, 115, 262)) {
		if (_globals[kPenlightCellStatus]) {
			dialogs.show(424);
		} else {
			_objects.setRoom(OBJ_DURAFAIL_CELLS, PLAYER_INVENTORY);
			_globals[kPenlightCellStatus] = _difficulty != 1 || _globals[kDurafailRecharged] ? 1 : 2;
			dialogs.show(423);
		}
	} else if (action.isAction(VERB_PUT, 115, 264)) {
		if (_globals[kHandsetCellStatus]) {
			dialogs.show(424);
		} else {
			_objects.setRoom(OBJ_DURAFAIL_CELLS, PLAYER_INVENTORY);
			_globals[kDurafailRecharged] = _difficulty != 1 || _globals[kHandsetCellStatus] ? 1 : 2;
			dialogs.show(425);
		}
	} else if (action.isAction(306, 369)) {
		dialogs.show(427);
	} else if (action.isAction(VERB_PUT, 42, 73) || action.isAction(VERB_PUT, 43, 73)) {
		_objects.setRoom(OBJ_CHICKEN, PLAYER_INVENTORY);
		if (_objects.isInInventory(OBJ_BOMBS)) {
			_objects.setRoom(OBJ_BOMBS, PLAYER_INVENTORY);
			_objects.addToInventory(OBJ_BOMB);
		} else {
			_objects.setRoom(OBJ_BOMB, PLAYER_INVENTORY);
		}

		_objects.addToInventory(OBJ_CHICKEN_BOMB);
		dialogs.showPicture(OBJ_CHICKEN_BOMB, 430);
	} else {
		return;
	}

	action._inProgress = false;
}

void GameNebular::unhandledAction() {
	int randVal = _vm->getRandomNumber(1, 1000);
	MADSAction &action = _scene._action;

	if (action.isAction(VERB_THROW, NOUN_BOMB) || action.isAction(VERB_THROW, NOUN_BOMBS)
	|| action.isAction(VERB_THROW, NOUN_TIMEBOMB) || action.isAction(VERB_THROW, NOUN_CHICKEN_BOMB))
		_vm->_dialogs->show(0x2A);
	else if (action.isAction(0x6C))
		_vm->_dialogs->show(0x1B3);
	else if ((action.isAction(NOUN_EAT, NOUN_DEAD_FISH) || action.isAction(NOUN_EAT, NOUN_STUFFED_FISH)) && _vm->_game->_objects.isInInventory(_vm->_game->_objects.getIdFromDesc(action._activeAction._objectNameId)))
		_vm->_dialogs->show(0xC);
	else if ((action.isAction(NOUN_SMELL, NOUN_DEAD_FISH) || action.isAction(NOUN_SMELL, NOUN_STUFFED_FISH)) && _vm->_game->_objects.isInInventory(_vm->_game->_objects.getIdFromDesc(action._activeAction._objectNameId)))
		_vm->_dialogs->show(0xD);
	else if (action.isAction(NOUN_EAT, NOUN_CHICKEN) && _vm->_game->_objects.isInInventory(OBJ_CHICKEN))
		_vm->_dialogs->show(0x390);
	else if ((action.isAction(NOUN_SHOOT) || action.isAction(NOUN_HOSE_DOWN)) && action.isAction(NOUN_BLOWGUN)) {
		if ((_scene._currentSceneId >= 104) && (_scene._currentSceneId <= 111))
			_vm->_dialogs->show(0x26);
		else if (action.isAction(0x10D))
			_vm->_dialogs->show(0x29);
		else if (action.isAction(NOUN_CHICKEN) || action.isAction(0x185) || action.isAction(0x14D)
				|| action.isAction(0x1DD) || action.isAction(0x15F) || action.isAction(NOUN_CAPTIVE_CREATURE)) {
			_vm->_dialogs->show(0x28);
		} else
			_vm->_dialogs->show(0x27);
	} else if (action.isAction(VERB_TALKTO)) {
		_globals[kTalkInanimateCount] = (_globals[kTalkInanimateCount] + 1) % 16;
		if (!_globals[kTalkInanimateCount]) {
			_vm->_dialogs->show(0x2);
		} else {
			Common::String tmpMsg = "\"Greetings, ";
			tmpMsg += _vm->_game->_scene.getVocab(action._activeAction._objectNameId);
			tmpMsg += "!\"";
			_scene._kernelMessages.reset();
			_scene._kernelMessages.add(Common::Point(0, 0), 0x1110, 34, 0, 120, tmpMsg);
		}
	} else if (action.isAction(VERB_GIVE, NOUN_DOOR, 0x46) || action.isAction(VERB_CLOSE, NOUN_CHAIR))
		_vm->_dialogs->show(0x3);
	else if (action.isAction(VERB_THROW)) {
		int objId = _vm->_game->_objects.getIdFromDesc(action._activeAction._objectNameId);
		if (objId < 0)
			_vm->_dialogs->show(0x4);
		else if (_vm->_game->_objects[objId]._roomNumber != 2)
			_vm->_dialogs->show(0x5);
		else
			_vm->_dialogs->show(0x6);
	} else if (action.isAction(VERB_LOOK)) {
		if (action.isAction(0x27) && (action._activeAction._indirectObjectId > 0))
			_vm->_dialogs->show(0xA);
		else if (randVal < 600)
			_vm->_dialogs->show(0x7);
		else
			_vm->_dialogs->show(0x15);
	} else if (action.isAction(VERB_TAKE)) {
		int objId = _vm->_game->_objects.getIdFromDesc(action._activeAction._objectNameId);
		if (_vm->_game->_objects.isInInventory(objId))
			_vm->_dialogs->show(0x10);
		else if (randVal <= 333)
			_vm->_dialogs->show(0x8);
		else if (randVal <= 666)
			_vm->_dialogs->show(0x16);
		else
			_vm->_dialogs->show(0x17);
	} else if (action.isAction(VERB_CLOSE)) {
		if (randVal <= 333)
			_vm->_dialogs->show(0x9);
		else
			_vm->_dialogs->show(0x21);
	} else if (action.isAction(VERB_OPEN)) {
		if (randVal <= 500)
			_vm->_dialogs->show(0x1E);
		else if (randVal <= 750)
			_vm->_dialogs->show(0x1F);
		else
			_vm->_dialogs->show(0x20);
	} else if (action.isAction(VERB_PULL))
		_vm->_dialogs->show(0x12);
	else if (action.isAction(VERB_PUSH)) {
		if (randVal < 750)
			_vm->_dialogs->show(0x13);
		else
			_vm->_dialogs->show(0x14);
	} else if (action.isAction(VERB_PUT)) {
		int objId = _vm->_game->_objects.getIdFromDesc(action._activeAction._objectNameId);
		if (_vm->_game->_objects.isInInventory(objId))
			_vm->_dialogs->show(0x19);
		else
			_vm->_dialogs->show(0x18);
	} else if (action.isAction(VERB_GIVE)) {
		int objId = _vm->_game->_objects.getIdFromDesc(action._activeAction._objectNameId);
		if (!_vm->_game->_objects.isInInventory(objId))
			_vm->_dialogs->show(0x1A);
		else if (randVal <= 500)
			_vm->_dialogs->show(0x1C);
		else
			_vm->_dialogs->show(0x1D);
	} else if (!action.isAction(VERB_WALKTO) && !action.isAction(0x187) && !action.isAction(0x18C) && !action.isAction(0x1AD)
			&& !action.isAction(0x15C) && !action.isAction(0x159) && !action.isAction(0x15A) && !action.isAction(0x15B)
			&& !action.isAction(0x15E)) {
		if (randVal <= 100)
			_vm->_dialogs->show(0x24);
		else if (randVal <= 200)
			_vm->_dialogs->show(0x1);
		else if (randVal <= 475)
			_vm->_dialogs->show(0x22);
		else if (randVal <= 750)
			_vm->_dialogs->show(0x23);
		else
			_vm->_dialogs->show(0x25);
	}
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
