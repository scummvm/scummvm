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

#ifndef STARK_TOOLS_BLOCK_H
#define STARK_TOOLS_BLOCK_H

#include "common/array.h"

namespace Stark {
namespace Tools {

class CFGCommand;
struct ControlStructure;

/**
 * An aggregate of script commands
 *
 * Commands in the same group are always executed in sequence.
 *
 * This class is a node in the disassembly node graph.
 */
class Block {
public:
	Block();

	/** Add a command at the end of the block, and set it as the command's block */
	void appendCommand(CFGCommand *command);

	/** Get the commands from the block that are not part of a condition */
	Common::Array<CFGCommand *> getLinearCommands() const;

	/** Get the command from the block that plays as a condition in a control structure */
	CFGCommand *getConditionCommand() const;

	/** Does the block contain commands? */
	bool isEmpty() const;

	/** Can the block branch? */
	bool isCondition() const;

	/**
	 * Blocks are linked together in the block graph with these relationships:
	 * - follower: The natural follower of the block. Used when the block is not a branch, nor an end point.
	 * - true branch: The next block when the block's condition evaluates to true.
	 * - false branch: The next block when the block's condition evaluates to false.
	 * - predecessors: A list of blocks whose execution can lead to this block.
	 */
	void setBranches(Block *trueBranch, Block *falseBranch);
	void setFollower(Block *follower);
	void addPredecessor(Block *predecessor);
	Block *getTrueBranch() const;
	Block *getFalseBranch() const;
	Block *getFollower() const;

	/**
	 * Print a list of this block's commands to the debug output
	 */
	void print() const;

	/**
	 * The high level control structure this block has the main role in
	 */
	bool hasControlStructure() const;
	ControlStructure *getControlStructure() const;
	void setControlStructure(ControlStructure *controlStructure);

	/** Flag to indicate this block is the first in an unconditional infinite loop */
	bool isInfiniteLoopStart() const;
	void setInfiniteLoopStart(bool infiniteLoopStart);

	/** Can this block appear multiple times in the decompiled output? */
	bool allowDuplication() const;

	// Graph query methods
	bool hasPredecessor(Block *predecessor) const;
	bool hasSuccessor(Block *successor) const;
	Block *findMergePoint(Block *other);
	bool checkAllBranchesConverge(Block *junction) const;

private:
	bool hasPredecessorIntern(Common::Array<const Block *> &visited, Block *predecessor) const;
	bool hasSuccessorIntern(Common::Array<const Block *> &visited, Block *successor) const;
	bool hasChildSuccessorIntern(Common::Array<const Block *> &visited, Block *child, Block *successor) const;
	Block *findMergePointIntern(Common::Array<const Block *> &visited, Block *other);
	Block *findChildMergePoint(Common::Array<const Block *> &visited, Block *child, Block *other) const;
	bool checkAllBranchesConvergeIntern(Common::Array<const Block *> &visited, Block *junction) const;
	bool checkChildConvergeIntern(Common::Array<const Block *> &visited, Block *child, Block *junction) const;

	uint16 getFirstCommandIndex() const;

	Common::Array<CFGCommand *> _commands;

	Block *_follower;
	Block *_trueBranch;
	Block *_falseBranch;
	Common::Array<Block *> _predecessors;

	ControlStructure *_controlStructure;
	bool _infiniteLoopStart;
};

struct ControlStructure {
	enum ControlStructureType {
		kTypeIf,
		kTypeWhile
	};

	ControlStructureType type;
	Block *condition;
	bool invertedCondition;
	Block *loopHead;
	Block *thenHead;
	Block *elseHead;
	Block *next;

	ControlStructure(ControlStructureType t);
};

} // End of namespace Tools
} // End of namespace Stark

#endif // STARK_TOOLS_BLOCK_H
