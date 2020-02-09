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

#ifndef TSAGE_RINGWORLD2_SCENES1_H
#define TSAGE_RINGWORLD2_SCENES1_H

#include "common/scummsys.h"
#include "tsage/converse.h"
#include "tsage/events.h"
#include "tsage/core.h"
#include "tsage/scenes.h"
#include "tsage/globals.h"
#include "tsage/sound.h"
#include "tsage/ringworld2/ringworld2_logic.h"
#include "tsage/ringworld2/ringworld2_speakers.h"

namespace TsAGE {

namespace Ringworld2 {

using namespace TsAGE;

class Scene1000 : public SceneExt {
public:
	SequenceManager _sequenceManager1;
	SequenceManager _sequenceManager2;
	SpeakerGameText _gameTextSpeaker;
	AnimationPlayer _animationPlayer;

	int _animCounter;
	bool _forceCheckAnimationFl;
public:
	Scene1000();

	void postInit(SceneObjectList *OwnerList = NULL) override;
	void remove() override;
	void signal() override;
	void dispatch() override;
};

class Scene1010 : public SceneExt {
public:
	SequenceManager _sequenceManager;

	void postInit(SceneObjectList *OwnerList = NULL) override;
	void signal() override;
};

class Scene1020 : public SceneExt {
public:
	SequenceManager _sequenceManager;

	void postInit(SceneObjectList *OwnerList = NULL) override;
	void signal() override;
	void dispatch() override;
};

class Scene1100 : public SceneExt {
	class Seeker : public SceneActor {
	public:
		bool startAction(CursorType action, Event &event) override;
	};
	class Trooper : public SceneActor {
	public:
		bool startAction(CursorType action, Event &event) override;
	};
	class Chief : public SceneActor {
	public:
		bool startAction(CursorType action, Event &event) override;
	};

public:
	int _nextStripNum, _paletteRefreshStatus;
	SpeakerSeeker1100 _seekerSpeaker;
	SpeakerQuinn1100 _quinnSpeaker;
	SpeakerChief1100 _chiefSpeaker;
	ScenePalette _palette1;
	NamedHotspot _background;
	NamedHotspot _sky;
	NamedHotspot _fuana1;
	NamedHotspot _fauna2;
	NamedHotspot _bouldersBlockingCave;
	NamedHotspot _boulders;
	NamedHotspot _trail;
	SceneActor _ship;
	SceneActor _cloud;
	SceneActor _shipFormation;
	SceneActor _shipFormationShadow;
	SceneActor _shotImpact1;
	SceneActor _shotImpact2;
	SceneActor _shotImpact3;
	SceneActor _shotImpact4;
	SceneActor _shotImpact5;
	SceneActor _laserShot;
	SceneActor _animation;	// Used for cliff collapse and ship theft
	SceneActor _leftImpacts;
	SceneActor _runningGuy1;
	SceneActor _runningGuy2;
	SceneActor _runningGuy3;
	BackgroundSceneObject _rightLandslide;
	BackgroundSceneObject _purplePlant;
	Seeker _seeker;
	Trooper _trooper;
	Chief _chief;
	SequenceManager _sequenceManager1;
	SequenceManager _sequenceManager2;
	SequenceManager _sequenceManager3;

	Scene1100();
	void synchronize(Serializer &s) override;

	void postInit(SceneObjectList *OwnerList = NULL) override;
	void remove() override;
	void signal() override;
	void dispatch() override;
	void saveCharacter(int characterIndex) override;
};

class Scene1500 : public SceneExt {
public:
	SceneActor _starship;
	SceneActor _starshipShadow;
	SceneActor _smallShip;
	SceneActor _smallShipShadow;
	SequenceManager _sequenceManager;

	void postInit(SceneObjectList *OwnerList = NULL) override;
	void remove() override;
	void signal() override;
	void dispatch() override;
};

class Scene1525 : public SceneExt {
public:
	SequenceManager _sequenceManager;

	void postInit(SceneObjectList *OwnerList = NULL) override;
	void signal() override;
};

class Scene1530 : public SceneExt {
public:
	SpeakerQuinn _quinnSpeaker;
	SpeakerSeeker _seekerSpeaker;
	SceneActor _seeker;
	SceneActor _leftReactor;
	SceneActor _rightReactor;

	SequenceManager _sequenceManager;

