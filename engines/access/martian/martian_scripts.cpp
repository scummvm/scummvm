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
#include "access/martian/martian_game.h"
#include "access/martian/martian_resources.h"
#include "access/martian/martian_scripts.h"

namespace Access {

namespace Martian {

MartianScripts::MartianScripts(AccessEngine *vm) : Scripts(vm) {
	_game = (MartianEngine *)_vm;
}

void MartianScripts::cmdSpecial1(int param1) {
	_vm->_events->hideCursor();
	
	if (param1 != -1) {
		_vm->_files->loadScreen(49, param1);
		_vm->_buffer2.copyBuffer(_vm->_screen);
	}

	_vm->_screen->setIconPalette();
	_vm->_screen->forceFadeIn();
	_vm->_events->showCursor();
}

void MartianScripts::cmdSpecial3() {
	_vm->_screen->forceFadeOut();
	_vm->_events->hideCursor();
	_vm->_files->loadScreen(57, 3);
	_vm->_buffer2.copyFrom(*_vm->_screen);

	_vm->_screen->setIconPalette();
	_vm->_events->showCursor();
	_vm->_screen->forceFadeIn();
}

void MartianScripts::doIntro(int param1) {
	_game->doSpecial5(param1);
}

void MartianScripts::cmdSpecial6() {
	_vm->_midi->stopSong();
	_vm->_screen->setDisplayScan();
	_vm->_events->clearEvents();
	_vm->_screen->forceFadeOut();
	_vm->_events->hideCursor();
	_vm->_files->loadScreen(49, 9);
	_vm->_events->showCursor();
	_vm->_screen->setIconPalette();
	_vm->_screen->forceFadeIn();

	Resource *cellsRes = _vm->_files->loadFile("CELLS00.LZ");
	_vm->_objectsTable[0] = new SpriteResource(_vm, cellsRes);
	delete cellsRes;

	_vm->_timers[20]._timer = _vm->_timers[20]._initTm = 30;
	_vm->_fonts._charSet._lo = 1;
	_vm->_fonts._charSet._hi = 10;
	_vm->_fonts._charFor._lo = 1;
	_vm->_fonts._charFor._hi = 255;

	_vm->_screen->_maxChars = 50;
	_vm->_screen->_printOrg = _vm->_screen->_printStart = Common::Point(24, 18);

	Resource *notesRes = _vm->_files->loadFile("ETEXT.DAT");
	notesRes->_stream->seek(72);

	// Read the message
	Common::String msg = "";
	byte c;
	while ((c = (char)notesRes->_stream->readByte()) != '\0')
		msg += c;

	//display the message
	_game->sub13D10(msg);

	delete notesRes;
	delete _vm->_objectsTable[0];
	_vm->_objectsTable[0] = nullptr;
	_vm->_midi->stopSong();
}

void MartianScripts::executeSpecial(int commandIndex, int param1, int param2) {
	switch (commandIndex) {
	case 0:
		warning("TODO: cmdSpecial0");
		break;
	case 1:
		cmdSpecial1(param1);
		break;
	case 2:
		warning("TODO: cmdSpecial2");
		break;
	case 3:
		cmdSpecial3();
		break;
	case 4:
		warning("TODO: cmdSpecial4");
		break;
	case 5:
		doIntro(param1);
		break;
	case 6:
		cmdSpecial6();
		break;
	case 7:
		warning("TODO: cmdSpecial7");
		break;
	default:
		warning("Unexpected Special code %d - Skipped", commandIndex);
	}
}

typedef void(MartianScripts::*MartianScriptMethodPtr)();

void MartianScripts::executeCommand(int commandIndex) {
	Scripts::executeCommand(commandIndex);
}

} // End of namespace Martian

} // End of namespace Access
