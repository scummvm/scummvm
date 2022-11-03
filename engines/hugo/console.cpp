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

#include "hugo/console.h"
#include "hugo/hugo.h"
#include "hugo/object.h"
#include "hugo/parser.h"
#include "hugo/schedule.h"
#include "hugo/text.h"

namespace Hugo {

HugoConsole::HugoConsole(HugoEngine *vm) : GUI::Debugger(), _vm(vm) {
	registerCmd("listscreens",   WRAP_METHOD(HugoConsole, Cmd_listScreens));
	registerCmd("listobjects",   WRAP_METHOD(HugoConsole, Cmd_listObjects));
	registerCmd("getobject",     WRAP_METHOD(HugoConsole, Cmd_getObject));
	registerCmd("getallobjects", WRAP_METHOD(HugoConsole, Cmd_getAllObjects));
	registerCmd("gotoscreen",    WRAP_METHOD(HugoConsole, Cmd_gotoScreen));
	registerCmd("Boundaries",    WRAP_METHOD(HugoConsole, Cmd_boundaries));
}

HugoConsole::~HugoConsole() {
}

static int strToInt(const char *s) {
	if (!*s)
		// No string at all
		return 0;
	else if (toupper(s[strlen(s) - 1]) != 'H')
		// Standard decimal string
		return atoi(s);

	// Hexadecimal string
	uint tmp = 0;
	int read = sscanf(s, "%xh", &tmp);
	if (read < 1)
		error("strToInt failed on string \"%s\"", s);
	return (int)tmp;
}

/**
 * This command loads up the specified screen number
 */
bool HugoConsole::Cmd_gotoScreen(int argc, const char **argv) {
	if ((argc != 2) || (strToInt(argv[1]) > _vm->_numScreens)){
		debugPrintf("Usage: %s <screen number>\n", argv[0]);
		return true;
	}

	_vm->_scheduler->newScreen(strToInt(argv[1]));
	return false;
}

/**
 * This command lists all the screens available
 */
bool HugoConsole::Cmd_listScreens(int argc, const char **argv) {
	if (argc != 1) {
		debugPrintf("Usage: %s\n", argv[0]);
		return true;
	}

	debugPrintf("Available screens for this game are:\n");
	for (int i = 0; i < _vm->_numScreens; i++)
		debugPrintf("%2d - %s\n", i, _vm->_text->getScreenNames(i));
	return true;
}

/**
 * This command lists all the objects available
 */
bool HugoConsole::Cmd_listObjects(int argc, const char **argv) {
	if (argc != 1) {
		debugPrintf("Usage: %s\n", argv[0]);
		return true;
	}

	debugPrintf("Available objects for this game are:\n");
	for (int i = 0; i < _vm->_object->_numObj; i++) {
		if (_vm->_object->_objects[i]._genericCmd & TAKE)
			debugPrintf("%2d - %s\n", i, _vm->_text->getNoun(_vm->_object->_objects[i]._nounIndex, 2));
	}
	return true;
}

/**
 * This command puts an object in the inventory
 */
bool HugoConsole::Cmd_getObject(int argc, const char **argv) {
	if ((argc != 2) || (strToInt(argv[1]) > _vm->_object->_numObj)) {
		debugPrintf("Usage: %s <object number>\n", argv[0]);
		return true;
	}

	if (_vm->_object->_objects[strToInt(argv[1])]._genericCmd & TAKE)
		_vm->_parser->takeObject(&_vm->_object->_objects[strToInt(argv[1])]);
	else
		debugPrintf("Object not available\n");

	return true;
}

/**
 * This command puts all the available objects in the inventory
 */
bool HugoConsole::Cmd_getAllObjects(int argc, const char **argv) {
	if (argc != 1) {
		debugPrintf("Usage: %s\n", argv[0]);
		return true;
	}

	for (int i = 0; i < _vm->_object->_numObj; i++) {
		if (_vm->_object->_objects[i]._genericCmd & TAKE)
			_vm->_parser->takeObject(&_vm->_object->_objects[i]);
	}

	return false;
}

/**
 * This command shows and hides boundaries
 */
bool HugoConsole::Cmd_boundaries(int argc, const char **argv) {
	if (argc != 1) {
		debugPrintf("Usage: %s\n", argv[0]);
		return true;
	}

	_vm->getGameStatus()._showBoundariesFl = !_vm->getGameStatus()._showBoundariesFl;
	return false;
}

} // End of namespace Hugo
