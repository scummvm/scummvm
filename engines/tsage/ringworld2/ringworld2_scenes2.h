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
		void signal() override;
	};

	class WestExit : public SceneExit {
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
	class NorthExit : public SceneExit {
	public:
		void changeScene() override;
	};
	class DoorExit : public SceneExit {
	public:
		void changeScene() override;
	};
public:
	bool _exitingFlag;
	int _mazePlayerMode;

	NamedHotspot _background;
	SceneActor _companion;
	SceneActor _persons[11];
	WestExit _westExit;
	EastExit _eastExit;
	SouthExit _southExit;
	NorthExit _northExit;
	DoorExit _doorExit;
	Action1 _action1, _action2, _action3, _action4, _action5;
	SequenceManager _sequenceManager;

	Scene2000();
	void postInit(SceneObjectList *OwnerList = NULL) override;
	void remove() override;
	void signal() override;
	void process(Event &event) override;
	void synchronize(Serializer &s) override;

	void initExits();
	void initPlayer();
};

class Scene2350 : public SceneExt {
	class Companion : public SceneActor {
		bool startAction(CursorType action, Event &event) override;
	};
	class Balloon : public SceneActor {
		bool startAction(CursorType action, Event &event) override;
	};

	class ExitUp : public SceneExit {
		void changeScene() override;
	};
	class ExitWest : public SceneExit {
		void changeScene() override;
	};
public:
	SpeakerQuinn _quinnSpeaker;
	SpeakerPharisha _pharishaSpeaker;
	NamedHotspot _background;
	SceneActor _person;
	Companion _companion;
	Balloon _balloon;
	Balloon _harness;
	ExitUp _exitUp;
	ExitWest _exitWest;
	SequenceManager _sequenceManager;

	void postInit(SceneObjectList *OwnerList = NULL) override;
	void remove() override;
	void signal() override;
};

class Scene2400 : public SceneExt {
	class WestExit : public SceneExit {
		void changeScene() override;
	};
	class EastExit : public SceneExit {
		void changeScene() override;
	};
public:
	WestExit _westExit;
	EastExit _eastExit;
	SequenceManager _sequenceManager;

	void postInit(SceneObjectList *OwnerList = NULL) override;
	void signal() override;
};

class Scene2425 : public SceneExt {
	class RopeDest1 : public NamedHotspot {
	public:
		bool startAction(CursorType action, Event &event) override;
	};
	class RopeDest2 : public NamedHotspot {
	public:
		bool startAction(CursorType action, Event &event) override;
	};
	class Crevasse : public NamedHotspot {
	public:
		bool startAction(CursorType action, Event &event) override;
	};
	class Background : public NamedHotspot {
	public:
		bool startAction(CursorType action, Event &event) override;
	};

	class Rope : public SceneActor {
	public:
		bool startAction(CursorType action, Event &event) override;
	};
	class Pictographs : public SceneActor {
	public:
		bool startAction(CursorType action, Event &event) override;
	};

	class SouthEastExit : public SceneExit {
	public:
		void changeScene() override;
	};
public:
	RopeDest1 _ropeDest1;
	RopeDest2 _ropeDest2;
	Crevasse _crevasse;
	Background _background;
	Rope _rope;
	Pictographs _pictographs1;
	Pictographs _pictographs2;
	SouthEastExit _southEastExit;
	SequenceManager _sequenceManager;

	void postInit(SceneObjectList *OwnerList = NULL) override;
	void remove() override;
	void signal() override;
};

class Scene2430 : public SceneExt {
	class Companion : public SceneActor {
	public:
		bool startAction(CursorType action, Event &event) override;
	};
	class GunPowder : public SceneActor {
	public:
		bool startAction(CursorType action, Event &event) override;
	};
	class OilLamp : public SceneActor {
	public:
		bool startAction(CursorType action, Event &event) override;
	};

	class SouthExit : public SceneExit {
	public:
		void changeScene() override;
	};
public:
	NamedHotspot _background;
	NamedHotspot _bottles2;
	NamedHotspot _furnishings;
	NamedHotspot _rug1;
	NamedHotspot _mirror;
	NamedHotspot _garments;
	NamedHotspot _bed;
	NamedHotspot _towel;
	NamedHotspot _bottles1;
	NamedHotspot _post;
	NamedHotspot _clothesPile1;
	NamedHotspot _clothesPile2;
	NamedHotspot _rug2;
	Companion _companion;
	GunPowder _gunPowder;
	OilLamp _oilLamp;
	SouthExit _southExit;
	SequenceManager _sequenceManager;

	void postInit(SceneObjectList *OwnerList = NULL) override;
	void signal() override;
};

class Scene2435 : public SceneExt {
	class Companion : public SceneActor {
	public:
		bool startAction(CursorType action, Event &event) override;
	};
	class Astor : public SceneActor {
	public:
		bool startAction(CursorType action, Event &event) override;
	};

