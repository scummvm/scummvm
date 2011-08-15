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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef TSAGE_BLUEFORCE_LOGIC_H
#define TSAGE_BLUEFORCE_LOGIC_H

#include "common/scummsys.h"
#include "tsage/events.h"
#include "tsage/core.h"
#include "tsage/scenes.h"
#include "tsage/globals.h"

#define BF_INTERFACE_Y 168

namespace TsAGE {

namespace BlueForce {

using namespace TsAGE;

class BlueForceGame: public Game {
public:
	virtual void start();
	virtual Scene *createScene(int sceneNumber);
};

#define OBJ_ARRAY_SIZE 10
class ObjArray: public EventHandler {
public:
	EventHandler *_objList[OBJ_ARRAY_SIZE];
	bool _inUse;
public:
	ObjArray();
	void clear();

	virtual Common::String getClassName() { return "ObjArray"; }
	virtual void synchronize(Serializer &s);
	virtual void process(Event &event);
	virtual void dispatch();
};

class SceneExt: public Scene {
public:
	ObjArray _objArray1, _objArray2;
	int _field372;
	int _field37A;
	EventHandler *_field37C;

	Rect _v51C34;
public:
	SceneExt();

	virtual Common::String getClassName() { return "SceneExt"; }
	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void process(Event &event);
	virtual void dispatch();
	virtual void loadScene(int sceneNum);
	virtual void proc13() { warning("TODO: SceneExt::proc13"); }
};

class GameScene: public SceneExt {
public:
	int _field412;
	int _field794;
public:
	GameScene();

	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void remove();
};

class BlueAnimatedSpeaker: public Speaker {
public:
};

} // End of namespace BlueForce

} // End of namespace TsAGE

#endif
