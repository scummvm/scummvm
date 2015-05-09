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

#ifndef SHERLOCK_TALK_H
#define SHERLOCK_TALK_H

#include "common/scummsys.h"
#include "common/array.h"
#include "common/rect.h"
#include "common/serializer.h"
#include "common/stream.h"
#include "common/stack.h"

namespace Sherlock {

#define MAX_TALK_SEQUENCES 11
#define MAX_TALK_FILES 500

enum {
	SWITCH_SPEAKER				= 128,
	RUN_CANIMATION				= 129,
	ASSIGN_PORTRAIT_LOCATION	= 130,
	PAUSE						= 131,
	REMOVE_PORTRAIT				= 132,
	CLEAR_WINDOW				= 133,
	ADJUST_OBJ_SEQUENCE			= 134,
	WALK_TO_COORDS				= 135,
	PAUSE_WITHOUT_CONTROL		= 136,
	BANISH_WINDOW				= 137,
	SUMMON_WINDOW				= 138,
	SET_FLAG					= 139,
	SFX_COMMAND					= 140,
	TOGGLE_OBJECT				= 141,
	STEALTH_MODE_ACTIVE			= 142,
	IF_STATEMENT				= 143,
	ELSE_STATEMENT				= 144,
	END_IF_STATEMENT			= 145,
	STEALTH_MODE_DEACTIVATE		= 146,
	TURN_HOLMES_OFF				= 147,
	TURN_HOLMES_ON				= 148,
	GOTO_SCENE					= 149,
	PLAY_PROLOGUE				= 150,
	ADD_ITEM_TO_INVENTORY		= 151,
	SET_OBJECT					= 152,
	CALL_TALK_FILE				= 153,
	MOVE_MOUSE					= 154,
	DISPLAY_INFO_LINE			= 155,
	CLEAR_INFO_LINE				= 156,
	WALK_TO_CANIMATION			= 157,
	REMOVE_ITEM_FROM_INVENTORY	= 158,
	ENABLE_END_KEY				= 159,
	DISABLE_END_KEY				= 160,
	CARRIAGE_RETURN				= 161
};

struct SequenceEntry {
	int _objNum;
	Common::Array<byte> _sequences;
	int _frameNumber;
	int _seqTo;

	SequenceEntry();
};

struct ScriptStackEntry {
	Common::String _name;
	int _currentIndex;
	int _select;
};

struct Statement {
	Common::String _statement;
	Common::String _reply;
	Common::String _linkFile;
	Common::String _voiceFile;
	Common::Array<int> _required;
	Common::Array<int> _modified;
	int _portraitSide;
	int _quotient;
	int _talkMap;
	Common::Rect _talkPos;

	void synchronize(Common::SeekableReadStream &s);
};

struct TalkHistoryEntry {
	bool _data[16];

	TalkHistoryEntry();
	bool &operator[](int index) { return _data[index]; }
};

struct TalkSequences {
	byte _data[MAX_TALK_SEQUENCES];

	TalkSequences() { clear(); }
	TalkSequences(const byte *data);

	byte &operator[](int idx) { return _data[idx]; }
	void clear();
};

class SherlockEngine;
class UserInterface;

class Talk {
	friend class UserInterface;
private:
	Common::Array<TalkSequences> STILL_SEQUENCES;
	Common::Array<TalkSequences> TALK_SEQUENCES;
private:
	SherlockEngine *_vm;
	Common::Stack<SequenceEntry> _savedSequences;
	Common::Stack<SequenceEntry> _sequenceStack;
	Common::Stack<ScriptStackEntry> _scriptStack;
	Common::Array<Statement> _statements;
	TalkHistoryEntry _talkHistory[MAX_TALK_FILES];
	int _speaker;
	int _talkIndex;
	int _scriptSelect;
	int _converseNum;
	int _talkStealth;
	int _talkToFlag;
	int _scriptSaveIndex;
	int _scriptCurrentIndex;
private:
	void stripVoiceCommands();
	void setTalkMap();

	bool displayTalk(bool slamIt);

	int talkLine(int lineNum, int stateNum, byte color, int lineY, bool slamIt);

	void doScript(const Common::String &script);

	int waitForMore(int delay);
public:
	bool _talkToAbort;
	int _talkCounter;
	int _talkTo;
	int _scriptMoreFlag;
	Common::String _scriptName;
	bool _moreTalkUp, _moreTalkDown;
public:
	Talk(SherlockEngine *vm);
	void setSequences(const byte *talkSequences, const byte *stillSequences,
		int maxPeople);

	Statement &operator[](int idx) { return _statements[idx]; }

	void talkTo(const Common::String &filename);

	void talk(int objNum);

	void freeTalkVars();

	void drawInterface();

	void loadTalkFile(const Common::String &filename);

	void setStillSeq(int speaker);
	void clearSequences();
	void pullSequence();
	void pushSequence(int speaker);
	void setSequence(int speaker);
	bool isSequencesEmpty() const { return _scriptStack.empty(); }

	void popStack();

	void synchronize(Common::Serializer &s);
};

} // End of namespace Sherlock

#endif
