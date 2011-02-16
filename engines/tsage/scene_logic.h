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

#ifndef TSAGE_SCENE_LOGIC_H
#define TSAGE_SCENE_LOGIC_H

#include "common/scummsys.h"
#include "tsage/events.h"
#include "tsage/core.h"
#include "tsage/scenes.h"
#include "tsage/globals.h"

namespace tSage {

class SceneFactory {
public:
	static Scene *createScene(int sceneNumber);
};

class DisplayHotspot: public SceneHotspot {
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

/*--------------------------------------------------------------------------*/

class Scene10: public Scene {
	/* Actions */
	class Scene10_Action1: public Action {
	public:
		virtual void signal();
	};
	class Scene10_Action2: public Action {
	public:
		virtual void signal();
	};
public:
	Speaker _speakerSText;
	Speaker _speakerQText;
	Scene10_Action1 _action1;
	Scene10_Action2 _action2;
	SceneObject _object1, _object2, _object3;
	SceneObject _object4, _object5, _object6;

	virtual void stripCallback(int v);
	virtual void postInit(SceneObjectList *OwnerList = NULL);
};

class Scene15: public Scene {
	/* Actions */
	class Scene15_Action1: public Action {
	public:
		virtual void signal();
		virtual void dispatch();
	};
public:
	Scene15_Action1 _action1;
	SceneObject _object1;
	SoundHandler _soundHandler;

	virtual void postInit(SceneObjectList *OwnerList = NULL);
};

class Scene20: public Scene {
	/* Actions */
	class Scene20_Action1: public Action {
	public:
		virtual void signal();
	};
	class Scene20_Action2: public Action {
	public:
		virtual void signal();
	};
	class Scene20_Action3: public Action {
	public:
		virtual void signal();
	};
	class Scene20_Action4: public Action {
	public:
		virtual void signal();
	};
public:
	SequenceManager _sequenceManager;
	SpeakerQText _speakerQText;
	SpeakerGameText _speakerGameText;
	Scene20_Action1 _action1;
	Scene20_Action2 _action2;
	Scene20_Action3 _action3;
	Scene20_Action4 _action4;
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
	class Scene30_beam: public SceneObject {
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
	class Scene30_door: public SceneObject {
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
	class Scene30_kzin: public SceneObject {
	public:
		virtual void doAction(int action);
	};

	/* Actions */
	class Scene30_beamAction: public Action {
	public:
		virtual void signal();
	};
	class Scene30_kzinAction: public Action {
	public:
		virtual void signal();
	};
	class Scene30_ringAction: public Action {
	public:
		virtual void signal();
	};
	class Scene30_talkAction: public Action {
	public:
		virtual void signal();
	};

public:
	SoundHandler _sound;
	DisplayHotspot _groundHotspot, _wallsHotspot, _courtyardHotspot, _treeHotspot;
	Scene30_beam _beam;
	Scene30_door _door;
	Scene30_kzin _kzin;

	Scene30_beamAction _beamAction;
	Scene30_kzinAction _kzinAction;
	Scene30_ringAction _ringAction;
	Scene30_talkAction _talkAction;
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
	class Scene40_Action1: public Action {
	public:
		virtual void signal();
	};
	class Scene40_Action2: public Action {
	public:
		virtual void signal();
	};
	class Scene40_Action3: public Action {
	public:
		virtual void signal();
	};
	class Scene40_Action4: public Action {
	public:
		virtual void signal();
	};
	class Scene40_Action5: public Action {
	public:
		virtual void signal();
	};
	class Scene40_Action6: public Action {
	public:
		virtual void signal();
	};
	class Scene40_Action7: public Action {
	public:
		virtual void signal();
	};
	class Scene40_Action8: public Action {
	public:
		virtual void signal();
	};

	/* Objects */
	class Scene40_DyingKzin: public SceneObject {
	public:
		virtual void doAction(int action);
	};
	class Scene40_Assassin: public SceneObject {
	public:
		virtual void doAction(int action);
	};

	/* Items */
	class Scene40_Item2: public SceneItem {
	public:
		virtual void doAction(int action);
	};
	class Scene40_Item6: public SceneItem {
	public:
		virtual void doAction(int action);
	};
	class Scene40_Item8: public SceneItem {
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
	Scene40_Action1 _action1;
	Scene40_Action2 _action2;
	Scene40_Action3 _action3;
	Scene40_Action4 _action4;
	Scene40_Action5 _action5;
	Scene40_Action6 _action6;
	Scene40_Action7 _action7;
	Scene40_Action8 _action8;
	SceneObject _object1, _object2, _object3;
	Scene40_DyingKzin _dyingKzin;
	Scene40_Assassin _assassin;
	SceneObject _doorway, _object7, _object8;
	DisplayHotspot _item1;
	Scene40_Item2 _item2;
	DisplayHotspot _item3, _item4, _item5;
	Scene40_Item6 _item6;
	DisplayHotspot _item7, _item8;

	Scene40();
	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void signal();
	virtual void dispatch();
};

class Scene50: public Scene {
	/* Actions */
	class Scene50_Action1: public Action {
	public:
		virtual void signal();
	};
	class Scene50_Action2: public Action {
	public:
		virtual void signal();
	};
	class Scene50_Action3: public Action {
	public:
		virtual void signal();
	};
	
	/* Objects */
	class Scene50_Object1: public SceneObject {
	public:
		virtual void doAction(int action);
	};
	class Scene50_Object2: public SceneObject {
	public:
		virtual void doAction(int action);
	};
	class Scene50_Object3: public SceneObject {
	public:
		virtual void doAction(int action);
	};
	class Scene50_Object4: public SceneObject {
	public:
		virtual void doAction(int action);
	};

public:
	SequenceManager _sequenceManager;
	Scene50_Action1 _action1;
	Scene50_Action2 _action2;
	Scene50_Action3 _action3;
	Scene50_Object1 _object1;
	Scene50_Object2 _object2;
	Scene50_Object3 _object3;
	Scene50_Object4 _object4;
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

class Scene1000: public Scene {
	/* Actions */
	class Scene1000_Action1: public Action {
	public:
		virtual void signal();
	};
	class Scene1000_Action2: public Action {
	public:
		virtual void signal();
	};
	class Scene1000_Action3: public Action {
	private:
		void zoom(bool up);
	public:
		virtual void signal();
	};

public:
	SceneObject _object1, _object2, _object3, _object4;
	Scene1000_Action1 _action1;
	Scene1000_Action2 _action2;
	Scene1000_Action3 _action3;

	virtual void postInit(SceneObjectList *OwnerList = NULL);
};

} // End of namespace tSage

#endif
