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

#ifndef M4_MADS_VIEWS_H
#define M4_MADS_VIEWS_H

#include "m4/gui.h"
#include "m4/viewmgr.h"
#include "common/rect.h"
#include "common/list.h"
#include "common/ptr.h"

namespace M4 {

class MadsView;

enum MadsActionMode {ACTMODE_NONE = 0, ACTMODE_VERB = 1, ACTMODE_OBJECT = 3, ACTMODE_TALK = 6};
enum MadsActionMode2 {ACTMODE2_0 = 0, ACTMODE2_2 = 2, ACTMODE2_4 = 4, ACTMODE2_5 = 5};
enum AbortTimerMode {ABORTMODE_0 = 0, ABORTMODE_1 = 1, ABORTMODE_2 = 2};

struct ActionDetails {
	int verbId;
	int objectNameId;
	int indirectObjectId;
};

struct MadsActionSavedFields {
	int articleNumber;
	int actionMode;
	int actionMode2;
	bool lookFlag;
	int selectedRow;
};

class MadsAction {
private:
	MadsView &_owner;
	char _statusText[100];
	char _dialogTitle[100];

	void appendVocab(int vocabId, bool capitalise = false);
public:
	ActionDetails _action, _activeAction;
	int _currentAction;
	int8 _flags1, _flags2;
	MadsActionMode _actionMode;
	MadsActionMode2 _actionMode2;
	int _articleNumber;
	bool _lookFlag;
	int _selectedRow;
	bool _textChanged;
	int _selectedAction;
	bool _startWalkFlag;
	int _statusTextIndex;
	int _hotspotId;
	MadsActionSavedFields _savedFields;
	bool _walkFlag;

	// Unknown fields
	int16 _v86F3A;
	int16 _v86F42;
	int16 _v86F4E;
	bool _v86F4A;
	int16 _v86F4C;
	int _v83338;
	bool _inProgress;
	AbortTimerMode _v8453A;

public:
	MadsAction(MadsView &owner);

	void clear();
	void set();
	const char *statusText() const { return _statusText; }
	void refresh();
	void startAction();
	void checkAction();
	bool isAction(int verbId, int objectNameId = 0, int indirectObjectId = 0);
};

class SpriteSlotSubset {
public:
	int spriteListIndex;
	int frameNumber;
	int xp;
	int yp;
	int depth;
	int scale;
};

class MadsSpriteSlot {
public:
	int spriteType;
	int seqIndex;
	int spriteListIndex;
	int frameNumber;
	int xp;
	int yp;
	int depth;
	int scale;

	MadsSpriteSlot() { }

	bool operator==(const SpriteSlotSubset &other) const;
	void copy(const SpriteSlotSubset &other);
};

#define SPRITE_SLOTS_SIZE 50

enum SpriteIdSpecial {
	BACKGROUND_SPRITE = -4, FULL_SCREEN_REFRESH = -2, EXPIRED_SPRITE = -1, SPRITE_ZERO = 0, FOREGROUND_SPRITE = 1
};

class MadsSpriteSlots {
private:
	MadsView &_owner;
	Common::Array<MadsSpriteSlot> _entries;
	Common::Array<SpriteAsset *> _sprites;
public:
	int startIndex;

	MadsSpriteSlots(MadsView &owner);
	~MadsSpriteSlots();

	MadsSpriteSlot &operator[](int idx) {
		assert(idx < SPRITE_SLOTS_SIZE);
		return _entries[idx];
	}
	SpriteAsset &getSprite(int idx) {
		assert(idx < (int)_sprites.size());
		return *_sprites[idx];
	}

	int getIndex();
	int addSprites(const char *resName, bool suppressErrors = false, int flags = 0);
	int addSprites(SpriteAsset *spriteSet);
	void deleteSprites(int listIndex);
	void clear();
	void deleteTimer(int seqIndex);

	void drawBackground();
	void drawForeground(M4Surface *viewport);
	void setDirtyAreas();
	void fullRefresh();
	void cleanUp();
};

class MadsTextDisplayEntry {
public:
	bool active;
	int expire;
	int spacing;
	Common::Rect bounds;
	uint8 color1;
	uint8 color2;
	Font *font;
	const char *msg;

	MadsTextDisplayEntry() { active = false; }
};

#define TEXT_DISPLAY_SIZE 40

class MadsTextDisplay {
private:
	MadsView &_owner;
	Common::Array<MadsTextDisplayEntry> _entries;
public:
	MadsTextDisplay(MadsView &owner);

