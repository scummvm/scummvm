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

	/* Objects */
	class Driver: public NamedObject {
	public:
		bool startAction(CursorType action, Event &event) override;
	};
	class Passenger: public NamedObject {
	public:
		bool startAction(CursorType action, Event &event) override;
	};
	class Harrison: public NamedObject {
	public:
		bool startAction(CursorType action, Event &event) override;
	};

	/* Items */
	class Motorcycle: public NamedHotspot {
	public:
		bool startAction(CursorType action, Event &event) override;
	};
	class TruckFront: public NamedHotspot {
	public:
		bool startAction(CursorType action, Event &event) override;
	};
public:
	SequenceManager _sequenceManager1, _sequenceManager2, _sequenceManager3;
	Driver _driver;
	Passenger _passenger;
	Harrison _harrison;
	NamedObject _patrolCar, _object5, _object6;
	NamedHotspot _background;
	Motorcycle _motorcycle;
	NamedHotspot _truckBack;
	TruckFront _truckFront;
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
	int _action1Count, _talkCount;
	bool _harrissonTalkFl;
	bool _cuffedDriverFl;
	bool _cuffedPassengerFl;
	bool _getDriverFl;
	bool _driverOutOfTruckFl, _harrisonMovedFl;

	Scene410();
	void postInit(SceneObjectList *OwnerList = NULL) override;
	void synchronize(Serializer &s) override;
	void signal() override;
	void process(Event &event) override;
	void dispatch() override;
};

class Scene415: public SceneExt {
	/* Objects */
	class GunInset: public FocusObject {
	public:
		bool startAction(CursorType action, Event &event) override;
		void remove() override;
	};
	class GunAndWig: public NamedObject {
	public:
		bool startAction(CursorType action, Event &event) override;
	};
	class BulletsInset: public FocusObject {
	public:
		bool startAction(CursorType action, Event &event) override;
		void remove() override;
	};
	class DashDrawer: public NamedObject {
	public:
		bool startAction(CursorType action, Event &event) override;
	};
	class TheBullets: public NamedObject {
	public:
		bool startAction(CursorType action, Event &event) override;
	};

	/* Items */
	class Lever: public NamedHotspot {
	public:
		bool startAction(CursorType action, Event &event) override;
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
	NamedObject _animatedSeat;
	NamedHotspot _item1, _steeringWheel, _horn, _dashboard;
	NamedHotspot _seat, _windowLever, _item7, _seatBelt;
	Lever _lever;
	SpeakerJakeRadio _jakeRadioSpeaker;
	bool _scoreWigRapFlag, _scoreBulletRapFlag;

	Scene415();
	void postInit(SceneObjectList *OwnerList = NULL) override;
	void synchronize(Serializer &s) override;
	void signal() override;
	void dispatch() override;
};

class Scene440: public SceneExt {
	/* Objects */
	class Doorway: public NamedObject {
	public:
		bool startAction(CursorType action, Event &event) override;
	};
	class Vechile: public NamedObject {
	public:
		bool startAction(CursorType action, Event &event) override;
	};
	class Lyle: public NamedObject {
	public:
		bool startAction(CursorType action, Event &event) override;
	};

	/* Items */
	class Item1: public NamedHotspot {
	public:
		bool startAction(CursorType action, Event &event) override;
	};
public:
	SequenceManager _sequenceManager;
	SpeakerGameText _gameTextSpeaker;
	Doorway _doorway;
	Vechile _vechile;
	Lyle _lyle;
	Item1 _item1;

	void postInit(SceneObjectList *OwnerList = NULL) override;
	void signal() override;
};

class Scene450: public SceneExt {
	/* Objects */
	class Weasel: public NamedObjectExt {
	public:
		bool startAction(CursorType action, Event &event) override;
	};
	class Object2: public NamedObject {
	public:
		bool startAction(CursorType action, Event &event) override;
	};
	class PinBoy: public NamedObject {
	public:
		bool startAction(CursorType action, Event &event) override;
	};
	class Manager: public NamedObject {
	public:
		bool startAction(CursorType action, Event &event) override;
	};

	/* Items */
	class Exit: public NamedHotspot {
	public:
		bool startAction(CursorType action, Event &event) override;
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
	Weasel _weasel;
	NamedObject _object2;
	PinBoy _pinBoy;
	Manager _manager;
	NamedObject _door, _counterDoor;
	Exit _exit;
	NamedHotspot _interior, _shelf, _counter;
	bool _managerCallsWeaselFl;
	bool _talkManagerFl;

	Scene450();
	void synchronize(Serializer &s) override;
	void postInit(SceneObjectList *OwnerList = NULL) override;
	void signal() override;
	void process(Event &event) override;
};

} // End of namespace BlueForce

} // End of namespace TsAGE

#endif