	void postInit(SceneObjectList *OwnerList = NULL) override;
	void signal() override;
	void dispatch() override;
};

class Scene1550 : public SceneExt {
	class Wall : public SceneActor {
	public:
		void setupWall(int frameNumber, int strip);
	};

	class Junk : public SceneActor {
	public:
		int _junkNumber;

		Junk();
		void synchronize(Serializer &s) override;

		bool startAction(CursorType action, Event &event) override;
	};

	class ShipComponent : public SceneActor {
	public:
		int _componentId;

		ShipComponent();
		void synchronize(Serializer &s) override;

		bool startAction(CursorType action, Event &event) override;
		void setupShipComponent(int componentId);
	};

	class DishControlsWindow : public ModalWindow {
		class DishControl : public SceneActor {
		public:
			int _controlId;

			DishControl();
			void synchronize(Serializer &s) override;

			bool startAction(CursorType action, Event &event) override;
		};
	public:
		byte _field20;
		SceneActor _areaActor;
		DishControl _button;
		DishControl _lever;

		void remove() override;
		void setup2(int visage, int stripFrameNum, int frameNum, int posX, int posY) override;
	};

	class WorkingShip : public NamedHotspot {
	public:
		bool startAction(CursorType action, Event &event) override;
	};

	class Wreckage : public SceneActor {
	public:
		bool startAction(CursorType action, Event &event) override;
	};

	class Companion : public SceneActor {
	public:
		bool startAction(CursorType action, Event &event) override;
	};

	class AirBag : public SceneActor {
	public:
		bool startAction(CursorType action, Event &event) override;
	};

	class Joystick : public SceneActor {
	public:
		bool startAction(CursorType action, Event &event) override;
	};

	class Gyroscope : public SceneActor {
	public:
		bool startAction(CursorType action, Event &event) override;
	};

	class DiagnosticsDisplay : public SceneActor {
	public:
		bool startAction(CursorType action, Event &event) override;
	};

	class DishTower : public SceneActor {
	public:
		bool startAction(CursorType action, Event &event) override;
	};

	class Dish : public SceneActor {
	public:
		bool startAction(CursorType action, Event &event) override;
	};

public:
	SpeakerQuinn _quinnSpeaker;
	SpeakerSeeker _seekerSpeaker;
	WorkingShip _intactHull1, _intactHull2;
	SceneHotspot _background;
	SceneActor _wreckage2;	// also used for Lance of Truth landing strut
	SceneActor _wreckage3;
	SceneActor _wreckage4;
	SceneActor _walkway;
	SceneActor _dishTowerShadow;
	Wreckage _wreckage;
	Companion _companion;
	AirBag _airbag;
	Joystick _joystick;
	Gyroscope _gyroscope;
	DiagnosticsDisplay _diagnosticsDisplay;
	DishTower _dishTower;
	Dish _dish;
	Junk _junk[8];
	Wall _wallCorner1;
	Wall _northWall;	// Is also reused for landing strip
	Wall _wallCorner2;
	Wall _westWall;		// Is also reused for left hand space
	Wall _eastWall;
	Wall _southWall;
	ShipComponent _shipComponents[8];
	DishControlsWindow _dishControlsWindow;
	SequenceManager _sequenceManager1;
	SequenceManager _sequenceManager2;

	bool _dontExit;
	int _wallType;
	int _dishMode;
	int _sceneResourceId;
	int _walkRegionsId;

	Scene1550();
	void synchronize(Serializer &s) override;
	void enterArea();

	void postInit(SceneObjectList *OwnerList = NULL) override;
	void signal() override;
	void process(Event &event) override;
	void dispatch() override;
	void saveCharacter(int characterIndex) override;
};

class Scene1575 : public SceneExt {
	class Button : public NamedHotspot {
	public:
		int _buttonId;
		bool _pressed;

		Button();
		void synchronize(Serializer &s) override;
		void initButton(int buttonId);

		void process(Event &event) override;
		bool startAction(CursorType action, Event &event) override;
	};
public:
	int _field412;
	int _field414;
	int _field416;
	int _field418;
	int _field41A;
	Button _button1;
	Button _button2;
	Button _button3;
	Button _button4;
	Button _button5;
	Button _button6;
	SceneActor _actor1;
	SceneActor _actor2;
	SceneActor _actor3;
	SceneActor _actor4;
	SceneActor _actor5;
	SceneActor _actor6;
	SceneActor _actor7;
	SceneActor _actor8;
	SceneActor _actor9;
	SceneActor _actor10;
	SceneActor _actor11;
	SceneActor _actor12;
	SceneActor _actor13;
	SceneActor _actor14;
	SceneActor _actor15;
	SceneActor _arrActor[17];
	SequenceManager _sequenceManager1;

