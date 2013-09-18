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

#ifndef TSAGE_RINGWORLD2_SCENES2_H
#define TSAGE_RINGWORLD2_SCENES2_H

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

class Scene2000 : public SceneExt {
	class Action1 : public ActionExt {
	public:
		virtual void signal();
	};

	class WestExit : public SceneExit {
	public:
		virtual void changeScene();
	};
	class EastExit : public SceneExit {
	public:
		virtual void changeScene();
	};
	class SouthExit : public SceneExit {
	public:
		virtual void changeScene();
	};
	class NorthExit : public SceneExit {
	public:
		virtual void changeScene();
	};
	class DoorExit : public SceneExit {
	public:
		virtual void changeScene();
	};
public:
	bool _exitingFlag;
	int _mazePlayerMode;

	NamedHotspot _item1;
	SceneActor _object1;
	SceneActor _objList1[11];
	WestExit _westExit;
	EastExit _eastExit;
	SouthExit _southExit;
	NorthExit _northExit;
	DoorExit _doorExit;
	Action1 _action1, _action2, _action3, _action4, _action5;
	SequenceManager _sequenceManager;

	Scene2000();
	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void remove();
	virtual void signal();
	virtual void process(Event &event);
	virtual void synchronize(Serializer &s);

	void initExits();
	void initPlayer();
};

class Scene2350 : public SceneExt {
	class Actor2 : public SceneActor {
		virtual bool startAction(CursorType action, Event &event);
	};
	class Actor3 : public SceneActor {
		virtual bool startAction(CursorType action, Event &event);
	};

	class ExitUp : public SceneExit {
		virtual void changeScene();
	};
	class ExitWest : public SceneExit {
		virtual void changeScene();
	};
public:

	SpeakerQuinn _quinnSpeaker;
	SpeakerPharisha _pharishaSpeaker;
	NamedHotspot _item1;
	SceneActor _actor1;
	Actor2 _actor2;
	Actor3 _actor3;
	Actor3 _actor4;
	ExitUp _exitUp;
	ExitWest _exitWest;
	SequenceManager _sequenceManager;

	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void remove();
	virtual void signal();
	virtual void process(Event &event);
};

class Scene2400 : public SceneExt {
	class Exit1 : public SceneExit {
		virtual void changeScene();
	};
	class Exit2 : public SceneExit {
		virtual void changeScene();
	};
public:
	Exit1 _exit1;
	Exit2 _exit2;
	SequenceManager _sequenceManager;

	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void signal();
};

class Scene2425 : public SceneExt {
	class RopeDest1 : public NamedHotspot {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};
	class RopeDest2 : public NamedHotspot {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};
	class Crevasse : public NamedHotspot {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};
	class Item4 : public NamedHotspot {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};

	class Rope : public SceneActor {
	public:
		bool startAction(CursorType action, Event &event);
	};
	class Pictographs : public SceneActor {
	public:
		bool startAction(CursorType action, Event &event);
	};

	class Exit1 : public SceneExit {
	public:
		virtual void changeScene();
	};
public:
	RopeDest1 _ropeDest1;
	RopeDest2 _ropeDest2;
	Crevasse _crevasse;
	Item4 _item4;
	Rope _rope;
	Pictographs _pictographs1;
	Pictographs _pictographs2;
	Exit1 _exit1;
	SequenceManager _sequenceManager;

	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void remove();
	virtual void signal();
};

class Scene2430 : public SceneExt {
	class Actor1 : public SceneActor {
	public:
		bool startAction(CursorType action, Event &event);
	};
	class GunPowder : public SceneActor {
	public:
		bool startAction(CursorType action, Event &event);
	};
	class OilLamp : public SceneActor {
	public:
		bool startAction(CursorType action, Event &event);
	};

	class Exit1 : public SceneExit {
	public:
		virtual void changeScene();
	};
public:
	NamedHotspot _item1;
	NamedHotspot _item2;
	NamedHotspot _item3;
	NamedHotspot _item4;
	NamedHotspot _item5;
	NamedHotspot _item6;
	NamedHotspot _item7;
	NamedHotspot _item8;
	NamedHotspot _item9;
	NamedHotspot _item10;
	NamedHotspot _item11;
	NamedHotspot _item12;
	NamedHotspot _item13;
	Actor1 _actor1;
	GunPowder _gunPowder;
	OilLamp _oilLamp;
	Exit1 _exit1;
	SequenceManager _sequenceManager;

	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void signal();
};

