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

#ifndef TSAGE_BLUEFORCE_SCENES2_H
#define TSAGE_BLUEFORCE_SCENES2_H

#include "common/scummsys.h"
#include "tsage/blue_force/blueforce_logic.h"
#include "tsage/events.h"
#include "tsage/core.h"
#include "tsage/scenes.h"
#include "tsage/globals.h"
#include "tsage/sound.h"

namespace TsAGE {

namespace BlueForce {

using namespace TsAGE;

class IntroObject: public NamedObject {
};

class Scene200: public SceneExt {
	/* Actions */
	class Action1: public ActionExt {
	public:
		virtual void signal();
	};
	class Action2: public Action {
	public:
		virtual void signal();
	};
public:
	SequenceManager _sequenceManager;
	Action1 _action1;
	Action2 _action2;
	NamedObject _object1, _object2, _object3, _object4, _object5, _object6;
	IntroObject  _object7, _object8, _object9;
	NamedObject _object10, _object11;

	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void remove();
};

class Scene210: public SceneExt {
	/* Actions */
	class Action1: public ActionExt {
	public:
		virtual void signal();
	};
	class Action2: public Action {
	public:
		virtual void signal();
	};
public:
	SequenceManager _sequenceManager;
	Action1 _action1;
	IntroObject _object1, _object2, _object3, _object4;
	IntroObject _object5, _object6, _object7, _object8;
	NamedObject _object9, _object10, _object11, _object12;
	NamedObject _object13, _object14, _object15;

	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void remove();
};

class Scene220: public SceneExt {
	/* Actions */
	class Action1: public ActionExt {
	public:
		virtual void signal();
	};
	class Action2: public Action {
	public:
		virtual void signal();
	};
public:
	SequenceManager _sequenceManager;
	Action1 _action1;
	Action2 _action2;
	NamedObject _object1, _object2, _object3, _object4, _object5;
	NamedObject _object6, _object7, _object8, _object9;
	IntroObject _object10, _object11, _object12, _object13;
	IntroObject _object14, _object15, _object16;

	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void remove();
};


} // End of namespace BlueForce

} // End of namespace TsAGE

#endif
