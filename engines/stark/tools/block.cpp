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

#include "engines/stark/tools/block.h"

#include "engines/stark/tools/command.h"

#include "common/debug.h"

namespace Stark {
namespace Tools {

Block::Block() :
		_follower(nullptr),
		_trueBranch(nullptr),
		_falseBranch(nullptr),
		_controlStructure(nullptr),
		_infiniteLoopStart(false) {

}

void Block::appendCommand(CFGCommand *command) {
	_commands.push_back(command);
	command->setBlock(this);
}

bool Block::isEmpty() const {
	return _commands.empty();
}

void Block::print() const {
	for (uint i = 0; i < _commands.size(); i++) {
		_commands[i]->printCall();
	}

	if (_controlStructure) {
		switch (_controlStructure->type) {
			case ControlStructure::kTypeIf:
				debug("if%s: %d else: %d next: %d",
				      _controlStructure->invertedCondition ? " not" : "",
				      _controlStructure->thenHead->getFirstCommandIndex(),
				      _controlStructure->elseHead ? _controlStructure->elseHead->getFirstCommandIndex() : -1,
				      _controlStructure->next ? _controlStructure->next->getFirstCommandIndex() : -1);
				break;
			case ControlStructure::kTypeWhile:
				debug("while%s: %d next: %d",
				      _controlStructure->invertedCondition ? " not" : "",
				      _controlStructure->loopHead->getFirstCommandIndex(),
				      _controlStructure->next->getFirstCommandIndex());
				break;
		}
	}

	if (_infiniteLoopStart) {
		debug("infinite loop start: %d", getFirstCommandIndex());
	}

	if (isCondition() && !_controlStructure) {
		debug("unrecognized control flow");
	}
}

void Block::setBranches(Block *trueBranch, Block *falseBranch) {
	_trueBranch = trueBranch;
	_falseBranch = falseBranch;

	trueBranch->addPredecessor(this);
	falseBranch->addPredecessor(this);
}

void Block::setFollower(Block *follower) {
	_follower = follower;
	follower->addPredecessor(this);
}

void Block::addPredecessor(Block *predecessor) {
	_predecessors.push_back(predecessor);
}

bool Block::isCondition() const {
	return _trueBranch != nullptr && _falseBranch != nullptr;
}

bool Block::hasPredecessor(Block *predecessor) const {
	Common::Array<const Block *> visited;
	return hasPredecessorIntern(visited, predecessor);
}

bool Block::hasPredecessorIntern(Common::Array<const Block *> &visited, Block *predecessor) const {
	visited.push_back(this);

	if (isInfiniteLoopStart()) {
		// Don't follow infinite loops
		return false;
	}

	for (uint i = 0; i < _predecessors.size(); i++) {
		if (_predecessors[i] == predecessor) {
			return true;
		}

		bool alreadyVisited = Common::find(visited.begin(), visited.end(), _predecessors[i]) != visited.end();
		if (!alreadyVisited && _predecessors[i]->hasPredecessorIntern(visited, predecessor)) {
			return true;
		}
	}

	return false;
}

bool Block::hasSuccessor(Block *successor) const {
	Common::Array<const Block *> visited;
	return hasSuccessorIntern(visited, successor);
}

bool Block::hasSuccessorIntern(Common::Array<const Block *> &visited, Block *successor) const {
	visited.push_back(this);

	if (this == successor) {
		return true;
	}

	bool followerHasSuccessor = hasChildSuccessorIntern(visited, _follower, successor);
	bool trueBranchHasSuccessor = hasChildSuccessorIntern(visited, _trueBranch, successor);
	bool falseBranchHasSuccessor = hasChildSuccessorIntern(visited, _falseBranch, successor);

	return followerHasSuccessor || trueBranchHasSuccessor || falseBranchHasSuccessor;
}

bool Block::hasChildSuccessorIntern(Common::Array<const Block *> &visited, Block *child, Block *successor) const {
	if (!child) {
		return false;
	}

	bool alreadyVisited = Common::find(visited.begin(), visited.end(), child) != visited.end();
	return !alreadyVisited && child->hasSuccessorIntern(visited, successor);
}

Block *Block::findMergePoint(Block *other) {
	// TODO: Find the closest merge point? How to define that notion?
	Common::Array<const Block *> visited;
	return findMergePointIntern(visited, other);
}

Block *Block::findMergePointIntern(Common::Array<const Block *> &visited, Block *other) {
	visited.push_back(this);

	if (other == this) {
		return this;
	}

	if (hasPredecessor(other)) {
		return this;
	}

	Block *mergePoint = findChildMergePoint(visited, _follower, other);
	if (mergePoint) {
		return mergePoint;
	}

	mergePoint = findChildMergePoint(visited, _trueBranch, other);
	if (mergePoint) {
		return mergePoint;
	}

	mergePoint = findChildMergePoint(visited, _falseBranch, other);
	if (mergePoint) {
		return mergePoint;
	}

	return nullptr;
}

Block *Block::findChildMergePoint(Common::Array<const Block *> &visited, Block *child, Block *other) const {
	if (child) {
		bool alreadyVisited = Common::find(visited.begin(), visited.end(), child) != visited.end();
		if (!alreadyVisited) {
			return child->findMergePointIntern(visited, other);
		}
	}

	return nullptr;
}

bool Block::checkAllBranchesConverge(Block *junction) const {
	// Check if there is at least one path to the junction node
	if (!hasSuccessor(junction)) {
		return false;
	}

	// Check all the successors go to the junction point or loop infinitely
	Common::Array<const Block *> visited;
	return checkAllBranchesConvergeIntern(visited, junction);
}

bool Block::checkAllBranchesConvergeIntern(Common::Array<const Block *> &visited, Block *junction) const {
	visited.push_back(this);

	if (this == junction) {
		return true;
	}

	if (!_follower && !_trueBranch && !_falseBranch) {
		return false;
	}

	if (isInfiniteLoopStart()) {
		// Don't follow infinite loops
		return false;
	}

	bool followerConverges = checkChildConvergeIntern(visited, _follower, junction);
	bool trueBranchConverges = checkChildConvergeIntern(visited, _trueBranch, junction);
	bool falseBranchConverges = checkChildConvergeIntern(visited, _falseBranch, junction);

	return followerConverges && trueBranchConverges && falseBranchConverges;
}

bool Block::checkChildConvergeIntern(Common::Array<const Block *> &visited, Block *child, Block *junction) const {
	if (child) {
		bool alreadyVisited = Common::find(visited.begin(), visited.end(), child) != visited.end();
		if (!alreadyVisited) {
			return child->checkAllBranchesConvergeIntern(visited, junction);
		}
	}

	return true;
}

Block *Block::getFollower() const {
	return _follower;
}

Block *Block::getTrueBranch() const {
	return _trueBranch;
}

Block *Block::getFalseBranch() const {
	return _falseBranch;
}

uint16 Block::getFirstCommandIndex() const {
	return _commands[0]->getIndex();
}

bool Block::hasControlStructure() const {
	return _controlStructure != nullptr;
}

void Block::setControlStructure(ControlStructure *controlStructure) {
	_controlStructure = controlStructure;
}

ControlStructure *Block::getControlStructure() const {
	return _controlStructure;
}

void Block::setInfiniteLoopStart(bool infiniteLoopStart) {
	_infiniteLoopStart = infiniteLoopStart;
}

bool Block::isInfiniteLoopStart() const {
	return _infiniteLoopStart;
}

Common::Array<CFGCommand *> Block::getLinearCommands() const {
	if (!hasControlStructure()) {
		return _commands;
	} else {
		Common::Array<CFGCommand *> commands;

		// The last command in the block is the condition.
		// Exclude it from the linear commands.
		for (uint i = 0; i < _commands.size() - 1; i ++) {
			commands.push_back(_commands[i]);
		}

		return commands;
	}
}

CFGCommand *Block::getConditionCommand() const {
	if (hasControlStructure()) {
		// The condition command is always the last one
		return _commands.back();
	} else {
		return nullptr;
	}
}

bool Block::allowDuplication() const {
	// Allow simple termination blocks to be duplicated in the decompiled output
	bool isScriptEnd = !_follower && !_trueBranch && !_falseBranch;
	bool isShort = _commands.size() < 5;

	return isScriptEnd && isShort;
}

ControlStructure::ControlStructure(ControlStructureType t) :
		type(t),
		condition(nullptr),
		invertedCondition(false),
		loopHead(nullptr),
		thenHead(nullptr),
		elseHead(nullptr),
		next(nullptr) {
}

} // End of namespace Tools
} // End of namespace Stark
