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

#ifndef TSAGE_RINGWORLD2_SCENES3_H
#define TSAGE_RINGWORLD2_SCENES3_H

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


class Scene3100 : public SceneExt {
	class Guard : public SceneActor {
		bool startAction(CursorType action, Event &event) override;
	};
public:
	bool _fadeSound;
	SpeakerGuard _guardSpeaker;
	NamedHotspot _background;
	NamedHotspot _hammerHead2;
	SceneActor _hammerHead;
	SceneActor _miranda;
	SceneActor _ghoul;
	SceneActor _technicians;
	SceneActor _deadBodies;
	Guard _guard;
	ASoundExt _sound1;
	SequenceManager _sequenceManager;

	Scene3100();
	void postInit(SceneObjectList *OwnerList = NULL) override;
	void remove() override;
	void signal() override;
	void dispatch() override;
	void synchronize(Serializer &s) override;
};

class Scene3125 : public SceneExt {
	class Background : public NamedHotspot {
	public:
		bool startAction(CursorType action, Event &event) override;
	};
	class Table : public NamedHotspot {
	public:
		bool startAction(CursorType action, Event &event) override;
	};
	class Computer : public NamedHotspot {
	public:
		bool startAction(CursorType action, Event &event) override;
	};

	class Door : public SceneActor {
		bool startAction(CursorType action, Event &event) override;
	};
public:
	bool _soundPlayed;
	Background _background;
	Door _door;
	Table _table;
	Computer _computer;
	SceneActor _ghoul1;
	SceneActor _ghoul2;
	SceneActor _ghoul3;
	SceneActor _ghoul4;
	SequenceManager _sequenceManager;

	Scene3125();
	void postInit(SceneObjectList *OwnerList = NULL) override;
	void signal() override;
	void dispatch() override;
	void synchronize(Serializer &s) override;
};

class Scene3150 : public SceneExt {
	class LightFixture : public NamedHotspot {
	public:
		bool startAction(CursorType action, Event &event) override;
	};
	class Toilet : public NamedHotspot {
	public:
		bool startAction(CursorType action, Event &event) override;
	};
	class Water : public SceneActor {
		bool startAction(CursorType action, Event &event) override;
	};
	class FoodTray : public SceneActor {
		bool startAction(CursorType action, Event &event) override;
	};
	class ToiletFlush : public SceneActor {
		bool startAction(CursorType action, Event &event) override;
	};
	class AirVent : public SceneActor {
		bool startAction(CursorType action, Event &event) override;
	};

	class DoorExit : public SceneExit {
	public:
		void changeScene() override;
	};
	class VentExit : public SceneExit {
	public:
		void changeScene() override;
	};
public:
	NamedHotspot _background;
	NamedHotspot _bed;
	NamedHotspot _lightFixture2;
	NamedHotspot _bars;
	LightFixture _lightFixture;
	Toilet _toilet;
	SceneActor _guard;
	SceneActor _doorBars;
	SceneActor _bulbOrWire;
	Water _water;
	FoodTray _foodTray;
	ToiletFlush _toiletFlush;
	AirVent _airVent;
	DoorExit _doorExit;
	VentExit _ventExit;
	SequenceManager _sequenceManager;

	void postInit(SceneObjectList *OwnerList = NULL) override;
	void signal() override;
	void dispatch() override;
};

class Scene3175 : public SceneExt {
	class RoomItem : public NamedHotspot {
	public:
		bool startAction(CursorType action, Event &event) override;
	};

	class Door : public SceneActor {
		bool startAction(CursorType action, Event &event) override;
	};
	class Corpse : public SceneActor {
		bool startAction(CursorType action, Event &event) override;
	};
public:
	RoomItem _background;
	RoomItem _table;
	RoomItem _autopsies;
	Door _door;
	SceneActor _computer;
	Corpse _corpse;
	SequenceManager _sequenceManager;

	void postInit(SceneObjectList *OwnerList = NULL) override;
	void signal() override;
};

class Scene3200 : public SceneExt {
public:
	SpeakerRocko3200 _rockoSpeaker;
	SpeakerJocko3200 _jockoSpeaker;
	SpeakerSocko3200 _sockoSpeaker;
	SceneActor _rocko;
	SceneActor _jocko;
	SceneActor _socko;
	SequenceManager _sequenceManager;

