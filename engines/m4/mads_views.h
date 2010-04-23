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

#define MADS_SURFACE_HEIGHT 156
#define MADS_SCREEN_HEIGHT 200
#define MADS_Y_OFFSET ((MADS_SCREEN_HEIGHT - MADS_SURFACE_HEIGHT) / 2)

class MadsView;

class MadsSpriteSlot {
public:
	int spriteId;
	int timerIndex;
	int spriteListIndex;
	int frameNumber;
	int xp;
	int yp;
	int depth;
	int scale;

	MadsSpriteSlot() { }
};

#define SPRITE_SLOTS_SIZE 50

typedef Common::Array<Common::SharedPtr<SpriteAsset> > SpriteList;

class MadsSpriteSlots {
private:
	MadsSpriteSlot _entries[SPRITE_SLOTS_SIZE];
	SpriteList _sprites;
public:
	int startIndex;

	MadsSpriteSlots() { startIndex = 0; }

	MadsSpriteSlot &operator[](int idx) {
		assert(idx < SPRITE_SLOTS_SIZE);
		return _entries[idx];
	}
	SpriteAsset &getSprite(int idx) {
		assert(idx < (int)_sprites.size());
		return *_sprites[idx].get();
	}

	int getIndex();
	int addSprites(const char *resName);
	void clear() {
		startIndex = 0;
		_sprites.clear();
	}
	void deleteTimer(int timerIndex);

	void draw(View *view);
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
	Common::Array<MadsTextDisplayEntry> _entries;
public:
	MadsTextDisplay();

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
	void draw(View *view);
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
	int timerIndex;
	Common::Rect bounds;
	Common::Point pos;
	int facing;
	int descId;
	int field_14;
	int articleNumber;
	int field_17;

	DynamicHotspot() { active = false; }
};

#define HITBOX_AREA_SIZE 8

class DynamicHotspots {
private:
	MadsView &_owner;
	Common::Array<DynamicHotspot> _entries;
	bool _flag;
	int _count;
public:
	DynamicHotspots(MadsView &owner);

	DynamicHotspot &operator[](uint idx) { return _entries[idx]; }
	int add(int descId, int field14, int timerIndex, const Common::Rect &bounds);
	int setPosition(int index, int xp, int yp, int facing);
	int set17(int index, int v);
	void remove(int index);
	void reset();
};

#define TIMER_ENTRY_SUBSET_MAX 5

struct MadsTimerEntry {
	int8 active;
	int8 spriteListIndex;
	
	int field_2;
	
	int frameIndex;
	int spriteNum;
	int numSprites;
	
	int field_A;
	int field_C;

	int depth;
	int scale;
	int dynamicHotspotIndex;

	int field_12;
	int field_13;
	
	int width;
	int height;
	
	int field_24;
	int field_25;
	int len27;
	int8 fld27[TIMER_ENTRY_SUBSET_MAX];
	int16 fld2C[TIMER_ENTRY_SUBSET_MAX];
	int8 field36;
	int field_3B;

	uint16 actionNouns[3];
	int numTicks;
	int extraTicks;
	uint32 timeout;
};

#define TIMER_LIST_SIZE 30

class MadsTimerList {
private:
	MadsView &_owner;
	Common::Array<MadsTimerEntry> _entries;
public:
	MadsTimerList(MadsView &owner);

	MadsTimerEntry &operator[](int index) { return _entries[index]; }	
	bool unk2(int index, int v1, int v2, int v3);
	int add(int spriteListIndex, int v0, int v1, char field_24, int timeoutTicks, int extraTicks, int numTicks, 
		int height, int width, char field_12, char scale, char depth, int field_C, int field_A, 
		int numSprites, int spriteNum);
	void remove(int timerIndex);
	void setSpriteSlot(int timerIndex, MadsSpriteSlot &spriteSlot);
	bool loadSprites(int timerIndex);
	void tick();
};

class MadsView {
private:
	View *_view;
public:
	MadsSpriteSlots _spriteSlots;
	MadsTextDisplay _textDisplay;
	ScreenObjects _screenObjects;
	DynamicHotspots _dynamicHotspots;
	MadsTimerList _timerList;

	int _abortTimers;
	int8 _abortTimers2;
public:
	MadsView(View *view);

	void refresh(RectList *rects);
};

#define CHEAT_SEQUENCE_MAX 8

class IntegerList : public Common::Array<int> {
public:
	int indexOf(int v) {
		for (uint i = 0; i < size(); ++i)
			if (operator [](i) == v)
				return i;
		return -1;
	}
};

enum InterfaceFontMode {ITEM_NORMAL, ITEM_HIGHLIGHTED, ITEM_SELECTED};

enum InterfaceObjects {ACTIONS_START = 0, SCROLL_UP = 10, SCROLL_SCROLLER = 11, SCROLL_DOWN = 12,
		INVLIST_START = 13, VOCAB_START = 18};

class MadsInterfaceView : public GameInterfaceView {
private:
	IntegerList _inventoryList;
	RectList _screenObjects;
	int _highlightedElement;
	int _topIndex;
	uint32 _nextScrollerTicks;
	int _cheatKeyCtr;

	// Object display fields
	int _selectedObject;
	SpriteAsset *_objectSprites;
	RGBList *_objectPalData;
	int _objectFrameNumber;

	void setFontMode(InterfaceFontMode newMode);
	bool handleCheatKey(int32 keycode);
	bool handleKeypress(int32 keycode);
	void leaveScene();
public:
	MadsInterfaceView(MadsM4Engine *vm);
	~MadsInterfaceView();

	virtual void initialise();
	virtual void setSelectedObject(int objectNumber);
	virtual void addObjectToInventory(int objectNumber);
	int getSelectedObject() { return _selectedObject; }
	int getInventoryObject(int objectIndex) { return _inventoryList[objectIndex]; }

	void onRefresh(RectList *rects, M4Surface *destSurface);
	bool onEvent(M4EventType eventType, int32 param1, int x, int y, bool &captureEvents);
};

}

#endif
