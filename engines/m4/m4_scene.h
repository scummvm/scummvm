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

#ifndef M4_M4_SCENE_H
#define M4_M4_SCENE_H

class View;

#include "m4/scene.h"

namespace M4 {

#define TITLE_SCENE_BURGER 951		// 951 = intro, 901 = demo menu, 971 = first scene
#define MAINMENU_SCENE_BURGER 903
#define FIRST_SCENE 101

class M4SceneResources : public SceneResources {
public:
	int32 frontY, backY;
	int32 frontScale, backScale;
	int16 depthTable[16];
	int32 railNodeCount;	// # of rails

	int32 parallaxCount;
	HotSpotList *parallax;
};

class M4Scene : public Scene {
private:
	M4Engine *_vm;
	M4SceneResources _sceneResources;
	SpriteAsset *_sceneSprites;
	SpriteAsset *_walkerSprite;
	byte *_inverseColorTable;

	void loadSceneSprites(int sceneNumber);
	void loadSceneResources(int sceneNumber);
	void loadSceneInverseColorTable(int sceneNumber);
	void loadSceneSpriteCodes(int sceneNumber);
	void nextCommonCursor();
public:
	M4Scene(M4Engine *vm);
	virtual ~M4Scene();

	// Methods that differ between engines
	virtual void loadScene(int sceneNumber);
	virtual void leaveScene();
	virtual void loadSceneCodes(int sceneNumber, int index = 0);
	virtual void show();
	virtual void mouseMove(int x, int y);
	virtual void leftClick(int x, int y);
	virtual void rightClick(int x, int y);
	virtual void update();
	virtual void showHotSpots();

	byte *getInverseColorTable() const { return _inverseColorTable; }
	M4InterfaceView *getInterface() { return (M4InterfaceView *)_interfaceSurface; }
	M4SceneResources &getSceneResources() { return _sceneResources; }
	void setStatusText(const char *text);
	void setAction(int action, int objectId = -1);
};

} // End of namespace M4

#endif
