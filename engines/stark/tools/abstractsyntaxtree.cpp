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

void ASTBlock::print(uint depth) {
	for (uint i = 0; i < _children.size(); i++) {
		_children[i]->print(depth);
	}
}

ASTCommand::ASTCommand(ASTNode *parent, Command *command) :
		ASTNode(parent),
		Command(command) {

}

void ASTCommand::print(uint depth) {
	printWithDepth(depth, callString());
}

Common::String ASTCommand::callString() {
	return Common::String::format("%s(%s)", _subTypeDesc->name, describeArguments().c_str());
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

void ASTCondition::print(uint depth) {
	Common::String ifHeader = Common::String::format("if (%s%s) {", invertedCondition ? "!" : "", condition->callString().c_str());
	printWithDepth(depth, ifHeader);

	thenBlock->print(depth + 1);

	if (elseBlock) {
		printWithDepth(depth, "} else {");
		elseBlock->print(depth + 1);
	}
	printWithDepth(depth, "}");
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

void ASTLoop::print(uint depth) {
	Common::String loopHeader = Common::String::format("while (%s%s) {", invertedCondition ? "!" : "", condition->callString().c_str());
	printWithDepth(depth, loopHeader);

	loopBlock->print(depth + 1);

	printWithDepth(depth, "}");
}

} // End of namespace Tools
} // End of namespace Stark
