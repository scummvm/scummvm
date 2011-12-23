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
		void signal();
	};
	
public:
	Action1 _action1;	

	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void process(Event &event);
};

class Scene100: public SceneExt {
	/* Objects */
	class Door: public SceneActorExt {
	public:
		bool startAction(CursorType action, Event &event);
	};
	class Table: public SceneActor {
	public:
		bool startAction(CursorType action, Event &event);
	};
	class StasisNegator: public SceneActor {
	public:
		bool startAction(CursorType action, Event &event);
	};
	class DoorDisplay: public SceneActorExt {
	public:
		bool startAction(CursorType action, Event &event);
	};
	class SteppingDisks: public SceneActor {
	public:
		bool startAction(CursorType action, Event &event);
	};

	/* Items */
	class Terminal: public NamedHotspot{
	public:
		bool startAction(CursorType action, Event &event);
	};

public:
	NamedHotspot _background, _duct, _bed, _desk;
	Terminal _terminal;
	SceneActor _bedLights1, _bedLights2, _object3, _object4, _object5;
	SceneActor _wardrobe;
	Door _door;
	Table _table;
	StasisNegator _stasisNegator;
	DoorDisplay _doorDisplay;
	SteppingDisks _steppingDisks;
	SequenceManager _sequenceManager1, _sequenceManager2;

	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void remove();
	virtual void signal();
	virtual void dispatch();
};

class Scene125: public SceneExt {
	/* Objects */
	class Object5: public SceneActor {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};

	class Icon: public SceneActor {
	public:
		int _lookLineNum, _field98;
		bool _pressed;
		SceneObject _object1, _object2;
		SceneText _sceneText1, _sceneText2;

		Icon();
		virtual Common::String getClassName() { return "Scene125_Icon"; }
		virtual void postInit(SceneObjectList *OwnerList = NULL);
		virtual void synchronize(Serializer &s);
		virtual void process(Event &event);

		void setIcon(int id);
		void showIcon();
		void hideIcon();
	};

	/* Items */
	class Item4: public NamedHotspot {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};

public:
	ScenePalette _palette;
	ASoundExt _sound1;
	NamedHotspot _background, _item2, _item3;
	Item4 _item4;
	SceneActor _object1, _object2, _object3, _object4, _object5, _object6, _object7;
	Icon _icon1, _icon2, _icon3, _icon4, _icon5,  _icon6;
	SequenceManager _sequenceManager;
	SceneText _sceneText;
	int _consoleMode, _iconFontNumber, _logIndex, _databaseIndex, _infodiskIndex;
	int _soundCount, _soundIndex;
	int _soundIndexes[10];

	Scene125();
	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void synchronize(Serializer &s);
	virtual void signal();
	virtual void process(Event &event);
	virtual void dispatch();

	void consoleAction(int id);
	void setDetails(int resNum, int lineNum);
	void stop();
	Common::String parseMessage(const Common::String &msg);
};

class Scene150: public Scene100 {
public:
	SceneActor _emptyRoomTable;
public:
	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void remove();
	virtual void signal();
};

class Scene175: public Scene150 {
};

class Scene200: public SceneExt {
	/* Objects */
	class NorthDoor: public SceneActor {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};
	class EastDoor: public SceneActor {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};
	class WestDoor: public SceneActor {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};

	/* Scene Exits */
	class EastExit: public SceneExit {
	public:
		virtual void changeScene();
	};
	class WestExit: public SceneExit {
	public:
		virtual void changeScene();
	};
public:
	NamedHotspot _background, _compartment, _westDoorDisplay, _eastDoorDisplay;
	NorthDoor _northDoor;
	EastDoor _eastDoor;
	WestDoor _westDoor;
	EastExit _eastExit;
	WestExit _westExit;
	SequenceManager _sequenceManager;

	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void signal();
};

class Scene250: public SceneExt {
	class Button: public SceneActor {
	public:
		int _floorNumber, _v2;
		Button();
		void setFloor(int floorNumber);

