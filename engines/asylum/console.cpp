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

#include "common/debug-channels.h"

#include "asylum/console.h"

#include "asylum/puzzles/puzzles.h"

#include "asylum/resources/actor.h"
#include "asylum/resources/encounters.h"
#include "asylum/resources/object.h"
#include "asylum/resources/script.h"
#include "asylum/resources/worldstats.h"

#include "asylum/system/cursor.h"
#include "asylum/system/graphics.h"
#include "asylum/system/screen.h"
#include "asylum/system/text.h"

#include "asylum/views/scene.h"
#include "asylum/views/video.h"

#include "asylum/asylum.h"
#include "asylum/respack.h"

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

static const struct EncounterData {
	int32 index;
	uint32 objectId1;
	uint32 objectId2;
	ActorIndex actorIndex;
} encounterData[13][20] = {
	// TowerCells
	{
		{ 0, kObjectPreAlphaNut,  kObjectPreAlphaNut,  kActorMax},
		{ 1, kObjectPreAlphaNut2, kObjectPreAlphaNut2, kActorMax},
		{ 2, kObjectRocker,       kObjectRocker,       kActorMax},
		{73, kObjectNone,         kObjectNone,         kActorMax},
		{-1, kObjectNone, kObjectNone, kActorMax}
	},
	// InnocentAbandoned
	{
		{ 3, 1072, 1091, kActorMax},
		{ 4, 1061, 1072, kActorMax},
		{ 5, 1200, 1199, kActorMax},
		{ 7, 1105,  991, kActorMax},
		{ 9, 1012, 1011, kActorMax},
		{10,  993,  993, kActorMax},
		{11, 1013, 1013, kActorMax},
		{12, 1082, 1084, kActorMax},
		{13, 1001, 1001, kActorMax},
		{14, 1587, 2280, kActorMax},
		{74, 2992, 2992, kActorMax},
		{76, 2990, 2990, kActorMax},
		{77, 2990, 2990, kActorMax},
		{78, 2990, 2990, kActorMax},
		{-1, kObjectNone, kObjectNone, kActorMax}
	},
	// CourtyardAndChapel
	{
		{36, 820, 820, kActorMax},
		{37, 863, 863, kActorMax},
		{38, 862, 1038, kActorMax},
		{39, 844, 844, kActorMax},
		{40, 845, 845, kActorMax},
		{41, 846, 846, kActorMax},
		{43, 873, 801, kActorMax},
		{-1, kObjectNone, kObjectNone, kActorMax}
	},
	// CircusOfFools
	{
		{-1, kObjectNone, kObjectNone, kActorMax}
	},
	// Laboratory
	{
		{-1, kObjectNone, kObjectNone, kActorMax}
	},
	// Hive
	{
		{-1, kObjectNone, kObjectNone, kActorMax}
	},
	// MorgueAndCemetery
	{
		{-1, kObjectNone, kObjectNone, kActorMax}
	},
	// LostVillage
	{
		{-1, kObjectNone, kObjectNone, kActorMax}
	},
	// Gauntlet
	{
		{-1, kObjectNone, kObjectNone, kActorMax}
	},
	// Mansion
	{
		{-1, kObjectNone, kObjectNone, kActorMax}
	},
	// Cave
	{
		{-1, kObjectNone, kObjectNone, kActorMax}
	},
	// Maze
	{
		{-1, kObjectNone, kObjectNone, kActorMax}
	},
	// MorgansLastGame
	{
		{-1, kObjectNone, kObjectNone, kActorMax}
	}
};

static const int32 itemIndices[][16] = {
	{61, 69},
	{107, 134, 104, 113, 110, 112, 117, 109, 108, 111, 106},
	{170, 182, 181, 172, 171, 169},
	{61, 64, 66, 67, 68, 69, 70, 78, 77},
	{197},
	{59, 81, 60, 84, 88, 54, 74, 139, 97, 121},
	{239, 234, 249, 250, 251, 263, 237, 253},
	{58, 59, 60, 111, 75, 76, 77, 78},
	{284, 285, 286, 329, 330, 331, 332, 322, 465},
	{91, 92, 93, 94, 95},
	{69, 70, 78}
};

