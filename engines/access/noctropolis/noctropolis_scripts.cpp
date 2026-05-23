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

#include "access/noctropolis/noctropolis_scripts.h"
#include "access/noctropolis/noctropolis_game.h"
#include "access/noctropolis/noctropolis_room.h"

namespace Access {

namespace Noctropolis {

NoctropolisScripts::NoctropolisScripts(NoctropolisEngine *vm) : Scripts(vm) {
	setOpcodes_v3();
}

static int COMMAND_ARG_SIZES[] = {
	-1, 0, 2, 2, 2, 2, 2, 2, 2, -1,
	0, 1, 2, 6, 2, 1, 2, 6, 4, 1,
	2, 6, 4, 1, 0, 0, 10, 1, 0, 2,
	4, 6, 2, 2, 8, 0, 8, 0, 0, 4,
	0, 2, 8, 0, 0, 0, 6, 6, 6, 0,
	-2, -2, -3, 2, 8, 4, 4, 0, 0, 2,
	0, 10, 0, -1, 0, 2, 6, 8, 0, 0,
	1, 1, 0, 0, -2, 0, 1, 6, 2, 2,
	0, 4, 2, 8, 0, 6, 2, 0, 0, 0,
	4, 1, 1, 2, 0, 0,
};

void NoctropolisScripts::skipCommand(byte cmd) {
	assert(cmd < ARRAYSIZE(COMMAND_ARG_SIZES));
	int argsize = COMMAND_ARG_SIZES[cmd];
	switch (argsize) {
	case -1:
		_data->readString();
		break;
	case -2:
		_data->readUint16LE();
		_data->readUint16LE();
		_data->readString();
		break;
	case -3:
		_data->readUint16LE();
		_data->readUint16LE();
		_data->readString();
		_data->readString();
		_data->readString();
		_data->readString();
		_data->readString();
		_data->readString();
		break;
	default:
		_data->seek(argsize, SEEK_CUR);
	}
}

//
// Unlike the previous games which just stop at any 0xE0 (SCRIPT_START_BYTE),
// Noctropolis skips to the next command using argument size.  This avoids
// mistakes from the 0xE0 appearing accidentally in arguments.
//
void NoctropolisScripts::searchForSequence() {
	assert(_data);

	_data->seek(0);
	int sequenceId;
	while (_data->pos() < _data->size()) {
		byte command = _data->readByte();
		if (command == 0xe0) {
			sequenceId = _data->readUint16LE();
			if (sequenceId == _sequence)
				break;
		} else {
			skipCommand(command ^ 0x80);
		}
	}
	if (_data->pos() == _data->size()) {
		warning("Hit EOS when searching for sequence %d", _sequence);
		_endFlag = true;
	}
}


bool NoctropolisScripts::executeSpecial(int commandIndex, int param1, int param2) {
	NoctropolisEngine *vm = ((NoctropolisEngine *)_vm);
	bool result = false;
	switch (commandIndex) {
	case 1:
		// Clear screen and restore pal here??
		vm->_screen->fadeOutThenClearAndSetPal();
		vm->_buffer2.clear();
		result = true;
		if (_vm->_player->_roomNumber == 54) {
			warning("TODO: Implement special pal hack for fade in room 54?");
			/*
			NoctPal_ClearRawPalette();
			pbVar4 = GetRawPalette();
			uVar3 = GetRawPaletteHash();
			RB_GetPlayfieldTexture(pbVar4,uVar3);
			NoctRoomEngine::clearPlotVids(&_gNoctRoomEngine);
			*/
		}
		break;
	case 3:
		vm->displayPegsTick();
		break;
	case 4:
		vm->movePeg(param1); // todo: should it be param2?
		break;
	case 5:
		vm->playSuccubusAttack();
		break;
	case 6: // Set player model
		((NoctropolisRoom *)vm->_room)->loadPlayer1();
		_vm->_player->load();
		_vm->_screen->setManPalette();
		_vm->_screen->setPalette();
		break;
	case 7:
		vm->_midi->loadMusic(98, param1);
		break;
	case 8:
		vm->_screen->setDarkPalette(param1 | (param1 << 8), 0, 236);
		break;
	case 9:
		vm->_screen->setDarkPalette(0x3200, 192, 43);
		break;
	case 10:
		vm->playStilMorph();
		break;
	case 11:
		vm->showComicCover();
		break;
	case 12:
		vm->playSuccubusSplit();
		break;
	case 13:
		vm->shotoMeanwhile();
		break;
	case 14:
		// Load some temp music
		_vm->_midi->newMusic(param1, 0);
		break;
	case 15:
		// Resume previous music
		if (!_vm->_midi->isPlaying())
			_vm->_midi->newMusic(0, 1);
		break;
	case 16:
		// This gets called at the start of the end credits
		((VideoPlayer_v2 *)_vm->_video)->setVideoPalNow();
		break;
	case 17:
		vm->showNightdiveCredits();
		_vm->_restartFl = true;
		break;
	case 18:
		// End of demo
		vm->doUpsell();
		_vm->quitGame();
		break;
	case 19:
		warning("TODO: work our correct step on Special 19 (flashpalette)");
		_vm->_screen->flashPalette(20);
		break;
	case 20:
		vm->doLastComic();
		result = true;
		break;
	case 21:
		vm->doSpecialComic();
		result = true;
		break;
	default:
		error("Unknown special %d in NoctropolisScripts::executeSpecial", commandIndex);
	}
	return result;
}


} // end namespace Noctropolis

} // end namespace Access
