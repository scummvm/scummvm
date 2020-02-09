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

#ifndef TSAGE_RINGWORLD2_SCENES0_H
#define TSAGE_RINGWORLD2_SCENES0_H

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

class Scene50: public SceneExt {

	class Action1: public Action {
	public:
		void signal() override;
	};

public:
	Action1 _action1;

	void postInit(SceneObjectList *OwnerList = NULL) override;
	void process(Event &event) override;
};

class Scene100: public SceneExt {
	/* Objects */
	class Door: public SceneActorExt {
	public:
		bool startAction(CursorType action, Event &event) override;
	};
	class Table: public SceneActor {
	public:
		bool startAction(CursorType action, Event &event) override;
	};
	class StasisNegator: public SceneActor {
	public:
		bool startAction(CursorType action, Event &event) override;
	};
	class DoorDisplay: public SceneActorExt {
	public:
		bool startAction(CursorType action, Event &event) override;
	};
	class SteppingDisks: public SceneActor {
	public:
		bool startAction(CursorType action, Event &event) override;
	};

	/* Items */
	class Terminal: public NamedHotspot{
	public:
		bool startAction(CursorType action, Event &event) override;
	};

public:
	NamedHotspot _background, _duct, _bed, _desk;
	Terminal _terminal;
	SceneActor _bedLights1, _bedLights2, _tableLocker, _wardrobeTopAnim, _wardrobeColorAnim;
	SceneActor _wardrobe;
	Door _door;
	Table _table;
	StasisNegator _stasisNegator;
	DoorDisplay _doorDisplay;
	SteppingDisks _steppingDisks;
	SequenceManager _sequenceManager1, _sequenceManager2;

	void postInit(SceneObjectList *OwnerList = NULL) override;
	void remove() override;
	void signal() override;
	void dispatch() override;
};

class Scene125: public SceneExt {
	/* Objects */
	class Food: public SceneActor {
	public:
		bool startAction(CursorType action, Event &event) override;
	};

	class Icon: public SceneActor {
	public:
		int _lookLineNum, _iconId;
		bool _pressed;
		SceneObject _glyph, _horizLine;
		SceneText _sceneText1, _sceneText2;

		Icon();
		Common::String getClassName() override { return "Scene125_Icon"; }
		void postInit(SceneObjectList *OwnerList = NULL) override;
		void synchronize(Serializer &s) override;
		void process(Event &event) override;

		void setIcon(int id);
		void showIcon();
		void hideIcon();
	};

	/* Items */
	class DiskSlot: public NamedHotspot {
	public:
		bool startAction(CursorType action, Event &event) override;
	};

public:
	ScenePalette _palette;
	ASoundExt _sound1;
	NamedHotspot _background, _item2, _item3;
	DiskSlot _diskSlot;
	SceneActor _starchart1, _starchart2, _starchart3, _starchart4;
	SceneActor _food, _foodDispenser, _infoDisk;
	Icon _icon1, _icon2, _icon3, _icon4, _icon5,  _icon6;
	SequenceManager _sequenceManager;
	SceneText _sceneText;
	int _consoleMode, _iconFontNumber, _logIndex, _databaseIndex, _infodiskIndex;
	int _soundCount, _soundIndex;
	int _soundIndexes[10];

	Scene125();
	void postInit(SceneObjectList *OwnerList = NULL) override;
	void synchronize(Serializer &s) override;
	void signal() override;
	void process(Event &event) override;
	void dispatch() override;

	void consoleAction(int id);
	void setDetails(int resNum, int lineNum);
	void stop();
	Common::String parseMessage(const Common::String &msg);
};

class Scene150: public Scene100 {
public:
	SceneActor _emptyRoomTable;
public:
	void postInit(SceneObjectList *OwnerList = NULL) override;
	void remove() override;
	void signal() override;
};

class Scene160: public SceneExt {
	class Action1: public Action {
	public:
		void signal() override;
	};
public:
	ASound _sound1;
	Action1 _action1;
	int _frameNumber, _yChange;
	int _lineNum;
	SynchronizedList<SceneText *> _creditsList;
public:
	Scene160();
	void postInit(SceneObjectList *OwnerList = NULL) override;
	void synchronize(Serializer &s) override;
	void remove() override;
	void process(Event &event) override;
};

class Scene175: public Scene150 {
};

