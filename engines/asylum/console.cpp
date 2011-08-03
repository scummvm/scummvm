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

#include "asylum/console.h"

#include "asylum/resources/actor.h"
#include "asylum/resources/encounters.h"
#include "asylum/resources/object.h"
#include "asylum/resources/script.h"
#include "asylum/resources/worldstats.h"

#include "asylum/system/cursor.h"
#include "asylum/system/graphics.h"
#include "asylum/system/screen.h"

#include "asylum/views/scene.h"
#include "asylum/views/video.h"

#include "asylum/asylum.h"
#include "asylum/respack.h"

#include "common/debug-channels.h"

namespace Asylum {

extern int g_debugActors;
extern int g_debugDrawRects;
extern int g_debugObjects;
extern int g_debugPolygons;
extern int g_debugSceneRects;
extern int g_debugScrolling;

const ResourcePackId puzzleToScenes[17] = {
	kResourcePackTowerCells,           // VCR
	kResourcePackCourtyardAndChapel,   // Pipes
	kResourcePackInnocentAbandoned,    // TicTacToe
	kResourcePackInnocentAbandoned,    // Lock
	kResourcePackInvalid,              // Puzzle 5 has no event handler
	kResourcePackLaboratory,           // Wheel
	kResourcePackLaboratory,           // Board Salvation
	kResourcePackLaboratory,           // Board Youth
	kResourcePackLaboratory,           // Board Key Hides To
	kResourcePackMorgueAndCemetery,    // Writings
	kResourcePackInvalid,              // ?? (11)
	kResourcePackMorgueAndCemetery,    // Morgue Door
	kResourcePackMansion,              // Clock
	kResourcePackMorgueAndCemetery,    // Time Machine
	kResourcePackLostVillage,          // Fisherman
	kResourcePackHive,                 // Hive Machine
	kResourcePackHive                  // Hive Control
};

Console::Console(AsylumEngine *engine) : _vm(engine) {
	// Commands
	DCmd_Register("help",           WRAP_METHOD(Console, cmdHelp));

	DCmd_Register("ls",             WRAP_METHOD(Console, cmdListFiles));

	DCmd_Register("actions",        WRAP_METHOD(Console, cmdListActions));
	DCmd_Register("actors",         WRAP_METHOD(Console, cmdListActors));
	DCmd_Register("flags",          WRAP_METHOD(Console, cmdListFlags));
	DCmd_Register("object",         WRAP_METHOD(Console, cmdShowObject));
	DCmd_Register("objects",        WRAP_METHOD(Console, cmdListObjects));
	DCmd_Register("world",          WRAP_METHOD(Console, cmdShowWorldStats));

	DCmd_Register("video",          WRAP_METHOD(Console, cmdPlayVideo));
	DCmd_Register("script",         WRAP_METHOD(Console, cmdRunScript));
	DCmd_Register("scene",          WRAP_METHOD(Console, cmdChangeScene));
	DCmd_Register("encounter",      WRAP_METHOD(Console, cmdRunEncounter));
	DCmd_Register("puzzle",         WRAP_METHOD(Console, cmdRunPuzzle));

	DCmd_Register("palette",        WRAP_METHOD(Console, cmdSetPalette));
	DCmd_Register("draw",           WRAP_METHOD(Console, cmdDrawResource));

	DCmd_Register("toggle_flag",    WRAP_METHOD(Console, cmdToggleFlag));

	// Variables
	DVar_Register("show_actors",     &g_debugActors,     DVAR_INT, 0);
	DVar_Register("show_drawrects",  &g_debugDrawRects,  DVAR_INT, 0);
	DVar_Register("show_objects",    &g_debugObjects,    DVAR_INT, 0);
	DVar_Register("show_polygons",   &g_debugPolygons,   DVAR_INT, 0);
	DVar_Register("show_scenerects", &g_debugSceneRects, DVAR_INT, 0);
	DVar_Register("use_scrolling",   &g_debugScrolling,  DVAR_INT, 0);
}

Console::~Console() {
	DebugMan.clearAllDebugChannels();

	// Zero passed pointers
	_vm = NULL;
}

//////////////////////////////////////////////////////////////////////////
// Help
//////////////////////////////////////////////////////////////////////////
bool Console::cmdHelp(int, const char **) {
	DebugPrintf("Debug flags\n");
	DebugPrintf("-----------\n");
	DebugPrintf(" debugflag_list    - Lists the available debug flags and their status\n");
	DebugPrintf(" debugflag_enable  - Enables a debug flag\n");
	DebugPrintf(" debugflag_disable - Disables a debug flag\n");
	DebugPrintf(" show_actors       - Show actors\n");
	DebugPrintf(" show_objects      - Show objects\n");
	DebugPrintf(" show_polygons     - Show polygons\n");
	DebugPrintf(" show_drawrects    - Show drawing rects\n");;
	DebugPrintf(" use_scrolling     - Use scrolling\n");
	DebugPrintf("\n");
	DebugPrintf("Commands\n");
	DebugPrintf("--------\n");
	DebugPrintf(" ls          - list engine files\n");
	DebugPrintf("\n");
	DebugPrintf(" actors      - show actors information\n");
	DebugPrintf(" actions     - show action information\n");
	DebugPrintf(" flags       - show flags\n");
	DebugPrintf(" object      - inspect a particular object\n");
	DebugPrintf(" objects     - show objects information\n");
	DebugPrintf(" world       - show worldstats\n");
	DebugPrintf("\n");
	DebugPrintf(" video       - play a video\n");
	DebugPrintf(" script      - run a script\n");
	DebugPrintf(" scene       - change the scene\n");
	DebugPrintf(" encounter   - run an encounter\n");
	DebugPrintf(" puzzle      - run an puzzle\n");
	DebugPrintf("\n");
	DebugPrintf(" palette     - set the screen palette\n");
	DebugPrintf(" draw        - draw a resource\n");
	DebugPrintf("\n");
	DebugPrintf(" toggle_flag - toggle a flag\n");
	DebugPrintf("\n");

	return true;
}

//////////////////////////////////////////////////////////////////////////
// List commands
//////////////////////////////////////////////////////////////////////////
bool Console::cmdListFiles(int argc, const char **argv) {
	if (argc != 2) {
		DebugPrintf("Syntax: %s <filter> (use * for all)\n", argv[0]);
		return true;
	}

	Common::String filter(const_cast<char *>(argv[1]));

	Common::ArchiveMemberList list;
	int count = SearchMan.listMatchingMembers(list, filter);

	DebugPrintf("Number of matches: %d\n", count);
	for (Common::ArchiveMemberList::iterator it = list.begin(); it != list.end(); ++it)
		DebugPrintf(" %s\n", (*it)->getName().c_str());

	return true;
}

bool Console::cmdListActions(int32 argc, const char **argv) {
	if (argc != 1 && argc != 2) {
		DebugPrintf("Syntax: %s <index> (use nothing for all)\n", argv[0]);
		return true;
	}

	if (argc == 1) {
		for (uint32 i = 0; i < getWorld()->actions.size(); i++)
			DebugPrintf("%s\n", getWorld()->actions[i]->toString().c_str());

	} else {
		int index = atoi(argv[1]);
		int maxIndex = getWorld()->actions.size() - 1;

		if (maxIndex == -1) {
			DebugPrintf("[error] No actions are present!\n");
			return true;
		}

		if (index < 0 || index > maxIndex) {
			DebugPrintf("[error] index should be between 0 and %d\n", maxIndex);
			return true;
		}

		DebugPrintf("%s\n", getWorld()->actions[index]->toString().c_str());
	}

	return true;
}

bool Console::cmdListActors(int32 argc, const char **argv) {
	if (argc != 1 && argc != 2 && argc != 4) {
		DebugPrintf("Syntax: %s <index> (use nothing for all) (<x>, <y>)\n", argv[0]);
		return true;
	}

	if (argc == 1) {
		for (uint32 i = 0; i < getWorld()->actors.size(); i++)
			DebugPrintf("%s\n", getWorld()->actors[i]->toString().c_str());

	} else if (argc == 2 || argc == 4) {
		int index = atoi(argv[1]);
		int maxIndex = getWorld()->actors.size() - 1;

		if (index < 0 || index > maxIndex) {
			DebugPrintf("[error] index should be between 0 and %d\n", maxIndex);
			return true;
		}

		if (argc == 2) {
			DebugPrintf("%s\n", getWorld()->actors[index]->toString(false).c_str());
			return true;
		}

		// Adjust actor coordinates
		int16 x = (int16)atoi(argv[2]);
		int16 y = (int16)atoi(argv[3]);

		// TODO add error handling

		*getWorld()->actors[index]->getPoint1() = Common::Point(x, y);
	}

	return true;
}

bool Console::cmdListFlags(int32 argc, const char **argv) {
	if (argc != 1 && argc != 2) {
		DebugPrintf("Syntax: %s <type> (nothing: all  -  1: show set flags  -  0: show unset flags)\n", argv[0]);
		return true;
	}

	// Show all flags
	if (argc == 1) {
		for (int32 i = 0; i < 1512; i++) {
			DebugPrintf("%04d: %d    ", i, _vm->isGameFlagSet((GameFlag)i));

			if ((i + 1) % 10 == 0)
				DebugPrintf("\n");
		}
		DebugPrintf("\n");
	} else {
		int32 type = atoi(argv[1]);

		if (type != 0 && type != 1) {
			DebugPrintf("Syntax: %s <type> (nothing: all  -  1: show set flags  -  0: show unset flags)\n", argv[0]);
			return true;
		}

		// Show only set/unset flags
		int count = 0;
		for (int32 i = 0; i < 1512; i++) {
			if (_vm->isGameFlagSet((GameFlag)i) == (bool)type) {
				DebugPrintf("%04d: %d    ", i, _vm->isGameFlagSet((GameFlag)i));
				++count;
			}

			if ((count + 1) % 10 == 0)
				DebugPrintf("\n");
		}
		DebugPrintf("\n\n%s flags: %d\n", (type ? "Set" : "Unset"), count);
	}

	return true;
}

bool Console::cmdShowWorldStats(int32, const char **) {
	DebugPrintf("WorldStats\n");
	DebugPrintf("----------\n");
	DebugPrintf("%s", getWorld()->toString().c_str());

	return true;
}

bool Console::cmdShowObject(int32 argc, const char **argv) {
	if (argc != 3) {
		DebugPrintf("Syntax: %s [id|idx] <target>\n", argv[0]);
		return true;
	}

	if (Common::String(argv[1]) == "id") {
		int id = atoi(argv[2]);
		for (uint32 i = 0; i < getWorld()->objects.size(); i++) {
			if (getWorld()->objects[i]->getId() == id) {
				DebugPrintf("%s", getWorld()->objects[i]->toString(false).c_str());
				return true;
			}
		}
		DebugPrintf("No object with id %d found\n", id);
	} else if (Common::String(argv[1]) == "idx") {
		int index = atoi(argv[2]);
		int maxIndex = getWorld()->objects.size() - 1;

		if (index < 0 || index > maxIndex) {
			DebugPrintf("[error] index should be between 0 and %d\n", maxIndex);
			return true;
		}

		DebugPrintf("%s", getWorld()->objects[index]->toString(false).c_str());

	} else {
		DebugPrintf("[error] valid options are 'id' and 'idx'\n");
	}

	return true;
}

bool Console::cmdListObjects(int32 argc, const char **argv) {
	if (argc != 2) {
		DebugPrintf("Syntax: %s [onscreen|*]\n", argv[0]);
		return true;
	}

	if (argc == 2) {
		if (Common::String(argv[1]) == "onscreen") {
			for (uint32 i = 0; i < getWorld()->objects.size(); i++) {
				if (getWorld()->objects[i]->isOnScreen()) {
					DebugPrintf("%s", getWorld()->objects[i]->toString().c_str());
				}
			}

			DebugPrintf("Total: %d\n", getWorld()->objects.size());

		} else if (Common::String(argv[1]) == "*"){
			for (uint32 i = 0; i < getWorld()->objects.size(); i++)
				DebugPrintf("%s", getWorld()->objects[i]->toString().c_str());

			DebugPrintf("Total: %d\n", getWorld()->objects.size());

		} else {
			DebugPrintf("[error] valid options are 'onscreen' and '*'\n");
		}
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////
// Video / Scene / Script commands
//////////////////////////////////////////////////////////////////////////
bool Console::cmdPlayVideo(int32 argc, const char **argv) {
	if (argc != 2) {
		DebugPrintf("Syntax: %s <video number>\n", argv[0]);
		return true;
	}

	int index = atoi(argv[1]);

	// Check if the video exists
	char filename[20];
	sprintf(filename, "mov%03d.smk", index);
	if (!SearchMan.hasFile(filename)) {
		DebugPrintf("[Error] Movie %d does not exists\n", index);
		return true;
	}

	_vm->_delayedVideoIndex = index;

	return false;
}

bool Console::cmdRunScript(int32 argc, const char **argv) {
	if (argc != 3) {
		DebugPrintf("Syntax: %s <script index> <actor index>\n", argv[0]);
		return true;
	}

	int32 index = atoi(argv[1]);
	int32 actor = atoi(argv[2]);

	// Check parameters
	if (index < 0 || index >= (int32)getScript()->_scripts.size()) {
		DebugPrintf("[Error] Invalid index (was: %d - valid: [0-%d])\n", index, _vm->encounter()->items()->size() - 1);
		return true;
	}

	if (actor < 0 || actor >= (int32)getWorld()->actors.size()) {
		DebugPrintf("[Error] Invalid actor index (was: %d - valid: [0-%d])\n", actor, getWorld()->actors.size() - 1);
	}

	getScript()->queueScript(index, actor);

	return false;
}

bool Console::cmdChangeScene(int32 argc, const char **argv) {
	if (argc != 2) {
		DebugPrintf("Syntax: %s <scene number>\n", argv[0]);
		return true;
	}

	ResourcePackId index = (ResourcePackId)atoi(argv[1]);

	// Check if the scene exists
	char filename[20];
	sprintf(filename, "scn.%03d", index);
	if (!SearchMan.hasFile(filename)) {
		DebugPrintf("[Error] Scene %d does not exists\n", index);
		return true;
	}

	_vm->_delayedSceneIndex = index;

	return false;
}

bool Console::cmdRunEncounter(int32 argc, const char **argv) {
	if (argc != 2) {
		DebugPrintf("Syntax: %s <encounter index>\n", argv[0]);
		return true;
	}

	int32 index = atoi(argv[1]);

	// Check index is valid
	if (index < 0 || index >= (int32)_vm->encounter()->_items.size()) {
		DebugPrintf("[Error] Invalid index (was: %d - valid: [0-%d])\n", index, _vm->encounter()->_items.size() - 1);
		return true;
	}

	// Line: 12/15 :: 0x25 (1, 1584, 1584, 0, 0, 0, 0, 0, 0) // First Encounter
	// TODO update with array of valid objects
	_vm->encounter()->run(index, kObjectNone, kObjectNone, kActorMax);

	return false;
}

bool Console::cmdRunPuzzle(int32 argc, const char **argv) {
	if (argc != 2) {
		DebugPrintf("Syntax: %s <puzzle index>\n", argv[0]);
		DebugPrintf("        0   VCR\n");
		DebugPrintf("        1   Pipes\n");
		DebugPrintf("        2   TicTacToe\n");
		DebugPrintf("        3   Lock\n");
		DebugPrintf("        4   N/A\n");
		DebugPrintf("        5   Wheel\n");
		DebugPrintf("        6   BoardSalvation\n");
		DebugPrintf("        7   BoardYouth\n");
		DebugPrintf("        8   BoardKeyHidesTo\n");
		DebugPrintf("        9   Writings\n");
		DebugPrintf("        10  Unknown\n");
		DebugPrintf("        11  MorgueDoor\n");
		DebugPrintf("        12  Clock\n");
		DebugPrintf("        13  TimerMachine\n");
		DebugPrintf("        14  Fisherman\n");
		DebugPrintf("        15  HiveMachine\n");
		DebugPrintf("        16  HiveControl\n");
		return true;
	}

	int32 index = atoi(argv[1]);

	// Check index is valid
	if (index < 0 || index >= ARRAYSIZE(puzzleToScenes)) {
		DebugPrintf("[Error] Invalid index (was: %d - valid: [0-%d])\n", index, ARRAYSIZE(_vm->_puzzles));
		return true;
	}

	EventHandler *puzzle = _vm->getPuzzle((uint32)index);
	if (puzzle == NULL) {
		DebugPrintf("[Error] This puzzle does not exists (%d)", index);
		return true;
	}

	getScreen()->clear();
	getScreen()->clearGraphicsInQueue();

	// Save previous scene
	_vm->_previousScene = getScene();

	// Load puzzle scene
	_vm->_scene = new Scene(_vm);
	getScene()->load(puzzleToScenes[index]);

	_vm->switchEventHandler(puzzle);

	return false;
}

bool Console::cmdSetPalette(int32 argc, const char **argv) {
	if (argc != 3) {
		DebugPrintf("Syntax: %s <pack> <index>\n", argv[0]);
		return true;
	}

	int32 pack = atoi(argv[1]);
	int32 index = atoi(argv[2]);

	// Check resource pack
	if (pack < 0 || pack > 18) {
		DebugPrintf("[Error] Invalid resource pack (was: %d - valid: [0-18])\n", pack);
		return true;
	}

	// Check index
	if (index < 0) {
		DebugPrintf("[Error] Invalid index (was: %d - valid: > 0)\n", index);
		return true;
	}

	// Try loading resource
	ResourceId id = MAKE_RESOURCE((uint32)pack, index);

	ResourceEntry *entry = getResource()->get(id);
	if (!entry) {
		DebugPrintf("[Error] Invalid resource (0x%X)\n", id);
		return true;
	}

	getScreen()->setPalette(id);

	return true;
}

bool Console::cmdDrawResource(int32 argc, const char **argv) {
	if (argc != 3 && argc != 4) {
		DebugPrintf("Syntax: %s <pack> <index> (<frame>)\n", argv[0]);
		return true;
	}

	int32 pack = atoi(argv[1]);
	int32 index = atoi(argv[2]);

	int32 frame = 0;
	if (argc == 4)
		frame = atoi(argv[3]);

	// Check resource pack
	if (pack < 0 || pack > 18) {
		DebugPrintf("[Error] Invalid resource pack (was: %d - valid: [0-18])\n", pack);
		return true;
	}

	// Check index
	if (index < 0) {
		DebugPrintf("[Error] Invalid index (was: %d - valid: > 0)\n", index);
		return true;
	}

	ResourceId resourceId = MAKE_RESOURCE((uint32)pack, index);

	// Try loading resource
	GraphicResource *resource = new GraphicResource(_vm);
	if (!resource->load(resourceId)) {
		DebugPrintf("[Error] Invalid resource index (was: %d)\n", index);
		delete resource;
		return true;
	}

	if (frame < 0 || frame >= (int32)resource->count()) {
		DebugPrintf("[Error] Invalid resource frame index (was: %d , max: %d)\n", frame, resource->count() - 1);
		delete resource;
		return true;
	}

	delete resource;

	// Stop current event handler (to prevent screen refresh)
	_vm->switchEventHandler(NULL);
	getCursor()->hide();

	// Draw resource
	getScreen()->clear();
	getScreen()->draw(resourceId, (uint32)frame, Common::Point(0, 0));
	getScreen()->copyBackBufferToScreen();

	g_system->updateScreen();

	return false;
}

//////////////////////////////////////////////////////////////////////////
// Flags commands
//////////////////////////////////////////////////////////////////////////
bool Console::cmdToggleFlag(int32 argc, const char **argv) {
	if (argc != 2 || atoi(argv[1]) > 1512 || atoi(argv[1]) < 0) {
		DebugPrintf("Syntax: <value> between 0 and 1512\n");
		return true;
	}

	_vm->toggleGameFlag((GameFlag)atoi(argv[1]));
	DebugPrintf("Flag %d == %d\n", atoi(argv[1]), _vm->isGameFlagSet((GameFlag)atoi(argv[1])));

	return true;
}

} // End of namespace Asulym
