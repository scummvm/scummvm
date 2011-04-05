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

#ifndef TSAGE_RINGWORLD_SCENES10_H
#define TSAGE_RINGWORLD_SCENES10_H

#include "common/scummsys.h"
#include "tsage/ringworld_logic.h"
#include "tsage/events.h"
#include "tsage/core.h"
#include "tsage/scenes.h"
#include "tsage/globals.h"

namespace tSage {

class Scene2: public Scene {
public :
	int _sceneState;

	Scene2();
	virtual void synchronise(Serialiser &s) {
		Scene::synchronise(s);
		s.syncAsSint16LE(_sceneState);
	}
};

class Object9350: public SceneObject {
public:
	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void draw();
};

class Scene9100: public Scene {
	/* Items */
	class SceneHotspot1: public SceneHotspot_3 {
	public:
		virtual void doAction(int action);
	};
public:
	SequenceManager _sequenceManager;
	SceneObject _object1;
	SceneObject _object2;
	SceneObject _object3;
	SceneObject _object4;
	SceneObject _object5;
	SceneObject _object6;
	SceneHotspot1  _sceneHotspot1;
	SceneHotspot_3 _sceneHotspot2;
	SceneHotspot_3 _sceneHotspot3;
	SceneHotspot_3 _sceneHotspot4;
	SceneHotspot_3 _sceneHotspot5;
	SceneHotspot_3 _sceneHotspot6;

	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void signal();
	virtual void dispatch();
};

class Scene9150: public Scene2 {
	class Object3: public SceneObject4 {
	public:
		virtual void signal();
		virtual void dispatch();
	};
public:
	SequenceManager _sequenceManager1;
	SequenceManager _sequenceManager2;
	SceneObject _object1;
	SceneObject _object2;
	Object3 _object3;
	SceneHotspot_3 _sceneHotspot1;
	SceneHotspot_3 _sceneHotspot2;
	SceneHotspot_3 _sceneHotspot3;
	SceneHotspot_3 _sceneHotspot4;
	SceneHotspot_3 _sceneHotspot5;
	SceneHotspot_3 _sceneHotspot6;
	SceneHotspot_3 _sceneHotspot7;
	SceneHotspot_3 _sceneHotspot8;
	SceneHotspot_3 _sceneHotspot9;
	SceneHotspot_3 _sceneHotspot10;

	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void signal();
	virtual void dispatch();
};

class Scene9200: public Scene2 {
	class SceneHotspot1: public SceneHotspot_3{
	public:
		virtual void doAction(int action);
	};
public:
	SequenceManager _sequenceManager;
	SceneObject _object1;
	SceneObject _object2;
	SceneObject _object3;
	Action _action1;
	SpeakerGText _speakerGText;
	SpeakerGR _speakerGR;
	SpeakerQText _speakerQText;
	SoundHandler _soundHandler;
	SceneHotspot1 _hotspot1;
	SceneHotspot_3 _hotspot2;
	SceneHotspot_3 _hotspot3;
	SceneHotspot_3 _hotspot4;
	SceneHotspot_3 _hotspot5;
	SceneHotspot_3 _hotspot6;
	SceneHotspot_3 _hotspot7;
	SceneHotspot_3 _hotspot8;

	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void signal();
	virtual void dispatch();
	virtual void process(Event &event);
};

class Scene9300: public Scene {
public:
	SequenceManager _sequenceManager;
	SceneObject _object1;
	SceneObject _object2;
	SceneHotspot_3 _hotspot1;
	SceneHotspot_3 _hotspot2;
	SceneHotspot_3 _hotspot3;
	SceneHotspot_3 _hotspot4;
	SceneHotspot_3 _hotspot5;
	SceneHotspot_3 _hotspot6;
	SceneHotspot_3 _hotspot7;
	SceneHotspot_3 _hotspot8;
	SceneHotspot_3 _hotspot9;
	SceneHotspot_3 _hotspot10;
	SceneHotspot_3 _hotspot11;
	SceneHotspot_3 _hotspot12;
	SceneHotspot_3 _hotspot13;

	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void signal();
	virtual void dispatch();
};

class Scene9350: public Scene2 {
public:
	SequenceManager _sequenceManager;
	Object9350 _object1;
	SceneObject _object2;
	SceneHotspot_3 _sceneHotspot1;
	SceneHotspot_3 _sceneHotspot2;
	SceneHotspot_3 _sceneHotspot3;
	SceneHotspot_3 _sceneHotspot4;
	SceneHotspot_3 _sceneHotspot5;

	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void signal();
	virtual void dispatch();
};

class Scene9360: public Scene2 {
public:
	SequenceManager _sequenceManager;
	Action _action1;
	Object9350 _object1;
	SceneHotspot_3 _hotspot1;
	SceneHotspot_3 _hotspot2;
	SceneHotspot_3 _hotspot3;
	SceneHotspot_3 _hotspot4;
	SceneHotspot_3 _hotspot5;
	SceneHotspot_3 _hotspot6;
	SceneHotspot_3 _hotspot7;
	SceneHotspot_3 _hotspot8;

	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void signal();
	virtual void dispatch();
};

class Scene9400: public Scene2 {
	class SceneHotspot7: public SceneHotspot_3{
	public:
		virtual void doAction(int action);
	};

