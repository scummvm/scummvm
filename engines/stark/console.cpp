/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
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

#include "engines/stark/console.h"

#include "engines/stark/formats/xarc.h"
#include "engines/stark/resources/object.h"
#include "engines/stark/resources/level.h"
#include "engines/stark/resources/location.h"
#include "engines/stark/resources/knowledge.h"
#include "engines/stark/resources/root.h"
#include "engines/stark/resources/script.h"
#include "engines/stark/services/archiveloader.h"
#include "engines/stark/services/dialogplayer.h"
#include "engines/stark/services/global.h"
#include "engines/stark/services/resourceprovider.h"
#include "engines/stark/services/services.h"
#include "engines/stark/services/staticprovider.h"

#include "common/file.h"

namespace Stark {

Console::Console() : GUI::Debugger() {
	registerCmd("dumpArchive",			WRAP_METHOD(Console, Cmd_DumpArchive));
	registerCmd("dumpRoot",				WRAP_METHOD(Console, Cmd_DumpRoot));
	registerCmd("dumpStatic",			WRAP_METHOD(Console, Cmd_DumpStatic));
	registerCmd("dumpGlobal",			WRAP_METHOD(Console, Cmd_DumpGlobal));
	registerCmd("dumpLevel",			WRAP_METHOD(Console, Cmd_DumpLevel));
	registerCmd("dumpKnowledge",	WRAP_METHOD(Console, Cmd_DumpKnowledge));
	registerCmd("dumpLocation",			WRAP_METHOD(Console, Cmd_DumpLocation));
	registerCmd("listScripts",			WRAP_METHOD(Console, Cmd_ListScripts));
	registerCmd("enableScript",			WRAP_METHOD(Console, Cmd_EnableScript));
	registerCmd("forceScript",			WRAP_METHOD(Console, Cmd_ForceScript));
	registerCmd("listInventory",			WRAP_METHOD(Console, Cmd_ListInventory));
	registerCmd("listLocations",			WRAP_METHOD(Console, Cmd_ListLocations));
	registerCmd("location",				WRAP_METHOD(Console, Cmd_Location));
	registerCmd("chapter",				WRAP_METHOD(Console, Cmd_Chapter));
	registerCmd("changeLocation",		WRAP_METHOD(Console, Cmd_ChangeLocation));
	registerCmd("changeChapter",		WRAP_METHOD(Console, Cmd_ChangeChapter));
	registerCmd("changeKnowledge",		WRAP_METHOD(Console, Cmd_ChangeKnowledge));
	registerCmd("selectDialogOption",	WRAP_METHOD(Console, Cmd_SelectDialogOption));
	registerCmd("enableInventoryItem", WRAP_METHOD(Console, Cmd_EnableInventoryItem));
}

Console::~Console() {
}

bool Console::Cmd_DumpArchive(int argc, const char **argv) {
	if (argc != 2) {
		debugPrintf("Extract all the files from a game archive.\n");
		debugPrintf("The destination folder, named 'dump', must exist.\n");
		debugPrintf("Usage :\n");
		debugPrintf("dumpArchive [archive name]\n");
		return true;
	}

	Formats::XARCArchive xarc;
	if (!xarc.open(argv[1])) {
		debugPrintf("Can't open archive with name '%s'\n", argv[1]);
		return true;
	}

	Common::ArchiveMemberList members;
	xarc.listMembers(members);

	for (Common::ArchiveMemberList::const_iterator it = members.begin(); it != members.end(); it++) {
		Common::String fileName = Common::String::format("dump/%s", it->get()->getName().c_str());

		// Open the output file
		Common::DumpFile outFile;
		if (!outFile.open(fileName)) {
			debugPrintf("Unable to open file '%s' for writing\n", fileName.c_str());
			return true;
		}

		// Copy the archive content to the output file using a temporary buffer
		Common::SeekableReadStream *inStream = it->get()->createReadStream();
		uint8 *buf = new uint8[inStream->size()];

		inStream->read(buf, inStream->size());
		outFile.write(buf, inStream->size());

		delete[] buf;
		delete inStream;
		outFile.close();

		debugPrintf("Extracted '%s'\n", it->get()->getName().c_str());
	}

	return true;
}

bool Console::Cmd_DumpRoot(int argc, const char **argv) {
	Global *global = StarkServices::instance().global;

	global->getRoot()->print();

	return true;
}

bool Console::Cmd_DumpGlobal(int argc, const char **argv) {
	Global *global = StarkServices::instance().global;

	global->getLevel()->print();

	return true;
}

bool Console::Cmd_DumpStatic(int argc, const char **argv) {
	StaticProvider *staticProvider = StarkServices::instance().staticProvider;

	staticProvider->getLevel()->print();

	return true;
}

bool Console::Cmd_DumpLevel(int argc, const char **argv) {
	Global *global = StarkServices::instance().global;

	global->getCurrent()->getLevel()->print();

	return true;
}

bool Console::Cmd_DumpKnowledge(int argc, const char **argv) {
	Global *global = StarkServices::instance().global;

	Resources::Level *level = global->getCurrent()->getLevel();
	Resources::Location *location = global->getCurrent()->getLocation();
	Common::Array<Resources::Knowledge*> knowledge = level->listChildrenRecursive<Resources::Knowledge>();
	knowledge.insert_at(knowledge.size(), location->listChildrenRecursive<Resources::Knowledge>());
	Common::Array<Resources::Knowledge*>::iterator it;
	for (it = knowledge.begin(); it != knowledge.end(); ++it) {
		(*it)->print();
	}
	return true;
}

bool Console::Cmd_ChangeKnowledge(int argc, const char **argv) {
	int index = 0;
	char type = 0;

	if (argc >= 4) {
		index = atoi(argv[1]);
		type = argv[2][0];
		if (type == 'b' || type == 'i') {
			Global *global = StarkServices::instance().global;

			Resources::Level *level = global->getCurrent()->getLevel();
			Resources::Location *location = global->getCurrent()->getLocation();
			Common::Array<Resources::Knowledge*> knowledgeArr = level->listChildrenRecursive<Resources::Knowledge>();
			knowledgeArr.insert_at(knowledgeArr.size(), location->listChildrenRecursive<Resources::Knowledge>());
			if (index < knowledgeArr.size() ) {
				Resources::Knowledge *knowledge = knowledgeArr[index];
				if (type == 'b') {
					knowledge->setBooleanValue(atoi(argv[3]));
				} else if (type == 'i') {
					knowledge->setIntegerValue(atoi(argv[3]));
				}
				return true;
			} else {
				debugPrintf("Invalid index %d, only %d indices available\n", index, knowledgeArr.size());
			}
		} else {
			debugPrintf("Invalid type: %c, only b and i are available\n", type);
		}
	} else {
		debugPrintf("Too few args\n");
	}

	debugPrintf("Change the value of some knowledge. Use dumpKnowledge to get an id\n");
	debugPrintf("Usage :\n");
	debugPrintf("changeKnowledge [id] [type] [value]\n");
	debugPrintf("available types: b(inary), i(nteger)\n");
	return true;
}

bool Console::Cmd_ListScripts(int argc, const char **argv) {
	Global *global = StarkServices::instance().global;

	Resources::Level *level = global->getCurrent()->getLevel();
	Resources::Location *location = global->getCurrent()->getLocation();
	Common::Array<Resources::Script*> scriptArr = level->listChildrenRecursive<Resources::Script>();
	scriptArr.insert_at(scriptArr.size(), location->listChildrenRecursive<Resources::Script>());
	Common::Array<Resources::Script*>::iterator it;
	int i = 0;
	for (it = scriptArr.begin(); it != scriptArr.end(); ++it) {
		debugPrintf("%d: %s - enabled: %d\n", i++, (*it)->getName().c_str(), (*it)->isEnabled());
	}
	return true;
}

bool Console::Cmd_EnableScript(int argc, const char **argv) {
	int index = 0;

	if (argc >= 2) {
		index = atoi(argv[1]);
		Global *global = StarkServices::instance().global;

		bool value = true;
		if (argc >= 3) {
			value = atoi(argv[2]);
		}
		Resources::Level *level = global->getCurrent()->getLevel();
		Resources::Location *location = global->getCurrent()->getLocation();
		Common::Array<Resources::Script*> scriptArr = level->listChildrenRecursive<Resources::Script>();
		scriptArr.insert_at(scriptArr.size(), location->listChildrenRecursive<Resources::Script>());
		if (index < scriptArr.size() ) {
			Resources::Script *script = scriptArr[index];
			script->enable(value);
			return true;
		} else {
			debugPrintf("Invalid index %d, only %d indices available\n", index, scriptArr.size());
		}
	} else {
		debugPrintf("Too few args\n");
	}

	debugPrintf("Change the value of some knowledge. Use dumpKnowledge to get an id\n");
	debugPrintf("Usage :\n");
	debugPrintf("enableScript [id] (value)\n");
	return true;
}

bool Console::Cmd_ForceScript(int argc, const char **argv) {
	int index = 0;

	if (argc >= 2) {
		index = atoi(argv[1]);
		Global *global = StarkServices::instance().global;

		bool value = true;
		if (argc >= 3) {
			value = atoi(argv[2]);
		}
		Resources::Level *level = global->getCurrent()->getLevel();
		Resources::Location *location = global->getCurrent()->getLocation();
		Common::Array<Resources::Script*> scriptArr = level->listChildrenRecursive<Resources::Script>();
		scriptArr.insert_at(scriptArr.size(), location->listChildrenRecursive<Resources::Script>());
		if (index < scriptArr.size() ) {
			Resources::Script *script = scriptArr[index];
			script->enable(value);
			// Use this mode to avoid being stopped
			script->execute(Resources::Script::kCallModeCalledByScript);
			return true;
		} else {
			debugPrintf("Invalid index %d, only %d indices available\n", index, scriptArr.size());
		}
	} else {
		debugPrintf("Too few args\n");
	}

	debugPrintf("Change the value of some knowledge. Use dumpKnowledge to get an id\n");
	debugPrintf("Usage :\n");
	debugPrintf("enableScript [id] (value)\n");
	return true;
}


bool Console::Cmd_DumpLocation(int argc, const char **argv) {
	Global *global = StarkServices::instance().global;

	global->getCurrent()->getLocation()->print();

	return true;
}

bool Console::Cmd_ListInventory(int argc, const char **argv) {
	Global *global = StarkServices::instance().global;
	global->printInventory(argc != 2);

	return true;
}

bool Console::Cmd_EnableInventoryItem(int argc, const char **argv) {
	if (argc != 2) {
		debugPrintf("Enable a specific inventory item.\n");
		debugPrintf("Usage :\n");
		debugPrintf("changeLocation [level] [location]\n");
		return true;
	}
	Global *global = StarkServices::instance().global;
	global->enableInventoryItem(atoi(argv[1]));
	return true;
}


bool Console::Cmd_ListLocations(int argc, const char **argv) {
	ArchiveLoader *archiveLoader = new ArchiveLoader();

	// Temporarily replace the global archive loader with our instance
	ArchiveLoader *gameArchiveLoader = StarkServices::instance().archiveLoader;
	StarkServices::instance().archiveLoader = archiveLoader;

	archiveLoader->load("x.xarc");
	Resources::Root *root = archiveLoader->useRoot<Resources::Root>("x.xarc");

	// Find all the levels
	Common::Array<Resources::Level *> levels = root->listChildren<Resources::Level>();

	// Loop over the levels
	for (uint i = 0; i < levels.size(); i++) {
		Resources::Level *level = levels[i];

		Common::String levelArchive = archiveLoader->buildArchiveName(level);
		debugPrintf("%s - %s\n", levelArchive.c_str(), level->getName().c_str());

		// Load the detailed level archive
		archiveLoader->load(levelArchive);
		level = archiveLoader->useRoot<Resources::Level>(levelArchive);

		Common::Array<Resources::Location *> locations = level->listChildren<Resources::Location>();

		// Loop over the locations
		for (uint j = 0; j < locations.size(); j++) {
			Resources::Location *location = locations[j];

			Common::String roomArchive = archiveLoader->buildArchiveName(level, location);
			debugPrintf("%s - %s\n", roomArchive.c_str(), location->getName().c_str());
		}

		archiveLoader->returnRoot(levelArchive);
		archiveLoader->unloadUnused();
	}

	// Restore the global archive loader
	StarkServices::instance().archiveLoader = gameArchiveLoader;

	delete archiveLoader;

	return true;
}

bool Console::Cmd_ChangeLocation(int argc, const char **argv) {
	if (argc != 3) {
		debugPrintf("Change the current location.\n");
		debugPrintf("Usage :\n");
		debugPrintf("changeLocation [level] [location]\n");
		return true;
	}

	uint levelIndex = strtol(argv[1] , nullptr, 16);
	uint locationIndex = strtol(argv[2] , nullptr, 16);

	ResourceProvider *resourceProvider = StarkServices::instance().resourceProvider;
	resourceProvider->requestLocationChange(levelIndex, locationIndex);

	return false;
}

bool Console::Cmd_ChangeChapter(int argc, const char **argv) {
	if (argc != 2) {
		debugPrintf("Change the current chapter.\n");
		debugPrintf("Usage :\n");
		debugPrintf("changeChapter [value]\n");
		return true;
	}

	uint32 value = atoi(argv[1]);

	Global *global = StarkServices::instance().global;
	global->setCurrentChapter(value);

	return true;
}

bool Console::Cmd_SelectDialogOption(int argc, const char **argv) {
	if (argc != 2) {
		debugPrintf("Select a dialog option.\n");
		debugPrintf("Usage :\n");
		debugPrintf("selectDialogOption [option]\n");
		return true;
	}
	DialogPlayer *dialogPlayer = StarkServices::instance().dialogPlayer;
	dialogPlayer->selectOption(atoi(argv[1]));

	return false;
}

bool Console::Cmd_Location(int argc, const char **argv) {
	if (argc != 1) {
		debugPrintf("Display the current location.\n");
		debugPrintf("Usage :\n");
		debugPrintf("location\n");
		return true;
	}

	Global *global = StarkServices::instance().global;
	Current *current = global->getCurrent();

	debugPrintf("location: %02x %02x\n", current->getLevel()->getIndex(), current->getLocation()->getIndex());

	return true;
}

bool Console::Cmd_Chapter(int argc, const char **argv) {
	if (argc != 1) {
		debugPrintf("Display the current chapter.\n");
		debugPrintf("Usage :\n");
		debugPrintf("chapter\n");
		return true;
	}

	Global *global = StarkServices::instance().global;
	int32 value = global->getCurrentChapter();

	debugPrintf("chapter: %d\n", value);

	return true;
}

} // End of namespace Stark
