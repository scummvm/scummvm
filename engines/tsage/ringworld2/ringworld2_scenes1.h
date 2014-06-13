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

	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void remove();
	virtual void signal();
	virtual void dispatch();
};

class Scene1010 : public SceneExt {
public:
	SequenceManager _sequenceManager;

	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void signal();
};

class Scene1020 : public SceneExt {
public:
	SequenceManager _sequenceManager;

	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void signal();
	virtual void dispatch();
};

class Scene1100 : public SceneExt {
	class Seeker : public SceneActor {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};
	class Trooper : public SceneActor {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};
	class Chief : public SceneActor {
	public:
		virtual bool startAction(CursorType action, Event &event);
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
	void synchronize(Serializer &s);

	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void remove();
	virtual void signal();
	virtual void dispatch();
	virtual void saveCharacter(int characterIndex);
};

class Scene1200 : public SceneExt {
	enum CrawlDirection { CRAWL_EAST = 1, CRAWL_WEST = 2, CRAWL_SOUTH = 3, CRAWL_NORTH = 4 };

	class LaserPanel: public ModalWindow {
	public:
		class Jumper : public SceneActorExt {
		public:
			void init(int state);
			virtual bool startAction(CursorType action, Event &event);
		};

		Jumper _jumper1;
		Jumper _jumper2;
		Jumper _jumper3;

		LaserPanel();

		virtual void postInit(SceneObjectList *OwnerList = NULL);
		virtual void remove();
	};

public:
	NamedHotspot _item1;
	SceneActor _actor1;
	LaserPanel _laserPanel;
	MazeUI _mazeUI;
	SequenceManager _sequenceManager;

	int _nextCrawlDirection;
	int _field414;
	int _field416;
	int _field418;
	int _field41A;
	bool _fixupMaze;

	Scene1200();
	void synchronize(Serializer &s);

	void startCrawling(CrawlDirection dir);

	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void signal();
	virtual void process(Event &event);
	virtual void dispatch();
	virtual void saveCharacter(int characterIndex);
};

class Scene1337 : public SceneExt {
	class OptionsDialog: public GfxDialog {
	private:
		GfxButton _autoplay;
		GfxButton _restartGame;
		GfxButton _quitGame;
		GfxButton _continueGame;

		OptionsDialog();
		virtual ~OptionsDialog() {}
		virtual GfxButton *execute(GfxButton *defaultButton);
	public:
		static void show();
	};

	class Card: public SceneHotspot {
	public:
		SceneObject _card;

		int _cardId;
		Common::Point _stationPos;

		Card();
		void synchronize(Serializer &s);
		bool isIn(Common::Point pt);
	};

	class GameBoardSide: public SceneHotspot {
	public:
		Card _handCard[4];
		Card _outpostStation[8];
		Card _delayCard;
		Card _emptyStationPos;

		Common::Point _card1Pos;
		Common::Point _card2Pos;
		Common::Point _card3Pos;
		Common::Point _card4Pos;
		int _frameNum;

		GameBoardSide();
		void synchronize(Serializer &s);
	};

	class Action1337: public Action {
	public:
		void waitFrames(int32 frameCount);
	};

	class Action1: public Action1337 {
	public:
		void signal();
	};
	class Action2: public Action1337 {
	public:
		void signal();
	};
	class Action3: public Action1337 {
	public:
		void signal();
	};
	class Action4: public Action1337 {
	public:
		void signal();
	};
	class Action5: public Action1337 {
	public:
		void signal();
	};
	class Action6: public Action1337 {
	public:
		void signal();
	};
	class Action7: public Action1337 {
	public:
		void signal();
	};
	class Action8: public Action1337 {
	public:
		void signal();
	};
	class Action9: public Action1337 {
	public:
		void signal();
	};
	class Action10: public Action1337 {
	public:
		void signal();
	};
	class Action11: public Action1337 {
	public:
		void signal();
	};
	class Action12: public Action1337 {
	public:
		void signal();
	};
	class Action13: public Action1337 {
	public:
		void signal();
	};
public:
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

	typedef void (Scene1337::*FunctionPtrType)();
	FunctionPtrType _delayedFunction;

	bool _autoplay;
	bool _shuffleEndedFl;
	bool _showPlayerTurn;
	bool _displayHelpFl;
	bool _instructionsDisplayedFl;

