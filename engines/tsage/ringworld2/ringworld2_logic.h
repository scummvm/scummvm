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

#ifndef TSAGE_RINGWORLD2_LOGIC_H
#define TSAGE_RINGWORLD2_LOGIC_H

#include "common/scummsys.h"
#include "tsage/events.h"
#include "tsage/core.h"
#include "tsage/scenes.h"
#include "tsage/globals.h"

namespace TsAGE {

namespace Ringworld2 {

using namespace TsAGE;

#define R2_INVENTORY (*((::TsAGE::Ringworld2::Ringworld2InvObjectList *)g_globals->_inventory))

class SceneFactory {
public:
	static Scene *createScene(int sceneNumber);
};

class SceneExt: public Scene {
private:
	static void startStrip();
	static void endStrip();
public:
	int _field372;
	bool _savedPlayerEnabled;
	bool _savedUiEnabled;
	bool _savedCanWalk;
	int _field37A;

	SceneObject *_focusObject;
	Visage _cursorVisage;

	Rect _v51C34;
public:
	SceneExt();

	virtual Common::String getClassName() { return "SceneExt"; }
	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void remove();
	virtual void process(Event &event);
	virtual void dispatch();
	virtual void loadScene(int sceneNum);
	virtual void refreshBackground(int xAmount, int yAmount);

	bool display(CursorType action);
	void fadeOut();
	void clearScreen();
};

class SceneHandlerExt: public SceneHandler {
public:
	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void process(Event &event);
};


class DisplayHotspot : public SceneObject {
private:
	Common::Array<int> _actions;
	bool performAction(int action);
public:
	DisplayHotspot(int regionId, ...);

	virtual void doAction(int action) {
		if (!performAction(action))
			SceneHotspot::doAction(action);
	}
};

class DisplayObject : public SceneObject {
private:
	Common::Array<int> _actions;
	bool performAction(int action);
public:
	DisplayObject(int firstAction, ...);

	virtual void doAction(int action) {
		if (!performAction(action))
			SceneHotspot::doAction(action);
	}
};

class SceneObjectExt : public SceneObject {
public:
	int _state;

	virtual void synchronize(Serializer &s) {
		SceneObject::synchronize(s);
		s.syncAsSint16LE(_state);
	}
	virtual Common::String getClassName() { return "SceneObjectExt"; }
};

/*--------------------------------------------------------------------------*/

class Ringworld2InvObjectList : public InvObjectList {
public:
	InvObject _none;
	InvObject _inv1;
	InvObject _inv2;
	InvObject _negatorGun;
	InvObject _steppingDisks;
	InvObject _inv5;
	InvObject _inv6;
	InvObject _inv7;
	InvObject _inv8;
	InvObject _inv9;
	InvObject _inv10;
	InvObject _inv11;
	InvObject _inv12;
	InvObject _inv13;
	InvObject _inv14;
	InvObject _inv15;
	InvObject _inv16;
	InvObject _inv17;
	InvObject _inv18;
	InvObject _inv19;
	InvObject _inv20;
	InvObject _inv21;
	InvObject _inv22;
	InvObject _inv23;
	InvObject _inv24;
	InvObject _inv25;
	InvObject _inv26;
	InvObject _inv27;
	InvObject _inv28;
	InvObject _inv29;
	InvObject _inv30;
	InvObject _inv31;
	InvObject _inv32;
	InvObject _inv33;
	InvObject _inv34;
	InvObject _inv35;
	InvObject _inv36;
	InvObject _inv37;
	InvObject _inv38;
	InvObject _inv39;
	InvObject _inv40;
	InvObject _inv41;
	InvObject _inv42;
	InvObject _inv43;
	InvObject _inv44;
	InvObject _inv45;
	InvObject _inv46;
	InvObject _inv47;
	InvObject _inv48;
	InvObject _inv49;
	InvObject _inv50;
	InvObject _inv51;
	InvObject _inv52;

	Ringworld2InvObjectList();
	void reset();
	void setObjectScene(int objectNum, int sceneNumber);

	virtual Common::String getClassName() { return "Ringworld2InvObjectList"; }
};

#define RING2_INVENTORY (*((::TsAGE::Ringworld2::Ringworld2InvObjectList *)g_globals->_inventory))

class Ringworld2Game: public Game {
public:
	virtual void start();
	virtual void restart();
	virtual void endGame(int resNum, int lineNum);

	virtual Scene *createScene(int sceneNumber);
	virtual void processEvent(Event &event);
	virtual void rightClick();
	virtual bool canSaveGameStateCurrently();
	virtual bool canLoadGameStateCurrently();
};

class NamedHotspot : public SceneHotspot {
public:
	int _resNum, _lookLineNum, _useLineNum, _talkLineNum;
	NamedHotspot();

	virtual bool startAction(CursorType action, Event &event);
	virtual Common::String getClassName() { return "NamedHotspot"; }
	virtual void synchronize(Serializer &s);
	virtual void setDetails(int ys, int xs, int ye, int xe, const int resnum, const int lookLineNum, const int useLineNum);
	virtual void setDetails(const Rect &bounds, int resNum, int lookLineNum, int talkLineNum, int useLineNum, int mode, SceneItem *item);
	virtual void setDetails(int sceneRegionId, int resNum, int lookLineNum, int talkLineNum, int useLineNum, int mode = 0);
};

class NamedHotspotExt : public NamedHotspot {
public:
	int _flag;
	NamedHotspotExt() { _flag = 0; }

	virtual Common::String getClassName() { return "NamedHotspot"; }
	virtual void synchronize(Serializer &s) {
		NamedHotspot::synchronize(s);
		s.syncAsSint16LE(_flag);
	}
};

class SceneActor: public SceneObject {
public:
	int _resNum;
	int _lookLineNum, _talkLineNum, _useLineNum;

	virtual Common::String getClassName() { return "SceneActor"; }
	virtual void synchronize(Serializer &s);
	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual bool startAction(CursorType action, Event &event);

	void setDetails(int resNum, int lookLineNum, int talkLineNum, int useLineNum, int mode, SceneItem *item);
	void setDetails(int resNum, int lookLineNum, int talkLineNum, int useLineNum);
};

class SceneActorExt: public SceneActor {
public:
	int _state;

	SceneActorExt() { _state = 0; }
	virtual Common::String getClassName() { return "SceneActorExt"; }
	virtual void synchronize(Serializer &s) {
		SceneActor::synchronize(s);
		s.syncAsSint16LE(_state);
	}
};


} // End of namespace Ringworld2

} // End of namespace TsAGE

#endif
