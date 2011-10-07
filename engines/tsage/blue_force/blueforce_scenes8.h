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

#ifndef TSAGE_BLUEFORCE_SCENES8_H
#define TSAGE_BLUEFORCE_SCENES8_H

#include "common/scummsys.h"
#include "tsage/blue_force/blueforce_logic.h"
#include "tsage/blue_force/blueforce_speakers.h"
#include "tsage/converse.h"
#include "tsage/events.h"
#include "tsage/core.h"
#include "tsage/scenes.h"
#include "tsage/globals.h"
#include "tsage/sound.h"

namespace TsAGE {

namespace BlueForce {

using namespace TsAGE;

class Scene800: public SceneExt {
	/* Actions */
	class Action1 : public Action {
	public:
		virtual void signal();
	};

	/* Items */
	class Item1: public SceneHotspot {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};
	class Item2: public SceneHotspot {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};

	/* Objects */
	class Doorway: public NamedObject {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};
	class Car1: public NamedObject {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};
	class Motorcycle: public NamedObject {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};
	class Lyle: public NamedObject {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};
	class Car2: public NamedObject {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};

public:
	Action1 _action1;
	SequenceManager _sequenceManager;
	SpeakerJakeJacket _jakeJacketSpeaker;
	SpeakerLyleHat _lyleHatSpeaker;
	SpeakerGameText _gameTextSpeaker;
	Doorway _doorway;
	Car1 _car1;
	Motorcycle _motorcycle;
	Lyle _lyle;
	Car2 _car2;
	NamedObject _object6;
	Item1 _item1;
	Item2 _item2;
	SceneText _text;

	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void signal();
	virtual void dispatch();
};

class Scene830: public PalettedScene {
	/* Items */
	class SouthEastExit: public NamedHotspot {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};
	class NoteBoard: public NamedHotspot {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};

	/* Objects */
	class Lyle: public NamedObject {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};
	class Door: public NamedObject {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};
	class Object4: public NamedObject {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};
	class Object5: public NamedObject {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};

public:
	SequenceManager _sequenceManager;
	SpeakerGameText _gameTextSpeaker;
	SpeakerJakeJacket _jakeJacketSpeaker;
	SpeakerLyleHat _lyleHatSpeaker;
	NamedObject _object1, _lyle, _object6;
	Door _door;
	Object4 _object4;
	Object5 _object5;
	SouthEastExit _seExit;
	NoteBoard _noteBoard;
	NamedHotspot _item2, _item4, _item5;
	NamedHotspot _item6, _item7, _item8;
	ASoundExt _sound1;
	int _field18A4, _field18A6, _field18A8, _field18AA, _field18AC;

	Scene830();
	virtual void synchronize(Serializer &s);
	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void remove();
	virtual void signal();
	virtual void process(Event &event);
	virtual void dispatch();
};

class Scene840: public PalettedScene {
	/* Items */
	class Coins: public NamedHotspot {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};
	class Exit: public NamedHotspot {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};

	/* Objects */
	class Object2: public FocusObject {
		class RentalKeys: public NamedObject {
		public:
			virtual bool startAction(CursorType action, Event &event);
		};
		class WaveKeys: public NamedObject {
		public:
			virtual bool startAction(CursorType action, Event &event);
		};
	public:
		RentalKeys _rentalKeys;
		WaveKeys _waveKeys;
		int _v1B4, _v1B6;

		virtual void postInit(SceneObjectList *OwnerList = NULL);
		virtual void remove();
		virtual void process(Event &event);
		virtual bool startAction(CursorType action, Event &event);
	};
	class Object6: public NamedObject {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};
	class Carter: public NamedObject {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};

public:
	SequenceManager _sequenceManager1, _sequenceManager2;
	NamedObject _object1;
	Object2 _object2;
	NamedObject _doors;
	Object6 _object6;
	Carter _carter;
	NamedObject _lyle;
	Coins _coins;
	NamedHotspot _item2, _item3, _item4, _item5, _item6;
	NamedHotspot _item7, _item8, _item9, _item10, _item11;
	NamedHotspot _item12, _item13, _item14, _item15;
	NamedHotspot _item16, _item17, _item18;
	Exit _exit;
	SpeakerGameText _gameTextSpeaker;
	SpeakerJakeJacket _jakeJacketSpeaker;
	SpeakerJakeUniform _jakeUniformSpeaker;
	SpeakerCarter _carterSpeaker;
	int _field1ABA, _field1ABC, _field1ABE, _field1AC0;
	int _field1AC2, _field1AC4, _field1AC6;

	Scene840();
	virtual void synchronize(Serializer &s);
	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void signal();
	virtual void process(Event &event);
	virtual void dispatch();
};


} // End of namespace BlueForce

} // End of namespace TsAGE

#endif