	Scene1575();
	void synchronize(Serializer &s) override;

	void postInit(SceneObjectList *OwnerList = NULL) override;
	void remove() override;
	void signal() override;
	void process(Event &event) override;
	void dispatch() override;
};

class Scene1580 : public SceneExt {
	class JoystickPlug : public NamedHotspot {
	public:
		bool startAction(CursorType action, Event &event) override;
	};
	class ScreenSlot : public NamedHotspot {
	public:
		bool startAction(CursorType action, Event &event) override;
	};

	class Joystick : public SceneActor {
	public:
		bool startAction(CursorType action, Event &event) override;
	};
	class Screen : public SceneActor {
	public:
		bool startAction(CursorType action, Event &event) override;
	};
	class StorageCompartment : public SceneActor {
	public:
		bool startAction(CursorType action, Event &event) override;
	};
	class HatchButton : public SceneActor {
	public:
		bool startAction(CursorType action, Event &event) override;
	};
	class ThrusterValve : public SceneActor {
	public:
		bool startAction(CursorType action, Event &event) override;
	};
	class Ignitor : public SceneActor {
	public:
		bool startAction(CursorType action, Event &event) override;
	};
public:
	SpeakerQuinn _quinnSpeaker;
	SpeakerSeeker _seekerSpeaker;
	JoystickPlug _joystickPlug;
	ScreenSlot _screenSlot;
	NamedHotspot _background;
	SceneActor _screenDisplay;
	SceneActor _arrActor[8];
	Joystick _joystick;
	Screen _screen;
	StorageCompartment _storageCompartment;
	HatchButton _hatchButton;
	ThrusterValve _thrusterValve;
	Ignitor _ignitor;
	SequenceManager _sequenceManager;

	Scene1580();
	void synchronize(Serializer &s) override;

	void postInit(SceneObjectList *OwnerList = NULL) override;
	void signal() override;
};

class Scene1625 : public SceneExt {
	class Wire : public SceneActor {
	public:
		bool startAction(CursorType action, Event &event) override;
	};
public:
	SpeakerMiranda1625 _mirandaSpeaker;
	SpeakerTeal1625 _tealSpeaker;
	SpeakerSoldier1625 _soldierSpeaker;
	NamedHotspot _background;
	SceneActor _teal;
	SceneActor _tealHead;
	SceneActor _mirandaMouth;
	SceneActor _glass;
	SceneActor _wristRestraints;
	SceneActor _tealRightArm;
	Wire _wire;
	SequenceManager _sequenceManager;

	Scene1625();
	void synchronize(Serializer &s) override;

	void postInit(SceneObjectList *OwnerList = NULL) override;
	void remove() override;
	void signal() override;
	void process(Event &event) override;
};

class Scene1700 : public SceneExt {
	class RimTransport : public SceneActor {
	public:
		bool startAction(CursorType action, Event &event) override;
	};
	class Companion : public SceneActor {
	public:
		bool startAction(CursorType action, Event &event) override;
	};

	class NorthExit : public SceneExit {
	public:
		void changeScene() override;
	};
	class SouthExit : public SceneExit {
	public:
		void changeScene() override;
	};
	class WestExit : public SceneExit {
	public:
		void changeScene() override;
	};
public:
	SpeakerQuinn _quinnSpeaker;
	SpeakerSeeker _seekerSpeaker;
	NamedHotspot _surface;
	NamedHotspot _background;
	SceneActor _playerShadow;
	SceneActor _companionShadow;
	SceneActor _slabWest;
	SceneActor _slabEast;
	SceneActor _slabShadowWest;
	SceneActor _slabShadowEast;
	SceneActor _westPlatform;
	SceneActor _rimTransportDoor;
	SceneActor _ledgeHopper;
	SceneActor _hatch;
	RimTransport _rimTransport;
	Companion _companion;
	NorthExit _northExit;
	SouthExit _southExit;
	WestExit _westExit;
	SequenceManager _sequenceManager;

	bool _walkFlag;

	Scene1700();
	void synchronize(Serializer &s) override;
	void enterArea();

