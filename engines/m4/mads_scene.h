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
	Common::Point playerPos;
	int playerDir;

	MadsSceneResources() { walkSize = 0; walkData = NULL; playerDir = 0; }
	~MadsSceneResources() { delete walkData; }
	void load(int sceneId);	
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

class MadsScene : public Scene, public MadsView {
private:
	MadsEngine *_vm;
	MadsSceneResources _sceneResources;
	MadsAction _action;

	MadsSceneLogic _sceneLogic;
	SpriteAsset *_playerSprites;

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

	virtual void updateState();

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