	// Discarded cards are put in the available cards pile, with an higher index so there no conflict
	int _currentDiscardIndex;
	int _availableCardsPile[100];
	int _cardsAvailableNumb;
	int _currentPlayerNumb;
	int _actionIdx1;
	int _actionIdx2;
	int _winnerId;
	int _instructionsWaitCount;
	int _cursorCurRes;
	int _cursorCurStrip;
	int _cursorCurFrame;

	ASound _aSound1;
	ASound _aSound2;
	GameBoardSide _gameBoardSide[4];
	SceneActor _helpIcon;
	SceneActor _stockPile;
	SceneItem _actionItem;
	SceneObject _currentPlayerArrow;

	Card *_actionCard1;
	Card *_actionCard2;
	Card *_actionCard3;
	Card _animatedCard;
	Card _shuffleAnimation;
	Card _discardedPlatformCard;
	Card _selectedCard;
	Card _discardPile;
	Card _stockCard;

	SceneObject _upperDisplayCard[8];
	SceneObject _lowerDisplayCard[8];

	Scene1337();
	virtual void synchronize(Serializer &s);

	void actionDisplay(int resNum, int lineNum, int x, int y, int keepOnScreen, int width, int textMode, int fontNum, int colFG, int colBGExt, int colFGExt);
	void setAnimationInfo(Card *card);
	void handleNextTurn();
	void handlePlayerTurn();
	bool isStationCard(int cardId);
	bool isStopConstructionCard(int cardId);
	int  getStationId(int playerId, int handCardId);
	int  findPlatformCardInHand(int playerId);
	int  findCard13InHand(int playerId);
	int  checkThieftCard(int playerId);
	int  isDelayCard(int cardId);
	int  getStationCardId(int cardId);
	void handlePlayer01Discard(int playerId);
	void playThieftCard(int playerId, Card *card, int victimId);
	int  getPreventionCardId(int cardId);
	bool isAttackPossible(int victimId, int cardId);
	int  getPlayerWithOutpost(int playerId);
	bool checkAntiDelayCard(int delayCardId, int cardId);
	void playStationCard(Card *station, Card *platform);
	void playDelayCard(Card *card, Card *dest);
	void playPlatformCard(Card *card, Card *dest);
	void playAntiDelayCard(Card *card, Card *dest);
	Card *getStationCard(int arg1);
	void playCounterTrickCard(Card *card, int playerId);
	int  getFreeHandCard(int playerId);
	void discardCard(Card *card);
	void subC4CD2();
	void subC4CEC();
	void subC51A0(Card *subObj1, Card *subObj2);
	void displayDialog(int dialogNumb);
	void subPostInit();
	void displayInstructions();
	void suggestInstructions();
	void shuffleCards();
	void dealCards();
	void showOptionsDialog();
	void handleClick(int arg1, Common::Point pt);
	void handlePlayer0();
	void handlePlayer1();
	void handlePlayer2();
	void handlePlayer3();
	void handleAutoplayPlayer2();
	void updateCursorId(int arg1, bool arg2);
	void setCursorData(int resNum, int rlbNum, int frameNum);
	void subD18F5();
	void subD1917();
	void subD1940(bool flag);
	void subD1975(int arg1, int arg2);

	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void remove();
	virtual void process(Event &event);
	virtual void dispatch();
};

class Scene1500 : public SceneExt {
public:
	SceneActor _starship;
	SceneActor _starshipShadow;
	SceneActor _smallShip;
	SceneActor _smallShipShadow;
	SequenceManager _sequenceManager;

	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void remove();
	virtual void signal();
	virtual void dispatch();
};

class Scene1525 : public SceneExt {
public:
	SequenceManager _sequenceManager;

	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void signal();
};

class Scene1530 : public SceneExt {
public:
	SpeakerQuinn _quinnSpeaker;
	SpeakerSeeker _seekerSpeaker;
	SceneActor _seeker;
	SceneActor _leftReactor;
	SceneActor _rightReactor;

	SequenceManager _sequenceManager;

	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void signal();
	virtual void dispatch();
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
		void synchronize(Serializer &s);

		virtual bool startAction(CursorType action, Event &event);
	};

	class ShipComponent : public SceneActor {
	public:
		int _componentId;

		ShipComponent();
		void synchronize(Serializer &s);

		virtual bool startAction(CursorType action, Event &event);
		void setupShipComponent(int componentId);
	};

	class DishControlsWindow : public ModalWindow {
		class DishControl : public SceneActor {
		public:
			int _controlId;

			DishControl();
			void synchronize(Serializer &s);

