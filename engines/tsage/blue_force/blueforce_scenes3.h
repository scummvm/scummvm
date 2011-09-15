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

#ifndef TSAGE_BLUEFORCE_SCENES3_H
#define TSAGE_BLUEFORCE_SCENES3_H

#include "common/scummsys.h"
#include "tsage/blue_force/blueforce_logic.h"
#include "tsage/blue_force/blueforce_speakers.h"
#include "tsage/converse.h"
#include "tsage/events.h"
#include "tsage/core.h"
#include "tsage/scenes.h"
#include "tsage/globals.h"
#include "tsage/sound.h"

namespace TsAGE {

namespace BlueForce {

using namespace TsAGE;

class Scene300: public SceneExt {
	/* Objects */
	class Object: public NamedObject {
	public:
		int _stripNumber;
	public:
		Object(int stripNumber) { _stripNumber = stripNumber; }

		virtual bool startAction(CursorType action, Event &event);
	};
	class Object19: public NamedObject {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};

	/* Items */
	class Item1: public NamedHotspot {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};
	class Item2: public NamedHotspot {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};
	class Item14: public NamedHotspot {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};
	class Item15: public NamedHotspot {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};

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
private:
	void setup();
public:
	SequenceManager _sequenceManager1, _sequenceManager2;
	SequenceManager _sequenceManager3, _sequenceManager4;
	NamedObject _object1;
	FollowerObject _object2, _object3, _object4, _object5, _object6, _object7;
	SceneObject _object8, _object9, _object10;
	NamedObject _object11, _object12;
	Object _object13, _object14, _object15, _object16;
	NamedObject _object17, _object18;
	Object19 _object19;
	Item1 _item1;
	Item2 _item2;
	NamedHotspot _item3, _item4, _item5, _item6, _item7;
	NamedHotspot _item8, _item9, _item10, _item11;
	NamedHotspot _item12, _item13;
	Item14 _item14;
	Item15 _item15;
	Action1 _action1;
	Action2 _action2;
	Action3 _action3;
	Action4 _action4;
	Action5 _action5;
	SpeakerGameText _gameTextSpeaker;
	SpeakerSutter _sutterSpeaker;
	SpeakerDoug _dougSpeaker;
	SpeakerJakeNoHead _jakeSpeaker;
	TimerExt _timer;
	int _field2760, _field2762;

	Scene300();
	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void signal();
	virtual void process(Event &event);
	virtual void dispatch();
};

class Scene315: public SceneExt {
	/* Objects */
	class Object1: public NamedObject {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};
	class Object2: public NamedObject {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};
	class Object3: public NamedObject {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};

	/* Items */
	class Item1: public NamedHotspot {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};
	class Item2: public NamedHotspot {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};
	class Item4: public NamedHotspot {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};
	class Item5: public NamedHotspot {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};
	class Item14: public NamedHotspot {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};
	class Item15: public NamedHotspot {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};
	class Item16: public NamedHotspot {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};
	class Item17: public NamedHotspot {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};

	/* Actions */
	class Action1: public Action {
	public:
		virtual void signal();
	};
public:
	SequenceManager _sequenceManager;
	SpeakerGameText _gameTextSpeaker;
	SpeakerSutter _sutterSpeaker;
	SpeakerHarrison _harrisonSpeaker;
	SpeakerJakeJacket _jakeJacketSpeaker;
	SpeakerJakeUniform _jakeUniformSpeaker;
	SpeakerJailer _jailerSpeaker;
	Item1 _item1;
	Item2 _item2;
	NamedHotspot _item3;
	Item4 _item4;
	Item5 _item5;
	Object1 _object1;
	Object2 _object2;
	Object3 _object3;
	SceneObject _object4, _object5, _object6;
	SceneObject _object7, _object8, _object9;
	NamedHotspot _item6, _item7, _item8, _item9;
	NamedHotspot _item10, _item11, _item12, _item13;
	Item14 _item14;
	Item15 _item15;
	Item16 _item16;
	Item17 _item17;
	Action1 _action1;
	int _field1390;
	int _stripNumber;
	int _field1398;
	int _field1B60, _field1B62, _field1B64;
	int _field1B66, _field1B6C, _field139C;
	bool _field1B68, _field1B6A;
	CursorType _currentCursor;