		virtual void synchronize(Serializer &s);
		virtual bool startAction(CursorType action, Event &event);
	};
public:
	int _field412, _field414, _field416, _field418, _field41A;
	NamedHotspot _background, _item2, _item3, _item4;
	Button _button1, _currentFloor;
	Button _floor1, _floor2, _floor3, _floor4, _floor5;
	Button _floor6, _floor7, _floor8, _floor9;
	ASoundExt _sound1;
	SequenceManager _sequenceManager1;
public:
	Scene250();
	void changeFloor(int floorNumber);

	virtual void synchronize(Serializer &s);
	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void signal();
	virtual void process(Event &event);
	virtual void dispatch();
};

class Scene300: public SceneExt {
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

	/* Items */
	class QuinnWorkstation: public NamedHotspot {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};
	class MirandaWorkstation: public NamedHotspot {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};
	class SeekerWorkstation: public NamedHotspot {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};

	/* Objects */
	class Miranda: public SceneActor {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};
	class Seeker: public SceneActor {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};
	class Quinn: public SceneActor {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};
	class Doorway: public SceneActor {
	public:
		virtual bool startAction(CursorType action, Event &event);
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
	SceneActor _object1, _object2, _object3, _object4, _protocolDisplay;
	SceneActor _object6, _object7, _object8, _object9;
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

	virtual void synchronize(Serializer &s);
	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void remove();
	virtual void signal();
};

class Scene400: public SceneExt {
	/* Items */
	class Terminal: public NamedHotspot {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};

	/* Objects */
	class Door: public SceneActor {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};
	class Reader: public SceneActor {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};
	class SensorProbe: public SceneActor {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};
	class AttractorUnit: public SceneActor {
	public:
		virtual bool startAction(CursorType action, Event &event);
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

	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void remove();
	virtual void signal();
	virtual void dispatch();
};

class Scene800: public SceneExt {
	/* Items */
	class Button: public NamedHotspot {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};
	class CableJunction: public NamedHotspot {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};
	class DeviceSlot: public NamedHotspot {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};

	/* Objects */
	class Door: public SceneActor {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};
	class Tray: public SceneActor {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};
	class ComScanner: public SceneActor {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};
	class Cabinet: public SceneActor {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};
public:
	NamedHotspot _background, _autoDoc, _diskSlot, _couch;
	NamedHotspot _medicalDatabase, _dataConduits;
	Button _button;
	CableJunction _cableJunction;
	DeviceSlot _deviceSlot;
	SceneActor _autodocCover, _opticalFibre, _reader;
	Door _door;
	Tray _tray;
	ComScanner _comScanner;
	Cabinet _cabinet;
	SequenceManager _sequenceManager1;

	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void signal();
};

class Scene825: public SceneExt {
	/* Objects */
	class Button: public SceneObject {
	public:
		int _buttonId, _v2;
		bool _buttonDown;
		SceneText _sceneText;
	public:
		Button();
		void setButton(int buttonId);
		void setText(int textId);

		virtual void synchronize(Serializer &s);
		virtual void process(Event &event);
		virtual bool startAction(CursorType action, Event &event);
	};
public:
	NamedHotspot _background, _item2;
	SceneActor _object1, _object2, _object3, _object4, _object5;
	Button _button1, _button2, _button3, _button4, _button5, _button6;
	ASoundExt _sound1, _sound2, _sound3, _sound4;
	SequenceManager _sequenceManager1;
	SceneText _sceneText;
	int _menuId, _frame1, _frame2;
public:
	Scene825();
	virtual void synchronize(Serializer &s);
	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void remove();
	virtual void signal();
	virtual void process(Event &event);
	virtual void dispatch();

	void doButtonPress(int buttonId);
};

class Scene850: public SceneExt {
	/* Items */
	class Indicator: public NamedHotspot {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};

	/* Objects */
	class LiftDoor: public SceneActor {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};
	class SickBayDoor: public SceneActor {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};
	class Clamp: public SceneActor {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};
	class Panel: public SceneActor {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};
public:
	NamedHotspot _background, _eastDoor, _compartment, _sickBayIndicator;
	NamedHotspot _liftControls;
	Indicator _indicator;
	SceneActor _object1, _fibre;
	LiftDoor _liftDoor;
	SickBayDoor _sickBayDoor;
	Clamp _clamp;
	Panel _panel;
	SequenceManager _sequenceManager1;

	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void signal();
};

} // End of namespace Ringworld2

} // End of namespace TsAGE

#endif
