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
	class Actor16 : public SceneActor {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};
	class Actor17 : public SceneActor {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};
	class Actor18 : public SceneActor {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};

public:
	int _field412, _field414;
	SpeakerSeeker1100 _seekerSpeaker;
	SpeakerQuinn1100 _quinnSpeaker;
	SpeakerChief1100 _chiefSpeaker;
	ScenePalette _palette1;
	NamedHotspot _item1;
	NamedHotspot _item2;
	NamedHotspot _item3;
	NamedHotspot _item4;
	NamedHotspot _item5;
	NamedHotspot _item6;
	NamedHotspot _item7;
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
	BackgroundSceneObject _object1;
	BackgroundSceneObject _object2;
	Actor16 _actor16;
	Actor17 _actor17;
	Actor18 _actor18;
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
	class Area1: public SceneArea {
	public:
		class Actor3 : public SceneActorExt {
		public:
			void init(int state);
			virtual bool startAction(CursorType action, Event &event);
		};

		SceneActor _actor2;
		Actor3 _actor3;
		Actor3 _actor4;
		Actor3 _actor5;

		byte _field20;

		Area1();
		void synchronize(Serializer &s);

		virtual void postInit(SceneObjectList *OwnerList = NULL);
		virtual void remove();
		virtual void process(Event &event);
		virtual void proc12(int visage, int stripFrameNum, int frameNum, int posX, int posY);
		virtual void proc13(int resNum, int lookLineNum, int talkLineNum, int useLineNum);
	};

	class Object1 : public SavedObject {
	public:
		Rect _rect1;
		Rect _rect2;

		int _field16;
		int _field26;
		int _field28;
		int _field2A;
		int _field2C;
		int _field2E;
		int _field30;

		Object1();
		void synchronize(Serializer &s);

		int sub51AF8(Common::Point pt);
		bool sub51AFD(Common::Point pt);
		void sub9EDE8(Rect rect);
		int sub9EE22(int &arg1, int &arg2);
		virtual Common::String getClassName() { return "UnkObject1200"; }
	};
		
public:
	NamedHotspot _item1;
	SceneActor _actor1;
	Area1 _area1;
	Object1 _object1;
	SequenceManager _sequenceManager;

	int _field412;
	int _field414;
	int _field416;
	int _field418;
	int _field41A;
	int _field41C;

	Scene1200();
	void synchronize(Serializer &s);

	void sub9DAD6(int indx);

	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void signal();
	virtual void process(Event &event);
	virtual void dispatch();
	virtual void saveCharacter(int characterIndex);
};

class Scene1500 : public SceneExt {
public:
	SceneActor _actor1;
	SceneActor _actor2;
	SceneActor _actor3;
	SceneActor _actor4;
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
	SceneActor _actor1;
	SceneActor _actor2;
	SceneActor _actor3;

	SequenceManager _sequenceManager;

	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void signal();
	virtual void dispatch();
};

class Scene1550 : public SceneExt {
	class SceneActor1550 : public SceneActor {
	public:
		void subA4D14(int frameNumber, int strip);
	};

	class UnkObj15501 : public SceneActor {
	public:
		int _fieldA4;
		int _fieldA6;

		UnkObj15501();
		void synchronize(Serializer &s);

		virtual bool startAction(CursorType action, Event &event);
	};

	class UnkObj15502 : public SceneActor {
	public:
		int _fieldA4;

		UnkObj15502();
		void synchronize(Serializer &s);

		virtual bool startAction(CursorType action, Event &event);
		void subA5CDF(int strip);
	};

	class UnkObj15503 : public SceneActor {
	public:
		int _fieldA4;

		UnkObj15503();
		void synchronize(Serializer &s);

		virtual bool startAction(CursorType action, Event &event);
	};

	class UnkArea1550 : public SceneArea {
	public:
		byte _field20;
		SceneActor _areaActor;
		UnkObj15503 _unkObj155031;
		UnkObj15503 _unkObj155032;

		virtual void remove();
		virtual void process(Event &event);
		virtual void proc12(int visage, int stripFrameNum, int frameNum, int posX, int posY);
		virtual void proc13(int resNum, int lookLineNum, int talkLineNum, int useLineNum);
	};

	class Hotspot1 : public NamedHotspot {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};

	class Hotspot3 : public NamedHotspot {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};

