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

#ifndef TSAGE_RINGWORLD_SCENES1_H
#define TSAGE_RINGWORLD_SCENES1_H

#include "common/scummsys.h"
#include "tsage/ringworld/ringworld_logic.h"
#include "tsage/ringworld/ringworld_speakers.h"
#include "tsage/converse.h"
#include "tsage/events.h"
#include "tsage/core.h"
#include "tsage/scenes.h"
#include "tsage/globals.h"
#include "tsage/sound.h"

namespace TsAGE {

namespace Ringworld {

using namespace TsAGE;

class Scene10 : public Scene {
	/* Actions */
	class Action1 : public Action {
	public:
		void signal() override;
	};
	class Action2 : public Action {
	public:
		void signal() override;
	};
public:
	Speaker _speakerSText;
	Speaker _speakerQText;
	Action1 _action1;
	Action2 _action2;
	SceneObject _veeshkaBody;
	SceneObject _veeshkaHead;
	SceneObject _veeshkaRightArm;
	SceneObject _centurion;
	SceneObject _leftSmoke;
	SceneObject _rightSmoke;

	void stripCallback(int v) override;
	void postInit(SceneObjectList *OwnerList = NULL) override;
};

class Scene15 : public Scene {
	/* Actions */
	class Action1 : public Action {
	public:
		void signal() override;
		void dispatch() override;
	};
public:
	Action1 _action1;
	SceneObject _ship;
	ASound _soundHandler;

	void postInit(SceneObjectList *OwnerList = NULL) override;
};

class Scene20 : public Scene {
	/* Actions */
	class Action1 : public Action {
	public:
		void signal() override;
	};
	class Action2 : public Action {
	public:
		void signal() override;
	};
	class Action3 : public Action {
	public:
		void signal() override;
	};
	class Action4 : public Action {
	public:
		void signal() override;
	};
public:
	SequenceManager _sequenceManager;
	SpeakerQText _speakerQText;
	SpeakerGameText _speakerGameText;
	Action1 _action1;
	Action2 _action2;
	Action3 _action3;
	Action4 _action4;
	SceneObject _assassinShip1;
	SceneObject _assassinShip2;
	SceneObject _laserShot1;
	SceneObject _laserShot2;
	ASound _sound;
public:
	Scene20();
	~Scene20() override {}

	void postInit(SceneObjectList *OwnerList = NULL) override;
	void signal() override;
};

class Scene30 : public Scene {
	/* Scene objects */
	// Doorway beam sensor
	class BeamObject : public SceneObject {
	public:
		void doAction(int action) override;
	};
	// Doorway object
	class DoorObject : public SceneObject {
	public:
		void doAction(int action) override;
	};
	// Kzin object
	class KzinObject : public SceneObject {
	public:
		void doAction(int action) override;
	};

	/* Actions */
	class BeamAction : public Action {
	public:
		void signal() override;
	};
	class KzinAction : public Action {
	public:
		void signal() override;
	};
	class RingAction : public Action {
	public:
		void signal() override;
	};
	class TalkAction : public Action {
	public:
		void signal() override;
	};

public:
	ASound _sound;
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
	~Scene30() override {}

	void postInit(SceneObjectList *OwnerList = NULL) override;
	void signal() override;
};

class Scene40 : public Scene {
	/* Actions */
	class Action1 : public Action {
	public:
		void signal() override;
	};
	class Action2 : public Action {
	public:
		void signal() override;
	};
	class Action3 : public Action {
	public:
		void signal() override;
	};
	class Action4 : public Action {
	public:
		void signal() override;
	};
	class Action5 : public Action {
	public:
		void signal() override;
	};
	class Action6 : public Action {
	public:
		void signal() override;
	};
	class Action7 : public Action {
	public:
		void signal() override;
	};
	class Action8 : public Action {
	public:
		void signal() override;
		void dispatch() override;
	};

	/* Objects */
	class DyingKzin : public SceneObject {
	public:
		void doAction(int action) override;
	};
	class Assassin : public SceneObject {
	public:
		void doAction(int action) override;
	};

