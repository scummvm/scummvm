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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#ifndef ASYLUM_SCENE_H_
#define ASYLUM_SCENE_H_

#include "common/file.h"

#define SCENEMASK   "scn.%03d"

namespace Asylum {

class AsylumEngine;
class WorldStats;
class GamePolies;
class ActionList;

class Scene {
public:
	Scene(AsylumEngine *vm);
	virtual ~Scene();

    bool load(uint8 sceneIdx);
private:
    AsylumEngine *_vm;

    WorldStats *_worldStats;
    GamePolies *_gamePolies;
    ActionList *_actionList;

    Common::String parseFilename(uint8 sceneIdx);
}; // end of class Scene


class WorldStats {
public:
    WorldStats() {};
    virtual ~WorldStats() {};

    uint32 _numChapter;
    uint32 _commonGrResIdArray[25];
    uint32 _width;
    uint32 _height;
    uint32 _numActions;
    uint32 _numActors;
}; // end of class WorldStats


class GamePolies {
public:
    GamePolies() {};
    virtual ~GamePolies() {};
}; // end of class GamePolies


class ActionList {
public:
    ActionList() {};
    virtual ~ActionList() {};
}; // end of class ActionList

} // end of namespace Asylum

#endif
