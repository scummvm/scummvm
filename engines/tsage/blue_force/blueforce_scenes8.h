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

} // End of namespace BlueForce

} // End of namespace TsAGE

#endif