	void postInit(SceneObjectList *OwnerList = NULL) override;
	void signal() override;
};

class Scene3210 : public SceneExt {
public:
	SpeakerCaptain3210 _captainSpeaker;
	SpeakerPrivate3210 _privateSpeaker;
	SceneActor _captain;
	SceneActor _private;
	SequenceManager _sequenceManager;

	void postInit(SceneObjectList *OwnerList = NULL) override;
	void signal() override;
};

class Scene3220 : public SceneExt {
public:
	SpeakerRocko3220 _rockoSpeaker;
	SpeakerJocko3220 _jockoSpeaker;
	SceneActor _rocko;
	SceneActor _jocko;
	SequenceManager _sequenceManager;

	void postInit(SceneObjectList *OwnerList = NULL) override;
	void signal() override;
};

class Scene3230 : public SceneExt {
public:
	SpeakerRocko3230 _rockoSpeaker;
	SpeakerJocko3230 _jockoSpeaker;
	SceneActor _rocko;
	SceneActor _jocko;
	SceneActor _ghoul;
	SequenceManager _sequenceManager;

	void postInit(SceneObjectList *OwnerList = NULL) override;
	void signal() override;
};

class Scene3240 : public SceneExt {
public:
	SpeakerTeal3240 _tealSpeaker;
	SpeakerWebbster3240 _webbsterSpeaker;
	SpeakerMiranda _mirandaSpeaker;
	SceneActor _teal;
	SceneActor _webbster;
	SequenceManager _sequenceManager;

	void postInit(SceneObjectList *OwnerList = NULL) override;
	void signal() override;
};

class Scene3245 : public SceneExt {
public:
	SpeakerRalf3245 _ralfSpeaker;
	SpeakerTomko3245 _tomkoSpeaker;
	SceneActor _ralf;
	SceneActor _tomko;
	SequenceManager _sequenceManager;

	void postInit(SceneObjectList *OwnerList = NULL) override;
	void signal() override;
};

class Scene3250 : public SceneExt {
	class Item : public NamedHotspot {
	public:
		bool startAction(CursorType action, Event &event) override;
	};

	class Door : public SceneActor {
		bool startAction(CursorType action, Event &event) override;
	};
public:
	Item _background;
	Item _tnuctipunShip;
	Item _floodLights;
	Item _negator;
	Door _leftDoor;
	Door _topDoor;
	Door _rightDoor;
	Door _grate;
	SceneActor _ghoul1;
	SceneActor _ghoul2;
	SceneActor _ghoul3;
	SequenceManager _sequenceManager;

	void postInit(SceneObjectList *OwnerList = NULL) override;
	void signal() override;
	void dispatch() override;
};

class Scene3255 : public SceneExt {
public:
	SceneActor _teal;
	SceneActor _guard;
	SceneActor _door;
	SceneActor _quinn;
	SceneActor _ghoul1;
	SceneActor _ghoul2;
	SceneActor _ghoul3;
	SpeakerQuinn3255 _quinnSpeaker;
	SpeakerMiranda3255 _mirandaSpeaker;
	SequenceManager _sequenceManager;

	void postInit(SceneObjectList *OwnerList = NULL) override;
	void signal() override;
	void dispatch() override;
};

class Scene3260 : public SceneExt {
	class Door : public SceneActor {
		bool startAction(CursorType action, Event &event) override;
	};
	class Toolbox : public SceneActor {
		bool startAction(CursorType action, Event &event) override;
	};

	class Action1: public Action {
	public:
		void signal() override;
	};
public:
	NamedHotspot _background;
	SceneActor _sceeen1;
	SceneActor _screen2;
	SceneActor _screen3;
	SceneActor _screen4;
	SceneActor _screen5;
	SceneActor _screen6;
	SceneActor _screen7;
	SceneActor _screen8;
	SceneActor _screen9;
	SceneActor _securityConsole;
	SceneActor _computerConsole;
	SceneActor _lightingConsole;
	Door _door;
	Toolbox _toolbox;
	Action1 _action1;
	Action1 _action2;
	Action1 _action3;
	Action1 _action4;
	Action1 _action5;
	Action1 _action6;
	Action1 _action7;
	Action1 _action8;
	Action1 _action9;
	Action1 _action10;
	Action1 _action11;
	Action1 _action12;
	SequenceManager _sequenceManager;

