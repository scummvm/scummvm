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

#ifndef TSAGE_BLUEFORCE_SCENES4_H
#define TSAGE_BLUEFORCE_SCENES4_H

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

class Scene410: public SceneExt {
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
	class Item2: public NamedHotspot {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};
	class Item4: public NamedHotspot {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};
public:
	SequenceManager _sequenceManager1, _sequenceManager2, _sequenceManager3;
	Object1 _object1;
	Object2 _object2;
	Object3 _object3;
	NamedObject _object4, _object5, _object6;
	NamedHotspot _item1;
	Item2 _item2;
	NamedHotspot _item3;
	Item4 _item4;
	Action1 _action1;
	Action2 _action2;
	Action3 _action3;
	Action4 _action4;
	Action5 _action5;
	Action6 _action6;
	Action7 _action7;
	SpeakerGameText _gameTextSpeaker;
	SpeakerJakeUniform _jakeUniformSpeaker;
	SpeakerHarrison _harrisonSpeaker;
	SpeakerDriver _driverSpeaker;
	SpeakerShooter _shooterSpeaker;
	ASoundExt _sound1;
	int _field1FB6, _field1FB8, _field1FBA;
	int _field1FBC, _field1FBE;
	int _field1FC0, _field1FC2, _field1FC4;

	Scene410();
	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void synchronize(Serializer &s);
	virtual void signal();
	virtual void process(Event &event);
	virtual void dispatch();
};

class Scene415: public SceneExt {
	/* Objects */
	class GunInset: public FocusObject {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};
	class GunAndWig: public NamedObject {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};
	class BulletsInset: public FocusObject {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};
	class DashDrawer: public NamedObject {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};
	class TheBullets: public NamedObject {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};

	/* Items */
	class Lever: public NamedHotspot {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};
private:
	void showBullets();
	void showGunAndWig();
public:
	SequenceManager _sequenceManager;
	GunInset _gunInset;
	GunAndWig _gunAndWig;
	BulletsInset _bulletsInset;
	DashDrawer _dashDrawer;
	TheBullets _theBullets;
	NamedObject _object6;
	NamedHotspot _item1, _steeringWheel, _horn, _dashboard;
	NamedHotspot _seat, _windowLever, _item7, _seatBelt;
	Lever _lever;
	SpeakerJakeRadio _jakeRadioSpeaker;
	bool _fieldE14, _fieldE16;

	Scene415();
	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void synchronize(Serializer &s);
	virtual void signal();
	virtual void dispatch();
};

class Scene440: public SceneExt {
	/* Objects */
	class Doorway: public NamedObject {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};
	class Vechile: public NamedObject {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};
	class Lyle: public NamedObject {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};

	/* Items */
	class Item1: public NamedHotspot {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};
public:
	SequenceManager _sequenceManager;
	SpeakerGameText _gameTextSpeaker;
	Doorway _doorway;
	Vechile _vechile;
	Lyle _lyle;
	Item1 _item1;

	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void signal();
};

class Scene450: public SceneExt {
	/* Objects */
	class Object1: public NamedObjectExt {
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
	class Object4: public NamedObject {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};

	/* Items */
	class Item1: public NamedHotspot {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};
public:
	SequenceManager _sequenceManager;
	SpeakerGameText _gameTextSpeaker;
	SpeakerLyleHat _lyleHatSpeaker;
	SpeakerJakeJacket _jakeJacketSpeaker;
	SpeakerJakeUniform _jakeUniformSpeaker;
	SpeakerEugene _eugeneSpeaker;
	SpeakerWeasel _weaselSpeaker;
	SpeakerBilly _billySpeaker;
	Object1 _object1;
	NamedObject _object2;
	Object3 _object3;
	Object4 _object4;
	NamedObject _door, _counterDoor;
	NamedHotspot _exit, _interior, _shelf, _counter;
	int _field19AC, _field19AE;

	Scene450();
	virtual void synchronize(Serializer &s);
	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void signal();
	virtual void process(Event &event);
};

} // End of namespace BlueForce

} // End of namespace TsAGE

#endif
