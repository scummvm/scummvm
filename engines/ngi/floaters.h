/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef NGI_FLOATERS_H
#define NGI_FLOATERS_H

#include "common/array.h"
#include "common/ptr.h"

namespace NGI {

class AniObject;
class Scene;
class ReactPolygonal;

struct FloaterArray1 {
	int val1;
	int targetX;

	FloaterArray1() { val1 = 0; targetX = 0; }
};

struct GutlFly {
	AniObject *ani;
	int targetX;
	int targetY;
	int x;
	int y;
	int exitX;
	int exitY;
	int val8;
	double speed;
	double maxSpeed;
	int idleTicks;
	int countdown;
	int val15;
	int flags;

	GutlFly() : ani(nullptr), targetX(0), targetY(0), x(0), y(0), exitX(0), exitY(0), val8(0),
		speed(0.0), maxSpeed(0.0), idleTicks(0), countdown(0), val15(0), flags(0) {}
};

class FlyPack {
public:
	Common::ScopedPtr<ReactPolygonal> _idleRegion;
	Common::Array<FloaterArray1> _idlePoints;
	Common::Array<GutlFly> _flies;

	void init(GameVar *var);
	void addFly(Scene *sc, int x, int y, int priority, int flags);
	void onUpdate();
	void deleteAllFlies();
};

} // End of namespace NGI

#endif /* NGI_FLOATERS_H */
