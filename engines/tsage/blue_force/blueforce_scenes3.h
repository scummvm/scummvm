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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
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

		bool startAction(CursorType action, Event &event) override;
	};
	class Object19: public NamedObject {
	public:
		bool startAction(CursorType action, Event &event) override;
	};

	/* Items */
	class Item1: public NamedHotspot {
	public:
		bool startAction(CursorType action, Event &event) override;
	};
	class Item2: public NamedHotspot {
	public:
		bool startAction(CursorType action, Event &event) override;
	};
	class Item14: public NamedHotspot {
	public:
		bool startAction(CursorType action, Event &event) override;
	};
	class Item15: public NamedHotspot {
	public:
		bool startAction(CursorType action, Event &event) override;
	};

	/* Actions */
	class Action1: public Action {
	public:
		void signal() override;
	};
	class Action2: public Action {
	public:
		void signal() override;
	};
	class Action3: public Action {
	public:
		void signal() override;
	};
	class Action4: public Action {
	public:
		void signal() override;
	};
	class Action5: public Action {
	public:
		void signal() override;
	};
private:
	void setupInspection();
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
	void postInit(SceneObjectList *OwnerList = NULL) override;
	void signal() override;
	void process(Event &event) override;
	void dispatch() override;
};

class Scene315: public SceneExt {
	/* Objects */
	class BulletinMemo: public NamedObject {
	public:
		bool startAction(CursorType action, Event &event) override;
	};
	class Object2: public NamedObject {
	public:
		bool startAction(CursorType action, Event &event) override;
	};
	class ATFMemo: public NamedObject {
	public:
		bool startAction(CursorType action, Event &event) override;
	};

	/* Items */
	class Barry: public NamedHotspot {
	public:
		bool startAction(CursorType action, Event &event) override;
	};
	class SutterSlot: public NamedHotspot {
	public:
		bool startAction(CursorType action, Event &event) override;
	};
	class Sign: public NamedHotspot {
	public:
		bool startAction(CursorType action, Event &event) override;
	};
	class BulletinBoard: public NamedHotspot {
	public:
		bool startAction(CursorType action, Event &event) override;
	};
	class CleaningKit: public NamedHotspot {
	public:
		bool startAction(CursorType action, Event &event) override;
	};
	class BriefingMaterial: public NamedHotspot {
	public:
		bool startAction(CursorType action, Event &event) override;
	};
	class WestExit: public NamedHotspot {
	public:
		bool startAction(CursorType action, Event &event) override;
	};
	class SouthWestExit: public NamedHotspot {
	public:
		bool startAction(CursorType action, Event &event) override;
	};

	/* Actions */
	class Action1: public Action {
	public:
		void signal() override;
	};
public:
	SequenceManager _sequenceManager;
	SpeakerGameText _gameTextSpeaker;
	SpeakerSutter _sutterSpeaker;
	SpeakerHarrison _harrisonSpeaker;
	SpeakerJakeJacket _jakeJacketSpeaker;
	SpeakerJakeUniform _jakeUniformSpeaker;
	SpeakerJailer _jailerSpeaker;
	Barry _barry;
	SutterSlot _sutterSlot;
	NamedHotspot _item3;
	Sign _sign;
	BulletinBoard _bulletinBoard;
	BulletinMemo _bulletinMemo;
	Object2 _object2;
	ATFMemo _atfMemo;
	SceneObject _object4, _object5, _object6;
	SceneObject _object7, _object8, _object9;
	NamedHotspot _item6, _item7, _item8, _item9;
	NamedHotspot _item10, _item11, _item12, _item13;
	CleaningKit _cleaningKit;
	BriefingMaterial _briefingMaterial;
	WestExit _westExit;
	SouthWestExit _swExit;
	Action1 _action1;
	int _stripNumber;
	int _field1398;
	int _invGreenCount, _bookGreenCount, _invGangCount;
	int _bookGangCount, _field1B6C, _field139C;
	bool _field1B68, _doorOpened;
	CursorType _currentCursor;

	Scene315();
	void synchronize(Serializer &s) override;
	void postInit(SceneObjectList *OwnerList = NULL) override;
	void signal() override;
	void process(Event &event) override;
	void dispatch() override;
};

