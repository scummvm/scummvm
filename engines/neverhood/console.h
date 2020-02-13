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

#ifndef NEVERHOOD_CONSOLE_H
#define NEVERHOOD_CONSOLE_H

#include "gui/debugger.h"

namespace Neverhood {

class NeverhoodEngine;

class Console : public GUI::Debugger {
public:
	Console(NeverhoodEngine *vm);
	~Console(void) override;

private:
	NeverhoodEngine *_vm;

	bool Cmd_Scene(int argc, const char **argv);
	bool Cmd_Surfaces(int argc, const char **argv);
	bool Cmd_Cheat(int argc, const char **argv);
	bool Cmd_Dumpvars(int argc, const char **argv);
	bool Cmd_PlaySound(int argc, const char **argv);
	bool Cmd_CheckResource(int argc, const char **argv);
	bool Cmd_DumpResource(int argc, const char **argv);

};

} // End of namespace Neverhood
#endif
