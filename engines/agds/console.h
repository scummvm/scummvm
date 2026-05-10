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

#ifndef CONSOLE_H
#define CONSOLE_H

#include "gui/debugger.h"

namespace AGDS {

class AGDSEngine;

class Console : public GUI::Debugger {
public:
	Console(AGDSEngine *engine);
	using GUI::Debugger::clearVars;
	using GUI::Debugger::registerVar;

private:
	bool load(int argc, const char **argv);
	bool run(int argc, const char **argv);
	bool stop(int argc, const char **argv);
	bool activate(int argc, const char **argv);
	bool info(int argc, const char **argv);
	bool setGlobal(int argc, const char **argv);
	bool inventoryAdd(int argc, const char **argv);
	bool patch(int argc, const char **argv);

	AGDSEngine *_engine;
};

} // namespace AGDS

#endif
