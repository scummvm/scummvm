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

#include "common/scummsys.h"
#include "common/config-manager.h"
#include "graphics/scaler.h"
#include "mads/mads.h"
#include "mads/game.h"
#include "mads/screen.h"
#include "mads/msurface.h"
#include "mads/menu_views.h"
#include "mads/nebular/game_nebular.h"
#include "mads/nebular/dialogs_nebular.h"
#include "mads/nebular/globals_nebular.h"
#include "mads/nebular/nebular_scenes.h"

namespace MADS {

namespace Nebular {

GameNebular::GameNebular(MADSEngine *vm)
	: Game(vm) {
	_surface = new MSurface(MADS_SCREEN_WIDTH, MADS_SCENE_HEIGHT);
	_storyMode = STORYMODE_NAUGHTY;
	_difficulty = DIFFICULTY_HARD;
}

ProtectionResult GameNebular::checkCopyProtection() {
	// Only show copy protection dialog if explicitly wanted
	if (!ConfMan.getBool("copy_protection"))
		return PROTECTION_SUCCEED;

	CopyProtectionDialog *dlg;
	bool correctAnswer;

	dlg = new CopyProtectionDialog(_vm, false);
	dlg->show();
	correctAnswer = dlg->isCorrectAnswer();
	delete dlg;

	if (!correctAnswer && !_vm->shouldQuit()) {
		dlg = new CopyProtectionDialog(_vm, true);
		dlg->show();
		correctAnswer = dlg->isCorrectAnswer();
		delete dlg;
	}

	return correctAnswer ? PROTECTION_SUCCEED : PROTECTION_FAIL;
}

void GameNebular::startGame() {
	// First handle any ending credits from a just finished game session.
	// Note that, with the exception of the decompression ending, which doesn't
	// use animations, the remaining animations will automatically launch their
	// own text view credits when the animation is completed
	switch (_winStatus) {
	case 1:
		// No shields failure ending
		AnimationView::execute(_vm, "rexend1");
		break;
	case 2:
		// Shields, but no targeting failure ending
		AnimationView::execute(_vm, "rexend2");
		break;
	case 3:
		// Completed game successfully, so activate quotes item on the main menu
		ConfMan.setBool("ShowQuotes", true);
		ConfMan.flushToDisk();

		AnimationView::execute(_vm, "rexend3");
		break;
	case 4:
		// Decompression ending
		TextView::execute(_vm, "ending4");
		break;
	default:
		break;
	}

	do {
		checkShowDialog();
		_winStatus = 0;

		_sectionNumber = 1;
		initSection(_sectionNumber);
		_vm->_events->setCursor(CURSOR_ARROW);
		_statusFlag = true;

		// Show the main menu
		_vm->_dialogs->_pendingDialog = DIALOG_MAIN_MENU;
		_vm->_dialogs->showDialog();
	} while (!_vm->shouldQuit() && _vm->_dialogs->_pendingDialog != DIALOG_NONE);

	if (_vm->shouldQuit())
		return;

	_priorSectionNumber = 0;
	_priorSectionNumber = -1;
	_scene._priorSceneId = 0;
	_scene._currentSceneId = -1;
	_scene._nextSceneId = 101;

	initializeGlobals();

	if (_loadGameSlot >= 0)
		// User selected to resume a savegame
		return;

	// Check copy protection
	ProtectionResult protectionResult = checkCopyProtection();

	switch (protectionResult) {
	case PROTECTION_FAIL:
		// Copy protection failed
		_scene._nextSceneId = 804;
		_globals[kCopyProtectFailed] = true;
		return;
	case PROTECTION_ESCAPE:
		// User escaped out of copy protection dialog
		_vm->quitGame();
		return;
	default:
		// Copy protection check succeeded
		break;
	}
}

void GameNebular::initializeGlobals() {
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
	_objects[OBJ_CHARGE_CASES].setQuality(EXPLOSIVES_INSIDE, 0);
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
	default:
		_objects.setRoom(OBJ_BLOWGUN, NOWHERE);
		_objects.setRoom(OBJ_NOTE, NOWHERE);

		_globals[kLeavesStatus] = LEAVES_ON_GROUND;
		_globals[kDurafailRecharged] = 0;
		_globals[kPenlightCellStatus] = FIRST_TIME_UNCHARGED_DURAFAIL;
		break;

	case DIFFICULTY_MEDIUM:
		_objects.setRoom(OBJ_PLANT_STALK, NOWHERE);

		_globals[kLeavesStatus] = LEAVES_ON_GROUND;
		_globals[kDurafailRecharged] = 1;
		_globals[kPenlightCellStatus] = FIRST_TIME_CHARGED_DURAFAIL;
		break;

	case DIFFICULTY_EASY:
		_objects.setRoom(OBJ_PLANT_STALK, NOWHERE);
		_objects.setRoom(OBJ_PENLIGHT, NOWHERE);

		_globals[kLeavesStatus] = LEAVES_ON_TRAP;
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
	// Loop for showing dialogs, if any need to be shown
	if (_vm->_dialogs->_pendingDialog && (_player._stepEnabled || _winStatus)
			&& !_globals[kCopyProtectFailed]) {
		_player.releasePlayerSprites();

		// Make a thumbnail in case it's needed for making a savegame
		_vm->_game->createThumbnail();

		// Show the dialog
		_vm->_dialogs->showDialog();
		_vm->_dialogs->_pendingDialog = DIALOG_NONE;
	}
}

void GameNebular::showRecipe() {
	Dialogs &dialogs = *_vm->_dialogs;
	int count;

	for (count = 0; count < 4; count++) {
		switch(_globals[kIngredientQuantity + count]) {
		case 0:
			dialogs._indexList[count] = NOUN_DROP;
			break;
		case 1:
			dialogs._indexList[count] = NOUN_DOLLOP;
			break;
		case 2:
			dialogs._indexList[count] = NOUN_DASH;
			break;
		case 3:
			dialogs._indexList[count] = NOUN_SPLASH;
			break;
		default:
			break;
		}
	}

	for (count = 0; count < 4; count++) {
		switch(_globals[kIngredientList + count]) {
		case 0:
			dialogs._indexList[count + 4] = NOUN_ALCOHOL;
			break;
		case 1:
			dialogs._indexList[count + 4] = NOUN_LECITHIN;
			break;
		case 2:
			dialogs._indexList[count + 4] = NOUN_PETROX;
			break;
		case 3:
			dialogs._indexList[count + 4] = NOUN_FORMALDEHYDE;
			break;
		default:
			break;
		}
	}

	_vm->_dialogs->show(401);
}

void GameNebular::doObjectAction() {
	Scene &scene = _scene;
	MADSAction &action = _scene._action;
	Dialogs &dialogs = *_vm->_dialogs;
	int id;

	if (action.isAction(VERB_SMELL) && scene._currentSceneId > 103 && scene._currentSceneId < 111) {
		dialogs.show(440);
	} else if (action.isAction(VERB_EAT) && scene._currentSceneId > 103 && scene._currentSceneId < 111) {
		dialogs.show(441);
	} else if (action.isAction(VERB_SMELL, NOUN_BURGER)) {
		dialogs.show(442);
	} else if (action.isAction(VERB_EAT, NOUN_BURGER)) {
		dialogs.show(443);
	} else if (action.isAction(VERB_SMELL, NOUN_STUFFED_FISH)) {
		dialogs.show(444);
	} else if (action.isAction(VERB_EAT, NOUN_STUFFED_FISH)) {
		dialogs.show(445);
	} else if (action.isAction(VERB_WEAR, NOUN_REBREATHER)) {
		dialogs.show(scene._currentSceneId > 103 && scene._currentSceneId < 111 ? 446 : 447);
	} else if (action.isAction(VERB_SET, NOUN_TIMER_MODULE)) {
		dialogs.show(448);
	} else if (action.isAction(VERB_NIBBLE_ON, NOUN_BIG_LEAVES)) {
		dialogs.show(449);
	} else if (action.isAction(VERB_LICK, NOUN_POISON_DARTS)) {
		dialogs.show(450);
	} else if (action.isAction(VERB_EAT, NOUN_TWINKIFRUIT)) {
		_objects.setRoom(OBJ_TWINKIFRUIT, NOWHERE);
		dialogs.show(451);
	} else if (action.isAction(VERB_GORGE_ON, NOUN_TWINKIFRUIT)) {
		_objects.setRoom(OBJ_TWINKIFRUIT, NOWHERE);
		dialogs.show(452);
	} else if (action.isAction(VERB_GNAW_ON)) {
		dialogs.show(453);
	} else if (action.isAction(VERB_MASSAGE, NOUN_AUDIO_TAPE)) {
		dialogs.show(454);
	} else if (action.isAction(VERB_MANGLE, NOUN_CREDIT_CHIP)) {
		dialogs.show(455);
	} else if (action.isAction(VERB_FONDLE, NOUN_CHARGE_CASES)) {
		dialogs.show(456);
	} else if (action.isAction(VERB_RUB, NOUN_BOMB)) {
		dialogs.show(457);
	} else if (action.isAction(VERB_SET, NOUN_TIMEBOMB)) {
		dialogs.show(458);
	} else if (action.isAction(VERB_GUZZLE, NOUN_ALIEN_LIQUOR)) {
		dialogs.show(459);
	} else if (action.isAction(VERB_SMASH, NOUN_TARGET_MODULE)) {
		dialogs.show(460);
	} else if (action.isAction(VERB_JUGGLE)) {
		dialogs.show(461);
	} else if (action.isAction(VERB_APPLY, NOUN_POLYCEMENT)) {
		dialogs.show(462);
	} else if (action.isAction(VERB_SNIFF, NOUN_POLYCEMENT)) {
		dialogs.show(465);
	} else if (action.isAction(VERB_TIE, NOUN_FISHING_LINE)) {
		dialogs.show(463);
	} else if (action.isAction(VERB_ATTACH, NOUN_FISHING_LINE)) {
		dialogs.show(463);
	} else if (action.isAction(VERB_UNLOCK)) {
		dialogs.show(464);
	} else if (action.isAction(VERB_REFLECT)) {
		dialogs.show(466);
	} else if (action.isAction(VERB_GAZE_INTO, NOUN_REARVIEW_MIRROR)) {
		dialogs.show(467);
	} else if (action.isAction(VERB_EAT, NOUN_CHICKEN_BOMB)) {
		dialogs.show(469);
	} else if (action.isAction(VERB_BREAK, NOUN_VASE)) {
		dialogs.show(471);
	} else if (action.isAction(VERB_SHAKE_HANDS, NOUN_GUARDS_ARM2)) {
		dialogs.show(472);
	} else if (action.isAction(VERB_READ, NOUN_LOG)) {
		dialogs.show(473);
	} else if (action.isAction(VERB_RUB, NOUN_BOMBS)) {
		dialogs.show(474);
	} else if (action.isAction(VERB_DRINK, NOUN_FORMALDEHYDE)) {
		dialogs.show(475);
	} else if (action.isAction(VERB_DRINK, NOUN_PETROX)) {
		dialogs.show(476);
	} else if (action.isAction(VERB_DRINK, NOUN_LECITHIN)) {
		dialogs.show(477);
	} else if (action.isAction(VERB_PUT, NOUN_POISON_DARTS, NOUN_PLANT_STALK) && _objects.isInInventory(OBJ_POISON_DARTS)
			&& _objects.isInInventory(OBJ_PLANT_STALK)) {
		_objects.addToInventory(OBJ_BLOWGUN);
		_objects.setRoom(OBJ_PLANT_STALK, NOWHERE);
		_globals[kBlowgunStatus] = 0;
		dialogs.showItem(OBJ_BLOWGUN, 809);
	} else if (action.isAction(VERB_PUT, NOUN_POISON_DARTS, NOUN_BLOWGUN) && _objects.isInInventory(OBJ_POISON_DARTS)
			&& _objects.isInInventory(OBJ_BLOWGUN)) {
		dialogs.show(433);
	} else if (action.isAction(VERB_DEFACE) && action.isAction(VERB_FOLD) && action.isAction(VERB_MUTILATE)) {
		dialogs.show(434);
	} else if (action.isAction(VERB_SPINDLE)) {
		dialogs.show(479);
	} else if ((action.isAction(VERB_READ) || action.isAction(VERB_LOOK_AT) || action.isAction(VERB_LOOK)) &&
			action.isObject(NOUN_NOTE) && _objects.isInInventory(OBJ_NOTE)) {
		_objects.setRoom(OBJ_NOTE, NOWHERE);
		_objects.addToInventory(OBJ_COMBINATION);
		dialogs.showItem(OBJ_COMBINATION, 851);
	} else if ((action.isAction(VERB_LOOK) || action.isAction(VERB_READ)) &&
			((id = _objects.getIdFromDesc(action._activeAction._objectNameId)) > 0 ||
			(action._activeAction._indirectObjectId > 0 &&
			(id = _objects.getIdFromDesc(action._activeAction._indirectObjectId)))) &&
			_objects.isInInventory(id)) {
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

			dialogs.showItem(id, 402);
		} else {
			int messageId = 800 + id;
			if ((id == OBJ_CHARGE_CASES) && _objects[OBJ_CHARGE_CASES].getQuality(3) != 0) {
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

			dialogs.showItem(id, messageId);
		}
	} else if (action.isAction(VERB_PUT, NOUN_BURGER, NOUN_DEAD_FISH)) {
		if (_objects.isInInventory(OBJ_BURGER) || _objects.isInInventory(OBJ_DEAD_FISH)) {
			_objects.removeFromInventory(OBJ_DEAD_FISH, PLAYER_INVENTORY);
			_objects.removeFromInventory(OBJ_BURGER, PLAYER_INVENTORY);
			_objects.addToInventory(OBJ_STUFFED_FISH);
			dialogs.showItem(OBJ_STUFFED_FISH, 803);
		}
	} else if (action.isAction(VERB_PUT, NOUN_AUDIO_TAPE, NOUN_TAPE_PLAYER) && _objects.isInInventory(OBJ_AUDIO_TAPE) &&
			_objects.isInInventory(OBJ_TAPE_PLAYER)) {
		_objects.setRoom(OBJ_AUDIO_TAPE, OBJ_TAPE_PLAYER);
	} else if (action.isAction(VERB_ACTIVATE, NOUN_TAPE_PLAYER) && _objects.isInInventory(OBJ_TAPE_PLAYER)) {
		if (_objects[OBJ_AUDIO_TAPE]._roomNumber == OBJ_TAPE_PLAYER) {
			showRecipe();
		} else {
			dialogs.show(406);
		}
	} else if (action.isAction(VERB_EJECT, NOUN_TAPE_PLAYER) && _objects.isInInventory(OBJ_TAPE_PLAYER)) {
		if (_objects[OBJ_AUDIO_TAPE]._roomNumber == OBJ_TAPE_PLAYER) {
			_objects.addToInventory(OBJ_AUDIO_TAPE);
		} else {
			dialogs.show(407);
		}
	} else if (action.isAction(VERB_DISASSEMBLE, NOUN_TAPE_PLAYER)) {
		dialogs.show(408);
	} else if (action.isAction(VERB_ACTIVATE, NOUN_REMOTE)) {
		dialogs.show(_globals[kTopButtonPushed] ? 502 : 501);
	} else if ((action.isAction(VERB_ATTACH, NOUN_DETONATORS, NOUN_CHARGE_CASES) || action.isAction(VERB_PUT, NOUN_DETONATORS, NOUN_CHARGE_CASES)) &&
			_objects.isInInventory(OBJ_DETONATORS) && _objects.isInInventory(OBJ_CHARGE_CASES)) {
		if (_objects[OBJ_CHARGE_CASES].getQuality(3)) {
			_objects.setRoom(OBJ_CHARGE_CASES, NOWHERE);
			_objects.setRoom(OBJ_DETONATORS, NOWHERE);
			_objects.addToInventory(OBJ_BOMBS);
			dialogs.showItem(OBJ_BOMBS, 403);
		} else {
			dialogs.show(405);
		}
	} else if (action.isAction(VERB_ATTACH, NOUN_DETONATORS)) {
		dialogs.show(470);
	} else if ((action.isAction(VERB_ATTACH, NOUN_TIMER_MODULE, NOUN_BOMBS) || action.isAction(VERB_PUT, NOUN_TIMER_MODULE, NOUN_BOMBS) || action.isAction(VERB_ATTACH, NOUN_TIMER_MODULE, NOUN_BOMB)
			|| action.isAction(VERB_PUT, NOUN_TIMER_MODULE, NOUN_BOMB)) && _objects.isInInventory(OBJ_TIMER_MODULE) && (
			_objects.isInInventory(OBJ_BOMBS) || _objects.isInInventory(OBJ_BOMB))) {
		if (_objects.isInInventory(OBJ_BOMBS)) {
			_objects.setRoom(OBJ_BOMBS, NOWHERE);
			_objects.addToInventory(OBJ_BOMB);
		} else {
			_objects.setRoom(OBJ_BOMB, NOWHERE);
		}

		_objects.setRoom(OBJ_TIMER_MODULE, NOWHERE);
		_objects.addToInventory(OBJ_TIMEBOMB);
		dialogs.showItem(OBJ_TIMEBOMB, 404);
	} else if (action.isAction(VERB_FONDLE, NOUN_PLANT_STALK)) {
		dialogs.show(410);
	} else if (action.isAction(VERB_EMPTY, NOUN_BOTTLE)) {
		_globals[kBottleStatus] = 0;
		dialogs.show(432);
	} else if (action.isAction(VERB_DISASSEMBLE, NOUN_FISHING_ROD)) {
		if (_objects[OBJ_FISHING_LINE]._roomNumber == 3) {
			_objects.addToInventory(OBJ_FISHING_LINE);
			dialogs.showItem(OBJ_FISHING_LINE, 409);
		} else {
			dialogs.show(428);
		}
	} else if (action.isAction(VERB_DISASSEMBLE, NOUN_PENLIGHT)) {
		switch (_globals[kPenlightCellStatus]) {
		case 1:
		case 2:
			_objects.addToInventory(OBJ_DURAFAIL_CELLS);
			dialogs.showItem(OBJ_DURAFAIL_CELLS, 412);
			break;
		case 3:
			_objects.addToInventory(OBJ_PHONE_CELLS);
			dialogs.showItem(OBJ_DURAFAIL_CELLS, 413);
			break;
		case 5:
			_objects.addToInventory(OBJ_DURAFAIL_CELLS);
			dialogs.showItem(OBJ_DURAFAIL_CELLS, 411);
			break;
		case 6:
			_objects.addToInventory(OBJ_DURAFAIL_CELLS);
			dialogs.showItem(OBJ_DURAFAIL_CELLS, 429);
			break;
		default:
			dialogs.show(478);
			break;
		}
	} else if (action.isAction(VERB_DISASSEMBLE, NOUN_PHONE_HANDSET)) {
		switch (_globals[kHandsetCellStatus]) {
		case 1:
			_objects.addToInventory(OBJ_DURAFAIL_CELLS);
			dialogs.showItem(OBJ_DURAFAIL_CELLS,
				_difficulty != DIFFICULTY_HARD || _globals[kDurafailRecharged] ? 415 : 414);
			_globals[kDurafailRecharged] = true;
			break;
		case 2:
			_objects.addToInventory(OBJ_DURAFAIL_CELLS);
			if (_difficulty == DIFFICULTY_HARD) {
				dialogs.showItem(OBJ_DURAFAIL_CELLS, 416);
			}
			_globals[kHandsetCellStatus] = 0;
			break;
		case 3:
			_objects.addToInventory(OBJ_PHONE_CELLS);
			dialogs.showItem(OBJ_PHONE_CELLS, 418);
			break;
		case 4:
			_objects.addToInventory(OBJ_PHONE_CELLS);
			dialogs.showItem(OBJ_PHONE_CELLS, 417);
			_globals[kHandsetCellStatus] = 0;
			break;
		default:
			dialogs.show(478);
			break;
		}
	} else if (action.isAction(VERB_PUT, NOUN_PHONE_CELLS, NOUN_PENLIGHT)) {
		if (_globals[kPenlightCellStatus] == 0) {
			_globals[kPenlightCellStatus] = 3;
			_objects.setRoom(OBJ_PHONE_CELLS, NOWHERE);
			dialogs.show(419);
		} else {
			dialogs.show(420);
		}
	} else if (action.isAction(VERB_PUT, NOUN_PHONE_CELLS, NOUN_PHONE_HANDSET)) {
		if (_globals[kHandsetCellStatus] == 0) {
			_globals[kHandsetCellStatus] = 3;
			_objects.setRoom(OBJ_PHONE_CELLS, NOWHERE);
			dialogs.show(421);
		} else {
			dialogs.show(422);
		}
	} else if (action.isAction(VERB_PUT, NOUN_DURAFAIL_CELLS, NOUN_PENLIGHT)) {
		if (_globals[kPenlightCellStatus]) {
			dialogs.show(424);
		} else {
			_objects.setRoom(OBJ_DURAFAIL_CELLS, NOWHERE);
			_globals[kPenlightCellStatus] = _difficulty != DIFFICULTY_HARD || _globals[kDurafailRecharged] ? 1 : 2;
			dialogs.show(423);
		}
	} else if (action.isAction(VERB_PUT, NOUN_DURAFAIL_CELLS, NOUN_PHONE_HANDSET)) {
		if (_globals[kHandsetCellStatus]) {
			dialogs.show(426);
		} else {
			_objects.setRoom(OBJ_DURAFAIL_CELLS, NOWHERE);
			_globals[kHandsetCellStatus] = _difficulty != DIFFICULTY_HARD || _globals[kHandsetCellStatus] ? 1 : 2;
			dialogs.show(425);
		}
	} else if (action.isAction(VERB_PUT, NOUN_BOMB, NOUN_CHICKEN) || action.isAction(VERB_PUT, NOUN_BOMBS, NOUN_CHICKEN)) {
		_objects.setRoom(OBJ_CHICKEN, NOWHERE);
		if (_objects.isInInventory(OBJ_BOMBS)) {
			_objects.setRoom(OBJ_BOMBS, NOWHERE);
			_objects.addToInventory(OBJ_BOMB);
		} else {
			_objects.setRoom(OBJ_BOMB, NOWHERE);
		}

		_objects.addToInventory(OBJ_CHICKEN_BOMB);
		dialogs.showItem(OBJ_CHICKEN_BOMB, 430);
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
		_vm->_dialogs->show(42);
	else if (action.isAction(VERB_DISASSEMBLE))
		_vm->_dialogs->show(435);
	else if ((action.isAction(VERB_EAT, NOUN_DEAD_FISH) || action.isAction(VERB_EAT, NOUN_STUFFED_FISH)) && _vm->_game->_objects.isInInventory(_vm->_game->_objects.getIdFromDesc(action._activeAction._objectNameId)))
		_vm->_dialogs->show(12);
	else if ((action.isAction(VERB_SMELL, NOUN_DEAD_FISH) || action.isAction(VERB_SMELL, NOUN_STUFFED_FISH)) && _vm->_game->_objects.isInInventory(_vm->_game->_objects.getIdFromDesc(action._activeAction._objectNameId)))
		_vm->_dialogs->show(13);
	else if (action.isAction(VERB_EAT, NOUN_CHICKEN) && _vm->_game->_objects.isInInventory(OBJ_CHICKEN))
		_vm->_dialogs->show(912);
	else if ((action.isAction(VERB_SHOOT) || action.isAction(VERB_HOSE_DOWN)) && action.isObject(NOUN_BLOWGUN)) {
		if ((_scene._currentSceneId >= 104) && (_scene._currentSceneId <= 111))
			_vm->_dialogs->show(38);
		else if (action.isObject(NOUN_PIRANHA))
			_vm->_dialogs->show(41);
		else if (action.isObject(NOUN_CHICKEN) || action.isObject(NOUN_VULTURE) || action.isObject(NOUN_SPIDER)
				|| action.isObject(NOUN_YELLOW_BIRD) || action.isObject(NOUN_SWOOPING_CREATURE) || action.isObject(NOUN_CAPTIVE_CREATURE)) {
			_vm->_dialogs->show(40);
		} else
			_vm->_dialogs->show(39);
	} else if (action.isAction(VERB_TALKTO)) {
		_globals[kTalkInanimateCount] = (_globals[kTalkInanimateCount] + 1) % 16;
		if (!_globals[kTalkInanimateCount]) {
			_vm->_dialogs->show(2);
		} else {
			Common::String tmpMsg = "\"Greetings, ";
			tmpMsg += _vm->_game->_scene.getVocab(action._activeAction._objectNameId);
			tmpMsg += "!\"";
			_scene._kernelMessages.reset();
			_scene._kernelMessages.add(Common::Point(0, 0), 0x1110, 34, 0, 120, tmpMsg);
		}
	} else if (action.isAction(VERB_GIVE, NOUN_DOOR, NOUN_CEILING) || action.isAction(VERB_CLOSE, NOUN_CHAIR))
		_vm->_dialogs->show(3);
	else if (action.isAction(VERB_THROW)) {
		int objId = _vm->_game->_objects.getIdFromDesc(action._activeAction._objectNameId);
		if (objId < 0)
			_vm->_dialogs->show(4);
		else if (_vm->_game->_objects[objId]._roomNumber != 2)
			_vm->_dialogs->show(5);
		else
			_vm->_dialogs->show(6);
	} else if (action.isAction(VERB_LOOK)) {
		if (action.isObject(NOUN_BINOCULARS) && (action._activeAction._indirectObjectId > 0))
			_vm->_dialogs->show(10);
		else if (randVal < 600)
			_vm->_dialogs->show(7);
		else
			_vm->_dialogs->show(21);
	} else if (action.isAction(VERB_TAKE)) {
		int objId = _vm->_game->_objects.getIdFromDesc(action._activeAction._objectNameId);
		if (_vm->_game->_objects.isInInventory(objId))
			_vm->_dialogs->show(16);
		else if (randVal <= 333)
			_vm->_dialogs->show(8);
		else if (randVal <= 666)
			_vm->_dialogs->show(22);
		else
			_vm->_dialogs->show(23);
	} else if (action.isAction(VERB_CLOSE)) {
		if (randVal <= 333)
			_vm->_dialogs->show(9);
		else
			_vm->_dialogs->show(33);
	} else if (action.isAction(VERB_OPEN)) {
		if (randVal <= 500)
			_vm->_dialogs->show(30);
		else if (randVal <= 750)
			_vm->_dialogs->show(31);
		else
			_vm->_dialogs->show(32);
	} else if (action.isAction(VERB_PULL))
		_vm->_dialogs->show(18);
	else if (action.isAction(VERB_PUSH)) {
		if (randVal < 750)
			_vm->_dialogs->show(19);
		else
			_vm->_dialogs->show(20);
	} else if (action.isAction(VERB_PUT)) {
		int objId = _vm->_game->_objects.getIdFromDesc(action._activeAction._objectNameId);
		if (_vm->_game->_objects.isInInventory(objId))
			_vm->_dialogs->show(25);
		else
			_vm->_dialogs->show(24);
	} else if (action.isAction(VERB_GIVE)) {
		int objId = _vm->_game->_objects.getIdFromDesc(action._activeAction._objectNameId);
		if (!_vm->_game->_objects.isInInventory(objId))
			_vm->_dialogs->show(26);
		else if (randVal <= 500)
			_vm->_dialogs->show(28);
		else
			_vm->_dialogs->show(29);
	} else if (!action.isAction(VERB_WALKTO) && !action.isAction(VERB_WALK_ACROSS) && !action.isAction(VERB_WALK_TOWARDS) && !action.isAction(VERB_WALK_DOWN)
			&& !action.isAction(VERB_SWIM_TO) && !action.isAction(VERB_SWIM_ACROSS) && !action.isAction(VERB_SWIM_INTO) && !action.isAction(VERB_SWIM_THROUGH)
			&& !action.isAction(VERB_SWIM_UNDER)) {
		if (randVal <= 100)
			_vm->_dialogs->show(36);
		else if (randVal <= 200)
			_vm->_dialogs->show(1);
		else if (randVal <= 475)
			_vm->_dialogs->show(34);
		else if (randVal <= 750)
			_vm->_dialogs->show(35);
		else
			_vm->_dialogs->show(37);
	}
}

void GameNebular::step() {
	if (_player._visible && _player._stepEnabled && !_player._moving &&
		(_player._facing == _player._turnToFacing)) {
		if (_scene._frameStartTime >= (uint32)_globals[kWalkerTiming]) {
			if (_player._stopWalkers.empty()) {
				int randomVal = _vm->getRandomNumber(29999);
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

					case FACING_SOUTH:
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

					default:
						break;
					}
				}
			}

			_globals[kWalkerTiming] += 6;
		}
	}

	// Below is countdown to set the timebomb off in room 604
	if (_globals[kTimebombStatus] == TIMEBOMB_ACTIVATED) {
		int diff = _scene._frameStartTime - _globals[kTimebombClock];
		if ((diff >= 0) && (diff <= 60))
			_globals[kTimebombTimer] += diff;
		else
			++_globals[kTimebombTimer];

		_globals[kTimebombClock] = (int)_scene._frameStartTime;
	}
}

void GameNebular::synchronize(Common::Serializer &s, bool phase1) {
	Game::synchronize(s, phase1);

	if (phase1) {
		_globals.synchronize(s);
		s.syncAsByte(_storyMode);
		s.syncAsByte(_difficulty);
	}
}

} // End of namespace Nebular

} // End of namespace MADS