class Scene180: public SceneExt {
	class Action1: public Action {
	public:
		void signal() override;
	};
private:
	void setSceneDelay(int v);
public:
	SpeakerWebbster180 _webbsterSpeaker;
	SpeakerDutyOfficer180 _dutyOfficerSpeaker;
	SpeakerTeal180 _tealSpeaker;
	SpeakerGameText _gameTextSpeaker;
	SceneActor _dutyOfficer, _teal, _webbster, _door, _shipDisplay;
	ScenePalette _palette;
	SceneText _textList[20];
	AnimationPlayerExt _animationPlayer;
	SequenceManager _sequenceManager;
	Action1 _action1;
	ASoundExt _sound1;

	int _frameNumber;
	bool _helpEnabled;
	int _frameInc;
	int _fontNumber, _fontHeight;
public:
	Scene180();

	void postInit(SceneObjectList *OwnerList = NULL) override;
	void synchronize(Serializer &s) override;
	void remove() override;
	void signal() override;
	void process(Event &event) override;
	void dispatch() override;
	void restore() override;
};

class Scene200: public SceneExt {
	/* Objects */
	class NorthDoor: public SceneActor {
	public:
		bool startAction(CursorType action, Event &event) override;
	};
	class EastDoor: public SceneActor {
	public:
		bool startAction(CursorType action, Event &event) override;
	};
	class WestDoor: public SceneActor {
	public:
		bool startAction(CursorType action, Event &event) override;
	};

	/* Scene Exits */
	class EastExit: public SceneExit {
	public:
		void changeScene() override;
	};
	class WestExit: public SceneExit {
	public:
		void changeScene() override;
	};
public:
	NamedHotspot _background, _compartment, _westDoorDisplay, _eastDoorDisplay;
	NorthDoor _northDoor;
	EastDoor _eastDoor;
	WestDoor _westDoor;
	EastExit _eastExit;
	WestExit _westExit;
	SequenceManager _sequenceManager;

	void postInit(SceneObjectList *OwnerList = NULL) override;
	void signal() override;
};

class Star: public SceneObject {
public:
	int _x100, _y100;
public:
	Common::String getClassName() override { return "Scene205_Star"; }
};

class Scene205: public SceneExt {
	/* Actions */
	class Action1: public Action {
	private:
		void textLoop();
	public:
		void signal() override;
	};
private:
	void setup();
	void processList(Star **ObjList, int count, const Common::Rect &bounds,
					int xMultiply, int yMultiply, int xCenter, int yCenter);
	void handleText();
public:
	AnimationPlayer _animationPlayer;
	int _fontHeight;
	SceneText _textList[15];
	Star *_starList1[3];
	Star *_starList2[3];
	Star *_starList3[4];
	ASound _sound1;
	Action1 _action1;
	int _yp;
	int _textIndex, _lineNum;
	Common::String _message;
public:
	Scene205();

	void postInit(SceneObjectList *OwnerList = NULL) override;
	void synchronize(Serializer &s) override;
	void remove() override;
	void process(Event &event) override;
	void dispatch() override;
};

class Scene205Demo: public SceneExt {
	/* Actions */
	class Action1: public Action {
	public:
		void signal() override;
	};
private:
	void leaveScene();
public:
	VisualSpeaker _animationPlayer;
	ASound _sound1;
	Action1 _action1;
public:
	void postInit(SceneObjectList *OwnerList = NULL) override;
	void remove() override;
	void process(Event &event) override;
};

class Scene250: public SceneExt {
	class Button: public SceneActor {
	public:
		int _floorNumber;
		Button();
		void setFloor(int floorNumber);

		void synchronize(Serializer &s) override;
		bool startAction(CursorType action, Event &event) override;
	};
public:
	int _currButtonY, _destButtonY, _elevatorSpeed;
	bool _skippingFl, _skippableFl;
	NamedHotspot _background, _door, _directionIndicator;
	Button _destinationFloor, _currentFloor;
	Button _floor1, _floor2, _floor3, _floor4, _floor5;
	Button _floor6, _floor7, _floor8, _floor9;
	ASoundExt _sound1;
	SequenceManager _sequenceManager1;
public:
	Scene250();
	void changeFloor(int floorNumber);

	void synchronize(Serializer &s) override;
	void postInit(SceneObjectList *OwnerList = NULL) override;
	void signal() override;
	void process(Event &event) override;
	void dispatch() override;
};

