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

#ifndef PINK_WALK_MGR_H
#define PINK_WALK_MGR_H

#include "common/rect.h"

#include "pink/objects/object.h"
#include "pink/objects/walk/walk_shortest_path.h"
#include "pink/utils.h"

namespace Pink {

class WalkLocation;
class LeadActor;
class WalkAction;

struct Coordinates {
	Common::Point point;
	int z;
};

class WalkMgr : public Object {
public:
	WalkMgr();
	~WalkMgr() override;
	void deserialize(Archive &archive) override;
	void toConsole() const override;

	WalkLocation *findLocation(const Common::String &name);
	void start(WalkLocation *destination);
	void update();

	double getLengthBetweenLocations(WalkLocation *first, WalkLocation *second);
	void setCurrentWayPoint(WalkLocation *location);

	void loadState(Archive &archive);
	void saveState(Archive &archive);

	void skip();

	const Coordinates &getStartCoords() { return _current.coords; }
	const Coordinates &getEndCoords() { return _next.coords; }

private:
	struct WayPoint {
		Common::String name;
		Coordinates coords;
	};

	Coordinates getLocationCoordinates(const Common::String &locationName);
	void end();
	void initNextWayPoint(WalkLocation *location);
	WalkAction *getWalkAction();

	LeadActor *_leadActor;
	WalkLocation *_destination;
	Array<WalkLocation *> _locations;
	WayPoint _current;
	WayPoint _next;
	bool _isWalking;
};

} // End of namespace Pink

#endif
