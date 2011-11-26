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

namespace TsAGE {

namespace Ringworld2 {

using namespace TsAGE;

class Scene2000: public SceneExt {
	class Action1 : public ActionExt {
	public:
		virtual void signal();
	};

	class Exit1 : public SceneExit {
		virtual void changeScene();
	};
	class Exit2 : public SceneExit {
		virtual void changeScene();
	};
	class Exit3 : public SceneExit {
		virtual void changeScene();
	};
	class Exit4 : public SceneExit {
		virtual void changeScene();
	};
	class Exit5 : public SceneExit {
		virtual void changeScene();
	};
public:
	bool _exitingFlag;
	int _mazePlayerMode;

	NamedHotspot _item1;
	SceneActor _object1;
	SceneActor _objList1[11];
	Exit1 _exit1;
	Exit2 _exit2;
	Exit3 _exit3;
	Exit4 _exit4;
	Exit5 _exit5;
	Action1 _action1, _action2, _action3, _action4, _action5;
	SequenceManager _sequenceManager;

	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void remove();
	virtual void signal();
	virtual void process(Event &event);
	virtual void synchronize(Serializer &s);

	void initExits();
	void initPlayer();
};


} // End of namespace Ringworld2
} // End of namespace TsAGE

#endif
