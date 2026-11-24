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

#ifndef DUNE_CONSOLE_H
#define DUNE_CONSOLE_H

#include "gui/debugger.h"

namespace Dune {

class DuneEngine;

class Console : public GUI::Debugger {
public:
	explicit Console(DuneEngine *vm);

private:
	bool cmdPlace(int argc, const char **argv);
	bool cmdStage(int argc, const char **argv);
	bool cmdPart(int argc, const char **argv);
	bool cmdCondition(int argc, const char **argv);
	bool cmdState(int argc, const char **argv);
	bool cmdDump(int argc, const char **argv);
	bool cmdBackground(int argc, const char **argv);
	bool cmdWorld(int argc, const char **argv);
	bool cmdPlaces(int argc, const char **argv);
	bool cmdTroops(int argc, const char **argv);
	bool cmdSave(int argc, const char **argv);
	bool cmdLoad(int argc, const char **argv);

	DuneEngine *_vm;
};

} // End of namespace Dune

#endif
