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
 * $URL: https://scummvm-misc.svn.sourceforge.net/svnroot/scummvm-misc/trunk/engines/tsage/scene_logic.h $
 * $Id: scene_logic.h 232 2011-02-12 11:56:38Z dreammaster $
 *
 */

#ifndef TSAGE_RINGWORLD_SCENES1_H
#define TSAGE_RINGWORLD_SCENES1_H

#include "common/scummsys.h"
#include "tsage/ringworld_logic.h"
#include "tsage/events.h"
#include "tsage/core.h"
#include "tsage/scenes.h"
#include "tsage/globals.h"

namespace tSage {

class Scene10: public Scene {
	/* Actions */
	class Action1: public Action {
	public:
		virtual void signal();
	};
	class Action2: public Action {
	public:
		virtual void signal();
	};
public:
	Speaker _speakerSText;
	Speaker _speakerQText;
	Action1 _action1;
	Action2 _action2;
	SceneObject _object1, _object2, _object3;
	SceneObject _object4, _object5, _object6;

	virtual void stripCallback(int v);
	virtual void postInit(SceneObjectList *OwnerList = NULL);
};

class Scene15: public Scene {
	/* Actions */
	class Action1: public Action {
	public:
		virtual void signal();
		virtual void dispatch();
	};
public:
	Action1 _action1;
	SceneObject _object1;
	SoundHandler _soundHandler;

	virtual void postInit(SceneObjectList *OwnerList = NULL);
};

class Scene20: public Scene {
	/* Actions */
	class Action1: public Action {
	public:
		virtual void signal();
	};
	class Action2: public Action {
	public:
		virtual void signal();
	};
	class Action3: public Action {
	public:
		virtual void signal();
	};
	class Action4: public Action {
	public:
		virtual void signal();
	};
public:
	SequenceManager _sequenceManager;
	SpeakerQText _speakerQText;
	SpeakerGameText _speakerGameText;
	Action1 _action1;
	Action2 _action2;
	Action3 _action3;
	Action4 _action4;
	SceneObject _sceneObject1, _sceneObject2, _sceneObject3, _sceneObject4, _sceneObject5;
	SoundHandler _sound;
public:
	Scene20();
	virtual ~Scene20() {}

	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void signal();
};

class Scene30: public Scene {
	/* Scene objects */
	// Doorway beam sensor
	class BeamObject: public SceneObject {
	public:
		virtual void doAction(int action) {
			if (action == OBJECT_SCANNER)
				display(30, 14, SET_WIDTH, 200, SET_EXT_BGCOLOUR, 7, LIST_END);
			else if (action == CURSOR_LOOK)
				display(30, 2, SET_WIDTH, 200, SET_EXT_BGCOLOUR, 7, LIST_END);
			else if (action == CURSOR_USE) {
				Scene30 *parent = (Scene30 *)_globals->_sceneManager._scene;
				parent->setAction(&parent->_beamAction);
			} else
				SceneObject::doAction(action);
		}
	};

	// Doorway object
	class DoorObject: public SceneObject {
	public:
		virtual void doAction(int action) {
			if (action == OBJECT_SCANNER)
				display(30, 13, SET_WIDTH, 200, SET_EXT_BGCOLOUR, 7, LIST_END);
			else if (action == CURSOR_LOOK)
				display(30, 1, SET_WIDTH, 200, SET_EXT_BGCOLOUR, 7, LIST_END);
			else if (action == CURSOR_USE)
				display(30, 7, SET_WIDTH, 200, SET_EXT_BGCOLOUR, 7, LIST_END);
			else
				SceneObject::doAction(action);
		}
	};

	// Kzin object
	class KzinObject: public SceneObject {
	public:
		virtual void doAction(int action);
	};

	/* Actions */
	class BeamAction: public Action {
	public:
		virtual void signal();
	};
	class KzinAction: public Action {
	public:
		virtual void signal();
	};
	class RingAction: public Action {
	public:
		virtual void signal();
	};
	class TalkAction: public Action {
	public:
		virtual void signal();
	};

public:
	SoundHandler _sound;
	DisplayHotspot _groundHotspot, _wallsHotspot, _courtyardHotspot, _treeHotspot;
	BeamObject _beam;
	DoorObject _door;
	KzinObject _kzin;

	BeamAction _beamAction;
	KzinAction _kzinAction;
	RingAction _ringAction;
	TalkAction _talkAction;
	SequenceManager _sequenceManager;

