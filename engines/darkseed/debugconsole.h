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

#ifndef DARKSEED_DEBUGCONSOLE_H
#define DARKSEED_DEBUGCONSOLE_H

#include "gui/debugger.h"
#include "darkseed/tostext.h"

namespace Darkseed {

class DebugConsole : public GUI::Debugger {
private:
	TosText *_tosText;
private:
	bool Cmd_tostext(int argc, const char **argv);
	bool Cmd_dt(int argc, const char **argv);
	bool Cmd_getvar(int argc, const char **argv);
	bool Cmd_setvar(int argc, const char **argv);
	bool Cmd_enablePathfinderOverlay(int argc, const char **argv);
	bool Cmd_info(int argc, const char **argv);
	bool Cmd_gotoRoom(int argc, const char **argv);
	bool Cmd_invAdd(int argc, const char **argv);
	bool Cmd_invRemove(int argc, const char **argv);
	bool Cmd_changeDay(int argc, const char **argv);
	bool validateObjVarIndex(int16 varIdx);
	void printDayAndTime();
public:
	DebugConsole(TosText *tostext);
	~DebugConsole() override;
};

} // End of namespace Darkseed

#endif // DARKSEED_DEBUGCONSOLE_H
