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
 * $URL$
 * $Id$
 *
 */

#include "mohawk/myst.h"
#include "mohawk/myst_saveload.h"

#include "common/util.h"

namespace Mohawk {

MystSaveLoad::MystSaveLoad(MohawkEngine_Myst *vm, Common::SaveFileManager *saveFileMan) : _vm(vm), _saveFileMan(saveFileMan) {
	_v = new MystVariables();
	initMystVariables(_v);
}

MystSaveLoad::~MystSaveLoad() {
	delete _v;
}

Common::StringList MystSaveLoad::generateSaveGameList() {
	return _saveFileMan->listSavefiles("*.mys");
}

bool MystSaveLoad::loadGame(Common::String filename) {
	if (_vm->getFeatures() & GF_DEMO) // Don't load games in the demo
		return false;

	Common::InSaveFile *loadFile;
	if (!(loadFile = _saveFileMan->openForLoading(filename.c_str())))
		return false;
	debugC(kDebugSaveLoad, "Loading game from \'%s\'", filename.c_str());

	// First, let's make sure we're using a saved game file from this version of Myst
	// By checking length of file...
	int32 size = loadFile->size();
	if ((size == -1)
	    || (size != 664 && (_vm->getFeatures() & GF_ME))
	    || (size != 601 && !(_vm->getFeatures() & GF_ME))) {
		warning ("Incompatible saved game version");
        // FIXME - Add Support to load original game saves in ME and vice versa
		delete loadFile;
		return false;
	}

	// Now, we'll read in the variable values.
	// Lots of checking code here so that save files with differing formats are flagged...
	if ((_v->game_globals[0] = loadFile->readUint16LE()) != 2)
		warning("Unexpected value at 0x%03X - Found %u Expected %u", loadFile->pos(), _v->game_globals[0], 2);

	_v->game_globals[1] = loadFile->readUint16LE();
	_v->game_globals[2] = loadFile->readUint16LE();

	if ((_v->game_globals[3] = loadFile->readUint16LE()) != 1)
		warning("Unexpected value at 0x%03X - Found %u Expected %u", loadFile->pos(), _v->game_globals[3], 1);

	_v->game_globals[4] = loadFile->readUint16LE();
	_v->game_globals[5] = loadFile->readUint16LE();
	_v->game_globals[6] = loadFile->readUint16LE();
	_v->game_globals[7] = loadFile->readUint16LE();

	for (byte i = 0; i < 8; i++) {
		if (_vm->getFeatures() & GF_ME) {
			_v->myst_vars[i] = loadFile->readUint16LE();

			if (loadFile->readUint16LE() != 0)
				warning("Non-zero value at 0x%03X", loadFile->pos());
		} else
			_v->myst_vars[i] = loadFile->readByte();

		if (_v->myst_vars[i] > 1)
			warning("Non-Boolean value found at 0x%03X - Found %u", loadFile->pos(), _v->myst_vars[i]);
	}

	if ((_v->myst_vars[8] = loadFile->readUint16LE()) > 1)
		warning("Non-Boolean value found at 0x%03X - Found %u", loadFile->pos(), _v->myst_vars[8]);

	if ((_v->myst_vars[9] = loadFile->readUint16LE()) > 1)
		warning("Non-Boolean value found at 0x%03X - Found %u", loadFile->pos(), _v->myst_vars[9]);

	if ((_v->myst_vars[10] = loadFile->readUint16LE()) > 25)
		warning("Value exceeds maximum of %u found at 0x%03X - Found %u", 25, loadFile->pos(), _v->myst_vars[10]);

	if ((_v->myst_vars[11] = loadFile->readUint16LE()) > 11)
		warning("Value exceeds maximum of %u found at 0x%03X - Found %u", 11, loadFile->pos(), _v->myst_vars[11]);

	_v->myst_vars[12] = loadFile->readUint16LE();
	// TODO: Add validation of valid set for Clock Tower Hour Hand

	if ((_v->myst_vars[13] = loadFile->readUint16LE()) > 1)
		warning("Non-Boolean value found at 0x%03X - Found %u", loadFile->pos(), _v->myst_vars[13]);

	if ((_v->myst_vars[14] = loadFile->readUint16LE()) > 1)
		warning("Non-Boolean value found at 0x%03X - Found %u", loadFile->pos(), _v->myst_vars[14]);

	if ((_v->myst_vars[15] = loadFile->readUint16LE()) > 2)
		warning("Value exceeds maximum of %u found at 0x%03X - Found %u", 2, loadFile->pos(), _v->myst_vars[15]);

	_v->myst_vars[16] = loadFile->readUint16LE();
	_v->myst_vars[17] = loadFile->readUint16LE();

	if ((_v->myst_vars[18] = loadFile->readUint16LE()) > 1)
		warning("Non-Boolean value found at 0x%03X - Found %u", loadFile->pos(), _v->myst_vars[18]);

	_v->myst_vars[19] = loadFile->readUint16LE();

	if ((_v->myst_vars[20] = loadFile->readUint16LE()) > 1)
		warning("Non-Boolean value found at 0x%03X - Found %u", loadFile->pos(), _v->myst_vars[20]);

	if ((_v->myst_vars[21] = loadFile->readUint16LE()) != 0)
		warning("Non-zero value at 0x%03X - Found %u", loadFile->pos(), _v->myst_vars[21]);

	if ((_v->myst_vars[22] = loadFile->readUint16LE()) != 0)
		warning("Non-zero value at 0x%03X - Found %u", loadFile->pos(), _v->myst_vars[22]);

	if ((_v->myst_vars[23] = loadFile->readUint16LE()) != 0)
		warning("Non-zero value at 0x%03X - Found %u", loadFile->pos(), _v->myst_vars[23]);

	if ((_v->myst_vars[24] = loadFile->readUint16LE()) != 0)
		warning("Non-zero value at 0x%03X - Found %u", loadFile->pos(), _v->myst_vars[24]);

	if ((_v->myst_vars[25] = loadFile->readUint16LE()) > 359)
		warning("Value exceeds maximum of %u found at 0x%03X - Found %u", 359, loadFile->pos(), _v->myst_vars[25]);

	_v->myst_vars[26] = loadFile->readUint16LE();

	if ((_v->myst_vars[27] = loadFile->readUint16LE()) > 1)
		warning("Non-Boolean value found at 0x%03X - Found %u", loadFile->pos(), _v->myst_vars[27]);

	_v->myst_vars[28] = loadFile->readUint16LE();
	if (_v->myst_vars[28] < 1 && _v->myst_vars[28] > 31)
		warning("Out of Range value found at 0x%03X - Found %u", loadFile->pos(), _v->myst_vars[28]);

	if ((_v->myst_vars[29] = loadFile->readUint16LE()) > 1)
		warning("Non-Boolean value found at 0x%03X - Found %u", loadFile->pos(), _v->myst_vars[29]);

	if ((_v->myst_vars[30] = loadFile->readUint16LE()) > 11)
		warning("Value exceeds maximum of %u found at 0x%03X - Found %u", 11, loadFile->pos(), _v->myst_vars[30]);

	if ((_v->myst_vars[31] = loadFile->readUint16LE()) > (24 * 60))
		warning("Value exceeds maximum of %u found at 0x%03X - Found %u", (24 * 60), loadFile->pos(), _v->myst_vars[31]);

	_v->myst_vars[32] = loadFile->readUint16LE();

	_v->myst_vars[33] = loadFile->readUint16LE();
	if (_v->myst_vars[33] < 1 && _v->myst_vars[33] > 31)
		warning("Out of Range value found at 0x%03X - Found %u", loadFile->pos(), _v->myst_vars[33]);

	if ((_v->myst_vars[34] = loadFile->readUint16LE()) > 11)
		warning("Value exceeds maximum of %u found at 0x%03X - Found %u", 11, loadFile->pos(), _v->myst_vars[34]);

	if ((_v->myst_vars[35] = loadFile->readUint16LE()) > (24 * 60))
		warning("Value exceeds maximum of %u found at 0x%03X - Found %u", (24 * 60), loadFile->pos(), _v->myst_vars[35]);

	_v->myst_vars[36] = loadFile->readUint16LE();

	if ((_v->myst_vars[37] = loadFile->readUint16LE()) > 999)
		warning("Value exceeds maximum of %u found at 0x%03X - Found %u", 999, loadFile->pos(), _v->myst_vars[37]);

	if ((_v->myst_vars[38] = loadFile->readUint16LE()) > 12)
		warning("Value exceeds maximum of %u found at 0x%03X - Found %u", 12, loadFile->pos(), _v->myst_vars[38]);

	_v->myst_vars[39] = loadFile->readUint16LE();
	_v->myst_vars[40] = loadFile->readUint16LE();

	_v->myst_vars[41] = loadFile->readUint16LE();
	_v->myst_vars[42] = loadFile->readUint16LE();
	_v->myst_vars[43] = loadFile->readUint16LE();
	_v->myst_vars[44] = loadFile->readUint16LE();
	_v->myst_vars[45] = loadFile->readUint16LE();

	_v->myst_vars[46] = loadFile->readUint16LE();
	_v->myst_vars[47] = loadFile->readUint16LE();
	_v->myst_vars[48] = loadFile->readUint16LE();
	_v->myst_vars[49] = loadFile->readUint16LE();

	for (byte i = 0; i < 4; i++) {
		if (_vm->getFeatures() & GF_ME) {
			_v->channelwood_vars[i] = loadFile->readUint16LE();

			if (loadFile->readUint16LE() != 0)
				warning("Non-zero value at 0x%03X", loadFile->pos());
		} else
			_v->channelwood_vars[i] = loadFile->readByte();

		if (_v->channelwood_vars[i] > 1)
			warning("Non-Boolean value found at 0x%03X - Found %u", loadFile->pos(), _v->channelwood_vars[i]);
	}

	_v->channelwood_vars[4] = loadFile->readUint16LE();

	if ((_v->channelwood_vars[5] = loadFile->readUint16LE()) > 3)
		warning("Value exceeds maximum of %u found at 0x%03X - Found %u", 3, loadFile->pos(), _v->channelwood_vars[5]);

	if ((_v->channelwood_vars[6] = loadFile->readUint16LE()) > 1)
		warning("Non-Boolean value found at 0x%03X - Found %u", loadFile->pos(), _v->channelwood_vars[6]);

	if (_vm->getFeatures() & GF_ME) {
		if (loadFile->readUint16LE() != 0)
			warning("Non-zero value at 0x%03X", loadFile->pos());

		if (loadFile->readUint16LE() != 0)
			warning("Non-zero value at 0x%03X", loadFile->pos());
	} else if (loadFile->readByte() != 0)
		warning("Non-zero value at 0x%03X", loadFile->pos());

	for (byte i = 0; i < 3; i++)
		if ((_v->mech_vars[i] = loadFile->readUint16LE()) > 1)
			warning("Non-Boolean value found at 0x%03X - Found %u", loadFile->pos(), _v->mech_vars[i]);

	_v->mech_vars[3] = loadFile->readUint16LE();
	for (byte i = 4; i < 8; i++)
		if ((_v->mech_vars[i] = loadFile->readUint16LE()) > 9)
			warning("Value exceeds maximum of %u found at 0x%03X - Found %u", 9, loadFile->pos(), _v->mech_vars[i]);

	for (byte i = 0; i < 7; i++) {
		if (_vm->getFeatures() & GF_ME) {
			_v->selenitic_vars[i] = loadFile->readUint16LE();

			if (loadFile->readUint16LE() != 0)
				warning("Non-zero value at 0x%03X", loadFile->pos());
		} else
			_v->selenitic_vars[i] = loadFile->readByte();

		if (_v->selenitic_vars[i] > 1)
			warning("Non-Boolean value found at 0x%03X - Found %u", loadFile->pos(), _v->selenitic_vars[i]);
	}

	for(byte i = 7; i < 18; i++)
		_v->selenitic_vars[i] = loadFile->readUint16LE();

	for (byte i = 0; i < 3; i++) {
		if (_vm->getFeatures() & GF_ME) {
			_v->stoneship_vars[i] = loadFile->readUint16LE();
		} else
			_v->stoneship_vars[i] = loadFile->readByte();

		if (_v->stoneship_vars[i] > 1)
			warning("Non-Boolean value found at 0x%03X - Found %u", loadFile->pos(), _v->selenitic_vars[i]);
	}
	for (byte i = 3; i < 14; i++)
		_v->stoneship_vars[i] = loadFile->readUint16LE();

	for (byte i = 0; i < 1; i++)
		_v->dunny_vars[i] = loadFile->readUint16LE();

	// Reading unknown region...
	// When Zero Value regions are included, these are 5 blocks of
	// 41 uint16 values.

	for (byte i = 0; i < 31; i++)
		_v->unknown_myst[i] = loadFile->readUint16LE();

	for (byte i = 0; i < 10; i++) {
		if (loadFile->readUint16LE() != 0)
			warning("Non-zero value at 0x%03X", loadFile->pos());
	}

	for (byte i = 0; i < 37; i++)
		_v->unknown_channelwood[i] = loadFile->readUint16LE();

	for (byte i = 0; i < 4; i++) {
		if (loadFile->readUint16LE() != 0)
			warning("Non-zero value at 0x%03X", loadFile->pos());
	}

	for (byte i = 0; i < 18; i++)
		_v->unknown_mech[i] = loadFile->readUint16LE();

	for (byte i = 0; i < 23; i++) {
		if (loadFile->readUint16LE() != 0)
			warning("Non-zero value at 0x%03X", loadFile->pos());
	}

	for (byte i = 0; i < 30; i++)
		_v->unknown_selenitic[i] = loadFile->readUint16LE();

	for (byte i = 0; i < 11; i++) {
		if (loadFile->readUint16LE() != 0)
			warning("Non-zero value at 0x%03X", loadFile->pos());
	}

	for (byte i = 0; i < 22; i++)
		_v->unknown_stoneship[i] = loadFile->readUint16LE();

	for (byte i = 0; i < 19; i++) {
		if (loadFile->readUint16LE() != 0)
			warning("Non-zero value at 0x%03X", loadFile->pos());
	}

	if (_vm->getFeatures() & GF_ME) {
		if (loadFile->pos() != 664)
			warning("Unexpected File Position 0x%03X At End of Load", loadFile->pos());
	} else {
		if (loadFile->pos() != 601)
			warning("Unexpected File Position 0x%03X At End of Load", loadFile->pos());
	}

	delete loadFile;

	debug_printMystVariables(_v);

	return true;
}

bool MystSaveLoad::saveGame(Common::String filename) {
	// Make sure we have the right extension
	if (!filename.hasSuffix(".mys") && !filename.hasSuffix(".MYS"))
		filename += ".mys";

	Common::OutSaveFile *saveFile;
	if (!(saveFile = _saveFileMan->openForSaving(filename.c_str())))
		return false;
	debugC(kDebugSaveLoad, "Saving game to \'%s\'", filename.c_str());

	debug_printMystVariables(_v);

	// Performs no validation of variable values - Assumes they are valid.
	for (byte i = 0; i < 8; i++)
		saveFile->writeUint16LE(_v->game_globals[i]);

	for (byte i = 0; i < 8; i++) {
		if (_vm->getFeatures() & GF_ME) {
			saveFile->writeUint16LE(_v->myst_vars[i]);
			saveFile->writeUint16LE(0);
		} else
			saveFile->writeByte(_v->myst_vars[i]);
	}

	for (byte i = 8; i < 50; i++)
		saveFile->writeUint16LE(_v->myst_vars[i]);

	for (byte i = 0; i < 4; i++) {
		if (_vm->getFeatures() & GF_ME) {
			saveFile->writeUint16LE(_v->channelwood_vars[i]);
			saveFile->writeUint16LE(0);
		} else
			saveFile->writeByte(_v->channelwood_vars[i]);
	}

	for (byte i = 4; i < 7; i++)
		saveFile->writeUint16LE(_v->channelwood_vars[i]);

	if (_vm->getFeatures() & GF_ME) {
		saveFile->writeUint16LE(0);
		saveFile->writeUint16LE(0);
	} else
		saveFile->writeByte(0);

	for (byte i = 0; i < 8; i++)
		saveFile->writeUint16LE(_v->mech_vars[i]);

	for (byte i = 0; i < 7; i++) {
		if (_vm->getFeatures() & GF_ME) {
			saveFile->writeUint16LE(_v->selenitic_vars[i]);
			saveFile->writeUint16LE(0);
		} else
			saveFile->writeByte(_v->selenitic_vars[i]);
	}

	for(byte i = 7; i < 18; i++)
		saveFile->writeUint16LE(_v->selenitic_vars[i]);

	for (byte i = 0; i < 3; i++) {
		if (_vm->getFeatures() & GF_ME) {
			saveFile->writeUint16LE(_v->stoneship_vars[i]);
		} else
			saveFile->writeByte(_v->stoneship_vars[i]);
	}
	for (byte i = 3; i < 14; i++)
		saveFile->writeUint16LE(_v->stoneship_vars[i]);

	for (byte i = 0; i < 1; i++)
		saveFile->writeUint16LE(_v->dunny_vars[i]);

	for (byte i = 0; i < 31; i++)
		saveFile->writeUint16LE(_v->unknown_myst[i]);

	for (byte i = 0; i < 10; i++)
		saveFile->writeUint16LE(0);

	for (byte i = 0; i < 37; i++)
		saveFile->writeUint16LE(_v->unknown_channelwood[i]);

	for (byte i = 0; i < 4; i++)
		saveFile->writeUint16LE(0);

	for (byte i = 0; i < 18; i++)
		saveFile->writeUint16LE(_v->unknown_mech[i]);

	for (byte i = 0; i < 23; i++)
		saveFile->writeUint16LE(0);

	for (byte i = 0; i < 30; i++)
		saveFile->writeUint16LE(_v->unknown_selenitic[i]);

	for (byte i = 0; i < 11; i++)
		saveFile->writeUint16LE(0);

	for (byte i = 0; i < 22; i++)
		saveFile->writeUint16LE(_v->unknown_stoneship[i]);

	for (byte i = 0; i < 19; i++)
		saveFile->writeUint16LE(0);

	saveFile->finalize();

	delete saveFile;

	return true;
}

void MystSaveLoad::deleteSave(Common::String saveName) {
	debugC(kDebugSaveLoad, "Deleting save file \'%s\'", saveName.c_str());
	_saveFileMan->removeSavefile(saveName.c_str());
}

void MystSaveLoad::initMystVariables(MystVariables *_tv) {
	uint8 i;

	// Most of the variables are zero at game start.
	for (i = 0; i < ARRAYSIZE(_tv->game_globals); i++)
		_tv->game_globals[i] = 0;
	for (i = 0; i < ARRAYSIZE(_tv->myst_vars); i++)
		_tv->myst_vars[i] = 0;
	for (i = 0; i < ARRAYSIZE(_tv->channelwood_vars); i++)
		_tv->channelwood_vars[i] = 0;
	for (i = 0; i < ARRAYSIZE(_tv->mech_vars); i++)
		_tv->mech_vars[i] = 0;
	for (i = 0; i < ARRAYSIZE(_tv->selenitic_vars); i++)
		_tv->selenitic_vars[i] = 0;
	for (i = 0; i < ARRAYSIZE(_tv->stoneship_vars); i++)
		_tv->stoneship_vars[i] = 0;
	for (i = 0; i < ARRAYSIZE(_tv->dunny_vars); i++)
		_tv->dunny_vars[i] = 0;
	for (i = 0; i < ARRAYSIZE(_tv->unknown_myst); i++)
		_tv->unknown_myst[i] = 0;
	for (i = 0; i < ARRAYSIZE(_tv->unknown_channelwood); i++)
		_tv->unknown_channelwood[i] = 0;
	for (i = 0; i < ARRAYSIZE(_tv->unknown_mech); i++)
		_tv->unknown_mech[i] = 0;
	for (i = 0; i < ARRAYSIZE(_tv->unknown_selenitic); i++)
		_tv->unknown_selenitic[i] = 0;
	for (i = 0; i < ARRAYSIZE(_tv->unknown_stoneship); i++)
		_tv->unknown_stoneship[i] = 0;

	// TODO: Not all these may be needed as some of the unknown opcodes
	//        called by init scripts may set these up as per the others..

	// Unknown - Fixed at 2
	_tv->game_globals[0] = 2;
	// Current Age / Stack - Start in Myst
	_tv->game_globals[1] = 2;
	// Unknown - Fixed at 1
	_tv->game_globals[3] = 1;

	// Library Bookcase Door - Default to Up
	_tv->myst_vars[18] = 1;
	// Dock Imager Numeric Selection - Default to 67
	_tv->myst_vars[19] = 67;
	// Dock Imager Active - Default to Active
	_tv->myst_vars[20] = 1;
	// Stellar Observatory Lights - Default to On
	_tv->myst_vars[29] = 1;

	// Lighthouse Trapdoor State - Default to Locked
	_tv->stoneship_vars[4] = 2;
	// Lighthouse Chest Water State - Default to Full
	_tv->stoneship_vars[5] = 1;
}

static const char *game_globals_names[] = {
	"Unknown - Fixed at 2",
	"Current Age / Stack",
	"Page Being Held",
	"Unknown - Fixed at 1",
	"Slide Transistions",
	"Zip Mode",
	"Red Pages in Book",
	"Blue Pages in Book"
};

static const char *myst_vars_names[] = {
	"Marker Switch Near Cabin",
	"Marker Switch Near Clock Tower",
	"Marker Switch on Dock",
	"Marker Switch Near Ship Pool",
	"Marker Switch Near Cogs",
	"Marker Switch Near Generator Room",
	"Marker Switch Near Stellar Observatory",
	"Marker Switch Near Rocket Ship",
	"Fireplace, Opened Green Book Before",
	"Ship State",
	"Cabin Gas Valve Position",
	"Clock Tower Hour Hand Position",
	"Clock Tower Minute Hand Position",
	"Clock Tower Puzzle Solved / Cogs Open",
	"Clock Tower Gear Bridge",
	"Generator Breaker State",
	"Generator Button State",
	"Generator Voltage State",
	"Library Bookcase Door",
	"Dock Imager Numeric Selection",
	"Dock Imager Active",
	"Unknown #1 - Fixed at 0",
	"Unknown #2 - Fixed at 0",
	"Unknown #3 - Fixed at 0",
	"Unknown #4 - Fixed at 0",
	"Tower Rotation Angle",
	"Boxes For Ship Float Puzzle",
	"Tree Boiler Pilot Light Lit",
	"Stellar Observatory Viewer, Control Setting Day",
	"Stellar Observatory Lights",
	"Stellar Observatory Viewer, Control Setting Month",
	"Stellar Observatory Viewer, Control Setting Time",
	"Stellar Observatory Viewer, Control Setting Year",
	"Stellar Observatory Viewer, Target Day",
	"Stellar Observatory Viewer, Target Month",
	"Stellar Observatory Viewer, Target Time",
	"Stellar Observatory Viewer, Target Year",
 	"Cabin Safe Combination",
	"Channelwood Tree Position",
	"Checksum? #1",
	"Checksum? #2",
	"Rocketship Music Puzzle Slider #1 Position",
	"Rocketship Music Puzzle Slider #2 Position",
	"Rocketship Music Puzzle Slider #3 Position",
	"Rocketship Music Puzzle Slider #4 Position",
	"Rocketship Music Puzzle Slider #5 Position",
	"Unknown #5",
	"Unknown #6",
	"Unknown #7",
	"Unknown #8"
};

static const char *channelwood_vars_names[] = {
	"Water Pump Bridge State",
	"Lower Walkway to Upper Walkway Elevator State",
	"Lower Walkway to Upper Walkway Spiral Stair Lower Door State",
	"Extendable Pipe State",
	"Water Valve States",
	"Achenar's Holoprojector Selection",
	"Lower Walkway to Upper Walkway Spiral Stair Upper Door State"
};

static const char *mech_vars_names[] = {
	"Achenar's Room Secret Panel State",
	"Sirrus' Room Secret Panel State",
	"Fortress Staircase State",
	"Fortress Elevator Rotation",
	"Code Lock Shape #1 (Left)",
	"Code Lock Shape #2",
	"Code Lock Shape #3",
	"Code Lock Shape #4 (Right)"
};

static const char *selenitic_vars_names[] = {
	"Sound Pickup At Water Pool",
	"Sound Pickup At Volcanic Crack",
	"Sound Pickup At Clock",
	"Sound Pickup At Crystal Rocks",
	"Sound Pickup At Windy Tunnel",
	"Sound Receiver Doors",
	"Windy Tunnel Lights",
	"Sound Receiver Current Input",
	"Sound Receiver Input #0 (Water Pool) Angle Value",
	"Sound Receiver Input #1 (Volcanic Crack) Angle Value",
	"Sound Receiver Input #2 (Clock) Angle Value",
	"Sound Receiver Input #3 (Crystal Rocks) Angle Value",
	"Sound Receiver Input #4 (Windy Tunnel) Angle Value",
	"Sound Lock Slider #1 (Left) Position",
	"Sound Lock Slider #2 Position",
	"Sound Lock Slider #3 Position",
	"Sound Lock Slider #4 Position",
	"Sound Lock Slider #5 (Right) Position"
};

static const char *stoneship_vars_names[] = {
	"Light State",
	"Unknown #1",
	"Unknown #2",
	"Water Pump State",
	"Lighthouse Trapdoor State",
	"Lighthouse Chest Water State",
	"Lighthouse Chest Valve State",
	"Lighthouse Chest Open State",
	"Lighthouse Trapdoor Key State",
	"Lighthouse Generator Power Level(?)",
	"Lighthouse Generator Power...?",
	"Lighthouse Generator Power Good",
	"Lighthouse Generator Power #1 ?",
	"Lighthouse Generator Power #2?"
};

static const char *dunny_vars_names[] = {
	"Outcome State"
};

void MystSaveLoad::debug_printMystVariables(MystVariables *_tv) {
	uint8 i;

	debugC(kDebugSaveLoad, "Printing Myst Variable State:");

	debugC(kDebugSaveLoad, "  Game Globals:");
	for (i = 0; i < ARRAYSIZE(_tv->game_globals); i++)
		debugC(kDebugSaveLoad, "    %s: %u", game_globals_names[i], _tv->game_globals[i]);

	debugC(kDebugSaveLoad, "  Myst Variables:");
	for (i = 0; i < ARRAYSIZE(_tv->myst_vars); i++)
		debugC(kDebugSaveLoad, "    %s: %u", myst_vars_names[i], _tv->myst_vars[i]);

	debugC(kDebugSaveLoad, "  Channelwood Variables:");
	for (i = 0; i < ARRAYSIZE(_tv->channelwood_vars); i++)
		debugC(kDebugSaveLoad, "    %s: %u", channelwood_vars_names[i], _tv->channelwood_vars[i]);

	debugC(kDebugSaveLoad, "  Mech Variables:");
	for (i = 0; i < ARRAYSIZE(_tv->mech_vars); i++)
		debugC(kDebugSaveLoad, "    %s: %u", mech_vars_names[i], _tv->mech_vars[i]);

	debugC(kDebugSaveLoad, "  Selenitic Variables:");
	for (i = 0; i < ARRAYSIZE(_tv->selenitic_vars); i++)
		debugC(kDebugSaveLoad, "    %s: %u", selenitic_vars_names[i], _tv->selenitic_vars[i]);

	debugC(kDebugSaveLoad, "  Stoneship Variables:");
	for (i = 0; i < ARRAYSIZE(_tv->stoneship_vars); i++)
		debugC(kDebugSaveLoad, "    %s: %u", stoneship_vars_names[i], _tv->stoneship_vars[i]);

	debugC(kDebugSaveLoad, "  Dunny Variables:");
	for (i = 0; i < ARRAYSIZE(_tv->dunny_vars); i++)
		debugC(kDebugSaveLoad, "    %s: %u", dunny_vars_names[i], _tv->dunny_vars[i]);

	debugC(kDebugSaveLoad, "  Other Variables:");

	debugC(kDebugSaveLoad, "    Unknown Myst:");
	for (i = 0; i < ARRAYSIZE(_tv->unknown_myst); i++)
		debugC(kDebugSaveLoad, "    %u: 0x%04X - %u", i, _tv->unknown_myst[i], _tv->unknown_myst[i]);

	debugC(kDebugSaveLoad, "    Unknown Channelwood:");
	for (i = 0; i < ARRAYSIZE(_tv->unknown_channelwood); i++)
		debugC(kDebugSaveLoad, "    %u: 0x%04X - %u", i, _tv->unknown_channelwood[i], _tv->unknown_channelwood[i]);

	debugC(kDebugSaveLoad, "    Unknown Mech:");
	for (i = 0; i < ARRAYSIZE(_tv->unknown_mech); i++)
		debugC(kDebugSaveLoad, "    %u: 0x%04X - %u", i, _tv->unknown_mech[i], _tv->unknown_mech[i]);

	debugC(kDebugSaveLoad, "    Unknown Selenitic:");
	for (i = 0; i < ARRAYSIZE(_tv->unknown_selenitic); i++)
		debugC(kDebugSaveLoad, "    %u: 0x%04X - %u", i, _tv->unknown_selenitic[i], _tv->unknown_selenitic[i]);

	debugC(kDebugSaveLoad, "    Unknown Stoneship:");
	for (i = 0; i < ARRAYSIZE(_tv->unknown_stoneship); i++)
		debugC(kDebugSaveLoad, "    %u: 0x%04X - %u", i, _tv->unknown_stoneship[i], _tv->unknown_stoneship[i]);
}

} // End of namespace Mohawk
