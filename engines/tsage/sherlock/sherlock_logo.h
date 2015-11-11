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

#ifdef TSAGE_SHERLOCK_ENABLED
#ifndef TSAGE_SHERLOCK_LOGO_H
#define TSAGE_SHERLOCK_LOGO_H

#include "common/scummsys.h"
#include "tsage/events.h"
#include "tsage/core.h"
#include "tsage/scenes.h"
#include "tsage/globals.h"
#include "tsage/sound.h"

namespace TsAGE {

namespace Sherlock {

using namespace TsAGE;

class Object : public SceneObject {
public:
	void setVisage(const Common::String &name);
};

class SherlockLogo: public Game {
public:
	virtual void start();
	virtual Scene *createScene(int sceneNumber);
	virtual void quitGame();
	virtual void processEvent(Event &event);
	virtual bool canSaveGameStateCurrently();
	virtual bool canLoadGameStateCurrently();
};

class SherlockSceneHandler : public SceneHandler {
public:
	virtual void postInit(SceneObjectList *OwnerList);
};

class SherlockLogoScene: public Scene {
	class Action1 : public Action {
	public:
		virtual void signal();
	};
public:
	ScenePalette _palette1, _palette2, _palette3;
	Object _object1, _object2, _object3, _object4;
	Action1 _action1;
	GfxManager _gfxManager2;

	virtual void postInit(SceneObjectList *OwnerList = NULL);
	void finish();
};

} // End of namespace Sherlock

} // End of namespace TsAGE

#endif
#endif
