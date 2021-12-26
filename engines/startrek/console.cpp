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
	registerCmd("score",			WRAP_METHOD(Console, Cmd_Score));
	registerCmd("bridgeseq",		WRAP_METHOD(Console, Cmd_BridgeSequence));
	registerCmd("dumptext",		WRAP_METHOD(Console, Cmd_DumpText));
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
	const RoomTextOffsets *textList = _vm->_room->_roomTextList;
	Common::String screenName = _vm->getScreenName();
	byte *rdfData = _vm->_room->loadRoomRDF(screenName);
	int index = 0;

	do {
		uint16 offset = textList[index].offsetEnglishCD;
		debugPrintf("%i - %i: %s\n", textList[index].id, offset, rdfData+offset);
		index++;
	} while (textList[index].id != -1);

	delete[] rdfData;

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

struct MessageInfo {
	Common::String key;
	Common::String value;
	uint16 pos;
};

struct MessageInfoComparator {
	bool operator()(const MessageInfo &x, const MessageInfo &y) const {
		return x.key < y.key;
	}
};

bool Console::Cmd_DumpText(int argc, const char **argv) {
	if (argc < 2) {
		debugPrintf("Dumps room text messages from CD-ROM versions of ST25\n");
		debugPrintf("Usage: %s <room RDF file name> <table format>\n", argv[0]);
	} else {
		Common::String fileName = argv[1];
		bool tableFormat = false;
		Common::List<MessageInfo> keys;

		if (argc > 2)
			tableFormat = !scumm_stricmp(argv[2], "true") || !strcmp(argv[2], "1");

		Common::MemoryReadStreamEndian *rdfFile = _vm->_resource->loadFile(fileName + ".RDF");
		rdfFile->seek(32, SEEK_SET);
		uint16 messageOffset = rdfFile->readUint16LE();
		rdfFile->seek(messageOffset, SEEK_SET);

		while (!rdfFile->eos() && !rdfFile->err()) {
			Common::String message;
			uint16 pos = rdfFile->pos();
			byte c = rdfFile->readByte();
			if (!Common::isPrint(c))
				break;

			while (c != '\0') {
				message += c;
				c = rdfFile->readByte();
			}

			if (!message.empty()) {
				if (!tableFormat) {
					debug("%s, %d", message.c_str(), pos);
				} else {
					MessageInfo m;
					m.key = message.size() >= 14 ? message.substr(6, 8) : message;
					m.value = message;
					m.pos = pos;
					keys.push_back(m);
				}
			}
		}

		int size = rdfFile->size();
		rdfFile->seek(14, SEEK_SET);
		uint16 startOffset = rdfFile->readUint16LE();
		uint16 offset = startOffset;
		rdfFile->seek(startOffset, SEEK_SET);
		const char *validPrefixes[] = {
		    "BRI", "COM", "DEM", "FEA", "GEN", "LOV", "MUD", "SIN", "TRI", "TUG", "VEN"};

		while (!rdfFile->eos() && !rdfFile->err()) {
			rdfFile->skip(4);
			uint16 nextOffset = rdfFile->readUint16LE();
			if (nextOffset >= size || offset >= nextOffset)
				break;

			while (offset < nextOffset) {
				int pos = rdfFile->pos();
				byte c = rdfFile->readByte();
				bool found = false;

				if (c == '#') {
					rdfFile->skip(4);
					c = rdfFile->readByte();

					if (c == '\\') {
						found = true;
						rdfFile->seek(pos, SEEK_SET);
						Common::String message;
						c = rdfFile->readByte();
						while (c != '\0') {
							message += c;
							c = rdfFile->readByte();
						}

						Common::String prefix = message.substr(1, 3);

						for (uint i = 0; i < ARRAYSIZE(validPrefixes); i++) {
							if (prefix == validPrefixes[i]) {
								MessageInfo m;
								m.key = message.size() >= 14 ? message.substr(6, 8) : message;
								m.value = message;
								m.pos = pos;
								keys.push_back(m);

								break;
							}
						}
					}
				}

				if (!found)
					rdfFile->seek(pos + 1, SEEK_SET);

				offset = rdfFile->pos();
			}
		}

		if (tableFormat) {
			int index = 0;
			Common::String line;

			Common::sort(keys.begin(), keys.end(), MessageInfoComparator());

			for (Common::List<MessageInfo>::const_iterator i = keys.begin(), end = keys.end(); i != end; ++i) {
				line += "TX_" + (*i).key + ", ";
				index++;
				if (index % 5 == 0) {
					debug("%s", line.c_str());
					line = "";
				}
			}

			debug("%s", line.c_str());

			for (Common::List<MessageInfo>::const_iterator i = keys.begin(), end = keys.end(); i != end; ++i) {
				debug("{ TX_%s, %d, 0 },", (*i).key.c_str(), (*i).pos);
			}
		}

		delete rdfFile;
	}

	return true;
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
