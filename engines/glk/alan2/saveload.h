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

#ifndef GLK_ALAN2_SAVELOAD
#define GLK_ALAN2_SAVELOAD

#include "common/scummsys.h"
#include "glk/alan2/types.h"

namespace Glk {
namespace Alan2 {

class SaveLoad {
public:
	SaveLoad(Common::String &gameName, ActElem *actors, LocElem *locations, ObjElem *objects, EvtqElem *events, int *eventTop) : 
	_gameName(gameName), _actors(actors), _locations(locations), _objects(objects), _events(events), _eventTop(eventTop) {}
	void save();
	void restore();
private:
	bool endOfTable(AtrElem *addr);
	
	Common::String _prevSaveName;

	// Save state related variables
	Common::String _gameName;
	ActElem *_actors;
	LocElem *_locations;
	ObjElem *_objects;
	EvtqElem *_events;
	int *_eventTop;
};

} // End of namespace Alan2
} // End of namespace Glk

#endif
