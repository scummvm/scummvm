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

#include "engines/stark/tools/abstractsyntaxtree.h"

#include "common/debug.h"

namespace Stark {
namespace Tools {

ASTNode::ASTNode(ASTNode *parent) :
		_parent(parent) {

}

ASTNode::~ASTNode() {

}

void ASTNode::printWithDepth(uint depth, const Common::String &string) const {
	Common::String prefix;
	for (uint i = 0; i < depth; i++) {
		prefix += "\t";
	}

	debug("%s%s", prefix.c_str(), string.c_str());
}

void ASTNode::findSuccessors(ASTNode **follower, ASTNode **trueBranch, ASTNode **falseBranch) const {
	findSuccessorsIntern(this, follower, trueBranch, falseBranch);
}

ASTBlock::ASTBlock(ASTNode *parent) :
		ASTNode(parent) {

}

ASTBlock::~ASTBlock() {
	for (uint i = 0; i < _children.size(); i++) {
		delete _children[i];
	}
}

void ASTBlock::addNode(ASTNode *node) {
	_children.push_back(node);
}

void ASTBlock::print(uint depth, DefinitionRegistry *definitions) {
	for (uint i = 0; i < _children.size(); i++) {
		_children[i]->print(depth, definitions);
	}
}

Common::Array<const ASTCommand *> ASTBlock::listCommands(uint16 index) const {
	Common::Array<const ASTCommand *> list;

	for (uint i = 0; i < _children.size(); i++) {
		list.push_back(_children[i]->listCommands(index));
	}

	return list;
}

void ASTBlock::findSuccessorsIntern(const ASTNode *node, ASTNode **follower, ASTNode **trueBranch, ASTNode **falseBranch) const {
	if (node == this) {
		if (_parent) {
			_parent->findSuccessorsIntern(node, follower, trueBranch, falseBranch);
		}
		return;
	}

	for (uint i = 0; i < _children.size() - 1; i++) {
		if (node == _children[i]) {
			*follower = _children[i+1];
			return;
		}
	}

	if (node == _children.back()) {
		if (_parent) {
			_parent->findSuccessorsIntern(this, follower, trueBranch, falseBranch);
		}
		return;
	}

	error("Unknown node");
}

const ASTCommand *ASTBlock::getFirstCommand() const {
	if (!_children.empty()) {
		return _children[0]->getFirstCommand();
	} else {
		return nullptr;
	}
}

ASTCommand::ASTCommand(ASTNode *parent, Command *command, DefinitionRegistry *definitions) :
		ASTNode(parent),
		Command(command) {
	_arguments = command->getEffectiveArguments();

	for (uint i = 0; i < _arguments.size(); i++) {
		if (_arguments[i].type == Resources::Command::Argument::kTypeResourceReference) {
			definitions->registerReference(_arguments[i].referenceValue);
		}
	}
}

void ASTCommand::print(uint depth, DefinitionRegistry *definitions) {
	printWithDepth(depth, callString(definitions));
}

Common::String ASTCommand::callString(DefinitionRegistry *definitions) {
	return Common::String::format("%s(%s)", _subTypeDesc->name, describeArguments(definitions).c_str());
}

Common::Array<const ASTCommand *> ASTCommand::listCommands(uint16 index) const {
	Common::Array<const ASTCommand *> list;

	if (_index == index) {
		list.push_back(this);
	}

	return list;
}

void ASTCommand::findSuccessorsIntern(const ASTNode *node, ASTNode **follower, ASTNode **trueBranch, ASTNode **falseBranch) const {
	assert(node == this);

	_parent->findSuccessorsIntern(node, follower, trueBranch, falseBranch);
}

const ASTCommand *ASTCommand::getFirstCommand() const {
	return this;
}

ASTCondition::ASTCondition(ASTNode *parent) :
		ASTNode(parent),
		condition(nullptr),
		invertedCondition(false),
		thenBlock(nullptr),
		elseBlock(nullptr) {

}

ASTCondition::~ASTCondition() {
	delete condition;
	delete thenBlock;
	delete elseBlock;
}

void ASTCondition::print(uint depth, DefinitionRegistry *definitions) {
	Common::String ifHeader = Common::String::format("if (%s%s) {", invertedCondition ? "!" : "",
	                                                 condition->callString(definitions).c_str());
	printWithDepth(depth, ifHeader);

	thenBlock->print(depth + 1, definitions);

	if (elseBlock) {
		printWithDepth(depth, "} else {");
		elseBlock->print(depth + 1, definitions);
	}
	printWithDepth(depth, "}");
}

Common::Array<const ASTCommand *> ASTCondition::listCommands(uint16 index) const {
	Common::Array<const ASTCommand *> list;

	list.push_back(condition->listCommands(index));
	list.push_back(thenBlock->listCommands(index));
	if (elseBlock) {
		list.push_back(elseBlock->listCommands(index));
	}

	return list;
}

void ASTCondition::findSuccessorsIntern(const ASTNode *node, ASTNode **follower, ASTNode **trueBranch, ASTNode **falseBranch) const {
	if (node == this) {
		_parent->findSuccessorsIntern(node, follower, trueBranch, falseBranch);
		return;
	}

	if (node == condition) {
		ASTNode *nextNode = nullptr;
		if (!elseBlock) {
			_parent->findSuccessorsIntern(this, &nextNode, nullptr, nullptr);
		}

		if (!invertedCondition) {
			*trueBranch = thenBlock;
			*falseBranch = elseBlock ? elseBlock : nextNode;
		} else {
			*trueBranch = elseBlock ? elseBlock : nextNode;
			*falseBranch = thenBlock;
		}

		return;
	}

	if (node == thenBlock) {
		_parent->findSuccessorsIntern(this, follower, trueBranch, falseBranch);
		return;
	}

	if (node == elseBlock) {
		_parent->findSuccessorsIntern(this, follower, trueBranch, falseBranch);
		return;
	}

	error("Unknown node");
}

const ASTCommand *ASTCondition::getFirstCommand() const {
	return condition->getFirstCommand();
}

ASTLoop::ASTLoop(ASTNode *parent) :
		ASTNode(parent),
		condition(nullptr),
		invertedCondition(false),
		loopBlock(nullptr) {

}

ASTLoop::~ASTLoop() {
	delete condition;
	delete loopBlock;
}

void ASTLoop::print(uint depth, DefinitionRegistry *definitions) {
	Common::String loopHeader;
	if (condition) {
		loopHeader = Common::String::format("while (%s%s) {", invertedCondition ? "!" : "",
		                                    condition->callString(definitions).c_str());
	} else {
		loopHeader = "loop {";
	}
	printWithDepth(depth, loopHeader);

	loopBlock->print(depth + 1, definitions);

	printWithDepth(depth, "}");
}

Common::Array<const ASTCommand *> ASTLoop::listCommands(uint16 index) const {
	Common::Array<const ASTCommand *> list;

	if (condition) {
		list.push_back(condition->listCommands(index));
	}
	list.push_back(loopBlock->listCommands(index));

	return list;
}

void ASTLoop::findSuccessorsIntern(const ASTNode *node, ASTNode **follower, ASTNode **trueBranch, ASTNode **falseBranch) const {
	if (node == this) {
		_parent->findSuccessorsIntern(node, follower, trueBranch, falseBranch);
		return;
	}

	if (node == condition) {
		ASTNode *nextNode = nullptr;
		_parent->findSuccessorsIntern(this, &nextNode, nullptr, nullptr);

		if (!invertedCondition) {
			*trueBranch = loopBlock;
			*falseBranch = nextNode;
		} else {
			*trueBranch = nextNode;
			*falseBranch = loopBlock;
		}

		return;
	}

	if (node == loopBlock) {
		*follower = condition ? (ASTNode *)condition : (ASTNode *)loopBlock;
		return;
	}

	error("Unknown node");
}

const ASTCommand *ASTLoop::getFirstCommand() const {
	return condition ? condition->getFirstCommand() : loopBlock->getFirstCommand();
}

} // End of namespace Tools
} // End of namespace Stark
