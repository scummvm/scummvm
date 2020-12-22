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

#include "common/file.h"
#include "gui/debugger.h"
#include "startrek/console.h"
#include "startrek/resource.h"
#include "startrek/room.h"
#include "startrek/startrek.h"

namespace StarTrek {

Console::Console(StarTrekEngine *vm) : GUI::Debugger(), _vm(vm) {
	registerCmd("room",			WRAP_METHOD(Console, Cmd_Room));
	registerCmd("actions",		WRAP_METHOD(Console, Cmd_Actions));
	registerCmd("text",			WRAP_METHOD(Console, Cmd_Text));
	registerCmd("bg",			WRAP_METHOD(Console, Cmd_Bg));
	registerCmd("filedump",		WRAP_METHOD(Console, Cmd_DumpFile));
	registerCmd("filesearch",	WRAP_METHOD(Console, Cmd_SearchFile));
	registerCmd("score",		WRAP_METHOD(Console, Cmd_Score));
	registerCmd("bridgeseq",    WRAP_METHOD(Console, Cmd_BridgeSequence));
}

Console::~Console() {
}

bool Console::Cmd_Room(int argc, const char **argv) {
	if (argc < 3) {
		debugPrintf("Current room: %s\n", _vm->getScreenName().c_str());
		debugPrintf("Use room <mission> <room> to teleport\n");
		debugPrintf("Valid missions are: DEMON, TUG, LOVE, MUDD, FEATHER, TRIAL, SINS, VENG\n");
		return true;
	}

	_vm->_missionToLoad = argv[1];
	_vm->_missionToLoad.toUppercase();
	_vm->_roomIndexToLoad = atoi(argv[2]);
	_vm->runAwayMission();

	return false;
}

bool Console::Cmd_Actions(int argc, const char **argv) {
	Common::String screenName = _vm->getScreenName();

	if (argc == 3) {
		Common::String missionName = argv[1];
		missionName.toUppercase();
		int roomIndex = atoi(argv[2]);

		screenName = missionName + (char)(roomIndex + '0');
	}

	Common::MemoryReadStreamEndian *rdfFile = _vm->_resource->loadFile(screenName + ".RDF");
	rdfFile->seek(14);

	uint16 startOffset = rdfFile->readUint16LE();
	uint16 endOffset = rdfFile->readUint16LE();
	uint16 offset = startOffset;

	while (offset < endOffset) {
		rdfFile->seek(offset);

		uint32 action = rdfFile->readUint32LE();
		uint16 nextOffset = rdfFile->readUint16LE();

		debugPrintf("Offset %d: %s\n", offset, EventToString(action).c_str());
		offset = nextOffset;
	}

	delete rdfFile;

	return true;
}

bool Console::Cmd_Text(int argc, const char **argv) {
	typedef Common::HashMap<int, Common::String>::iterator MessageIterator;

	debugPrintf("\nLook messages\n");
	debugPrintf("-------------\n");
	for (MessageIterator i = _vm->_room->_lookMessages.begin(); i != _vm->_room->_lookMessages.end(); ++i) {
		debugPrintf("%i: %s\n", i->_key, i->_value.c_str());
	}

	debugPrintf("\nLook with talker messages\n");
	debugPrintf("-------------------------\n");
	for (MessageIterator i = _vm->_room->_lookWithTalkerMessages.begin(); i != _vm->_room->_lookWithTalkerMessages.end(); ++i) {
		debugPrintf("%i: %s\n", i->_key, i->_value.c_str());
	}

	debugPrintf("\nTalk messages\n");
	debugPrintf("-------------\n");
	for (MessageIterator i = _vm->_room->_talkMessages.begin(); i != _vm->_room->_talkMessages.end(); ++i) {
		debugPrintf("%i: %s\n", i->_key, i->_value.c_str());
	}

	return true;
}

bool Console::Cmd_Bg(int argc, const char **argv) {
	if (argc < 2) {
		debugPrintf("Usage: %s <background image name>\n", argv[0]);
		return true;
	}

	_vm->_gfx->setBackgroundImage(argv[1]);
	_vm->_gfx->copyBackgroundScreen();
	_vm->_system->updateScreen();

	return false;
}

void Console::dumpFile(Common::String fileName) {
	debugPrintf("Dumping %s...\n", fileName.c_str());

	Common::MemoryReadStreamEndian *stream = _vm->_resource->loadFile(fileName, 0, false);
	if (!stream) {
		debugPrintf("File not found\n");
		return;
	}

	uint32 size = stream->size();
	byte *data = new byte[size];
	stream->read(data, size);
	delete stream;

	Common::DumpFile out;
	out.open(fileName);
	out.write(data, size);
	out.flush();
	out.close();
	delete[] data;
}

bool Console::Cmd_DumpFile(int argc, const char **argv) {
	if (argc < 2) {
		debugPrintf("Usage: %s <file name>\n", argv[0]);
		return true;
	}

	Common::String fileName = argv[1];

	if (fileName != "*") {
		dumpFile(fileName);
	} else {
		for (Common::List<ResourceIndex>::const_iterator i = _vm->_resource->_resources.begin(), end = _vm->_resource->_resources.end(); i != end; ++i) {
			if (i->fileName == "S5ROOM3.BMP" || i->fileName == "Z_LIST.TXT")
				continue;
			dumpFile(i->fileName);
		}
	}

	return true;
}

bool Console::Cmd_SearchFile(int argc, const char **argv) {
	if (argc < 2) {
		debugPrintf("Usage: %s <file name>\n", argv[0]);
		return true;
	}

	Common::String filename = argv[1];
	filename.toUppercase();

	Common::List<ResourceIndex> records = _vm->_resource->searchIndex(filename);
	debugPrintf("Found:\n");
	for (Common::List<ResourceIndex>::const_iterator i = records.begin(), end = records.end(); i != end; ++i) {
		debugPrintf("%s, offset: %d\n", i->fileName.c_str(), i->indexOffset);
	}

	return true;
}

bool Console::Cmd_Score(int argc, const char **argv) {
	debugPrintf("Chapter 1: Demon world (demon): %d\n", _vm->_awayMission.demon.missionScore);
	debugPrintf("Chapter 2: Hijacked (tug): %d\n", _vm->_awayMission.tug.missionScore);
	debugPrintf("Chapter 3: Love's Labor Jeopardized (love): %d\n", _vm->_awayMission.love.missionScore);
	debugPrintf("Chapter 4: Another Fine Mess (mudd): %d\n", _vm->_awayMission.mudd.missionScore);
	debugPrintf("Chapter 5A: The Feathered Serpent (feather): %d\n", _vm->_awayMission.feather.missionScore);
	debugPrintf("Chapter 5B: The Feathered Serpent (trial): %d\n", _vm->_awayMission.trial.missionScore);
	debugPrintf("Chapter 6: The Old Devil Moon (sins): %d\n", _vm->_awayMission.sins.missionScore);
	debugPrintf("Chapter 7: Vengeance (veng): %d\n", _vm->_awayMission.veng.missionScore);
	return true;
}

bool Console::Cmd_BridgeSequence(int argc, const char **argv) {
	if (argc < 2) {
		debugPrintf("Usage: %s <sequence ID> to start a bridge sequence\n", argv[0]);
		return true;
	} else {
		_vm->_bridgeSequenceToLoad = atoi(argv[1]);
		return false;
	}
}

Common::String Console::EventToString(uint32 action) {
	const char *actions[] = {
		"Tick",
		"Walk",
		"Use",
		"Get",
		"Look",
		"Talk"
	};

	byte verb =            action & 0xff;
	byte subject = (action >>  8) & 0xff;
	byte b2 =      (action >> 16) & 0xff;
	byte b3 =      (action >> 24) & 0xff;

	String retString;
	switch (verb) {
	case 0:	// Tick
		retString = Common::String::format("Tick %d", (subject | (b2 << 8)));
		break;
	case 2: // Use
		retString = Common::String(actions[verb]) + " " + ItemToString(subject) + ", " + ItemToString(b2);
		break;
	case 1:	// Walk
	case 3:	// Get
	case 4:	// Look
	case 5:	// Talk
		retString = Common::String(actions[verb]) + " " + ItemToString(subject);
		break;
	case 6:	// Warp touched
		retString = Common::String::format("Touched warp %d", subject);
		break;
	case 7:	// Hotspot touched
		retString = Common::String::format("Touched hotspot %d", subject);
		break;
	case 8:	// Timer expired
		retString = Common::String::format("Timer %d expired", subject);
		break;
	case 10: // Animation finished
		retString = Common::String::format("Finished animation (%d)", subject);
		break;
	case 12: // Walking finished
		retString = Common::String::format("Finished walking (%d)", subject);
		break;
	default:
		retString = Common::String::format("%x%x%x%x", verb, subject, b2, b3);
		break;
	}

	// Check for actions using bytes they're not expected to use
	if (b3 != 0)
		debugPrintf("WARNING: b3 nonzero in action: %s\n", retString.c_str());
	if (b2 != 0 && verb != 0 && verb != 2)
		debugPrintf("WARNING: b2 nonzero in action: %s\n", retString.c_str());

	return retString;
}

const char *itemNames[] = {
	"IPHASERS",
	"IPHASERK",
	"IHAND",
	"IROCK",
	"ISTRICOR",
	"IMTRICOR",
	"IDEADGUY",
	"ICOMM",
	"IPBC",
	"IRLG",
	"IWRENCH",
	"IINSULAT",
	"ISAMPLE",
	"ICURE",
	"IDISHES",
	"IRT",
	"IRTWB",
	"ICOMBBIT",
	"IJNKMETL",
	"IWIRING",
	"IWIRSCRP",
	"IPWF",
	"IPWE",
	"IDEADPH",
	"IBOMB",
	"IMETAL",
	"ISKULL",
	"IMINERAL",
	"IMETEOR",
	"ISHELLS",
	"IDEGRIME",
	"ILENSES",
	"IDISKS",
	"IANTIGRA",
	"IN2GAS",
	"IO2GAS",
	"IH2GAS",
	"IN2O",
	"INH3",
	"IH2O",
	"IWROD",
	"IIROD",
	"IREDGEM_A",
	"IREDGEM_B",
	"IREDGEM_C",
	"IGRNGEM_A",
	"IGRNGEM_B",
	"IGRNGEM_C",
	"IBLUGEM_A",
	"IBLUGEM_B",
	"IBLUGEM_C",
	"ICONECT",
	"IS8ROCKS",
	"IIDCARD",
	"ISNAKE",
	"IFERN",
	"ICRYSTAL",
	"IKNIFE",
	"IDETOXIN",
	"IBERRY",
	"IDOOVER",
	"IALIENDV",
	"ICAPSULE",
	"IMEDKIT",
	"IBEAM",
	"IDRILL",
	"IHYPO",
	"IFUSION",
	"ICABLE1",
	"ICABLE2",
	"ILMD",
	"IDECK",
	"ITECH"
};

Common::String Console::ItemToString(byte index) {
	if (index == 0)
		return "KIRK";
	else if (index == 1)
		return "SPOCK";
	else if (index == 2)
		return "MCCOY";
	else if (index == 3)
		return "REDSHIRT";
	else if (index >= 0x40 && (index - 0x40) < ARRAYSIZE(itemNames))
		return itemNames[index - 0x40];
	return Common::String(Common::String::format("0x%02x:", index)); // TODO
}

} // End of namespace StarTrek
