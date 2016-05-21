/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
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

#ifndef STARK_TOOLS_SCRIPT_H
#define STARK_TOOLS_SCRIPT_H

#include "common/str.h"

#include "engines/stark/resources/script.h"

namespace Stark {

namespace Resources {
class Script;
}

namespace Tools {

class Block;
class Command;
struct ControlStructure;

class Decompiler {
public:
	Decompiler(Resources::Script *script);
	~Decompiler();

	void printCommands() const;
	void printBlocks() const;

private:
	void linkCommandBranches();
	Command *findEntryPoint();
	void buildBlocks();
	void analyseControlFlow();

	Common::Array<Command *> _commands;
	Command *_entryPoint;

	Common::Array<Block *> _blocks;
	Common::Array<ControlStructure *> _controlStructures;

	void buildBlocks(Block *block, Command *command);
	Block *buildBranchBlocks(Command *command);

	void detectWhile();
	void detectIf();
};

} // End of namespace Tools
} // End of namespace Stark

#endif // STARK_TOOLS_SCRIPT_H