Console::Console(AsylumEngine *engine) : _vm(engine) {
	// Commands
	registerCmd("help",           WRAP_METHOD(Console, cmdHelp));

	registerCmd("ls",             WRAP_METHOD(Console, cmdListFiles));

	registerCmd("action",         WRAP_METHOD(Console, cmdShowAction));
	registerCmd("actions",        WRAP_METHOD(Console, cmdListActions));
	registerCmd("actors",         WRAP_METHOD(Console, cmdListActors));
	registerCmd("flags",          WRAP_METHOD(Console, cmdListFlags));
	registerCmd("object",         WRAP_METHOD(Console, cmdShowObject));
	registerCmd("objects",        WRAP_METHOD(Console, cmdListObjects));
	registerCmd("world",          WRAP_METHOD(Console, cmdShowWorldStats));

	registerCmd("video",          WRAP_METHOD(Console, cmdPlayVideo));
	registerCmd("script",         WRAP_METHOD(Console, cmdRunScript));
	registerCmd("show_script",    WRAP_METHOD(Console, cmdShowScript));
	registerCmd("kill_script",    WRAP_METHOD(Console, cmdKillScript));

	registerCmd("scene",          WRAP_METHOD(Console, cmdChangeScene));
	registerCmd("puzzle",         WRAP_METHOD(Console, cmdRunPuzzle));

	registerCmd("get_status",     WRAP_METHOD(Console, cmdGetStatus));
	registerCmd("set_status",     WRAP_METHOD(Console, cmdSetStatus));

	registerCmd("encounter",      WRAP_METHOD(Console, cmdRunEncounter));
	registerCmd("show_enc",       WRAP_METHOD(Console, cmdShowEncounter));

	registerCmd("items",          WRAP_METHOD(Console, cmdListItems));
	registerCmd("grab",           WRAP_METHOD(Console, cmdAddToInventory));
	registerCmd("throw",          WRAP_METHOD(Console, cmdRemoveFromInventory));

	registerCmd("palette",        WRAP_METHOD(Console, cmdSetPalette));
	registerCmd("draw",           WRAP_METHOD(Console, cmdDrawResource));

	registerCmd("toggle_flag",    WRAP_METHOD(Console, cmdToggleFlag));

	// Variables
	registerVar("show_actors",     &g_debugActors);
	registerVar("show_drawrects",  &g_debugDrawRects);
	registerVar("show_objects",    &g_debugObjects);
	registerVar("show_polygons",   &g_debugPolygons);
	registerVar("show_scenerects", &g_debugSceneRects);
	registerVar("use_scrolling",   &g_debugScrolling);
}

Console::~Console() {
	DebugMan.clearAllDebugChannels();
}

