/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef ULTIMA4_GAME_SCRIPT_H
#define ULTIMA4_GAME_SCRIPT_H

#include "ultima/ultima4/core/types.h"
#include "ultima/shared/conf/xml_node.h"
#include "ultima/shared/std/containers.h"
#include "common/file.h"

namespace Ultima {
namespace Ultima4 {

/**
 * An xml-scripting class. It loads and runs xml scripts that
 * take information and interact with the game environment itself.
 * Currently, it is mainly useful for writing vendor code; however,
 * it should be possible to write scripts for other parts of the
 * game.
 *
 * @todo
 * <ul>
 *      <li>Strip vendor-specific code from the language</li>
 *      <li>Fill in some of the missing integration with the game</li>
 * </ul>
 */
class Script {
public:
	/**
	 * A class that provides information to a script.  It is designed to
	 * translate qualifiers and identifiers in a script to another value.
	 * Each provider is assigned a qualifier that the script uses to
	 * select a provider.  The provider then uses the rest of the information
	 * to translate the information to something useful.
	 */
	class Provider {
	public:
		virtual ~Provider() {}
		virtual Common::String translate(Std::vector<Common::String> &parts) = 0;
	};

private:
	/**
	 * A class that represents a script variable
	 */
	class Variable {
	public:
		Variable();
		Variable(const Common::String &v);
		Variable(const int &v);

		int    &getInt();
		Common::String &getString();

		void    setValue(const int &v);
		void    setValue(const Common::String &v);
		void    unset();

		bool    isInt() const;
		bool    isString() const;
		bool    isSet() const;

	private:
		int _iVal;
		Common::String _sVal;
		bool _set;
	};

public:
	/**
	 * A script return code
	 */
	enum ReturnCode {
		RET_OK,
		RET_REDIRECTED,
		RET_STOP
	};

	/**
	 * The current state of the script
	 */
	enum State {
		STATE_UNLOADED,
		STATE_NORMAL,
		STATE_DONE,
		STATE_INPUT
	};

	/**
	 * The type of input the script is requesting
	 */
	enum InputType {
		INPUT_CHOICE,
		INPUT_NUMBER,
		INPUT_STRING,
		INPUT_DIRECTION,
		INPUT_PLAYER,
		INPUT_KEYPRESS
	};

	/**
	 * The action that the script is taking
	 */
	enum Action {
		ACTION_SET_CONTEXT,
		ACTION_UNSET_CONTEXT,
		ACTION_END,
		ACTION_REDIRECT,
		ACTION_WAIT_FOR_KEY,
		ACTION_WAIT,
		ACTION_STOP,
		ACTION_INCLUDE,
		ACTION_FOR_LOOP,
		ACTION_RANDOM,
		ACTION_MOVE,
		ACTION_SLEEP,
		ACTION_CURSOR,
		ACTION_PAY,
		ACTION_IF,
		ACTION_INPUT,
		ACTION_ADD,
		ACTION_LOSE,
		ACTION_HEAL,
		ACTION_CAST_SPELL,
		ACTION_DAMAGE,
		ACTION_KARMA,
		ACTION_MUSIC,
		ACTION_SET_VARIABLE,
		ACTION_ZTATS
	};

	/**
	 * Constructs a script object
	 */
	Script();
	~Script();

	/**
	 * Adds an information provider for the script
	 */
	void addProvider(const Common::String &name, Provider *p);

	/**
	 * Loads the vendor script
	 */
	bool load(const Common::String &filename, const Common::String &baseId, const Common::String &subNodeName = "", const Common::String &subNodeId = "");

	/**
	 * Unloads the script
	 */
	void unload();

	/**
	 * Runs a script after it's been loaded
	 */
	void run(const Common::String &script);

	/**
	 * Executes the subscript 'script' of the main script
	 */
	ReturnCode execute(Shared::XMLNode *script, Shared::XMLNode *currentItem = nullptr, Common::String *output = nullptr);