class Scene2435 : public SceneExt {
	class Companion : public SceneActor {
	public:
		bool startAction(CursorType action, Event &event);
	};
	class Astor : public SceneActor {
	public:
		bool startAction(CursorType action, Event &event);
	};

	class Exit1 : public SceneExit {
	public:
		virtual void changeScene();
	};
public:
	SpeakerQuinn2435 _quinnSpeaker;
	SpeakerSeeker2435 _seekerSpeaker;
	SpeakerPharisha2435 _pharishaSpeaker;
	NamedHotspot _background;
	NamedHotspot _leftWindow;
	NamedHotspot _rightWindow;
	Companion _companion;
	Astor _astor;
	Exit1 _exit1;
	SequenceManager _sequenceManager;

	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void remove();
	virtual void signal();
};

class Scene2440 : public SceneExt {
	class Actor1 : public SceneActor {
	public:
		bool startAction(CursorType action, Event &event);
	};
	class OilLamp : public SceneActor {
	public:
		bool startAction(CursorType action, Event &event);
	};

	class Exit1 : public SceneExit {
	public:
		virtual void changeScene();
	};
public:
	NamedHotspot _item1;
	NamedHotspot _item2;
	NamedHotspot _item3;
	NamedHotspot _item4;
	NamedHotspot _item5;
	NamedHotspot _item6;
	NamedHotspot _item7;
	Actor1 _actor1;
	OilLamp _oilLamp;
	Exit1 _exit1;
	SequenceManager _sequenceManager;

	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void remove();
	virtual void signal();
};

class Scene2445 : public SceneExt {
public:
	SequenceManager _sequenceManager;

	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void signal();
};

class Scene2450 : public SceneExt {
	class Parker : public SceneActor {
	public:
		bool startAction(CursorType action, Event &event);
	};
	class CareTaker : public SceneActor {
	public:
		bool startAction(CursorType action, Event &event);
	};

	class Exit1 : public SceneExit {
	public:
		virtual void changeScene();
	};
public:
	SpeakerQuinn2450 _quinnSpeaker;
	SpeakerSeeker2450 _seekerSpeaker;
	SpeakerCaretaker2450 _caretakerSpeaker;
	NamedHotspot _background;
	NamedHotspot _post;
	NamedHotspot _bedspread;
	SceneActor _companion;
	Parker _parker;
	CareTaker _careTaker;
	Exit1 _exit1;
	SequenceManager _sequenceManager;

	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void remove();
	virtual void signal();
};

class Scene2455 : public SceneExt {
	class Lamp : public SceneActor {
	public:
		bool startAction(CursorType action, Event &event);
	};
	class Pool : public SceneActor {
	public:
		bool startAction(CursorType action, Event &event);
	};
	class ScrithKey : public SceneActor {
	public:
		bool startAction(CursorType action, Event &event);
	};

	class Exit1 : public SceneExit {
	public:
		virtual void changeScene();
	};
public:
	NamedHotspot _background;
	Lamp _lamp;
	Pool _pool;
	ScrithKey _scrithKey;
	Exit1 _exit1;
	SequenceManager _sequenceManager;

	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void remove();
	virtual void signal();
};

class Scene2500 : public SceneExt {
	class Exit1 : public SceneExit {
	public:
		virtual void changeScene();
	};
public:
	SpeakerQuinn _quinnSpeaker;
	SpeakerSeeker _seekerSpeaker;
	SpeakerMiranda _mirandaSpeaker;
	SpeakerWebbster2500 _webbsterSpeaker;
	NamedHotspot _item1;
	SceneActor _actor1;
	SceneActor _actor2;
	SceneActor _actor3;
	Exit1 _exit1;
	SequenceManager _sequenceManager;

	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void signal();
};

class Scene2525 : public SceneExt {
	class Item5 : public NamedHotspot {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};

	class GlassDome : public SceneActor {
	public:
		bool startAction(CursorType action, Event &event);
	};

