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

enum DialogCommands {
	cmdNodeEnd = 0,
	//
	cmdHide = 2,
	cmdUnhide = 3,
	cmdMessage = 4,
	//
	//
	cmdGoto = 7,
	//
	cmdAssign = 9,
	cmdDialogEnd = 255
};

struct ConvDialog {
	int16 _textLineIndex;	// 0-based
	int16 _speechIndex;		// 1-based
	uint16 _nodeOffset;		// offset in section 6
	uint16 _nodeSize;		// size in section 6
};

struct ConvNode {
	uint16 _index;
	uint16 _dialogCount;
	int16 _unk1;
	int16 _unk2;
	int16 _unk3;

	Common::Array<ConvDialog> _dialogs;
};

struct ConversationData {
	uint16 _nodeCount;		// conversation nodes, each one containing several dialog options and messages
	uint16 _dialogCount;		// messages (non-selectable) + texts (selectable)
	uint16 _messageCount;	// messages (non-selectable)
	uint16 _textLineCount;
	uint16 _unk2;
	uint16 _importCount;
	uint16 _speakerCount;
	int _textSize;
	int _commandsSize;

	Common::String _portraits[MAX_SPEAKERS];
	bool _speakerExists[MAX_SPEAKERS];
	Common::String _speechFile;
	Common::Array<uint> _messages;
	Common::StringArray _textLines;
	Common::Array<ConvNode> _convNodes;

	/**
	 * Load the specified conversation resource file
	 */
	void load(const Common::String &filename);
};

struct ConversationCnd {
	struct ConversationVar {
		int v1;
		int v2;
		int v3;
	};

	Common::Array<ConversationVar> _vars;

	/**
	 * Load the specified conversation resource file
	 */
	void load(const Common::String &filename);
};

struct ConversationEntry {
	int _convId;
	ConversationData _data;
	ConversationCnd _cnd;
};

class MADSEngine;

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
	int _val1, _val2, _val3, _val4, _val5;

	/**
	 * Returns the record for the specified conversation, if it's loaded
	 */
	ConversationEntry *getConv(int convId);

	/**
	 * Start a specified conversation slot
	 */
	void start();
public:
	ConversationEntry *_runningConv;
	int _restoreRunning;
	bool _playerEnabled;
	uint32 _startFrameNumber;
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
	 * Sets a variable
	 */
	void setVariable(uint idx, int v1, int v2 = -1);

	int* _nextStartNode;
	int* getVariable(int idx);

	void stop();
	void exportPointer(int *val);
	void exportValue(int val);
	void setHeroTrigger(int val);
	void setInterlocutorTrigger(int val);
	void hold();
	void release();
	void reset(int id);
	void abortConv();
	
	/**
	 * Returns true if any conversation is currently atcive
	 */
	bool active() const { return _runningConv != nullptr; }

	/**
	 * Returns the currently active conversation Id
	 */
	int activeConvId() const { return !active() ? -1 : _runningConv->_convId; }
};

} // End of namespace MADS

#endif /* MADS_CONVERSATIONS_H */
