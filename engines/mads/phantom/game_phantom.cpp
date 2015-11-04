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

void GamePhantom::doObjectAction() {
	// TODO: Copied from Nebular
	//Scene &scene = _scene;
	MADSAction &action = _scene._action;
	//Dialogs &dialogs = *_vm->_dialogs;
	//int id;

	action._inProgress = false;
}

void GamePhantom::unhandledAction() {
	// TODO
}

void GamePhantom::step() {
	if (_player._visible && _player._stepEnabled && !_player._moving &&
		(_player._facing == _player._turnToFacing)) {

		// TODO
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

	case DIFFICULTY_HARD:
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
	int scene = _globals[kCatacombsRoom];
	assert ((scene < _catacombSize) && (dir < 4));
	return (_catacombs[scene]._exit[dir]);
};

void GamePhantom::moveCatacombs(int dir) {
	assert(_globals[kCatacombsRoom] = CLIP(_globals[kCatacombsRoom], 0, _catacombSize));
	assert(dir = CLIP(dir, 0, 3));

	newCatacombRoom(_catacombs[_globals[kCatacombsRoom]]._fromDirection[dir], _catacombs[_globals[kCatacombsRoom]]._exit[dir]);
};

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