	class Actor6 : public SceneActor {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};

	class Actor7 : public SceneActor {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};

	class Actor8 : public SceneActor {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};

	class Actor9 : public SceneActor {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};

	class Actor10 : public SceneActor {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};

	class Actor11 : public SceneActor {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};

	class Actor12 : public SceneActor {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};

	class Actor13 : public SceneActor {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};

	class Actor14 : public SceneActor1550 {
		// Nothing specific found in the original
		// TODO: check if it's an useless class
	};

public:
	SpeakerQuinn _quinnSpeaker;
	SpeakerSeeker _seekerSpeaker;
	Hotspot1 _item1;
	Hotspot1 _item2;
	Hotspot3 _item3;
	SceneActor _actor1;
	SceneActor _actor2;
	SceneActor _actor3;
	SceneActor _actor4;
	SceneActor _actor5;
	Actor6 _actor6;
	Actor7 _actor7;
	Actor8 _actor8;
	Actor9 _actor9;
	Actor10 _actor10;
	Actor11 _actor11;
	Actor12 _actor12;
	Actor13 _actor13;
	UnkObj15501 _arrUnkObj15501[8];
	Actor14 _actor14;
	Actor14 _actor15;
	Actor14 _actor16;
	Actor14 _actor17;
	Actor14 _actor18;
	Actor14 _actor19;
	UnkObj15502 _arrUnkObj15502[8];
	UnkArea1550 _unkArea1;
	SequenceManager _sequenceManager1;
	SequenceManager _sequenceManager2;

	int _field412;
	byte _field414;
	int _field415;
	int _field417;
	int _field419;

	Scene1550();
	void synchronize(Serializer &s);
	void subA2B2F();

	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void signal();
	virtual void process(Event &event);
	virtual void dispatch();
	virtual void saveCharacter(int characterIndex);
};

class Scene1575 : public SceneExt {
	class Hotspot1 : public NamedHotspot {
	public:
		int _field34;
		int _field36;
		
		Hotspot1();
		void synchronize(Serializer &s);
		void subA910D(int indx);

		virtual void process(Event &event);
		virtual bool startAction(CursorType action, Event &event);
	};
public:
	int _field412;
	int _field414;
	int _field416;
	int _field418;
	int _field41A;
	Hotspot1 _item1;
	Hotspot1 _item2;
	Hotspot1 _item3;
	Hotspot1 _item4;
	Hotspot1 _item5;
	Hotspot1 _item6;
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
	class Hotspot1 : public NamedHotspot {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};
	class Hotspot2 : public NamedHotspot {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};

	class Actor2 : public SceneActor {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};
	class Actor3 : public SceneActor {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};
	class Actor4 : public SceneActor {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};
	class Actor5 : public SceneActor {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};
	class Actor6 : public SceneActor {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};
	class Actor7 : public SceneActor {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};
public:
	int _field412;
	SpeakerQuinn _quinnSpeaker;
	SpeakerSeeker _seekerSpeaker;
	Hotspot1 _item1;
	Hotspot2 _item2;
	NamedHotspot _item3;
	SceneActor _actor1;
	SceneActor _arrActor[8];
	Actor2 _actor2;
	Actor3 _actor3;
	Actor4 _actor4;
	Actor5 _actor5;
	Actor6 _actor6;
	Actor7 _actor7;
	SequenceManager _sequenceManager;

	Scene1580();
	void synchronize(Serializer &s);

	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void signal();
};

class Scene1625 : public SceneExt {
	class Actor7 : public SceneActor {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};
public:
	int _field412;
	SpeakerMiranda1625 _mirandaSpeaker;
	SpeakerTeal1625 _tealSpeaker;
	SpeakerSoldier1625 _soldierSpeaker;
	NamedHotspot _item1;
	SceneActor _actor1;
	SceneActor _actor2;
	SceneActor _actor3;
	SceneActor _actor4;
	SceneActor _actor5;
	SceneActor _actor6;
	Actor7 _actor7;
	SequenceManager _sequenceManager;

	Scene1625();
	void synchronize(Serializer &s);

	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void remove();
	virtual void signal();
	virtual void process(Event &event);
};

class Scene1700 : public SceneExt {
	class Item2 : public NamedHotspot {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};

	class Actor11 : public SceneActor {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};
	class Actor12 : public SceneActor {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};

