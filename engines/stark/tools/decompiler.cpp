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
		_commands.push_back(new CFGCommand(resourceCommands[i]));
	}
	if (_commands.empty()) {
		return;
	}
	if (!checkCommands()) {
		return;
	}

	_entryPoint = findEntryPoint();

	linkCommandBranches();
	buildBlocks();
	analyseControlFlow();
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

	for (uint i = 0; i < _controlStructures.size(); i++) {
		delete _controlStructures[i];
	}
}

void Decompiler::linkCommandBranches() {
	for (uint i = 0; i < _commands.size(); i++) {
		_commands[i]->linkBranches(_commands);
	}
}

CFGCommand *Decompiler::findEntryPoint() {
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

void Decompiler::buildBlocks(Block *block, CFGCommand *command) {
	CFGCommand *blockCommand = command;
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

Block *Decompiler::buildBranchBlocks(CFGCommand *command) {
	if (command->getBlock()) {
		// The command already has a block. No need to go through this path again.
		return command->getBlock();
	}

	Block *branchBlock = new Block();
	_blocks.push_back(branchBlock);

	buildBlocks(branchBlock, command);

	return branchBlock;
}

void Decompiler::analyseControlFlow() {
	detectWhile();
	detectIf();
}

void Decompiler::detectWhile() {
	for (uint i = 0; i < _blocks.size(); i++) {
		Block *block = _blocks[i];

		if (block->hasControlStructure()) continue;
		if (!block->isCondition()) continue;

		// Check all paths from the body branch go back to the condition
		// TODO: If the original had "break" statement, this will not work
		bool trueBranchConvergesToCondition = block->getTrueBranch()->checkAllBranchesConverge(block);
		bool falseBranchConvergesToCondition = block->getFalseBranch()->checkAllBranchesConverge(block);

		if (!trueBranchConvergesToCondition && !falseBranchConvergesToCondition) continue;
		if (trueBranchConvergesToCondition && falseBranchConvergesToCondition) {
			warning("Both branches of a condition converge back to the condition");
		}

		ControlStructure *controlStructure = new ControlStructure(ControlStructure::kTypeWhile);
		if (trueBranchConvergesToCondition) {
			controlStructure->invertedCondition = false;
			controlStructure->loopHead = block->getTrueBranch();
			controlStructure->next = block->getFalseBranch();
		} else {
			controlStructure->invertedCondition = true;
			controlStructure->loopHead = block->getFalseBranch();
			controlStructure->next = block->getTrueBranch();
		}

		block->setControlStructure(controlStructure);
		_controlStructures.push_back(controlStructure);
	}
}

void Decompiler::detectIf() {
	for (uint i = 0; i < _blocks.size(); i++) {
		Block *block = _blocks[i];

		if (block->hasControlStructure()) continue;
		if (!block->isCondition()) continue;

		ControlStructure *controlStructure = new ControlStructure(ControlStructure::kTypeIf);
		controlStructure->next = block->getTrueBranch()->findMergePoint(block->getFalseBranch());

		if (!controlStructure->next) {
			// When one (or both) of the branches return, there is no merge point
			controlStructure->invertedCondition = false;
			controlStructure->thenHead = block->getTrueBranch();
			controlStructure->elseHead = block->getFalseBranch();
		} else if (block->getTrueBranch() != controlStructure->next) {
			// Use the "true" branch as the "then" block ...
			controlStructure->invertedCondition = false;
			controlStructure->thenHead = block->getTrueBranch();
			controlStructure->elseHead = controlStructure->next != block->getFalseBranch() ? block->getFalseBranch() : nullptr;
		} else {
			// ... unless the true branch is empty.
			// In which case use the false branch and invert the condition.
			controlStructure->invertedCondition = true;
			controlStructure->thenHead = block->getFalseBranch();
			controlStructure->elseHead = nullptr;
		}

		block->setControlStructure(controlStructure);
		_controlStructures.push_back(controlStructure);
	}
}

bool Decompiler::checkCommands() {
	for (uint i = 0; i < _commands.size(); i++) {
		Command *command = _commands[i];
		if (!command->hasSubtypeDescription()) {
			_error = Common::String::format("Command subtype %d is not described", command->getSubType());
			return false;
		}
	}

	return true;
}

Common::String Decompiler::getError() const {
	return _error;
}

} // End of namespace Tools
} // End of namespace Stark