class Scene325: public SceneExt {
	/* Items */
	class Item1: public NamedHotspot {
	public:
		bool startAction(CursorType action, Event &event) override;
	};
public:
	SequenceManager _sequenceManager;
	SpeakerGameText _gameTextSpeaker;
	SpeakerPSutter _PSutterSpeaker;
	Item1 _item1;
	NamedObject _object1, _object2, _object3, _object4, _object5;

	void postInit(SceneObjectList *OwnerList = NULL) override;
	void signal() override;
};

class Scene330: public SceneExt {
	class Timer1: public Timer {
	public:
		void signal() override;
	};
public:
	SequenceManager _sequenceManager;
	NamedObject _object1;
	SceneObject _object2;
	ASoundExt _sound1, _sound2;
	Timer1 _timer;
	int _seqNumber;

	Scene330();
	void synchronize(Serializer &s) override;
	void postInit(SceneObjectList *OwnerList = NULL) override;
	void remove() override;
	void signal() override;
};

class Scene340: public PalettedScene {
	/* Actions */
	class Action1: public Action {
	public:
		void signal() override;
	};
	class Action2: public Action {
	public:
		void signal() override;
	};
	class Action3: public Action {
	public:
		void signal() override;
	};
	class Action4: public Action {
	public:
		void signal() override;
	};
	class Action5: public Action {
	public:
		void signal() override;
	};
	class Action6: public Action {
	public:
		void signal() override;
	};
	class Action7: public Action {
	public:
		void signal() override;
	};
	class Action8: public Action {
	public:
		void signal() override;
		void process(Event &event) override;
	};

	/* Items */
	class Item1: public NamedHotspot {
	public:
		bool startAction(CursorType action, Event &event) override;
	};
	class WestExit: public NamedHotspot {
	public:
		bool startAction(CursorType action, Event &event) override;
	};
	class SouthWestExit: public NamedHotspot {
	public:
		bool startAction(CursorType action, Event &event) override;
	};
	class NorthExit: public NamedHotspot {
	public:
		bool startAction(CursorType action, Event &event) override;
	};

	/* Objects */
	class Child: public NamedObject {
	public:
		bool startAction(CursorType action, Event &event) override;
	};
	class Woman: public NamedObject {
	public:
		bool startAction(CursorType action, Event &event) override;
	};
	class Harrison: public NamedObject {
	public:
		bool startAction(CursorType action, Event &event) override;
	};

	/* Timers */
	class Timer2: public Timer {
	public:
		void signal() override;
	};
public:
	SequenceManager _sequenceManager1, _sequenceManager2, _sequenceManager3;
	Child _child;
	Woman _woman;
	Harrison _harrison;
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
	int _seqNumber1, _womanDialogCount, _backupPresent;

	Scene340();
	void synchronize(Serializer &s) override;
	void postInit(SceneObjectList *OwnerList = NULL) override;
	void remove() override;
	void signal() override;
	void process(Event &event) override;
	void dispatch() override;
};

class Scene342: public PalettedScene {
	/* Items */
	class Item1: public NamedHotspot {
	public:
		bool startAction(CursorType action, Event &event) override;
	};
	class WestExit: public NamedHotspot {
	public:
		bool startAction(CursorType action, Event &event) override;
	};
	class SouthWestExit: public NamedHotspot {
	public:
		bool startAction(CursorType action, Event &event) override;
	};
	class NorthExit: public NamedHotspot {
	public:
		bool startAction(CursorType action, Event &event) override;
	};

	/* Objects */
	class Lyle: public NamedObject {
	public:
		bool startAction(CursorType action, Event &event) override;
	};

	/* Timers */
	class Timer1: public Timer {
	public:
		void signal() override;
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
	void synchronize(Serializer &s) override;
	void postInit(SceneObjectList *OwnerList = NULL) override;
	void remove() override;
	void signal() override;
	void process(Event &event) override;
	void dispatch() override;
};

class Scene350: public SceneExt {
	/* Items */
	class FireBox: public NamedHotspot {
	public:
		bool startAction(CursorType action, Event &event) override;
	};
	class Yacht: public NamedHotspot {
	public:
		bool _flag;

