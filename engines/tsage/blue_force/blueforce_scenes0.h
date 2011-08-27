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

#ifndef TSAGE_BLUEFORCE_SCENES0_H
#define TSAGE_BLUEFORCE_SCENES0_H

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

class Scene20 : public SceneExt {
	/* Actions */
	class Action1 : public Action {
	private:
		ASoundExt _sound;
	public:
		virtual void signal();
	};
public:
	Action1 _action1;
	ScenePalette _scenePalette;
	SceneObject _object1, _object2, _object3, _object4;
	SceneObject _object5, _object6, _object7, _object8;

	virtual void postInit(SceneObjectList *OwnerList = NULL);
};

class Scene50: public SceneExt {
	class Tooltip: public SavedObject {
	public:
		Rect _bounds;
		Common::String _msg;
		int _newSceneNumber;
		int _locationId;
	public:
		Tooltip();
		void set(const Rect &bounds, int v60, const Common::String &msg, int v62);
		void update();
		void highlight(bool btnDown);

		virtual Common::String getClassName() { return "Scene50_Tooltip"; }
		virtual void synchronize(Serializer &s);
	};
	class Tooltip2: public Action {
	public:
		Tooltip2(): Action() {}

		virtual Common::String getClassName() { return "Scene50_Tooltip2"; }
		virtual void signal();
		virtual void dispatch();
	};
public:
	int _field380, _field382;
	int _sceneNumber;
	SceneText _text;
	SceneItemType2 _item;
	Tooltip _location1, _location2, _location3, _location4, _location5;
	Tooltip _location6, _location7, _location8, _location9;
	Timer _timer;
public:
	Scene50();
	virtual Common::String getClassName() { return "Scene50"; }
	virtual void postInit(SceneObjectList *OwnerList = NULL);	
	virtual void remove();
	virtual void signal();
	virtual void process(Event &event);
};

} // End of namespace BlueForce

} // End of namespace TsAGE

#endif