class Scene300: public SceneExt {
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

	/* Items */
	class QuinnWorkstation: public NamedHotspot {
	public:
		bool startAction(CursorType action, Event &event) override;
	};
	class MirandaWorkstation: public NamedHotspot {
	public:
		bool startAction(CursorType action, Event &event) override;
	};
	class SeekerWorkstation: public NamedHotspot {
	public:
		bool startAction(CursorType action, Event &event) override;
	};

	/* Objects */
	class Miranda: public SceneActor {
	public:
		bool startAction(CursorType action, Event &event) override;
	};
	class Seeker: public SceneActor {
	public:
		bool startAction(CursorType action, Event &event) override;
	};
	class Quinn: public SceneActor {
	public:
		bool startAction(CursorType action, Event &event) override;
	};
	class Doorway: public SceneActor {
	public:
		bool startAction(CursorType action, Event &event) override;
	};
public:
	SequenceManager _sequenceManager1, _sequenceManager2, _sequenceManager3, _sequenceManager4;
	ASoundExt _sound1;
	SpeakerMiranda300 _mirandaSpeaker;
	SpeakerSeeker300 _seekerSpeaker;
	SpeakerSeekerL _seekerLSpeaker;
	SpeakerQuinn300 _quinnSpeaker;
	SpeakerQuinnL _quinnLSpeaker;
	SpeakerTeal300 _tealSpeaker;
	SpeakerSoldier300 _soldierSpeaker;

	NamedHotspot _background, _hull, _statusDisplays, _damageControl, _manualOverrides;
	NamedHotspot _scanners1, _scanners2, _indirectLighting1, _indirectLighting2, _lighting;
	QuinnWorkstation _quinnWorkstation1, _quinnWorkstation2;
	SeekerWorkstation _seekerWorkstation;
	MirandaWorkstation _mirandaWorkstation1, _mirandaWorkstation2;
	SceneActor _atmosphereLeftWindow, _atmosphereRightWindow, _leftVerticalBarsAnim, _rightVerticalBarsAnim, _protocolDisplay;
	SceneActor _rightTextDisplay, _mirandaScreen, _leftTextDisplay, _quinnScreen;
	SceneActor _teal, _soldier, _object12;
	Doorway _doorway;
	Miranda _miranda;
	Seeker _seeker;
	Quinn _quinn;
	Action1 _action1;
	Action2 _action2;
	Action3 _action3;
	Action4 _action4;
	PaletteRotation *_rotation;
	int _stripId;

	Scene300();
	void signal309();

	void synchronize(Serializer &s) override;
	void postInit(SceneObjectList *OwnerList = NULL) override;
	void remove() override;
	void signal() override;
};

class Scene325: public SceneExt {
	class Icon: public SceneActor {
	public:
		int _lookLineNum, _iconId;
		bool _pressed;
		SceneObject _glyph, _horizLine;
		SceneText _sceneText1, _sceneText2;

		Icon();
		Common::String getClassName() override { return "Scene325_Icon"; }
		void postInit(SceneObjectList *OwnerList = NULL) override;
		void synchronize(Serializer &s) override;
		void process(Event &event) override;

		void setIcon(int id);
		void showIcon();
		void hideIcon();
	};

private:
	void removeText();
	void consoleAction(int id);
	void setMessage(int resNum, int lineNum);
	Common::String parseMessage(const Common::String &msg);
public:
	int _consoleAction, _iconFontNumber, _databasePage, _priorConsoleAction;
	int _moveCounter, _yChange, _yDirection, _scannerLocation;
	int _soundCount, _soundIndex;
	int _soundQueue[10];
	SpeakerQuinn _quinnSpeaker;
	ScenePalette _palette;
	SceneHotspot _background, _terminal;
	SceneObject _starGrid1, _starGrid2, _starGrid3; // Both starchart & scan grid objects
	SceneObject _starGrid4, _starGrid5, _starGrid6, _starGrid7;
	SceneObject _starGrid8, _starGrid9, _starGrid10, _starGrid11;
	SceneObject _starGrid12, _starGrid13;
	SceneObject _objList[4];
	Icon _icon1, _icon2, _icon3, _icon4, _icon5, _icon6;
	ASoundExt _sound1;
	SequenceManager _sequenceManager1;
	SceneText _text1;
public:
	Scene325();

