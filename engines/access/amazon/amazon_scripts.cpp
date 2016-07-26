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
#include "access/access.h"
#include "access/resources.h"
#include "access/amazon/amazon_game.h"
#include "access/amazon/amazon_resources.h"
#include "access/amazon/amazon_scripts.h"

namespace Access {

namespace Amazon {

AmazonScripts::AmazonScripts(AccessEngine *vm) : Scripts(vm) {
	_game = (AmazonEngine *)_vm;

	setOpcodes_v2();
}

void AmazonScripts::cLoop() {
	searchForSequence();
	_vm->_images.clear();
	_vm->_buffer2.blitFrom(_vm->_buffer1);
	_vm->_oldRects.clear();
	_vm->_scripts->executeScript();
	_vm->plotList1();
	_vm->copyBlocks();
}

void AmazonScripts::mWhile1() {
	_vm->_screen->setDisplayScan();
	_vm->_screen->fadeOut();
	_vm->_events->hideCursor();

	_vm->_files->loadScreen(14, 0);
	_vm->_buffer2.blitFrom(*_vm->_screen);
	_vm->_buffer1.blitFrom(*_vm->_screen);
	_vm->_events->showCursor();

	_vm->_screen->setIconPalette();
	_vm->_screen->forceFadeIn();

	Resource *spriteData = _vm->_files->loadFile(14, 6);
	_vm->_objectsTable[0] = new SpriteResource(_vm, spriteData);
	delete spriteData;

	_vm->_images.clear();
	_vm->_oldRects.clear();
	_sequence = 2100;

	do {
		cLoop();
		_sequence = 2100;
	} while (_vm->_flags[52] == 1);

	_vm->_buffer1.copyTo(_vm->_screen);
	_vm->_buffer2.copyTo(&_vm->_buffer1);

	_game->establish(-1, 14);

	spriteData = _vm->_files->loadFile(14, 7);
	_vm->_objectsTable[1] = new SpriteResource(_vm, spriteData);
	delete spriteData;

	_vm->_sound->playSound(0);
	_vm->_screen->setDisplayScan();
	_vm->_events->hideCursor();

	_vm->_files->loadScreen(14, 1);
	_vm->_screen->setPalette();
	_vm->_buffer2.blitFrom(*_vm->_screen);
	_vm->_buffer1.blitFrom(*_vm->_screen);
	_vm->_events->showCursor();

	_vm->_screen->setIconPalette();
	_vm->_images.clear();
	_vm->_oldRects.clear();
	_sequence = 2200;

	_vm->_sound->loadSoundTable(0, 14, 15);

	do {
		cLoop();
		_sequence = 2200;
	} while (_vm->_flags[52] == 2);

	_vm->_screen->setDisplayScan();
	_vm->_events->hideCursor();

	_vm->_files->loadScreen(14, 2);
	_vm->_screen->setPalette();
	_vm->_buffer2.blitFrom(*_vm->_screen);
	_vm->_buffer1.blitFrom(*_vm->_screen);
	_vm->_events->showCursor();

	_vm->_screen->setIconPalette();
	_vm->freeCells();

	spriteData = _vm->_files->loadFile(14, 8);
	_vm->_objectsTable[2] = new SpriteResource(_vm, spriteData);
	delete spriteData;

	_vm->_images.clear();
	_vm->_oldRects.clear();
	_sequence = 2300;
	_vm->_sound->playSound(0);

	do {
		cLoop();
		_sequence = 2300;
	} while (_vm->_flags[52] == 3);

	_vm->freeCells();
	spriteData = _vm->_files->loadFile(14, 9);
	_vm->_objectsTable[3] = new SpriteResource(_vm, spriteData);
	delete spriteData;

	_vm->_screen->setDisplayScan();
	_vm->_events->hideCursor();

	_vm->_files->loadScreen(14, 3);
	_vm->_screen->setPalette();
	_vm->_buffer2.blitFrom(*_vm->_screen);
	_vm->_buffer1.blitFrom(*_vm->_screen);
	_vm->_events->showCursor();

	_vm->_screen->setIconPalette();
	_vm->_images.clear();
	_vm->_oldRects.clear();
	_sequence = 2400;

	do {
		cLoop();
		_sequence = 2400;
	} while (_vm->_flags[52] == 4);
}

void AmazonScripts::mWhile2() {
	_vm->_screen->setDisplayScan();
	_vm->_screen->fadeOut();
	_vm->_events->hideCursor();

	_vm->_files->loadScreen(14, 0);
	_vm->_buffer2.blitFrom(*_vm->_screen);
	_vm->_buffer1.blitFrom(*_vm->_screen);
	_vm->_events->showCursor();

	_vm->_screen->setIconPalette();
	_vm->_screen->forceFadeIn();

	Resource *spriteData = _vm->_files->loadFile(14, 6);
	_vm->_objectsTable[0] = new SpriteResource(_vm, spriteData);
	delete spriteData;

	_vm->_images.clear();
	_vm->_oldRects.clear();
	_sequence = 2100;

	do {
		cLoop();
		_sequence = 2100;
	} while (_vm->_flags[52] == 1);

	_vm->_screen->fadeOut();
	_vm->freeCells();
	spriteData = _vm->_files->loadFile(14, 9);
	_vm->_objectsTable[3] = new SpriteResource(_vm, spriteData);
	delete spriteData;

	_vm->_screen->setDisplayScan();
	_vm->_events->hideCursor();

	_vm->_files->loadScreen(14, 3);
	_vm->_screen->setPalette();
	_vm->_buffer2.blitFrom(*_vm->_screen);
	_vm->_buffer1.blitFrom(*_vm->_screen);
	_vm->_events->showCursor();

	_vm->_screen->setIconPalette();
	_vm->_images.clear();
	_vm->_oldRects.clear();
	_sequence = 2400;

	do {
		cLoop();
		_sequence = 2400;
	} while (_vm->_flags[52] == 4);
}

void AmazonScripts::mWhile(int param1) {
	switch(param1) {
	case 1:
		mWhile1();
		break;
	case 2:
		_game->_plane->mWhileFly();
		break;
	case 3:
		_game->_plane->mWhileFall();
		break;
	case 4:
		_game->_jungle->mWhileJWalk();
		break;
	case 5:
		_game->_jungle->mWhileDoOpen();
		break;
	case 6:
		_game->_river->mWhileDownRiver();
		break;
	case 7:
		mWhile2();
		break;
	case 8:
		_game->_jungle->mWhileJWalk2();
		break;
	default:
		break;
	}
}

void AmazonScripts::loadBackground(int param1, int param2) {
	_vm->_files->_setPaletteFlag = false;
	_vm->_files->loadScreen(param1, param2);

	_vm->_buffer2.blitFrom(*_vm->_screen);
	_vm->_buffer1.blitFrom(*_vm->_screen);

	_vm->_screen->forceFadeIn();
}

void AmazonScripts::loadNSound(int param1, int param2) {
	Resource *sound = _vm->_files->loadFile(param1, param2);
	_vm->_sound->_soundTable.push_back(SoundEntry(sound, 1));
}

void AmazonScripts::setInactive() {
	_game->_rawInactiveX = _vm->_player->_rawPlayer.x;
	_game->_rawInactiveY = _vm->_player->_rawPlayer.y;
	_game->_charSegSwitch = false;

	mWhile(_game->_rawInactiveY);
}

void AmazonScripts::boatWalls(int param1, int param2) {
	if (param1 == 1)
		_vm->_room->_plotter._walls[42] = Common::Rect(96, 27, 96 + 87, 27 + 42);
	else {
		_vm->_room->_plotter._walls[39].bottom = _vm->_room->_plotter._walls[41].bottom = 106;
		_vm->_room->_plotter._walls[40].left = 94;
	}
}

void AmazonScripts::plotInactive() {
	Player &player = *_vm->_player;
	InactivePlayer &inactive = _game->_inactive;

	if (_game->_charSegSwitch) {
		_game->_currentCharFlag = true;
		SWAP(inactive._altSpritesPtr, player._playerSprites);
		_game->_charSegSwitch = false;
	} else if (_game->_jasMayaFlag != (_game->_currentCharFlag ? 1 : 0)) {
		if (player._playerOff) {
			_game->_jasMayaFlag = (_game->_currentCharFlag ? 1 : 0);
		} else {
			_game->_currentCharFlag = (_game->_jasMayaFlag == 1);
			int tmpX = _game->_rawInactiveX;
			int tmpY = _game->_rawInactiveY;
			_game->_rawInactiveX = player._rawPlayer.x;
			_game->_rawInactiveY = player._rawPlayer.y;
			player._rawPlayer.x = tmpX;
			player._rawPlayer.y = tmpY;
			_game->_inactiveYOff = player._playerOffset.y;
			player.calcManScale();

			SWAP(inactive._altSpritesPtr, player._playerSprites);
			_vm->_room->setWallCodes();
		}
	}

	_game->_flags[155] = 0;
	if (_game->_rawInactiveX >= 152 && _game->_rawInactiveX <= 167 &&
			_game->_rawInactiveY >= 158 && _game->_rawInactiveY <= 173) {
		_game->_flags[155] = 1;
	} else {
		_game->_flags[160] = 0;
		if (!_game->_jasMayaFlag && _game->_rawInactiveX >= 266 && _game->_rawInactiveX <= 290
			&& _game->_rawInactiveY >= 70 && _game->_rawInactiveY <= 87) {
			_game->_flags[160] = 1;
		}
	}

	inactive._flags &= ~IMGFLAG_UNSCALED;
	inactive._flags &= ~IMGFLAG_BACKWARDS;
	inactive._position.x = _game->_rawInactiveX;
	inactive._position.y = _game->_rawInactiveY - _game->_inactiveYOff;
	inactive._offsetY = _game->_inactiveYOff;
	inactive._spritesPtr = inactive._altSpritesPtr;

	_vm->_images.addToList(_game->_inactive);
}

void AmazonScripts::executeSpecial(int commandIndex, int param1, int param2) {
	switch (commandIndex) {
	case 0:
		warning("TODO: DEMO - RESETAN");
		break;
	case 1:
		_vm->establish(param1, param2);
		break;
	case 2:
		loadBackground(param1, param2);
		break;
	case 3:
		if (_vm->isDemo())
			warning("TODO: DEMO - LOADCELLSET");
		else
			_game->_cast->doCast(param1);
		break;
	case 4:
		if (_vm->isDemo())
			loadNSound(param1, param2);
		else
			setInactive();
		break;
	case 5:
		warning("TODO: DEMO - UNLOADCELLSET");
		break;
	case 6:
		mWhile(param1);
		break;
	case 7:
		warning("TODO: DEMO - ADDMONEY");
		break;
	case 8:
		warning("TODO: DEMO - CHKMONEY");
		break;
	case 9:
		_game->_guard->doGuard();
		break;
	case 10:
		_vm->_midi->newMusic(param1, param2);
		break;
	case 11:
		plotInactive();
		break;
	case 13:
		_game->_river->doRiver();
		break;
	case 14:
		_game->_ant->doAnt();
		break;
	case 15:
		boatWalls(param1, param2);
		break;
	default:
		warning("Unexpected Special code %d - Skipped", commandIndex);
	}
}

typedef void(AmazonScripts::*AmazonScriptMethodPtr)();

void AmazonScripts::executeCommand(int commandIndex) {
	static const AmazonScriptMethodPtr AMAZON_COMMAND_LIST[] = {
		&AmazonScripts::cmdHelp_v2, &AmazonScripts::cmdCycleBack,
		&AmazonScripts::cmdChapter, &AmazonScripts::cmdSetHelp,
		&AmazonScripts::cmdCenterPanel, &AmazonScripts::cmdMainPanel,
		&AmazonScripts::CMDRETFLASH
	};

	if (commandIndex >= 73)
		(this->*AMAZON_COMMAND_LIST[commandIndex - 73])();
	else
		Scripts::executeCommand(commandIndex);
}

void AmazonScripts::cmdHelp_v2() {
	Common::String helpMessage = readString();

	if (_game->_helpLevel == 0) {
		_game->_timers.saveTimers();
		_game->_useItem = 0;

		if (_game->_noHints) {
			printString(AMRES.NO_HELP_MESSAGE);
			return;
		} else if (_game->_hintLevel == 0) {
			printString(AMRES.NO_HINTS_MESSAGE);
			return;
		}
	}

	int level = _game->_hintLevel - 1;
	if (level < _game->_helpLevel)
		_game->_moreHelp = 0;

	_game->drawHelp(helpMessage);

	while (!_vm->shouldQuit()) {
		while (!_vm->shouldQuit() && !_vm->_events->_leftButton)
			_vm->_events->pollEventsAndWait();

		_vm->_events->debounceLeft();

		static const Common::Rect butn1 = Common::Rect(HELP1COORDS[0][0], HELP1COORDS[0][2], HELP1COORDS[0][1], HELP1COORDS[0][3]);
		static const Common::Rect butn2 = Common::Rect(HELP1COORDS[1][0], HELP1COORDS[1][2], HELP1COORDS[1][1], HELP1COORDS[1][3]);
		const Common::Point pt = _vm->_events->_mousePos;

		int choice = -1;
		if (butn1.contains(pt))
			choice = 0;
		else if (butn2.contains(pt))
			choice = 1;

		if (choice < 0)
			continue;

		if (choice == 1) {
			// Done button selected
			_game->_helpLevel = 0;
			_game->_moreHelp = 1;
			_game->_useItem = 0;
			_vm->_events->hideCursor();
			if (_vm->_screen->_vesaMode) {
				_vm->_screen->restoreScreen();
				_vm->_screen->setPanel(0);
			} else {
				_vm->_screen->fadeOut();
				_vm->_screen->clearBuffer();
			}

			_vm->_buffer2.copyTo(_vm->_screen);
			_vm->_screen->restorePalette();
			_vm->_screen->setPalette();
			_vm->_events->showCursor();

			delete _vm->_objectsTable[45];
			_vm->_objectsTable[45] = nullptr;
			_vm->_timers.restoreTimers();
			break;
		} else {
			// More button selected
			if ((_game->_moreHelp == 0) || (choice != 0))
				continue;
			++_game->_helpLevel;
			_game->_useItem = 1;
			break;
		}
	}
	findNull();
}

void AmazonScripts::cmdCycleBack() {
	if (_vm->_startup == -1)
		_vm->_screen->cyclePaletteBackwards();
}

void AmazonScripts::cmdChapter() {
	Resource *activeScript = nullptr;

	if (_vm->isDemo()) {
		cmdSetHelp();
	} else {
		int chapter = _data->readByte();

		if (!_vm->isCD()) {
			// For floppy version, the current script remains active even
			// after the end of the chapter start, so we need to save it
			activeScript = _resource;
			_resource = nullptr;
			_data = nullptr;
		}

		_game->startChapter(chapter);

		if (!_vm->isCD()) {
			assert(!_resource);
			setScript(activeScript, false);
		}
	}
}

void AmazonScripts::cmdSetHelp() {
	int arrayId = (_data->readUint16LE() & 0xFF) - 1;
	int helpId = _data->readUint16LE() & 0xFF;

	byte *help = _game->_helpTbl[arrayId];
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