		Common::String getClassName() override { return "Scene350_Item6"; }
		void synchronize(Serializer &s) override;
		bool startAction(CursorType action, Event &event) override;
	};
	class SouthWestExit: public NamedHotspot {
	public:
		bool startAction(CursorType action, Event &event) override;
	};

	/* Objects */
	class Hook: public NamedObject {
	public:
		bool startAction(CursorType action, Event &event) override;
	};
	class FireboxInset: public FocusObject {
	public:
		bool startAction(CursorType action, Event &event) override;
	};

	/* Timers */
	class Timer1: public Timer {
	public:
		void signal() override;
	};
public:
	SequenceManager _sequenceManager1, _sequenceManager2, _sequenceManager3;
	NamedObject _harrison;
	SceneObject _yachtDoor;
	BackgroundSceneObject _yachtBody;
	Hook _hook;
	FireboxInset _fireBoxInset;
	NamedHotspot _item1, _item2, _item3, _item4;
	FireBox _fireBox;
	Yacht _yacht;
	SouthWestExit _swExit;
	SpeakerGameText _gameTextSpeaker;
	SpeakerJakeUniform _jakeUniformSpeaker;
	SpeakerHarrison _harrisonSpeaker;
	ASoundExt _sound1, _sound2;
	Timer1 _timer1;
	int _field1D44, _field1D46;

	Scene350();
	void postInit(SceneObjectList *OwnerList = NULL) override;
	void remove() override;
	void signal() override;
	void process(Event &event) override;
	void checkGun() override;
};

class Scene355: public PalettedScene {
	/* Objects */
	class Doorway: public NamedObject {
	public:
		int _mode1356Count, _talkCount;
		bool _onDuty;

		Doorway() { _mode1356Count = _talkCount = 0; _onDuty = false; }
		Common::String getClassName() override { return "Scene355_Doorway"; }
		void synchronize(Serializer &s) override;
		bool startAction(CursorType action, Event &event) override;
	};
	class Locker: public NamedObject {
	public:
		bool startAction(CursorType action, Event &event) override;
	};
	class LockerInset: public FocusObject {
	public:
		bool startAction(CursorType action, Event &event) override;
	};
	class Object5: public NamedObject {
	public:
		bool startAction(CursorType action, Event &event) override;
	};
	class Green: public NamedObjectExt {
	public:
		bool startAction(CursorType action, Event &event) override;
	};
	class Lyle: public NamedObjectExt {
	public:
		bool startAction(CursorType action, Event &event) override;
	};
	class Object8: public NamedObject {
	public:
		bool startAction(CursorType action, Event &event) override;
	};

	/* Items */
	class Item1: public SceneHotspot {
	public:
		bool startAction(CursorType action, Event &event) override;
	};
	class Item2: public NamedHotspot {
	public:
		bool startAction(CursorType action, Event &event) override;
	};
	class Item3: public SceneHotspotExt {
	public:
		bool startAction(CursorType action, Event &event) override;
	};
	class Item4: public SceneHotspot {
	public:
		bool startAction(CursorType action, Event &event) override;
	};
	class Pouch: public NamedHotspot {
	public:
		bool startAction(CursorType action, Event &event) override;
	};
	class Item11: public NamedHotspot {
	public:
		bool startAction(CursorType action, Event &event) override;
	};
	class RentalExit: public NamedHotspot {
	public:
		bool startAction(CursorType action, Event &event) override;
	};

	/* Actions */
	class Action1: public Action {
	public:
		void signal() override;
	};
	class Action2: public Action {
	public:
		void signal() override;
	};
public:
	SequenceManager _sequenceManager;
	SpeakerGameText _gameTextSpeaker;
	SpeakerJakeUniform _jakeUniformSpeaker;
	SpeakerJakeJacket _jakeJacketSpeaker;
	SpeakerHarrison _harrisonSpeaker;
	SpeakerLyleHat _lyleHatSpeaker;
	SpeakerGreen _greenSpeaker;
	NamedObject _harrison;
	Doorway _doorway;
	Locker _locker;
	LockerInset _lockerInset;
	Object5 _object5;
	Green _green;
	Lyle _lyle;
	Object8 _object8;
	NamedObject _object9, _object10, _object11;
	Item1 _item1;
	Item2 _item2;
	Item3 _item3;
	Item4 _item4;
	Pouch _pouch;
	NamedHotspot _item6, _item7, _item8;
	NamedHotspot _item9, _item10;
	Item11 _item11;
	RentalExit _rentalExit;
	ASoundExt _sound1, _sound2, _sound3;
	Action1 _action1;
	Action2 _action2;
	int _nextSceneMode;
	bool _modeFlag;

