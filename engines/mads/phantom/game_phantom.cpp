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

#include "common/scummsys.h"
#include "common/config-manager.h"
#include "mads/mads.h"
#include "mads/game.h"
#include "mads/screen.h"
#include "mads/msurface.h"
#include "mads/phantom/game_phantom.h"
//#include "mads/nebular/dialogs_nebular.h"
#include "mads/phantom/globals_phantom.h"
#include "mads/phantom/phantom_scenes.h"

namespace MADS {

namespace Phantom {

	Catacombs _easyCatacombs[32] = {
		{ 401, { -1,  1,  2,  6 }, { 2, 3, 0, 1 }, MAZE_EVENT_BRICK },
		{ 404, { 10, 11,  3,  0 }, { 2, 3, 0, 1 }, MAZE_EVENT_PUDDLE },
		{ 404, {  0,  3,  4, -2 }, { 2, 3, 0, 1 }, MAZE_EVENT_BLOCK },
		{ 401, {  1, 14,  5,  2 }, { 2, 3, 0, 1 }, MAZE_EVENT_POT },
		{ 453, {  2,  4, -1,  4 }, { 2, 3, 0, 1 }, MAZE_EVENT_DRAIN },
		{ 403, {  3,  6, -1, -1 }, { 2, 3, 0, 1 }, MAZE_EVENT_BRICK | MAZE_EVENT_PLANK },
		{ 406, { -1,  0, -1,  5 }, { 2, 3, 0, 1 }, MAZE_EVENT_NONE },
		{ 453, { -1,  8, -1, -1 }, { 2, 3, 0, 1 }, MAZE_EVENT_BRICK },
		{ 406, { -1,  9, -1,  7 }, { 2, 3, 0, 1 }, MAZE_EVENT_NONE },
		{ 401, {  1, -1, 10,  8 }, { 2, 3, 0, 1 }, MAZE_EVENT_RAT_NEST | MAZE_EVENT_SKULL },
		{ 408, {  9, -1,  1, -1 }, { 2, 3, 0, 1 }, MAZE_EVENT_NONE },
		{ 453, { 12, -1, -1,  1 }, { 2, 3, 0, 1 }, MAZE_EVENT_BRICK | MAZE_EVENT_STONE },
		{ 408, { 13, -1, 11, -1 }, { 2, 3, 0, 1 }, MAZE_EVENT_BRICK },
		{ 401, { 13, 20, 12, 13 }, { 3, 3, 0, 0 }, MAZE_EVENT_BRICK },
		{ 453, { 16, 15, -1,  3 }, { 2, 3, 0, 1 }, MAZE_EVENT_BRICK | MAZE_EVENT_RAT_NEST },
		{ 456, { -1, -1, -1, 14 }, { 2, 3, 0, 1 }, MAZE_EVENT_NONE },
		{ 404, { -1, 17, 14, -1 }, { 2, 3, 0, 1 }, MAZE_EVENT_WEB | MAZE_EVENT_POT },
		{ 401, { 18, -1, 19, 16 }, { 2, 3, 0, 1 }, MAZE_EVENT_BRICK },
		{ 408, { -1, -1, 17, -1 }, { 2, 3, 0, 1 }, MAZE_EVENT_BRICK },
		{ 403, { 17, -1, -1, -1 }, { 2, 3, 0, 1 }, MAZE_EVENT_HOLE | MAZE_EVENT_WEB },
		{ 403, { 21, 22, -1, 13 }, { 2, 3, 0, 1 }, MAZE_EVENT_WEB | MAZE_EVENT_SKULL },
		{ 404, { -1, -1, 20, -1 }, { 2, 3, 0, 1 }, MAZE_EVENT_NONE },
		{ 406, { -1, 23, -1, 20 }, { 2, 3, 0, 1 }, MAZE_EVENT_NONE },
		{ 404, { 24, 23, 23, 22 }, { 2, 2, 1, 1 }, MAZE_EVENT_RAT_NEST | MAZE_EVENT_BRICK },
		{ 401, { -1,  1, 23, 25 }, { 2, 1, 0, 1 }, MAZE_EVENT_PUDDLE | MAZE_EVENT_POT | MAZE_EVENT_BRICK },
		{ 407, { 29, 24, 28, 26 }, { 3, 3, 1, 1 }, MAZE_EVENT_NONE },
		{ 401, { 27, 25, 23, -1 }, { 2, 3, 0, 1 }, MAZE_EVENT_SKULL },
		{ 404, { -1, 28, 26, -1 }, { 2, 3, 0, 1 }, MAZE_EVENT_WEB | MAZE_EVENT_FALLEN_BLOCK },
		{ 456, { -1, 25, -1, 27 }, { 2, 2, 0, 1 }, MAZE_EVENT_NONE },
		{ 406, { -1, 30, -1, 25 }, { 2, 3, 0, 0 }, MAZE_EVENT_NONE },
		{ 453, { -3, 30, -1, 29 }, { 2, 3, 0, 1 }, MAZE_EVENT_STONE | MAZE_EVENT_RAT_NEST | MAZE_EVENT_WEB },
		{ 408, { -5, -1, -4, -1 }, { 2, 3, 0, 1 }, MAZE_EVENT_WEB | MAZE_EVENT_BRICK }
	};

