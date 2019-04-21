/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
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

#ifndef MUTATIONOFJB_COMMAND_H
#define MUTATIONOFJB_COMMAND_H

namespace Common {
class String;
}

namespace MutationOfJB {

class Command;
class ScriptExecutionContext;
class ScriptParseContext;

/**
 * Base class for command parsers.
 *
 * The parser's main job is to create a Command instance from input line.
 */
class CommandParser {
public:
	virtual ~CommandParser();

	/**
	 * Parses the specified line and possibly returns a Command instance.
	 *
	 * @param line Line to parse.
	 * @param parseCtx Parse context.
	 * @param command Output parameter for newly created command.
	 * @return True if the line has been successfully parsed by this parser, false otherwise.
	 * @note You may return true and set command to nullptr.
	 * That means the line has been successfully parsed, but no command is needed.
	 */
	virtual bool parse(const Common::String &line, ScriptParseContext &parseCtx, Command *&command) = 0;

	/**
	 * Called when transitioning parsing between two commands.
	 *
	 * For example, cmdParserA->transition(parseCtx, cmdA, cmdB, cmdParserB) is called after command B is done parsing
	 * to notify command A parser about the transition from command A to command B.
	 * This is useful for sequential commands, because at the time command A is being parsed,
	 * we don't have any information about command B, so we cannot set the next pointer.
	 * Transition method can be used to set the next pointer after command B is available.
	 *
	 * @param parseCtx Parse context.
	 * @param oldCommand Old command (created by this parser).
	 * @param newCommand New command (created by newCommandParser).
	 * @param newCommandParser Command parser which created the new command.
	 */
	virtual void transition(ScriptParseContext &parseCtx, Command *oldCommand, Command *newCommand, CommandParser *newCommandParser);

	/**
	 * Called after the whole script is parsed.
	 *
	 * Can be used for cleanup.
	 *
	 * @param parseCtx Parse context.
	 */
	virtual void finish(ScriptParseContext &parseCtx);
};

/**
 * Base class for script commands.
 */
class Command {
public:
	enum ExecuteResult {
		None,
		Finished,
		InProgress
	};

	virtual ~Command();

	virtual ExecuteResult execute(ScriptExecutionContext &scriptExecCtx) = 0;
	virtual Command *next() const = 0;

	virtual Common::String debugString() const = 0;
};

}

#endif