	void postInit(SceneObjectList *OwnerList = NULL) override;
	void synchronize(Serializer &s) override;
	void remove() override;
	void signal() override;
	void process(Event &event) override;
	void dispatch() override;
};

class Scene400: public SceneExt {
	/* Items */
	class Terminal: public NamedHotspot {
	public:
		bool startAction(CursorType action, Event &event) override;
	};

	/* Objects */
	class Door: public SceneActor {
	public:
		bool startAction(CursorType action, Event &event) override;
	};
	class Reader: public SceneActor {
	public:
		bool startAction(CursorType action, Event &event) override;
	};
	class SensorProbe: public SceneActor {
	public:
		bool startAction(CursorType action, Event &event) override;
	};
	class AttractorUnit: public SceneActor {
	public:
		bool startAction(CursorType action, Event &event) override;
	};
public:
	NamedHotspot _background, _equipment1, _equipment2, _equipment3;
	NamedHotspot _equipment4, _equipment5, _equipment6;
	NamedHotspot _desk, _desk2, _console;
	NamedHotspot _duct, _shelves, _cabinet, _doorDisplay, _lights;
	NamedHotspot _equalizer, _transducer, _optimizer, _soundModule, _tester;
	NamedHotspot _helmet, _nullifier;
	Terminal _terminal;
	SceneActor _consoleDisplay, _testerDisplay;
	Door _door;
	Reader _reader;
	SensorProbe _sensorProbe;
	AttractorUnit _attractorUnit;
	SequenceManager _sequenceManager1;
	ASoundExt _sound1;

	void postInit(SceneObjectList *OwnerList = NULL) override;
	void remove() override;
	void signal() override;
	void dispatch() override;
};

class Scene500: public SceneExt {
	/* Dialogs */
	class PanelDialog: public SceneAreaObject {
		class Button: public SceneActor {
		private:
			int _buttonId;
			bool _buttonDown;

			void doButtonPress();
		public:
			Button();
			Common::String getClassName() override { return "Scene500_Button"; }
			void process(Event &event) override;
			bool startAction(CursorType action, Event &event) override;
			void synchronize(Serializer &s) override;

			void setupButton(int buttonId);
		};
	public:
		Button _button1, _button2, _button3;

		Common::String getClassName() override { return "Scene500_PanelWindow"; }
		void remove() override;
		void setDetails(int visage, int strip, int frameNumber, const Common::Point &pt);
	};

	/* Items */
	class ControlPanel: public SceneHotspot {
	public:
		bool startAction(CursorType action, Event &event) override;
	};

	/* Objects */
	class Seeker: public SceneActor {
	public:
		bool startAction(CursorType action, Event &event) override;
	};
	class Suit: public SceneActor {
	public:
		bool startAction(CursorType action, Event &event) override;
	};
	class Doorway: public SceneActor {
	public:
		bool startAction(CursorType action, Event &event) override;
	};
	class OxygenTanks: public SceneActor {
	public:
		bool startAction(CursorType action, Event &event) override;
	};
	class AirLock: public SceneActor {
	public:
		bool startAction(CursorType action, Event &event) override;
	};
	class TransparentDoor: public SceneActor {
	public:
		void draw() override;
	};
	class Aerosol: public SceneActor {
	public:
		bool startAction(CursorType action, Event &event) override;
	};
	class SonicStunner: public SceneActor {
	public:
		bool startAction(CursorType action, Event &event) override;
	};
	class Locker1: public SceneActor {
	public:
		bool startAction(CursorType action, Event &event) override;
	};
	class Locker2: public SceneActor {
	public:
		bool startAction(CursorType action, Event &event) override;
	};
public:
	int _stripNumber;
	SpeakerSeeker500 _seekerSpeaker;
	SpeakerQuinn500 _quinnSpeaker;
	SceneHotspot _background, _airlockCorridor;
	ControlPanel _controlPanel;
	SceneActor _suits;
	Seeker _seeker;
	Suit _suit;
	Doorway _doorway;
	OxygenTanks _tanks1, _tanks2;
	AirLock _airLock;
	TransparentDoor _transparentDoor;
	Aerosol _aerosol;
	SonicStunner _sonicStunner;
	Locker1 _locker1;
	Locker2 _locker2;
	PanelDialog _panelDialog;
	ASoundExt _sound1;
	SequenceManager _sequenceManager1, _sequenceManager2;
public:
	void postInit(SceneObjectList *OwnerList = NULL) override;
	void synchronize(Serializer &s) override;
	void signal() override;
};

