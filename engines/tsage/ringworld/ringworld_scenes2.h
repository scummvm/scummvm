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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef TSAGE_RINGWORLD_SCENES2_H
#define TSAGE_RINGWORLD_SCENES2_H

#include "common/scummsys.h"
#include "tsage/ringworld/ringworld_logic.h"
#include "tsage/ringworld/ringworld_speakers.h"
#include "tsage/events.h"
#include "tsage/core.h"
#include "tsage/scenes.h"
#include "tsage/globals.h"

namespace TsAGE {

namespace Ringworld {

using namespace TsAGE;

class Scene1000 : public Scene {
	/* Actions */
	class Action1 : public Action {
	public:
		void signal() override;
	};
	class Action2 : public Action {
	public:
		void signal() override;
	};
	class Action3 : public Action {
	private:
		void zoom(bool up);
	public:
		void signal() override;
	};

public:
	SceneObject _object1, _object2, _object3, _object4;
	Action1 _action1;
	Action2 _action2;
	Action3 _action3;

	void postInit(SceneObjectList *OwnerList = NULL) override;
};

class Scene1001 : public Scene {
	/* Actions */
	class Action1 : public Action {
	public:
		void signal() override;
	};
public:
	SpeakerQText _speakerQText;
	SpeakerSL _speakerSL;
	SpeakerCText _speakerCText;
	SpeakerCR _speakerCR;
	Action1 _action1;
	SceneObject _object1, _object2, _object3, _object4;
	SceneObject _object5, _object6, _object7;
	ASound _soundHandler1, _soundHandler2;

	void postInit(SceneObjectList *OwnerList = NULL) override;
};

class Scene1250 : public Scene {
public:
	/* Actions */
	class Action1 : public Action {
	public:
		void signal() override;
	};
	class Action2 : public Action {
	public:
		void signal() override;
	};
	class Action3 : public Action {
	public:
		void signal() override;
	};
	class Action4 : public Action {
	public:
		void signal() override;
	};
public:
	SpeakerQText _speakerQText;
	Action1 _action1;
	Action2 _action2;
	Action3 _action3;
	Action4 _action4;
	SceneObject _object1, _object2;

	void postInit(SceneObjectList *OwnerList = NULL) override;
};

class Scene1400 : public Scene {
public:
	/* Actions */
	class Action1 : public Action {
	public:
		void signal() override;
		void dispatch() override;
	};
public:
	Action1 _action1;

	void postInit(SceneObjectList *OwnerList = NULL) override;

};

class Scene1500 : public Scene {
public:
	/* Actions */
	class Action1 : public Action {
	public:
		void signal() override;
	};
	class Action2 : public Action {
	public:
		void signal() override;
	};
public:
	ASound _soundHandler;
	Action1 _action1;
	Action2 _action2;
	SceneObject _object1, _object2, _object3;

	void postInit(SceneObjectList *OwnerList = NULL) override;
};

} // End of namespace Ringworld

} // End of namespace TsAGE

#endif