	/**
	 * Continues the script from where it left off, or where the last script indicated
	 */
	void _continue();

	/**
	 * Set and retrieve property values
	 */
	void resetState();
	void setState(State state);
	State getState();

	void setTarget(const Common::String &val);
	void setChoices(const Common::String &val);
	void setVar(const Common::String &name, const Common::String &val);
	void setVar(const Common::String &name, int val);
	void unsetVar(const Common::String &name);

	Common::String getTarget();
	InputType getInputType();
	Common::String getInputName();
	Common::String getChoices();
	int getInputMaxLen();

private:
	/**
	 * Translates a script Common::String with dynamic variables
	 */
	void translate(Common::String *script);

	/**
	 * Finds a subscript of script 'node'
	 */
	Shared::XMLNode *find(Shared::XMLNode *node, const Common::String &script, const Common::String &choice = "", bool _default = false);

	/**
	 * Gets a property as Common::String from the script, and
	 * translates it using scriptTranslate.
	 */
	Common::String getPropAsStr(Std::list<Shared::XMLNode *> &nodes, const Common::String &prop, bool recursive);

	Common::String getPropAsStr(Shared::XMLNode *node, const Common::String &prop, bool recursive = false);

	/**
	 * Gets a property as int from the script
	 */
	int getPropAsInt(Std::list<Shared::XMLNode *> &nodes, const Common::String &prop, bool recursive);

	int getPropAsInt(Shared::XMLNode *node, const Common::String &prop, bool recursive = false);

	/**
	 * Gets the content of a script node
	 */
	Common::String getContent(Shared::XMLNode *node);

	/*
	 * Action Functions
	 */
	/**
	 * Sets a new translation context for the script
	 */
	ReturnCode pushContext(Shared::XMLNode *script, Shared::XMLNode *current);

	/**
	 * Removes a node from the translation context
	 */
	ReturnCode popContext(Shared::XMLNode *script, Shared::XMLNode *current);

	/**
	 * End script execution
	 */
	ReturnCode end(Shared::XMLNode *script, Shared::XMLNode *current);

	/**
	 * Wait for keypress from the user
	 */
	ReturnCode waitForKeypress(Shared::XMLNode *script, Shared::XMLNode *current);

	/**
	 * Redirects script execution to another script
	 */
	ReturnCode redirect(Shared::XMLNode *script, Shared::XMLNode *current);

	/**
	 * Includes a script to be executed
	 */
	ReturnCode include(Shared::XMLNode *script, Shared::XMLNode *current);

	/**
	 * Waits a given number of milliseconds before continuing execution
	 */
	ReturnCode wait(Shared::XMLNode *script, Shared::XMLNode *current);

	/**
	 * Executes a 'for' loop script
	 */
	ReturnCode forLoop(Shared::XMLNode *script, Shared::XMLNode *current);

	/**
	 * Randomely executes script code
	 */
	ReturnCode randomScript(Shared::XMLNode *script, Shared::XMLNode *current);

	/**
	 * Moves the player's current position
	 */
	ReturnCode move(Shared::XMLNode *script, Shared::XMLNode *current);

	/**
	 * Puts the player to sleep. Useful when coding inn scripts
	 */
	ReturnCode sleep(Shared::XMLNode *script, Shared::XMLNode *current);

	/**
	 * Enables/Disables the keyboard cursor
	 */
	ReturnCode cursor(Shared::XMLNode *script, Shared::XMLNode *current);

	/**
	 * Pay gold to someone
	 */
	ReturnCode pay(Shared::XMLNode *script, Shared::XMLNode *current);

	/**
	 * Perform a limited 'if' statement
	 */
	ReturnCode _if(Shared::XMLNode *script, Shared::XMLNode *current);

	/**
	 * Get input from the player
	 */
	ReturnCode input(Shared::XMLNode *script, Shared::XMLNode *current);