	Scene355();
	void synchronize(Serializer &s) override;
	void postInit(SceneObjectList *OwnerList = NULL) override;
	void signal() override;
	void process(Event &event) override;
	void dispatch() override;

	void setMode(bool mode, int sceneMode);
};

class Scene360: public SceneExt {
	/* Objects */
	class SlidingDoor: public NamedObject {
	public:
		bool startAction(CursorType action, Event &event) override;
	};
	class Window: public NamedObject {
	public:
		bool startAction(CursorType action, Event &event) override;
	};
	class Object4: public NamedObject {
	public:
		bool startAction(CursorType action, Event &event) override;
	};
	class BaseballCards: public NamedObject {
	public:
		bool startAction(CursorType action, Event &event) override;
	};
	class Harrison: public NamedObject {
	public:
		bool startAction(CursorType action, Event &event) override;
	};
	class Object7: public NamedObject {
	public:
		bool startAction(CursorType action, Event &event) override;
	};

	/* Items */
	class Item1: public SceneHotspot {
	public:
		bool startAction(CursorType action, Event &event) override;
	};
	class Item2: public SceneHotspot {
	public:
		bool startAction(CursorType action, Event &event) override;
	};
	class Item3: public SceneHotspot {
	public:
		bool startAction(CursorType action, Event &event) override;
	};
	class Barometer: public SceneHotspot {
	public:
		bool startAction(CursorType action, Event &event) override;
	};

	/* Actions */
	class Action1: public Action {
	public:
		void signal() override;
	};
public:
	SequenceManager _sequenceManager1, _sequenceManager2;
	SpeakerGameText _gameTextSpeaker;
	SpeakerJakeUniform _jakeUniformSpeaker;
	SpeakerHarrison _harrisonSpeaker;
	SpeakerGreen _greenSpeaker;
	SlidingDoor _slidingDoor;
	SceneObject _object2;
	Window _window;
	Object4 _object4;
	BaseballCards _baseballCards;
	Harrison _harrison;
	Object7 _object7;
	Item1 _item1;
	Item2 _item2;
	Item3 _item3;
	Barometer _barometer;
	Action1 _action1;
	ASound _sound1;

	void synchronize(Serializer &s) override;
	void postInit(SceneObjectList *OwnerList = NULL) override;
	void signal() override;
	void process(Event &event) override;
	void dispatch() override;
};

class Scene370: public SceneExt {
	/* Items */
	class Item1: public NamedHotspot {
	public:
		bool startAction(CursorType action, Event &event) override;
	};
	class Item6: public SceneHotspot {
	public:
		bool startAction(CursorType action, Event &event) override;
	};
	class Exit: public NamedHotspot {
	public:
		bool startAction(CursorType action, Event &event) override;
	};

	/* Objects */
	class GreensGun: public NamedObject {
	public:
		bool startAction(CursorType action, Event &event) override;
	};
	class Green: public NamedObject2 {
	public:
		bool startAction(CursorType action, Event &event) override;
	};
	class Harrison: public NamedObject {
	public:
		bool startAction(CursorType action, Event &event) override;
	};
	class Laura: public NamedObject {
	public:
		bool startAction(CursorType action, Event &event) override;
	};

	/* Speakers */
	class SpeakerLaura370: public SpeakerLaura {
	public:
		void setText(const Common::String &msg) override;
	};
public:
	SequenceManager _sequenceManager;
	SpeakerGameText _gameTextSpeaker;
	SpeakerJakeUniform _jakeUniformSpeaker;
	SpeakerLaura _lauraSpeaker;
	SpeakerLauraHeld _lauraHeldSpeaker;
	SpeakerGreen _greenSpeaker;
	SpeakerHarrison _harrisonSpeaker;
	GreensGun _greensGun;
	Green _green;
	Harrison _harrison;
	Laura _laura;
	NamedObject _object5;
	SceneObject _object6;
	Item1 _item1;
	NamedHotspot _item2, _item3, _item4, _item5;
	Item6 _item6;
	Exit _exit;
	ASound _sound1;