	void postInit(SceneObjectList *OwnerList = NULL) override;
	void remove() override;
	void signal() override;
};

class Scene3275 : public SceneExt {
	class Door : public SceneActor {
		bool startAction(CursorType action, Event &event) override;
	};

	class CellExit : public SceneExit {
	public:
		void changeScene() override;
	};
public:
	NamedHotspot _background;
	NamedHotspot _emptyCell1;
	NamedHotspot _emptyCell2;
	NamedHotspot _securityBeams1;
	NamedHotspot _securityBeams2;
	SceneActor _doorFrame;
	Door _door;
	CellExit _cellExit;
	SequenceManager _sequenceManager;

	void postInit(SceneObjectList *OwnerList = NULL) override;
	void signal() override;
};

class Scene3350 : public SceneExt {
public:
	SceneActor _miranda;
	SceneActor _seeker;
	SceneActor _webbster;
	SceneActor _seatedPeople;
	SceneActor _ship;
	SceneActor _landedShip;
	SceneActor _shipShadow;
	SceneActor _canopy;
	SceneActor _shipFront;
	SequenceManager _sequenceManager;
	PaletteRotation *_rotation;

	void postInit(SceneObjectList *OwnerList = NULL) override;
	void remove() override;
	void signal() override;
};

class Scene3375 : public SceneExt {
	class Companion1 : public SceneActor {
		bool startAction(CursorType action, Event &event) override;
	};
	class Companion2 : public SceneActor {
		bool startAction(CursorType action, Event &event) override;
	};
	class Webbster : public SceneActor {
		bool startAction(CursorType action, Event &event) override;
	};
	class Door : public SceneActor {
		bool startAction(CursorType action, Event &event) override;
	};

	class LeftExit : public SceneExit {
	public:
		void changeScene() override;
	};
	class DownExit : public SceneExit {
	public:
		void changeScene() override;
	};
	class RightExit : public SceneExit {
	public:
		void changeScene() override;
	};

	void signalCase3379();
	void enterArea(int sceneMode);

public:
	SpeakerQuinn3375 _quinnSpeaker;
	SpeakerSeeker3375 _seekerSpeaker;
	SpeakerMiranda3375 _mirandaSpeaker;
	SpeakerWebbster3375 _webbsterSpeaker;
	NamedHotspot _background;
	NamedHotspot _itemArray[13];
	Companion1 _companion1;
	Companion2 _companion2;
	Webbster _webbster;
	Door _door;
	LeftExit _leftExit;
	DownExit _downExit;
	RightExit _rightExit;
	SequenceManager _sequenceManager;
	int _newSceneMode;
	int _sceneAreas[4];

	Scene3375();
	void postInit(SceneObjectList *OwnerList = NULL) override;
	void remove() override;
	void signal() override;
	void dispatch() override;
	void synchronize(Serializer &s) override;
};

class Scene3385 : public SceneExt {
	class Companion1 : public SceneActor {
		bool startAction(CursorType action, Event &event) override;
	};
	class Companion2 : public SceneActor {
		bool startAction(CursorType action, Event &event) override;
	};
	class Webbster : public SceneActor {
		bool startAction(CursorType action, Event &event) override;
	};
	class Door : public SceneActor {
		bool startAction(CursorType action, Event &event) override;
	};

	class SouthExit : public SceneExit {
	public:
		void changeScene() override;
	};

	class Action1: public Action {
	public:
		void signal() override;
	};

public:
	SpeakerQuinn3385 _quinnSpeaker;
	SpeakerSeeker3385 _seekerSpeaker;
	SpeakerMiranda3385 _mirandaSpeaker;
	SpeakerWebbster3385 _webbsterSpeaker;
	NamedHotspot _background;
	Companion1 _companion1;
	Companion2 _companion2;
	Webbster _webbster;
	Door _door;
	SouthExit _southExit;
	Action1 _action1;
	SequenceManager _sequenceManager;

	int _playerStrip;

	Scene3385();
	void postInit(SceneObjectList *OwnerList = NULL) override;
	void remove() override;
	void signal() override;
	void synchronize(Serializer &s) override;
};

