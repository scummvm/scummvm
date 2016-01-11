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

#ifndef MADS_CONVERSATIONS_H
#define MADS_CONVERSATIONS_H

#include "common/scummsys.h"
#include "common/array.h"
#include "common/str-array.h"
#include "mads/screen.h"

namespace MADS {

#define MAX_CONVERSATIONS 5
#define MAX_SPEAKERS 5

enum ConversationMode {
	CONVMODE_NONE = -1,
	CONVMODE_0 = 0,
	CONVMODE_1 = 1,
	CONVMODE_2 = 2,
	CONVMODE_3 = 3,
	CONVMODE_4 = 4,
	CONVMODE_5 = 5,
	CONVMODE_6 = 6,
	CONVMODE_7 = 7,
	CONVMODE_8 = 8,
	CONVMODE_9 = 9,
	CONVMODE_STOP = 10
};

enum DialogCommand {
	CMD_NODE_END = 0,
	CMD_1 = 1,
	CMD_HIDE = 2,
	CMD_UNHIDE = 3,
	CMD_MESSAGE = 4,
	CMD_5 = 5,
	CMD_ERROR = 6,
	CMD_7 = 7,
	CMD_GOTO = 8,
	CMD_ASSIGN = 9,
	CMD_DIALOG_END = 255
};

enum ConvEntryFlag {
	ENTRYFLAG_2 = 2,
	ENTRYFLAG_4000 = 0x4000,
	ENTRYFLAG_8000 = 0x8000
};

struct ScriptEntry {
	struct Conditional {
		struct CondtionalParamEntry {
			bool _isVariable;
			int _val;

			/**
			 * Constructor
			 */
			CondtionalParamEntry() : _isVariable(false), _val(0) {}
		};

		uint _paramsFlag;
		CondtionalParamEntry _param1;
		CondtionalParamEntry _param2;

		/**
		 * Constructor
		 */
		Conditional() : _paramsFlag(false) {}

		/**
		 * Loads data from a passed stream into the parameters structure
		 */
		void load(Common::SeekableReadStream &s);
	};

	DialogCommand _command;
	Conditional _conditionals[3];
	Common::Array<int> _params;

	/**
	 * Constructor
	 */
	ScriptEntry() : _command(CMD_NODE_END) {}

	/**
	 * Loads data from a passed stream into the parameters structure
	 */
	void load(Common::SeekableReadStream &s);
};

/**
 * Representation of scripts associated with a dialog
 */
class DialogScript : public Common::Array<ScriptEntry> {
public:
	/**
	 * Loads a script from the passed stream
	 */
	void load(Common::SeekableReadStream &s, uint startingOffset);
};

/**
 * Reperesents the data for a dialog to be displayed in a conversation
 */
struct ConvDialog {
	struct ScriptEntry {
		DialogCommand _command;
	};

	int16 _textLineIndex;	// 0-based
	int16 _speechIndex;		// 1-based
	uint16 _scriptOffset;	// offset of script entry
	uint16 _scriptSize;		// size of script entry

	DialogScript _script;
};

/**
 * Represents a node within the conversation control logic
 */
struct ConvNode {
	uint16 _index;
	uint16 _dialogCount;
	int16 _unk1;
	bool _active;
	int16 _unk3;

	Common::Array<ConvDialog> _dialogs;
};

/**
 * Represents the static, non-changing data for a conversation
 */
struct ConversationData {
	uint16 _nodeCount;		// conversation nodes, each one containing several dialog options and messages
	uint16 _dialogCount;		// messages (non-selectable) + texts (selectable)
	uint16 _messageCount;	// messages (non-selectable)
	uint16 _textLineCount;
	uint16 _unk2;
	uint16 _maxImports;
	uint16 _speakerCount;
	int _textSize;
	int _commandsSize;

	Common::String _portraits[MAX_SPEAKERS];
	bool _speakerExists[MAX_SPEAKERS];
	Common::String _speechFile;
	Common::Array<uint> _messages;
	Common::StringArray _textLines;
	Common::Array<ConvNode> _nodes;
	Common::Array<ConvDialog> _dialogs;

	/**
	 * Load the specified conversation resource file
	 */
	void load(const Common::String &filename);
};

struct ConversationVar {
	bool _isPtr;
	int _val;
	int *_valPtr;

	/**
	 * Constructor
	 */
	ConversationVar() : _isPtr(false), _val(0), _valPtr(nullptr) {}

	/**
	 * Sets a numeric value
	 */
	void setValue(int val);

	/**
	 * Sets a pointer value
	 */
	void setValue(int *val);

	/**
	 * Return either the variable's pointer, or a pointer to it's direct value
	 */
	int *getValue() { return _isPtr ? _valPtr : &_val; }

	/**
	 * Returns true if variable is a pointer
	 */
	bool isPtr() const { return _isPtr; }

