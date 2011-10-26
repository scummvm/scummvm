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

namespace TsAGE {

namespace Ringworld2 {

using namespace TsAGE;

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
	class Object10: public SceneActorExt {
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
	SceneActor _object1, _object2, _object3, _object4, _object5;
	SceneActor _object6;
	Door _door;
	Table _table;
	StasisNegator _stasisNegator;
	Object10 _object10;
	SteppingDisks _steppingDisks;
	SequenceManager _sequenceManager1, _sequenceManager2;

	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void remove();
	virtual void signal();
	virtual void dispatch();
};

} // End of namespace Ringworld2

} // End of namespace TsAGE

#endif