	class SceneHotspot8: public SceneHotspot_3{
	public:
		virtual void doAction(int action);
	};
public:
	Scene9400();
	SequenceManager _sequenceManager;
	Action _action1;
	SceneObject _object1;
	SceneObject _object2;
	SceneObject _object3;
	SpeakerOText _speakerOText;
	SpeakerOR _speakerOR;
	SpeakerQText _speakerQText;
	SceneHotspot_3 _hotspot1;
	SceneHotspot_3 _hotspot2;
	SceneHotspot_3 _hotspot3;
	SceneHotspot_3 _hotspot4;
	SceneHotspot_3 _hotspot5;
	SceneHotspot_3 _hotspot6;
	SoundHandler _soundHandler;
	int _field1032;
	SceneHotspot7 _hotspot7;
	SceneHotspot8 _hotspot8;

	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void signal();
	virtual void dispatch();
};

class Scene9450: public Scene2 {
	class Object2: public SceneObject {
	public:
		virtual void signal();
	};

	class Object3: public SceneObject4 {
	public:
		virtual void dispatch();
	};

	class Hotspot1: public SceneHotspot_3{
	public:
		virtual void doAction(int action);
	};

	class Hotspot3: public SceneHotspot_3{
	public:
		virtual void doAction(int action);
	};
public:
	SceneObject _object1;
	SequenceManager _sequenceManager1;
	SequenceManager _sequenceManager2;
	Object2 _object2;
	SequenceManager _sequenceManager3;
	Object3 _object3;
	Hotspot1 _hotspot1;
	SceneHotspot_3 _hotspot2;
	Hotspot3 _hotspot3;
	SceneHotspot_3 _hotspot4;
	SceneHotspot_3 _hotspot5;
	SceneHotspot_3 _hotspot6;
	SceneHotspot_3 _hotspot7;
	SceneHotspot_3 _hotspot8;
	SceneHotspot_3 _hotspot9;
	SceneHotspot_3 _hotspot10;
	SceneHotspot_3 _hotspot11;
	SceneHotspot_3 _hotspot12;
	SceneHotspot_3 _hotspot13;
	SceneHotspot_3 _hotspot14;
	SceneHotspot_3 _hotspot15;
	SceneHotspot_3 _hotspot16;

	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void signal();
	virtual void dispatch();
};

class Scene9700: public Scene2 {
	SequenceManager _sequenceManager;
	SceneObject _object1;
	SceneHotspot_3 _sceneHotspot1;
	SceneHotspot_3 _sceneHotspot2;
	SceneHotspot_3 _sceneHotspot3;
	SceneHotspot_3 _sceneHotspot4;
	SceneHotspot_3 _sceneHotspot5;
	SceneHotspot_3 _sceneHotspot6;
	GfxButton _gfxButton1;

	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void signal();
	virtual void process(Event &event);
};

class Scene9750: public Scene {
public:
	SequenceManager _sequenceManager;
	SceneObject _object1;
	SceneObject _object2;

	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void signal();
	virtual void dispatch();
};

class Scene9999: public Scene {
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

	Action1 _action1;
	Action2 _action2;
	Action _action3;
	SceneObject _object1;
	SceneObject _object2;
	SceneObject _object3;

	virtual void postInit(SceneObjectList *OwnerList = NULL);
};


} // End of namespace tSage

#endif
