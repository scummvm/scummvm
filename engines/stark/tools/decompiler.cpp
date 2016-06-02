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

#include "engines/stark/tools/abstractsyntaxtree.h"
#include "engines/stark/tools/block.h"

#include "common/debug.h"

namespace Stark {
namespace Tools {

Decompiler::Decompiler(Resources::Script *script) :
		_entryPoint(nullptr),
		_astHead(nullptr),
		_definitionRegistry(nullptr) {
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

	_definitionRegistry = new DefinitionRegistry();
	_astHead = buildAST();
	verifyAST();
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

void Decompiler::printDecompiled() {
	if (_astHead) {
		_definitionRegistry->printAll();
		debug(" "); // Empty line
		_astHead->print(0, _definitionRegistry);
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

	delete _astHead;
	delete _definitionRegistry;
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
	detectInfiniteLoop();
	detectWhile();
	detectIf();
}

void Decompiler::detectInfiniteLoop() {
	for (uint i = 0; i < _blocks.size(); i++) {
		Block *block = _blocks[i];

		// Check all paths from the block go back to itself
		if (block->getFollower()) {
			bool followerConvergesBack = block->getFollower()->checkAllBranchesConverge(block);

			if (followerConvergesBack) {
				block->setInfiniteLoopStart(true);
			}
		} else if (block->isCondition()) {
			bool trueBranchConvergesBack = block->getTrueBranch()->checkAllBranchesConverge(block);
			bool falseBranchConvergesBack = block->getFalseBranch()->checkAllBranchesConverge(block);

			if (trueBranchConvergesBack && falseBranchConvergesBack) {
				block->setInfiniteLoopStart(true);
			}
		}
	}
}

void Decompiler::detectWhile() {
	for (uint i = 0; i < _blocks.size(); i++) {
		Block *block = _blocks[i];

		if (block->hasControlStructure()) continue;
		if (!block->isCondition()) continue;
		if (block->isInfiniteLoopStart()) continue; // TODO: This should be handled by checkAllBranchesConverge already

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

ASTNode *Decompiler::buildAST() {
	Block *entryPoint = _entryPoint->getBlock();

	ASTBlock *head = new ASTBlock(nullptr);
	buildASTFromBlock(head, entryPoint, nullptr);
	return head;
}

void Decompiler::buildASTFromBlock(ASTBlock *parent, Block *block, Block *stopBlock) {
	if (block->isInfiniteLoopStart()) {
		bool alreadyVisited = Common::find(_visitedInfiniteLoopStarts.begin(), _visitedInfiniteLoopStarts.end(), block)
		                      != _visitedInfiniteLoopStarts.end();
		if (alreadyVisited) {
			// Don't add the same loop multiple times in the AST
			return;
		}

		_visitedInfiniteLoopStarts.push_back(block);

		ASTLoop *loop = new ASTLoop(parent);
		loop->loopBlock = new ASTBlock(loop);
		parent->addNode(loop);

		// Hijack the parameters to add the current block to the loop body and continue
		parent = loop->loopBlock;
		stopBlock = block;
	}

	{
		bool alreadyVisited = Common::find(_visitedBlocks.begin(), _visitedBlocks.end(), block) != _visitedBlocks.end();
		if (alreadyVisited && !block->allowDuplication()) {
			// FIXME: We just return for now when an already visited block is visited again.
			// Obviously, this leads to invalid decompiled code, which is caught by the verification step.
			// To fix, either handle the cases leading to multiple visits, or generate gotos.
			return;
		}
	}

	_visitedBlocks.push_back(block);

	Common::Array<CFGCommand *> commands = block->getLinearCommands();
	for (uint i = 0; i < commands.size(); i++) {
		parent->addNode(new ASTCommand(parent, commands[i], _definitionRegistry));
	}

	if (block->hasControlStructure()) {
		ControlStructure *cfgControlStructure = block->getControlStructure();
		ASTNode *astControlStructure;
		switch (cfgControlStructure->type) {
			case ControlStructure::kTypeIf:
				astControlStructure = buildASTConditionFromBlock(parent, block);
				break;
			case ControlStructure::kTypeWhile:
				astControlStructure = buildASTLoopFromBlock(parent, block);
				break;
			default:
				error("Unknown control structure type %d", cfgControlStructure->type);
		}

		parent->addNode(astControlStructure);

		if (cfgControlStructure->next && cfgControlStructure->next != stopBlock) {
			buildASTFromBlock(parent, cfgControlStructure->next, stopBlock);
		}
	} else {
		Block *follower = block->getFollower();
		if (follower && follower != stopBlock) {
			buildASTFromBlock(parent, follower, stopBlock);
		}
	}
}

ASTCondition *Decompiler::buildASTConditionFromBlock(ASTNode *parent, Block *block) {
	ControlStructure *controlStructure = block->getControlStructure();

	ASTCondition *condition = new ASTCondition(parent);
	condition->condition = new ASTCommand(condition, block->getConditionCommand(), _definitionRegistry);
	condition->invertedCondition = controlStructure->invertedCondition;

	condition->thenBlock = new ASTBlock(condition);
	buildASTFromBlock(condition->thenBlock, controlStructure->thenHead, controlStructure->next);

	if (controlStructure->elseHead) {
		condition->elseBlock = new ASTBlock(condition);
		buildASTFromBlock(condition->elseBlock, controlStructure->elseHead, controlStructure->next);
	}

	return condition;
}

ASTLoop *Decompiler::buildASTLoopFromBlock(ASTNode *parent, Block *block) {
	ControlStructure *controlStructure = block->getControlStructure();

	ASTLoop *loop = new ASTLoop(parent);
	loop->condition = new ASTCommand(loop, block->getConditionCommand(), _definitionRegistry);
	loop->invertedCondition = controlStructure->invertedCondition;

	loop->loopBlock = new ASTBlock(loop);
	buildASTFromBlock(loop->loopBlock, controlStructure->loopHead, block);

	return loop;
}

void Decompiler::verifyAST() {
	for (uint i = 0; i < _commands.size(); i++) {
		CFGCommand *command = _commands[i];
		if (!verifyCommandInAST(command)) {
			return;
		}
	}
}

bool Decompiler::verifyCommandInAST(CFGCommand *cfgCommand) {
	Common::Array<const ASTCommand *> list = _astHead->listCommands(cfgCommand->getIndex());

	if (list.empty()) {
		_error = Common::String::format("Command %d not found in the AST", cfgCommand->getIndex());
		return false;
	}

	if (list.size() > 1 && !cfgCommand->getBlock()->allowDuplication()) {
		_error = Common::String::format("Command %d found %d times in the AST", cfgCommand->getIndex(), list.size());
		return false;
	}

	const ASTCommand *astCommand = list[0];

	ASTNode *follower = nullptr, *trueBranch = nullptr, *falseBranch = nullptr;
	astCommand->findSuccessors(&follower, &trueBranch, &falseBranch);

	if (!verifyCommandSuccessorInAST(cfgCommand, cfgCommand->getFollower(), follower, "follower")) {
		return false;
	}

	if (!verifyCommandSuccessorInAST(cfgCommand, cfgCommand->getTrueBranch(), trueBranch, "trueBranch")) {
		return false;
	}

	if (!verifyCommandSuccessorInAST(cfgCommand, cfgCommand->getFalseBranch(), falseBranch, "falseBranch")) {
		return false;
	}

	return true;
}

bool Decompiler::verifyCommandSuccessorInAST(CFGCommand *cfgCommand, CFGCommand *cfgSuccessor, ASTNode *astSuccessor, const char *successorType) {
	if (cfgSuccessor) {
		if (!astSuccessor) {
			_error = Common::String::format("Command %d does not have a %s in the AST",
			                                cfgCommand->getIndex(), successorType);
			return false;
		}

		const ASTCommand *astSuccessorCommand = astSuccessor->getFirstCommand();
		if (!astSuccessorCommand) {
			_error = Common::String::format("Command %d has an empty %s in the AST",
			                                cfgCommand->getIndex(), successorType);
			return false;
		}

		int16 expectedSuccessorIndex = cfgSuccessor->getIndex();
		if (astSuccessorCommand->getIndex() != expectedSuccessorIndex) {
			_error = Common::String::format("Command %d has an unexpected %s %d in the AST, should be %d",
			                                cfgCommand->getIndex(), successorType, astSuccessorCommand->getIndex(), expectedSuccessorIndex);
			return false;
		}
	}

	return true;
}

} // End of namespace Tools
} // End of namespace Stark