	/**
	 * Add item to inventory
	 */
	ReturnCode add(Shared::XMLNode *script, Shared::XMLNode *current);

	/**
	 * Lose item
	 */
	ReturnCode lose(Shared::XMLNode *script, Shared::XMLNode *current);

	/**
	 * Heals a party member
	 */
	ReturnCode heal(Shared::XMLNode *script, Shared::XMLNode *current);

	/**
	 * Performs all of the visual/audio effects of casting a spell
	 */
	ReturnCode castSpell(Shared::XMLNode *script, Shared::XMLNode *current);

	/**
	 * Apply damage to a player
	 */
	ReturnCode damage(Shared::XMLNode *script, Shared::XMLNode *current);

	/**
	 * Apply karma changes based on the action taken
	 */
	ReturnCode karma(Shared::XMLNode *script, Shared::XMLNode *current);

	/**
	 * Set the currently playing music
	 */
	ReturnCode music(Shared::XMLNode *script, Shared::XMLNode *current);

	/**
	 * Sets a variable
	 */
	ReturnCode setVar(Shared::XMLNode *script, Shared::XMLNode *current);

	/**
	 * Display a different ztats screen
	 */
	ReturnCode ztats(Shared::XMLNode *script, Shared::XMLNode *current);

	/*
	 * Math and comparison functions
	 */

	 /**
	  * Parses a math Common::String's children into results so
	  * there is only 1 equation remaining.
	  *
	  * ie. <math>5*<math>6/3</math></math>
	  */
	void mathParseChildren(Shared::XMLNode *math, Common::String *result);

	/**
	 * Takes a simple equation Common::String and returns the value
	 */
	int mathValue(const Common::String &str);

	/**
	 * Performs simple math operations in the script
	 */
	int math(int lval, int rval, Common::String &op);

	/**
	 * Parses a Common::String into left integer value, right integer value,
	 * and operator. Returns false if the Common::String is not a valid
	 * math equation
	 */
	bool mathParse(const Common::String &str, int *lval, int *rval, Common::String *op);

	/**
	 * Parses a Common::String containing an operator (+, -, *, /, etc.) into 3 parts,
	 * left, right, and operator.
	 */
	void parseOperation(const Common::String &str, Common::String *lval, Common::String *rval, Common::String *op);

	/**
	 * Does a boolean comparison on a Common::String (math or Common::String),
	 * fails if the Common::String doesn't contain a valid comparison
	 */
	bool compare(const Common::String &str);

	/**
	 * Parses a function into its name and contents
	 */
	void funcParse(const Common::String &str, Common::String *funcName, Common::String *contents);

	/*
	 * Static variables
	 */
private:
	typedef Common::HashMap<Common::String, Action> ActionMap;
	ActionMap _actionMap;

private:
	void removeCurrentVariable(const Common::String &name);
	Shared::XMLNode *_vendorScriptDoc;
	Shared::XMLNode *_scriptNode;
	bool _debug;

	State _state;                    /**< The state the script is in */
	Shared::XMLNode *_currentScript;       /**< The currently running script */
	Shared::XMLNode *_currentItem;         /**< The current position in the script */
	Std::list<Shared::XMLNode *> _translationContext;  /**< A list of nodes that make up our translation context */
	Common::String _target;                  /**< The name of a target script */
	InputType _inputType;            /**< The type of input required */
	Common::String _inputName;               /**< The variable in which to place the input (by default, "input") */
	int _inputMaxLen;                /**< The maximum length allowed for input */

	Common::String _nounName;                /**< The name that identifies a node name of noun nodes */
	Common::String _idPropName;              /**< The name of the property that uniquely identifies a noun node
										 and is used to find a new translation context */

	Common::String _choices;
	int _iterator;

	Common::HashMap<Common::String, Variable *> _variables;
	Common::HashMap<Common::String, Provider *> _providers;
};

} // End of namespace Ultima4
} // End of namespace Ultima

#endif