class Scene3395 : public SceneExt {
	class Companion1 : public SceneActor {
		bool startAction(CursorType action, Event &event) override;
	};
	class Companion2 : public SceneActor {
		bool startAction(CursorType action, Event &event) override;
	};
	class Webbster : public SceneActor {
		bool startAction(CursorType action, Event &event) override;
	};
	class Door : public SceneActor {
		bool startAction(CursorType action, Event &event) override;
	};

	class Action1: public Action {
	public:
		void signal() override;
	};

public:
	SpeakerQuinn3395 _quinnSpeaker;
	SpeakerSeeker3395 _seekerSpeaker;
	SpeakerMiranda3395 _mirandaSpeaker;
	SpeakerWebbster3395 _webbsterSpeaker;
	NamedHotspot _background;
	NamedHotspot _itemArray[13];
	Companion1 _companion1;
	Companion2 _companion2;
	Webbster _webbster;
	Door _door;
	Action1 _action1;
	SequenceManager _sequenceManager;

	int _playerStrip;

	Scene3395();
	void postInit(SceneObjectList *OwnerList = NULL) override;
	void remove() override;
	void signal() override;
	void synchronize(Serializer &s) override;
};

class Scene3400 : public SceneExt {
public:
	SpeakerQuinn3400 _quinnSpeaker;
	SpeakerSeeker3400 _seekerSpeaker;
	SpeakerMiranda3400 _mirandaSpeaker;
	SpeakerWebbster3400 _webbsterSpeaker;
	SpeakerTeal3400 _tealSpeaker;
	SceneActor _companion1;
	SceneActor _companion2;
	SceneActor _webbster;
	SceneActor _teal;
	SceneActor _door;
	SceneActor _manholeCover;
	SceneActor _sapphire;
	SequenceManager _sequenceManager;
	bool _soundFaded;

	Scene3400();
	void postInit(SceneObjectList *OwnerList = NULL) override;
	void remove() override;
	void signal() override;
	void synchronize(Serializer &s) override;
};

class Scene3500 : public SceneExt {
	class Action1: public Action {
	public:
		int _direction;
		bool _headingRightFl;
		bool _turningFl;

		Action1();
		void synchronize(Serializer &s) override;
		void handleHorzButton(int direction);
		void turnShuttle(bool arg1);
		void signal() override;
		void dispatch() override;
	};
	class Action2: public Action {
	public:
		int _direction;

		Action2();
		void synchronize(Serializer &s) override;
		void handleVertButton(int direction);

		void signal() override;
	};

	class DirectionButton : public NamedHotspot {
	public:
		int _movementId;

		DirectionButton();
		void synchronize(Serializer &s) override;

		bool startAction(CursorType action, Event &event) override;
	};

	class Throttle : public SceneActor {
	public:
		Common::Point _pos;
		int _deltaX;
		int _deltaY;
		int _slideDeltaY;
		int _deltaMouseY;

		Throttle();
		void synchronize(Serializer &s) override;

		void init(int xp, int yp, int dx, int dy, int speed);
		void updateSpeed();
		void setSpeed(int arg1);
		void changePosition(const Common::Point &pt);

		void process(Event &event) override;
		bool startAction(CursorType action, Event &event) override;
	};

	class MazeUI3500 : public MazeUI {
	public:
		int16 cellFromX(int arg1);
		int16 cellFromY(int arg1);
		int getCellFromMapXY(Common::Point pt);
		bool setMazePosition2(Common::Point &p);
	};
public:
	Action1 _action1;
	Action2 _action2;
	NamedHotspot _background;
	NamedHotspot _outsideView;
	NamedHotspot _mapScreen;
	DirectionButton _pitchDown;
	DirectionButton _turnLeft;
	DirectionButton _pitchUp;
	DirectionButton _turnRight;
	// Glyph of vessel on top of the maze ui
	SceneActor _shuttle;
	SceneActor _verticalSpeedDisplay;
	SceneActor _horizontalSpeedDisplay;
	SceneActor _symbolVertical;
	SceneActor _symbolLeft;
	SceneActor _symbolRight;
	Throttle _throttle;
	SceneActor _tunnelVertCircle;
	SceneActor _tunnelHorzCircle;
	ASoundExt _aSound1;
	MazeUI3500 _mazeUI;
	SequenceManager _sequenceManager;

