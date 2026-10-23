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

#include "engines/nancy/box2d/Collision/Shapes/b2CircleShape.h"
#include "engines/nancy/box2d/Collision/Shapes/b2PolygonShape.h"
#include "engines/nancy/box2d/Dynamics/b2Body.h"
#include "engines/nancy/box2d/Dynamics/b2World.h"
#include "engines/nancy/box2d/Dynamics/Contacts/b2Contact.h"
#include "engines/nancy/box2d/Dynamics/Joints/b2MouseJoint.h"

#include "engines/nancy/script/luabind.h"
#include "engines/nancy/script/mathtypes.h"
#include "engines/nancy/script/physics.h"
#include "engines/nancy/script/scenecontext.h"

namespace Nancy {
namespace Script {

static const char *const kBodyType = "ND.PhysicsBody";
static const char *const kShapeType = "ND.PhysicsShape";
static const char *const kJointType = "ND.PhysicsJoint";

// What the objects of the scripts hold
struct PhysicsHandle {
	PhysicsRecord *record;
	uint id;
};

// The world objects keep their ids as their user data
static uint getID(void *userData) {
	return (uint)(uintptr)userData;
}

static void *toUserData(uint id) {
	return (void *)(uintptr)id;
}

void PhysicsContactListener::Add(const b2ContactPoint *point) {
	_record->onContact(point->shape1, point->shape2, point->position.x, point->position.y);
}

void PhysicsDestructionListener::SayGoodbye(b2Joint *joint) {
	_record->forgetJoint(joint);
}

void PhysicsDestructionListener::SayGoodbye(b2Shape *shape) {
	_record->forgetShape(shape);
}

PhysicsRecord::PhysicsRecord(SceneContext &context) :
		Record(context, "ND.PhysicsAR"),
		_contactListener(this),
		_destructionListener(this) {}

PhysicsRecord::~PhysicsRecord() {
	if (_world) {
		// Destroying the world destroys the joints that hang off its ground
		_world->SetDestructionListener(nullptr);
		delete _world;
	}
}

void PhysicsRecord::addBody(b2Body *body) {
	body->SetUserData(toUserData(_nextID));
	_bodies[_nextID++] = body;
}

void PhysicsRecord::addShape(b2Shape *shape) {
	shape->SetUserData(toUserData(_nextID));
	_shapes[_nextID++] = shape;
}

void PhysicsRecord::addJoint(b2Joint *joint) {
	joint->SetUserData(toUserData(_nextID));
	_joints[_nextID++] = joint;
}

void PhysicsRecord::forgetBody(b2Body *body) {
	_bodies.erase(getID(body->GetUserData()));
}

void PhysicsRecord::forgetShape(b2Shape *shape) {
	_shapes.erase(getID(shape->GetUserData()));
}

void PhysicsRecord::forgetJoint(b2Joint *joint) {
	_joints.erase(getID(joint->GetUserData()));
}

b2Body *PhysicsRecord::findBody(uint id) const {
	return _bodies.getValOrDefault(id, nullptr);
}

b2MouseJoint *PhysicsRecord::findJoint(uint id) const {
	return (b2MouseJoint *)_joints.getValOrDefault(id, nullptr);
}

static void pushHandle(lua_State *L, const char *typeName, PhysicsRecord *record, uint id) {
	PhysicsHandle handle;
	handle.record = record;
	handle.id = id;
	pushValue(L, typeName, handle);
}

void PhysicsRecord::pushBody(b2Body *body) {
	pushHandle(getLua(), kBodyType, this, getID(body->GetUserData()));
}

void PhysicsRecord::pushShape(b2Shape *shape) {
	pushHandle(getLua(), kShapeType, this, getID(shape->GetUserData()));
}

void PhysicsRecord::pushJoint(b2Joint *joint) {
	pushHandle(getLua(), kJointType, this, getID(joint->GetUserData()));
}

PhysicsRecord *PhysicsRecord::checkOwner(lua_State *L, int index, const char *typeName, uint &id) {
	const PhysicsHandle &handle = *(const PhysicsHandle *)checkUserData(L, index, typeName);
	id = handle.id;
	return handle.record;
}

static b2Vec2 checkPhysicsVector(lua_State *L, int index) {
	const ScriptVector *vector = testVector(L, index);
	if (!vector) {
		luaL_typerror(L, index, "Vector");
	}

	return b2Vec2((float32)vector->v[0], (float32)vector->v[1]);
}

static bool readPhysicsVector(lua_State *L, int tableIndex, const char *name, b2Vec2 &value) {
	if (!pushField(L, tableIndex, name)) {
		return false;
	}

	value = checkPhysicsVector(L, -1);
	lua_pop(L, 1);
	return true;
}

static void pushPhysicsVector(lua_State *L, double x, double y) {
	ScriptVector vector;
	vector.size = 2;
	vector.v[0] = x;
	vector.v[1] = y;
	pushVector(L, vector);
}

static bool readFloat(lua_State *L, int tableIndex, const char *name, float32 &value) {
	double number;
	if (!readField(L, tableIndex, name, number)) {
		return false;
	}

	value = (float32)number;
	return true;
}

static bool readTruth(lua_State *L, int tableIndex, const char *name, bool &value) {
	lua_getfield(L, tableIndex, name);
	const bool isSet = !lua_isnil(L, -1);
	if (isSet) {
		value = lua_toboolean(L, -1) != 0;
	}

	lua_pop(L, 1);
	return isSet;
}

// The world only simulates what is inside its bounds, a Vector of the left,
// top, right and bottom edges
void PhysicsRecord::readTable(int tableIndex) {
	lua_State *L = getLua();

	if (!pushField(L, tableIndex, "bounds")) {
		luaL_error(L, "PhysicsAR must be created with an 'bounds' parameter");
	}

	const ScriptVector *bounds = testVector(L, -1);
	if (!bounds) {
		luaL_typerror(L, -1, "Vector");
	}

	b2AABB worldBounds;
	worldBounds.lowerBound.Set((float32)bounds->v[0], (float32)bounds->v[3]);
	worldBounds.upperBound.Set((float32)bounds->v[2], (float32)bounds->v[1]);
	lua_pop(L, 1);

	b2Vec2 gravity(0.0f, 0.0f);
	readPhysicsVector(L, tableIndex, "gravity", gravity);

	// Bodies that stopped moving get left alone until something hits them
	bool allowsSleep = true;
	readTruth(L, tableIndex, "inactiveSleep", allowsSleep);

	_world = new b2World(worldBounds, gravity, allowsSleep);
	_world->SetDestructionListener(&_destructionListener);
	_world->SetContactListener(&_contactListener);

	readFloat(L, tableIndex, "timeStep", _timeStep);
	readField(L, tableIndex, "iterationCount", _numIterations);
}

void PhysicsRecord::run(double sceneTime, double elapsedTime) {
	_world->Step((float32)elapsedTime, _numIterations);
}

// OnContact(this, shape, shape, point) gets called for every new point at which
// two shapes touch, while the world steps
// TODO: OnContactResolved, which the original calls after the contacts are
// solved, and which no script sets
void PhysicsRecord::onContact(b2Shape *a, b2Shape *b, double x, double y) {
	pushShape(a);
	pushShape(b);
	pushPhysicsVector(getLua(), x, y);
	callFunction("OnContact", 3);
}

// world:CreateBody({ position, angle, linearDampening, angularDampening, fixedAngle,
// fastMoving, asleep, inactiveSleep })
// TODO: "massData", which no script sets
static int physicsCreateBody(lua_State *L) {
	PhysicsRecord *record = (PhysicsRecord *)checkRecord(L, 1);
	luaL_checktype(L, 2, LUA_TTABLE);

	b2BodyDef def;
	readPhysicsVector(L, 2, "position", def.position);
	readFloat(L, 2, "angle", def.angle);
	readFloat(L, 2, "linearDampening", def.linearDamping);
	readFloat(L, 2, "angularDampening", def.angularDamping);
	readTruth(L, 2, "asleep", def.isSleeping);
	readTruth(L, 2, "fixedAngle", def.fixedRotation);
	readTruth(L, 2, "inactiveSleep", def.allowSleep);
	readTruth(L, 2, "fastMoving", def.isBullet);

	b2Body *body = record->getWorld()->CreateBody(&def);
	record->addBody(body);
	record->pushBody(body);
	return 1;
}

static int physicsDestroyBody(lua_State *L) {
	PhysicsRecord *record = (PhysicsRecord *)checkRecord(L, 1);
	uint id;
	if (PhysicsRecord::checkOwner(L, 2, kBodyType, id) == record) {
		b2Body *body = record->findBody(id);
		if (body) {
			record->forgetBody(body);
			record->getWorld()->DestroyBody(body);
		}
	}

	return 0;
}

// The bodies a joint connects, of which a missing one is the ground
static b2Body *readJointBody(lua_State *L, PhysicsRecord *record, int bodiesIndex, int index) {
	lua_rawgeti(L, bodiesIndex, index);
	b2Body *body = nullptr;
	if (!lua_isnil(L, -1)) {
		uint id;
		if (PhysicsRecord::checkOwner(L, -1, kBodyType, id) == record) {
			body = record->findBody(id);
		}
	}

	lua_pop(L, 1);
	return body ? body : record->getWorld()->GetGroundBody();
}

// world:CreateJoint({ bodies = { ground, body }, target, frequency, maxForce, dampening,
// collide }), which makes a mouse joint
static int physicsCreateJoint(lua_State *L) {
	PhysicsRecord *record = (PhysicsRecord *)checkRecord(L, 1);
	luaL_checktype(L, 2, LUA_TTABLE);

	b2MouseJointDef def;
	if (!readPhysicsVector(L, 2, "target", def.target)) {
		return luaL_error(L, "PhysicsAR:CreateJoint{} requires one of: target");
	}

	readFloat(L, 2, "frequency", def.frequencyHz);
	readFloat(L, 2, "maxForce", def.maxForce);
	readFloat(L, 2, "dampening", def.dampingRatio);
	readTruth(L, 2, "collide", def.collideConnected);
	def.timeStep = record->getTimeStep();

	if (!pushField(L, 2, "bodies")) {
		return luaL_error(L, "PhysicsAR:CreateJoint{} requires bodies field.");
	}

	const int bodiesIndex = absoluteIndex(L, -1);
	def.body1 = readJointBody(L, record, bodiesIndex, 1);
	def.body2 = readJointBody(L, record, bodiesIndex, 2);
	lua_pop(L, 1);

	b2Joint *joint = record->getWorld()->CreateJoint(&def);
	record->addJoint(joint);
	record->pushJoint(joint);
	return 1;
}

static int physicsDestroyJoint(lua_State *L) {
	PhysicsRecord *record = (PhysicsRecord *)checkRecord(L, 1);
	uint id;
	if (PhysicsRecord::checkOwner(L, 2, kJointType, id) == record) {
		b2Joint *joint = record->findJoint(id);
		if (joint) {
			record->forgetJoint(joint);
			record->getWorld()->DestroyJoint(joint);
		}
	}

	return 0;
}

bool PhysicsRecord::getField(const Common::String &key) {
	static const luaL_Reg kWorldFunctions[] = {
		{ "CreateBody", physicsCreateBody },
		{ "DestroyBody", physicsDestroyBody },
		{ "CreateJoint", physicsCreateJoint },
		{ "DestroyJoint", physicsDestroyJoint },
		{ nullptr, nullptr }
	};

	for (const luaL_Reg *function = kWorldFunctions; function->name; ++function) {
		if (key == function->name) {
			lua_pushcfunction(getLua(), function->func);
			return true;
		}
	}

	if (key == "timeStep") {
		lua_pushnumber(getLua(), _timeStep);
		return true;
	} else if (key == "iterationCount") {
		lua_pushinteger(getLua(), _numIterations);
		return true;
	}

	return false;
}

static b2Body *checkBody(lua_State *L, int index, PhysicsRecord *&record) {
	uint id;
	record = PhysicsRecord::checkOwner(L, index, kBodyType, id);
	b2Body *body = record->findBody(id);
	if (!body) {
		luaL_error(L, "The PhysicsBody was destroyed");
	}

	return body;
}

// What every kind of shape has: filter = { category, mask, group }, friction,
// restitution, density and sensor
static void readShapeDef(lua_State *L, int tableIndex, b2ShapeDef &def) {
	if (pushField(L, tableIndex, "filter")) {
		const int filterIndex = absoluteIndex(L, -1);
		int value;
		if (readField(L, filterIndex, "category", value)) {
			def.filter.categoryBits = (uint16)value;
		}

		if (readField(L, filterIndex, "mask", value)) {
			def.filter.maskBits = (uint16)value;
		}

		if (readField(L, filterIndex, "group", value)) {
			def.filter.groupIndex = (int16)value;
		}

		lua_pop(L, 1);
	}

	readFloat(L, tableIndex, "friction", def.friction);
	readFloat(L, tableIndex, "restitution", def.restitution);
	readFloat(L, tableIndex, "density", def.density);
	readTruth(L, tableIndex, "sensor", def.isSensor);
}

// body:CreateShape({ radius, center } or { verticies }, with the fields of
// readShapeDef() and computeMass). The corners of a polygon go around it
// counterclockwise, eight at the most.
// TODO: "box", which no script uses, and body:DestroyShape(), which no script calls
static int bodyCreateShape(lua_State *L) {
	PhysicsRecord *record;
	b2Body *body = checkBody(L, 1, record);
	luaL_checktype(L, 2, LUA_TTABLE);

	b2CircleDef circle;
	b2PolygonDef polygon;
	b2ShapeDef *def;

	if (readFloat(L, 2, "radius", circle.radius)) {
		readPhysicsVector(L, 2, "center", circle.localPosition);
		def = &circle;
	} else if (pushField(L, 2, "verticies")) {
		for (int i = 1; polygon.vertexCount < b2_maxPolygonVertices; ++i) {
			lua_rawgeti(L, -1, i);
			if (lua_isnil(L, -1)) {
				lua_pop(L, 1);
				break;
			}

			polygon.vertices[polygon.vertexCount++] = checkPhysicsVector(L, -1);
			lua_pop(L, 1);
		}

		lua_pop(L, 1);
		def = &polygon;
	} else {
		return luaL_error(L, "PhysicsBody:CreateShape{} requires one of: radius, verticies, OR box");
	}

	readShapeDef(L, 2, *def);

	b2Shape *shape = body->CreateShape(def);
	record->addShape(shape);

	bool computesMass = true;
	readTruth(L, 2, "computeMass", computesMass);
	if (computesMass) {
		body->SetMassFromShapes();
	}

	record->pushShape(shape);
	return 1;
}

// body:ApplyForce(force, point), for the next step
static int bodyApplyForce(lua_State *L) {
	PhysicsRecord *record;
	b2Body *body = checkBody(L, 1, record);
	body->ApplyForce(checkPhysicsVector(L, 2), checkPhysicsVector(L, 3));
	return 0;
}

static int bodyIndex(lua_State *L) {
	PhysicsRecord *record;
	b2Body *body = checkBody(L, 1, record);
	const Common::String key = luaL_checkstring(L, 2);

	if (key == "position") {
		const b2Vec2 &position = body->GetPosition();
		pushPhysicsVector(L, position.x, position.y);
		return 1;
	} else if (key == "velocity") {
		const b2Vec2 velocity = body->GetLinearVelocity();
		pushPhysicsVector(L, velocity.x, velocity.y);
		return 1;
	}

	return indexFunctions(L);
}

static int bodyNewIndex(lua_State *L) {
	PhysicsRecord *record;
	b2Body *body = checkBody(L, 1, record);
	const Common::String key = luaL_checkstring(L, 2);

	if (key != "velocity") {
		return luaL_error(L, "PhysicsBody does not have assignable field: %s.", key.c_str());
	}

	body->SetLinearVelocity(checkPhysicsVector(L, 3));
	return 0;
}

// Objects of the same body, shape or joint are equal
static int handleEquals(lua_State *L) {
	const PhysicsHandle *a = (const PhysicsHandle *)lua_touserdata(L, 1);
	const PhysicsHandle *b = (const PhysicsHandle *)lua_touserdata(L, 2);
	lua_pushboolean(L, a && b && a->record == b->record && a->id == b->id);
	return 1;
}

static int shapeNewIndex(lua_State *L) {
	return luaL_error(L, "PhysicsShape does not have assignable fields");
}

// joint:SetTarget(point)
static int jointSetTarget(lua_State *L) {
	uint id;
	PhysicsRecord *record = PhysicsRecord::checkOwner(L, 1, kJointType, id);
	b2MouseJoint *joint = record->findJoint(id);
	if (joint) {
		joint->SetTarget(checkPhysicsVector(L, 2));
	}

	return 0;
}

static int jointNewIndex(lua_State *L) {
	return luaL_error(L, "PhysicsJoint does not have assignable fields");
}

void registerPhysicsAPI(lua_State *L) {
	static const luaL_Reg kBodyFunctions[] = {
		{ "__index", bodyIndex },
		{ "__newindex", bodyNewIndex },
		{ "__eq", handleEquals },
		{ "CreateShape", bodyCreateShape },
		{ "ApplyForce", bodyApplyForce },
		{ nullptr, nullptr }
	};

	static const luaL_Reg kShapeFunctions[] = {
		{ "__index", indexFunctions },
		{ "__newindex", shapeNewIndex },
		{ "__eq", handleEquals },
		{ nullptr, nullptr }
	};

	static const luaL_Reg kJointFunctions[] = {
		{ "__index", indexFunctions },
		{ "__newindex", jointNewIndex },
		{ "__eq", handleEquals },
		{ "SetTarget", jointSetTarget },
		{ nullptr, nullptr }
	};

	registerType(L, kBodyType, kBodyFunctions);
	registerType(L, kShapeType, kShapeFunctions);
	registerType(L, kJointType, kJointFunctions);
}

} // End of namespace Script
} // End of namespace Nancy
