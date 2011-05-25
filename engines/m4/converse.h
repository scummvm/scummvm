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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef M4_CONVERSE_H
#define M4_CONVERSE_H

#include "common/array.h"
#include "common/hashmap.h"

#include "m4/globals.h"
#include "m4/viewmgr.h"
#include "m4/sound.h"

namespace M4 {

enum ConversationState {
	kConversationOptionsShown = 0,
	kEntryIsActive = 1,
	kReplyIsActive = 2,
	kNoConversation = 3
};

enum EntryType {
	kVariable = 0,
	kNode = 1,
	kLinearNode = 2,
	kEntry = 3,
	kReply = 4,
	kWeightedReply = 5
};

// Flags are:
// bit 0: if it's 1, the entry is "initial", i.e. not hidden when the dialog starts
// bit 1: if it's 1, the entry persists if selected
enum EntryFlags {
	kEntryInitial = 1,	// byte 0
	kEntryPersists = 2	// byte 1
};

enum EntryActionType {
	kUnknownAction = 0,
	kGotoEntry = 1,
	kHideEntry = 2,
	kUnhideEntry = 3,
	kDestroyEntry = 4,
	kAssignValue = 5,
	kExitConv = 6
};

enum LogicOp {
	kOpPercent = 405,
	kOpGreaterOrEqual = 421,
	kOpLessOrEqual = 420,
	kOpGreaterThan = 413,
	kOpLessThan = 412,
	kOpNotEqual = 422,
	kOpCondNotEqual = 448,
	kOpAssign = 407,
	kOpAnd = 444,
	kOpOr = 445
};

struct Condition {
	int32 offset;
	int32 op;
	int32 val;
};

struct EntryAction {
	int32 targetOffset;				// absolute offset (inside the *.chk file) of the action's target
	int32 value;					// the value set by assignment chunks
	EntryActionType actionType;
	EntryType targetType;
	int32 nodeId;
	int32 entryId;
	bool isConditional;
	Condition condition;
};

struct ConvEntry {
	EntryType entryType;
	int32 id;
	int32 offset;					// absolute offset inside the *.chk file, referenced by other chunks
	int32 size;						// entry action data size (for MADS games)
	int32 flags;
	int32 fallthroughMinEntries;
	int32 fallthroughOffset;
	int32 weight;					// weight for weighted replies
	int32 totalWeight;
	uint16 entryCount;				// entries inside this node (for MADS games)
	char voiceFile[10];
	char text[512];
	bool autoSelect;
	bool visible;
	bool isConditional;
	Condition condition;
	Common::Array<EntryAction*>actions;
	Common::Array<ConvEntry*>entries;
};

struct EntryInfo {
	EntryType targetType;
	int32 nodeIndex;
	int32 entryIndex;
};

struct MessageEntry {
	Common::Array<char*>messageStrings;
};

enum ConverseStyle {CONVSTYLE_EARTH, CONVSTYLE_SPACE};

typedef Common::HashMap<Common::String,EntryInfo,Common::IgnoreCase_Hash,Common::IgnoreCase_EqualTo> OffsetHashMap;
typedef Common::HashMap<Common::String,int32,Common::IgnoreCase_Hash,Common::IgnoreCase_EqualTo> ConvVarHashMap;

class ConversationView : public View {
public:
	ConversationView(MadsM4Engine *vm);
	~ConversationView();
	void setNode(int32 nodeIndex);

	void onRefresh(RectList *rects, M4Surface *destSurface);
	bool onEvent(M4EventType eventType, int32 param, int x, int y, bool &captureEvents);
	int32 getCurrentNodeIndex() { return _currentNodeIndex; }
	void selectEntry(int entryIndex);

private:
	void updateState();
	void playNextReply();

	int32 _currentNodeIndex;
	Common::Array<ConvEntry *> _activeItems;
	int _highlightedIndex;
	int _xEnd;
	bool _entriesShown;
	ConversationState _conversationState;
	SndHandle *_currentHandle;
};

class Converse {

public:
	Converse(MadsM4Engine *vm) : _vm(vm) {}
	~Converse() {}

	void startConversation(const char *convName, bool showConversebox = true, ConverseStyle style = CONVSTYLE_EARTH );
	void endConversation();
	const EntryInfo* getEntryInfo(int32 offset);
	ConvEntry *getNode(int32 index) { return _convNodes[index]; }

	void setValue(int32 offset, int32 value);
	int32 getValue(int32 offset);
	bool evaluateCondition(int32 leftVal, int32 op, int32 rightVal);
	bool performAction(EntryAction *action);
	/*
	void resume() { play(); }
	void play();
	*/
private:
	MadsM4Engine *_vm;
	Common::Array<ConvEntry*>_convNodes;
	Common::Array<MessageEntry*>_madsMessageList;
	Common::Array<char *>_convStrings;
	bool _playerCommandsAllowed;
	bool _interfaceWasVisible;
	ConverseStyle _style;
	OffsetHashMap _offsetMap;
	ConvVarHashMap _variables;

	void loadConversation(const char *convName);
	void loadConversationMads(const char *convName);
	void readConvEntryActions(Common::ReadStream *convS, ConvEntry *curEntry);
	void setEntryInfo(int32 offset, EntryType type, int32 nodeIndex, int32 entryIndex);
};


struct MadsTalkEntry {
	uint16 id;
	const char *desc;
};

#define MADS_TALK_SIZE 5

class MadsConversation {
private:
	MadsTalkEntry _talkList[MADS_TALK_SIZE];
public:
	MadsConversation();

	MadsTalkEntry &operator[](int index) { return _talkList[index]; }
};

} // End of namespace M4

#endif