	void postInit(SceneObjectList *OwnerList = NULL) override;
	void remove() override;
	void signal() override;
};

class Scene1750 : public SceneExt {
	class SpeedSlider : public SceneActor {
	public:
		int _incrAmount;
		int _xp;
		int _ys;
		int _height;
		int _thumbHeight;
		bool _mouseDown;

		SpeedSlider();
		void synchronize(Serializer &s) override;
		void setupSlider(int incrAmount, int xp, int ys, int height, int thumbHeight);
		void calculateSlider();

		void process(Event &event) override;
		bool startAction(CursorType action, Event &event) override;
	};

	class Button : public SceneActor {
	public:
		int _buttonId;

		Button();
		void synchronize(Serializer &s) override;

		bool startAction(CursorType action, Event &event) override;
	};

public:
	NamedHotspot _background;
	NamedHotspot _redLightsDescr;
	NamedHotspot _greenLights;
	NamedHotspot _frontView;
	NamedHotspot _rearView;
	SceneActor _scannerIcon;
	SceneActor _redLights;
	SceneActor _radarSweep;
	SpeedSlider _speedSlider;
	Button _forwardButton;
	Button _backwardButton;
	Button _exitButton;
	SequenceManager _sequenceManager;
	PaletteRotation *_rotation;

	int _direction;
	int _speedCurrent;
	int _speed;
	int _speedDelta;
	int _rotationSegment;
	int _rotationSegCurrent;
	int _newRotation;

	Scene1750();
	void synchronize(Serializer &s) override;

	void postInit(SceneObjectList *OwnerList = NULL) override;
	void remove() override;
	void signal() override;
	void process(Event &event) override;
	void dispatch() override;
};

class Scene1800 : public SceneExt {
	class Background : public NamedHotspot {
	public:
		bool startAction(CursorType action, Event &event) override;
	};

	class Lever : public SceneActor {
	public:
		bool startAction(CursorType action, Event &event) override;
	};
	class Doors : public SceneActor {
	public:
		bool startAction(CursorType action, Event &event) override;
	};
	class PassengerDoor : public SceneActor {
	public:
		bool startAction(CursorType action, Event &event) override;
	};

	class SouthExit : public SceneExit {
	public:
		void changeScene() override;
	};
public:
	int _locationMode;
	SpeakerQuinn _quinnSpeaker;
	SpeakerSeeker _seekerSpeaker;
	NamedHotspot _elevator;
	NamedHotspot _elevatorContents;
	NamedHotspot _surface;
	NamedHotspot _secBackground;
	Background _background;
	SceneActor _playerShadow;
	SceneActor _companion;
	SceneActor _companionShadow;
	SceneActor _leftStaircase;
	SceneActor _rightStaircase;
	Lever _lever;
	Doors _doors;
	PassengerDoor _leftDoor;
	PassengerDoor _rightDoor;
	SouthExit _southExit;
	SequenceManager _sequenceManager;

	Scene1800();
	void synchronize(Serializer &s) override;

	void postInit(SceneObjectList *OwnerList = NULL) override;
	void signal() override;
	void saveCharacter(int characterIndex) override;
};

class Scene1850 : public SceneExt {
	class Button : public NamedHotspot {
	public:
		bool startAction(CursorType action, Event &event) override;
	};

	class Robot : public SceneActor {
	public:
		bool startAction(CursorType action, Event &event) override;
	};
	class Door : public SceneActor {
	public:
		bool startAction(CursorType action, Event &event) override;
	};
	class DisplayScreen : public SceneActor {
	public:
		bool startAction(CursorType action, Event &event) override;
	};

public:
	int _sceneMode;
	int _shadeCountdown;
	int _shadeDirection;
	bool _shadeChanging;
	Common::Point _playerDest;
	int _seqNumber;
	ScenePalette _palette1;
	SpeakerQuinn _quinnSpeaker;
	SpeakerSeeker _seekerSpeaker;
	NamedHotspot _background;
	Button _button;
	SceneActor _companion;
	SceneActor _airbag;
	SceneActor _screen;
	SceneActor _helmet;
	Robot _robot;
	Door _leftDoor;
	Door _rightDoor;
	DisplayScreen _displayScreen;
	SequenceManager _sequenceManager1;
	SequenceManager _sequenceManager2;

	Scene1850();
	void synchronize(Serializer &s) override;