			virtual bool startAction(CursorType action, Event &event);
		};
	public:
		byte _field20;
		SceneActor _areaActor;
		DishControl _button;
		DishControl _lever;

		virtual void remove();
		virtual void setup2(int visage, int stripFrameNum, int frameNum, int posX, int posY);
	};

	class WorkingShip : public NamedHotspot {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};

	class Wreckage : public SceneActor {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};

	class Companion : public SceneActor {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};

	class AirBag : public SceneActor {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};

	class Joystick : public SceneActor {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};

	class Gyroscope : public SceneActor {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};

	class DiagnosticsDisplay : public SceneActor {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};

	class DishTower : public SceneActor {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};

	class Dish : public SceneActor {
	public:
		virtual bool startAction(CursorType action, Event &event);
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
	void synchronize(Serializer &s);
	void enterArea();

	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void signal();
	virtual void process(Event &event);
	virtual void dispatch();
	virtual void saveCharacter(int characterIndex);
};

class Scene1575 : public SceneExt {
	class Button : public NamedHotspot {
	public:
		int _buttonId;
		bool _pressed;

		Button();
		void synchronize(Serializer &s);
		void initButton(int buttonId);

		virtual void process(Event &event);
		virtual bool startAction(CursorType action, Event &event);
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
	void synchronize(Serializer &s);

	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void remove();
	virtual void signal();
	virtual void process(Event &event);
	virtual void dispatch();
};

class Scene1580 : public SceneExt {
	class JoystickPlug : public NamedHotspot {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};
	class ScreenSlot : public NamedHotspot {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};

	class Joystick : public SceneActor {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};
	class Screen : public SceneActor {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};
	class StorageCompartment : public SceneActor {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};
	class HatchButton : public SceneActor {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};
	class ThrusterValve : public SceneActor {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};
	class Ignitor : public SceneActor {
	public:
		virtual bool startAction(CursorType action, Event &event);
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
	void synchronize(Serializer &s);

	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void signal();
};

class Scene1625 : public SceneExt {
	class Wire : public SceneActor {
	public:
		virtual bool startAction(CursorType action, Event &event);
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
	void synchronize(Serializer &s);

	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void remove();
	virtual void signal();
	virtual void process(Event &event);
};

class Scene1700 : public SceneExt {
	class RimTransport : public SceneActor {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};
	class Companion : public SceneActor {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};

	class NorthExit : public SceneExit {
	public:
		virtual void changeScene();
	};
	class SouthExit : public SceneExit {
	public:
		virtual void changeScene();
	};
	class WestExit : public SceneExit {
	public:
		virtual void changeScene();
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
	void synchronize(Serializer &s);
	void enterArea();

	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void remove();
	virtual void signal();
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
		virtual void synchronize(Serializer &s);
		void setupSlider(int incrAmount, int xp, int ys, int height, int thumbHeight);
		void calculateSlider();

		virtual void process(Event &event);
		virtual bool startAction(CursorType action, Event &event);
	};

	class Button : public SceneActor {
	public:
		int _buttonId;

		Button();
		virtual void synchronize(Serializer &s);

		virtual bool startAction(CursorType action, Event &event);
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
	virtual void synchronize(Serializer &s);

	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void remove();
	virtual void signal();
	virtual void process(Event &event);
	virtual void dispatch();
};

class Scene1800 : public SceneExt {
	class Background : public NamedHotspot {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};

	class Lever : public SceneActor {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};
	class Doors : public SceneActor {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};
	class PassengerDoor : public SceneActor {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};

	class SouthExit : public SceneExit {
	public:
		virtual void changeScene();
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
	void synchronize(Serializer &s);

	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void signal();
	virtual void saveCharacter(int characterIndex);
};

class Scene1850 : public SceneExt {
	class Button : public NamedHotspot {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};

	class Robot : public SceneActor {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};
	class Door : public SceneActor {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};
	class DisplayScreen : public SceneActor {
	public:
		virtual bool startAction(CursorType action, Event &event);
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
	void synchronize(Serializer &s);

	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void remove();
	virtual void signal();
	virtual void process(Event &event);
	virtual void dispatch();
};

class Scene1875 : public SceneExt {
	class Button : public SceneActor {
	public:
		int _buttonId;
		bool _buttonDown;

		Button();
		void doButtonPress();
		void initButton(int buttonId);

		virtual Common::String getClassName() { return "Scene1875_Button"; }
		void synchronize(Serializer &s);
		virtual void process(Event &event);
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

	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void signal();
	virtual void process(Event &event);
};

class Scene1900 : public SceneExt {
	class LiftDoor : public SceneActor {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};

