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

#ifndef M4_SCENE_H
#define M4_SCENE_H

class View;

#include "m4/assets.h"
#include "m4/hotspot.h"
#include "m4/graphics.h"
#include "m4/viewmgr.h"

namespace M4 {

#define TITLE_SCENE_BURGER 951		// 951 = intro, 901 = demo menu, 971 = first scene
#define MAINMENU_SCENE_BURGER 903
#define FIRST_SCENE 101
#define MAX_CHK_FILENAME_SIZE 144

#define INTERFACE_HEIGHT 106
#define MADS_SURFACE_HEIGHT 156

#define CHEAT_SEQUENCE_MAX 8

enum MADSVerbs {
	kVerbNone   = 0,
	kVerbLook	= 3,
	kVerbTake	= 4,
	kVerbPush	= 5,
	kVerbOpen	= 6,
	kVerbPut	= 7,
	kVerbTalkTo	= 8,
	kVerbGive	= 9,
	kVerbPull	= 10,
	kVerbClose	= 11,
	kVerbThrow	= 12,
	kVerbWalkTo	= 13
};

struct SceneResources {
	char artBase[MAX_CHK_FILENAME_SIZE];
	char pictureBase[MAX_CHK_FILENAME_SIZE];
	int32 hotspotCount;
	HotSpotList *hotspots;
	int32 parallaxCount;
	HotSpotList *parallax;
	int32 propsCount;
	HotSpotList *props;
	int32 frontY, backY;
	int32 frontScale, backScale;
	int16 depthTable[16];
	int32 railNodeCount;	// # of rails
};

class M4Engine;
class MadsEngine;
class InterfaceView;

class Scene: public View {
private:
	byte *_inverseColorTable;
	HotSpotList _sceneHotspots;
	char _statusText[100];
protected:
	int _currentScene;
	InterfaceView *_interfaceSurface;
	M4Surface *_backgroundSurface;
	M4Surface *_codeSurface;
	RGBList *_palData;
	RGBList *_interfacePal;
	SceneResources _sceneResources;
public:
	Scene(MadsM4Engine *vm);
	virtual ~Scene();

	// Methods that differ between engines
	virtual void loadScene(int sceneNumber);
	virtual void loadSceneCodes(int sceneNumber, int index = 0) = 0;
	virtual void show();
	virtual void checkHotspotAtMousePos(int x, int y) = 0;
	virtual void leftClick(int x, int y) = 0;
	virtual void rightClick(int x, int y) = 0;
	virtual void setAction(int action, int objectId = -1) = 0;

	// TODO: perhaps move playIntro() someplace else?
	void playIntro();
	void loadSceneResources(int sceneNumber);
	void loadSceneHotSpotsMads(int sceneNumber);
	void loadSceneInverseColorTable(int sceneNumber);
	void loadSceneSpriteCodes(int sceneNumber);
	void showSprites();
	void showHotSpots();
	void showCodes();
	int getCurrentScene() { return _currentScene; }
	M4Surface *getBackgroundSurface() const { return _backgroundSurface; }
	byte *getInverseColorTable() const { return _inverseColorTable; }
	void update();
	void setMADSStatusText(const char *text) { strcpy(_statusText, text); }
	void showMADSV2TextBox(char *text, int x, int y, char *faceName);
	InterfaceView *getInterface() { return _interfaceSurface; }
	SceneResources getSceneResources() { return _sceneResources; }

	void onRefresh(RectList *rects, M4Surface *destSurface);
	bool onEvent(M4EventType eventType, int32 param1, int x, int y, bool &captureEvents);
};

class M4Scene: public Scene {
private:
	M4Engine *_vm;
	SpriteAsset *_sceneSprites;
	SpriteAsset *_walkerSprite;

	void loadSceneSprites(int sceneNumber);
	void nextCommonCursor();
public:
	M4Scene(M4Engine *vm);
	virtual ~M4Scene();

	// Methods that differ between engines
	virtual void loadScene(int sceneNumber);
	virtual void loadSceneCodes(int sceneNumber, int index = 0);
	virtual void checkHotspotAtMousePos(int x, int y);
	virtual void leftClick(int x, int y);
	virtual void rightClick(int x, int y);
	virtual void setAction(int action, int objectId = -1);
};

class MadsScene: public Scene {
private:
	MadsEngine *_vm;

	int _currentAction;
public:
	MadsScene(MadsEngine *vm);
	virtual ~MadsScene() {};

	// Methods that differ between engines
	virtual void loadScene(int sceneNumber);
	virtual void loadSceneCodes(int sceneNumber, int index = 0);
	virtual void show();
	virtual void checkHotspotAtMousePos(int x, int y);
	virtual void leftClick(int x, int y);
	virtual void rightClick(int x, int y);
	virtual void setAction(int action, int objectId = -1);
};

enum InterfaceFontMode {ITEM_NORMAL, ITEM_HIGHLIGHTED, ITEM_SELECTED};

enum InterfaceObjects {ACTIONS_START = 0, SCROLL_UP = 10, SCROLL_SCROLLER = 11, SCROLL_DOWN = 12,
		INVLIST_START = 13, VOCAB_START = 18};

class IntegerList: public Common::Array<int> {
public:
	int indexOf(int v) {
		for (uint i = 0; i < size(); ++i)
			if (operator [](i) == v)
				return i;
		return -1;
	}
};

class InterfaceView: public View {
public:
	InterfaceView(MadsM4Engine *vm, const Common::Rect &bounds): View(vm, bounds) {};
	~InterfaceView() {};

	virtual void initialise() {};
	virtual void setSelectedObject(int objectNumber) {};
	virtual void addObjectToInventory(int objectNumber) {};
};

class MadsInterfaceView: public InterfaceView {
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
public:
	MadsInterfaceView(MadsM4Engine *vm);
	~MadsInterfaceView();

	virtual void initialise();
	virtual void setSelectedObject(int objectNumber);
	virtual void addObjectToInventory(int objectNumber);

	void onRefresh(RectList *rects, M4Surface *destSurface);
	bool onEvent(M4EventType eventType, int32 param1, int x, int y, bool &captureEvents);
};

} // End of namespace M4

#endif
