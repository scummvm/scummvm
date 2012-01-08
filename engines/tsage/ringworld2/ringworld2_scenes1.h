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
		bool startAction(CursorType action, Event &event);
	};
	class Actor17 : public SceneActor {
	public:
		bool startAction(CursorType action, Event &event);
	};
	class Actor18 : public SceneActor {
	public:
		bool startAction(CursorType action, Event &event);
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
	SpeakerQuinn1530 _quinnSpeaker;
	SpeakerSeeker1530 _seekerSpeaker;
	SceneActor _actor1;
	SceneActor _actor2;
	SceneActor _actor3;

	SequenceManager _sequenceManager;

	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void signal();
	virtual void dispatch();
};
} // End of namespace Ringworld2
} // End of namespace TsAGE

#endif
