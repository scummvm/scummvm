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

#ifndef STARK_TOOLS_ABSTRACT_SYNTAX_TREE_H
#define STARK_TOOLS_ABSTRACT_SYNTAX_TREE_H

#include "common/array.h"

#include "engines/stark/tools/command.h"

namespace Stark {
namespace Tools {

/**
 * Base Abstract Syntax Tree node
 *
 * The abstract syntax tree directly maps the script source code.
 */
struct ASTNode {
	ASTNode(ASTNode *parent);
	virtual ~ASTNode();

	/** Print the script source code for this node and its children */
	virtual void print(uint depth) = 0;

protected:
	void printWithDepth(uint depth, const Common::String &string) const;

	ASTNode *_parent;
};

/**
 * Command AST Node
 *
 * Commands are leaf AST nodes. They represent an engine command call.
 */
struct ASTCommand : public ASTNode, public Command {
	ASTCommand(ASTNode *parent, Command *command);

	// ASTNode API
	virtual void print(uint depth) override;

	/** Build a script source code call for this command */
	Common::String callString();
};

/**
 * Block AST Node
 *
 * A list of consecutive script expressions.
 */
struct ASTBlock : public ASTNode {
	ASTBlock(ASTNode *parent);
	~ASTBlock() override;

	// ASTNode API
	virtual void print(uint depth) override;

	/** Append a child expression to this block */
	void addNode(ASTNode *node);

private:
	Common::Array<ASTNode *> _children;
};

/**
 * Condition AST Node
 *
 * An if / then / else branching condition.
 */
struct ASTCondition : public ASTNode {
	ASTCondition(ASTNode *parent);
	~ASTCondition() override;

	// ASTNode API
	virtual void print(uint depth) override;

	ASTCommand *condition;
	bool invertedCondition;
	ASTBlock *thenBlock;
	ASTBlock *elseBlock;
};

/**
 * Loop AST Node
 *
 * A while loop.
 */
struct ASTLoop : public ASTNode {
	ASTLoop(ASTNode *parent);
	~ASTLoop() override;

	// ASTNode API
	virtual void print(uint depth) override;

	ASTCommand *condition;
	bool invertedCondition;
	ASTBlock *loopBlock;
};

} // End of namespace Tools
} // End of namespace Stark

#endif // STARK_TOOLS_ABSTRACT_SYNTAX_TREE_H
