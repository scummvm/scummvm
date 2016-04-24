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

#include "engines/stark/tools/decompiler.h"

#include "engines/stark/resources/command.h"
#include "engines/stark/tools/block.h"
#include "engines/stark/tools/command.h"

#include "common/debug.h"

namespace Stark {
namespace Tools {

Decompiler::Decompiler(Resources::Script *script) {
	// Convert the script commands to decompiler commands
	Common::Array<Resources::Command *> resourceCommands = script->listChildren<Resources::Command>();
	for (uint i = 0; i < resourceCommands.size(); i++) {
		_commands.push_back(new Command(resourceCommands[i]));
	}
	if (_commands.empty()) {
		return;
	}

	_entryPoint = findEntryPoint();

	linkCommandBranches();
	buildBlocks();
}

void Decompiler::printCommands() const {
	for (uint i = 0; i < _commands.size(); i++) {
		_commands[i]->printCall();
	}
}

void Decompiler::printBlocks() const {
	for (uint i = 0; i < _blocks.size(); i++) {
		_blocks[i]->print();
		debug("- - - -");
	}
}

Decompiler::~Decompiler() {
	for (uint i = 0; i < _commands.size(); i++) {
		delete _commands[i];
	}

	for (uint i = 0; i < _blocks.size(); i++) {
		delete _blocks[i];
	}
}

void Decompiler::linkCommandBranches() {
	for (uint i = 0; i < _commands.size(); i++) {
		_commands[i]->linkBranches(_commands);
	}
}

Command *Decompiler::findEntryPoint() {
	for (uint i = 0; i < _commands.size(); i++) {
		if (_commands[i]->isEntryPoint()) {
			return _commands[i];
		}
	}

	error("Unable to find an entry point");
}

void Decompiler::buildBlocks() {
	Block *entryPointBlock = new Block();
	_blocks.push_back(entryPointBlock);

	buildBlocks(entryPointBlock, _entryPoint);
}

void Decompiler::buildBlocks(Block *block, Command *command) {
	Command *blockCommand = command;
	while (blockCommand) {
		if (blockCommand->getBlock()) {
			block->setFollower(blockCommand->getBlock());
			break;
		}

		if (blockCommand->isBranchTarget() && !block->isEmpty()) {
			Block *follower = buildBranchBlocks(blockCommand);

			block->setFollower(follower);
			break;
		}

		block->appendCommand(blockCommand);

		if (blockCommand->isBranch()) {
			Block *falseBranch = buildBranchBlocks(blockCommand->getFalseBranch());
			Block *trueBranch = buildBranchBlocks(blockCommand->getTrueBranch());

			block->setBranches(trueBranch, falseBranch);
			break;
		}

		blockCommand = blockCommand->getFollower();
	}
}

Block *Decompiler::buildBranchBlocks(Command *command) {
	if (command->getBlock()) {
		// The command already has a block. No need to go through this path again.
		return command->getBlock();
	}

	Block *branchBlock = new Block();
	_blocks.push_back(branchBlock);

	buildBlocks(branchBlock, command);

	return branchBlock;
}

} // End of namespace Tools
} // End of namespace Stark