	void postInit(SceneObjectList *OwnerList = NULL) override;
	void remove() override;
	void signal() override;
	void process(Event &event) override;
	void dispatch() override;
};

class Scene1875 : public SceneExt {
	class Button : public SceneActor {
	public:
		int _buttonId;
		bool _buttonDown;

		Button();
		void doButtonPress();
		void initButton(int buttonId);

		Common::String getClassName() override { return "Scene1875_Button"; }
		void synchronize(Serializer &s) override;
		void process(Event &event) override;
	};
public:
	SpeakerQuinn _quinnSpeaker;
	SpeakerSeeker _seekerSpeaker;
	NamedHotspot _background;
	NamedHotspot _screen;
	SceneActor _map;
	SceneActor _rimPosition;
	Button _button1;
	Button _button2;
	Button _button3;
	Button _button4;
	Button _button5;
	SequenceManager _sequenceManager;

	void postInit(SceneObjectList *OwnerList = NULL) override;
	void signal() override;
	void process(Event &event) override;
};

class Scene1900 : public SceneExt {
	class LiftDoor : public SceneActor {
	public:
		bool startAction(CursorType action, Event &event) override;
	};

	class WestExit : public SceneExit {
	public:
		void changeScene() override;
	};
	class EastExit : public SceneExit {
	public:
		void changeScene() override;
	};
public:
	SpeakerSeeker1900 _seekerSpeaker;
	NamedHotspot _background;
	NamedHotspot _elevator;
	SceneActor _companion;
	BackgroundSceneObject _leftDoorFrame;
	BackgroundSceneObject _rightDoorFrame;
	LiftDoor _leftDoor, _rightDoor;
	WestExit _westExit;
	EastExit _eastExit;
	SequenceManager _sequenceManager1;
	SequenceManager _sequenceManager2;

	void postInit(SceneObjectList *OwnerList = NULL) override;
	void remove() override;
	void signal() override;
};

class Scene1925 : public SceneExt {
	class Button : public NamedHotspot {
	public:
		bool startAction(CursorType action, Event &event) override;
	};
	class Ladder : public NamedHotspot {
	public:
		bool startAction(CursorType action, Event &event) override;
	};

	class ExitUp : public SceneExit {
	public:
		void changeScene() override;
	};
	class ExitDown : public SceneExit {
	public:
		void changeScene() override;
	};
	class WestExit : public SceneExit {
	public:
		void changeScene() override;
	};
	class EastExit : public SceneExit {
	public:
		void changeScene() override;
	};
public:
	NamedHotspot _background;
	Button _button;
	Ladder _ladder;
	SceneActor _door;
	ExitUp _exitUp;
	ExitDown _exitDown;
	WestExit _westExit;
	EastExit _eastExit;
	SequenceManager _sequenceManager;

	int _newSceneMode;
	int _levelResNum[5];

	Scene1925();
	void synchronize(Serializer &s) override;

	void changeLevel(bool upFlag);
	void postInit(SceneObjectList *OwnerList = NULL) override;
	void remove() override;
	void signal() override;
};

class Scene1945 : public SceneExt {
	class Ice : public NamedHotspot {
	public:
		bool startAction(CursorType action, Event &event) override;
	};
	class Ladder : public NamedHotspot {
	public:
		bool startAction(CursorType action, Event &event) override;
	};

	class Gunpowder : public SceneActor {
	public:
		bool startAction(CursorType action, Event &event) override;
	};

	class ExitUp : public SceneExit {
	public:
		void changeScene() override;
	};
	class CorridorExit : public SceneExit {
	public:
		void changeScene() override;
	};
public:
	NamedHotspot _hole;
	NamedHotspot _ice2;
	Ice _ice;
	Ladder _ladder;
	SceneActor _coveringIce;
	SceneActor _alcoholLamp;
	Gunpowder _gunpowder;
	ExitUp _exitUp;
	CorridorExit _corridorExit;
	SequenceManager _sequenceManager1;
	SequenceManager _sequenceManager2;

	int _nextSceneMode1;
	int _nextSceneMode2;
	CursorType _lampUsed;

	Scene1945();
	void synchronize(Serializer &s) override;

	void postInit(SceneObjectList *OwnerList = NULL) override;
	void remove() override;
	void signal() override;
};

} // End of namespace Ringworld2
} // End of namespace TsAGE

#endif