//////////////////////////////////////////////////////////////////////////
// Help
//////////////////////////////////////////////////////////////////////////
bool Console::cmdHelp(int, const char **) {
	debugPrintf("Debug flags\n");
	debugPrintf("-----------\n");
	debugPrintf(" debugflag_list    - Lists the available debug flags and their status\n");
	debugPrintf(" debugflag_enable  - Enables a debug flag\n");
	debugPrintf(" debugflag_disable - Disables a debug flag\n");
	debugPrintf("\n");
	debugPrintf(" show_actors       - Show actors\n");
	debugPrintf(" show_objects      - Show objects\n");
	debugPrintf(" show_polygons     - Show polygons\n");
	debugPrintf(" show_drawrects    - Show drawing rects\n");
	debugPrintf(" use_scrolling     - Scroll scene using the mouse\n");
	debugPrintf("\n");
	debugPrintf("Commands\n");
	debugPrintf("--------\n");
	debugPrintf(" ls          - list engine files\n");
	debugPrintf("\n");
	debugPrintf(" actors      - show actors information\n");
	debugPrintf(" action      - show action information\n");
	debugPrintf(" actions     - list actions information\n");
	debugPrintf(" flags       - show flags\n");
	debugPrintf(" object      - inspect a particular object\n");
	debugPrintf(" objects     - show objects information\n");
	debugPrintf(" world       - show worldstats\n");
	debugPrintf("\n");
	debugPrintf(" video       - play a video\n");
	debugPrintf(" script      - run a script\n");
	debugPrintf(" scene       - change the scene\n");
	debugPrintf(" show_script - show script commands\n");
	debugPrintf(" kill_script - terminate a script\n");
	debugPrintf(" puzzle      - run an puzzle\n");
	debugPrintf("\n");
	debugPrintf(" get_status  - get actor's status\n");
	debugPrintf(" set_status  - set actor's status\n");
	debugPrintf("\n");
	debugPrintf(" encounter   - run an encounter\n");
	debugPrintf(" show_enc    - show encounter commands\n");
	debugPrintf("\n");
	debugPrintf(" items       - list all grabbable objects\n");
	debugPrintf(" grab        - add an item to inventory\n");
	debugPrintf(" throw       - remove an item from inventory\n");
	debugPrintf("\n");
	debugPrintf(" palette     - set the screen palette\n");
	debugPrintf(" draw        - draw a resource\n");
	debugPrintf("\n");
	debugPrintf(" toggle_flag - toggle a flag\n");
	debugPrintf("\n");

	return true;
}

//////////////////////////////////////////////////////////////////////////
// List commands
//////////////////////////////////////////////////////////////////////////
bool Console::cmdListFiles(int argc, const char **argv) {
	if (argc != 2) {
		debugPrintf("Syntax: %s <filter> (use * for all)\n", argv[0]);
		return true;
	}

	Common::String filter(const_cast<char *>(argv[1]));

	Common::ArchiveMemberList list;
	int count = SearchMan.listMatchingMembers(list, filter);

	debugPrintf("Number of matches: %d\n", count);
	for (Common::ArchiveMemberList::iterator it = list.begin(); it != list.end(); ++it)
		debugPrintf(" %s\n", (*it)->getName().c_str());

	return true;
}

bool Console::cmdListActions(int argc, const char **argv) {
	if (argc != 1 && argc != 2) {
		debugPrintf("Syntax: %s <index> (use nothing for all)\n", argv[0]);
		return true;
	}

	if (argc == 1) {
		for (uint32 i = 0; i < getWorld()->actions.size(); i++)
			debugPrintf("%s\n", getWorld()->actions[i]->toString().c_str());

	} else {
		int index = atoi(argv[1]);
		int maxIndex = getWorld()->actions.size() - 1;

		if (maxIndex == -1) {
			debugPrintf("[error] No actions are present!\n");
			return true;
		}

		if (index < 0 || index > maxIndex) {
			debugPrintf("[error] index should be between 0 and %d\n", maxIndex);
			return true;
		}

		debugPrintf("%s\n", getWorld()->actions[index]->toString().c_str());
	}

	return true;
}

