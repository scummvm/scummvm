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

struct ASTCommand;

/**
 * Base Abstract Syntax Tree node
 *
 * The abstract syntax tree directly maps the script source code.
 */
struct ASTNode {
	ASTNode(ASTNode *parent);
	virtual ~ASTNode();

	/** Print the script source code for this node and its children */
	virtual void print(uint depth, DefinitionRegistry *definitions) = 0;

	/** Recursively list all the commands in the tree with the requested index */
	virtual Common::Array<const ASTCommand *> listCommands(uint16 index) const = 0;

	/** Find the successors of a node, either the direct follower or the condition branches */
	void findSuccessors(ASTNode **follower, ASTNode **trueBranch, ASTNode **falseBranch) const;
	virtual void findSuccessorsIntern(const ASTNode *node, ASTNode **follower, ASTNode **trueBranch, ASTNode **falseBranch) const = 0;

	/** Find the first command to be executed when running this job */
	virtual const ASTCommand *getFirstCommand() const = 0;

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
	ASTCommand(ASTNode *parent, Command *command, DefinitionRegistry *definitions);

	// ASTNode API
	void print(uint depth, DefinitionRegistry *definitions) override;
	Common::Array<const ASTCommand *> listCommands(uint16 index) const override;
	void findSuccessorsIntern(const ASTNode *node, ASTNode **follower, ASTNode **trueBranch, ASTNode **falseBranch) const override;
	const ASTCommand *getFirstCommand() const override;

	/** Build a script source code call for this command */
	Common::String callString(DefinitionRegistry *definitions);
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
	void print(uint depth, DefinitionRegistry *definitions) override;
	Common::Array<const ASTCommand *> listCommands(uint16 index) const override;
	void findSuccessorsIntern(const ASTNode *node, ASTNode **follower, ASTNode **trueBranch, ASTNode **falseBranch) const override;
	const ASTCommand *getFirstCommand() const override;

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
	void print(uint depth, DefinitionRegistry *definitions) override;
	Common::Array<const ASTCommand *> listCommands(uint16 index) const override;
	void findSuccessorsIntern(const ASTNode *node, ASTNode **follower, ASTNode **trueBranch, ASTNode **falseBranch) const override;
	const ASTCommand *getFirstCommand() const override;

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
	void print(uint depth, DefinitionRegistry *definitions) override;
	Common::Array<const ASTCommand *> listCommands(uint16 index) const override;
	void findSuccessorsIntern(const ASTNode *node, ASTNode **follower, ASTNode **trueBranch, ASTNode **falseBranch) const override;
	const ASTCommand *getFirstCommand() const override;

	ASTCommand *condition;
	bool invertedCondition;
	ASTBlock *loopBlock;
};

} // End of namespace Tools
} // End of namespace Stark

#endif // STARK_TOOLS_ABSTRACT_SYNTAX_TREE_H
