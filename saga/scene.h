/* ScummVM - Scumm Interpreter
 * Copyright (C) 2004-2005 The ScummVM project
 *
 * The ReInherit Engine is (C)2000-2003 by Daniel Balsom.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

// Scene management module private header file

#ifndef SAGA_SCENE_H
#define SAGA_SCENE_H

#include "saga/font.h"
#include "saga/list.h"
#include "saga/actor.h"

namespace Saga {

#define SCENE_DOORS_MAX 16

class ObjectMap;

struct EVENT;

enum SceneFlags {
	kSceneFlagISO        = 1,
	kSceneFlagShowCursor = 2
};

struct BGInfo {
	Rect bounds;
	byte *buffer;
	size_t bufferLength;
};

typedef int (SceneProc) (int, void *);


enum SCENE_PROC_PARAMS {
	SCENE_BEGIN = 0,
	SCENE_END
};

// Resource type numbers
enum SAGAResourceTypes {
	SAGA_BG_IMAGE = 2,
	SAGA_BG_MASK = 3,
	SAGA_STRINGS = 5,
	SAGA_OBJECT_MAP = 6,
	SAGA_ACTION_MAP = 7,
	SAGA_ISO_IMAGES = 8,
	SAGA_ISO_MAP = 9,
	SAGA_ISO_PLATFORMS = 10,
	SAGA_ISO_METATILES = 11,
	SAGA_ENTRY = 12,
	SAGA_ANIM_1 = 14,
	SAGA_ANIM_2,
	SAGA_ANIM_3,
	SAGA_ANIM_4,
	SAGA_ANIM_5,
	SAGA_ANIM_6,
	SAGA_ANIM_7,
	SAGA_ISO_MULTI = 22,
	SAGA_PAL_ANIM = 23,
	SAGA_FACES = 24,
	SAGA_PALETTE = 25
};

#define SAGA_RESLIST_ENTRY_LEN 4

struct SCENE_RESLIST {
	uint32 res_number;
	int res_type;
	byte *res_data;
	size_t res_data_len;
};

#define SAGA_SCENE_DESC_LEN 16

struct SceneDescription {
	int16 flags;
	int16 resListRN;
	int16 endSlope;
	int16 beginSlope;
	uint16 scriptModuleNumber;
	uint16 sceneScriptEntrypointNumber;
	uint16 startScriptEntrypointNumber;
	int16 musicRN;
	SCENE_RESLIST *resList;
	size_t resListCnt;
};

struct SceneEntry {
	Location location;
	int facing;
};

struct SceneEntryList {
	SceneEntry *entryList;
	int entryListCount;

	const SceneEntry * getEntry(int index) {
		if ((index < 0) || (index >= entryListCount)) {
			error("SceneEntryList::getEntry wrong index (%d)", index);
		}
		return &entryList[index];
	}
	void freeMem() {
		free(entryList);
		memset(this, 0, sizeof(*this));
	}
	SceneEntryList() {
		memset(this, 0, sizeof(*this));
	}
	~SceneEntryList() {
		freeMem();
	}
};

struct SceneImage {
	int loaded;
	int w;
	int h;
	int p;
	byte *buf;
	size_t buf_len;
	byte *res_buf;
	size_t res_len;
	PalEntry pal[256];
};


enum SceneTransitionType {
	kTransitionNoFade,
	kTransitionFade
};

enum SceneLoadFlags {
	kLoadByResourceId,
	kLoadBySceneNumber,
	kLoadByDescription
};

struct LoadSceneParams {
	uint32 sceneDescriptor;
	SceneLoadFlags loadFlag;
	SceneDescription* sceneDescription;
	SceneProc *sceneProc;
	bool sceneSkipTarget;
	SceneTransitionType transitionType;
	int actorsEntrance;
};

typedef Common::List<LoadSceneParams> SceneQueueList;

///// IHNM-specific stuff
#define IHNM_PALFADE_TIME    1000
#define IHNM_INTRO_FRAMETIME 80
#define IHNM_DGLOGO_TIME     8000
#define IHNM_TITLE_TIME_GM   28750
#define IHNM_TITLE_TIME_FM   19500

///// ITE-specific stuff
#define ITE_INTRO_FRAMETIME 90

#define INTRO_CAPTION_Y 170
#define INTRO_DE_CAPTION_Y 160
#define VOICE_PAD 50
#define VOICE_LETTERLEN 90

#define PALETTE_FADE_DURATION 1000
#define DISSOLVE_DURATION 3000
#define LOGO_DISSOLVE_DURATION 1000

#define CREDIT_DURATION1 4000

struct INTRO_DIALOGUE {
	uint32 i_voice_rn;
	const char *i_str;
};

struct INTRO_CREDIT {
	Common::Language lang;
	int game;
	int type;
	const char *string;
};


class Scene {
 public:
	Scene(SagaEngine *vm);
	~Scene();

// Console functions
	void cmdSceneInfo();
	void cmdActionMapInfo();
	void cmdObjectMapInfo();

	void cmdSceneChange(int argc, const char **argv);

	void startScene();
	void nextScene();
	void skipScene();
	void endScene();
	void queueScene(LoadSceneParams *sceneQueue) {
		_sceneQueue.push_back(*sceneQueue);
	}

	void draw();
	int getFlags() const { return _sceneDescription.flags; }
	int getScriptModuleNumber() const { return _sceneDescription.scriptModuleNumber; }
	bool isInDemo() { return !_inGame; }
	const Rect& getSceneClip() const { return _sceneClip; }

	void getBGMaskInfo(int &width, int &height, byte *&buffer, size_t &bufferLength);
	int isBGMaskPresent() { return _bgMask.loaded; }
	int getBGMaskType(const Point &testPoint);
	bool validBGMaskPoint(const Point &testPoint);
	bool canWalk(const Point &testPoint);
	bool offscreenPath(Point &testPoint);

	void setDoorState(int doorNumber, int doorState);
	int getDoorState(int doorNumber);
	void initDoorsState();

	void getBGInfo(BGInfo &bgInfo);
	int getBGPal(PalEntry **pal);
	void getSlopes(int &beginSlope, int &endSlope);

	void clearSceneQueue(void) {
		_sceneQueue.clear();
	}
	void changeScene(uint16 sceneNumber, int actorsEntrance, SceneTransitionType transitionType);

	bool initialized() const { return _initialized; }
	bool isSceneLoaded() const { return _sceneLoaded; }


	int getSceneResourceId(int sceneNumber) {
		if ((sceneNumber < 0) || (sceneNumber >= _sceneCount)) {
 			error("getSceneResourceId: wrong sceneNumber %i", sceneNumber);
		}
		return _sceneLUT[sceneNumber];
	}
	int currentSceneNumber() const { return _sceneNumber; }
	int getOutsetSceneNumber() const { return _outsetSceneNumber; }
	int currentSceneResourceId() const { return _sceneResourceId; }

	void drawTextList(Surface *ds);
 private:
	void loadScene(LoadSceneParams *loadSceneParams);
	int loadSceneDescriptor(uint32 res_number);
	int loadSceneResourceList(uint32 res_number);
	void loadSceneEntryList(const byte* resourcePointer, size_t resourceLength);
	int processSceneResources();

 
	SagaEngine *_vm;
	bool _initialized;

	RSCFILE_CONTEXT *_sceneContext;
	int *_sceneLUT;
	int _sceneCount;
	SceneQueueList _sceneQueue;
	int _firstScene;
	bool _sceneLoaded;
	int _sceneNumber;
	int _outsetSceneNumber;
	int _sceneResourceId;
	bool _inGame;
	bool _loadDescription;
	SceneDescription _sceneDescription;
	int _resListEntries;
	SCENE_RESLIST *_resList;
	SceneProc *_sceneProc;
	SceneImage _bg;
	SceneImage _bgMask;
	Common::Rect _sceneClip;

	int _sceneDoors[SCENE_DOORS_MAX];


 public:
	ObjectMap *_actionMap;
	ObjectMap *_objectMap;
	SceneEntryList _entryList;
	StringsTable _sceneStrings;
	TextList _textList;

 private:
	int IHNMStartProc();
	int ITEStartProc();

 public:
	static int SC_IHNMIntroMovieProc1(int param, void *refCon);
	static int SC_IHNMIntroMovieProc2(int param, void *refCon);
	static int SC_IHNMIntroMovieProc3(int param, void *refCon);
	static int SC_IHNMHateProc(int param, void *refCon);

 private:
	int IHNMIntroMovieProc1(int param);
	int IHNMIntroMovieProc2(int param);
	int IHNMIntroMovieProc3(int param);
	int IHNMHateProc(int param);

 public:
	static int SC_ITEIntroAnimProc(int param, void *refCon);
	static int SC_ITEIntroCave1Proc(int param, void *refCon);
	static int SC_ITEIntroCave2Proc(int param, void *refCon);
	static int SC_ITEIntroCave3Proc(int param, void *refCon);
	static int SC_ITEIntroCave4Proc(int param, void *refCon);
	static int SC_ITEIntroValleyProc(int param, void *refCon);
	static int SC_ITEIntroTreeHouseProc(int param, void *refCon);
	static int SC_ITEIntroFairePathProc(int param, void *refCon);
	static int SC_ITEIntroFaireTentProc(int param, void *refCon);

 private:
	EVENT *ITEQueueDialogue(EVENT *q_event, int n_dialogues, const INTRO_DIALOGUE dialogue[]);
	EVENT *ITEQueueCredits(int delta_time, int duration, int n_credits, const INTRO_CREDIT credits[]);
	int ITEIntroAnimProc(int param);
	int ITEIntroCave1Proc(int param);
	int ITEIntroCave2Proc(int param);
	int ITEIntroCave3Proc(int param);
	int ITEIntroCave4Proc(int param);
	int ITEIntroValleyProc(int param);
	int ITEIntroTreeHouseProc(int param);
	int ITEIntroFairePathProc(int param);
	int ITEIntroFaireTentProc(int param);

};

} // End of namespace Saga

#endif