bool Console::cmdListActors(int argc, const char **argv) {
	if (argc != 1 && argc != 2 && argc != 4) {
		debugPrintf("Syntax: %s <index> (use nothing for all) (<x>, <y>)\n", argv[0]);
		return true;
	}

	if (argc == 1) {
		Actor *player = getScene()->getActor();

		for (uint32 i = 0; i < getWorld()->actors.size(); i++) {
			Actor *actor = getWorld()->actors[i];

			debugPrintf("%c", actor == player ? '*' : ' ');
			debugPrintf("%s\n", actor->toString().c_str());
		}
	} else if (argc == 2 || argc == 4) {
		int index = atoi(argv[1]);
		int maxIndex = getWorld()->actors.size() - 1;

		if (index < 0 || index > maxIndex) {
			debugPrintf("[error] index should be between 0 and %d\n", maxIndex);
			return true;
		}

		if (argc == 2) {
			debugPrintf("%s\n", getWorld()->actors[index]->toString(false).c_str());
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

bool Console::cmdListFlags(int argc, const char **argv) {
	if (argc != 1 && argc != 2) {
		debugPrintf("Syntax: %s <type> (nothing: all  -  1: show set flags  -  0: show unset flags)\n", argv[0]);
		return true;
	}

	// Show all flags
	if (argc == 1) {
		for (int32 i = 0; i < 1512; i++) {
			debugPrintf("%04d: %d    ", i, _vm->isGameFlagSet((GameFlag)i));

			if ((i + 1) % 10 == 0)
				debugPrintf("\n");
		}
		debugPrintf("\n");
	} else {
		int32 type = atoi(argv[1]);

		if (type != 0 && type != 1) {
			debugPrintf("Syntax: %s <type> (nothing: all  -  1: show set flags  -  0: show unset flags)\n", argv[0]);
			return true;
		}

		// Show only set/unset flags
		int count = 0;
		for (int32 i = 0; i < 1512; i++) {
			if (_vm->isGameFlagSet((GameFlag)i) == (bool)type) {
				debugPrintf("%04d: %d    ", i, _vm->isGameFlagSet((GameFlag)i));
				++count;
			}

			if ((count + 1) % 10 == 0)
				debugPrintf("\n");
		}
		debugPrintf("\n\n%s flags: %d\n", (type ? "Set" : "Unset"), count);
	}

	return true;
}

bool Console::cmdShowWorldStats(int, const char **) {
	debugPrintf("WorldStats\n");
	debugPrintf("----------\n");
	debugPrintf("%s", getWorld()->toString().c_str());

	return true;
}

bool Console::cmdShowObject(int argc, const char **argv) {
	if (argc != 3) {
		debugPrintf("Syntax: %s [id|idx] <target>\n", argv[0]);
		return true;
	}

	if (Common::String(argv[1]) == "id") {
		int id = atoi(argv[2]);
		for (uint32 i = 0; i < getWorld()->objects.size(); i++) {
			if (getWorld()->objects[i]->getId() == id) {
				debugPrintf("%s", getWorld()->objects[i]->toString(false).c_str());
				return true;
			}
		}
		debugPrintf("No object with id %d found\n", id);
	} else if (Common::String(argv[1]) == "idx") {
		int index = atoi(argv[2]);
		int maxIndex = getWorld()->objects.size() - 1;

		if (index < 0 || index > maxIndex) {
			debugPrintf("[error] index should be between 0 and %d\n", maxIndex);
			return true;
		}

		debugPrintf("%s", getWorld()->objects[index]->toString(false).c_str());

	} else {
		debugPrintf("[error] valid options are 'id' and 'idx'\n");
	}

	return true;
}

bool Console::cmdShowAction(int argc, const char **argv) {
	if (argc != 3) {
		debugPrintf("Syntax: %s [id|idx] <target>\n", argv[0]);
		return true;
	}

	if (Common::String(argv[1]) == "id") {
		int id = atoi(argv[2]);
		for (uint32 i = 0; i < getWorld()->actions.size(); i++) {
			if (getWorld()->actions[i]->id == id) {
				debugPrintf("%s", getWorld()->actions[i]->toString().c_str());
				return true;
			}
		}
		debugPrintf("No action with id %d found\n", id);
	} else if (Common::String(argv[1]) == "idx") {
		int index = atoi(argv[2]);
		int maxIndex = getWorld()->actions.size() - 1;

		if (index < 0 || index > maxIndex) {
			debugPrintf("[error] index should be between 0 and %d\n", maxIndex);
			return true;
		}

		debugPrintf("%s", getWorld()->actions[index]->toString().c_str());

	} else {
		debugPrintf("[error] valid options are 'id' and 'idx'\n");
	}

	return true;
}

bool Console::cmdListObjects(int argc, const char **argv) {
	if (argc != 2) {
		debugPrintf("Syntax: %s [onscreen|*]\n", argv[0]);
		return true;
	}

	if (argc == 2) {
		if (Common::String(argv[1]) == "onscreen") {
			for (uint32 i = 0; i < getWorld()->objects.size(); i++) {
				if (getWorld()->objects[i]->isOnScreen()) {
					debugPrintf("%s", getWorld()->objects[i]->toString().c_str());
				}
			}

			debugPrintf("Total: %d\n", getWorld()->objects.size());

		} else if (Common::String(argv[1]) == "*") {
			for (uint32 i = 0; i < getWorld()->objects.size(); i++)
				debugPrintf("%s", getWorld()->objects[i]->toString().c_str());

			debugPrintf("Total: %d\n", getWorld()->objects.size());

		} else {
			debugPrintf("[error] valid options are 'onscreen' and '*'\n");
		}
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////
// Video / Scene / Script commands
//////////////////////////////////////////////////////////////////////////
bool Console::cmdPlayVideo(int argc, const char **argv) {
	if (argc != 2) {
		debugPrintf("Syntax: %s <video number>\n", argv[0]);
		return true;
	}

	int index = atoi(argv[1]);

	// Check if the video exists
	char filename[20];
	snprintf(filename, 20, "mov%03d.smk", index);
	if (!SearchMan.hasFile(filename)) {
		debugPrintf("[Error] Movie %d does not exists\n", index);
		return true;
	}

	_vm->_delayedVideoIndex = index;

	return false;
}

bool Console::cmdGetStatus(int argc, const char **argv) {
	Actor *actor;

	if (argc == 1) {
		actor = getScene()->getActor();
	} else if (argc == 2) {
		int32 index = atoi(argv[1]);

		if (index < 0 || index >= (int32)getWorld()->actors.size()) {
			debugPrintf("[Error] Invalid actor index (was: %d - valid: [0-%d])\n", index, getWorld()->actors.size() - 1);
			return true;
		}

		actor = getScene()->getActor(index);
	} else {
		debugPrintf("Syntax: %s (<actor index>)\n", argv[0]);
		return true;
	}

	debugPrintf("%s's status = %d\n", actor->getName(), actor->getStatus());
	return true;
}

bool Console::cmdSetStatus(int argc, const char **argv) {
	Actor *actor;
	int32 status;

	if (argc == 2) {
		actor = getScene()->getActor();
		status = atoi(argv[1]);
	} else if (argc == 3) {
		int32 index = atoi(argv[1]);

		if (index < 0 || index >= (int32)getWorld()->actors.size()) {
			debugPrintf("[Error] Invalid actor index (was: %d - valid: [0-%d])\n", index, getWorld()->actors.size() - 1);
			return true;
		}

		actor = getScene()->getActor(index);
		status = atoi(argv[2]);
	} else {
		debugPrintf("Syntax: %s (<actor index>) <status>\n", argv[0]);
		return true;
	}

	if (status <= 0 || status > 21) {
		debugPrintf("[Error] Invalid status (was: %d - valid: [1-21])\n", status);
		return true;
	}

	actor->updateStatus((ActorStatus)status);
	return false;
}

bool Console::cmdShowScript(int argc, const char **argv) {
	if (argc != 2) {
		debugPrintf("Syntax: %s <script index>\n", argv[0]);
		return true;
	}

	int32 index = atoi(argv[1]);

	// Check parameters
	if (index < 0 || index >= (int32)getWorld()->numScripts) {
		debugPrintf("[Error] Invalid index (was: %d - valid: [0-%d])\n", index, getWorld()->numScripts - 1);
		return true;
	}

	int32 lines = getScript()->_scripts[index].commands[0].numLines;
	for (uint8 i = 0; i <= lines; i++) {
		Asylum::ScriptManager::ScriptEntry *cmd = &getScript()->_scripts[index].commands[i];

		debugPrintf("%02d: [0x%02X] %s (%d, %d, %d, %d, %d, %d, %d, %d, %d)\n",
			i, cmd->opcode, getScript()->_opcodes[cmd->opcode]->name,
			cmd->param1, cmd->param2, cmd->param3, cmd->param4, cmd->param5,
			cmd->param6, cmd->param7, cmd->param8, cmd->param9);
	}

	return true;
}

bool Console::cmdRunScript(int argc, const char **argv) {
	if (argc != 3) {
		debugPrintf("Syntax: %s <script index> <actor index>\n", argv[0]);
		return true;
	}

	int32 index = atoi(argv[1]);
	int32 actor = atoi(argv[2]);

	// Check parameters
	if (index < 0 || index >= (int32)getScript()->_scripts.size()) {
		debugPrintf("[Error] Invalid index (was: %d - valid: [0-%d])\n", index, (int32)getScript()->_scripts.size() - 1);
		return true;
	}

	if (actor < 0 || actor >= (int32)getWorld()->actors.size()) {
		debugPrintf("[Error] Invalid actor index (was: %d - valid: [0-%d])\n", actor, getWorld()->actors.size() - 1);
	}

	getScript()->queueScript(index, actor);

	return false;
}

bool Console::cmdKillScript(int argc, const char **argv) {
	if (argc == 1) {
		debugPrintf("Scripts running:\n");
		for (uint32 i = 0; i < ARRAYSIZE(getScript()->_queue.entries); i++)
			if (getScript()->_queue.entries[i].scriptIndex > 0)
				debugPrintf(" %d\n", getScript()->_queue.entries[i].scriptIndex);

		return true;
	}

	int32 index = atoi(argv[1]);
	for (uint32 i = 0; i < ARRAYSIZE(getScript()->_queue.entries); i++)
		if (getScript()->_queue.entries[i].scriptIndex == index) {
			getScript()->removeFromQueue(i);
			break;
		}

	return false;
}

bool Console::cmdChangeScene(int argc, const char **argv) {
	if (argc != 2) {
		debugPrintf("Syntax: %s <scene number>\n", argv[0]);
		return true;
	}

	ResourcePackId index = (ResourcePackId)atoi(argv[1]);

	// Check if the scene exists
	char filename[20];
	snprintf(filename, 20, "scn.%03d", index);
	if (!SearchMan.hasFile(filename)) {
		debugPrintf("[Error] Scene %d does not exists\n", index);
		return true;
	}

	_vm->_delayedSceneIndex = index;
	_vm->_puzzles->reset();

	return false;
}

bool Console::cmdRunEncounter(int argc, const char **argv) {
	if (argc != 2) {
		debugPrintf("Syntax: %s <encounter index>\n", argv[0]);
		return true;
	}

	// Check that we are inside a scene
	if (!getScene()) {
		debugPrintf("[Error] Cannot run an encounter outside of a scene\n");
		return true;
	}

	// Check index is valid
	int32 index = atoi(argv[1]);
	if (index < 0 || index >= (int32)_vm->encounter()->_items.size()) {
		debugPrintf("[Error] Invalid index (was: %d - valid: [0-%d])\n", index, _vm->encounter()->_items.size() - 1);
		return true;
	}

	// Get the encounter data
	const EncounterData *data;
	for (data = (const EncounterData *)&encounterData[getScene()->getPackId() - 5]; data->index != -1; data++) {
		if (data->index == index)
			break;
	}

	if (data->index == -1) {
		debugPrintf("[Error] No encounter data for this index (index: %d)\n", index);
		return true;
	}

	_vm->encounter()->run(index,
	                      (ObjectId)data->objectId1,
	                      (ObjectId)data->objectId2,
	                      data->actorIndex);

	return false;
}

bool Console::cmdShowEncounter(int argc, const char **argv) {
	if (argc != 2) {
		debugPrintf("Syntax: %s <encounter index>\n", argv[0]);
		return true;
	}

	// Check index is valid
	int32 index = atoi(argv[1]);
	if (index < 0 || index >= (int32)getEncounter()->_items.size()) {
		debugPrintf("[Error] Invalid index (was: %d - valid: [0-%d])\n", index, getEncounter()->_items.size() - 1);
		return true;
	}

	int32 i = 0;
	ResourceId resourceId = getEncounter()->_items[index].scriptResourceId;
	do {
		Encounter::ScriptEntry entry = getEncounter()->getScriptEntry(resourceId, i);

		if (entry.opcode > 25)
			break;
		debugPrintf("%3d %s\n", i, entry.toString().c_str());
	} while (++i);

	return true;
}

bool Console::cmdRunPuzzle(int argc, const char **argv) {
	if (argc != 2) {
		debugPrintf("Syntax: %s <puzzle index>\n", argv[0]);
		debugPrintf("        0   VCR\n");
		debugPrintf("        1   Pipes\n");
		debugPrintf("        2   TicTacToe\n");
		debugPrintf("        3   Lock\n");
		debugPrintf("        4   N/A\n");
		debugPrintf("        5   Wheel\n");
		debugPrintf("        6   BoardSalvation\n");
		debugPrintf("        7   BoardYouth\n");
		debugPrintf("        8   BoardKeyHidesTo\n");
		debugPrintf("        9   Writings\n");
		debugPrintf("        10  Unknown\n");
		debugPrintf("        11  MorgueDoor\n");
		debugPrintf("        12  Clock\n");
		debugPrintf("        13  TimerMachine\n");
		debugPrintf("        14  Fisherman\n");
		debugPrintf("        15  HiveMachine\n");
		debugPrintf("        16  HiveControl\n");
		return true;
	}

	int32 index = atoi(argv[1]);

	// Check index is valid
	if (index < 0 || index >= ARRAYSIZE(puzzleToScenes)) {
		debugPrintf("[Error] Invalid index (was: %d - valid: [0-%d])\n", index, ARRAYSIZE(_vm->_puzzles->_puzzles));
		return true;
	}

	EventHandler *puzzle = getPuzzles()->getPuzzle((uint32)index);
	if (puzzle == NULL) {
		debugPrintf("[Error] This puzzle does not exists (%d)", index);
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

bool Console::cmdListItems(int argc, const char **argv) {
	ChapterIndex chapter = getWorld()->chapter;
	uint32 maxIndex;
	for (maxIndex = 0; maxIndex < 16; maxIndex++) {
		if (!itemIndices[chapter - 1][maxIndex])
			break;
	}

	int32 offset, actorType = getWorld()->actorType;
	if (actorType == kActorMax)
		offset = 83;
	else if (actorType == kActorSarah)
		offset = 586;
	else if (actorType == kActorCyclops)
		offset = 743;
	else if (actorType == kActorAztec)
		offset = 893;
	else
		return true;

	for (uint32 i = 0; i < maxIndex; i++) {
		char *text = getText()->get(MAKE_RESOURCE(kResourcePackText, offset + itemIndices[chapter - 1][i]));
		text += 4;
		debugPrintf("%02d: %s\n", i + 1, text);
	}

	return true;
}

bool Console::cmdAddToInventory(int argc, const char **argv) {
	if (argc != 2 && argc != 3) {
		debugPrintf("Syntax: %s <item index> (<count>)\n", argv[0]);
		return true;
	}

	uint32 index = atoi(argv[1]), count = argc == 3 ? atoi(argv[2]) : 0, maxIndex;

	for (maxIndex = 0; maxIndex < 16; maxIndex++) {
		if (!getWorld()->cursorResourcesAlternate[maxIndex])
			break;
	}

	if (index > maxIndex || !index) {
		debugPrintf("[Error] index should be between 1 and %d\n", maxIndex);
		return true;
	}

	getScene()->getActor()->addReactionHive(index, count);

	return true;
}

bool Console::cmdRemoveFromInventory(int argc, const char **argv) {
	if (argc != 2 && argc != 3) {
		debugPrintf("Syntax: %s <item index> (<count>)\n", argv[0]);
		return true;
	}

	uint32 index = atoi(argv[1]), count = argc == 3 ? atoi(argv[2]) : 0, maxIndex;

	for (maxIndex = 0; maxIndex < 16; maxIndex++) {
		if (!getWorld()->cursorResourcesAlternate[maxIndex])
			break;
	}

	if (index > maxIndex || !index) {
		debugPrintf("[Error] index should be between 1 and %d\n", maxIndex);
		return true;
	}

	getScene()->getActor()->removeReactionHive(index, count);

	return true;
}

bool Console::cmdSetPalette(int argc, const char **argv) {
	if (argc != 3) {
		debugPrintf("Syntax: %s <pack> <index>\n", argv[0]);
		return true;
	}

	int32 pack = atoi(argv[1]);
	int32 index = atoi(argv[2]);

	// Check resource pack
	if (pack < 0 || pack > 18) {
		debugPrintf("[Error] Invalid resource pack (was: %d - valid: [0-18])\n", pack);
		return true;
	}

	// Check index
	if (index < 0) {
		debugPrintf("[Error] Invalid index (was: %d - valid: > 0)\n", index);
		return true;
	}

	// Try loading resource
	ResourceId id = MAKE_RESOURCE((uint32)pack, index);

	ResourceEntry *entry = getResource()->get(id);
	if (!entry) {
		debugPrintf("[Error] Invalid resource (0x%X)\n", id);
		return true;
	}

	getScreen()->setPalette(id);

	return true;
}

bool Console::cmdDrawResource(int argc, const char **argv) {
	if (argc != 3 && argc != 4) {
		debugPrintf("Syntax: %s <pack> <index> (<frame>)\n", argv[0]);
		return true;
	}

	int32 pack = atoi(argv[1]);
	int32 index = atoi(argv[2]);

	int32 frame = 0;
	if (argc == 4)
		frame = atoi(argv[3]);

	// Check resource pack
	if (pack < 0 || pack > 18) {
		debugPrintf("[Error] Invalid resource pack (was: %d - valid: [0-18])\n", pack);
		return true;
	}

	// Check index
	if (index < 0) {
		debugPrintf("[Error] Invalid index (was: %d - valid: > 0)\n", index);
		return true;
	}

	ResourceId resourceId = MAKE_RESOURCE((uint32)pack, index);

	// Try loading resource
	GraphicResource *resource = new GraphicResource(_vm);
	if (!resource->load(resourceId)) {
		debugPrintf("[Error] Invalid resource index (was: %d)\n", index);
		delete resource;
		return true;
	}

	if (frame < 0 || frame >= (int32)resource->count()) {
		debugPrintf("[Error] Invalid resource frame index (was: %d , max: %d)\n", frame, resource->count() - 1);
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
bool Console::cmdToggleFlag(int argc, const char **argv) {
	if (argc != 2 || atoi(argv[1]) > 1512 || atoi(argv[1]) < 0) {
		debugPrintf("Syntax: <value> between 0 and 1512\n");
		return true;
	}

	_vm->toggleGameFlag((GameFlag)atoi(argv[1]));
	debugPrintf("Flag %d == %d\n", atoi(argv[1]), _vm->isGameFlagSet((GameFlag)atoi(argv[1])));

	return true;
}

} // End of namespace Asulym
