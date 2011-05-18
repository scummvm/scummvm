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

#ifndef M4_SCENE_H
#define M4_SCENE_H

class View;

#include "m4/assets.h"
#include "m4/font.h"
#include "m4/hotspot.h"
#include "m4/graphics.h"
#include "m4/viewmgr.h"
#include "m4/gui.h"
#include "m4/m4_views.h"
#include "common/array.h"

namespace M4 {

#define MAX_CHK_FILENAME_SIZE 144

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
	kVerbWalkTo	= 13,
	kVerbLookAt = 209
};

class SceneResources {
public:
	char artBase[MAX_CHK_FILENAME_SIZE];
	char pictureBase[MAX_CHK_FILENAME_SIZE];
	HotSpotList *hotspots;
	HotSpotList *dynamicHotspots;
	int32 frontY, backY;
	int32 frontScale, backScale;
	int16 depthTable[16];
	int32 railNodeCount;	// # of rails
};

class M4Engine;
class MadsEngine;
class InterfaceView;

class Scene : public View {
private:
	HotSpotList _sceneHotspots;
protected:
	GameInterfaceView *_interfaceSurface;
	M4Surface *_backgroundSurface;
	M4Surface *_walkSurface;
	RGBList *_palData;
	RGBList *_interfacePal;
	SceneResources *_sceneResources;
public:
	int _currentScene;
	int _previousScene;
	int _nextScene;
public:
	Scene(MadsM4Engine *vm, SceneResources *res);
	virtual ~Scene();

	// Methods that differ between engines
	virtual void loadScene(int sceneNumber);
	virtual void leaveScene();
	virtual void loadSceneCodes(int sceneNumber, int index = 0) = 0;
	virtual void show();
	virtual void mouseMove(int x, int y) = 0;
	virtual void leftClick(int x, int y) = 0;
	virtual void rightClick(int x, int y) = 0;
	virtual void update() = 0;
	virtual void showHotSpots();

	// TODO: perhaps move playIntro() someplace else?
	void playIntro();
	void showSprites();
	void showCodes();
	int getCurrentScene() { return _currentScene; }
	M4Surface *getBackgroundSurface() const { return _backgroundSurface; }
	void showInterface();
	void hideInterface();
	GameInterfaceView *getInterface() { return _interfaceSurface; }
	SceneResources &getSceneResources() { return *_sceneResources; }
	M4Surface *getWalkSurface() const { return _walkSurface; }

	void onRefresh(RectList *rects, M4Surface *destSurface);
	bool onEvent(M4EventType eventType, int32 param1, int x, int y, bool &captureEvents);
};

} // End of namespace M4

#endif
