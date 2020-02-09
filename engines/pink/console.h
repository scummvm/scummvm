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

#ifndef PINK_CONSOLE_H
#define PINK_CONSOLE_H

#include "gui/debugger.h"

namespace Pink {

class PinkEngine;

class Console : public GUI::Debugger {
public:
	Console(PinkEngine *vm);

	~Console(void) override {}

private:
	bool Cmd_ListModules(int argc, const char **argv);
	bool Cmd_GoToModule(int argc, const char **argv);

	bool Cmd_ListPages(int argc, const char **argv);
	bool Cmd_GoToPage(int argc, const char **argv);

	bool Cmd_ListGameVars(int argc, const char **argv);
	bool Cmd_SetGameVar(int argc, const char **argv);

	bool Cmd_ListModuleVars(int argc, const char **argv);
	bool Cmd_SetModuleVar(int argc, const char **argv);

	bool Cmd_ListPageVars(int argc, const char **argv);
	bool Cmd_SetPageVar(int argc, const char **argv);

	bool Cmd_ListItems(int argc, const char **argv);
	bool Cmd_addItem(int argc, const char **argv);

private:
	PinkEngine *_vm;
};

} // End of namespace Pink

#endif
