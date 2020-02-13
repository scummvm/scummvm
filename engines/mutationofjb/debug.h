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

#ifndef MUTATIONOFJB_DEBUG_H
#define MUTATIONOFJB_DEBUG_H

#include "gui/debugger.h"

namespace MutationOfJB {

class MutationOfJBEngine;
class Command;
class Script;

class Console : public GUI::Debugger {
public:
	Console(MutationOfJBEngine *vm);
	~Console(void) override {}
private:
	bool cmd_showallcommands(int argc, const char **argv);
	bool cmd_listsections(int argc, const char **argv);
	bool cmd_showsection(int argc, const char **argv);
	bool cmd_listmacros(int argc, const char **argv);
	bool cmd_showmacro(int argc, const char **argv);
	bool cmd_liststartups(int argc, const char **argv);
	bool cmd_showstartup(int argc, const char **argv);
	bool cmd_changescene(int argc, const char **argv);
	bool cmd_dumpsceneinfo(int argc, const char **argv);
	bool cmd_dumpdoorinfo(int argc, const char **argv);
	bool cmd_dumpobjectinfo(int argc, const char **argv);
	bool cmd_dumpstaticinfo(int argc, const char **argv);
	bool cmd_dumpbitmapinfo(int argc, const char **argv);
	bool cmd_listinventory(int argc, const char **argv);

	void showIndent(int indentLevel);
	void showCommands(Command *command, int indentLevel = 0);
	Script *getScriptFromArg(const char *arg);

	MutationOfJBEngine *_vm;
};

}

#endif