class Scene525: public SceneExt {
public:
	SceneActor _actor1;
	SequenceManager _sequenceManager;

	void postInit(SceneObjectList *OwnerList = NULL) override;
	void signal() override;
};

class Scene600 : public SceneExt {
	class CompartmentHotspot : public NamedHotspot {
	public:
		bool startAction(CursorType action, Event &event) override;
	};
	class EngineCompartment : public NamedHotspot {
	public:
		bool startAction(CursorType action, Event &event) override;
	};

	class Smoke : public SceneActor {
	public:
		void signal() override;
		bool startAction(CursorType action, Event &event) override;
		void draw() override;
	};
	class Doorway : public SceneActor {
	public:
		bool startAction(CursorType action, Event &event) override;
	};
	class Laser : public SceneActor {
	public:
		bool startAction(CursorType action, Event &event) override;
	};
	class Aerosol : public SceneActor {
	public:
		bool startAction(CursorType action, Event &event) override;
	};
	class Scanner : public SceneActor {
	public:
		bool startAction(CursorType action, Event &event) override;
	};
public:
	int _roomState;
	CompartmentHotspot _quantumDrive;
	CompartmentHotspot _quantumRegulator;
	CompartmentHotspot _powerNode;
	EngineCompartment _engineCompartment;
	CompartmentHotspot _background;
	SceneActor _stasisArea;
	SceneActor _laserBeam;
	SceneActor _computer;
	SceneActor _stasisField;
	Smoke _smoke;
	Doorway _doorway;
	Laser _laser;
	Aerosol _aerosol;
	Scanner _scanner;
	ASoundExt _sound1;
	SequenceManager _sequenceManager1;
	SequenceManager _sequenceManager2;
	byte _pixelMap[256];

	Scene600();
	void postInit(SceneObjectList *OwnerList = NULL) override;
	void remove() override;
	void signal() override;
	void process(Event &event) override;
	void dispatch() override;
	void synchronize(Serializer &s) override;
};

class Scene700: public SceneExt {
	class Loft : public NamedHotspot {
	public:
		bool startAction(CursorType action, Event &event) override;
	};
	class HandGrip : public NamedHotspot {
	public:
		bool startAction(CursorType action, Event &event) override;
	};

	class LiftDoor : public SceneActor {
	public:
		bool startAction(CursorType action, Event &event) override;
	};
	class SuitRoomDoor : public SceneActor {
	public:
		bool startAction(CursorType action, Event &event) override;
	};
	class ControlPanel : public SceneActor {
	public:
		bool startAction(CursorType action, Event &event) override;
	};
	class Cable : public SceneActor {
	public:
		bool startAction(CursorType action, Event &event) override;
	};
	class LoftDoor : public SceneActor {
	public:
		bool startAction(CursorType action, Event &event) override;
	};
public:
	NamedHotspot _background;
	NamedHotspot _debris1;
	NamedHotspot _debris2;
	NamedHotspot _debris3;
	NamedHotspot _debris4;
	NamedHotspot _restraintCollar;
	NamedHotspot _storage2;
	NamedHotspot _storage1;
	NamedHotspot _stars;
	NamedHotspot _light;
	Loft _loft;
	HandGrip _handGrip;
	SceneActor _electromagnet;
	LiftDoor _liftDoor;
	SuitRoomDoor _suitRoomDoor;
	ControlPanel _controlPanel;
	Cable _cable;
	LoftDoor _loftDoor1;
	LoftDoor _loftDoor2;
	LoftDoor _loftDoor3;
	LoftDoor _loftDoor4;
	SequenceManager _sequenceManager;
	PaletteRotation *_rotation;

	Scene700();
	void postInit(SceneObjectList *OwnerList = NULL) override;
	void remove() override;
	void signal() override;
	void synchronize(Serializer &s) override;
};

class Scene800: public SceneExt {
	/* Items */
	class Button: public NamedHotspot {
	public:
		bool startAction(CursorType action, Event &event) override;
	};
	class CableJunction: public NamedHotspot {
	public:
		bool startAction(CursorType action, Event &event) override;
	};
	class DeviceSlot: public NamedHotspot {
	public:
		bool startAction(CursorType action, Event &event) override;
	};

