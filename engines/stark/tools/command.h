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

#ifndef STARK_TOOLS_COMMAND_H
#define STARK_TOOLS_COMMAND_H

#include "common/array.h"

#include "engines/stark/resources/command.h"

namespace Stark {
namespace Tools {

class Block;

/**
 * A script command for disassembly use
 *
 * As opposed to the Command class in the Resources namespace, this class
 * is not meant to be executed. It is meant to be used for script disassembly,
 * to store analysis results.
 * It contains informations relative to the control flow.
 *
 * This class is a node in the disassembly command graph.
 */
class Command {
public:
	enum ControlFlowType {
		kFlowNormal,
		kFlowBranch,
		kFlowEnd
	};

	Command(Resources::Command *resource);

	/** Is this command an entry point for the whole script? */
	bool isEntryPoint() const;

	/** Can this command influence the control flow? */
	bool isBranch() const;

	/** Is this command a jump target? */
	bool isBranchTarget() const;

	/**
	 * Commands are linked together in the command graph with these relationships:
	 * - follower: The natural follower of the command. Used when the command is not a branch, nor an end point.
	 * - true branch: The next command when the command's condition evaluates to true.
	 * - false branch: The next command when the command's condition evaluates to false.
	 * - predecessors: A list of commands whose execution can lead to this command.
	 */
	Command *getFollower() const;
	Command *getTrueBranch() const;
	Command *getFalseBranch() const;

	/**
	 * Commands are aggregated into blocks
	 */
	Block *getBlock() const;
	void setBlock(Block *block);

	/**
	 * Add the command to the command graph
	 *
	 * This sets the graph edges concerning this command.
	 */
	void linkBranches(const Common::Array<Command *> &commands);

	/**
	 * Print a call to this command to the debug output
	 */
	void printCall() const;

protected:
	struct SubTypeDesc {
		Resources::Command::SubType subType;
		const char *name;
		ControlFlowType controlFlowType;
	};

	/** Get a description for a command subtype from an internal database */
	static const SubTypeDesc *searchSubTypeDesc(Resources::Command::SubType subType);

	/** Set the link indices from the argument values */
	void initBranches();

	/** List the arguments values as a coma separated string */
	Common::String describeArguments() const;

	/** Gets the command with the specifed index */
	static Command *findCommandWithIndex(const Common::Array<Command *> &commands, int32 index);

	uint16 _index;
	Resources::Command::SubType _subType;
	const SubTypeDesc *_subTypeDesc;
	Common::Array<Resources::Command::Argument> _arguments;

	int32 _followerIndex;
	int32 _trueBranchIndex;
	int32 _falseBranchIndex;

	Command *_follower;
	Command *_trueBranch;
	Command *_falseBranch;
	Common::Array<Command *> _predecessors;

	Block *_block;
};

} // End of namespace Tools
} // End of namespace Stark

#endif // STARK_TOOLS_COMMAND_H
