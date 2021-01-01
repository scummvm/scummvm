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

#ifndef STARTREK_CONSOLE_H
#define STARTREK_CONSOLE_H

#include "gui/debugger.h"

namespace StarTrek {

class StarTrekEngine;

class Console : public GUI::Debugger {
public:
	Console(StarTrekEngine *vm);
	~Console(void) override;

private:
	StarTrekEngine *_vm;

	bool Cmd_Room(int argc, const char **argv);
	bool Cmd_Actions(int argc, const char **argv);
	bool Cmd_Text(int argc, const char **argv);
	bool Cmd_Bg(int argc, const char **argv);
	bool Cmd_DumpFile(int argc, const char **argv);
	bool Cmd_SearchFile(int argc, const char **argv);
	bool Cmd_Score(int argc, const char **argv);
	bool Cmd_BridgeSequence(int argc, const char **argv);

	Common::String EventToString(uint32 action);
	Common::String ItemToString(byte index);
	void dumpFile(Common::String fileName);
};

} // End of namespace StarTrek
#endif