	/* Items */
	class Item2 : public SceneHotspot {
	public:
		void doAction(int action) override;
	};
	class Item6 : public SceneHotspot {
	public:
		void doAction(int action) override;
	};
	class Item8 : public SceneHotspot {
	public:
		void doAction(int action) override;
	};
public:
	SequenceManager _sequenceManager;
	SpeakerSL _speakerSL;
	SpeakerQR _speakerQR;
	SpeakerQText _speakerQText;
	SpeakerSText _speakerSText;
	SpeakerGameText _speakerGameText;
	ASound _soundHandler;
	Action1 _action1;
	Action2 _action2;
	Action3 _action3;
	Action4 _action4;
	Action5 _action5;
	Action6 _action6;
	Action7 _action7;
	Action8 _action8;
	SceneObject _seeker;
	SceneObject _seekerTail;
	SceneObject _seekerHand;
	DyingKzin _dyingKzin;
	Assassin _assassin;
	SceneObject _doorway;
	SceneObject _leftEntrance;
	DisplayHotspot _ball;
	Item2 _statue;
	DisplayHotspot _window;
	DisplayHotspot _entrance;
	DisplayHotspot _background;
	Item6 _pedestal;
	DisplayHotspot _emerald;
	DisplayHotspot _tree;

	Scene40();
	void postInit(SceneObjectList *OwnerList = NULL) override;
	void signal() override;
	void dispatch() override;
};

class Scene50 : public Scene {
	/* Actions */
	class Action1 : public Action {
	public:
		void signal() override;
	};
	class Action2 : public Action {
	public:
		void signal() override;
	};
	class Action3 : public Action {
	public:
		void signal() override;
	};

	/* Objects */
	class Object1 : public SceneObject {
	public:
		void doAction(int action) override;
	};
	class LeftFlyCycle : public SceneObject {
	public:
		void doAction(int action) override;
	};
	class CenterFlyCycle : public SceneObject {
	public:
		void doAction(int action) override;
	};
	class RightFlyCycle : public SceneObject {
	public:
		void doAction(int action) override;
	};

public:
	SequenceManager _sequenceManager;
	Action1 _action1;
	Action2 _action2;
	Action3 _action3;
	LeftFlyCycle _leftFlyCycle;
	CenterFlyCycle _centerFlyCycle;
	RightFlyCycle _rightFlyCycle;
	Rect _doorwayRect;
	SpeakerSText _speakerSText;
	SpeakerQText _speakerQText;
	DisplayHotspot _background;
	DisplayHotspot _item1;
	DisplayHotspot _entrance;
	DisplayHotspot _bulwark;
	DisplayHotspot _tree;
	DisplayHotspot _flagstones;

	Scene50();
	void postInit(SceneObjectList *OwnerList = NULL) override;
	void signal() override;
	void dispatch() override;
};

class Scene60 : public Scene {
	class Action1 : public Action {
	public:
		void signal() override;
	};
	class Action2 : public Action {
	public:
		void signal() override;
	};
	class PrevObject : public SceneObject {
	public:
		void doAction(int action) override;
	};
	class NextObject : public SceneObject {
	public:
		void doAction(int action) override;
	};
	class ExitObject : public SceneObject {
	public:
		void doAction(int action) override;
	};
	class MessageObject : public SceneObject {
	public:
		void doAction(int action) override;
	};
	class ControlObject : public SceneObject {
	public:
		void doAction(int action) override;
	};
	class SlaveObject : public SceneObjectExt {
	public:
		void doAction(int action) override;
	};
	class MasterObject : public SceneObjectExt {
	public:
		void doAction(int action) override;
	};
	class FloppyDrive : public SceneObject {
	public:
		void doAction(int action) override;
	};
	class Item1 : public SceneHotspot {
	public:
		void doAction(int action) override;
	};
	class Item : public SceneHotspot {
	public:
		int _messageNum, _sceneMode;

