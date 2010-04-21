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

#ifndef M4_MADS_SCENE_H
#define M4_MADS_SCENE_H

#include "m4/scene.h"
#include "m4/mads_logic.h"
#include "m4/mads_views.h"

namespace M4 {

#define INTERFACE_HEIGHT 106

struct SpriteSlot {
	int16 spriteId;
	int16 scale;
	uint16 spriteListIndex;
};

struct DirtyArea {
	bool active;
	bool active2;
	Common::Rect bounds;
};


class MadsSceneResources: public SceneResources {
public:
	int sceneId;
	int artFileNum;
	int field_4;
	int width;
	int height;

	int objectCount;
	MadsObject objects[32];
	
	int walkSize;
	byte *walkData;

	MadsSceneResources() { walkSize = 0; walkData = NULL; }
	~MadsSceneResources() { delete walkData; }
	void load(int sceneId);	
};

#define TIMED_TEXT_SIZE 10
#define TEXT_4A_SIZE 30

enum TalkTextFlags {TEXTFLAG_2 = 2, TEXTFLAG_4 = 4, TEXTFLAG_8 = 8, TEXTFLAG_40 = 0x40,
		TEXTFLAG_ACTIVE = 0x80};

struct TimedText {
	uint8 flags;
	int colour1;
	int colour2;
	Common::Point position;
	int textDisplayIndex;
	int timerIndex;
	uint32 timeout;
	uint32 frameTimer;
	bool field_1C;
	uint8 field_1D;
	uint16 actionNouns[3];
	char message[100];
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
	int walkObjectIndex;

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
	int32 timeout;
};

#define TIMER_LIST_SIZE 30

class MadsTimerList {
private:
	Common::Array<MadsTimerEntry> _entries;
public:
	MadsTimerList();

	MadsTimerEntry &operator[](int index) { return _entries[index]; }	
	bool unk2(int index, int v1, int v2, int v3);
	int add(int spriteListIndex, int v0, int v1, char field_24, int timeoutTicks, int extraTicks, int numTicks, 
		int height, int width, char field_12, char scale, char depth, int field_C, int field_A, 
		int numSprites, int spriteNum);
};

enum MadsActionMode {ACTMODE_NONE = 0, ACTMODE_VERB = 1, ACTMODE_OBJECT = 3, ACTMODE_TALK = 6};
enum MAdsActionMode2 {ACTMODE2_0 = 0, ACTMODE2_2 = 2, ACTMODE2_5 = 5};

class MadsAction {
private:
	char _statusText[100];

	void appendVocab(int vocabId, bool capitalise = false);
public:
	int _currentHotspot;
	int _objectNameId;
	int _objectDescId;
	int _currentAction;
	int8 _flags1, _flags2;
	MadsActionMode _actionMode;
	MAdsActionMode2 _actionMode2;
	int _articleNumber;
	bool _lookFlag;
	int _selectedRow;
	// Unknown fields
	int16 _word_86F3A;
	int16 _word_86F42;
	int16 _word_86F4E;
	int16 _word_86F4A;
	int16 _word_83334;
	int16 _word_86F4C;

public:
	MadsAction();

	void clear();
	void set();
	const char *statusText() const { return _statusText; }
};

#define DIRTY_AREA_SIZE 90

class MadsScene : public Scene, public MadsView {
private:
	MadsEngine *_vm;
	MadsSceneResources _sceneResources;
	MadsAction _action;

	MadsSceneLogic _sceneLogic;
	SpriteAsset *_playerSprites;
	DirtyArea _dirtyAreas[DIRTY_AREA_SIZE];

	void drawElements();
	void loadScene2(const char *aaName);
	void loadSceneTemporary();
	void loadSceneHotspots(int sceneNumber);
	void clearAction();
	void appendActionVocab(int vocabId, bool capitalise);
	void setAction();
public:
	char _aaName[100];
	uint16 actionNouns[3];
	MadsTimerList _timerList;
public:
	MadsScene(MadsEngine *vm);
	virtual ~MadsScene();

	// Methods that differ between engines
	virtual void loadScene(int sceneNumber);
	virtual void leaveScene();
	virtual void loadSceneCodes(int sceneNumber, int index = 0);
	virtual void show();
	virtual void checkHotspotAtMousePos(int x, int y);
	virtual void leftClick(int x, int y);
	virtual void rightClick(int x, int y);
	virtual void setAction(int action, int objectId = -1);
	virtual void update();

	int loadSceneSpriteSet(const char *setName);
	void loadPlayerSprites(const char *prefix);
	void showMADSV2TextBox(char *text, int x, int y, char *faceName);

	MadsInterfaceView *getInterface() { return (MadsInterfaceView *)_interfaceSurface; }
	MadsSceneResources &getSceneResources() { return _sceneResources; }
	MadsAction &getAction() { return _action; }
	void setStatusText(const char *text) {}//***DEPRECATED***
};

} // End of namespace M4

#endif
