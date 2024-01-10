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

#include "twp/console.h"
#include "twp/twp.h"
#include "twp/squtil.h"

namespace Twp {

Console::Console() : GUI::Debugger() {
	registerCmd("!",   WRAP_METHOD(Console, Cmd_exec));
}

Console::~Console() {
}

bool Console::Cmd_exec(int argc, const char **argv) {
	Common::String s;
	if (argc > 0) {
		s += argv[0];
		for (int i = 1; i < argc; i++) {
			s += ' ';
			s += argv[i];
		}
	}
	sqexec(g_engine->getVm(), s.c_str(), "console");
	return true;
}

} // End of namespace Twp