	class Exit1 : public SceneExit {
	public:
		virtual void changeScene();
	};
public:
	NamedHotspot _item1;
	NamedHotspot _item2;
	NamedHotspot _item3;
	NamedHotspot _item4;
	Item5 _item5;
	SceneActor _actor1;
	SceneActor _actor2;
	GlassDome _glassDome;
	Exit1 _exit1;
	SequenceManager _sequenceManager;

	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void remove();
	virtual void signal();
};

class Scene2530 : public SceneExt {
	class Flask : public SceneActor {
	public:
		bool startAction(CursorType action, Event &event);
	};
	class Crank : public SceneActor {
	public:
		bool startAction(CursorType action, Event &event);
	};

	class Exit1 : public SceneExit {
	public:
		virtual void changeScene();
	};
public:
	NamedHotspot _background;
	NamedHotspot _item2;
	NamedHotspot _shelf;
	NamedHotspot _item4;
	NamedHotspot _item5;
	SceneActor _actor1;
	Flask _flask;
	Crank _crank;
	Exit1 _exit1;
	SequenceManager _sequenceManager;

	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void signal();
};

class Scene2535 : public SceneExt {
	class RebreatherTank : public SceneActor {
	public:
		bool startAction(CursorType action, Event &event);
	};
	class TannerMask : public SceneActor {
	public:
		bool startAction(CursorType action, Event &event);
	};

	class Exit1 : public SceneExit {
	public:
		virtual void changeScene();
	};
public:
	NamedHotspot _background;
	NamedHotspot _roof;
	NamedHotspot _skin1;
	NamedHotspot _skin2;
	NamedHotspot _skin3;
	NamedHotspot _skin4;
	NamedHotspot _depression;
	SceneActor _companion;
	SceneActor _rope;
	RebreatherTank _rebreatherTank;
	TannerMask _tannerMask;
	Exit1 _exit1;
	SequenceManager _sequenceManager;

	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void signal();
};

class Scene2600 : public SceneExt {
public:
	SequenceManager _sequenceManager;
	PaletteRotation *_rotation;

	Scene2600();
	virtual void synchronize(Serializer &s);
	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void remove();
	virtual void signal();
};

class Scene2700 : public SceneExt {
	class Action1: public Action {
	public:
		void signal();
	};
	class Action2: public Action {
	public:
		void signal();
	};
	class Action3: public Action {
	public:
		void signal();
	};
	class Action4: public Action {
	public:
		void signal();
	};

	class Area1: public SceneArea {
	public:
		void process(Event &event);
	};
	class Area2: public SceneArea {
	public:
		void process(Event &event);
	};
public:
	SpeakerQuinn2700 _quinnSpeaker;
	SpeakerNej2700 _nejSpeaker;
	NamedHotspot _background;
	NamedHotspot _ghoulHome1;
	NamedHotspot _ghoulHome2;
	NamedHotspot _ghoulHome3;
	NamedHotspot _ghoulHome4;
	NamedHotspot _ghoulHome5;
	SceneActor _nej;
	SceneActor _ghoulHome6;
	SceneActor _ghoulHome7;
	SceneActor _ghoulHome8;
	SceneActor _ghoulHome9;
	Action1 _action1;
	Action2 _action2;
	Action3 _action3;
	Action4 _action4;
	Area1 _area1;
	Area2 _area2;
	Rect _walkRect1, _walkRect2, _walkRect3;
	Rect _walkRect4, _walkRect5, _walkRect6;
	SequenceManager _sequenceManager;
	int _field412, _field414, _field416;

	Scene2700();
	virtual void synchronize(Serializer &s);
	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void signal();
	virtual void process(Event &event);
};

class Scene2750 : public SceneExt {
	class Action1: public Action {
	public:
		void signal();
	};
	class Action2: public Action {
	public:
		void signal();
	};
	class Action3: public Action {
	public:
		void signal();
	};
	class Action4: public Action {
	public:
		void signal();
	};
	class Action5: public Action {
	public:
		void signal();
	};
	class Action6: public Action {
	public:
		void signal();
	};
	class Action7: public Action {
	public:
		void signal();
	};

