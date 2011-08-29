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

#ifndef TSAGE_BLUEFORCE_LOGIC_H
#define TSAGE_BLUEFORCE_LOGIC_H

#include "common/scummsys.h"
#include "tsage/events.h"
#include "tsage/core.h"
#include "tsage/scenes.h"
#include "tsage/globals.h"

namespace TsAGE {

namespace BlueForce {

using namespace TsAGE;

#define BLUE_INVENTORY (*((::TsAGE::BlueForce::BlueForceInvObjectList *)_globals->_inventory))

class BlueForceGame: public Game {
public:
	virtual void start();
	virtual Scene *createScene(int sceneNumber);
	virtual void rightClick();
};

#define OBJ_ARRAY_SIZE 10
class AObjectArray: public EventHandler {
public:
	EventHandler *_objList[OBJ_ARRAY_SIZE];
	bool _inUse;
	int getNewIndex();
public:
	AObjectArray();
	void clear();

	virtual Common::String getClassName() { return "AObjectArray"; }
	virtual void synchronize(Serializer &s);
	virtual void process(Event &event);
	virtual void dispatch();

	void add(EventHandler *obj);
	void remove(EventHandler *obj);
};

class Timer: public EventHandler {
public:
	Action *_tickAction;
	Action *_endAction;
	uint32 _endFrame;
public:
	Timer();
	void set(uint32 delay, Action *endAction);

	virtual Common::String getClassName() { return "Timer"; }
	virtual void synchronize(Serializer &s);
	virtual void remove();
	virtual void signal();
	virtual void dispatch();
};

class TimerExt: public Timer {
public:
	Action *_newAction;
public:	
	TimerExt();
	void set(uint32 delay, Action *endAction, Action *action);

	virtual Common::String getClassName() { return "TimerExt"; }
	virtual void synchronize(Serializer &s);
	virtual void remove();
	virtual void signal();
	virtual void dispatch();
};	

class SceneItemType2: public SceneHotspot {
public:
	virtual void startMove(SceneObject *sceneObj, va_list va);
};

class NamedObject: public SceneObject {
public:
	int _resNum;
	int _lookLineNum, _talkLineNum, _useLineNum;

	virtual Common::String getClassName() { return "NamedObject"; }
	virtual void synchronize(Serializer &s);
	virtual void postInit(SceneObjectList *OwnerList = NULL);

	void setup(int resNum, int lookLineNum, int talkLineNum, int useLineNum, int mode, SceneItem *item);
};

class CountdownObject: public NamedObject {
public:
	int _countDown;
	CountdownObject();
	void fixCountdown(int mode, ...);

	virtual Common::String getClassName() { return "CountdownObject"; }
	virtual void synchronize(Serializer &s);
	virtual void dispatch();
};

class FollowerObject: public NamedObject {
public:
	SceneObject *_object;
	FollowerObject();

	virtual Common::String getClassName() { return "SceneObjectExt4"; }
	virtual void synchronize(Serializer &s);
	virtual void remove();
	virtual void dispatch();
	virtual void reposition();

	void setup(SceneObject *object, int visage, int frameNum, int yDiff);
};

class SceneExt: public Scene {
public:
	AObjectArray _timerList, _objArray2;
	int _field372;
	int _field37A;
	EventHandler *_eventHandler;

	Rect _v51C34;
public:
	SceneExt();

	virtual Common::String getClassName() { return "SceneExt"; }
	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void process(Event &event);
	virtual void dispatch();
	virtual void loadScene(int sceneNum);
	virtual void proc13() { warning("TODO: SceneExt::proc13"); }

	void addTimer(Timer *timer) { _timerList.add(timer); }
	void removeTimer(Timer *timer) { _timerList.remove(timer); }
};

class GameScene: public SceneExt {
public:
	int _field412;
	int _field794;
public:
	GameScene();

	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void remove();
};

class SceneHandlerExt: public SceneHandler {
public:
	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void process(Event &event);
};

class VisualSpeaker: public Speaker {
public:
	NamedObject _object1;
	CountdownObject _object2;
	bool _removeObject1, _removeObject2;
	int _field20C, _field20E;
	int _numFrames;
	Common::Point _offsetPos;
public:
	VisualSpeaker();

	virtual Common::String getClassName() { return "VisualSpeaker"; }
	virtual void synchronize(Serializer &s);
	virtual void remove();
	virtual void proc12(Action *action);
	virtual void setText(const Common::String &msg);
};

class SpeakerSutter: public VisualSpeaker {
public:
	SpeakerSutter();

	virtual Common::String getClassName() { return "SpeakerSutter"; }
	virtual void setText(const Common::String &msg);
};

class SpeakerDoug: public VisualSpeaker {
public:
	SpeakerDoug();

	virtual Common::String getClassName() { return "SpeakerDoug"; }
};

class SpeakerJakeNoHead: public VisualSpeaker {
public:
	SpeakerJakeNoHead();

	virtual Common::String getClassName() { return "SpeakerJakeNoHead"; }
};

class BlueForceInvObjectList : public InvObjectList {
public:
	InvObject _business_card;
	InvObject _lauras_sweater;
	InvObject _handcuffs;
	InvObject _magnum;
	InvObject _ticket_book;
	InvObject _miranda_card;
	InvObject _forest_follet;
	InvObject _bradford_id;
	InvObject _baseball_card;
	InvObject _slip_bradford;
	InvObject _flare;
	InvObject _rap_sheet;
	InvObject _cartridges;
	InvObject _rifle;
	InvObject _wig;
	InvObject _frankies_id;
	InvObject _tyrones_id;
	InvObject _pistol22;
	InvObject _unused;
	InvObject _slip_frankie;
	InvObject _slip_tyrone;
	InvObject _atf_teletype;
	InvObject _da_note;
	InvObject _blueprints;
	InvObject _planter_key; 
	InvObject _center_punch;
	InvObject _tranquilizer;
	InvObject _boat_hook;
	InvObject _oily_rags;
	InvObject _fuel_jar;
	InvObject _screwdriver;
	InvObject _floppy_disk1;
	InvObject _floppy_disk2;
	InvObject _driftwood;
	InvObject _crate_piece1;
	InvObject _crate_piece2;
	InvObject _shoebox;
	InvObject _badge;
	InvObject _unused2;
	InvObject _rental_coupons;
	InvObject _nickel;
	InvObject _calendar;
	InvObject _dixon_note;
	InvObject _cobb_mugshot;
	InvObject _murder_article;
	InvObject _microfiche;
	InvObject _future_wave_keys;
	InvObject _rental_boat_keys;
	InvObject _napkin;
	InvObject _cobb_printout;
	InvObject _fishing_net;
	InvObject _id;
	InvObject _rounds_9mm;
	InvObject _dates_note;
	InvObject _hand_grenade;
	InvObject _cord_110;
	InvObject _cord_110_plug;
	InvObject _cord_220;
	InvObject _unused3;
	InvObject _cord_220_plug;
	InvObject _official_document;
	InvObject _red_sweater;
	InvObject _jackknife;
	InvObject _whistle;
	InvObject _gun;
	InvObject _alley_cat_key;

	BlueForceInvObjectList();

	virtual Common::String getClassName() { return "BlueForceInvObjectList"; }
};

} // End of namespace BlueForce

} // End of namespace TsAGE

#endif
