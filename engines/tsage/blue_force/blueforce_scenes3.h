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

#ifndef TSAGE_BLUEFORCE_SCENES3_H
#define TSAGE_BLUEFORCE_SCENES3_H

#include "common/scummsys.h"
#include "tsage/blue_force/blueforce_logic.h"
#include "tsage/converse.h"
#include "tsage/events.h"
#include "tsage/core.h"
#include "tsage/scenes.h"
#include "tsage/globals.h"
#include "tsage/sound.h"

namespace TsAGE {

namespace BlueForce {

using namespace TsAGE;

class Scene300: public SceneExt {
	/* Objects */
	class Object: public NamedObject {
	public:
		int _stripNumber;
	public:
		Object(int stripNumber) { _stripNumber = stripNumber; }

		virtual void startMover(CursorType action);
	};
	class Object17: public NamedObject {
	public:
		virtual void startMover(CursorType action);
	};

	/* Items */
	class Item1: public NamedHotspot {
	public:
		virtual void startMover(CursorType action);
	};
	class Item2: public NamedHotspot {
	public:
		virtual void startMover(CursorType action);
	};
	class Item14: public NamedHotspot {
	public:
		virtual void startMover(CursorType action);
	};
	class Item15: public NamedHotspot {
	public:
		virtual void startMover(CursorType action);
	};

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
	class Action5: public Action {
	public:
		virtual void signal();
	};
private:
	void setup();
public:
	SequenceManager _sequenceManager1, _sequenceManager2;
	SequenceManager _sequenceManager3, _sequenceManager4;
	NamedObject _object1;
	FollowerObject _object2, _object3, _object4, _object5, _object6, _object7;
	SceneObject _object8, _object9, _object10;
	NamedObject _object11, _object12;
	Object _object13, _object14, _object15, _object16;
	Object17 _object17;
	NamedObject _object18, _object19;
	Item1 _item1;
	Item2 _item2;
	NamedHotspot _item3, _item4, _item5, _item6, _item7;
	NamedHotspot _item8, _item9, _item10, _item11;
	NamedHotspot _item12, _item13;
	Item14 _item14;
	Item15 _item15;
	Action1 _action1;
	Action2 _action2;
	Action3 _action3;
	Action4 _action4;
	Action5 _action5;
	SpeakerGameText _gameTextSpeaker;
	SpeakerSutter _sutterSpeaker;
	SpeakerDoug _dougSpeaker;
	SpeakerJakeNoHead _jakeSpeaker;
	TimerExt _timer;
	int _field2760, _field2762;

	Scene300();
	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void signal();
	virtual void process(Event &event);
	virtual void dispatch();
};

} // End of namespace BlueForce

} // End of namespace TsAGE

#endif
