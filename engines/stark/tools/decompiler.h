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
class CFGCommand;
class DefinitionRegistry;
struct ControlStructure;
struct ASTBlock;
struct ASTNode;
struct ASTCondition;
struct ASTLoop;

class Decompiler {
public:
	Decompiler(Resources::Script *script);
	~Decompiler();

	void printCommands() const;
	void printBlocks() const;
	void printDecompiled();

	Common::String getError() const;

private:
	// Command control flow graph construction
	bool checkCommands();
	void linkCommandBranches();
	CFGCommand *findEntryPoint();

	// Block control flow graph construction
	void buildBlocks();
	void buildBlocks(Block *block, CFGCommand *command);
	Block *buildBranchBlocks(CFGCommand *command);

	// Control flow analysis
	void analyseControlFlow();
	void detectInfiniteLoop();
	void detectWhile();
	void detectIf();

	// AST generation
	ASTNode *buildAST();
	void buildASTFromBlock(ASTBlock *parent, Block *block, Block *stopBlock);
	ASTCondition *buildASTConditionFromBlock(ASTNode *parent, Block *block);
	ASTLoop *buildASTLoopFromBlock(ASTNode *parent, Block *block);

	// AST verification
	void verifyAST();
	bool verifyCommandInAST(CFGCommand *cfgCommand);
	bool verifyCommandSuccessorInAST(CFGCommand *cfgCommand, CFGCommand *cfgSuccessor, ASTNode *astSuccessor, const char *successorType);

	Common::String _error;

	Common::Array<CFGCommand *> _commands;
	CFGCommand *_entryPoint;

	Common::Array<Block *> _blocks;
	Common::Array<ControlStructure *> _controlStructures;

	ASTNode *_astHead;
	Common::Array<Block *> _visitedInfiniteLoopStarts;
	Common::Array<Block *> _visitedBlocks;

	DefinitionRegistry *_definitionRegistry;
};

} // End of namespace Tools
} // End of namespace Stark

#endif // STARK_TOOLS_SCRIPT_H