	void postInit(SceneObjectList *OwnerList = NULL) override;
	void signal() override;
	void process(Event &event) override;
	void dispatch() override;
};

class Scene380: public SceneExt {
	/* Objects */
	class Vechile: public NamedObjectExt {
	public:
		bool startAction(CursorType action, Event &event) override;
	};
	class Door: public NamedObjectExt {
	public:
		bool startAction(CursorType action, Event &event) override;
	};
public:
	SequenceManager _sequenceManager;
	SpeakerGameText _gameTextSpeaker;
	Vechile _vechile;
	Door _door;
	NamedHotspot _item1, _item2, _item3, _item4, _item5;
	NamedHotspot _item6, _item7, _item8, _item9;

	void postInit(SceneObjectList *OwnerList = NULL) override;
	void signal() override;
};

class Scene385: public SceneExt {
	/* Items */
	class Exit: public NamedHotspot {
	public:
		bool startAction(CursorType action, Event &event) override;
	};

	/* Objects */
	class Door: public NamedObject {
	public:
		bool startAction(CursorType action, Event &event) override;
	};
	class Jim: public NamedObject {
	public:
		bool startAction(CursorType action, Event &event) override;
	};
	class Dezi: public NamedObject {
	public:
		bool startAction(CursorType action, Event &event) override;
	};

	/* Actions */
	class Action1: public Action {
	public:
		void signal() override;
	};
	class Action2: public Action {
	public:
		void signal() override;
	};
public:
	SequenceManager _sequenceManager;
	Action1 _action1;
	Action2 _action2;
	Door _door;
	Jim _jim;
	Dezi _dezi;
	SpeakerGameText _gameTextSpeaker;
	SpeakerJim _jimSpeaker;
	SpeakerDezi _deziSpeaker;
	SpeakerJake385 _jake385Speaker;
	NamedHotspot _item1, _item2, _item3, _item4, _item5;
	Exit _exit;
	int _talkAction;
	bool _jimFlag;

	Scene385();
	void synchronize(Serializer &s) override;
	void postInit(SceneObjectList *OwnerList = NULL) override;
	void signal() override;
	void process(Event &event) override;
	void dispatch() override;
};

class Scene390: public SceneExt {
	/* Items */
	class BookingForms: public NamedHotspotExt {
	public:
		bool startAction(CursorType action, Event &event) override;
	};

	/* Objects */
	class Green: public NamedObjectExt {
	public:
		bool startAction(CursorType action, Event &event) override;
	};
	class Object2: public NamedObjectExt {
	public:
		bool startAction(CursorType action, Event &event) override;
	};
	class Object3: public NamedObject {
	public:
		bool startAction(CursorType action, Event &event) override;
	};
	class GangMember1: public NamedObjectExt {
	public:
		bool startAction(CursorType action, Event &event) override;
	};
	class GangMember2: public NamedObjectExt {
	public:
		bool startAction(CursorType action, Event &event) override;
	};
	class Door: public NamedObject {
	public:
		bool startAction(CursorType action, Event &event) override;
	};

	/* Actions */
	class Action1: public Action {
	public:
		void signal() override;
	};
public:
	SequenceManager _sequenceManager;
	SpeakerGameText _gameTextSpeaker;
	SpeakerJakeUniform _jakeUniformSpeaker;
	SpeakerJakeJacket _jakeJacketSpeaker;
	SpeakerGreen _greenSpeaker;
	SpeakerJailer _jailerSpeaker;
	SpeakerDriver _driverSpeaker;
	SpeakerShooter _shooterSpeaker;
	Action1 _action1;
	Green _green;
	Object2 _object2;
	Object3 _object3;
	GangMember1 _gangMember1;
	GangMember2 _gangMember2;
	Door _door;
	NamedHotspot _item1, _item3, _item4;
	BookingForms _bookingForms;

	void postInit(SceneObjectList *OwnerList = NULL) override;
	void signal() override;
};

} // End of namespace BlueForce

} // End of namespace TsAGE

#endif