	class SouthExit : public SceneExit {
	public:
		void changeScene() override;
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
	SouthExit _southExit;
	SequenceManager _sequenceManager;

	void postInit(SceneObjectList *OwnerList = NULL) override;
	void remove() override;
	void signal() override;
};

class Scene2440 : public SceneExt {
	class Companion : public SceneActor {
	public:
		bool startAction(CursorType action, Event &event) override;
	};
	class OilLamp : public SceneActor {
	public:
		bool startAction(CursorType action, Event &event) override;
	};

	class SouthEastExit : public SceneExit {
	public:
		void changeScene() override;
	};
public:
	NamedHotspot _background;
	NamedHotspot _garments;
	NamedHotspot _bedspread;
	NamedHotspot _post;
	NamedHotspot _rug;
	NamedHotspot _furnishings;
	NamedHotspot _bottles;
	Companion _companion;
	OilLamp _oilLamp;
	SouthEastExit _southEastExit;
	SequenceManager _sequenceManager;

	void postInit(SceneObjectList *OwnerList = NULL) override;
	void remove() override;
	void signal() override;
};

class Scene2445 : public SceneExt {
public:
	SequenceManager _sequenceManager;

	void postInit(SceneObjectList *OwnerList = NULL) override;
	void signal() override;
};

class Scene2450 : public SceneExt {
	class Parker : public SceneActor {
	public:
		bool startAction(CursorType action, Event &event) override;
	};
	class CareTaker : public SceneActor {
	public:
		bool startAction(CursorType action, Event &event) override;
	};

	class SouthWestExit : public SceneExit {
	public:
		void changeScene() override;
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
	SouthWestExit _southWestExit;
	SequenceManager _sequenceManager;

	void postInit(SceneObjectList *OwnerList = NULL) override;
	void remove() override;
	void signal() override;
};

class Scene2455 : public SceneExt {
	class Lamp : public SceneActor {
	public:
		bool startAction(CursorType action, Event &event) override;
	};
	class Pool : public SceneActor {
	public:
		bool startAction(CursorType action, Event &event) override;
	};
	class ScrithKey : public SceneActor {
	public:
		bool startAction(CursorType action, Event &event) override;
	};

	class NorthExit : public SceneExit {
	public:
		void changeScene() override;
	};
public:
	NamedHotspot _background;
	Lamp _lamp;
	Pool _pool;
	ScrithKey _scrithKey;
	NorthExit _northExit;
	SequenceManager _sequenceManager;

	void postInit(SceneObjectList *OwnerList = NULL) override;
	void remove() override;
	void signal() override;
};

class Scene2500 : public SceneExt {
	class WestExit : public SceneExit {
	public:
		void changeScene() override;
	};
public:
	SpeakerQuinn _quinnSpeaker;
	SpeakerSeeker _seekerSpeaker;
	SpeakerMiranda _mirandaSpeaker;
	SpeakerWebbster2500 _webbsterSpeaker;
	NamedHotspot _background;
	SceneActor _companion;
	SceneActor _quinn;
	SceneActor _ship;
	WestExit _westExit;
	SequenceManager _sequenceManager;

	void postInit(SceneObjectList *OwnerList = NULL) override;
	void signal() override;
};

class Scene2525 : public SceneExt {
	class StopCock : public NamedHotspot {
	public:
		bool startAction(CursorType action, Event &event) override;
	};

	class GlassDome : public SceneActor {
	public:
		bool startAction(CursorType action, Event &event) override;
	};

	class SouthExit : public SceneExit {
	public:
		void changeScene() override;
	};
public:
	NamedHotspot _background;
	NamedHotspot _machine;
	NamedHotspot _pipes1;
	NamedHotspot _pipes2;
	StopCock _stopcock;
	SceneActor _companion;
	SceneActor _compressor;
	GlassDome _glassDome;
	SouthExit _southExit;
	SequenceManager _sequenceManager;

	void postInit(SceneObjectList *OwnerList = NULL) override;
	void remove() override;
	void signal() override;
};

class Scene2530 : public SceneExt {
	class Flask : public SceneActor {
	public:
		bool startAction(CursorType action, Event &event) override;
	};
	class Crank : public SceneActor {
	public:
		bool startAction(CursorType action, Event &event) override;
	};

	class SouthExit : public SceneExit {
	public:
		void changeScene() override;
	};
public:
	NamedHotspot _background;
	NamedHotspot _crank2;
	NamedHotspot _shelf;
	NamedHotspot _item4;
	NamedHotspot _rope;
	SceneActor _companion;
	Flask _flask;
	Crank _crank;
	SouthExit _southExit;
	SequenceManager _sequenceManager;

	void postInit(SceneObjectList *OwnerList = NULL) override;
	void signal() override;
};

class Scene2535 : public SceneExt {
	class RebreatherTank : public SceneActor {
	public:
		bool startAction(CursorType action, Event &event) override;
	};
	class TannerMask : public SceneActor {
	public:
		bool startAction(CursorType action, Event &event) override;
	};

