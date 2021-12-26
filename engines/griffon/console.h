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

#ifndef GRIFFON_CONSOLE_H
#define GRIFFON_CONSOLE_H

#include "gui/debugger.h"

namespace Griffon {

enum {
	kGodModeNone = 0,
	kGodModeAll = 1,
	kGodModeNoDamage = 2
};

class Console : public GUI::Debugger {
public:
	int _godMode;

	explicit Console();
	~Console(void) override {}

	bool Cmd_godMode(int argc, const char** argv);
	bool Cmd_noDamage(int argc, const char** argv);
};
}

#endif
