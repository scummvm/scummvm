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

#include "igor/console.h"
#include "igor/igor.h"

namespace Igor {

	Console::Console() : GUI::Debugger() {
	registerCmd("test",   WRAP_METHOD(Console, Cmd_test));
	registerCmd("fast_mode",     WRAP_METHOD(Console, Cmd_fastMode));
	registerCmd("room",     WRAP_METHOD(Console, Cmd_changePart));
	registerCmd("give", WRAP_METHOD(Console, Cmd_addObjectToInventory));
	registerCmd("action", WRAP_METHOD(Console, Cmd_executeAction));
}

Console::~Console() {
}

bool Console::Cmd_test(int argc, const char **argv) {
	debugPrintf("Test\n");
	return true;
}

bool Console::Cmd_fastMode(int argc, const char **argv) {

	if (argc != 2) {
		debugPrintf("Usage: %s <factor>\n", argv[0]);
		debugPrintf("  1 = original speed (%d ms per tick), higher is faster; Ctrl+F toggles 1/%d\n",
		            kTimerTicksCount * 1000 / kTickDelay, kFastModeFactor);
		debugPrintf("  Current factor: %d\n", g_engine->debugGetFastMode());
		return true;
	}

	const int factor = atoi(argv[1]);
	if (factor < 1 || factor > kFastModeMaxFactor) {
		debugPrintf("Invalid factor '%s' (valid range 1-%d)\n", argv[1], kFastModeMaxFactor);
		return true;
	}

	g_engine->debugSetFastMode(factor);
	debugPrintf("Speed factor %d (%d ms per tick)\n", factor, kTimerTicksCount * 1000 / kTickDelay / factor);
	return true;
}

bool Console::Cmd_changePart(int argc, const char **argv) {
	if (argc != 2) {
		debugPrintf("Usage: %s <state code>\n", argv[0]);
		return true;
	}

	const int state = atoi(argv[1]);
	if (state <= 0 || state > 999) {
		debugPrintf("Invalid state code '%s'\n", argv[1]);
		return true;
	}

	g_engine->debugChangePart(state);
	debugPrintf("Changing to part state %d\n", state);
	return false;
}

bool Console::Cmd_addObjectToInventory(int argc, const char **argv) {
	if (argc != 2) {
		debugPrintf("Usage: %s <object ID>\n", argv[0]);
		return true;
	}

	const int object = atoi(argv[1]);
	if (!g_engine->debugAddObjectToInventory(object)) {
		debugPrintf("Cannot add object %d (valid IDs are 1-36, and it must not already be held)\n", object);
		return true;
	}

	debugPrintf("Added object %d to the inventory\n", object);
	return true;
}

bool Console::Cmd_executeAction(int argc, const char **argv) {
	if (argc != 2) {
		debugPrintf("Usage: %s <action code>\n", argv[0]);
		return true;
	}

	const int action = atoi(argv[1]);
	if (action < 0 || action > 199) {
		debugPrintf("Invalid action code '%s' (valid range 0-199)\n", argv[1]);
		return true;
	}

	g_engine->debugExecuteAction(action);
	debugPrintf("Executed action %d\n", action);
	return false;
}

} // End of namespace Igor