	MadsTextDisplayEntry &operator[](int idx) {
		assert(idx < TEXT_DISPLAY_SIZE);
		return _entries[idx];
	}

	void expire(int idx) {
		assert(idx < TEXT_DISPLAY_SIZE);
		_entries[idx].expire = -1;
	}

	int add(int xp, int yp, uint fontColor, int charSpacing, const char *msg, Font *font);
	void clear();
	void draw(M4Surface *view);
	void setDirtyAreas();
	void setDirtyAreas2();
	void cleanUp();
};

#define TIMED_TEXT_SIZE 10
#define INDEFINITE_TIMEOUT 9999999

enum KernelMessageFlags {KMSG_QUOTED = 1, KMSG_PLAYER_TIMEOUT = 2, KMSG_SEQ_ENTRY = 4, KMSG_SCROLL = 8, KMSG_RIGHT_ALIGN = 0x10,
	KMSG_CENTER_ALIGN = 0x20, KMSG_EXPIRE = 0x40, KMSG_ACTIVE = 0x80};

class MadsKernelMessageEntry {
public:
	uint8 flags;
	int sequenceIndex;
	char asciiChar;
	char asciiChar2;
	int color1;
	int color2;
	Common::Point position;
	int textDisplayIndex;
	int msgOffset;
	int numTicks;
	uint32 frameTimer2;
	uint32 frameTimer;
	uint32 timeout;
	int abortTimers;
	AbortTimerMode abortMode;
	uint16 actionNouns[3];
	char msg[100];

	MadsKernelMessageEntry() {
		flags = 0;
	}
};

class MadsKernelMessageList {
private:
	MadsView &_owner;
	Common::Array<MadsKernelMessageEntry> _entries;
	Font *_talkFont;
public:
	int word_8469E;
public:
	MadsKernelMessageList(MadsView &owner);

	void clear();
	int add(const Common::Point &pt, uint fontColor, uint8 flags, uint8 abortTimers, uint32 timeout, const char *msg);
	int addQuote(int quoteId, int abortTimers, uint32 timeout);
	void scrollMessage(int msgIndex, int numTicks, bool quoted);
	void setSeqIndex(int msgIndex, int seqIndex);
	void remove(int msgIndex);
	void reset();
	void update();
	void processText(int msgIndex);
};

class ScreenObjectEntry {
public:
	Common::Rect bounds;
	int category;
	int index;
	int layer;
	bool active;

	ScreenObjectEntry() { active = false; }
};

class ScreenObjects {
private:
	MadsView &_owner;
	Common::Array<ScreenObjectEntry> _entries;
public:
	int _v832EC;
	int _v7FECA;
	int _v7FED6;
	int _v8332A;
	int _yp;
	int _v8333C;
	int _selectedObject;
	int _category;
	int _objectIndex;

	ScreenObjects(MadsView &owner);
	ScreenObjectEntry &operator[](uint idx) {
		assert(idx <= _entries.size());
		return _entries[idx - 1];
	}

	void clear();
	void add(const Common::Rect &bounds, int layer, int idx, int category);
	void draw(View *view);
	int scan(int xp, int yp, int layer);
	int scanBackwards(int xp, int yp, int layer);
	void setActive(int category, int idx, bool active);
	void check(bool scanFlag, bool mouseClick);
};

class DynamicHotspot {
public:
	bool active;
	int seqIndex;
	Common::Rect bounds;
	Common::Point pos;
	int facing;
	int descId;
	int field_14;
	int articleNumber;
	int field_17;

	DynamicHotspot() { active = false; }
};

#define DYNAMIC_HOTSPOTS_SIZE 8

class MadsDynamicHotspots {
private:
	MadsView &_owner;
	Common::Array<DynamicHotspot> _entries;
	int _count;
public:
	bool _changed;
public:
	MadsDynamicHotspots(MadsView &owner);

	DynamicHotspot &operator[](uint idx) { return _entries[idx]; }
	int add(int descId, int field14, int seqIndex, const Common::Rect &bounds);
	int setPosition(int index, int xp, int yp, int facing);
	int set17(int index, int v);
	void remove(int index);
	void reset();
	void refresh() {
		// TODO
	}
};

class MadsDirtyArea {
public:
	Common::Rect bounds;
	Common::Rect bounds2;
	bool textActive;
	bool active;

	MadsDirtyArea() { active = false; }
	void setArea(int width, int height, int maxWidth, int maxHeight);
};

#define DIRTY_AREAS_SIZE 90
#define DIRTY_AREAS_TEXT_DISPLAY_IDX 50

class MadsDirtyAreas {
private:
	MadsView &_owner;
	Common::Array<MadsDirtyArea> _entries;
public:
	MadsDirtyAreas(MadsView &owner);