	SpeakerSR _speakerSR;
	SpeakerQL _speakerQL;
	SpeakerSText _speakerSText;
	SpeakerQText _speakerQText;
public:
	Scene30();
	virtual ~Scene30() {}

	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void signal();
};

class Scene40: public Scene {
	/* Actions */
	class Action1: public Action {
	public:
		virtual void signal();
	};
	class Action2: public Action {
	public:
		virtual void signal();
	};
	class Action3: public Action {
	public:
		virtual void signal();
	};
	class Action4: public Action {
	public:
		virtual void signal();
	};
	class Action5: public Action {
	public:
		virtual void signal();
	};
	class Action6: public Action {
	public:
		virtual void signal();
	};
	class Action7: public Action {
	public:
		virtual void signal();
	};
	class Action8: public Action {
	public:
		virtual void signal();
	};

	/* Objects */
	class DyingKzin: public SceneObject {
	public:
		virtual void doAction(int action);
	};
	class Assassin: public SceneObject {
	public:
		virtual void doAction(int action);
	};

	/* Items */
	class Item2: public SceneItem {
	public:
		virtual void doAction(int action);
	};
	class Item6: public SceneItem {
	public:
		virtual void doAction(int action);
	};
	class Item8: public SceneItem {
	public:
		virtual void doAction(int action);
	};
public:
	SequenceManager _sequenceManager;
	SpeakerSL _speakerSL;
	SpeakerQR _speakerQR;
	SpeakerQText _speakerQText;
	SpeakerSText _speakerSText;
	SpeakerGameText _speakerGameText;
	SoundHandler _soundHandler;
	Action1 _action1;
	Action2 _action2;
	Action3 _action3;
	Action4 _action4;
	Action5 _action5;
	Action6 _action6;
	Action7 _action7;
	Action8 _action8;
	SceneObject _object1, _object2, _object3;
	DyingKzin _dyingKzin;
	Assassin _assassin;
	SceneObject _doorway, _object7, _object8;
	DisplayHotspot _item1;
	Item2 _item2;
	DisplayHotspot _item3, _item4, _item5;
	Item6 _item6;
	DisplayHotspot _item7, _item8;

	Scene40();
	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void signal();
	virtual void dispatch();
};

class Scene50: public Scene {
	/* Actions */
	class Action1: public Action {
	public:
		virtual void signal();
	};
	class Action2: public Action {
	public:
		virtual void signal();
	};
	class Action3: public Action {
	public:
		virtual void signal();
	};
	
	/* Objects */
	class Object1: public SceneObject {
	public:
		virtual void doAction(int action);
	};
	class Object2: public SceneObject {
	public:
		virtual void doAction(int action);
	};
	class Object3: public SceneObject {
	public:
		virtual void doAction(int action);
	};
	class Object4: public SceneObject {
	public:
		virtual void doAction(int action);
	};

public:
	SequenceManager _sequenceManager;
	Action1 _action1;
	Action2 _action2;
	Action3 _action3;
	Object1 _object1;
	Object2 _object2;
	Object3 _object3;
	Object4 _object4;
	Rect _doorwayRect;
	SpeakerSText _speakerSText;
	SpeakerQText _speakerQText;
	DisplayHotspot _item0, _item1, _item2;
	DisplayHotspot _item3, _item4, _item5;

	Scene50();
	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void signal();
	virtual void dispatch();
};

class Scene60: public Scene {	
	class Action1: public Action {
	public:
		virtual void signal();
	};
	class Action2: public Action {
	public:
		virtual void signal();
	};
	class Object2: public SceneObject {
	public:
		virtual void doAction(int action);
	};
	class Object3: public SceneObject {
	public:
		virtual void doAction(int action);
	};
	class Object4: public SceneObject {
	public:
		virtual void doAction(int action);
	};
	class Object5: public SceneObject {
	public:
		virtual void doAction(int action);
	};
	class Object6: public SceneObject {
	public:
		virtual void doAction(int action);
	};
	class Object7: public SceneObject2 {
	public:
		virtual void doAction(int action);
	};
	class SceneObject2: public SceneObject {
	public:
		int _state;
		virtual void synchronise(Serialiser &s);
	};
	class Object8: public SceneObject2 {
	public:
		virtual void doAction(int action);
	};
	class Object9: public SceneObject {
	public:
		virtual void doAction(int action);
	};
	class Item1: public SceneItem {
	public:
		virtual void doAction(int action);
	};
	class Item: public SceneItem {
	public:
		int _messageNum, _sceneMode;

