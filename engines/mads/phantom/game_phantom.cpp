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

GamePhantom::GamePhantom(MADSEngine *vm)
	: Game(vm) {
	_surface = new MSurface(MADS_SCREEN_WIDTH, MADS_SCENE_HEIGHT);
	_storyMode = STORYMODE_NAUGHTY;
}

void GamePhantom::startGame() {
	_scene._priorSceneId = 0;
	_scene._currentSceneId = -1;
	_scene._nextSceneId = 101;

	initializeGlobals();
}

void GamePhantom::initializeGlobals() {
	_globals.reset();

	// TODO: Catacombs setup

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
	_globals[kUnknown]                 = false;
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

} // End of namespace Phantom

} // End of namespace MADS