	int _moverVertX;
	int _moverHorzX;
	PaletteRotation *_rotation;
	int _mazeChangeAmount;
	int _speed;
	bool _updateIdxChangeFl;
	int _mazeDirection;
	int _nextMove;
	Common::Point _mazePosition;
	bool _postFixupFl;
	int _exitCounter;
	bool _directionChangesEnabled;

	Scene3500();
	void doMovement(int id);

	void postInit(SceneObjectList *OwnerList = NULL) override;
	void remove() override;
	void signal() override;
	void process(Event &event) override;
	void dispatch() override;
	void synchronize(Serializer &s) override;
};

class Scene3600 : public SceneExt {
	class Action3600: public ActionExt {
	public:
		bool _part2Fl;
		int _fadePct;

		Action3600();
		void synchronize(Serializer &s) override;
		void signal() override;
	};
	class Action2: public Action {
	public:
		void signal() override;
	};

	class LightShaft : public NamedHotspot {
	public:
		bool startAction(CursorType action, Event &event) override;
	};

	class Protector : public SceneActorExt {
		bool startAction(CursorType action, Event &event) override;
	};
public:
	Action3600 _action1;
	Action2 _action2;
	SpeakerQuinn3600 _quinnSpeaker;
	SpeakerSeeker3600 _seekerSpeaker;
	SpeakerMiranda3600 _mirandaSpeaker;
	SpeakerTeal3600 _tealSpeaker;
	SpeakerProtector3600 _protectorSpeaker;
	NamedHotspot _background;
	NamedHotspot _console;
	NamedHotspot _tapestry1;
	NamedHotspot _tapestry2;
	LightShaft _lightShaft;
	SceneActor _protector3400;
	SceneActor _steppingDisk;
	SceneActor _consoleLights;
	SceneActor _webbster;
	SceneActor _teal;
	SceneActor _door3400;
	SceneActor _quinn;
	SceneActor _seeker;
	SceneActor _miranda;
	Protector _protector;
	SequenceManager _sequenceManager1;
	SequenceManager _sequenceManager2;
	SequenceManager _sequenceManager3;
	SequenceManager _sequenceManager4;
	ScenePalette _palette1;

	bool _tealDead;
	bool _lightEntered;
	bool _ghoulTeleported;

	Scene3600();
	void postInit(SceneObjectList *OwnerList = NULL) override;
	void remove() override;
	void signal() override;
	void process(Event &event) override;
	void dispatch() override;
	void synchronize(Serializer &s) override;
};

class Scene3700 : public SceneExt {
public:
	SpeakerQuinn3700 _quinnSpeaker;
	SpeakerSeeker3700 _seekerSpeaker;
	SpeakerMiranda3700 _mirandaSpeaker;
	SceneActor _quinn;
	SceneActor _seeker;
	SceneActor _miranda;
	SceneActor _webbster;
	SceneActor _teleportPad;
	SequenceManager _sequenceManager;

	void postInit(SceneObjectList *OwnerList = NULL) override;
	void remove() override;
	void signal() override;
};

class Scene3800 : public SceneExt {
	class NorthExit : public SceneExit {
	public:
		void changeScene() override;
	};

	class EastExit : public SceneExit {
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
	SceneObject _balloon;
	SceneObject _harness;
	SceneActor _quinnShadow;
	NamedHotspot _background;
	NorthExit _northExit;
	EastExit _eastExit;
	SouthExit _southExit;
	WestExit _westExit;
	Rect _skylineRect;
	SequenceManager _sequenceManager1;

	int _desertDirection;

	Scene3800();
	void initExits();
	void enterArea();

	void postInit(SceneObjectList *OwnerList = NULL) override;
	void signal() override;
	void process(Event &event) override;
	void synchronize(Serializer &s) override;
};

class Scene3900 : public SceneExt {
	class NorthExit : public SceneExit {
	public:
		void changeScene() override;
	};

	class EastExit : public SceneExit {
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

	class Exit5 : public SceneExit {
	public:
		void changeScene() override;
	};
public:
	SceneActor _quinnShadow;
	NamedHotspot _background;
	NorthExit _northExit;
	EastExit _eastExit;
	SouthExit _southExit;
	WestExit _westExit;
	Exit5 _westEnterForest;
	Rect _skylineRect;

	void postInit(SceneObjectList *OwnerList = NULL) override;
	void signal() override;
	void process(Event &event) override;
};

} // End of namespace Ringworld2
} // End of namespace TsAGE

#endif