	class Area1: public SceneArea {
	public:
		void process(Event &event);
	};
	class Area2: public SceneArea {
	public:
		void process(Event &event);
	};
public:
	SpeakerQuinn2750 _quinnSpeaker;
	SpeakerNej2750 _nejSpeaker;
	NamedHotspot _background;
	NamedHotspot _ghoulHome1;
	NamedHotspot _ghoulHome2;
	NamedHotspot _ghoulHome3;
	NamedHotspot _ghoulHome4;
	SceneActor _nej;
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
	Action1 _action1;
	Action2 _action2;
	Action3 _action3;
	Action4 _action4;
	Action5 _action5;
	Action6 _action6;
	Action7 _action7;
	Area1 _area1;
	Area2 _area2;
	Rect _rect1, _rect2, _rect3;
	SequenceManager _sequenceManager;
	int _field412, _field414, _field416;

	Scene2750();
	virtual void synchronize(Serializer &s);
	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void signal();
	virtual void process(Event &event);
};

class Scene2800 : public SceneExt {
	class Outpost : public NamedHotspot {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};

	class Guard : public SceneActor {
		virtual bool startAction(CursorType action, Event &event);
	};

	class Action1: public Action {
	public:
		void signal();
	};
	class Action2: public Action {
		SceneObject _object2;
		SceneObject _object3;
		SceneObject _object4;
	public:
		void signal();
	};
public:
	SpeakerQuinn2800 _quinnSpeaker;
	SpeakerNej2800 _nejSpeaker;
	SpeakerGuard2800 _guardSpeaker;
	NamedHotspot _item1;
	Outpost _outpost;
	Guard _guard;
	SceneActor _actor2;
	SceneActor _actor3;
	SceneObject _object1;
	Action1 _action1;
	Action2 _action2;
	SequenceManager _sequenceManager;
	int _field412;

	Scene2800();
	virtual void synchronize(Serializer &s);
	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void signal();
};

class Scene2900 : public SceneExt {
	/* Items */
	class Scenery : public NamedHotspot {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};
	class ControlPanel : public NamedHotspot {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};
	class Altimeter : public NamedHotspot {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};
	class KnobLeft : public NamedHotspot {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};
	class KnobRight : public NamedHotspot {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};
	class Skip : public NamedHotspot {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};

	/* Actions */
	class Action1: public Action {
	public:
		void signal();
	};

	/* Miscellaneous */
	class Map {
	private:
		void moveArea(Rect &r, int xAmt, int yAmt);
		void moveLine(int xpSrc, int ypSrc, int xpDest, int ypDest, int width);
		int adjustRect(Common::Rect &r1, const Common::Rect &r2);
		void drawBlock(const byte *data, int xp, int yp, const Rect &r1, const Rect &r2);
	public:
		int _mapWidth, _mapHeight;
		int _field4;
		int _field6;
		int _field8;
		int _fieldA;
		int _resNum;
		int _xV, _yV;
		Rect _bounds;

		Map();
		void load(int resNum);
		Common::Point setPosition(const Common::Point &pos, bool initialFlag = false);
		void synchronize(Serializer &s);
		void redraw(Rect *updateRect = NULL);
	};
public:
	SceneObject _leftEdge;
	SceneObject _rightEdge;
	SceneObject _knob;
	SceneObject _altimeterContent;
	SceneObject _knobRightContent;
	SceneObject _knobLeftContent;
	Scenery _scenery;
	ControlPanel _controlPanel;
	Altimeter _altimeter;
	KnobLeft _knobLeft;
	KnobRight _knobRight;
	Skip _skip;
	Action1 _action1;
	Map _map;
	SceneText _skipText;

	int _field412;
	bool _altitudeChanging;
	bool _field416;
	int _field41C;
	int _altitudeMajorChange;
	Common::Point _balloonLocation;
	Common::Point _balloonScreenPos;
	int _newAltitude;
	int _field425;
	int _field426;
	int _field427;
	bool _field8F8;

	Scene2900();
	virtual void synchronize(Serializer &s);
	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void remove();
	virtual void signal();
	virtual void dispatch();
	virtual void refreshBackground(int xAmount, int yAmount);
};

} // End of namespace Ringworld2
} // End of namespace TsAGE

#endif
