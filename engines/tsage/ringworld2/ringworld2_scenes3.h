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
		virtual bool startAction(CursorType action, Event &event);
	};
public:

	int _field412;
	SpeakerGuard3100 _guardSpeaker;
	NamedHotspot _item1;
	NamedHotspot _item2;
	SceneActor _actor1;
	SceneActor _actor2;
	SceneActor _actor3;
	SceneActor _actor4;
	SceneActor _actor5;
	Guard _guard;
	ASoundExt _sound1;
	SequenceManager _sequenceManager;

	Scene3100();
	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void remove();
	virtual void signal();
	virtual void dispatch();
	virtual void synchronize(Serializer &s);
};

class Scene3125 : public SceneExt {
	class Item1 : public NamedHotspot {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};
	class Item2 : public Item1 {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};
	class Item3 : public Item1 {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};

	class Actor1 : public SceneActor {
		virtual bool startAction(CursorType action, Event &event);
	};
public:

	int _field412;
	Item1 _item1;
	Actor1 _actor1;
	Item2 _item2;
	Item3 _item3;
	SceneActor _actor2;
	SceneActor _actor3;
	SceneActor _actor4;
	SceneActor _actor5;
	SequenceManager _sequenceManager1;
	// Second sequence manager... Unused?
	SequenceManager _sequenceManager2;

	Scene3125();
	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void signal();
	virtual void dispatch();
	virtual void synchronize(Serializer &s);
};

class Scene3150 : public SceneExt {
	class Item5 : public NamedHotspot {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};
	class Item6 : public NamedHotspot {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};
	class Actor4 : public SceneActor {
		virtual bool startAction(CursorType action, Event &event);
	};
	class Actor5 : public SceneActor {
		virtual bool startAction(CursorType action, Event &event);
	};
	class Actor6 : public SceneActor {
		virtual bool startAction(CursorType action, Event &event);
	};
	class Actor7 : public SceneActor {
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

	NamedHotspot _item1;
	NamedHotspot _item2;
	NamedHotspot _item3;
	NamedHotspot _item4;
	Item5 _item5;
	Item6 _item6;
	SceneActor _actor1;
	SceneActor _actor2;
	SceneActor _actor3;
	Actor4 _actor4;
	Actor5 _actor5;
	Actor6 _actor6;
	Actor7 _actor7;
	Exit1 _exit1;
	Exit2 _exit2;
	SequenceManager _sequenceManager;

	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void signal();
	virtual void dispatch();
};

class Scene3175 : public SceneExt {
	class Item1 : public NamedHotspot {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};

	class Actor3 : public SceneActor {
		virtual bool startAction(CursorType action, Event &event);
	};
	class Actor1 : public Actor3 {
		virtual bool startAction(CursorType action, Event &event);
	};
public:

	Item1 _item1;
	Item1 _item2;
	Item1 _item3;
	Actor1 _actor1;
	SceneActor _actor2;
	Actor3 _actor3;
	SequenceManager _sequenceManager;

	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void signal();
};

class Scene3200 : public SceneExt {
public:
	SpeakerRocko3200 _rockoSpeaker;
	SpeakerJocko3200 _jockoSpeaker;
	SpeakerSocko3200 _sockoSpeaker;
	SceneActor _actor1;
	SceneActor _actor2;
	SceneActor _actor3;
	SequenceManager _sequenceManager;

	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void signal();
};

class Scene3210 : public SceneExt {
public:
	SpeakerCaptain3210 _captainSpeaker;
	SpeakerPrivate3210 _privateSpeaker;
	SceneActor _actor1;
	SceneActor _actor2;
	SequenceManager _sequenceManager;

	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void signal();
};

class Scene3220 : public SceneExt {
public:
	SpeakerRocko3220 _rockoSpeaker;
	SpeakerJocko3220 _jockoSpeaker;
	SceneActor _actor1;
	SceneActor _actor2;
	SequenceManager _sequenceManager;

	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void signal();
};

class Scene3230 : public SceneExt {
public:
	SpeakerRocko3230 _rockoSpeaker;
	SpeakerJocko3230 _jockoSpeaker;
	SceneActor _actor1;
	SceneActor _actor2;
	SceneActor _actor3;
	SequenceManager _sequenceManager;

	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void signal();
};

class Scene3240 : public SceneExt {
public:
	SpeakerTeal3240 _tealSpeaker;
	SpeakerWebbster3240 _webbsterSpeaker;
	SpeakerMiranda2500 _mirandaSpeaker;
	SceneActor _actor1;
	SceneActor _actor2;
	SequenceManager _sequenceManager;

	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void signal();
};

class Scene3245 : public SceneExt {
public:
	SpeakerRalf3245 _ralfSpeaker;
	SpeakerTomko3245 _tomkoSpeaker;
	SceneActor _actor1;
	SceneActor _actor2;
	SequenceManager _sequenceManager;

	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void signal();
};

class Scene3250 : public SceneExt {
	class Item : public NamedHotspot {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};

	class Actor : public SceneActor {
		virtual bool startAction(CursorType action, Event &event);
	};
public:

	Item _item1;
	Item _item2;
	Item _item3;
	Item _item4;
	Actor _actor1;
	Actor _actor2;
	Actor _actor3;
	Actor _actor4;
	SceneActor _actor5;
	SceneActor _actor6;
	SceneActor _actor7;
	SequenceManager _sequenceManager;

	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void signal();
	virtual void dispatch();
};

class Scene3255 : public SceneExt {
public:
	SceneActor _actor1;
	SceneActor _actor2;
	SceneActor _actor3;
	SceneActor _actor4;
	SceneActor _actor5;
	SceneActor _actor6;
	SceneActor _actor7;
	SpeakerQuinn3255 _quinnSpeaker;
	SpeakerMiranda3255 _mirandaSpeaker;
	SequenceManager _sequenceManager;

	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void signal();
	virtual void dispatch();
};

class Scene3260 : public SceneExt {
	class Actor13 : public SceneActor {
		virtual bool startAction(CursorType action, Event &event);
	};
	class Actor14 : public SceneActor {
		virtual bool startAction(CursorType action, Event &event);
	};

	class Action1: public Action {
	public:
		void signal();
	};
public:

	NamedHotspot _item1;
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
	Actor13 _actor13;
	Actor14 _actor14;
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

	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void remove();
	virtual void signal();
};

} // End of namespace Ringworld2
} // End of namespace TsAGE

#endif