	/**
	 * Returns true if variable is numeric
	 */
	bool isNumeric() const { return !_isPtr; }
};

/**
 * Conditional (i.e. changeable) data for the conversation
 */
struct ConversationConditionals {
	Common::Array<uint> _importVariables;
	Common::Array<uint> _entryFlags;
	Common::Array<ConversationVar> _vars;
	int _numImports;

	int _currentNode;
	int _fieldC;
	int _fieldE;
	int _field10;
	int _field12;
	int _field28;
	int _field3C;
	int _field50;
	int _field64;

	/**
	 * Constructor
	 */
	ConversationConditionals();

	/**
	 * Load the specified conversation conditionals resource file
	 */
	void load(const Common::String &filename);
};

/**
 * Represents all the data needed for a particular loaded conversation
 */
struct ConversationEntry {
	int _convId;
	ConversationData _data;
	ConversationConditionals _cnd;
};

class MADSEngine;

/**
 * Manager for loading and running conversations
 */
class GameConversations {
private:
	MADSEngine *_vm;
	ConversationEntry _conversations[MAX_CONVERSATIONS];
	bool _speakerActive[MAX_SPEAKERS];
	int _speakerPortraits[MAX_SPEAKERS];
	int _speakerExists[MAX_SPEAKERS];
	int _arr4[MAX_SPEAKERS];
	int _arr5[MAX_SPEAKERS];
	int _arr6[MAX_SPEAKERS];
	InputMode _inputMode;
	int _val1;
	ConversationMode _currentMode;
	ConversationMode _priorMode;
	int _verbId;
	int _speakerVal;
	int _heroTrigger;
	TriggerMode _heroTriggerMode;
	int _interlocutorTrigger;
	TriggerMode _interlocutorTriggerMode;
	ConversationEntry *_runningConv;
	int _restoreRunning;
	bool _playerEnabled;
	uint32 _startFrameNumber;
	ConversationVar *_vars;
	ConversationVar *_nextStartNode;
	int _currentNode;
	int _dialogNodeOffset, _dialogNodeSize;
	
	/**
	 * Returns the record for the specified conversation, if it's loaded
	 */
	ConversationEntry *getConv(int convId);

	/**
	 * Start a specified conversation slot
	 */
	void start();

	/**
	 * Remove any currently active dialog window
	 */
	void removeActiveWindow();

	/**
	 * Flags a conversation option/entry
	 */
	void flagEntry(DialogCommand mode, int entryIndex);

	/**
	 * Generate a menu
	 */
	ConversationMode generateMenu();

	/**
	 * Generate text
	 */
	void generateText(int textLineIndex, int v2, int *v3);

	/**
	 * Generate message
	 */
	void generateMessage(int textLineIndex, int v2, int *v3, int *v4);

	/**
	 * Gets the next node
	 */
	bool nextNode();

	/**
	 * Executes a conversation entry
	 */
	int executeEntry(int index);
public:
	/**
	 * Constructor
	 */
	GameConversations(MADSEngine *vm);

	/**
	 * Destructor
	 */
	virtual ~GameConversations();

	/**
	 * Gets the specified conversation and loads into into a free slot
	 * in the conversation list
	 */
	void load(int id);

	/**
	 * Run a specified conversation number. The conversation must have
	 * previously been loaded by calling the load method
	 */
	void run(int id);

	/**
	 * Sets a variable to a numeric value
	 */
	void setVariable(uint idx, int val);

	/**
	 * Sets a variable to a pointer value
	 */
	void setVariable(uint idx, int *val);

	/**
	 * Sets the starting node index
	 */
	void setStartNode(uint nodeIndex);

	/**
	 * Set the hero trigger
	 */
	void setHeroTrigger(int val);
	
	/**
	 * Set the interlocutor trigger
	 */
	void setInterlocutorTrigger(int val);

	/**
	 * Returns either the pointer value of a variable, or if the variable
	 * contains a numeric value directly, returns a pointer to it
	 */
	int *getVariable(int idx);

	/**
	 * Hold the current mode value
	 */
	void hold();

	/**
	 * Release the prevoiusly held mode value
	 */
	void release();

	/**
	 * Stop any currently running conversation
	 */
	void stop();

	/**
	 * Adds the passed pointer into the list of import variables for the given conversation
	 */
	void exportPointer(int *ptr);

	/**
	 * Adds the passed value into the list of import variables for the given conversation
	 */
	void exportValue(int val);

	void reset(int id);
	
	/**
	 * Handles updating the conversation display
	 */
	void update(bool flag);

	/**
	 * Returns true if any conversation is currently atcive
	 */
	bool active() const { return _runningConv != nullptr; }

	/**
	 * Returns the currently active conversation Id
	 */
	int activeConvId() const { return !active() ? -1 : _runningConv->_convId; }

	/**
	 * Returns _restoreRunning value
	 */
	int restoreRunning() const { return _restoreRunning; }

	/**
	 * Returns the current conversation mode
	 */
	ConversationMode currentMode() const { return _currentMode; }
};

} // End of namespace MADS

#endif /* MADS_CONVERSATIONS_H */
