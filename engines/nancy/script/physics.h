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

#ifndef NANCY_SCRIPT_PHYSICS_H
#define NANCY_SCRIPT_PHYSICS_H

#include "common/hashmap.h"

#include "engines/nancy/box2d/Dynamics/b2WorldCallbacks.h"

#include "engines/nancy/script/record.h"

class b2Body;
class b2Joint;
class b2MouseJoint;
class b2Shape;
class b2World;

namespace Nancy {
namespace Script {

class PhysicsRecord;

// Hands the shapes that begin to touch to the record, while the world steps
class PhysicsContactListener : public b2ContactListener {
public:
	PhysicsContactListener(PhysicsRecord *record) : _record(record) {}
	void Add(const b2ContactPoint *point) override;

private:
	PhysicsRecord *_record;
};

// Tells the record about the joints and shapes that go with a destroyed body
class PhysicsDestructionListener : public b2DestructionListener {
public:
	PhysicsDestructionListener(PhysicsRecord *record) : _record(record) {}
	void SayGoodbye(b2Joint *joint) override;
	void SayGoodbye(b2Shape *shape) override;

private:
	PhysicsRecord *_record;
};

// Nancy21's air hockey table. A Physics record is a world of bodies that collide
// and move by themselves, which the scripts read the places of:
//
//   world = AR:Physics({ bounds = ..., OnContact = function(this, a, b, point) ... end })
//   puck = world:CreateBody({ position = Vector:New(0, 0), fastMoving = true })
//   puck:CreateShape({ radius = 0.1, density = 1, restitution = 1, computeMass = true })
//
// OnContact is called with the two shapes whenever they begin to touch.
//
// The world is Box2D 2.0.1, which the original has built in, too. Each frame
// takes one step of the world, as long as the frame took.
class PhysicsRecord : public Record {
public:
	PhysicsRecord(SceneContext &context);
	~PhysicsRecord() override;

	b2World *getWorld() const { return _world; }
	float getTimeStep() const { return _timeStep; }

	// The objects of the scripts hold ids, which these keep track of
	void addBody(b2Body *body);
	void addShape(b2Shape *shape);
	void addJoint(b2Joint *joint);
	void forgetBody(b2Body *body);
	void forgetShape(b2Shape *shape);
	void forgetJoint(b2Joint *joint);

	// Null when it was destroyed
	b2Body *findBody(uint id) const;
	b2MouseJoint *findJoint(uint id) const;

	// Pushes the objects that stand for these
	void pushBody(b2Body *body);
	void pushShape(b2Shape *shape);
	void pushJoint(b2Joint *joint);

	// The record a physics object belongs to
	static PhysicsRecord *checkOwner(lua_State *L, int index, const char *typeName, uint &id);

	void onContact(b2Shape *a, b2Shape *b, double x, double y);

protected:
	void readTable(int tableIndex) override;
	void run(double sceneTime, double elapsedTime) override;
	bool getField(const Common::String &key) override;

private:
	b2World *_world = nullptr;
	PhysicsContactListener _contactListener;
	PhysicsDestructionListener _destructionListener;

	float _timeStep = 1.0f / 60.0f;		// what the mouse joints soften their pull by
	int _numIterations = 10;

	uint _nextID = 1;
	Common::HashMap<uint, b2Body *> _bodies;
	Common::HashMap<uint, b2Shape *> _shapes;
	Common::HashMap<uint, b2Joint *> _joints;
};

void registerPhysicsAPI(lua_State *L);

} // End of namespace Script
} // End of namespace Nancy

#endif // NANCY_SCRIPT_PHYSICS_H
