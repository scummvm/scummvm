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

#ifndef M4_MADS_SCENE_H
#define M4_MADS_SCENE_H

#include "m4/scene.h"
#include "m4/mads_logic.h"
#include "m4/mads_views.h"

namespace M4 {

#define INTERFACE_HEIGHT 106
class MadsInterfaceView;

#define DEPTH_BANDS_SIZE 15
#define MAX_ROUTE_NODES 22

enum ScreenCategory {CAT_NONE = 0, CAT_ACTION = 1, CAT_INV_LIST = 2, CAT_INV_VOCAB, CAT_HOTSPOT = 4,
	CAT_INV_ANIM = 6, CAT_6, CAT_INV_SCROLLER = 7, CAT_12 = 12};

class SceneNode {
public:
	Common::Point pt;
	int indexes[MAX_ROUTE_NODES];

	bool active;

	SceneNode() {
		active = false;
	}

	void load(Common::SeekableReadStream *stream);
};

typedef Common::Array<SceneNode> SceneNodeList;

class MadsSceneResources: public SceneResources {
private:
	int getRouteFlags(const Common::Point &src, const Common::Point &dest, M4Surface *depthSurface);
public:
	int _sceneId;
	int _artFileNum;
	int _depthStyle;
	int _width;
	int _height;
	SceneNodeList _nodes;
	Common::Array<Common::String> _setNames;
	int _yBandsStart, _yBandsEnd;
	int _maxScale, _minScale;
	int _depthBands[DEPTH_BANDS_SIZE];

	MadsSceneResources() {}
	~MadsSceneResources() {}
	void load(int sceneId, const char *resName, int v0, M4Surface *depthSurface, M4Surface *surface);
	int bandsRange() const { return _yBandsEnd - _yBandsStart; }
	int scaleRange() const { return _maxScale - _minScale; }
	void setRouteNode(int nodeIndex, const Common::Point &pt, M4Surface *depthSurface);
};

class MadsScene : public Scene, public MadsView {
private:
	MadsEngine *_vm;
	MadsSceneResources _sceneResources;
	Animation *_activeAnimation;

	MadsSceneLogic _sceneLogic;
	SpriteAsset *_playerSprites;
	int _mouseMsgIndex;
	int _highlightedHotspot;

	void drawElements();
	void loadScene2(const char *aaName, int sceneNumber);
	void loadSceneTemporary();
	void loadSceneHotspots(int sceneNumber);
	void clearAction();
	void appendActionVocab(int vocabId, bool capitalise);
	void setAction();
	void checkStartWalk();
	void doPreactions();
	void doSceneStep();
	void doAction();
public:
	char _aaName[100];
	bool _showMousePos;
	Common::Point _destPos;
	int _destFacing;
	Common::Point _customDest;
public:
	MadsScene(MadsEngine *vm);
	virtual ~MadsScene();
	void initialize() {
		_sceneLogic.initializeScripts();
	}

	// Methods that differ between engines
	virtual void loadScene(int sceneNumber);
	virtual void leaveScene();
	virtual void loadSceneCodes(int sceneNumber, int index = 0);
	virtual void show();
	virtual void mouseMove(int x, int y);
	virtual void leftClick(int x, int y);
	virtual void rightClick(int x, int y);
	virtual void setAction(int action, int objectId = -1);
	virtual void update();

	virtual void updateState();

	int loadSceneSpriteSet(const char *setName);
	void showMADSV2TextBox(char *text, int x, int y, char *faceName);
	void loadAnimation(const Common::String &animName, int abortTimers);
	Animation *activeAnimation() const { return _activeAnimation; }
	void freeAnimation();

	MadsInterfaceView *getInterface() { return (MadsInterfaceView *)_interfaceSurface; }
	MadsSceneResources &getSceneResources() { return _sceneResources; }
	bool getDepthHighBit(const Common::Point &pt);
	bool getDepthHighBits(const Common::Point &pt);
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

	virtual void initialize();
	virtual void setSelectedObject(int objectNumber);
	virtual void addObjectToInventory(int objectNumber);
	int getSelectedObject() { return _selectedObject; }
	int getInventoryObject(int objectIndex) { return _inventoryList[objectIndex]; }

	void onRefresh(RectList *rects, M4Surface *destSurface);
	bool onEvent(M4EventType eventType, int32 param1, int x, int y, bool &captureEvents);
};

extern int getActiveAnimationBool();
extern int getAnimationCurrentFrame();

} // End of namespace M4

#endif
