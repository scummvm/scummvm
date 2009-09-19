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
 * $URL$
 * $Id$
 *
 */

#ifndef ASYLUM_SCENERESOURCE_H_
#define ASYLUM_SCENERESOURCE_H_

#include "common/file.h"

#include "asylum/actor.h"
#include "asylum/barrier.h"
#include "asylum/screen.h"
#include "asylum/respack.h"
#include "asylum/polygons.h"
#include "asylum/actionlist.h"
#include "asylum/worldstats.h"

#define SCENEMASK "scn.%03d"

namespace Asylum {

class AsylumEngine;
struct ActionArea;

class SceneResource {
public:
	SceneResource();
	virtual ~SceneResource();

	bool load(uint8 sceneIdx);

	WorldStats*	getWorldStats()	  { return _worldStats; }
	Polygons*   getGamePolygons() { return _gamePolygons; }
	ActionList*	getActionList()	  { return _actionList; }
	
	int getBarrierIndexById(uint32 id);
	int getActionAreaIndexById(uint32 id);
	ActionArea* getActionAreaById(uint32 id);
	Barrier* getBarrierById(uint32 id);
    Barrier* getBarrierByIndex(uint32 idx);

private:

	WorldStats *_worldStats;
	Polygons   *_gamePolygons;
	ActionList *_actionList;

	void loadWorldStats(Common::SeekableReadStream *stream);
	void loadGamePolygons(Common::SeekableReadStream *stream);
	void loadActionList(Common::SeekableReadStream *stream);

	Common::String parseFilename(uint8 sceneIdx);

}; // end of class Scene

} // end of namespace Asylum

#endif