		Item(int sceneRegionId, int messageNum, int sceneMode) {
			_sceneRegionId = sceneRegionId;
			_messageNum = messageNum;
			_sceneMode = sceneMode;
		}
		void doAction(int action) override;
	};

public:
	GfxButton _gfxButton;
	SequenceManager _sequenceManager;
	SpeakerQText _speakerQText;
	SpeakerSText _speakerSText;
	Action1 _action1;
	Action2 _action2;
	SceneObject _rose;
	PrevObject _prevButton;
	NextObject _nextButton;
	ExitObject _exitButton;
	MessageObject _message;
	ControlObject _controlButton;
	SlaveObject _slaveButton;
	MasterObject _masterButton;
	FloppyDrive _floppyDrive;
	SceneObject _redLights;
	Item1 _diskDrive;
	Item _dashboard;
	Item _intercomm;
	Item _viewScreen;
	Item _speedControl;
	Item _speaker;
	ASound _soundHandler1;
	ASound _soundHandler2;
	ASound _soundHandler3;

	Scene60();
	void postInit(SceneObjectList *OwnerList = NULL) override;
	void signal() override;
	void process(Event &event) override;
};

class Scene90 : public Scene {
	class Action1 : public Action {
	public:
		void signal() override;
	};
	class SeekerShip : public SceneObject {
	public:
		void doAction(int action) override;
	};
	class Guard : public SceneObject {
	public:
		void doAction(int action) override;
	};
public:
	SequenceManager _sequenceManager;
	SpeakerSText _speakerSText;
	SpeakerQText _speakerQText;
	SpeakerQL _speakerQL;
	SpeakerSR _speakerSR;
	SpeakerMText _speakerMText;
	Action1 _action1;
	SeekerShip _seekerShip;
	Guard _guard;
	DisplayObject _headGate;
	DisplayObject _emptyShip;
	DisplayObject _quinnShip;
	SceneObject _bubble;
	DisplayHotspot _guardShack;
	DisplayHotspot _shed;
	DisplayHotspot _background;
	ASound _soundHandler1, _soundHandler2;

	Scene90();

	void stripCallback(int v) override;
	void postInit(SceneObjectList *OwnerList = NULL) override;
	void signal() override;
};

class Scene95 : public Scene {
	class Action1 : public ActionExt {
	public:
		void signal() override;
	};

public:
	Action1 _action1;
	SceneObject _object1, _object2, _object3;
	ASound _soundHandler;

	Scene95();
	void postInit(SceneObjectList *OwnerList) override;
};

class Scene6100 : public Scene {
	/* Actions */
	class Action1 : public Action {
	public:
		void signal() override;
	};
	class Action2 : public Action {
	public:
		void signal() override;
	};
	class Action3 : public Action {
	public:
		void signal() override;
	};
	class Action4 : public Action {
	public:
		void signal() override;
	};
	class Action5 : public Action {
	public:
		void dispatch() override;
	};
	class GetBoxAction : public Action {
	public:
		void signal() override;
		void dispatch() override;
	};
	class Action7 : public Action {
	public:
		void signal() override;
	};

	/* Objects */
	class Object : public SceneObject {
	public:
		FloatSet _floats;

		void synchronize(Serializer &s) override;
	};
	class ProbeMover : public NpcMover {
	public:
		void dispatch() override;
	};

	/* Items */
	class Item1 : public SceneItem {
	public:
		void doAction(int action) override;
	};

public:
	Action1 _action1;
	Action2 _action2;
	Action3 _action3;
	Action4 _action4;
	Action5 _action5;
	GetBoxAction _getBoxAction;
	Action7 _action7;
	ASound _soundHandler;
	Speaker _speaker1;
	SpeakerQR _speakerQR;
	SpeakerSL _speakerSL;
	SceneObject _object1, _object2, _object3;
	Object _rocks, _probe;
	Object _sunflower1, _sunflower2, _sunflower3;
	SceneText _sceneText;
	SceneItem _item1;

	int _turnAmount, _angle, _speed, _fadePercent;
	int _hitCount;
	bool _rocksCheck;
	Object *_objList[4];
	bool _msgActive;

	Scene6100();
	void synchronize(Serializer &s) override;
	void postInit(SceneObjectList *OwnerList = NULL) override;
	void remove() override;
	void process(Event &event) override;
	void dispatch() override;
	void showMessage(const Common::String &msg, int color, Action *action);

};

} // End of namespace Ringworld

} // End of namespace TsAGE

#endif