	Catacombs _hardCatacombs[62] = {
		{ 401, { -1,  1,  2,  6 }, { 2, 3, 0, 1 }, MAZE_EVENT_BRICK },
		{ 404, { 10, 11,  3,  0 }, { 2, 3, 0, 1 }, MAZE_EVENT_PUDDLE },
		{ 404, {  0,  3,  4, -2 }, { 2, 3, 0, 1 }, MAZE_EVENT_BLOCK },
		{ 401, {  1, 20,  5,  2 }, { 2, 0, 0, 1 }, MAZE_EVENT_POT },
		{ 453, {  2,  4, -1,  4 }, { 2, 3, 0, 1 }, MAZE_EVENT_DRAIN },
		{ 403, {  3,  6, -1,  4 }, { 2, 3, 0, 1 }, MAZE_EVENT_BRICK | MAZE_EVENT_PLANK },
		{ 406, { -1,  0, -1,  5 }, { 2, 3, 0, 1 }, MAZE_EVENT_NONE },
		{ 453, { -1,  8, -1, -1 }, { 2, 3, 0, 1 }, MAZE_EVENT_BRICK },
		{ 406, { -1,  9, -1,  7 }, { 2, 3, 0, 1 }, MAZE_EVENT_NONE },
		{ 401, {  1, -1, 10,  8 }, { 0, 3, 0, 1 }, MAZE_EVENT_RAT_NEST | MAZE_EVENT_SKULL },
		{ 408, {  9, -1,  1, -1 }, { 2, 3, 0, 1 }, MAZE_EVENT_NONE },
		{ 453, { 12, -1, -1,  1 }, { 2, 3, 0, 1 }, MAZE_EVENT_BRICK | MAZE_EVENT_STONE },
		{ 408, { 13, -1, 11, -1 }, { 2, 3, 0, 1 }, MAZE_EVENT_BRICK },
		{ 401, { 13, 21, 12, 13 }, { 3, 3, 0, 0 }, MAZE_EVENT_BRICK },
		{ 453, { 16, 15, -1, 20 }, { 2, 3, 0, 2 }, MAZE_EVENT_RAT_NEST | MAZE_EVENT_BRICK },
		{ 456, { -1, -1, -1, 14 }, { 2, 3, 0, 1 }, MAZE_EVENT_NONE },
		{ 404, { -1, 17, 14, -1 }, { 2, 3, 0, 1 }, MAZE_EVENT_WEB | MAZE_EVENT_POT },
		{ 401, { 18, -1, 19, 16 }, { 2, 3, 0, 1 }, MAZE_EVENT_BRICK },
		{ 408, { -1, -1, 17, -1 }, { 2, 3, 0, 1 }, MAZE_EVENT_BRICK },
		{ 403, { 17, -1, -1, -1 }, { 2, 3, 0, 1 }, MAZE_EVENT_HOLE | MAZE_EVENT_WEB },
		{ 408, {  3, -1, 14, -1 }, { 1, 3, 3, 1 }, MAZE_EVENT_NONE },
		{ 404, {  9, 30, 22, 13 }, { 0, 3, 0, 1 }, MAZE_EVENT_RAT_NEST },
		{ 403, { 21, 23, -1, -1 }, { 2, 3, 0, 1 }, MAZE_EVENT_HOLE | MAZE_EVENT_WEB },
		{ 401, { -1, -1, 24, 22 }, { 2, 3, 3, 1 }, MAZE_EVENT_BRICK },
		{ 406, { -1, 26, -1, 23 }, { 2, 0, 0, 2 }, MAZE_EVENT_NONE },
		{ 407, { 36, 33, 35, 34 }, { 3, 3, 1, 1 }, MAZE_EVENT_NONE },
		{ 453, { 24, 27, -1, -1 }, { 1, 0, 0, 1 }, MAZE_EVENT_BRICK },
		{ 403, { 26, -1, -1, 28 }, { 1, 3, 0, 0 }, MAZE_EVENT_BRICK | MAZE_EVENT_SKULL },
		{ 404, { 27, 28, 28, 29 }, { 3, 2, 1, 2 }, MAZE_EVENT_NONE },
		{ 408, { -1, -1, 28, -1 }, { 2, 3, 3, 1 }, MAZE_EVENT_BRICK },
		{ 406, { -1, 31, -1, 21 }, { 2, 0, 0, 1 }, MAZE_EVENT_NONE },
		{ 401, { 30, 33,  1, -1 }, { 1, 2, 1, 1 }, MAZE_EVENT_PUDDLE | MAZE_EVENT_POT },
		{ 456, { -1, 31, -1, 33 }, { 2, 1, 0, 0 }, MAZE_EVENT_NONE },
		{ 404, { 32, -1, 31, 25 }, { 3, 3, 1, 1 }, MAZE_EVENT_NONE },
		{ 401, { 46, 25, 31, -1 }, { 2, 3, 0, 1 }, MAZE_EVENT_SKULL },
		{ 401, { -1, 25, 41, -1 }, { 2, 2, 1, 1 }, MAZE_EVENT_BRICK | MAZE_EVENT_POT },
		{ 406, { -1, 37, -1, 25 }, { 2, 3, 0, 0 }, MAZE_EVENT_NONE },
		{ 453, { -3, 37, -1, 36 }, { 2, 3, 0, 1 }, MAZE_EVENT_STONE | MAZE_EVENT_RAT_NEST | MAZE_EVENT_WEB },
		{ 408, { 57, -1, 54, -1 }, { 2, 3, 0, 1 }, MAZE_EVENT_NONE },
		{ 408, { 40, -1, -4, -1 }, { 2, 3, 0, 1 }, MAZE_EVENT_BRICK | MAZE_EVENT_WEB },
		{ 404, { 40, 40, 39, 53 }, { 1, 0, 0, 1 }, MAZE_EVENT_BLOCK | MAZE_EVENT_FALLEN_BLOCK },
		{ 456, { -1, 35, -1, 42 }, { 2, 2, 0, 2 }, MAZE_EVENT_NONE },
		{ 408, { 43, -1, 41, -1 }, { 1, 3, 3, 1 }, MAZE_EVENT_BRICK },
		{ 406, { -1, 42, -1, 61 }, { 2, 0, 0, 1 }, MAZE_EVENT_NONE },
		{ 403, { 58, 45, -1, -1 }, { 2, 3, 0, 1 }, MAZE_EVENT_BRICK | MAZE_EVENT_RAT_NEST },
		{ 401, { 34, -1, 46, 44 }, { 0, 3, 0, 1 }, MAZE_EVENT_RAT_NEST | MAZE_EVENT_BRICK },
		{ 404, { 45, -1, 34, 47 }, { 2, 3, 0, 1 }, MAZE_EVENT_WEB | MAZE_EVENT_FALLEN_BLOCK },
		{ 406, { -1, 46, -1, 48 }, { 2, 3, 0, 1 }, MAZE_EVENT_NONE },
		{ 403, { 49, 47, -1, -1 }, { 2, 3, 0, 1 }, MAZE_EVENT_BRICK | MAZE_EVENT_SKULL | MAZE_EVENT_WEB },
		{ 408, { 50, -1, 48, -1 }, { 2, 3, 0, 1 }, MAZE_EVENT_BRICK },
		{ 408, { 51, -1, 49, -1 }, { 2, 3, 0, 1 }, MAZE_EVENT_NONE },
		{ 408, { 52, -1, 50, -1 }, { 2, 3, 0, 1 }, MAZE_EVENT_BRICK },
		{ 408, { -1, -1, 51, -1 }, { 2, 3, 0, 1 }, MAZE_EVENT_BRICK },
		{ 406, { -1, 40, -1, 54 }, { 2, 3, 0, 1 }, MAZE_EVENT_NONE },
		{ 403, { 38, 53, -1, 55 }, { 2, 3, 0, 1 }, MAZE_EVENT_SKULL },
		{ 453, { 56, 54, -1, -1 }, { 2, 3, 0, 1 }, MAZE_EVENT_BRICK | MAZE_EVENT_WEB },
		{ 401, { 56, -5, 55, 56 }, { 3, 3, 0, 0 }, MAZE_EVENT_BRICK | MAZE_EVENT_SKULL },
		{ 404, { -1, 57, 38, 57 }, { 2, 3, 0, 1 }, MAZE_EVENT_POT | MAZE_EVENT_BLOCK },
		{ 404, { 59, 59, 44, 60 }, { 2, 3, 0, 1 }, MAZE_EVENT_NONE },
		{ 404, { 59, 60, 59, 58 }, { 2, 3, 0, 1 }, MAZE_EVENT_NONE },
		{ 404, { 61, 58, 59, 59 }, { 2, 3, 0, 1 }, MAZE_EVENT_NONE },
		{ 404, { 34, 43, 60, 44 }, { 0, 3, 0, 1 }, MAZE_EVENT_NONE }
	};

GamePhantom::GamePhantom(MADSEngine *vm) : Game(vm) {
	_surface = new MSurface(MADS_SCREEN_WIDTH, MADS_SCENE_HEIGHT);
	_difficulty = DIFFICULTY_HARD;
	_catacombs = nullptr;
	_catacombSize = -1;
}

void GamePhantom::startGame() {
	_scene._priorSceneId = 0;
	_scene._currentSceneId = -1;
	_scene._nextSceneId = 101;

	initializeGlobals();
}

void GamePhantom::initializeGlobals() {
	_globals.reset();
	setupCatacombs();

	_player._facing = FACING_NORTH;
	_player._turnToFacing = FACING_NORTH;

	_globals[kTempVar]                 = false;
	_globals[kRoom103104Transition]    = 1;		// new room
	_globals[kCurrentYear]             = 1993;
	_globals[kTrapDoorStatus]          = 0;		// open
	_globals[kChristineDoorStatus]     = 0;		// Christine is in her room
	_globals[kSandbagStatus]           = 0;		// sandbag is secure
	_globals[kJacquesStatus]           = 0;		// alive
	_globals[kChrisFStatus]            = 1;		// Christine F. is alive in 1993
	_globals[kBrieTalkStatus]          = 0;		// before Brie motions
	_globals[kPanelIn206]              = 0;		// not discovered
	_globals[kFightStatus]             = 0;
	_globals[kJuliesDoor]              = 1;		// cracked open
	_globals[kPrompterStandStatus]     = 0;
	_globals[kChrisDStatus]            = 0;		// before love
	_globals[kJulieNameIsKnown]        = 0;
	_globals[kDoorsIn205]              = 0;		// both locked
	_globals[kMadameGiryLocation]      = 1;		// middle
	_globals[kTicketPeoplePresent]     = 0;
	_globals[kCoffinStatus]            = 0;		// closed and locked
	_globals[kDoneBrieConv203]         = 0;
	_globals[kFlorentNameIsKnown]      = 0;
	_globals[kDegasNameIsKnown]        = 0;
	_globals[kMadameGiryShowsUp]       = false;
	_globals[kJacquesNameIsKnown]      = 0;
	_globals[kCharlesNameIsKnown]      = false;
	_globals[kTopFloorLocked]          = true;
	_globals[kMadameNameIsKnown]       = 0;
	_globals[kChrisKickedRaoulOut]     = false;
	_globals[kLookedAtCase]            = false;
	_globals[kRingIsOnFinger]          = false;
	_globals[kHeListened]              = false;
	_globals[kKnockedOverHead]         = false;
	_globals[kObservedPhan104]         = false;
	_globals[kReadBook]                = false;
	_globals[kCanFindBookInLibrary]    = false;
	_globals[kLookedAtSkullFace]       = false;
	_globals[kScannedBookcase]         = false;
	_globals[kRanConvIn205]            = false;
	_globals[kDoneRichConv203]         = false;
	_globals[kHintThatDaaeIsHome1]     = false;
	_globals[kHintThatDaaeIsHome2]     = false;
	_globals[kMakeBrieLeave203]        = false;
	_globals[kMakeRichLeave203]        = false;
	_globals[kCameFromFade]            = false;
	_globals[kChristineToldEnvelope]   = false;
	_globals[kLeaveAngelMusicOn]       = false;
	_globals[kDoorIn409IsOpen]         = false;
	_globals[kPriestPistonPuke]        = false;
	_globals[kCobwebIsCut]             = false;
	_globals[kChristineIsInBoat]       = false;
	_globals[kRightDoorIsOpen504]      = false;
	_globals[kChrisLeft505]            = false;
	_globals[kChrisWillTakeSeat]       = true;
	_globals[kFlickedLever1]           = 0;
	_globals[kFlickedLever2]           = 0;
	_globals[kFlickedLever3]           = 0;
	_globals[kFlickedLever4]           = 0;
	_globals[kPlayerScore]             = 0;
	_globals[kPlayerScoreFlags]        = 0;

	_globals[kMusicSelected] = _vm->getRandomNumber(1, 4);

	_player._spritesPrefix = "RAL";	// Fixed prefix
	Player::preloadSequences("RAL", 1);
}

void GamePhantom::setSectionHandler() {
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
	default:
		break;
	}
}

void GamePhantom::checkShowDialog() {
	// TODO: Copied from Nebular
	if (_vm->_dialogs->_pendingDialog && _player._stepEnabled && !_globals[5]) {
		_player.releasePlayerSprites();

		// HACK: Skip the main menu (since it'll then try to show Rex's main menu)
		if (_vm->_dialogs->_pendingDialog == DIALOG_MAIN_MENU)
			_vm->_dialogs->_pendingDialog = DIALOG_NONE;

		_vm->_dialogs->showDialog();
		_vm->_dialogs->_pendingDialog = DIALOG_NONE;
	}
}

void GamePhantom::genericObjectExamine() {
	MADSAction &action = _scene._action;
	int	id = _objects.getIdFromDesc(action._activeAction._objectNameId);

	if (action.isAction(VERB_LOOK, NOUN_RED_FRAME))
		_vm->_dialogs->showItem(id, (_globals[kCurrentYear] == 1993) ? 802 : 842, 0);
	else if (action.isAction(VERB_LOOK, NOUN_YELLOW_FRAME))
		_vm->_dialogs->showItem(id, (_globals[kCurrentYear] == 1993) ? 804 : 843, 0);
	else if (action.isAction(VERB_LOOK, NOUN_BLUE_FRAME))
		_vm->_dialogs->showItem(id, (_globals[kCurrentYear] == 1993) ? 817 : 844, 0);
	else if (action.isAction(VERB_LOOK, NOUN_GREEN_FRAME))
		_vm->_dialogs->showItem(id, (_globals[kCurrentYear] == 1993) ? 819 : 845, 0);
	else if (action.isAction(VERB_LOOK, NOUN_LANTERN))
		_vm->_dialogs->showItem(id, (_globals[kLanternStatus] == 1) ? 831 : 801, 0);
	else if (action.isAction(VERB_LOOK, NOUN_SMALL_NOTE))
		_vm->_dialogs->showItem(OBJ_SMALL_NOTE, 846, 2);
	else if (action.isAction(VERB_LOOK, NOUN_PARCHMENT))
		_vm->_dialogs->showItem(OBJ_PARCHMENT, 812, 3);
	else if (action.isAction(VERB_LOOK, NOUN_LETTER))
		_vm->_dialogs->showItem(OBJ_LETTER, 813, 4);
	else if (action.isAction(VERB_LOOK, NOUN_NOTICE))
		_vm->_dialogs->showItem(OBJ_NOTICE, 814, 5);
	else if (action.isAction(VERB_LOOK, NOUN_CRUMPLED_NOTE))
		_vm->_dialogs->showItem(OBJ_CRUMPLED_NOTE, 816, 6);
	else if (action.isAction(VERB_LOOK, NOUN_LARGE_NOTE))
		_vm->_dialogs->showItem(OBJ_LARGE_NOTE, 818, 7);
	else
	_vm->_dialogs->showItem(id, 800 + id, 0);
}

void GamePhantom::doObjectAction() {
	MADSAction &action = _scene._action;

	if ((_scene._currentSceneId >= 401) && (_scene._currentSceneId <= 456)
	 && (action.isObject(NOUN_RED_FRAME) || action.isObject(NOUN_YELLOW_FRAME) || action.isObject(NOUN_GREEN_FRAME) || action.isObject(NOUN_BLUE_FRAME))
	 && action.isAction(VERB_PUT)) {
		if (action.isTarget(NOUN_UNLUCKY_ADVENTURER)) {
			_vm->_dialogs->show(35);
			action._inProgress = false;
			return;
		} else if (action.isTarget(NOUN_HOLE)) {
			_vm->_dialogs->show(36);
			action._inProgress = false;
			return;
		} else if (action.isTarget(NOUN_GRATE)) {
			_vm->_dialogs->show(37);
			action._inProgress = false;
			return;
		} else if (action.isTarget(NOUN_WALL)) {
			_vm->_dialogs->show(38);
			action._inProgress = false;
			return;
		}
	}

	if (action._lookFlag) {
		_vm->_dialogs->show(810);
		action._inProgress = false;
		return;
	}

	if (action.isAction(VERB_PUT, NOUN_RED_FRAME) || action.isAction(VERB_PUT, NOUN_BLUE_FRAME) || action.isAction(VERB_PUT, NOUN_YELLOW_FRAME) || action.isAction(VERB_PUT, NOUN_GREEN_FRAME)) {
		_vm->_dialogs->show((action.isTarget(NOUN_PUDDLE)) ? 40124 : 40125);
		action._inProgress = false;
		return;
	}

	if (action.isAction(VERB_WEAR, NOUN_WEDDING_RING)) {
		if (_globals[kRingIsOnFinger])
			_vm->_dialogs->show(849);
		else {
			_vm->_dialogs->show(835);
			_globals[kRingIsOnFinger] = true;
		}
		action._inProgress = false;
		return;
	}

	if (action.isAction(VERB_REMOVE, NOUN_WEDDING_RING)) {
		if (!_globals[kRingIsOnFinger])
			_vm->_dialogs->show(848);
		else {
			_vm->_dialogs->show(836);
			_globals[kRingIsOnFinger] = false;
		}
		action._inProgress = false;
		return;
	}

	if (action.isAction(VERB_LOOK, NOUN_ARCHWAY_TO_WEST) || action.isAction(VERB_LOOK, NOUN_ARCHWAY_TO_EAST) || action.isAction(VERB_LOOK, NOUN_ARCHWAY_TO_NORTH)) {
		_vm->_dialogs->show(34);
		action._inProgress = false;
		return;
	}

	if (action.isAction(VERB_ATTACK, NOUN_CHRISTINE)) {
		_vm->_dialogs->show(33);
		action._inProgress = false;
		return;
	}

	if (action.isAction(VERB_LOOK, NOUN_KEY)) {
		_vm->_dialogs->showItem(OBJ_KEY, 800, 0);
		action._inProgress = false;
		return;
	}

	if (action.isAction(VERB_LOOK, NOUN_SANDBAG)) {
		_vm->_dialogs->showItem(OBJ_SANDBAG, 803, 0);
		action._inProgress = false;
		return;
	}

	if (action.isAction(VERB_LOOK, NOUN_SMALL_NOTE) || action.isAction(VERB_READ, NOUN_SMALL_NOTE)) {
		_vm->_dialogs->showItem(OBJ_SMALL_NOTE, 806, 2);
		action._inProgress = false;
		return;
	}

	if (action.isAction(VERB_LOOK, NOUN_ROPE)) {
		_vm->_dialogs->showItem(OBJ_ROPE, 807, 0);
		action._inProgress = false;
		return;
	}

	if (action.isAction(VERB_LOOK, NOUN_SWORD)) {
		_vm->_dialogs->showItem(OBJ_SWORD, 808, 0);
		action._inProgress = false;
		return;
	}

	if (action.isAction(VERB_LOOK, NOUN_ENVELOPE) || action.isAction(VERB_READ, NOUN_ENVELOPE)) {
		_vm->_dialogs->showItem(OBJ_ENVELOPE, 809, 0);
		action._inProgress = false;
		return;
	}

	if (action.isAction(VERB_LOOK, NOUN_TICKET) || action.isAction(VERB_READ, NOUN_TICKET)) {
		_vm->_dialogs->showItem(OBJ_TICKET, 810, 0);
		action._inProgress = false;
		return;
	}

	if (action.isAction(VERB_LOOK, NOUN_PIECE_OF_PAPER) || action.isAction(VERB_READ, NOUN_PIECE_OF_PAPER)) {
		_vm->_dialogs->showItem(OBJ_PIECE_OF_PAPER, 811, 1);
		action._inProgress = false;
		return;
	}

	if (action.isAction(VERB_LOOK, NOUN_PARCHMENT) || action.isAction(VERB_READ, NOUN_PARCHMENT)) {
		_vm->_dialogs->showItem(OBJ_PARCHMENT, 812, 3);
		action._inProgress = false;
		return;
	}

	if (action.isAction(VERB_LOOK, NOUN_LETTER) || action.isAction(VERB_READ, NOUN_LETTER)) {
		_vm->_dialogs->showItem(OBJ_LETTER, 813, 4);
		action._inProgress = false;
		return;
	}

	if (action.isAction(VERB_LOOK, NOUN_NOTICE) || action.isAction(VERB_READ, NOUN_NOTICE)) {
		_vm->_dialogs->showItem(OBJ_NOTICE, 814, 5);
		action._inProgress = false;
		return;
	}

	if (action.isAction(VERB_LOOK, NOUN_BOOK) || action.isAction(VERB_READ, NOUN_BOOK)) {
		_vm->_dialogs->showItem(OBJ_BOOK, 815, 0);
		action._inProgress = false;
		return;
	}

	if (action.isAction(VERB_LOOK, NOUN_CRUMPLED_NOTE) || action.isAction(VERB_READ, NOUN_CRUMPLED_NOTE)) {
		_vm->_dialogs->showItem(OBJ_CRUMPLED_NOTE, 816, 6);
		action._inProgress = false;
		return;
	}

	if (action.isAction(VERB_LOOK, NOUN_LARGE_NOTE) || action.isAction(VERB_READ, NOUN_LARGE_NOTE)) {
		_vm->_dialogs->showItem(OBJ_LARGE_NOTE, 818, 7);
		action._inProgress = false;
		return;
	}

	if (action.isAction(VERB_LOOK, NOUN_MUSIC_SCORE) || action.isAction(VERB_READ, NOUN_MUSIC_SCORE)) {
		_vm->_dialogs->showItem(OBJ_MUSIC_SCORE, 820, 0);
		action._inProgress = false;
		return;
	}

	if (action.isAction(VERB_LOOK, NOUN_WEDDING_RING)) {
		_vm->_dialogs->showItem(OBJ_WEDDING_RING, 821, 0);
		action._inProgress = false;
		return;
	}

	if (action.isAction(VERB_LOOK, NOUN_CABLE_HOOK)) {
		_vm->_dialogs->showItem(OBJ_CABLE_HOOK, 822, 0);
		action._inProgress = false;
		return;
	}

	if (action.isAction(VERB_LOOK, NOUN_ROPE_WITH_HOOK)) {
		_vm->_dialogs->showItem(OBJ_ROPE_WITH_HOOK, 823, 0);
		action._inProgress = false;
		return;
	}

	if (action.isAction(VERB_LOOK, NOUN_OAR)) {
		_vm->_dialogs->showItem(OBJ_OAR, 824, 0);
		action._inProgress = false;
		return;
	}

	if (action.isAction(VERB_LOOK) && _objects.isInInventory(_objects.getIdFromDesc(action._activeAction._objectNameId))) {
		genericObjectExamine();
		action._inProgress = false;
		return;
	}

	if (action.isAction(VERB_TURN_ON, NOUN_LANTERN)) {
		if ((_globals[kLanternStatus] == 1) && !_trigger)
			_vm->_dialogs->show(828);
		else {
			switch (_trigger) {
			case 0:
				_scene._sequences.addTimer(4, 1);
				_globals[kLanternStatus] = 1;
				_vm->_dialogs->spinObject(OBJ_LANTERN);
				break;

			case 1:
				_vm->_dialogs->show(825);
				break;

			default:
				break;
			}
		}
		action._inProgress = false;
		return;
	}

	if (action.isAction(VERB_TURN_OFF, NOUN_LANTERN)) {
		if ((_globals[kLanternStatus] == 0) && !_trigger)
			_vm->_dialogs->show(829);
		else if ((_scene._currentSceneId / 100) == 4)
			_vm->_dialogs->show(826);
		else {
			switch (_trigger) {
			case 0:
				_scene._sequences.addTimer(4, 1);
				_globals[kLanternStatus] = 0;
				_vm->_dialogs->spinObject(OBJ_LANTERN);
				break;

			case 1:
				_vm->_dialogs->show(827);
				break;

			default:
				break;
			}
		}
		action._inProgress = false;
		return;
	}

	if (action.isAction(VERB_OPEN, NOUN_ENVELOPE)) {
		_objects.setRoom(OBJ_ENVELOPE, NOWHERE);
		_objects.addToInventory(OBJ_TICKET);
		_objects.addToInventory(OBJ_PIECE_OF_PAPER);
		_vm->_dialogs->show(833);
		action._inProgress = false;
		return;
	}

	if (action.isAction(VERB_ATTACH, NOUN_CABLE_HOOK, NOUN_ROPE)) {
		if (!_objects.isInInventory(OBJ_ROPE))
			_vm->_dialogs->show(11438);
		else {
			_objects.setRoom(OBJ_CABLE_HOOK, NOWHERE);
			_objects.setRoom(OBJ_ROPE, NOWHERE);
			_objects.addToInventory(OBJ_ROPE_WITH_HOOK);
			_vm->_dialogs->showItem(OBJ_ROPE_WITH_HOOK, 823, 0);
		}
		action._inProgress = false;
	}
}

void GamePhantom::unhandledAction() {
	int messageId = 0;
	int rndNum = _vm->getRandomNumber(1, 1000);
	MADSAction &action = _scene._action;

	if (action.isAction(VERB_PUT, NOUN_CHANDELIER, NOUN_SEATS))
		_vm->_dialogs->show(10123);
	else if (action.isAction(VERB_TAKE)) {
		if (_objects.isInInventory(_objects.getIdFromDesc(action._activeAction._objectNameId)))
			messageId = 25;
		else {
			if (rndNum <= 333)
				messageId = 1;
			else if (rndNum <= 666)
				messageId = 2;
			else
				messageId = 3;
		}
	} else if (action.isAction(VERB_PUSH)) {
		if (rndNum < 750)
			messageId = 4;
		else
			messageId = 5;
	} else if (action.isAction(VERB_PULL)) {
		if (rndNum < 750)
			messageId = 6;
		else
			messageId = 7;
	} else if (action.isAction(VERB_OPEN)) {
		if (rndNum <= 500)
			messageId = 8;
		else if (rndNum <= 750)
			messageId = 9;
		else
			messageId = 10;
	} else if (action.isAction(VERB_CLOSE)) {
		if (rndNum <= 500)
			messageId = 11;
		else if (rndNum <= 750)
			messageId = 12;
		else
			messageId = 13;
	} else if (action.isAction(VERB_PUT)) {
		if (_objects.isInInventory(_objects.getIdFromDesc(action._activeAction._objectNameId)))
			messageId = 26;
		else if (rndNum < 500)
			messageId = 14;
		else
			messageId = 15;
	} else if (action.isAction(VERB_TALK_TO)) {
		if (rndNum <= 500)
			messageId = 16;
		else
			messageId = 17;
	} else if (action.isAction(VERB_GIVE)) {
		if (_objects.isInInventory(_objects.getIdFromDesc(action._activeAction._objectNameId)))
			messageId = 27;
		else
			messageId = 18;
	} else if (action.isAction(VERB_THROW)) {
		if (_objects.isInInventory(_objects.getIdFromDesc(action._activeAction._objectNameId)))
			messageId = 19;
		else
			messageId = 28;
	} else if (action.isAction(VERB_LOOK)) {
		if (rndNum <= 333)
			messageId = 20;
		else if (rndNum <= 666)
			messageId = 21;
		else
			messageId = 22;
	} else if ((action.isAction(VERB_UNLOCK) || action.isAction(VERB_LOCK))
			&& (action.isObject(NOUN_DOOR) || action.isObject(NOUN_LEFT_DOOR) || action.isObject(NOUN_MIDDLE_DOOR) || action.isObject(NOUN_RIGHT_DOOR) || action.isObject(NOUN_TRAP_DOOR)))
		messageId = 32;
	else if (!action.isAction(VERB_WALK_TO) && !action.isAction(VERB_WALK_ACROSS) && !action.isAction(VERB_WALK_DOWN) && !action.isAction(VERB_WALK_UP)) {
		if (rndNum < 500)
			messageId = 23;
		else
			messageId = 24;
	}

	if (messageId)
		_vm->_dialogs->show(messageId);
}

void GamePhantom::stopWalker() {
	int state   = _globals[kWalkerConverseState];
	int command = _globals[kWalkerConverse];

	_globals[kWalkerConverseNow] = state;

	if ((_player._facing != FACING_NORTHEAST) && (_player._facing != FACING_NORTHWEST)) {
		state   = 0;
		command = 0;
	}

	switch (state) {
	case 1:
		switch (command) {
		case 1:
			_player.addWalker(3, 0);
			break;

		case 2:
		case 3:
			_player.addWalker(6, 0);
			_player.addWalker(5, 0);
			_player.addWalker(4, 0);
			state = 2;
			break;

		case 4:
			_player.addWalker(8, 0);
			_player.addWalker(4, 0);
			state = 4;
			break;

		default:
			_player.addWalker(-2, 0);
			state = 0;
			break;
		}
		break;

	case 2:
	case 3:
		switch (command) {
		case 2:
		case 3:
			if (state == 2) {
				if (_vm->getRandomNumber(1, 30000) < 2000) {
					_player.addWalker(10, 0);
					_player.addWalker(7, 0);
					state = 3;
				} else
					_player.addWalker(6, 0);
			} else {
				if (_vm->getRandomNumber(1, 30000) < 1000) {
					_player.addWalker(6, 0);
					_player.addWalker(7, 0);
					state = 2;
				} else
					_player.addWalker(10, 0);
			}
			break;

		default:
			_player.addWalker(-4, 0);
			_player.addWalker(-5, 0);
			if (state == 3) {
				_player.addWalker(6, 0);
				_player.addWalker(7, 0);
			}
			state = 1;
			break;
		}
		break;

	case 4:
		if (command == 4)
			_player.addWalker(9, 0);
		else {
			_player.addWalker(-4, 0);
			_player.addWalker(-8, 0);
			state = 1;
		}
		break;

	case 0:
	default:
		switch (command) {
		case 1:
		case 2:
		case 3:
		case 4:
			_player.addWalker(2, 0);
			state = 1;
			break;

		default:
			stopWalkerBasic();
			break;
		}
		break;
	}

	_globals[kWalkerConverse]       = command;
	_globals[kWalkerConverseState] = state;
}

void GamePhantom::step() {
	if (_player._visible  && !_globals[kStopWalkerDisabled]
	 && (_player._stepEnabled || (_vm->_gameConv->activeConvId() >= 0))
	 && !_player._moving && (_player._facing == _player._turnToFacing)
	 && (_scene._frameStartTime >= (uint32)_globals[kWalkerTiming])) {
		if (_player._stopWalkers.empty())
			stopWalker();

		_globals[kWalkerTiming] += 6;
	}
}

void GamePhantom::stopWalkerBasic() {
	int rndVal  = _vm->getRandomNumber(1, 30000);

	switch (_player._facing) {
	case FACING_SOUTH:
		if (rndVal < 500) {
			int maxSteps = _vm->getRandomNumber(4, 10);
			for (int i = 0; i < maxSteps; i++)
				_player.addWalker((rndVal < 250) ? 1 : 2, 0);
		} else if (rndVal < 750) {
			for (int i = 0; i < 4; i++)
				_player.addWalker(1, 0);

			_player.addWalker(0, 0);

			for (int i = 0; i < 4; i++)
				_player.addWalker(2, 0);

			_player.addWalker(0, 0);
		}
		break;

	case FACING_SOUTHEAST:
	case FACING_SOUTHWEST:
	case FACING_NORTHEAST:
	case FACING_NORTHWEST:
		if (rndVal < 150) {
			_player.addWalker(-1, 0);
			_player.addWalker(1, 0);
			for (int i = 0; i < 6; i++)
				_player.addWalker(0, 0);
		}
		break;

	case FACING_EAST:
	case FACING_WEST:
		if (rndVal < 250) {
			_player.addWalker(-1, 0);
			int maxSteps = _vm->getRandomNumber(2, 6);
			for (int i = 0; i < maxSteps; i++)
				_player.addWalker(2, 0);
			_player.addWalker(1, 0);
			_player.addWalker(0, 0);
			_player.addWalker(0, 0);
		} else if (rndVal < 500)
			_globals[kWalkerTiming] = (int)_scene._frameStartTime;
		break;

	case FACING_NORTH:
		if (rndVal < 250) {
			_player.addWalker(-1, 0);
			int maxSteps = _vm->getRandomNumber(3, 7);
			for (int i = 0; i < maxSteps; i++)
				_player.addWalker(2, 0);
			_player.addWalker(1, 0);
			_player.addWalker(0, 0);
		}
		break;

	default:
		break;
	}
}

void GamePhantom::synchronize(Common::Serializer &s, bool phase1) {
	Game::synchronize(s, phase1);

	// TODO: Copied from Nebular
	if (!phase1) {
		_globals.synchronize(s);
	}
}

void GamePhantom::enterCatacombs(bool val) {
	setupCatacombs();

	int var4, var2;
	if (_scene._currentSceneId == 409) {
		if (val) {
			var4 = _globals[kCatacombs409b];
			var2 = _globals[kCatacombs409bFrom];
		} else {
			var4 = _globals[kCatacombs409a];
			var2 = _globals[kCatacombs409aFrom];
		}
	} else if (_scene._currentSceneId == 501) {
		var4 = _globals[kCatacombs501];
		var2 = _globals[kCatacombs501From];
	} else {
		var4 = _globals[kCatacombs309];
		var2 = _globals[kCatacombs309From];
	}

	newCatacombRoom(var4, var2);
}

void GamePhantom::initCatacombs() {
	_globals[kCatacombsRoom] = _globals[kCatacombsNextRoom];
}

void GamePhantom::setupCatacombs() {
	switch (_difficulty) {
	case DIFFICULTY_EASY:
		_catacombs = _easyCatacombs;
		_catacombSize = 32;

		_globals[kCatacombs309]      = 2;
		_globals[kCatacombs309From]  = 3;
		_globals[kCatacombs409a]     = 30;
		_globals[kCatacombs409aFrom] = 0;
		_globals[kCatacombs409b]     = 31;
		_globals[kCatacombs409bFrom] = 2;
		_globals[kCatacombs501]      = 31;
		_globals[kCatacombs501From]  = 0;
		break;

	case DIFFICULTY_MEDIUM:
		// TODO: FIXME. Do we need to set something here?
		break;

	case DIFFICULTY_HARD:
	default:
		_catacombs = _hardCatacombs;
		_catacombSize = 62;

		_globals[kCatacombs309]      = 2;
		_globals[kCatacombs309From]  = 3;
		_globals[kCatacombs409a]     = 37;
		_globals[kCatacombs409aFrom] = 0;
		_globals[kCatacombs409b]     = 39;
		_globals[kCatacombs409bFrom] = 2;
		_globals[kCatacombs501]      = 56;
		_globals[kCatacombs501From]  = 1;
		break;
	}
}

int GamePhantom::exitCatacombs(int dir) {
	assert(_globals[kCatacombsRoom] == CLIP(_globals[kCatacombsRoom], 0, _catacombSize - 1));
	assert(dir == CLIP(dir, 0, 3));

	return (_catacombs[_globals[kCatacombsRoom]]._exit[dir]);
}

void GamePhantom::moveCatacombs(int dir) {
	assert(_globals[kCatacombsRoom] == CLIP(_globals[kCatacombsRoom], 0, _catacombSize - 1));
	assert(dir == CLIP(dir, 0, 3));

	newCatacombRoom(_catacombs[_globals[kCatacombsRoom]]._fromDirection[dir], _catacombs[_globals[kCatacombsRoom]]._exit[dir]);
}

void GamePhantom::newCatacombRoom(int toRoom, int fromExit) {
	_globals[kCatacombsNextRoom] = toRoom;
	_globals[kCatacombsFrom] = fromExit & 0x03;
	_globals[kCatacombsFlag] = fromExit & 0xFC;

	int newSceneNum = -1;

	if (toRoom < 0) {
		switch (toRoom) {
		case -5:
			newSceneNum = 501;
			break;

		case -4:
		case -3:
			newSceneNum = 409;
			break;

		case -2:
			newSceneNum = 309;
			break;

		default:
			error("Unexpected room in newCatacombRoom");
		}
	} else {
		newSceneNum = _catacombs[toRoom]._sceneNum;
		_globals[81] = _catacombs[toRoom]._flags;
	}

	if (_triggerSetupMode == SEQUENCE_TRIGGER_PREPARE) {
		_player._walkOffScreenSceneId = newSceneNum;
	} else {
		_scene._reloadSceneFlag = true;
		_scene._nextSceneId = newSceneNum;
	}
}

} // End of namespace Phantom

} // End of namespace MADS
