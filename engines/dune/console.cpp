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

#include "common/file.h"

#include "dune/condit.h"
#include "dune/console.h"
#include "dune/dialogue.h"
#include "dune/dune.h"

namespace Dune {

	Console::Console(DuneEngine *vm) : GUI::Debugger(), _vm(vm) {
	registerCmd("place", WRAP_METHOD(Console, cmdPlace));
	registerCmd("stage", WRAP_METHOD(Console, cmdStage));
	registerCmd("part", WRAP_METHOD(Console, cmdPart));
	registerCmd("condition", WRAP_METHOD(Console, cmdCondition));
	registerCmd("state", WRAP_METHOD(Console, cmdState));
	registerCmd("dump", WRAP_METHOD(Console, cmdDump));
	registerCmd("background", WRAP_METHOD(Console, cmdBackground));
	registerCmd("world", WRAP_METHOD(Console, cmdWorld));
	registerCmd("places", WRAP_METHOD(Console, cmdPlaces));
	registerCmd("troops", WRAP_METHOD(Console, cmdTroops));
	registerCmd("save", WRAP_METHOD(Console, cmdSave));
	registerCmd("load", WRAP_METHOD(Console, cmdLoad));
}

bool Console::cmdSave(int argc, const char **argv) {
	const int slot = argc > 1 ? atoi(argv[1]) : 1;

	const Common::Error error = _vm->saveGameState(slot, "a test of saving");
	if (error.getCode() != Common::kNoError) {
		debugPrintf("the game would not be saved: %s\n", error.getDesc().c_str());
		return true;
	}

	debugPrintf("the game is saved in slot %d\n", slot);
	return true;
}

bool Console::cmdLoad(int argc, const char **argv) {
	const int slot = argc > 1 ? atoi(argv[1]) : 1;

	const Common::Error error = _vm->loadGameState(slot);
	if (error.getCode() != Common::kNoError) {
		debugPrintf("the game would not be loaded: %s\n", error.getDesc().c_str());
		return true;
	}

	debugPrintf("the game is loaded from slot %d\n", slot);
	return false;
}

bool Console::cmdPlace(int argc, const char **argv) {
	if (argc < 2) {
		debugPrintf("The place is %02x: bank %d, scene %d\n", _vm->_place,
					(_vm->_place >> 4) & 0xf, _vm->_place & 0xf);
		debugPrintf("Usage: place <byte> [scene file]\n");
		return true;
	}

	_vm->_place = (byte)strtol(argv[1], nullptr, 16);
	if (argc > 2)
		_vm->_salFile = argv[2];

	_vm->drawScene();
	return false;
}

bool Console::cmdStage(int argc, const char **argv) {
	if (argc < 2) {
		debugPrintf("The story is at stage %d%s, the hour is %d, the spice is %d, "
					"Paul is thought of at %d\n",
					_vm->storyStage(), _vm->isOver() ? " (run its course)" : "",
					_vm->hour(), _vm->spice(), _vm->charisma());
		return true;
	}

	_vm->advanceStory((byte)atoi(argv[1]));
	debugPrintf("The story is at stage %d\n", _vm->storyStage());
	return true;
}

bool Console::cmdPart(int argc, const char **argv) {
	if (argc < 2) {
		debugPrintf("Reading part %u of %u\n", _vm->_dialoguePart, _vm->_dialogue.count());
		return true;
	}

	_vm->_dialoguePart = atoi(argv[1]);
	_vm->_dialogueLine = 0;
	_vm->sayNextLine();
	return false;
}

bool Console::cmdCondition(int argc, const char **argv) {
	if (argc < 2) {
		debugPrintf("Usage: condition <number>\n");
		return true;
	}

	const uint index = atoi(argv[1]);
	debugPrintf("%u: %s -> %s\n", index, _vm->_condit.describe(index).c_str(),
				_vm->conditionHolds(index) ? "holds" : "does not hold");
	return true;
}

bool Console::cmdState(int argc, const char **argv) {
	if (argc < 2) {
		debugPrintf("Usage: state <address> [value], both in hexadecimal\n");
		return true;
	}

	const uint address = strtol(argv[1], nullptr, 16);
	if (address >= _vm->_state.size()) {
		debugPrintf("The state is only %u bytes\n", _vm->_state.size());
		return true;
	}

	if (argc > 2)
		_vm->_state[address] = (byte)strtol(argv[2], nullptr, 16);

	debugPrintf("state[%04x] = %02x\n", address, _vm->_state[address]);
	return true;
}

bool Console::cmdBackground(int argc, const char **argv) {
	if (argc < 2) {
		debugPrintf("The room stands on %s\n",
					_vm->_background.empty() ? "nothing" : _vm->_background.c_str());
		debugPrintf("Usage: background <resource>, or none\n");
		return true;
	}

	_vm->_background = strcmp(argv[1], "none") == 0 ? Common::String() : argv[1];
	_vm->drawScene();
	return false;
}

bool Console::cmdWorld(int argc, const char **argv) {
	_vm->showWorld();
	return false;
}

bool Console::cmdPlaces(int argc, const char **argv) {
	const Common::Array<Place> &places = _vm->_map.places();

	for (uint i = 0; i != places.size(); i++) {
		const Place &place = places[i];
		if (argc < 2 && !place.isKnown())
			continue;

		debugPrintf("%2u: type %2d at %5d,%4d %s, %3d there, skills %d%d%d%d%d%d\n",
					i, place.type, place.longitude, place.latitude,
					place.isKnown() ? "known" : "unfound", place.people,
					place.skills[0], place.skills[1], place.skills[2],
					place.skills[3], place.skills[4], place.skills[5]);
	}

	const Common::Array<Troop> &troops = _vm->_map.troops();
	uint met = 0;

	for (uint i = 0; i != places.size(); i++) {
		const Common::Array<uint> here =
			_vm->_map.troopsAt(0x100 + i * 28, _vm->_state.begin(), _vm->_state.size());

		if (!here.empty()) {
			debugPrintf("place %2u has troops:", i);
			for (uint t = 0; t != here.size(); t++)
				debugPrintf(" %u", here[t]);
			debugPrintf("\n");
		}
	}

	for (uint i = 0; i != troops.size(); i++)
		met += troops[i].isKnown() ? 1 : 0;

	debugPrintf("%u places, %u of them known; %u troops, %u of them met\n",
				places.size(), argc < 2 ? 0u : places.size(), troops.size(), met);
	return true;
}

bool Console::cmdTroops(int argc, const char **argv) {
	const Common::Array<Troop> &troops = _vm->_map.troops();

	// troops <number> <order> gives a troop its orders.
	if (argc > 2) {
		const uint which = atoi(argv[1]);
		const byte order = (byte)atoi(argv[2]) & 0xf;

		if (which >= troops.size()) {
			debugPrintf("There are only %u troops\n", troops.size());
			return true;
		}

		const uint at = 0x8aa + which * 27 + 3;
		_vm->_state[at] = (_vm->_state[at] & 0xf0) | order;
		_vm->_map.readTroops(_vm->_state.begin(), _vm->_state.size());

		debugPrintf("Troop %u is now to be %s\n", which, _vm->orderName(order).c_str());
		return true;
	}

	for (uint i = 0; i != troops.size(); i++) {
		const Troop &troop = troops[i];
		if (argc < 2 && !troop.isKnown())
			continue;

		debugPrintf("%2u: %-5s state %02x, %-20s rate %3d, worth %3d, %d of them\n",
					i, troop.isKnown() ? "met" : "unmet", troop.state,
					_vm->orderName(troop.order()).c_str(), troop.rate,
					_vm->troopRate(troop), troop.size);
	}

	return true;
}

bool Console::cmdDump(int argc, const char **argv) {
	if (argc < 2) {
		debugPrintf("Usage: dump <resource> - writes it out unpacked\n");
		return true;
	}

	Resource resource = _vm->_archive.openMember(argv[1]);
	if (!resource) {
		debugPrintf("There is no %s\n", argv[1]);
		return true;
	}

	Common::DumpFile out;
	if (!out.open(Common::Path(Common::String(argv[1]) + ".dump"))) {
		debugPrintf("Could not write the file\n");
		return true;
	}

	const uint32 size = resource->size();
	byte *data = new byte[size];
	resource->read(data, size);
	out.write(data, size);
	delete[] data;

	debugPrintf("%s: %u bytes\n", argv[1], size);
	return true;
}

} // End of namespace Dune