	class SouthExit : public SceneExit {
	public:
		void changeScene() override;
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
	SouthExit _southExit;
	SequenceManager _sequenceManager;

	void postInit(SceneObjectList *OwnerList = NULL) override;
	void signal() override;
};

class Scene2600 : public SceneExt {
public:
	SequenceManager _sequenceManager;
	PaletteRotation *_rotation;

	Scene2600();
	void synchronize(Serializer &s) override;
	void postInit(SceneObjectList *OwnerList = NULL) override;
	void remove() override;
	void signal() override;
};

class Scene2700 : public SceneExt {
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

	class SouthExit: public SceneArea {
	public:
		void process(Event &event) override;
	};
	class EastExit: public SceneArea {
	public:
		void process(Event &event) override;
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
	SouthExit _southExit;
	EastExit _eastExit;
	Rect _walkRect1, _walkRect2, _walkRect3;
	Rect _walkRect4, _walkRect5, _walkRect6;
	SequenceManager _sequenceManager;
	int _areaMode, _moveMode, _stripNumber;

	Scene2700();
	void synchronize(Serializer &s) override;
	void postInit(SceneObjectList *OwnerList = NULL) override;
	void signal() override;
	void process(Event &event) override;
};

class Scene2750 : public SceneExt {
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

	class WestExit: public SceneArea {
	public:
		void process(Event &event) override;
	};
	class EastExit: public SceneArea {
	public:
		void process(Event &event) override;
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
	SceneActor _fire;
	SceneActor _bird1;
	SceneActor _folliage1;
	SceneActor _bird2;
	SceneActor _folliage2;
	SceneActor _folliage3;
	SceneActor _folliage4;
	SceneActor _folliage5;
	SceneActor _folliage6;
	SceneActor _folliage7;
	Action1 _action1;
	Action2 _action2;
	Action3 _action3;
	Action4 _action4;
	Action5 _action5;
	Action6 _action6;
	Action7 _action7;
	WestExit _westExit;
	EastExit _eastExit;
	Rect _walkRect1, _walkRect2, _walkRect3;
	SequenceManager _sequenceManager;
	int _areaMode, _moveMode, _stripNumber;

	Scene2750();
	void synchronize(Serializer &s) override;
	void postInit(SceneObjectList *OwnerList = NULL) override;
	void signal() override;
	void process(Event &event) override;
};

class Scene2800 : public SceneExt {
	class Outpost : public NamedHotspot {
	public:
		bool startAction(CursorType action, Event &event) override;
	};

	class Guard : public SceneActor {
		bool startAction(CursorType action, Event &event) override;
	};

	class Action1: public Action {
	public:
		void signal() override;
	};
	class Action2: public Action {
		SceneObject _object2;
		SceneObject _object3;
		SceneObject _object4;
	public:
		void signal() override;
	};
public:
	SpeakerQuinn2800 _quinnSpeaker;
	SpeakerNej2800 _nejSpeaker;
	SpeakerGuard2800 _guardSpeaker;
	NamedHotspot _background;
	Outpost _outpost;
	Guard _guard;
	SceneActor _nej;
	SceneActor _lightBar;
	SceneObject _bird;
	Action1 _action1;
	Action2 _action2;
	SequenceManager _sequenceManager;
	int _stripNumber;

	Scene2800();
	void synchronize(Serializer &s) override;
	void postInit(SceneObjectList *OwnerList = NULL) override;
	void signal() override;
};

class Scene2900 : public SceneExt {
	/* Items */
	class Scenery : public NamedHotspot {
	public:
		bool startAction(CursorType action, Event &event) override;
	};
	class ControlPanel : public NamedHotspot {
	public:
		bool startAction(CursorType action, Event &event) override;
	};
	class Altimeter : public NamedHotspot {
	public:
		bool startAction(CursorType action, Event &event) override;
	};
	class KnobLeft : public NamedHotspot {
	public:
		bool startAction(CursorType action, Event &event) override;
	};
	class KnobRight : public NamedHotspot {
	public:
		bool startAction(CursorType action, Event &event) override;
	};
	class Skip : public NamedHotspot {
	public:
		bool startAction(CursorType action, Event &event) override;
	};

	/* Actions */
	class Action1: public Action {
	public:
		void signal() override;
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

	bool _controlsActiveChanging;
	bool _altitudeChanging;
	bool _majorMinorFlag;
	int _altitudeMinorChange;
	int _altitudeMajorChange;
	Common::Point _balloonLocation;
	Common::Point _balloonScreenPos;
	int _newAltitude;
	int _xAmount;
	int _xComparison;
	int _fadeCounter;
	bool _paletteReloadNeeded;

	Scene2900();
	void synchronize(Serializer &s) override;
	void postInit(SceneObjectList *OwnerList = NULL) override;
	void remove() override;
	void signal() override;
	void dispatch() override;
	void refreshBackground(int xAmount, int yAmount) override;
};

} // End of namespace Ringworld2
} // End of namespace TsAGE

#endif
