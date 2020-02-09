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

/*
* Based on the Reverse Engineering work of Christophe Fontanel,
* maintainer of the Dungeon Master Encyclopaedia (http://dmweb.free.fr/)
*/

#ifndef DM_CONSOLE_H
#define DM_CONSOLE_H

#include "gui/debugger.h"

namespace DM {

class DMEngine;

class Console : public GUI::Debugger {
private:
	DMEngine *_vm;

	bool Cmd_godmode(int argc, const char **argv);
	bool Cmd_noclip(int argc, const char **argv);
	bool Cmd_pos(int argc, const char **argv);
	bool Cmd_map(int argc, const char **argv);
	bool Cmd_listItems(int argc, const char **argv);
	bool Cmd_gimme(int argc, const char **argv);

	const char *debugGetDirectionName(int16 dir);

public:
	explicit Console(DM::DMEngine *vm);
	~Console(void) override {}

	bool _debugGodmodeMana;
	bool _debugGodmodeHP;
	bool _debugGodmodeStamina;
	bool _debugNoclip;
};
}

#endif