	class WestExit : public SceneExit {
	public:
		virtual void changeScene();
	};
	class EastExit : public SceneExit {
	public:
		virtual void changeScene();
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

	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void remove();
	virtual void signal();
};

class Scene1925 : public SceneExt {
	class Button : public NamedHotspot {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};
	class Ladder : public NamedHotspot {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};

	class ExitUp : public SceneExit {
	public:
		virtual void changeScene();
	};
	class ExitDown : public SceneExit {
	public:
		virtual void changeScene();
	};
	class WestExit : public SceneExit {
	public:
		virtual void changeScene();
	};
	class EastExit : public SceneExit {
	public:
		virtual void changeScene();
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
	void synchronize(Serializer &s);

	void changeLevel(bool upFlag);
	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void remove();
	virtual void signal();
};

class Scene1945 : public SceneExt {
	class Ice : public NamedHotspot {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};
	class Ladder : public NamedHotspot {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};

	class Gunpowder : public SceneActor {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};

	class ExitUp : public SceneExit {
	public:
		virtual void changeScene();
	};
	class CorridorExit : public SceneExit {
	public:
		virtual void changeScene();
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
	void synchronize(Serializer &s);

	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void remove();
	virtual void signal();
};

class Scene1950 : public SceneExt {
	/* Windows */
	class KeypadWindow: public ModalWindow {
	public:
		class KeypadButton : public SceneActor {
		public:
			int _buttonIndex;
			bool _pressed;
			bool _toggled;

			KeypadButton();
			void synchronize(Serializer &s);

			void init(int indx);
			virtual void process(Event &event);
			virtual bool startAction(CursorType action, Event &event);
		};

		SceneActor _areaActor;
		KeypadButton _buttons[16];

		int _buttonIndex;

		KeypadWindow();
		virtual void synchronize(Serializer &s);
		virtual void remove();
		virtual void setup2(int visage, int stripFrameNum, int frameNum, int posX, int posY);
		virtual void setup3(int resNum, int lookLineNum, int talkLineNum, int useLineNum);
	};

	class Keypad : public NamedHotspot {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};

	/* Actors */
	class Door : public SceneActor {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};
	class Scrolls : public SceneActor {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};
	class Gem : public SceneActor {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};
	class Vampire : public SceneActor {
	public:
		Common::Point _deadPosition;
		int _deltaX;
		int _deltaY;
		int _vampireMode;

		Vampire();
		void synchronize(Serializer &s);

		virtual void signal();
		virtual bool startAction(CursorType action, Event &event);
	};

	/* Exits */
	class NorthExit : public SceneExit {
	public:
		virtual void changeScene();
	};
	class UpExit : public SceneExit {
	public:
		virtual void changeScene();
	};
	class EastExit : public SceneExit {
	public:
		virtual void changeScene();
	};
	class DownExit : public SceneExit {
	public:
		virtual void changeScene();
	};
	class SouthExit : public SceneExit {
	public:
		virtual void changeScene();
	};
	class WestExit : public SceneExit {
	public:
		virtual void changeScene();
	};
	class ShaftExit : public SceneExit {
	public:
		virtual void changeScene();
	};
	class DoorExit : public SceneExit {
	public:
		virtual void changeScene();
	};
private:
	void initArea();
	void enterArea();
	void doButtonPress(int indx);
public:
	NamedHotspot _background;
	Keypad _keypad;
	SceneActor _southDoorway;
	SceneObject _northDoorway;
	Door _door;
	Scrolls _scrolls;
	SceneActor _containmentField;
	Gem _gem;
	SceneActor _cube;
	SceneActor _pulsingLights;
	Vampire _vampire;
	KeypadWindow _KeypadWindow;
	NorthExit _northExit;
	UpExit _upExit;
	EastExit _eastExit;
	DownExit _downExit;
	SouthExit _southExit;
	WestExit _westExit;
	ShaftExit _shaftExit;
	DoorExit _doorExit;
	SequenceManager _sequenceManager;

	bool _upExitStyle;
	bool _removeFlag;
	bool _vampireActive;
	Common::Point _vampireDestPos;
	int _vampireIndex;

	Scene1950();
	void synchronize(Serializer &s);

	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void remove();
	virtual void signal();
	virtual void process(Event &event);
};

} // End of namespace Ringworld2
} // End of namespace TsAGE

#endif
