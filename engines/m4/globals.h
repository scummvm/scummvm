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

#ifndef M4_GLOBALS_H
#define M4_GLOBALS_H

#include "common/scummsys.h"
#include "common/array.h"
#include "common/hashmap.h"
#include "common/rect.h"
#include "common/list.h"
#include "common/ptr.h"

namespace Common {
class SeekableReadStream;
}

namespace M4 {

class MadsM4Engine;
class M4Engine;
class MadsEngine;
class ScriptInterpreter;
class ScriptFunction;

// Globals
enum WoodScriptGlobals {
	kGlobTime		= 0,
	kGlobTimeDelta	= 1,
	kGlobMinY		= 2,
	kGlobMaxY		= 3,
	kGlobMinScale	= 4,
	kGlobMaxScale	= 5,
	kGlobScaler		= 6,
	kGlobTemp1		= 7,
	kGlobTemp2		= 8,
	kGlobTemp3		= 9,
	kGlobTemp4		= 10,
	kGlobTemp5		= 11,
	kGlobTemp6		= 12,
	kGlobTemp7		= 13,
	kGlobTemp8		= 14,
	kGlobTemp9		= 15,
	kGlobTemp10		= 16,
	kGlobTemp11		= 17,
	kGlobTemp12		= 18,
	kGlobTemp13		= 19,
	kGlobTemp14		= 20,
	kGlobTemp15		= 21,
	kGlobTemp16		= 22,
	kGlobTemp17		= 23,
	kGlobTemp18		= 24,
	kGlobTemp19		= 25,
	kGlobTemp20		= 26,
	kGlobTemp21		= 27,
	kGlobTemp22		= 28,
	kGlobTemp23		= 29,
	kGlobTemp24		= 30,
	kGlobTemp25		= 31,
	kGlobTemp26		= 32,
	kGlobTemp27		= 33,
	kGlobTemp28		= 34,
	kGlobTemp29		= 35,
	kGlobTemp30		= 36,
	kGlobTemp31		= 37,
	kGlobTemp32		= 38
};

const uint32 SERIES_FORWARD				=  0;
const uint32 SERIES_PINGPONG				=  1;
const uint32 SERIES_BACKWARD				=  2;
const uint32 SERIES_RANDOM					=  4;
const uint32 SERIES_NO_TOSS				=  8;
const uint32 SERIES_STICK					= 16;
const uint32 SERIES_LOOP_TRIGGER			= 32;
const uint32 SERIES_LOAD_PALETTE			= 64;
const uint32 SERIES_HORZ_FLIP				=128;

enum KernelTriggerType {
	KT_PARSE = 1,
	KT_DAEMON,
	KT_PREPARSE
};

class Kernel {
private:
	MadsM4Engine *_vm;
	ScriptFunction *_globalDaemonFn, *_globalParserFn;
	ScriptFunction *_sectionInitFn, *_sectionDaemonFn, *_sectionParserFn;
	ScriptFunction *_roomInitFn, *_roomDaemonFn, *_roomPreParserFn, *_roomParserFn;
	void pauseEngines();
	void unpauseEngines();
public:
	Kernel(MadsM4Engine *vm);

	// TODO: Move to some palette/fading class
	int fadeUpDuration, firstFadeColorIndex;
	int	minPalEntry, maxPalEntry;

	bool paused;
	//machine* myWalker;
	bool repeatedlyCallDeamon;
	bool daemonTriggerAvailable;
	bool betweenRooms;
	int currentSection, newSection, previousSection;
	int currentRoom, newRoom, previousRoom;

	int32 trigger;
	KernelTriggerType triggerMode;

	int32 createTrigger(int32 triggerNum);
	bool sendTrigger(int32 triggerNum);
	bool handleTrigger(int32 triggerNum);

	void loadGlobalScriptFunctions();
	void loadSectionScriptFunctions();
	void loadRoomScriptFunctions();

	void globalDaemon();
	void globalParser();

	void sectionInit();
	void sectionDaemon();
	void sectionParser();

	void roomInit();
	void roomDaemon();
	void roomPreParser();
	void roomParser();

	void pauseGame(bool value);
};

#define TOTAL_NUM_VARIABLES 210

#define PLAYER_INVENTORY 2

enum MADSArticles {
	kArticleNone	= 0,
	kArticleWith	= 1,
	kArticleTo		= 2,
	kArticleAt		= 3,
	kArticleFrom	= 4,
	kArticleOn		= 5,
	kArticleIn		= 6,
	kArticleUnder	= 7,
	kArticleBehind	= 8
};

struct VocabEntry {
	uint8 flags1;
	uint8 flags2;
	uint16 vocabId;
};

class MadsObject {
public:
	MadsObject() {}
	MadsObject(Common::SeekableReadStream *stream);
	void load(Common::SeekableReadStream *stream);
	bool isInInventory() const { return _roomNumber == PLAYER_INVENTORY; }
	void setRoom(int roomNumber);

	uint16 _descId;
	uint16 _roomNumber;
	MADSArticles _article;
	uint8 _vocabCount;
	VocabEntry _vocabList[3];
};

typedef Common::Array<Common::SharedPtr<MadsObject> > MadsObjectArray;

class Globals {
private:
	MadsM4Engine *_vm;
public:
	Globals(MadsM4Engine *vm);
	virtual ~Globals() {}