	/* Objects */
	class Door: public SceneActor {
	public:
		bool startAction(CursorType action, Event &event) override;
	};
	class Tray: public SceneActor {
	public:
		bool startAction(CursorType action, Event &event) override;
	};
	class ComScanner: public SceneActor {
	public:
		bool startAction(CursorType action, Event &event) override;
	};
	class Cabinet: public SceneActor {
	public:
		bool startAction(CursorType action, Event &event) override;
	};
public:
	NamedHotspot _background, _autoDoc, _diskSlot, _couch;
	NamedHotspot _medicalDatabase, _dataConduits;
	Button _button;
	CableJunction _cableJunction;
	DeviceSlot _deviceSlot;
	SceneActor _autodocCover, _opticalFiber, _reader;
	Door _door;
	Tray _tray;
	ComScanner _comScanner;
	Cabinet _cabinet;
	SequenceManager _sequenceManager1;

	void postInit(SceneObjectList *OwnerList = NULL) override;
	void signal() override;
};

class Scene825: public SceneExt {
	/* Objects */
	class Button: public SceneObject {
	public:
		int _buttonId;
		bool _buttonDown;
		SceneText _sceneText;
	public:
		Button();
		void setButton(int buttonId);
		void setText(int textId);

		void synchronize(Serializer &s) override;
		void process(Event &event) override;
		bool startAction(CursorType action, Event &event) override;
	};
public:
	NamedHotspot _background, _console;
	SceneActor _vertLine1, _vertLine2, _vertLine3, _vertLine4, _vertLine5;
	Button _button1, _button2, _button3, _button4, _button5, _button6;
	ASoundExt _sound1, _sound2, _sound3, _sound4;
	SequenceManager _sequenceManager1;
	SceneText _sceneText;
	int _menuId, _frame1, _frame2;
	const char *_autodocItems[11];
public:
	Scene825();
	void synchronize(Serializer &s) override;
	void postInit(SceneObjectList *OwnerList = NULL) override;
	void remove() override;
	void signal() override;
	void process(Event &event) override;
	void dispatch() override;

	void doButtonPress(int buttonId);
};

class Scene850: public SceneExt {
	/* Items */
	class Indicator: public NamedHotspot {
	public:
		bool startAction(CursorType action, Event &event) override;
	};

	/* Objects */
	class LiftDoor: public SceneActor {
	public:
		bool startAction(CursorType action, Event &event) override;
	};
	class SickBayDoor: public SceneActor {
	public:
		bool startAction(CursorType action, Event &event) override;
	};
	class Clamp: public SceneActor {
	public:
		bool startAction(CursorType action, Event &event) override;
	};
	class Panel: public SceneActor {
	public:
		bool startAction(CursorType action, Event &event) override;
	};
public:
	NamedHotspot _background, _eastDoor, _compartment, _sickBayIndicator;
	NamedHotspot _liftControls;
	Indicator _indicator;
	SceneActor _spark, _fiber;
	LiftDoor _liftDoor;
	SickBayDoor _sickBayDoor;
	Clamp _clamp;
	Panel _panel;
	SequenceManager _sequenceManager1;

	void postInit(SceneObjectList *OwnerList = NULL) override;
	void signal() override;
};

class Scene900 : public SceneExt {
	class Button : public SceneActor {
	public:
		int _buttonId;

		Button();
		void initButton(int buttonId);
		void synchronize(Serializer &s) override;
		bool startAction(CursorType action, Event &event) override;
	};
public:
	int _controlsScreenNumber;
	Common::Point _magnetChangeAmount;
	NamedHotspot _background;
	SceneActor _controls;
	SceneActor _cable;
	SceneActor _electromagnet;
	Button _button1;
	Button _button2;
	Button _button3;
	Button _button4;
	Button _button5;
	Button _button6;
	Button _button7;
	ASoundExt _aSound1;
	SequenceManager _sequenceManager1;

	Scene900();
	void postInit(SceneObjectList *OwnerList = NULL) override;
	void remove() override;
	void signal() override;
	void dispatch() override;
	void synchronize(Serializer &s) override;
};

} // End of namespace Ringworld2

} // End of namespace TsAGE

#endif
