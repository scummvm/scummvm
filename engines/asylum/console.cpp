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

#include "asylum/console.h"

#include "asylum/resources/actionarea.h"

#include "asylum/views/scene.h"

#include "asylum/asylum.h"

#include "common/debug-channels.h"

namespace Asylum {

extern int32 g_debugPolygons;
extern int32 g_debugBarriers;
extern int32 g_debugScrolling;

Console::Console(AsylumEngine *engine) : _vm(engine) {
	// Commands
	DCmd_Register("help",           WRAP_METHOD(Console, cmdHelp));

	DCmd_Register("ls",             WRAP_METHOD(Console, cmdListFiles));

	DCmd_Register("actions",        WRAP_METHOD(Console, cmdListActions));
	DCmd_Register("actors",         WRAP_METHOD(Console, cmdListActors));
	DCmd_Register("flags",          WRAP_METHOD(Console, cmdListFlags));
	DCmd_Register("objects",        WRAP_METHOD(Console, cmdListObjects));

	DCmd_Register("video",          WRAP_METHOD(Console, cmdPlayVideo));
	DCmd_Register("script",         WRAP_METHOD(Console, cmdRunScript));
	DCmd_Register("scene",          WRAP_METHOD(Console, cmdChangeScene));

	DCmd_Register("toggle_flag",    WRAP_METHOD(Console, cmdToggleFlag));

	// Variables
	DVar_Register("show_polygons",  &g_debugPolygons, DVAR_INT, 0);
	DVar_Register("show_objects",   &g_debugBarriers, DVAR_INT, 0);
	DVar_Register("use_scrolling",  &g_debugScrolling, DVAR_INT, 0);
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
	DebugPrintf(" show_polygons     - Show polygons\n");
	DebugPrintf(" show_barriers     - Show objects\n");
	DebugPrintf(" use_scrolling     - Use scrolling\n");
	DebugPrintf("\n");
	DebugPrintf("Commands\n");
	DebugPrintf("--------\n");
	DebugPrintf(" ls      - list engine files\n");
	DebugPrintf("\n");
	DebugPrintf(" actors  - show actors information\n");
	DebugPrintf(" action  - show action information\n");
	DebugPrintf(" flags   - show flags\n");
	DebugPrintf(" objects - show objects information\n");
	DebugPrintf("\n");
	DebugPrintf(" video   - play a video\n");
	DebugPrintf(" script  - run a script\n");
	DebugPrintf(" scene   - change the scene\n");
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

		if (index < 0 || index > maxIndex) {
			DebugPrintf("[error] index should be between 0 and %d\n", maxIndex);
			return true;
		}

		DebugPrintf("%s\n", getWorld()->actions[index]->toString().c_str());
	}

	return true;
}

bool Console::cmdListActors(int32 argc, const char **argv) {
	if (argc != 1 && argc != 2) {
		DebugPrintf("Syntax: %s <index> (use nothing for all)\n", argv[0]);
		return true;
	}

	if (argc == 1) {
		for (uint32 i = 0; i < getWorld()->actors.size(); i++)
			DebugPrintf("%s\n", getWorld()->actors[i]->toString().c_str());

	} else {
		int index = atoi(argv[1]);
		int maxIndex = getWorld()->actors.size() - 1;

		if (index < 0 || index > maxIndex) {
			DebugPrintf("[error] index should be between 0 and %d\n", maxIndex);
			return true;
		}

		DebugPrintf("%s\n", getWorld()->actors[index]->toString().c_str());
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
		bool type = atoi(argv[1]);

		if (type != 0 && type != 1) {
			DebugPrintf("Syntax: %s <type> (nothing: all  -  1: show set flags  -  0: show unset flags)\n", argv[0]);
			return true;
		}

		// Show only set/unset flags
		int count = 0;
		for (int32 i = 0; i < 1512; i++) {
			if (_vm->isGameFlagSet((GameFlag)i) == type) {
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

bool Console::cmdListObjects(int32 argc, const char **argv) {
	if (argc != 1 && argc != 2) {
		DebugPrintf("Syntax: %s <index> (use nothing for all)\n", argv[0]);
		return true;
	}

	if (argc == 1) {
		for (uint32 i = 0; i < getWorld()->barriers.size(); i++)
			DebugPrintf("%s\n", getWorld()->barriers[i]->toString().c_str());

	} else {
		int index = atoi(argv[1]);
		int maxIndex = getWorld()->barriers.size() - 1;

		if (index < 0 || index > maxIndex) {
			DebugPrintf("[error] index should be between 0 and %d\n", maxIndex);
			return true;
		}

		DebugPrintf("%s\n", getWorld()->barriers[index]->toString().c_str());
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

	getScreen()->clearScreen();
	getScene()->actions()->setDelayedVideoIndex(index);

	return false;
}

bool Console::cmdRunScript(int32 argc, const char **argv) {
	if (argc != 2) {
		DebugPrintf("Syntax: %s <script number>\n", argv[0]);
		return true;
	}
	// FIXME push the script index into the script queue
	//_vm->scene()->actions()->setScriptByIndex(atoi(argv[1]));

	return false;
}

bool Console::cmdChangeScene(int32 argc, const char **argv) {
	if (argc != 2) {
		DebugPrintf("Syntax: %s <scene number>\n", argv[0]);
		return true;
	}

	if (atoi(argv[1]) - 4 < 1 || atoi(argv[1]) - 4 >= 15) {
		DebugPrintf("[Error] Invalid world: %d\n", atoi(argv[1]));
		return true;
	}

	_vm->scene()->actions()->setDelayedSceneIndex(atoi(argv[1]));
	// FIXME push the script index into the script queue
	// XXX is this right or should it be ws->actionListIdx???
	//_vm->scene()->actions()->setScriptByIndex(0);

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