	bool isInterfaceVisible();

};

class M4Globals : public Globals {
private:
	M4Engine *_vm;
public:
	M4Globals(M4Engine *vm);
	virtual ~M4Globals() {}

	bool invSuppressClickSound;
};

enum RexPlayerSex { SEX_MALE = 0, SEX_FEMALE = 2, SEX_UNKNOWN = 1};

enum MadsDialogType { DIALOG_NONE = 0, DIALOG_GAME_MENU = 1, DIALOG_SAVE = 2, DIALOG_RESTORE = 3, DIALOG_OPTIONS = 4,
		DIALOG_DIFFICULTY = 5, DIALOG_ERROR = 6};

struct MadsConfigData {
	bool musicFlag;
	bool soundFlag;
	bool easyMouse;
	bool invObjectsStill;
	bool textWindowStill;
	int storyMode;
	int screenFades;
};

#define GET_GLOBAL(x) (_madsVm->globals()->_globals[x])
#define GET_GLOBAL32(x) (((uint32)_madsVm->globals()->_globals[x + 1] << 16) | _madsVm->globals()->_globals[x])
#define SET_GLOBAL(x,y) _madsVm->globals()->_globals[x] = y
#define SET_GLOBAL32(x,y) { _madsVm->globals()->_globals[x] = (y) & 0xffff; _madsVm->globals()->_globals[(x) + 1] = (y) >> 16; }

typedef int (*IntFunctionPtr)();

union DataMapEntry {
	bool *boolValue;
	uint16 *uint16Value;
	int *intValue;
	IntFunctionPtr fnPtr;
};

typedef Common::HashMap<uint16, uint16> DataMapHash;

class MadsGlobals : public Globals {
private:
	struct MessageItem {
		uint32 id;
		uint32 offset;
		uint16 uncompSize;
		uint16 compSize;
	};

	MadsEngine *_vm;
	Common::Array<char* > _madsVocab;
	Common::Array<char* > _madsQuotes;
	Common::Array<MessageItem> _madsMessages;
	MadsObjectArray _madsObjects;
	Common::List<int> _visitedScenes;
public:
	MadsGlobals(MadsEngine *vm);
	~MadsGlobals();

	// MADS variables
	uint16 _globals[TOTAL_NUM_VARIABLES];
	MadsConfigData _config;
	bool playerSpriteChanged;
	MadsDialogType dialogType;
	int sceneNumber;
	int previousScene;
	int16 _nextSceneId;
	uint16 actionNouns[3];
	DataMapHash _dataMap;
	int _difficultyLevel;

	void loadMadsVocab();
	uint32 getVocabSize() { return _madsVocab.size(); }
	const char *getVocab(uint32 index) {
		// Vocab list is 1-based, so always subtract one from index provided
		assert((index > 0) && (index <= _madsVocab.size()));
		return _madsVocab[index - 1];
	}

	void loadQuotes();
	uint32 getQuotesSize() { return _madsQuotes.size(); }
	const char *getQuote(uint32 index) { return _madsQuotes[index - 1]; }
	// DEPRECATED: ScummVM re-implementation keeps all the quotes loaded, so the methods below are stubs
	void clearQuotes() {}
	void loadQuoteRange(int startNum, int endNum) {}
	void loadQuoteSet(...) {}
	void loadQuote(int quoteNum) {}

	void loadMadsMessagesInfo();
	uint32 getMessagesSize() { return _madsMessages.size(); }
	int messageIndexOf(uint32 messageId);
	const char *loadMessage(uint index);

	void loadMadsObjects();
	uint32 getObjectsSize() { return _madsObjects.size(); }
	MadsObject *getObject(uint32 index) { return _madsObjects[index].get(); }

	void addVisitedScene(int sceneNumber);
	bool isSceneVisited(int sceneNumber);
	void removeVisitedScene(int sceneNumber);
};

#define PLAYER_FIELD_LENGTH 40

class Player {
public:
	Player(MadsM4Engine *vm);
	void setCommandsAllowed(bool value);

	// Variables
	Common::Point position;				// Player's current position
	int facing;							// Facing direction

	char verb[PLAYER_FIELD_LENGTH];		// Action strings
	char noun[PLAYER_FIELD_LENGTH];
	char prep[PLAYER_FIELD_LENGTH];
	char object[PLAYER_FIELD_LENGTH];
	Common::String assetName, shadowName;
	int walkerType, shadowType;
	bool needToWalk, readyToWalk, waitingForWalk;
	bool commandsAllowed;
	bool commandReady;
	bool visible;
	bool beenInRoomBefore;
	bool walkerInCurrentRoom;
	int32 walkerTriggerNum;
	int walkFacing;
	bool setCommandsAllowedFlag;

	bool said(const char *word1, const char *word2 = NULL, const char *word3 = NULL);
	bool saidAny(const char *word1, const char *word2, const char *word3,
		const char *word4, const char *word5, const char *word6, const char *word7,
		const char *word8, const char *word9, const char *word10);

private:
	 MadsM4Engine *_vm;
};

} // End of namespace M4

#endif