	class Exit1 : public SceneExit {
	public:
		virtual void changeScene();
	};
	class Exit2 : public SceneExit {
	public:
		virtual void changeScene();
	};
	class Exit3 : public SceneExit {
	public:
		virtual void changeScene();
	};
public:
	SpeakerQuinn _quinnSpeaker;
	SpeakerSeeker _seekerSpeaker;
	NamedHotspot _item1;
	Item2 _item2;
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
	Actor11 _actor11;
	Actor12 _actor12;
	Exit1 _exit1;
	Exit2 _exit2;
	Exit3 _exit3;
	SequenceManager _sequenceManager;
	
	int _field77A;
	int _field77C;

	Scene1700();
	void synchronize(Serializer &s);
	void subAF3F8();

	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void remove();
	virtual void signal();
};

class Scene1800 : public SceneExt {
	class Hotspot5 : public NamedHotspot {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};

	class Actor6 : public SceneActor {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};
	class Actor7 : public SceneActor {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};
	class Actor8 : public SceneActor {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};
	
	class Exit1 : public SceneExit {
	public:
		virtual void changeScene();
	};
public:
	int _field412;
	SpeakerQuinn _quinnSpeaker;
	SpeakerSeeker _seekerSpeaker;
	NamedHotspot _item1;
	NamedHotspot _item2;
	NamedHotspot _item3;
	NamedHotspot _item4;
	Hotspot5 _item5;
	SceneActor _actor1;
	SceneActor _actor2;
	SceneActor _actor3;
	SceneActor _actor4;
	SceneActor _actor5;
	Actor6 _actor6;
	Actor7 _actor7;
	Actor8 _actor8;
	Actor8 _actor9;
	Exit1 _exit1;
	SequenceManager _sequenceManager;

	Scene1800();
	void synchronize(Serializer &s);

	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void signal();
	virtual void saveCharacter(int characterIndex);
};

class Scene1875 : public SceneExt {
	class Actor1875 : public SceneActor {
	public:
		int _fieldA4;
		int _fieldA6;
		
		Actor1875();
		void subB84AB();
		void subB8271(int indx);
		
		void synchronize(Serializer &s);
		virtual void process(Event &event);
	};
public:
	SpeakerQuinn _quinnSpeaker;
	SpeakerSeeker _seekerSpeaker;
	NamedHotspot _item1;
	NamedHotspot _item2;
	SceneActor _actor1;
	SceneActor _actor2;
	SceneActor _actor3;
	Actor1875 _actor4;
	Actor1875 _actor5;
	Actor1875 _actor6;
	Actor1875 _actor7;
	Actor1875 _actor8;
	SequenceManager _sequenceManager;

	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void signal();
	virtual void process(Event &event);
};

class Scene1900 : public SceneExt {
	class Actor2 : public SceneActor {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};

	class Exit1 : public SceneExit {
	public:
		virtual void changeScene();
	};
	class Exit2 : public SceneExit {
	public:
		virtual void changeScene();
	};
public:
	SpeakerSeeker1900 _seekerSpeaker;
	NamedHotspot _item1;
	NamedHotspot _item2;
	SceneActor _actor1;
	BackgroundSceneObject _object1;
	BackgroundSceneObject _object2;
	Actor2 _actor2;
	Actor2 _actor3;
	Exit1 _exit1;
	Exit2 _exit2;
	SequenceManager _sequenceManager1;
	SequenceManager _sequenceManager2;

	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void remove();
	virtual void signal();
};

class Scene1925 : public SceneExt {
	class Hotspot2 : public NamedHotspot {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};
	class Hotspot3 : public NamedHotspot {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};

	class ExitUp : public SceneExit {
	public:
		virtual void changeScene();
	};
	class Exit2 : public SceneExit {
	public:
		virtual void changeScene();
	};
	class Exit3 : public SceneExit {
	public:
		virtual void changeScene();
	};
	class Exit4 : public SceneExit {
	public:
		virtual void changeScene();
	};
public:
	NamedHotspot _item1;
	Hotspot2 _item2;
	Hotspot3 _item3;
	SceneActor _actor1;
	ExitUp _exitUp;
	Exit2 _exit2;
	Exit3 _exit3;
	Exit4 _exit4;
	SequenceManager _sequenceManager;

