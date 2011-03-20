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
 * $URL: https://scummvm-misc.svn.sourceforge.net/svnroot/scummvm-misc/trunk/engines/tsage/scene_logic.h $
 * $Id: scene_logic.h 232 2011-02-12 11:56:38Z dreammaster $
 *
 */

#ifndef TSAGE_RINGWORLD_SCENES8_H
#define TSAGE_RINGWORLD_SCENES8_H

#include "common/scummsys.h"
#include "tsage/ringworld_logic.h"
#include "tsage/events.h"
#include "tsage/core.h"
#include "tsage/scenes.h"
#include "tsage/globals.h"

namespace tSage {

class Scene7000: public Scene {
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
		virtual void dispatch();
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
	class Action6: public Action {
	public:
		virtual void signal();
	};
	class Action7: public Action {
	public:
		virtual void signal();
	};

	/* Objects */
	class Object1: public SceneObject {
	public:
		virtual void doAction(int action);
	};

	/* Items */
	class SceneItem1: public SceneItem {
	public:
		virtual void doAction(int action);
	};

public:
	SoundHandler _soundHandler;
	SequenceManager _sequenceManager;
	SpeakerSKText _speakerSKText;
	SpeakerSKL _speakerSKL;
	SpeakerQL _speakerQL;
	SpeakerQR _speakerQR;
	SpeakerQText _speakerQText;
	SceneObject _object1;
	SceneObject _object2;
	SceneObject _object3;
	SceneObject _object4;
	SceneObject _object5;
	SceneObject _object6;
	SceneObject _object7;
	SceneObject _object8;
	SceneObject _object9;
	SceneObject _object10;
	Action1 _action1;
	Action2 _action2;
	Action3 _action3;
	Action4 _action4;
	Action5 _action5;
	Action6 _action6;
	Action7 _action7;
	SceneItem1  _sceneItem1;

	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void signal();
	virtual void dispatch();
};

class Scene7200: public Scene {
	/* Actions */
	class Action1: public Action {
	public:
		virtual void signal();
	};
	class Action2: public Action {
	public:
		virtual void signal();
	};

public:
	Action1 _action1;
	Action2 _action2;
	SceneObject _swimmer;
	SceneObject _object2;
	SceneObject _object3;
	SceneObject _object4;
	SceneObject _object5;
	SceneObject _object6;
	SceneObject _object7;
	SceneObject _object8;
	SceneObject _object9;
	SoundHandler _soundHandler;

	virtual void postInit(SceneObjectList *OwnerList = NULL);
};

class Scene7300: public Scene {
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

public:
	SpeakerPOR _speakerPOR;
	SpeakerPOText _speakerPOText;
	SpeakerSKText _speakerSKText;
	SpeakerQU _speakerQU;
	SceneObject _object1;
	SceneObject _object2;
	SceneObject _object3;
	SceneObject _object4;
	SceneObject _object5;
	SceneObject _object6;
	SceneObject _object7;
	SceneObject _object8;
	Action1 _action1;
	Action2 _action2;
	Action3 _action3;
	Action4 _action4;

	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void dispatch();
};

class Scene7600: public Scene {
	/* Actions */
	class Action1: public Action {
	public:
		virtual void signal();
	};
	class Action2: public Action {
	public:
		virtual void signal();
	};

public:
	Action1 _action1;
	Action2 _action2;
	SceneObject _object1;
	SceneObject _object2;
	SceneObject _object3;
	SceneObject _object4;
	SceneObject _object5;
	SceneObject _object6;
	SoundHandler _soundHandler1;
	SoundHandler _soundHandler2;

	virtual void postInit(SceneObjectList *OwnerList = NULL);
};

} // End of namespace tSage

#endif
