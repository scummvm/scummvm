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

/* Based on Archetype interpreter version 1.01 */

#ifndef ARCHETYPE_ARCHETYPE
#define ARCHETYPE_ARCHETYPE

#include "glk/glk_api.h"
#include "glk/archetype/array.h"
#include "glk/archetype/interpreter.h"
#include "glk/archetype/semantic.h"
#include "glk/archetype/statement.h"
#include "glk/archetype/string.h"

namespace Glk {
namespace Archetype {

enum DebugFlag {
	DEBUG_BYTES = 0x01,
	DEBUG_MSGS = 0x02,
	DEBUG_EXPR = 0x04,
	DEBUG_STMT = 0x08
};

/**
 * Archetype game interpreter
 */
class Archetype : public GlkAPI {
private:
	int _saveSlot;
	winid_t _mainWindow;
	String _lastOutputText;
public:
	// keywords.cpp
	XArrayType Literals, Vocabulary;
	XArrayType Type_ID_List, Object_ID_List, Attribute_ID_List;

	// parser.cpp
	String Command;
	int Abbreviate;
	ListType Proximate;
	ListType verb_names;
	ListType object_names;

	// semantic.cpp
	XArrayType Type_List, Object_List;
	ListType Overlooked;
	StringPtr NullStr;
private:
	/**
	 * Engine initialization
	 */
	bool initialize();

	/**
	 * Engine cleanup
	 */
	void deinitialize();

	/**
	 * Main interpreter method
	 */
	void interpret();

	/**
	 * Given an object number, attribute number, anddesired_type, returns the value of the lookup
	 * in the given result.If the desired_type is LVALUE, then it creates a new attribute node
	 * in the object's own attribute list(if not already existing) and returns a pointer to it.
	 * If RVALUE, it evaluates any expression it may find, returning the result of the evaluation.
	 *
	 * Also performs inheritance, looking back through the object's family tree to find the attribute.
	 */
	void lookup(int the_obj, int the_attr, ResultType &result, ContextType &context, DesiredType desired);

	/**
	 * Sends the given message number to the object of the given number. This procedure performs
	 * inheritance; that is, it will search back through the object's ancestry in order to find
	 * someone to perform the message.Has to do something tricky with the default message:
	 * it must first search the entire ancestry for an explicit message, then search again for
	 * a default, if none found.
	 * @param transport			how to send the message : sending to an object,
	 *							passing to an object, or sending(passing) to a type.
	 * @param message			message to send
	 * @param recipient			number of object to receive message
	 * @param result			Output result of the sending
	 * @param context			Context
	 * @returns true if the recipient handles the message; false if it doesn't
	 */
	bool send_message(int transport, int message_sent, int recipient, ResultType &result,
		ContextType &context);

	/**
	 * Evaluates the given expression
	 */
	void eval_expr(ExprTree the_expr, ResultType &result, ContextType &context, DesiredType desired);

	/**
	 * Evaluates the given expression as though it were a condition. Will succeed if the given
	 * expression is not UNDEFINED and not false.
	 * @param the_expr		Expression to evaluate
	 * @returns				true if the condition can be considered true; false otherwise
	 */
	bool eval_condition(ExprTree the_expr, ContextType &context);

	/**
	 * Given a pointer to a statement, executes that statement. Very heavily called
	 * @param the_stmt		pointer to statement to be executed
	 * @param result		the "value" of the execution(for example, the last expression
	 *						of a compound statement
	 */
	void exec_stmt(StatementPtr the_stmt, ResultType &result, ContextType &context);
public:
	/**
	 * Constructor
	 */
	Archetype(OSystem *syst, const GlkGameDescription &gameDesc);

	/**
	 * Run the game
	 */
	void runGame() override;

	/**
	 * Returns the running interpreter type
	 */
	InterpreterType getInterpreterType() const override {
		return INTERPRETER_ARCHETYPE;
	}

	/**
	 * Savegames aren't supported for Archetype games
	 */
	Common::Error readSaveData(Common::SeekableReadStream *rs) override;

	/**
	 * Savegames aren't supported for Archetype games
	 */
	Common::Error writeGameData(Common::WriteStream *ws) override;

	/**
	 * Returns true if a savegame is being loaded directly from the ScummVM launcher
	 */
	bool loadingSavegame() const {
		return _saveSlot >= 0;
	}

	/**
	 * Handles loading the savegame specified in the ScummVM launcher
	 */
	Common::Error loadLauncherSavegame();

	/**
	 * Write some text to the screen
	 */
	void write(const String fmt, ...);

	/**
	 * Write a line to the screen
	 */
	void writeln(const String fmt, ...);
	void writeln() { writeln(""); }

	/**
	 * Read an input line typed by the player
	 */
	String readLine();

	/**
	 * Read in a single key
	 */
	char readKey();
};

extern Archetype *g_vm;

} // End of namespace Archetype
} // End of namespace Glk

#endif
