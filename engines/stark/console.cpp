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

#include "engines/stark/console.h"

#include "engines/stark/formats/xarc.h"
#include "engines/stark/resources/object.h"
#include "engines/stark/resources/anim.h"
#include "engines/stark/resources/level.h"
#include "engines/stark/resources/location.h"
#include "engines/stark/resources/knowledge.h"
#include "engines/stark/resources/root.h"
#include "engines/stark/resources/script.h"
#include "engines/stark/resources/knowledgeset.h"
#include "engines/stark/resources/item.h"
#include "engines/stark/resources/textureset.h"
#include "engines/stark/services/archiveloader.h"
#include "engines/stark/services/dialogplayer.h"
#include "engines/stark/services/global.h"
#include "engines/stark/services/resourceprovider.h"
#include "engines/stark/services/userinterface.h"
#include "engines/stark/services/services.h"
#include "engines/stark/services/staticprovider.h"
#include "engines/stark/tools/decompiler.h"

#include "common/file.h"

namespace Stark {

Console::Console() :
		GUI::Debugger() {
	registerCmd("dumpArchive",          WRAP_METHOD(Console, Cmd_DumpArchive));
	registerCmd("dumpRoot",             WRAP_METHOD(Console, Cmd_DumpRoot));
	registerCmd("dumpStatic",           WRAP_METHOD(Console, Cmd_DumpStatic));
	registerCmd("dumpGlobal",           WRAP_METHOD(Console, Cmd_DumpGlobal));
	registerCmd("dumpLevel",            WRAP_METHOD(Console, Cmd_DumpLevel));
	registerCmd("dumpKnowledge",        WRAP_METHOD(Console, Cmd_DumpKnowledge));
	registerCmd("dumpLocation",         WRAP_METHOD(Console, Cmd_DumpLocation));
	registerCmd("listScripts",          WRAP_METHOD(Console, Cmd_ListScripts));
	registerCmd("enableScript",         WRAP_METHOD(Console, Cmd_EnableScript));
	registerCmd("forceScript",          WRAP_METHOD(Console, Cmd_ForceScript));
	registerCmd("decompileScript",      WRAP_METHOD(Console, Cmd_DecompileScript));
	registerCmd("testDecompiler",       WRAP_METHOD(Console, Cmd_TestDecompiler));
	registerCmd("listAnimations",       WRAP_METHOD(Console, Cmd_ListAnimations));
	registerCmd("forceAnimation",       WRAP_METHOD(Console, Cmd_ForceAnimation));
	registerCmd("listInventoryItems",   WRAP_METHOD(Console, Cmd_ListInventoryItems));
	registerCmd("listLocations",        WRAP_METHOD(Console, Cmd_ListLocations));
	registerCmd("location",             WRAP_METHOD(Console, Cmd_Location));
	registerCmd("chapter",              WRAP_METHOD(Console, Cmd_Chapter));
	registerCmd("changeLocation",       WRAP_METHOD(Console, Cmd_ChangeLocation));
	registerCmd("changeChapter",        WRAP_METHOD(Console, Cmd_ChangeChapter));
	registerCmd("changeKnowledge",      WRAP_METHOD(Console, Cmd_ChangeKnowledge));
	registerCmd("enableInventoryItem",  WRAP_METHOD(Console, Cmd_EnableInventoryItem));
	registerCmd("extractAllTextures",   WRAP_METHOD(Console, Cmd_ExtractAllTextures));
}

Console::~Console() {
}

bool Console::Cmd_DumpArchive(int argc, const char **argv) {
	if (argc != 2) {
		debugPrintf("Extract all the files from a game archive\n");
		debugPrintf("The destination folder, named 'dump', is in the location ScummVM was launched from\n");
		debugPrintf("Usage :\n");
		debugPrintf("dumpArchive [path to archive]\n");
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
		if (!outFile.open(fileName, true)) {
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
	Resources::Root *root = StarkGlobal->getRoot();
	if (root) {
		root->print();
	} else {
		debugPrintf("The global root has not been loaded\n");
	}

	return true;
}

bool Console::Cmd_DumpGlobal(int argc, const char **argv) {
	Resources::Level *level = StarkGlobal->getLevel();
	if (level) {
		level->print();
	} else {
		debugPrintf("The global level has not been loaded\n");
	}

	return true;
}

bool Console::Cmd_DumpStatic(int argc, const char **argv) {
	// Static resources are initialized in the beginning of the running
	StarkStaticProvider->getLevel()->print();

	return true;
}

bool Console::Cmd_DumpLevel(int argc, const char **argv) {
	Current *current = StarkGlobal->getCurrent();
	if (current) {
		current->getLevel()->print();
	} else {
		debugPrintf("Game levels have not been loaded\n");
	}

	return true;
}

bool Console::Cmd_DumpKnowledge(int argc, const char **argv) {
	Current *current = StarkGlobal->getCurrent();

	if (!current) {
		debugPrintf("Game levels have not been loaded\n");
		return true;
	}

	Resources::Level *level = current->getLevel();
	Resources::Location *location = current->getLocation();
	Common::Array<Resources::Knowledge *> knowledge = level->listChildrenRecursive<Resources::Knowledge>();
	knowledge.insert_at(knowledge.size(), location->listChildrenRecursive<Resources::Knowledge>());
	Common::Array<Resources::Knowledge *>::iterator it;
	for (it = knowledge.begin(); it != knowledge.end(); ++it) {
		(*it)->print();
	}
	return true;
}

bool Console::Cmd_ChangeKnowledge(int argc, const char **argv) {
	Current *current = StarkGlobal->getCurrent();

	if (!current) {
		debugPrintf("Game levels have not been loaded\n");
		return true;
	}

	uint index = 0;
	char type = 0;

	if (argc >= 4) {
		index = atoi(argv[1]);
		type = argv[2][0];
		if (type == 'b' || type == 'i') {
			Resources::Level *level = current->getLevel();
			Resources::Location *location = current->getLocation();
			Common::Array<Resources::Knowledge *> knowledgeArr = level->listChildrenRecursive<Resources::Knowledge>();
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
	} else if (argc > 1 ) {
		debugPrintf("Too few args\n");
	}

	debugPrintf("Change the value of some knowledge. Use dumpKnowledge to get an id\n");
	debugPrintf("Usage :\n");
	debugPrintf("changeKnowledge [id] [type] [value]\n");
	debugPrintf("available types: b(inary), i(nteger)\n");
	return true;
}

Common::Array<Resources::Script *> Console::listAllLocationScripts() const {
	Common::Array<Resources::Script *> scripts;

	Resources::Level *level = StarkGlobal->getCurrent()->getLevel();
	Resources::Location *location = StarkGlobal->getCurrent()->getLocation();
	scripts.push_back(level->listChildrenRecursive<Resources::Script>());
	scripts.push_back(location->listChildrenRecursive<Resources::Script>());

	return scripts;
}

bool Console::Cmd_ListScripts(int argc, const char **argv) {
	Current *current = StarkGlobal->getCurrent();
	if (!current) {
		debugPrintf("Game levels have not been loaded\n");
		return true;
	}

	Common::Array<Resources::Script *> scripts = listAllLocationScripts();

	for (uint i = 0; i < scripts.size(); i++) {
		Resources::Script *script = scripts[i];

		debugPrintf("%d: %s - enabled: %d", i, script->getName().c_str(), script->isEnabled());

		// Print which resource is causing the script to wait
		if (script->isSuspended()) {
			Resources::Object *suspending = script->getSuspendingResource();
			if (suspending) {
				debugPrintf(", waiting for: %s (%s)", suspending->getName().c_str(), suspending->getType().getName());
			} else {
				debugPrintf(", paused");
			}
		}

		debugPrintf("\n");
	}

	return true;
}

bool Console::Cmd_EnableScript(int argc, const char **argv) {
	Current *current = StarkGlobal->getCurrent();
	if (!current) {
		debugPrintf("Game levels have not been loaded\n");
		return true;
	}

	uint index = 0;

	if (argc >= 2) {
		index = atoi(argv[1]);

		bool value = true;
		if (argc >= 3) {
			value = atoi(argv[2]);
		}

		Common::Array<Resources::Script *> scripts = listAllLocationScripts();
		if (index < scripts.size() ) {
			Resources::Script *script = scripts[index];
			script->enable(value);
			return true;
		} else {
			debugPrintf("Invalid index %d, only %d indices available\n", index, scripts.size());
		}
	}

	debugPrintf("Enable or disable a script. Use listScripts to get an id\n");
	debugPrintf("Usage :\n");
	debugPrintf("enableScript [id] (value)\n");
	return true;
}

bool Console::Cmd_ForceScript(int argc, const char **argv) {
	Current *current = StarkGlobal->getCurrent();
	if (!current) {
		debugPrintf("Game levels have not been loaded\n");
		return true;
	}

	uint index = 0;

	if (argc >= 2) {
		index = atoi(argv[1]);

		Common::Array<Resources::Script *> scripts = listAllLocationScripts();
		if (index < scripts.size() ) {
			Resources::Script *script = scripts[index];
			script->enable(true);
			script->goToNextCommand(); // Skip the begin command to avoid checks
			script->execute(Resources::Script::kCallModePlayerAction);
			return true;
		} else {
			debugPrintf("Invalid index %d, only %d indices available\n", index, scripts.size());
		}
	}

	debugPrintf("Force the execution of a script. Use listScripts to get an id\n");
	debugPrintf("Usage :\n");
	debugPrintf("forceScript [id]\n");
	return true;
}

bool Console::Cmd_DecompileScript(int argc, const char **argv) {
	Current *current = StarkGlobal->getCurrent();
	if (!current) {
		debugPrintf("Game levels have not been loaded\n");
		return true;
	}

	if (argc >= 2) {
		uint index = atoi(argv[1]);

		Common::Array<Resources::Script *> scripts = listAllLocationScripts();
		if (index < scripts.size()) {
			Resources::Script *script = scripts[index];

			Tools::Decompiler *decompiler = new Tools::Decompiler(script);
			if (decompiler->getError() != "") {
				debugPrintf("Decompilation failure: %s\n", decompiler->getError().c_str());
			}

			debug("Script %d - %s:", index, script->getName().c_str());
			decompiler->printDecompiled();

			delete decompiler;

			return true;
		} else {
			debugPrintf("Invalid index %d, only %d indices available\n", index, scripts.size());
		}
	}

	debugPrintf("Decompile a script. Use listScripts to get an id\n");
	debugPrintf("Usage :\n");
	debugPrintf("decompileScript [id]\n");
	return true;
}

class ArchiveVisitor {
public:
	virtual ~ArchiveVisitor() {}
	virtual void acceptLevelArchive(Resources::Level *level) = 0;
	virtual void acceptLocationArchive(Resources::Location *location) = 0;
};

void Console::walkAllArchives(ArchiveVisitor *visitor) {
	ArchiveLoader *archiveLoader = new ArchiveLoader();

	// Temporarily replace the global archive loader with our instance
	ArchiveLoader *gameArchiveLoader = StarkArchiveLoader;
	StarkArchiveLoader = archiveLoader;

	archiveLoader->load("x.xarc");
	Resources::Root *root = archiveLoader->useRoot<Resources::Root>("x.xarc");

	// Find all the levels
	Common::Array<Resources::Level *> levels = root->listChildren<Resources::Level>();

	// Loop over the levels
	for (uint i = 0; i < levels.size(); i++) {
		Resources::Level *level = levels[i];

		Common::String levelArchive = archiveLoader->buildArchiveName(level);
		debug("%s - %s", levelArchive.c_str(), level->getName().c_str());

		// Load the detailed level archive
		archiveLoader->load(levelArchive);
		level = archiveLoader->useRoot<Resources::Level>(levelArchive);

		// Visit the level archive
		visitor->acceptLevelArchive(level);

		Common::Array<Resources::Location *> locations = level->listChildren<Resources::Location>();

		// Loop over the locations
		for (uint j = 0; j < locations.size(); j++) {
			Resources::Location *location = locations[j];

			Common::String locationArchive = archiveLoader->buildArchiveName(level, location);
			debug("%s - %s", locationArchive.c_str(), location->getName().c_str());

			// Load the detailed location archive
			archiveLoader->load(locationArchive);
			location = archiveLoader->useRoot<Resources::Location>(locationArchive);

			// Visit the location archive
			visitor->acceptLocationArchive(location);

			archiveLoader->returnRoot(locationArchive);
			archiveLoader->unloadUnused();
		}

		archiveLoader->returnRoot(levelArchive);
		archiveLoader->unloadUnused();
	}

	// Restore the global archive loader
	StarkArchiveLoader = gameArchiveLoader;

	delete archiveLoader;
}

class DecompilingArchiveVisitor : public ArchiveVisitor {
public:
	DecompilingArchiveVisitor() :
	    _totalScripts(0),
	    _okScripts(0) {}

	void acceptLevelArchive(Resources::Level *level) override {
		decompileScriptChildren(level);
	}

	void acceptLocationArchive(Resources::Location *location) override {
		decompileScriptChildren(location);
	}

	int getTotalScripts() const { return _totalScripts; }
	int getOKScripts() const { return _okScripts; }

private:
	int _totalScripts;
	int _okScripts;

	void decompileScriptChildren(Resources::Object *resource) {
		Common::Array<Resources::Script *> scripts = resource->listChildrenRecursive<Resources::Script>();

		for (uint i = 0; i < scripts.size(); i++) {
			Resources::Script *script = scripts[i];

			Tools::Decompiler decompiler(script);
			_totalScripts++;

			Common::String result;
			if (decompiler.getError() == "") {
				result = "OK";
				_okScripts++;
			} else {
				result = decompiler.getError();
			}

			debug("%d - %s: %s", script->getIndex(), script->getName().c_str(), result.c_str());
		}
	}
};

bool Console::Cmd_TestDecompiler(int argc, const char **argv) {
	DecompilingArchiveVisitor visitor;
	walkAllArchives(&visitor);

	debugPrintf("Successfully decompiled %d scripts out of %d\n", visitor.getOKScripts(), visitor.getTotalScripts());

	return true;
}

class TextureExtractingArchiveVisitor : public ArchiveVisitor {
public:
	void acceptLevelArchive(Resources::Level *level) override {
		decompileScriptChildren(level);
	}

	void acceptLocationArchive(Resources::Location *location) override {
		decompileScriptChildren(location);
	}

private:
	void decompileScriptChildren(Resources::Object *resource) {
		Common::Array<Resources::TextureSet *> textureSets = resource->listChildrenRecursive<Resources::TextureSet>();

		for (uint i = 0; i < textureSets.size(); i++) {
			Resources::TextureSet *textureSet = textureSets[i];
			textureSet->extractArchive();
		}
	}
};

bool Console::Cmd_ExtractAllTextures(int argc, const char **argv) {
	TextureExtractingArchiveVisitor visitor;
	walkAllArchives(&visitor);

	return true;
}

Common::Array<Resources::Anim *> Console::listAllLocationAnimations() const {
	Common::Array<Resources::Anim *> animations;

	Resources::Level *level = StarkGlobal->getCurrent()->getLevel();
	Resources::Location *location = StarkGlobal->getCurrent()->getLocation();
	animations.push_back(level->listChildrenRecursive<Resources::Anim>());
	animations.push_back(location->listChildrenRecursive<Resources::Anim>());

	return animations;
}

bool Console::Cmd_ListAnimations(int argc, const char **argv) {
	Current *current = StarkGlobal->getCurrent();
	if (!current) {
		debugPrintf("This command is only available in game.\n");
		return true;
	}

	Common::Array<Resources::Anim *> animations = listAllLocationAnimations();

	for (uint i = 0; i < animations.size(); i++) {
		Resources::Anim *anim = animations[i];
		Resources::Item *item = anim->findParent<Resources::Item>();

		debugPrintf("%d: %s - %s - in use: %d\n", i, item->getName().c_str(), anim->getName().c_str(), anim->isInUse());
	}

	return true;
}

bool Console::Cmd_ForceAnimation(int argc, const char **argv) {
	Current *current = StarkGlobal->getCurrent();
	if (!current) {
		debugPrintf("This command is only available in game.\n");
		return true;
	}

	if (argc < 2) {
		debugPrintf("Force the execution of an animation. Use listAnimations to get an id\n");
		debugPrintf("Usage :\n");
		debugPrintf("forceAnimation [id]\n");
		return true;
	}

	uint index = atoi(argv[1]);

	Common::Array<Resources::Anim *> animations = listAllLocationAnimations();
	if (index >= animations.size() ) {
		debugPrintf("Invalid animation %d\n", index);
		return true;
	}

	Resources::Anim *anim = animations[index];
	Resources::Item *item = anim->findParent<Resources::Item>();
	Resources::ItemVisual *sceneItem = item->getSceneInstance();

	if (!sceneItem->isEnabled()) {
		sceneItem->setEnabled(true);
	}

	sceneItem->playActionAnim(anim);

	return false;
}

bool Console::Cmd_DumpLocation(int argc, const char **argv) {
	if (StarkStaticProvider->isStaticLocation()) {
		StarkStaticProvider->getLocation()->print();
		return true;
	}

	Current *current = StarkGlobal->getCurrent();
	if (current) {
		current->getLocation()->print();
	} else {
		debugPrintf("Locations have not been loaded\n");
	}

	return true;
}

bool Console::Cmd_ListInventoryItems(int argc, const char **argv) {
	Resources::KnowledgeSet *inventory = StarkGlobal->getInventory();

	if (!inventory) {
		debugPrintf("The inventory has not been loaded\n");
		return true;
	}

	Common::Array<Resources::Item*> inventoryItems = inventory->listChildren<Resources::Item>(Resources::Item::kItemInventory);
	Common::Array<Resources::Item*>::iterator it = inventoryItems.begin();
	for (int i = 0; it != inventoryItems.end(); ++it, i++) {
		debugPrintf("Item %d: %s%s\n", i, (*it)->getName().c_str(), (*it)->isEnabled() ? " (enabled)" : "");
	}

	return true;
}

bool Console::Cmd_EnableInventoryItem(int argc, const char **argv) {
	Resources::KnowledgeSet *inventory = StarkGlobal->getInventory();

	if (!inventory) {
		debugPrintf("The inventory has not been loaded\n");
		return true;
	}

	if (argc != 2) {
		debugPrintf("Enable a specific inventory item. Use listInventoryItems to get an id\n");
		debugPrintf("Usage :\n");
		debugPrintf("enableInventoryItem [id]\n");
		return true;
	}

	uint num = atoi(argv[1]);
	Common::Array<Resources::Item*> inventoryItems = inventory->listChildren<Resources::Item>(Resources::Item::kItemInventory);
	if (num < inventoryItems.size()) {
		inventoryItems[num]->setEnabled(true);
	} else {
		debugPrintf("Invalid index %d, only %d indices available\n", num, inventoryItems.size());
	}

	return true;
}

bool Console::Cmd_ListLocations(int argc, const char **argv) {
	ArchiveLoader *archiveLoader = new ArchiveLoader();

	// Temporarily replace the global archive loader with our instance
	ArchiveLoader *gameArchiveLoader = StarkArchiveLoader;
	StarkArchiveLoader = archiveLoader;

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
	StarkArchiveLoader = gameArchiveLoader;

	delete archiveLoader;

	return true;
}

bool Console::Cmd_ChangeLocation(int argc, const char **argv) {
	if (argc >= 3) {
		// Assert indices
		Common::String xarcFileName = Common::String::format("%s/%s/%s.xarc", argv[1], argv[2], argv[2]);
		if (!Common::File::exists(xarcFileName)) {
			debugPrintf("Invalid location %s %s. Use listLocations to get correct indices\n", argv[1], argv[2]);
			return true;
		}

		uint levelIndex = strtol(argv[1] , nullptr, 16);
		uint locationIndex = strtol(argv[2] , nullptr, 16);

		StarkUserInterface->changeScreen(Screen::kScreenGame);

		if (!StarkGlobal->getRoot()) {
			StarkResourceProvider->initGlobal();
		}

		StarkResourceProvider->requestLocationChange(levelIndex, locationIndex);

		return false;
	} else if (argc > 1) {
		debugPrintf("Too few args\n");
	}

	debugPrintf("Change the current location. Use listLocations to get indices\n");
	debugPrintf("Usage :\n");
	debugPrintf("changeLocation [level] [location]\n");
	return true;
}

bool Console::Cmd_ChangeChapter(int argc, const char **argv) {
	if (!StarkGlobal->getLevel()) {
		debugPrintf("The global level has not been loaded\n");
		return true;
	}

	if (argc != 2) {
		debugPrintf("Change the current chapter\n");
		debugPrintf("Usage :\n");
		debugPrintf("changeChapter [value]\n");
		return true;
	}

	char *endPtr = nullptr;
	long value = strtol(argv[1], &endPtr, 10);
	if (*endPtr == '\0' && value >= 0 && value <= INT_MAX)
		StarkGlobal->setCurrentChapter((int32) value);
	else
		debugPrintf("Invalid chapter\n");

	return true;
}

bool Console::Cmd_Location(int argc, const char **argv) {
	Current *current = StarkGlobal->getCurrent();

	if (!current) {
		debugPrintf("Game levels have not been loaded\n");
		return true;
	}

	if (argc != 1) {
		debugPrintf("Display the current location\n");
		debugPrintf("Usage :\n");
		debugPrintf("location\n");
		return true;
	}

	debugPrintf("location: %02x %02x\n", current->getLevel()->getIndex(), current->getLocation()->getIndex());

	return true;
}

bool Console::Cmd_Chapter(int argc, const char **argv) {
	if (!StarkGlobal->getLevel()) {
		debugPrintf("The global level has not been loaded\n");
		return true;
	}

	if (argc != 1) {
		debugPrintf("Display the current chapter\n");
		debugPrintf("Usage :\n");
		debugPrintf("chapter\n");
		return true;
	}

	int32 value = StarkGlobal->getCurrentChapter();

	debugPrintf("chapter: %d\n", value);

	return true;
}

} // End of namespace Stark