	Scene315();
	virtual void synchronize(Serializer &s);
	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void signal();
	virtual void process(Event &event);
	virtual void dispatch();
};

class Scene325: public SceneExt {
	/* Items */
	class Item1: public NamedHotspot {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};
public:
	SequenceManager _sequenceManager;
	SpeakerGameText _gameTextSpeaker;
	SpeakerPSutter _PSutterSpeaker;
	Item1 _item1;
	NamedObject _object1, _object2, _object3, _object4, _object5;

	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void signal();
};

class Scene330: public SceneExt {
	class Timer1: public Timer {
	public:	
		virtual void signal();
	};
public:
	SequenceManager _sequenceManager;
	NamedObject _object1;
	SceneObject _object2;
	ASoundExt _sound1, _sound2;
	Timer1 _timer;
	int _seqNumber;

	Scene330();
	virtual void synchronize(Serializer &s);
	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void remove();
	virtual void signal();
};

class Scene340: public PalettedScene {
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
		void process(Event &event);
	};

	/* Items */
	class Item1: public NamedHotspot {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};
	class WestExit: public NamedHotspot {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};
	class SouthWestExit: public NamedHotspot {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};
	class NorthExit: public NamedHotspot {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};

	/* Objects */
	class Child: public NamedObject {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};
	class Woman: public NamedObject {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};
	class Lyle: public NamedObject {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};

	/* Timers */
	class Timer2: public Timer {
	public:	
		virtual void signal();
	};
public:
	SequenceManager _sequenceManager1, _sequenceManager2, _sequenceManager3;
	Child _child;
	Woman _woman;
	Lyle _lyle;
	SceneObject _object4, _object5;
	Item1 _item1;
	NamedHotspot _item2, _item3;
	WestExit _westExit;
	SouthWestExit _swExit;
	NorthExit _northExit;
	Action1 _action1;
	Action2 _action2;
	Action3 _action3;
	Action4 _action4;
	Action5 _action5;
	Action6 _action6;
	Action7 _action7;
	Action8 _action8;
	SpeakerGameText _gameTextSpeaker;
	SpeakerJordan _jordanSpeaker;
	SpeakerSkipB _skipBSpeaker;
	SpeakerJakeUniform _jakeUniformSpeaker;
	SpeakerHarrison _harrisonSpeaker;
	ASoundExt _sound1, _sound2;
	TimerExt _timer1;
	Timer2 _timer2;
	int _seqNumber1, _field2652, _field2654;

	Scene340();
	virtual void synchronize(Serializer &s);
	virtual void postInit(SceneObjectList *OwnerList = NULL);
	void remove();
	virtual void signal();
	virtual void process(Event &event);
	virtual void dispatch();
};

class Scene342: public PalettedScene {
	/* Items */
	class Item1: public NamedHotspot {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};
	class WestExit: public NamedHotspot {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};
	class SouthWestExit: public NamedHotspot {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};
	class NorthExit: public NamedHotspot {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};

	/* Objects */
	class Lyle: public NamedObject {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};

	/* Timers */
	class Timer1: public Timer {
	public:	
		virtual void signal();
	};
public:
	SequenceManager _sequenceManager1, _sequenceManager2;
	Lyle _lyle;
	NamedObject _object2;
	SceneObject _object3, _object4;
	Item1 _item1;
	NamedHotspot _item2, _item3;
	WestExit _westExit;
	SouthWestExit _swExit;
	NorthExit _northExit;
	SpeakerGameText _gameTextSpeaker;
	SpeakerJakeJacket _jakeJacketSpeaker;
	SpeakerLyleHat _lyleHatSpeaker;
	ASoundExt _sound1;
	Timer1 _timer1;
	int _field1A1A;

	Scene342();
	virtual void synchronize(Serializer &s);
	virtual void postInit(SceneObjectList *OwnerList = NULL);
	void remove();
	virtual void signal();
	virtual void process(Event &event);
	virtual void dispatch();
};


} // End of namespace BlueForce

} // End of namespace TsAGE

#endif
