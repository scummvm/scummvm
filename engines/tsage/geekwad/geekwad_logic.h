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

#ifndef TSAGE_GEEKWAD_LOGIC_H
#define TSAGE_GEEKWAD_LOGIC_H

#include "common/scummsys.h"
#include "tsage/events.h"
#include "tsage/core.h"
#include "tsage/scenes.h"
#include "tsage/globals.h"

namespace TsAGE {

namespace Geekwad {

using namespace TsAGE;

class SceneFactory {
public:
	static Scene *createScene(int sceneNumber);
};

class SceneExt: public Scene {
public:
public:
	SceneExt();

	virtual Common::String getClassName() { return "SceneExt"; }
};

class GeekwadGame: public Game {
public:
	virtual void start();

	virtual Scene *createScene(int sceneNumber);
	virtual void processEvent(Event &event);
	virtual void rightClick();
	virtual bool canSaveGameStateCurrently();
	virtual bool canLoadGameStateCurrently();
};

class SceneObject2: public SceneObject {
public:
	int _v1, _v2;

	SceneObject2() { _v1 = _v2 = 0; }
	virtual Common::String getClassName() { return "SceneObject2"; }
	virtual void synchronize(Serializer &s) {
		SceneObject::synchronize(s);
		s.syncAsSint16LE(_v1);
		s.syncAsSint16LE(_v2);
	}
};

} // End of namespace Geekwad

} // End of namespace TsAGE

#endif