		Item(int sceneRegionId, int messageNum, int sceneMode) {
			_sceneRegionId = sceneRegionId;
			_messageNum = messageNum;
			_sceneMode = sceneMode;
		}
		virtual void doAction(int action);
	};

public:
	GfxButton _gfxButton;
	SequenceManager _sequenceManager;
	SpeakerQText _speakerQText;
	SpeakerSText _speakerSText;
	Action1 _action1;
	Action2 _action2;
	SceneObject _object1;
	Object2 _object2;
	Object3 _object3;
	Object4 _object4;
	Object5 _object5;
	Object6 _object6;
	Object7 _object7;
	Object8 _object8;
	Object9 _object9;
	SceneObject _object10;
	SceneItem _item1;
	Item _item2, _item3, _item4, _item5, _item6;
	SoundHandler _soundHandler1;
	SoundHandler _soundHandler2;
	SoundHandler _soundHandler3;

	Scene60();
	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void signal();
	virtual void process(Event &event);
};

class Scene90: public Scene {
	class Action1: public Action {
	public:
		virtual void signal();
	};
	class Object1: public SceneObject {
	public:
		virtual void doAction(int action);
	};
	class Object2: public SceneObject {
	public:
		virtual void doAction(int action);
	};
	class Object3: public SceneObject {
	public:
		virtual void doAction(int action);
	};
	class Object4: public SceneObject {
	public:
		virtual void doAction(int action);
	};
public:
	SequenceManager _sequenceManager;
	SpeakerSText _speakerSText;
	SpeakerQText _speakerQText;
	SpeakerQL _speakerQL;
	SpeakerSR _speakerSR;
	SpeakerMText _speakerMText;
	Action _action1;
	SceneObject _object1, _object2;
	DisplayObject _object3, _object4, _object5;
	SceneObject _object6;
	DisplayHotspot _item1, _item2, _item3;
	SoundHandler _soundHandler1, _soundHandler2;

	Scene90();

	virtual void stripCallback(int v);
	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void signal();
};

class Scene95: public Scene {
	class Action1: public Action2 {
	public:
		virtual void signal();
	};

public:
	Action1 _action1;
	int _field326;
	SceneObject _object1, _object2, _object3;
	SoundHandler _soundHandler;

	Scene95();
	virtual void postInit(SceneObjectList *OwnerList);
};

class Scene1000: public Scene {
	/* Actions */
	class Action1: public Action {
	public:
		virtual void signal();
	};
	class Action2: public Action {
	public:
		virtual void signal();
	};
	class Action3: public Action {
	private:
		void zoom(bool up);
	public:
		virtual void signal();
	};

public:
	SceneObject _object1, _object2, _object3, _object4;
	Action1 _action1;
	Action2 _action2;
	Action3 _action3;

	virtual void postInit(SceneObjectList *OwnerList = NULL);
};

class Scene6100: public Scene {
	/* Actions */
	class Action1: public Action {
	public:
		virtual void signal();
	};
	class Action2: public Action {
	public:
		virtual void signal();
	};
	class Action3: public Action {
	public:
		virtual void signal();
	};
	class Action4: public Action {
	public:
		virtual void signal();
	};
	class Action5: public Action {
	public:
		virtual void dispatch();
	};
	class Action6: public Action {
	public:
		virtual void signal();
	};
	class Action7: public Action {
	public:
		virtual void signal();
	};

	/* Objects */
	class Object: public SceneObject {
	public:
		FloatSet _floats;
	};

	/* Items */
	class Item1: public SceneItem {
	public:
		virtual void doAction(int action);
	};

public:
	Action1 _action1;
	Action2 _action2;
	Action3 _action3;
	Action4 _action4;
	Action5 _action5;
	Action6 _action6;
	Action7 _action7;
	SoundHandler _soundHandler;
	Speaker _speaker1;
	SpeakerQR _speakerQR;
	SpeakerSL _speakerSL;
	SceneObject _object1, _object2, _object3;
	Object _object4, _object5, _object6;
	Object _object7, _object8;
	SceneText _sceneText;
	SceneItem _item1;

	int _field_30A, _field_30C, _field_30E, _field_310;
	int _field_312, _field_314;
	Object *_objList[4];

	virtual void postInit(SceneObjectList *OwnerList = NULL);
	void showMessage(const Common::String &msg, int colour, Action *action);

};

} // End of namespace tSage

#endif
