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

#include "pink/console.h"
#include "pink/pink.h"
#include "pink/objects/module.h"
#include "pink/objects/pages/game_page.h"
#include "pink/objects/actors/lead_actor.h"

namespace Pink {

Console::Console(PinkEngine *vm)
	: _vm(vm) {
	registerCmd("listModules", WRAP_METHOD(Console, Cmd_ListModules));
	registerCmd("goToModule", WRAP_METHOD(Console, Cmd_GoToModule));

	registerCmd("listPages", WRAP_METHOD(Console, Cmd_ListPages));
	registerCmd("goToPage", WRAP_METHOD(Console, Cmd_GoToPage));

	registerCmd("listGameVars", WRAP_METHOD(Console, Cmd_ListGameVars));
	registerCmd("setGameVar", WRAP_METHOD(Console, Cmd_SetGameVar));

	registerCmd("listModuleVars", WRAP_METHOD(Console, Cmd_ListModuleVars));
	registerCmd("setModuleVar", WRAP_METHOD(Console, Cmd_SetModuleVar));

	registerCmd("listPageVars", WRAP_METHOD(Console, Cmd_ListPageVars));
	registerCmd("setPageVar", WRAP_METHOD(Console, Cmd_SetPageVar));

	registerCmd("listItems", WRAP_METHOD(Console, Cmd_ListItems));
	registerCmd("addItem", WRAP_METHOD(Console, Cmd_addItem));
}

bool Console::Cmd_ListModules(int argc, const char **argv) {
	const Array<NamedObject *> modules = _vm->_modules;
	for (uint i = 0; i < modules.size(); ++i) {
		debugPrintf("%d.%s\n", i, modules[i]->getName().c_str());
	}
	return true;
}

bool Console::Cmd_GoToModule(int argc, const char **argv) {
	if (argc != 2) {
		debugPrintf("Usage: %s moduleName\n", argv[0]);
		debugPrintf("Module may not work properly because of Game vars\n");
		return true;
	}
	const Array<NamedObject *> modules = _vm->_modules;
	for (uint i = 0; i < modules.size(); ++i) {
		if (modules[i]->getName() == argv[1]) {
			_vm->initModule(argv[1], "", nullptr);
			return true;
		}
	}
	debugPrintf("Module %s doesn't exist\n", argv[1]);
	return true;
}

bool Console::Cmd_ListPages(int argc, const char **argv) {
	const Array<GamePage*> pages = _vm->_module->_pages;
	for (uint i = 0; i < pages.size(); ++i) {
		debugPrintf("%d.%s\n", i, pages[i]->getName().c_str());
	}
	return true;
}

bool Console::Cmd_GoToPage(int argc, const char **argv) {
	if (argc != 2) {
		debugPrintf("Usage: %s pageName\n", argv[0]);
		debugPrintf("Page may not work properly because of vars\n");
		return true;
	}
	const Array<GamePage*> pages = _vm->_module->_pages;
	for (uint i = 0; i < pages.size(); ++i) {
		if (pages[i]->getName() == argv[1]) {
			_vm->setNextExecutors("", pages[i]->getName());
			_vm->changeScene();
			return true;
		}
	}
	debugPrintf("Page %s doesn't exist\n", argv[1]);
	return true;
}


bool Console::Cmd_ListGameVars(int argc, const char **argv) {
	const StringMap &vars = _vm->_variables;
	for (StringMap::const_iterator it = vars.begin(); it != vars.end() ; ++it) {
		debugPrintf("%s %s \n", it->_key.c_str(), it->_value.c_str());
	}
	return true;
}

bool Console::Cmd_SetGameVar(int argc, const char **argv) {
	if (argc != 3) {
		debugPrintf("Usage: %s varName value\n", argv[0]);
		return true;
	}
	_vm->_variables[argv[1]] = argv[2];
	return true;
}

bool Console::Cmd_ListModuleVars(int argc, const char **argv) {
	const StringMap &vars = _vm->_module->_variables;
	for (StringMap::const_iterator it = vars.begin(); it != vars.end() ; ++it) {
		debugPrintf("%s %s \n", it->_key.c_str(), it->_value.c_str());
	}
	return true;
}

bool Console::Cmd_SetModuleVar(int argc, const char **argv) {
	if (argc != 3) {
		debugPrintf("Usage: %s varName value\n", argv[0]);
		return true;
	}
	_vm->_module->_variables[argv[1]] = argv[2];
	return true;
}

bool Console::Cmd_ListPageVars(int argc, const char **argv) {
	const StringMap &vars = _vm->_module->_page->_variables;
	for (StringMap::const_iterator it = vars.begin(); it != vars.end() ; ++it) {
		debugPrintf("%s %s \n", it->_key.c_str(), it->_value.c_str());
	}
	return true;
}

bool Console::Cmd_SetPageVar(int argc, const char **argv) {
	if (argc != 3) {
		debugPrintf("Usage: %s varName value\n", argv[0]);
		return true;
	}
	_vm->_module->_page->_variables[argv[1]] = argv[2];
	return true;
}

bool Console::Cmd_ListItems(int argc, const char **argv) {
	const Common::Array<InventoryItem *> &items = _vm->_module->_invMgr._items;
	for (uint i = 0; i < items.size(); ++i) {
		debugPrintf("%s\n", items[i]->getName().c_str());
	}
	return true;
}

bool Console::Cmd_addItem(int argc, const char **argv) {
	if (argc != 2) {
		debugPrintf("Usage: %s item\n", argv[0]);
		return true;
	}
	InventoryMgr *inv = &_vm->_module->_invMgr;
	LeadActor *actor = _vm->_actor;
	inv->setItemOwner(actor->getName(), inv->findInventoryItem(argv[1]));
	return true;
}

} // End of namespace Pink
