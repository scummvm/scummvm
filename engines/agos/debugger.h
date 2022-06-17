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

#ifndef AGOS_DEBUGGER_H
#define AGOS_DEBUGGER_H

#include "gui/debugger.h"

namespace AGOS {

class AGOSEngine;

class Debugger : public GUI::Debugger {
public:
	Debugger(AGOSEngine *vm);

private:
	AGOSEngine *_vm;

	bool Cmd_PlayMusic(int argc, const char **argv);
	bool Cmd_PlaySound(int argc, const char **argv);
	bool Cmd_PlayVoice(int argc, const char **argv);
	bool Cmd_SetBit(int argc, const char **argv);
	bool Cmd_SetBit2(int argc, const char **argv);
	bool Cmd_SetBit3(int argc, const char **argv);
	bool Cmd_SetVar(int argc, const char **argv);
	bool Cmd_SetObjectFlag(int argc, const char **argv);
	bool Cmd_StartSubroutine(int argc, const char **argv);
	bool Cmd_dumpImage(int argc, const char **argv);
	bool Cmd_dumpScript(int argc, const char **argv);
};

} // End of namespace AGOS

#endif
