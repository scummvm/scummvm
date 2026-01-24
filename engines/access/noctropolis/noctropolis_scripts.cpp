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

// Unlike the previous games which just stop at any 0xE0 (SCRIPT_START_BYTE),
// Noctropolis skips to the next command using argument size.  This avoids
// mistakes from the 0xE0 appearing accidentally in arguments.
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


void NoctropolisScripts::executeSpecial(int commandIndex, int param1, int param2) {
	switch (commandIndex) {
		case 1:
			_vm->_screen->fadeOut();
			break;
		case 3:
			error("TODO: Special 3: Call DisplayPegs_Tick");
		case 4:
			error("TODO: Special 4: Call MovePeg(%d)", param2);
		case 5:
			error("TODO: Special 5: Call SucAttack()");
		case 6:
			error("TODO: Special 6: Load DARKSHEER player!");
		case 7:
			_vm->_midi->loadMusic(98, param2);
			break;
		case 8:
			error("TODO: Special 8: Dim palette");
		case 9:
			error("TODO: Special 9: DimPalette1(0x32,0xc0,0x2b);");
		case 10:
			error("TODO: Special 10: StilMorph");
		case 11:
			error("TODO: Special 11: Ending");
		case 12:
			error("TODO: Special 12: Split");
		case 13:
			error("TODO: Special 13: ShotoMean");
		case 14:
			error("TODO: Special 14: StingerSeq(0x62, param_2);");
		case 15:
			if (!_vm->_midi->isPlaying())
				_vm->_midi->resume();
			break;
		case 17:
			error("TODO: Special 17: EndGame");
		case 19:
			warning("TODO: work our correct step on Special 19 (flashpalette)");
			_vm->_screen->flashPalette(20);
		case 20:
			error("TODO: Special 20: DoLastComic");
		case 21:
			error("TODO: Special 20: DoSpecialComic");
		default:
			error("TODO: Unknown special %d in NoctropolisScripts::executeSpecial", commandIndex);
	}
}


} // end namespace Noctropolis

} // end namespace Access
