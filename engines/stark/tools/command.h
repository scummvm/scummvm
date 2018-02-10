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
#include "common/hashmap.h"
#include "common/hash-ptr.h"

#include "engines/stark/resources/command.h"

namespace Stark {
namespace Tools {

class Block;
class DefinitionRegistry;

/**
 * A base script command for disassembly use
 *
 * As opposed to the Command class in the Resources namespace, this class
 * is not meant to be executed. It is meant to be used for script disassembly,
 * to store analysis results.
 */
class Command {
public:
	enum ControlFlowType {
		kFlowNormal,
		kFlowBranch,
		kFlowEnd
	};
	typedef Common::Array<Resources::Command::Argument> ArgumentArray;

	Command(Command *command);
	Command(Resources::Command *resource);

	/**
	 * Print a call to this command to the debug output
	 */
	void printCall() const;

	/** This command's resource tree index */
	uint16 getIndex() const;
	Resources::Command::SubType getSubType() const;
	bool hasSubtypeDescription() const;

	/** List the command's arguments ignoring the control flow related ones */
	ArgumentArray getEffectiveArguments() const;

protected:
	struct SubTypeDesc {
		Resources::Command::SubType subType;
		const char *name;
		ControlFlowType controlFlowType;
	};

	/** Get a description for a command subtype from an internal database */
	static const SubTypeDesc *searchSubTypeDesc(Resources::Command::SubType subType);

	/** List the arguments values as a coma separated string */
	Common::String describeArguments(DefinitionRegistry *definitions) const;

	uint16 _index;
	Resources::Command::SubType _subType;
	const SubTypeDesc *_subTypeDesc;
	ArgumentArray _arguments;
};

/**
 * A script command with control flow information
 *
 * This class is a node in the disassembly command control flow graph.
 * It is referenced by the blocks in the block control flow graph.
 */
class CFGCommand : public Command {
public:
	CFGCommand(Resources::Command *resource);

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
	CFGCommand *getFollower() const;
	CFGCommand *getTrueBranch() const;
	CFGCommand *getFalseBranch() const;

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
	void linkBranches(const Common::Array<CFGCommand *> &commands);

protected:
	/** Set the link indices from the argument values */
	void initBranches();

	/** Gets the command with the specifed index */
	static CFGCommand *findCommandWithIndex(const Common::Array<CFGCommand *> &commands, int32 index);

	int32 _followerIndex;
	int32 _trueBranchIndex;
	int32 _falseBranchIndex;

	CFGCommand *_follower;
	CFGCommand *_trueBranch;
	CFGCommand *_falseBranch;
	Common::Array<CFGCommand *> _predecessors;

	Block *_block;
};

/**
 * Storage for aliases between world resources and names
 */
class DefinitionRegistry {
public:
	/**
	 * Add a definition from a reference
	 *
	 * The name is computed from the object's name
	 */
	void registerReference(const ResourceReference &reference);

	/** Get a previously registered definition from a reference */
	Common::String getFromReference(const ResourceReference &reference) const;

	/** Print all the registered definitions */
	void printAll() const;

private:
	typedef Common::HashMap<Resources::Object *, Common::String> DefinitionMap;

	Common::String stringToCamelCase(const Common::String &input);

	DefinitionMap _definitions;
};

} // End of namespace Tools
} // End of namespace Stark

#endif // STARK_TOOLS_COMMAND_H
