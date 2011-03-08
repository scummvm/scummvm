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

#ifndef TSAGE_RINGWORLD_SCENES3_H
#define TSAGE_RINGWORLD_SCENES3_H

#include "common/scummsys.h"
#include "tsage/core.h"
#include "tsage/converse.h"
#include "tsage/ringworld_logic.h"

namespace tSage {

class Scene2000: public Scene {
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
	class Action9: public Action {
	public:
		virtual void signal();
	};
	class Action10: public Action {
	public:
		virtual void signal();
	};
	class Action11: public Action {
	public:
		virtual void signal();
	};
	class Action12: public Action {
	public:
		virtual void signal();
	};
	class Action13: public Action {
	public:
		virtual void signal();
	};
	class Action14: public Action {
	public:
		virtual void signal();
	};

public:
	SpeakerSL _speakerSL;
	SpeakerQR _speakerQR;
	SpeakerMR _speakerMR;
	SpeakerQText _speakerQText;
	SpeakerMText _speakerMText;
	SpeakerSText _speakerSText;
	SpeakerHText _speakerHText;
	SpeakerGameText _speakerGameText;
	Action1 _action1;
	Action2 _action2;
	Action3 _action3;
	Action4 _action4;
	Action5 _action5;
	Action6 _action6;
	Action7 _action7;
	Action8 _action8;
	Action9 _action9;
	Action10 _action10;
	Action11 _action11;
	Action12 _action12;
	Action13 _action13;
	Action14 _action14;
	SceneObject _object1, _object2, _object3, _object4, _object5;
	SceneObject _object6, _object7, _object8, _object9, _object10;
	SoundHandler _soundHandler1, _soundHandler2;

	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void stripCallback(int v);
};

class Scene2100: public Scene {
	/* Actions */
	class Action1: public Action2 {
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
	class Action9: public Action {
	public:
		virtual void signal();
	};
	class Action10: public Action {
	public:
		virtual void signal();
	};
	class Action11: public Action {
	public:
		virtual void signal();
	};
	class Action12: public Action {
	public:
		virtual void signal();
	};
	class Action13: public Action {
	public:
		virtual void signal();
	};
	class Action14: public Action {
	public:
		virtual void signal();
	};
	class Action15: public Action {
	public:
		virtual void signal();
	};
	class Action16: public Action {
	public:
		virtual void signal();
	};
	class Action17: public Action {
	public:
		virtual void signal();
	};

	/* Hotspots */
	class Hotspot2: public SceneObject {
	public:
		virtual void doAction(int action);
	};
	class Hotspot3: public SceneObject {
	public:
		virtual void doAction(int action);
	};
	class Hotspot4: public SceneObject {
	public:
		virtual void doAction(int action);
	};
	class Hotspot8: public SceneObject {
	public:
		virtual void doAction(int action);
	};
	class Hotspot10: public SceneObject {
	public:
		virtual void doAction(int action);
	};
	class Hotspot14: public SceneObject {
	public:
		virtual void doAction(int action);
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
public:
	SequenceManager _sequenceManager;
	SoundHandler _soundHandler;
	SpeakerMText _speakerMText;
	SpeakerMR _speakerMR;
	SpeakerQL _speakerQL;
	SpeakerQR _speakerQR;
	SpeakerQText _speakerQText;
	SpeakerGameText _speakerGameText;
	SpeakerSText _speakerSText;
	SpeakerSL _speakerSL;
	SpeakerSAL _speakerSAL;
	SpeakerHText _speakerHText;
	
	DisplayHotspot _hotspot1;
	Hotspot2 _hotspot2;
	Hotspot3 _hotspot3;
	Hotspot4 _hotspot4;
	DisplayHotspot _hotspot5, _hotspot6, _hotspot7;
	Hotspot8 _hotspot8;
	DisplayHotspot _hotspot9;
	Hotspot10 _hotspot10;
	DisplayHotspot _hotspot11, _hotspot12, _hotspot13;
	Hotspot14 _hotspot14;
	DisplayHotspot _hotspot15;

	Object1 _object1;
	Object2 _object2;
	Object3 _object3;
	SceneObject _object4;

	Action1 _action1;
	Action2 _action2;
	Action3 _action3;
	Action4 _action4;
	Action5 _action5;
	Action6 _action6;
	Action7 _action7;
	Action8 _action8;
	Action9 _action9;
	Action10 _action10;
	Action11 _action11;
	Action12 _action12;
	Action13 _action13;
	Action14 _action14;
	Action15 _action15;
	Action16 _action16;
	Action17 _action17;
	int _field1800;
	SceneArea _area1, _area2, _area3, _area4;

