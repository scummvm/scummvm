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
 * $URL$
 * $Id$
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

enum AbortTimerMode {ABORTMODE_0 = 0, ABORTMODE_1 = 1, ABORTMODE_2 = 2};

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
	int addSprites(const char *resName);
	void deleteSprites(int listIndex);
	void clear();
	void deleteTimer(int seqIndex);

	void drawBackground();
	void drawForeground(View *view, int yOffset);
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
	uint8 colour1;
	uint8 colour2;
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

	int add(int xp, int yp, uint fontColour, int charSpacing, const char *msg, Font *font);
	void clear();
	void draw(View *view, int yOffset);
	void setDirtyAreas();
	void setDirtyAreas2();
	void cleanUp();
};

#define TIMED_TEXT_SIZE 10
#define INDEFINITE_TIMEOUT 9999999

enum KernelMessageFlags {KMSG_QUOTED = 1, KMSG_OWNER_TIMEOUT = 2, KMSG_SEQ_ENTRY = 4, KMSG_SCROLL = 8, KMSG_RIGHT_ALIGN = 0x10, 
	KMSG_CENTER_ALIGN = 0x20, KMSG_EXPIRE = 0x40, KMSG_ACTIVE = 0x80};

class MadsKernelMessageEntry {
public:
	uint8 flags;
	int sequenceIndex;
	char asciiChar;
	char asciiChar2;
	int colour1;
	int colour2;
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
	int add(const Common::Point &pt, uint fontColour, uint8 flags, uint8 abortTimers, uint32 timeout, const char *msg);
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
	Common::Array<ScreenObjectEntry> _entries;
public:
	ScreenObjects() {}

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
	bool _flag;
public:
	MadsDynamicHotspots(MadsView &owner);

	DynamicHotspot &operator[](uint idx) { return _entries[idx]; }
	int add(int descId, int field14, int seqIndex, const Common::Rect &bounds);
	int setPosition(int index, int xp, int yp, int facing);
	int set17(int index, int v);
	void remove(int index);
	void reset();
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
	void copy(M4Surface *dest, M4Surface *src, int yOffset);
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
	
	int field_2;
	
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
	int add(int spriteListIndex, int v0, int v1, int triggerCountdown, int delayTicks, int extraTicks, int numTicks, 
		int msgX, int msgY, bool nonFixed, char scale, uint8 depth, int frameInc, SpriteAnimType animType, 
		int numSprites, int frameStart);
	void remove(int seqIndex);
	void setSpriteSlot(int seqIndex, MadsSpriteSlot &spriteSlot);
	bool loadSprites(int seqIndex);
	void tick();
	void delay(uint32 v1, uint32 v2);
	void setAnimRange(int seqIndex, int startVal, int endVal);
	void scan();
};

class Animation {
protected:
	MadsM4Engine *_vm;
public:
	Animation(MadsM4Engine *vm);
	virtual ~Animation();
	virtual void initialise(const Common::String &filename, uint16 flags, M4Surface *walkSurface, M4Surface *sceneSurface) = 0;
	virtual void load(const Common::String &filename, int v0) = 0;
	virtual void update() = 0;
	virtual void setCurrentFrame(int frameNumber) = 0;
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

	int _textSpacing;
	int _ticksAmount;
	uint32 _newTimeout;
	int _abortTimers;
	int8 _abortTimers2;
	AbortTimerMode _abortTimersMode;
	AbortTimerMode _abortTimersMode2;
	Common::Point _posAdjust;

	M4Surface *_depthSurface;
	M4Surface *_bgSurface;
	int _yOffset;
public:
	MadsView(View *view);
	~MadsView();

	void refresh();
	void clearLists();
};

}

#endif
