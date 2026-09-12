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

#ifndef HOLLYWOOD_CONSOLE_H
#define HOLLYWOOD_CONSOLE_H

#include "gui/debugger.h"

namespace Hollywood {

class HollywoodEngine;

class Console : public GUI::Debugger {
public:
	Console(HollywoodEngine *vm);
	~Console() override {}

	bool cmdGet(int argc, const char **argv);
	bool cmdSolve(int argc, const char **argv);

private:
	bool parseItemId(const char *argument, uint &itemId);
	byte inventoryOwner() const;

	HollywoodEngine *_vm;
};

} // End of namespace Hollywood

#endif // HOLLYWOOD_CONSOLE_H