	Scene2100();
	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void stripCallback(int v);
	virtual void signal();
};

class Scene2120: public Scene {
	/* Actions */
	class Entry {
	public:
		int _size;
		int _lineNum;
		int _visage;

		Entry() { _size = 0; _lineNum = 0; _visage = 0; }
		Entry(int size, int lineNum, int visage) { _size = size; _lineNum = lineNum; _visage = visage; }
	};

	class Action1: public Action {
	private:
		Common::Array<Entry> _entries;
	public:
		Action1();

		virtual void signal();
		virtual void dispatch();
	};

public:
	SoundHandler _soundHandler;
	SceneObject _topicArrowHotspot, _arrowHotspot, _visageHotspot;
	SceneObject _subjectButton, _nextPageButton, _previousPageButton, _exitButton;
	Action1 _action1;
	Rect _listRect;
	int _dbMode, _prevDbMode;
	bool _visageVisable;
	int _subjectIndex;
	int _lineOffset;

	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void synchronise(Serialiser &s);
};

class Scene2150: public Scene {
	/* Actions */
	class Action1: public Action2 {
	public:
		virtual void signal();
	};
	class Action2: public Action {
	public:
		virtual void signal();
	};

	/* Hotspots */
	class Hotspot1: public SceneObject {
	public:
		virtual void doAction(int action);
	};
	class Hotspot2: public SceneObject {
	public:
		virtual void doAction(int action);
	};
	class Hotspot4: public SceneObject {
	public:
		virtual void doAction(int action);
	};
	class Hotspot7: public SceneObject {
	public:
		virtual void doAction(int action);
	};
	class Hotspot10: public SceneObject {
	public:
		virtual void doAction(int action);
	};
public:
	SoundHandler _soundHandler;
	SequenceManager _sequenceManager;
	SpeakerGameText _speakerGameText;

	Rect _rect1, _rect2;
	Hotspot1 _hotspot1;
	Hotspot2 _hotspot2;
	DisplayHotspot _hotspot3;
	Hotspot4 _hotspot4;
	DisplayHotspot _hotspot5, _hotspot6;
	Hotspot7 _hotspot7;
	DisplayHotspot _hotspot8, _hotspot9;
	Hotspot10 _hotspot10;
	DisplayHotspot _hotspot11;
	SceneObject _hotspot12, _hotspot13, _hotspot14;
	SceneArea _area1, _area2, _area3, _area4;
	Action1 _action1;
	Action2 _action2;

	Scene2150();
	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void synchronise(Serialiser &s);
	virtual void signal();
	virtual void dispatch();
};

class Scene2200: public Scene {
	/* Actions */
	class Action1: public Action {
	public:
		virtual void signal();
	};
	class Action2: public Action {
	public:
		virtual void signal();
	};
	class Action3: public tSage::Action2 {
	public:
		virtual void signal();
		virtual void process(Event &event);
	};
	class Action4: public Action {
	public:
		virtual void signal();
	};

	/* Hotspots */
	class Hotspot3: public SceneObject {
	public:
		virtual void doAction(int action);
	};
	class Hotspot5: public SceneObject {
	public:
		virtual void doAction(int action);
	};
	class Hotspot9: public SceneObject {
	public:
		virtual void doAction(int action);
	};
public:
	SequenceManager _sequenceManager;
	SpeakerMText _speakerMText;
	SpeakerSText _speakerSText;
	SpeakerQText _speakerQText;
	SpeakerSL _speakerSL;
	SpeakerQR _speakerQR;
	SpeakerQL _speakerQL;
	SpeakerMR _speakerMR;
	SpeakerGameText _speakerGameText;
	Rect _exitRect;
	Action1 _action1;
	Action2 _action2;
	Action3 _action3;
	Action4 _action4;
	DisplayHotspot _hotspot1;
	Hotspot3 _hotspot3;
	Hotspot5 _hotspot5;
	Hotspot9 _hotspot9;
	DisplayHotspot _hotspot10;
	SceneObject _hotspot2, _hotspot4;
	SceneObject _hotspot6, _hotspot7, _hotspot8;
	SoundHandler _soundHandler1, _soundHandler2;

	Scene2200();
	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void stripCallback(int v);
	virtual void synchronise(Serialiser &s);
	virtual void signal();
	virtual void dispatch();
};

} // End of namespace tSage

#endif
