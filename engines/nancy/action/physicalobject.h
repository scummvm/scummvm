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

#ifndef NANCY_ACTION_PHYSICALOBJECT_H
#define NANCY_ACTION_PHYSICALOBJECT_H

#include "common/array.h"
#include "common/rect.h"

namespace Nancy {
namespace Action {

// Moves things about the way the Nancy17 arcade puzzles do. Time goes by in ticks
// of 20 ms, speeds are in pixels per second and angles in radians, counterclockwise
// with the y axis pointing down the screen. Each tick the speed goes down by the
// friction, a steady force (gravity, a gust) is added to the velocity, and the
// object moves no more than a pixel at a time, so it can't skip past its bounds.
class PhysicalObject {
public:
	static const uint32 kTickLength = 20;	// in milliseconds

	enum BoundsMode {
		kStopAtBounds,	// stands still where it would have left them
		kBounceOffBounds
	};

	// Something in the way of an object, inside of its bounds
	struct Obstacle {
		enum Type {
			kPin,	// a point the object's middle keeps the object's width away from
			kBlock	// a rect the object can't overlap
		};

		Type type = kPin;
		Common::Rect rect;
	};

	void setSize(int width, int height) { _width = width; _height = height; }
	// An empty rect means there are none
	void setBounds(const Common::Rect &bounds) { _bounds = bounds; }
	void setBoundsMode(BoundsMode mode) { _boundsMode = mode; }
	// Shared by the objects of a puzzle, which keeps the list
	void setObstacles(const Common::Array<Obstacle> *obstacles) { _obstacles = obstacles; }

	// Puts the object there, unless that is outside of its bounds
	void moveTo(double x, double y);

	double getX() const { return _x; }
	double getY() const { return _y; }
	// The rect the object takes up, centered on its position
	Common::Rect getRect() const;

	void setSpeed(double speed);
	double getSpeed() const { return _speed; }
	void setMaxSpeed(double maxSpeed) { _hasMaxSpeed = true; _maxSpeed = maxSpeed; setSpeed(_speed); }
	void setHeading(double heading);
	double getHeading() const { return _heading; }
	void setFriction(double friction) { _friction = friction; }
	void setForce(double x, double y) { _forceX = x; _forceY = y; }

	// Whether the object ran into its bounds since this was last cleared
	bool hasHitBounds() const { return _hasHitBounds; }
	void clearHitBounds() { _hasHitBounds = false; }

	void update(uint32 numTicks);

	// The direction of a vector, between 0 and 2 pi
	static double getAngle(double x, double y);

private:
	Common::Rect getRectAt(double x, double y) const;
	bool isInsideBounds(const Common::Rect &rect) const;
	// The way out of the bounds, -1 while the rect is inside of them
	double getBoundsNormal(const Common::Rect &rect) const;
	bool isClearOfObstacle(const Obstacle &obstacle, const Common::Rect &rect) const;
	bool isClearOfObstacles(const Common::Rect &rect) const;
	// The way off the first obstacle the rect ran into, -1 when there is none
	double getObstacleNormal(const Common::Rect &rect) const;

	void accelerate(double numTicks);
	void applyFriction(double numTicks);
	void move(double numTicks);
	void stopAtBounds(double x, double y);
	void bounceOffBounds(double x, double y);

	int _width = 1;
	int _height = 1;
	Common::Rect _bounds;
	const Common::Array<Obstacle> *_obstacles = nullptr;
	BoundsMode _boundsMode = kStopAtBounds;

	double _x = 0;
	double _y = 0;
	double _speed = 0;
	double _heading = 0;
	double _friction = 0;
	double _forceX = 0;
	double _forceY = 0;

	bool _hasMaxSpeed = false;
	double _maxSpeed = 0;
	bool _hasHitBounds = false;
};

} // End of namespace Action
} // End of namespace Nancy

#endif // NANCY_ACTION_PHYSICALOBJECT_H