	int _field9B8;
	int _levelResNum[5];

	Scene1925();
	void synchronize(Serializer &s);

	void changeLevel(bool upFlag);
	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void remove();
	virtual void signal();
};

class Scene1945 : public SceneExt {
	class Hotspot3 : public NamedHotspot {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};
	class Hotspot4 : public NamedHotspot {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};

	class Actor3 : public SceneActor {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};

	class ExitUp : public SceneExit {
	public:
		virtual void changeScene();
	};
	class Exit2 : public SceneExit {
	public:
		virtual void changeScene();
	};
public:
	NamedHotspot _item1;
	NamedHotspot _item2;
	Hotspot3 _item3;
	Hotspot4 _item4;
	SceneActor _actor1;
	SceneActor _actor2;
	Actor3 _actor3;
	ExitUp _exitUp;
	Exit2 _exit2;
	SequenceManager _sequenceManager1;
	SequenceManager _sequenceManager2;

	int _fieldEAA;
	int _fieldEAC;
	CursorType _fieldEAE;

	Scene1945();
	void synchronize(Serializer &s);

	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void remove();
	virtual void signal();
};

class Scene1950 : public SceneExt {
	class Area1: public SceneArea {
	public:
		class Actor10 : public SceneActor {
		public:
			int _fieldA4;
			int _fieldA6;
			int _fieldA8;

			Actor10();
			void synchronize(Serializer &s);

			void init(int indx);
			virtual void process(Event &event);
			virtual bool startAction(CursorType action, Event &event);
		};

		SceneActor _areaActor;
		Actor10 _arrActor1[16];

		byte _field20;
		int _fieldB65;

		Area1();
		void synchronize(Serializer &s);

		virtual void remove();
		virtual void process(Event &event);
		virtual void proc12(int visage, int stripFrameNum, int frameNum, int posX, int posY);
		virtual void proc13(int resNum, int lookLineNum, int talkLineNum, int useLineNum);
	};

	class Hotspot2 : public NamedHotspot {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};

	class Actor2 : public SceneActor {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};
	class Actor3 : public SceneActor {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};
	class Actor4 : public SceneActor {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};
	class Actor5 : public SceneActor {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};
	class Actor8 : public SceneActor {
	public:
		int _fieldA4;
		int _fieldA6;
		int _fieldA8;
		int _fieldAA;
		int _fieldAC;
		byte _fieldAE;
		byte _fieldAF;

		Actor8();
		void synchronize(Serializer &s);

		virtual void signal();
		virtual bool startAction(CursorType action, Event &event);
	};

	class Exit1 : public SceneExit {
	public:
		virtual void changeScene();
	};
	class Exit2 : public SceneExit {
	public:
		virtual void changeScene();
	};
	class Exit3 : public SceneExit {
	public:
		virtual void changeScene();
	};
	class Exit4 : public SceneExit {
	public:
		virtual void changeScene();
	};
	class Exit5 : public SceneExit {
	public:
		virtual void changeScene();
	};
	class Exit6 : public SceneExit {
	public:
		virtual void changeScene();
	};
	class Exit7 : public SceneExit {
	public:
		virtual void changeScene();
	};
	class Exit8 : public SceneExit {
	public:
		virtual void changeScene();
	};
public:
	NamedHotspot _item1;
	Hotspot2 _item2;
	SceneActor _actor1;
	BackgroundSceneObject _object1;
	Actor2 _actor2;
	Actor3 _actor3;
	SceneActor _actor4;
	Actor5 _actor5;
	SceneActor _actor6;
	SceneActor _actor7;
	Actor8 _actor8;
	Area1 _area1;
	Exit1 _exit1;
	Exit2 _exit2;
	Exit3 _exit3;
	Exit4 _exit4;
	Exit5 _exit5;
	Exit6 _exit6;
	Exit7 _exit7;
	Exit8 _exit8;
	SequenceManager _sequenceManager;

	int _field412;
	int _field414;
	int _field416;
	Common::Point _field418;
	int _field41C;

	Scene1950();
	void synchronize(Serializer &s);

	void subBDC1E();
	void subBE59B();
	void subBF4B4(int indx);
	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void remove();
	virtual void signal();
	virtual void process(Event &event);
};
} // End of namespace Ringworld2
} // End of namespace TsAGE

#endif
