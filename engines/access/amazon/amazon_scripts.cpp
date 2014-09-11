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
#include "access/access.h"
#include "access/amazon/amazon_game.h"
#include "access/amazon/amazon_resources.h"
#include "access/amazon/amazon_scripts.h"

namespace Access {

namespace Amazon {

AmazonScripts::AmazonScripts(AccessEngine *vm) : Scripts(vm) {
	_game = (AmazonEngine *)_vm;
}

void AmazonScripts::mWhile(int param1) {
	switch(param1) {
	case 1:
		warning("TODO MWHILE1");
		break;
	case 2:
		warning("TODO FLY");
		break;
	case 3:
		warning("TODO FALL");
		break;
	case 4:
		warning("TODO JWALK");
		break;
	case 5:
		warning("TODO DOOPEN");
		break;
	case 6:
		warning("TODO DOWNRIVER");
		break;
	case 7:
		warning("MWHILE2");
		break;
	case 8:
		warning("JWALK2");
		break;
	default:
		break;
	}
}

void AmazonScripts::setInactive() {
	_game->_rawInactiveX = _vm->_player->_rawPlayer.x;
	_game->_rawInactiveY = _vm->_player->_rawPlayer.y;
	_game->_charSegSwitch = false;

	mWhile(_game->_rawInactiveY);
}

void AmazonScripts::executeSpecial(int commandIndex, int param1, int param2) {
	switch (commandIndex) {
	case 1:
		_vm->establish(param1, param2);
		break;
	case 2:
		warning("TODO LOADBACKGROUND");
		break;
	case 3:
		warning("TODO DOCAST");
		break;
	case 4:
		setInactive();
		break;
	case 6:
		mWhile(param1);
		break;
	case 9:
		warning("TODO GUARD");
		break;
	case 10:
		warning("TODO NEWMUSIC");
		break;
	case 11:
		warning("TODO PLOTINACTIVE");
		break;
	case 13:
		warning("TODO RIVER");
		break;
	case 14:
		warning("TODO ANT");
		break;
	case 15:
		warning("TODO BOATWALLS");
		break;
	default:
		warning("Unexpected Special code %d - Skipped", commandIndex);
	}
}

typedef void(AmazonScripts::*AmazonScriptMethodPtr)();

void AmazonScripts::executeCommand(int commandIndex) {
	static const AmazonScriptMethodPtr COMMAND_LIST[] = {
		&AmazonScripts::cmdHelp, &AmazonScripts::CMDCYCLEBACK,
		&AmazonScripts::CMDCHAPTER, &AmazonScripts::cmdSetHelp,
		&AmazonScripts::cmdCenterPanel, &AmazonScripts::cmdMainPanel,
		&AmazonScripts::CMDRETFLASH
	};

	if (commandIndex >= 73)
		(this->*COMMAND_LIST[commandIndex - 73])();
	else
		Scripts::executeCommand(commandIndex);
}

void AmazonScripts::cmdHelp() {
	Common::String helpMessage = readString();

	if (_game->_helpLevel == 0) {
		_game->_timers.saveTimers();
		_game->_useItem = 0;

		if (_game->_noHints) {
			printString(NO_HELP_MESSAGE);
			return;
		} else if (_game->_hintLevel == 0) {
			printString(NO_HINTS_MESSAGE);
			return;
		}
	}

	int level = _game->_hintLevel - 1;
	if (level < _game->_helpLevel)
		_game->_moreHelp = 0;

	_game->drawHelp();
	error("TODO: more cmdHelp");
}

void AmazonScripts::CMDCYCLEBACK() { 
	error("TODO CMDCYCLEBACK"); 
}
void AmazonScripts::CMDCHAPTER() { 
	error("TODO CMDCHAPTER"); 
}

void AmazonScripts::cmdSetHelp() {
	int arrayId = (_data->readUint16LE() && 0xFF) - 1;
	int helpId = _data->readUint16LE() && 0xFF;

	byte *help = _vm->_helpTbl[arrayId];
	help[helpId] = 1;

	if (_vm->_useItem == 0) {
		_sequence = 11000;
		searchForSequence();
	}
}

void AmazonScripts::cmdCenterPanel() {
	if (_vm->_screen->_vesaMode) {
		_vm->_screen->clearScreen();
		_vm->_screen->setPanel(3);
	}
}

void AmazonScripts::cmdMainPanel() {
	if (_vm->_screen->_vesaMode) {
		_vm->_room->init4Quads();
		_vm->_screen->setPanel(0);
	}
}

void AmazonScripts::CMDRETFLASH() { 
	error("TODO CMDRETFLASH"); 
}

} // End of namespace Amazon

} // End of namespace Access