	MadsDirtyArea &operator[](uint idx) {
		assert(idx < _entries.size());
		return _entries[idx];
	}

	void setSpriteSlot(int dirtyIdx, const MadsSpriteSlot &spriteSlot);
	void setTextDisplay(int dirtyIdx, const MadsTextDisplayEntry &textDisplay);
	void merge(int startIndex, int count);
	bool intersects(int idx1, int idx2);
	void mergeAreas(int idx1, int idx2);
	void copy(M4Surface *dest, M4Surface *src, const Common::Point &posAdjust);
	void clear();
};

enum SpriteAnimType {ANIMTYPE_CYCLED = 1, ANIMTYPE_REVERSIBLE = 2};

enum SequenceSubEntryMode {SM_0 = 0, SM_1 = 1, SM_FRAME_INDEX = 2};

#define TIMER_ENTRY_SUBSET_MAX 5

struct MadsSequenceSubEntries {
	int count;
	SequenceSubEntryMode mode[TIMER_ENTRY_SUBSET_MAX];
	int16 frameIndex[TIMER_ENTRY_SUBSET_MAX];
	int8 abortVal[TIMER_ENTRY_SUBSET_MAX];
};

struct MadsSequenceEntry {
	int8 active;
	int8 spriteListIndex;
	bool flipped;

	int frameIndex;
	int frameStart;
	int numSprites;

	SpriteAnimType animType;
	int frameInc;

	int depth;
	int scale;
	int dynamicHotspotIndex;

	bool nonFixed;
	int field_13;

	Common::Point msgPos;
	int triggerCountdown;
	bool doneFlag;
	MadsSequenceSubEntries entries;
	AbortTimerMode abortMode;

	uint16 actionNouns[3];
	int numTicks;
	int extraTicks;
	uint32 timeout;
};

#define TIMER_LIST_SIZE 30

class MadsSequenceList {
private:
	MadsView &_owner;
	Common::Array<MadsSequenceEntry> _entries;
public:
	MadsSequenceList(MadsView &owner);

	MadsSequenceEntry &operator[](int index) { return _entries[index]; }
	void clear();
	bool addSubEntry(int index, SequenceSubEntryMode mode, int frameIndex, int abortVal);
	int add(int spriteListIndex, bool flipped, int frameIndex, int triggerCountdown, int delayTicks,
		int extraTicks, int numTicks, int msgX, int msgY, bool nonFixed, char scale, uint8 depth,
		int frameInc, SpriteAnimType animType, int numSprites, int frameStart);
	void remove(int seqIndex);
	void setSpriteSlot(int seqIndex, MadsSpriteSlot &spriteSlot);
	bool loadSprites(int seqIndex);
	void tick();
	void delay(uint32 v1, uint32 v2);
	void setAnimRange(int seqIndex, int startVal, int endVal);
	void scan();
	void setDepth(int seqIndex, int depth);
};

class Animation {
protected:
	MadsM4Engine *_vm;
public:
	Animation(MadsM4Engine *vm);
	virtual ~Animation();
	virtual void initialize(const Common::String &filename, uint16 flags, M4Surface *surface, M4Surface *depthSurface) = 0;
	virtual void load(const Common::String &filename, int v0) = 0;
	virtual void update() = 0;
	virtual void setCurrentFrame(int frameNumber) = 0;
	virtual int getCurrentFrame() = 0;
};


class MadsView {
private:
	View *_view;
public:
	Animation *_sceneAnimation;
	MadsSpriteSlots _spriteSlots;
	MadsTextDisplay _textDisplay;
	MadsKernelMessageList _kernelMessages;
	ScreenObjects _screenObjects;
	MadsDynamicHotspots _dynamicHotspots;
	MadsSequenceList _sequenceList;
	MadsDirtyAreas _dirtyAreas;
	MadsAction _action;

	int _textSpacing;
	uint32 _newTimeout;
	int _abortTimers;
	int8 _abortTimers2;
	AbortTimerMode _abortTimersMode;
	AbortTimerMode _abortTimersMode2;
	Common::Point _posAdjust;

	M4Surface *_depthSurface;
	M4Surface *_bgSurface;
	M4Surface *_viewport;
public:
	MadsView(View *view);
	~MadsView();

	void refresh();
	void update();
	void clearLists();
	void setViewport(const Common::Rect &bounds);
};

}

#endif
