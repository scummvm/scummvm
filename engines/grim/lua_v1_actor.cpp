/* Residual - A 3D game interpreter
 *
 * Residual is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.

 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 *
 */

#include "engines/grim/grim.h"
#include "engines/grim/lua.h"
#include "engines/grim/actor.h"
#include "engines/grim/lipsync.h"
#include "engines/grim/costume.h"
#include "engines/grim/scene.h"
#include "engines/grim/model.h"
#include "engines/grim/gfx_base.h"

#include "engines/grim/lua/lauxlib.h"

namespace Grim {

void L1_LoadActor() {
	lua_Object nameObj = lua_getparam(1);
	const char *name;

	if (lua_isnil(nameObj) || !lua_isstring(nameObj))
		name = "<unnamed>";
	else
		name = lua_getstring(nameObj);
	Actor *a = new Actor(name);
	lua_pushusertag(a->getId(), MKTAG('A','C','T','R'));
}

void L1_GetActorTimeScale() {
	lua_Object actorObj = lua_getparam(1);
	if (!lua_isuserdata(actorObj) || lua_tag(actorObj) != MKTAG('A','C','T','R'))
		return;

	Actor *actor = getactor(actorObj);
	lua_pushnumber(actor->getTimeScale());
}

void L1_SetSelectedActor() {
	lua_Object actorObj = lua_getparam(1);
	if (!lua_isuserdata(actorObj) || lua_tag(actorObj) != MKTAG('A','C','T','R'))
		return;
	// TODO there is missing some check here: else lua_pushnil()
	g_grim->setSelectedActor(getactor(actorObj));
}

/* Get the currently selected actor, this is used in
 * "Camera-Relative" mode to handle the appropriate
 * actor for movement
 */
void L1_GetCameraActor() {
	// TODO verify what is going on with selected actor
	Actor *actor = g_grim->getSelectedActor();
	lua_pushusertag(actor->getId(), MKTAG('A','C','T','R'));
}

void L1_SetActorTalkColor() {
	lua_Object actorObj = lua_getparam(1);
	lua_Object colorObj = lua_getparam(2);

	if (!lua_isuserdata(actorObj) || lua_tag(actorObj) != MKTAG('A','C','T','R'))
		return;
	if (!lua_isuserdata(colorObj) && lua_tag(colorObj) != MKTAG('C','O','L','R'))
		return;
	Actor *actor = getactor(actorObj);
	PoolColor *color = getcolor(colorObj);
	actor->setTalkColor(color);
}

void L1_GetActorTalkColor() {
	lua_Object actorObj = lua_getparam(1);
	if (!lua_isuserdata(actorObj) || lua_tag(actorObj) != MKTAG('A','C','T','R')) {
		lua_pushnil();
		return;
	}
	Actor *actor = getactor(actorObj);
	lua_pushusertag(actor->getTalkColor()->getId(), MKTAG('C','O','L','R'));
}

void L1_SetActorRestChore() {
	lua_Object actorObj = lua_getparam(1);
	lua_Object choreObj = lua_getparam(2);
	lua_Object costumeObj = lua_getparam(3);
	Costume *costume;
	int chore;

	if (!lua_isuserdata(actorObj) || lua_tag(actorObj) != MKTAG('A','C','T','R') ||
			(!lua_isnumber(choreObj) && !lua_isnil(choreObj))) {
		return;
	}

	Actor *actor = getactor(actorObj);

	if (lua_isnil(choreObj)) {
		chore = -1;
	} else {
		chore = (int)lua_getnumber(choreObj);
	}
	if (!findCostume(costumeObj, actor, &costume))
		return;

	actor->setRestChore(chore, costume);
}

void L1_SetActorWalkChore() {
	lua_Object actorObj = lua_getparam(1);
	lua_Object choreObj = lua_getparam(2);
	lua_Object costumeObj = lua_getparam(3);
	Costume *costume;
	int chore;

	if (!lua_isuserdata(actorObj) || lua_tag(actorObj) != MKTAG('A','C','T','R') ||
			(!lua_isnumber(choreObj) && !lua_isnil(choreObj))) {
		return;
	}

	Actor *actor = getactor(actorObj);

	if (lua_isnil(choreObj)) {
		chore = -1;
	} else {
		chore = (int)lua_getnumber(choreObj);
	}
	if (!findCostume(costumeObj, actor, &costume))
		return;

	actor->setWalkChore(chore, costume);
}

void L1_SetActorTurnChores() {
	lua_Object actorObj = lua_getparam(1);
	lua_Object leftChoreObj = lua_getparam(2);
	lua_Object rightChoreObj = lua_getparam(3);
	lua_Object costumeObj = lua_getparam(4);
	Costume *costume;

	if (!lua_isuserdata(actorObj) || lua_tag(actorObj) != MKTAG('A','C','T','R') ||
			(!lua_isnumber(leftChoreObj) && !lua_isnumber(rightChoreObj))) {
		return;
	}

	Actor *actor = getactor(actorObj);
	int leftChore = (int)lua_getnumber(leftChoreObj);
	int rightChore = (int)lua_getnumber(rightChoreObj);

	if (!findCostume(costumeObj, actor, &costume))
		return;

	actor->setTurnChores(leftChore, rightChore, costume);
}

void L1_SetActorTalkChore() {
	lua_Object actorObj = lua_getparam(1);
	lua_Object indexObj = lua_getparam(2);
	lua_Object choreObj = lua_getparam(3);
	lua_Object costumeObj = lua_getparam(4);
	Costume *costume;
	int chore;

	if (!lua_isuserdata(actorObj) || lua_tag(actorObj) != MKTAG('A','C','T','R') ||
			!lua_isnumber(indexObj) || (!lua_isnumber(choreObj) && !lua_isnil(choreObj))) {
		return;
	}

	int index = (int)lua_getnumber(indexObj);
	if (index < 1 || index > 16)
		return;

	Actor *actor = getactor(actorObj);

	if (lua_isnil(choreObj)) {
		chore = -1;
	} else {
		chore = (int)lua_getnumber(choreObj);
	}
	if (!findCostume(costumeObj, actor, &costume))
		return;

	if (!costume)
		return;

	actor->setTalkChore(index, chore, costume);
}

void L1_SetActorMumblechore() {
	lua_Object actorObj = lua_getparam(1);
	lua_Object choreObj = lua_getparam(2);
	lua_Object costumeObj = lua_getparam(3);
	Costume *costume;
	int chore;

	if (!lua_isuserdata(actorObj) || lua_tag(actorObj) != MKTAG('A','C','T','R') ||
			(!lua_isnumber(choreObj) && !lua_isnil(choreObj))) {
		return;
	}

	Actor *actor = getactor(actorObj);

	if (lua_isnil(choreObj)) {
		chore = -1;
	} else {
		chore = (int)lua_getnumber(choreObj);
	}
	if (!findCostume(costumeObj, actor, &costume))
		return;

	if (!costume)
		return;

	actor->setMumbleChore(chore, costume);
}

void L1_SetActorVisibility() {
	lua_Object actorObj = lua_getparam(1);
	if (!lua_isuserdata(actorObj) || lua_tag(actorObj) != MKTAG('A','C','T','R'))
		return;

	Actor *actor = getactor(actorObj);

	bool val = getbool(2);
	actor->setVisibility(val);
}

void L1_SetActorScale() {
	lua_Object actorObj = lua_getparam(1);
	lua_Object scaleObj = lua_getparam(2);

	if (!lua_isuserdata(actorObj) || lua_tag(actorObj) != MKTAG('A','C','T','R'))
		return;

	Actor *actor = getactor(actorObj);
	float scale = 1.f;

	if (lua_isnumber(scaleObj))
		scale = lua_getnumber(scaleObj);

	actor->setScale(scale);
}

void L1_SetActorTimeScale() {
	lua_Object actorObj = lua_getparam(1);
	lua_Object scaleObj = lua_getparam(2);

	if (!lua_isuserdata(actorObj) || lua_tag(actorObj) != MKTAG('A','C','T','R'))
		return;

	Actor *actor = getactor(actorObj);
	float scale = 1.f;

	if (lua_isnumber(scaleObj))
		scale = lua_getnumber(scaleObj);

	actor->setTimeScale(scale);
}

void L1_SetActorCollisionMode() {
	lua_Object actorObj = lua_getparam(1);
	lua_Object modeObj  = lua_getparam(2);

	if (!lua_isuserdata(actorObj) || lua_tag(actorObj) != MKTAG('A','C','T','R')) {
		lua_pushnil();
		return;
	}

	Actor *actor = getactor(actorObj);
	Actor::CollisionMode mode = (Actor::CollisionMode)(int)(lua_getnumber(modeObj));

	actor->setCollisionMode(mode);
}

void L1_SetActorCollisionScale() {
	lua_Object actorObj = lua_getparam(1);
	lua_Object scaleObj  = lua_getparam(2);

	if (!lua_isuserdata(actorObj) || lua_tag(actorObj) != MKTAG('A','C','T','R')) {
		lua_pushnil();
		return;
	}

	Actor *actor = getactor(actorObj);
	float scale = lua_getnumber(scaleObj);

	actor->setCollisionScale(scale);
}

void L1_PutActorAt() {
	lua_Object actorObj = lua_getparam(1);
	lua_Object xObj = lua_getparam(2);
	lua_Object yObj = lua_getparam(3);
	lua_Object zObj = lua_getparam(4);
	if (!lua_isuserdata(actorObj) || lua_tag(actorObj) != MKTAG('A','C','T','R'))
		return;

	if (!lua_isnumber(xObj) || !lua_isnumber(yObj) || !lua_isnumber(zObj))
		return;

	Actor *actor = getactor(actorObj);
	float x = lua_getnumber(xObj);
	float y = lua_getnumber(yObj);
	float z = lua_getnumber(zObj);
	actor->setPos(Graphics::Vector3d(x, y, z));
}

void L1_GetActorPos() {
	lua_Object actorObj = lua_getparam(1);

	if (!lua_isuserdata(actorObj) || lua_tag(actorObj) != MKTAG('A','C','T','R'))
		return;

	Actor *actor = getactor(actorObj);
	Graphics::Vector3d pos = actor->getPos();
	lua_pushnumber(pos.x());
	lua_pushnumber(pos.y());
	lua_pushnumber(pos.z());
}

void L1_SetActorRot() {
	lua_Object actorObj = lua_getparam(1);

	if (!lua_isuserdata(actorObj) || lua_tag(actorObj) != MKTAG('A','C','T','R'))
		return;

	Actor *actor = getactor(actorObj);
	lua_Object p = lua_getparam(2);
	lua_Object y = lua_getparam(3);
	lua_Object r = lua_getparam(4);
	if (!lua_isnumber(p) || !lua_isnumber(y) || !lua_isnumber(r))
		return;
	float pitch = lua_getnumber(p);
	float yaw = lua_getnumber(y);
	float roll = lua_getnumber(r);
	if (getbool(5))
		actor->turnTo(pitch, yaw, roll);
	else
		actor->setRot(pitch, yaw, roll);
}

void L1_GetActorRot() {
	lua_Object actorObj = lua_getparam(1);

	if (!lua_isuserdata(actorObj) || lua_tag(actorObj) != MKTAG('A','C','T','R'))
		return;

	Actor *actor = getactor(actorObj);
	lua_pushnumber(actor->getPitch());
	lua_pushnumber(actor->getYaw());
	lua_pushnumber(actor->getRoll());
}

void L1_IsActorTurning() {
	lua_Object actorObj = lua_getparam(1);

	if (!lua_isuserdata(actorObj) || lua_tag(actorObj) != MKTAG('A','C','T','R'))
		return;

	Actor *actor = getactor(actorObj);
	pushbool(actor->isTurning());
}

void L1_GetAngleBetweenActors() {
	lua_Object actor1Obj = lua_getparam(1);
	lua_Object actor2Obj = lua_getparam(2);

	if (!lua_isuserdata(actor1Obj) || lua_tag(actor1Obj) != MKTAG('A','C','T','R')) {
		lua_pushnil();
		return;
	}
	if (!lua_isuserdata(actor2Obj) || lua_tag(actor2Obj) != MKTAG('A','C','T','R')) {
		lua_pushnil();
		return;
	}
	Actor *actor1 = getactor(actor1Obj);
	Actor *actor2 = getactor(actor2Obj);

	if (!actor1 || !actor2) {
		lua_pushnil();
		return;
	}

	Graphics::Vector3d vec1 = actor1->getPuckVector();
	Graphics::Vector3d vec2 = actor2->getPos();
	vec2 -= actor1->getPos();
	vec1.z() = 0;
	vec2.z() = 0;
	vec1.normalize();
	vec2.normalize();
	float dot = vec1.dotProduct(vec2.x(), vec2.y(), 0);
	float angle = 90.0f - (180.0f * asin(dot)) / LOCAL_PI;
	if (angle < 0)
		angle = -angle;
	lua_pushnumber(angle);
}

void L1_GetActorYawToPoint() {
	lua_Object actorObj = lua_getparam(1);
	lua_Object pointObj = lua_getparam(2);
	lua_Object xObj, yObj, zObj;

	if (!lua_isuserdata(actorObj) || lua_tag(actorObj) != MKTAG('A','C','T','R')) {
		lua_pushnil();
		return;
	}

	Actor *actor = getactor(actorObj);
	if (lua_istable(pointObj)) {
		lua_pushobject(pointObj);
		lua_pushstring("x");
		xObj = lua_gettable();
		lua_pushobject(pointObj);
		lua_pushstring("y");
		yObj = lua_gettable();
		lua_pushobject(pointObj);
		lua_pushstring("z");
		zObj = lua_gettable();
	} else {
		xObj = pointObj;
		yObj = lua_getparam(3);
		zObj = lua_getparam(4);
	}
	float x = lua_getnumber(xObj);
	float y = lua_getnumber(yObj);
	float z = lua_getnumber(zObj);

	Graphics::Vector3d yawVector(x, y, z);

	lua_pushnumber(actor->getYawTo(yawVector));
}

/* Changes the set that an actor is associated with,
 * by changing the set to "nil" an actor is disabled
 * but should still not be destroyed.
 */
void L1_PutActorInSet() {
	lua_Object actorObj = lua_getparam(1);
	lua_Object setObj = lua_getparam(2);

	if (!lua_isuserdata(actorObj) || lua_tag(actorObj) != MKTAG('A','C','T','R'))
		return;

	Actor *actor = getactor(actorObj);

	if (!lua_isstring(setObj) && !lua_isnil(setObj))
		return;

	if (actor->_toClean) {
		actor->_toClean = false;

		// FIXME HACK: This hack allows manny to exit from the sets where actors are freezed
		// (and though ActorToClean is called), otherwise the set will never change and manny
		// will be trapped inside. I'm aware this is really ugly, but i could not come up
		// with a better solution, since the bug here seems to be inside the lua scripts, and
		// not in the engine. If you want to have a look, the important bits are in:
		// _system.LUA, TrackManny()
		// _actors.LUA, put_in_set(), freeze() and stamp()
		// Be aware that is not needed for the OpenGL renderer.
		lua_call("reset_doorman");
		return;
	}

	const char *set = lua_getstring(setObj);

	// FIXME verify adding actor to set
	if (!set)
		set = "";
	if (!actor->isInSet(set))
		actor->putInSet(set);
}

void L1_SetActorWalkRate() {
	lua_Object actorObj = lua_getparam(1);
	lua_Object rateObj = lua_getparam(2);

	if (!lua_isuserdata(actorObj) || lua_tag(actorObj) != MKTAG('A','C','T','R'))
		return;
	if (!lua_isnumber(rateObj))
		return;

	Actor *actor = getactor(actorObj);
	float rate = lua_getnumber(rateObj);
	actor->setWalkRate(rate);
}

void L1_GetActorWalkRate() {
	lua_Object actorObj = lua_getparam(1);
	if (!lua_isuserdata(actorObj) || lua_tag(actorObj) != MKTAG('A','C','T','R'))
		return;

	Actor *actor = getactor(actorObj);
	lua_pushnumber(actor->getWalkRate());
}

void L1_SetActorTurnRate() {
	lua_Object actorObj = lua_getparam(1);
	lua_Object rateObj = lua_getparam(2);

	if (!lua_isuserdata(actorObj) || lua_tag(actorObj) != MKTAG('A','C','T','R'))
		return;
	if (!lua_isnumber(rateObj))
		return;

	Actor *actor = getactor(actorObj);
	float rate = lua_getnumber(rateObj); // FIXME verify negate values of rate
	actor->setTurnRate(rate);
}

void L1_WalkActorForward() {
	lua_Object actorObj = lua_getparam(1);
	if (!lua_isuserdata(actorObj) || lua_tag(actorObj) != MKTAG('A','C','T','R'))
		return;
	Actor *actor = getactor(actorObj);
	actor->walkForward();
}

void L1_SetActorReflection() {
	lua_Object actorObj = lua_getparam(1);
	lua_Object angleObj = lua_getparam(2);

	if (!lua_isuserdata(actorObj) || lua_tag(actorObj) != MKTAG('A','C','T','R'))
		return;

	Actor *actor = getactor(actorObj);
	float angle = lua_getnumber(angleObj);
	actor->setReflection(angle);
}

void L1_GetActorPuckVector() {
	lua_Object actorObj = lua_getparam(1);
	lua_Object addObj = lua_getparam(2);

	if (!lua_isuserdata(actorObj) || lua_tag(actorObj) != MKTAG('A','C','T','R')) {
		lua_pushnil();
		return;
	}

	Actor *actor = getactor(actorObj);
	if (!actor) {
		lua_pushnil();
		return;
	}

	Graphics::Vector3d result = actor->getPuckVector();
	if (!lua_isnil(addObj))
		result += actor->getPos();

	lua_pushnumber(result.x());
	lua_pushnumber(result.y());
	lua_pushnumber(result.z());
}

void L1_WalkActorTo() {
	lua_Object actorObj = lua_getparam(1);
	lua_Object xObj = lua_getparam(2);
	lua_Object yObj = lua_getparam(3);
	lua_Object zObj = lua_getparam(4);

	lua_Object txObj = lua_getparam(5);
	lua_Object tyObj = lua_getparam(6);
	lua_Object tzObj = lua_getparam(7);

	if (!lua_isuserdata(actorObj) || lua_tag(actorObj) != MKTAG('A','C','T','R'))
		return;

	Graphics::Vector3d destVec;
	Actor *actor = getactor(actorObj);
	if (!lua_isnumber(xObj)) {
		if (!lua_isuserdata(xObj) || lua_tag(xObj) != MKTAG('A','C','T','R'))
			return;
		Actor *destActor = getactor(xObj);
		destVec = destActor->getPos();
	} else {
		float x = lua_getnumber(xObj);
		float y = lua_getnumber(yObj);
		float z = lua_getnumber(zObj);
		destVec.set(x, y, z);
	}

	// TODO figure out purpose this
	float tx = lua_getnumber(txObj);
	float ty = lua_getnumber(tyObj);
	float tz = lua_getnumber(tzObj);
	Graphics::Vector3d tVec(tx, ty, tz);

	actor->walkTo(destVec);
}

void L1_ActorToClean() {
	lua_Object actorObj = lua_getparam(1);

	if (!lua_isuserdata(actorObj) || lua_tag(actorObj) != MKTAG('A','C','T','R')) {
		lua_pushnil();
		return;
	}

	Actor *actor = getactor(actorObj);

	// TODO: It seems this function should load/create an image to be used in place
	// of the real actor until it is put in the set again.
	// For now this Actor::_toClean is used to leave the actor in the set.
	actor->_toClean = true;
}

void L1_IsActorMoving() {
	lua_Object actorObj = lua_getparam(1);
	if (!lua_isuserdata(actorObj) || lua_tag(actorObj) != MKTAG('A','C','T','R'))
		return;

	Actor *actor = getactor(actorObj);
	pushbool(actor->isWalking());
}

void L1_IsActorResting() {
	lua_Object actorObj = lua_getparam(1);
	if (!lua_isuserdata(actorObj) || lua_tag(actorObj) != MKTAG('A','C','T','R'))
		return;

	Actor *actor = getactor(actorObj);
	// FIXME verify if exist one flag for resting mode
	pushbool(!(actor->isWalking() || actor->isTurning()));
}

/* Get the location of one of the actor's nodes, this is
 * used by Glottis to watch where Manny is located in
 * order to hand him the work order.  This function is
 * also important for when Velasco hands Manny the logbook
 * in Rubacava. It is also used for calculating many positions
 * passed to ActorLookAt.
 */
void L1_GetActorNodeLocation() {
	lua_Object actorObj = lua_getparam(1);
	lua_Object nodeObj = lua_getparam(2);

	if (!lua_isuserdata(actorObj) || lua_tag(actorObj) != MKTAG('A','C','T','R'))
		return;

	if (!lua_isnumber(nodeObj))
		return;

	Actor *actor = getactor(actorObj);
	if (!actor->getCurrentCostume() || !actor->getCurrentCostume()->getModelNodes())
		return;

	int nodeId = (int)lua_getnumber(nodeObj);

	ModelNode *allNodes = actor->getCurrentCostume()->getModelNodes();
	ModelNode *node = allNodes + nodeId;

	ModelNode *root = node;
	while (root->_parent) {
		root = root->_parent;
	}

	Graphics::Matrix4 matrix;
	matrix._pos = actor->getPos();
	matrix._rot.buildFromPitchYawRoll(actor->getPitch(), actor->getYaw(), actor->getRoll());
	root->setMatrix(matrix);
	root->update();

	lua_pushnumber(node->_pivotMatrix._pos.x());
	lua_pushnumber(node->_pivotMatrix._pos.y());
	lua_pushnumber(node->_pivotMatrix._pos.z());
}

void L1_SetActorWalkDominate() {
	lua_Object actorObj = lua_getparam(1);
	lua_Object modeObj = lua_getparam(2);

	if (!lua_isuserdata(actorObj) || lua_tag(actorObj) != MKTAG('A','C','T','R'))
		return;

	bool mode = lua_isnil(modeObj) != 0;
	Actor *actor = getactor(actorObj);
	actor->setRunning(mode);
}

void L1_SetActorColormap() {
	lua_Object actorObj = lua_getparam(1);
	lua_Object nameObj = lua_getparam(2);

	if (!lua_isuserdata(actorObj) || lua_tag(actorObj) != MKTAG('A','C','T','R'))
		return;

	Actor *actor = getactor(actorObj);
	if (lua_isstring(nameObj)) {
		const char *name = lua_getstring(nameObj);
		actor->setColormap(name);
	} else if (lua_isnil(nameObj)) {
		error("SetActorColormap: implement remove cmap");
		// remove ?
	}
}

void L1_TurnActor() {
	lua_Object actorObj = lua_getparam(1);
	lua_Object dirObj = lua_getparam(2);

	if (!lua_isuserdata(actorObj) || lua_tag(actorObj) != MKTAG('A','C','T','R'))
		return;

	if (!lua_isnumber(dirObj))
		return;

	Actor *actor = getactor(actorObj);
	int dir = (int)lua_getnumber(dirObj);
	// TODO verify. need clear mode walking
	actor->turn(dir);
}

void L1_PushActorCostume() {
	lua_Object actorObj = lua_getparam(1);
	lua_Object nameObj = lua_getparam(2);

	if (!lua_isuserdata(actorObj) || lua_tag(actorObj) != MKTAG('A','C','T','R'))
		return;

	if (!lua_isstring(nameObj))
		return;

	Actor *actor = getactor(actorObj);
	const char *costumeName = lua_getstring(nameObj);
	actor->pushCostume(costumeName);
}

void L1_SetActorCostume() {
	lua_Object actorObj = lua_getparam(1);
	lua_Object costumeObj = lua_getparam(2);

	if (!lua_isuserdata(actorObj) || lua_tag(actorObj) != MKTAG('A','C','T','R'))
		return;

	Actor *actor = getactor(actorObj);
	if (lua_isnil(costumeObj)) {
		actor->clearCostumes();
		pushbool(true);
		return;
	}
	if (!lua_isstring(costumeObj)) {
		pushbool(false);
		return;
	}

	const char *costumeName = lua_getstring(costumeObj);
	actor->setCostume(costumeName);
	pushbool(true);
}

void L1_GetActorCostume() {
	lua_Object actorObj = lua_getparam(1);
	lua_Object costumeObj = lua_getparam(2);

	if (!lua_isuserdata(actorObj) || lua_tag(actorObj) != MKTAG('A','C','T','R')) {
		lua_pushnil();
		return;
	}

	Actor *actor = getactor(actorObj);
	Costume *costume = actor->getCurrentCostume();
	if (lua_isnil(costumeObj)) {
		// dummy
	} else if (lua_isnumber(costumeObj)) {
/*		int num = (int)lua_getnumber(costumeObj);*/
		error("GetActorCostume: implement number Id");
	} else
		return;

	if (costume)
		lua_pushstring(costume->getFilename().c_str());
	else
		lua_pushnil();
}

void L1_PopActorCostume() {
	lua_Object actorObj = lua_getparam(1);
	if (!lua_isuserdata(actorObj) || lua_tag(actorObj) != MKTAG('A','C','T','R'))
		return;

	Actor *actor = getactor(actorObj);
	if (actor->getCurrentCostume()) {
		lua_pushstring(actor->getCurrentCostume()->getFilename().c_str());
		actor->popCostume();
	} else
		lua_pushnil();
}

void L1_GetActorCostumeDepth() {
	lua_Object actorObj = lua_getparam(1);
	if (!lua_isuserdata(actorObj) || lua_tag(actorObj) != MKTAG('A','C','T','R')) {
		lua_pushnil();
		return;
	}

	Actor *actor = getactor(actorObj);
	lua_pushnumber(actor->getCostumeStackDepth());
}

void L1_PrintActorCostumes() {
	// dummy
}

void L1_LoadCostume() {
	lua_Object nameObj = lua_getparam(1);
	if (lua_isstring(nameObj)) {
		// FIXME disable loading costume due creating issue with colormap, this opcode is unknown purpose
		//const char *name = lua_getstring(nameObj);
		//g_resourceloader->loadCostume(name, NULL);
	} else
		lua_pushnil();
}

void L1_PlayActorChore() {
	lua_Object actorObj = lua_getparam(1);
	lua_Object choreObj = lua_getparam(2);
	lua_Object costumeObj = lua_getparam(3);

	if (!lua_isuserdata(actorObj) || lua_tag(actorObj) != MKTAG('A','C','T','R'))
		return;

	Actor *actor = getactor(actorObj);

	Costume *costume;
	if (!findCostume(costumeObj, actor, &costume))
		return;

	if (!lua_isnumber(choreObj)) {
		lua_pushnil();
		return;
	}
	int chore = (int)lua_getnumber(choreObj);

	if (!costume) {
		lua_pushnil();
		return;
	}

	costume->playChore(chore);
	pushbool(true);
}

void L1_CompleteActorChore() {
	lua_Object actorObj = lua_getparam(1);
	lua_Object choreObj = lua_getparam(2);
	lua_Object costumeObj = lua_getparam(3);

	if (!lua_isuserdata(actorObj) || lua_tag(actorObj) != MKTAG('A','C','T','R'))
		return;

	Actor *actor = getactor(actorObj);

	Costume *costume;
	if (!findCostume(costumeObj, actor, &costume))
		return;

	if (!lua_isnumber(choreObj)) {
		lua_pushnil();
		return;
	}
	int chore = (int)lua_getnumber(choreObj);

	if (!costume) {
		lua_pushnil();
		return;
	}

	costume->setChoreLastFrame(chore);
	pushbool(true);
}

void L1_PlayActorChoreLooping() {
	lua_Object actorObj = lua_getparam(1);
	lua_Object choreObj = lua_getparam(2);
	lua_Object costumeObj = lua_getparam(3);

	if (!lua_isuserdata(actorObj) || lua_tag(actorObj) != MKTAG('A','C','T','R'))
		return;

	Actor *actor = getactor(actorObj);

	Costume *costume;
	if (!findCostume(costumeObj, actor, &costume))
		return;

	if (!lua_isnumber(choreObj)) {
		lua_pushnil();
		return;
	}
	int chore = (int)lua_getnumber(choreObj);

	if (!costume) {
		lua_pushnil();
		return;
	}

	costume->playChoreLooping(chore);
	pushbool(true);
}

void L1_SetActorChoreLooping() {
	lua_Object actorObj = lua_getparam(1);
	lua_Object choreObj = lua_getparam(2);
	lua_Object costumeObj = lua_getparam(4);

	if (!lua_isuserdata(actorObj) || lua_tag(actorObj) != MKTAG('A','C','T','R'))
		return;

	Actor *actor = getactor(actorObj);

	Costume *costume;
	if (!findCostume(costumeObj, actor, &costume))
		return;

	if (!costume)
		return;

	if (lua_isnumber(choreObj)) {
		int chore = (int)lua_getnumber(choreObj);
		costume->setChoreLooping(chore, getbool(3));
	} else if (lua_isnil(choreObj)) {
		error("SetActorChoreLooping: implement nil case");
	}
}

void L1_StopActorChore() {
	lua_Object actorObj = lua_getparam(1);
	lua_Object choreObj = lua_getparam(2);
	lua_Object costumeObj = lua_getparam(3);

	if (!lua_isuserdata(actorObj) || lua_tag(actorObj) != MKTAG('A','C','T','R'))
		return;

	Actor *actor = getactor(actorObj);

	Costume *costume;
	if (!findCostume(costumeObj, actor, &costume))
		return;

	if (!costume)
		return;

	if (lua_isnumber(choreObj)) {
		int chore = (int)lua_getnumber(choreObj);
		costume->stopChore(chore);
	} else if (lua_isnil(choreObj)) {
		costume->stopChores();
	}
}

void L1_FadeOutChore() {
	lua_Object actorObj = lua_getparam(1);
	lua_Object costumeObj = lua_getparam(2);
	lua_Object choreObj = lua_getparam(3);
	lua_Object timeObj = lua_getparam(4);

	if (!lua_isuserdata(actorObj) || lua_tag(actorObj) != MKTAG('A','C','T','R'))
		return;

	Actor *actor = getactor(actorObj);

	Costume *costume;
	if (!findCostume(costumeObj, actor, &costume))
		return;

	if (!costume)
		return;

	if (lua_isnumber(choreObj)) {
		int chore = (int)lua_getnumber(choreObj);
		int time = (int)lua_getnumber(timeObj);

		costume->fadeChoreOut(chore, time);
	}
}

void L1_FadeInChore() {
	lua_Object actorObj = lua_getparam(1);
	lua_Object costumeObj = lua_getparam(2);
	lua_Object choreObj = lua_getparam(3);
	lua_Object timeObj = lua_getparam(4);

	if (!lua_isuserdata(actorObj) || lua_tag(actorObj) != MKTAG('A','C','T','R'))
		return;

	Actor *actor = getactor(actorObj);

	Costume *costume;
	if (!findCostume(costumeObj, actor, &costume))
		return;

	if (!costume)
		return;

	if (lua_isnumber(choreObj)) {
		int chore = (int)lua_getnumber(choreObj);
		int time = (int)lua_getnumber(timeObj);

		costume->fadeChoreIn(chore, time);
	}
}

void L1_IsActorChoring() {
	lua_Object actorObj = lua_getparam(1);
	lua_Object choreObj = lua_getparam(2);
	lua_Object costumeObj = lua_getparam(4);

	if (!lua_isuserdata(actorObj) || lua_tag(actorObj) != MKTAG('A','C','T','R'))
		return;

	Actor *actor = getactor(actorObj);

	Costume *costume;
	if (!findCostume(costumeObj, actor, &costume))
		return;

	if (!costume) {
		lua_pushnil();
		return;
	}

	bool excludeLoop = getbool(3);
	if (lua_isnumber(choreObj)) {
		int chore = (int)lua_getnumber(choreObj);
		if (costume->isChoring(chore, excludeLoop) != -1) {
			lua_pushobject(choreObj);
			pushbool(true);
		} else
			lua_pushnil();
		return;
	} else if (lua_isnil(choreObj)) {
		for (int i = 0; i < costume->getNumChores(); i++) {
			int chore = costume->isChoring(i, excludeLoop);
			if (chore != -1) {
				// Ignore talk chores.
				bool isTalk = false;
				for (int j = 0; j < 10; j++) {
					if (costume == actor->getTalkCostume(j) && actor->getTalkChore(j) == chore) {
						isTalk = true;
						break;
					}
				}
				if (isTalk)
					continue;

				lua_pushnumber(chore);
				pushbool(true);
				return;
			}
		}
	}

	lua_pushnil();
}

void L1_ActorLookAt() {
	lua_Object actorObj = lua_getparam(1);
	lua_Object xObj = lua_getparam(2);
	lua_Object yObj = lua_getparam(3);
	lua_Object zObj = lua_getparam(4);
	lua_Object rateObj = lua_getparam(5);

	if (!lua_isuserdata(actorObj) || lua_tag(actorObj) != MKTAG('A','C','T','R'))
		return;
	Actor *actor = getactor(actorObj);
	if (!actor->getCurrentCostume())
		return;

	if (lua_isnumber(rateObj))
		actor->setLookAtRate(lua_getnumber(rateObj));

	// Look at nothing
	if (lua_isnil(xObj)) {
		if (actor->isLookAtVectorZero())
			return;

		actor->setLookAtVectorZero();
		actor->setLooking(false);
		// FIXME: When grabbing Chepito lua_getnumber(yObj) returns -3.50214
		// which doesn't make any sense. I suspect that is a bug in Lua, since
		// i couldn't find any call to manny:head_look_at(nil, -3.50214) while
		// there are some calls to glottis:setpos(-0.120987, -3.50214, 0).
		// The same number, strange indeed eh?
		if (lua_isnumber(yObj) && lua_getnumber(yObj) > 0)
			actor->setLookAtRate(lua_getnumber(yObj));
		return;
	} else if (lua_isnumber(xObj)) { // look at xyz
		float fY;
		float fZ;

		float fX = lua_getnumber(xObj);

		if (lua_isnumber(yObj))
			fY = lua_getnumber(yObj);
		else
			fY = 0.0f;

		if (lua_isnumber(zObj))
			fZ = lua_getnumber(zObj);
		else
			fZ = 0.0f;

		Graphics::Vector3d vector;
		vector.set(fX, fY, fZ);
		actor->setLookAtVector(vector);

		if (lua_isnumber(rateObj))
			actor->setLookAtRate(lua_getnumber(rateObj));
	} else if (lua_isuserdata(xObj) && lua_tag(xObj) == MKTAG('A','C','T','R')) { // look at another actor
		Actor *lookedAct = getactor(xObj);
		actor->setLookAtVector(lookedAct->getPos());

		if (lua_isnumber(yObj))
			actor->setLookAtRate(lua_getnumber(yObj));
	} else {
		return;
	}

	actor->setLooking(true);
}

/* Turn the actor to a point specified in the 3D space,
 * this should not have the actor look toward the point
 * but should rotate the entire actor toward it.
 *
 * This function must use a yaw value around the unit
 * circle and not just a difference in angles.
 */
void L1_TurnActorTo() {
	lua_Object actorObj = lua_getparam(1);
	lua_Object xObj = lua_getparam(2);
	lua_Object yObj = lua_getparam(3);
	lua_Object zObj = lua_getparam(4);
	float x, y, z;

	if (!lua_isuserdata(actorObj) || lua_tag(actorObj) != MKTAG('A','C','T','R')) {
		lua_pushnil();
		return;
	}
	Actor *actor = getactor(actorObj);

	if (lua_isuserdata(xObj) && lua_tag(xObj) == MKTAG('A','C','T','R')) {
		Actor *destActor = getactor(xObj);
		x = destActor->getPos().x();
		y = destActor->getPos().y();
		z = destActor->getPos().z();
	} else {
		x = lua_getnumber(xObj);
		y = lua_getnumber(yObj);
		z = lua_getnumber(zObj);
	}

	// TODO turning stuff below is not complete

	// Find the vector pointing from the actor to the desired location
	Graphics::Vector3d turnToVector(x, y, z);
	Graphics::Vector3d lookVector = turnToVector - actor->getPos();
	// find the angle the requested position is around the unit circle
	float yaw = lookVector.unitCircleAngle();
	// yaw is offset from forward by 90 degrees
	yaw -= 90.0f;
	if (yaw < 0) {
		yaw += 360.f;
	}
	actor->turnTo(0, yaw, 0);

	float diff = actor->getYaw() - yaw;
	// Return true if the actor is still turning and its yaw is not the target one.
	// This allows manny to have the right yaw when he exits the elevator in the garage
	pushbool((diff > 0.005) || (diff < -0.005)); //fuzzy compare
}

void L1_PointActorAt() {
	lua_Object actorObj = lua_getparam(1);
	lua_Object xObj = lua_getparam(2);
	lua_Object yObj = lua_getparam(3);
	lua_Object zObj = lua_getparam(4);
	float x, y, z;

	if (!lua_isuserdata(actorObj) || lua_tag(actorObj) != MKTAG('A','C','T','R')) {
		lua_pushnil();
		return;
	}
	Actor *actor = getactor(actorObj);

	if (lua_isuserdata(xObj) && lua_tag(xObj) == MKTAG('A','C','T','R')) {
		Actor *destActor = getactor(xObj);
		x = destActor->getPos().x();
		y = destActor->getPos().y();
		z = destActor->getPos().z();
	} else {
		x = lua_getnumber(xObj);
		y = lua_getnumber(yObj);
		z = lua_getnumber(zObj);
	}

	// TODO turning stuff below is not complete

	// Find the vector pointing from the actor to the desired location
	Graphics::Vector3d turnToVector(x, y, z);
	Graphics::Vector3d lookVector = turnToVector - actor->getPos();
	// find the angle the requested position is around the unit circle
	float yaw = lookVector.unitCircleAngle();
	// yaw is offset from forward by 90 degrees
	yaw -= 90.0f;
	actor->turnTo(0, yaw, 0);

	// Game will lock in elevator if this doesn't return false
	pushbool(false);
}

void L1_WalkActorVector() {
	lua_Object actorObj = lua_getparam(1);
	lua_Object actor2Obj = lua_getparam(2);
//	lua_Object xObj = lua_getparam(3);
//	lua_Object yObj = lua_getparam(4);
//	lua_Object zObj = lua_getparam(5);
//	lua_Object param6Obj = lua_getparam(6);

	if (!lua_isuserdata(actorObj) || lua_tag(actorObj) != MKTAG('A','C','T','R') ||
			!lua_isuserdata(actor2Obj) || lua_tag(actor2Obj) != MKTAG('A','C','T','R'))
		return;

//	Actor *actor = static_cast<Actor *>(lua_getuserdata(actorObj));
	Actor *actor2 = getactor(actor2Obj);

	// TODO whole below part need rewrote to much original
	float moveHoriz, moveVert, yaw;

	// Third option is the "left/right" movement
	moveHoriz = luaL_check_number(3);
	// Fourth Option is the "up/down" movement
	moveVert = luaL_check_number(4);

	// Get the direction the camera is pointing
	Graphics::Vector3d cameraVector = g_grim->getCurrScene()->getCurrSetup()->_interest - g_grim->getCurrScene()->getCurrSetup()->_pos;
	// find the angle the camera direction is around the unit circle
	float cameraYaw = cameraVector.unitCircleAngle();

	// Handle the turning
	Graphics::Vector3d adjustVector(moveHoriz, moveVert, 0);
	// find the angle the adjust vector is around the unit circle
	float adjustYaw = adjustVector.unitCircleAngle();

	yaw = cameraYaw + adjustYaw;
	// yaw is offset from forward by 180 degrees
	yaw -= 180.0f;
	// set the yaw so it can be compared against the current
	// value for the actor yaw
	if (yaw < 0.0f)
		yaw += 360.0f;
	if (yaw >= 360.0f)
		yaw -= 360.0f;
	// set the new direction or walk forward
	if (actor2->getYaw() != yaw)
		actor2->turnTo(0, yaw, 0);
	else
		actor2->walkForward();
}

/* Set the pitch of the actor to the requested value,
 * this will rotate an actor toward/away from the ground.
 * This is used when Glottis runs over the signpost in
 * the Petrified Forest
 */
void L1_SetActorPitch() {
	lua_Object actorObj = lua_getparam(1);
	lua_Object pitchObj = lua_getparam(2);

	if (!lua_isuserdata(actorObj) || lua_tag(actorObj) != MKTAG('A','C','T','R'))
		return;

	Actor *actor = getactor(actorObj);
	float pitch = lua_getnumber(pitchObj);
	actor->setRot(pitch, actor->getYaw(), actor->getRoll());
}

void L1_SetActorLookRate() {
	lua_Object actorObj = lua_getparam(1);
	lua_Object rateObj = lua_getparam(2);

	if (!lua_isuserdata(actorObj) || lua_tag(actorObj) != MKTAG('A','C','T','R'))
		return;

	if (!lua_isnumber(rateObj))
		return;

	Actor *actor = getactor(actorObj);
	if (!actor->getCurrentCostume())
		return;

	float rate = lua_getnumber(rateObj);
	actor->setLookAtRate(rate);
}

void L1_GetActorLookRate() {
	lua_Object actorObj = lua_getparam(1);

	if (!lua_isuserdata(actorObj) || lua_tag(actorObj) != MKTAG('A','C','T','R'))
		return;

	Actor *actor = getactor(actorObj);
	if (!actor->getCurrentCostume())
		lua_pushnil();
	else
		lua_pushnumber(actor->getLookAtRate());
}

void L1_SetActorHead() {
	lua_Object actorObj = lua_getparam(1);
	lua_Object joint1Obj = lua_getparam(2);
	lua_Object joint2Obj = lua_getparam(3);
	lua_Object joint3Obj = lua_getparam(4);
	lua_Object maxRollObj = lua_getparam(5);
	lua_Object maxPitchObj = lua_getparam(6);
	lua_Object maxYawObj = lua_getparam(7);

	if (!lua_isuserdata(actorObj) || lua_tag(actorObj) != MKTAG('A','C','T','R'))
		return;
	if (!lua_isnumber(joint1Obj) || !lua_isnumber(joint2Obj) || !lua_isnumber(joint3Obj) ||
			!lua_isnumber(maxRollObj) || !lua_isnumber(maxPitchObj) || !lua_isnumber(maxYawObj))
		return;

	Actor *actor = getactor(actorObj);
	int joint1 = (int)lua_getnumber(joint1Obj);
	int joint2 = (int)lua_getnumber(joint2Obj);
	int joint3 = (int)lua_getnumber(joint3Obj);
	float maxRoll = lua_getnumber(maxRollObj);
	float maxPitch = lua_getnumber(maxPitchObj);
	float maxYaw = lua_getnumber(maxYawObj);

	actor->setHead(joint1, joint2, joint3, maxRoll, maxPitch, maxYaw);
}

void L1_PutActorAtInterest() {
	lua_Object actorObj = lua_getparam(1);

	if (!lua_isuserdata(actorObj) || lua_tag(actorObj) != MKTAG('A','C','T','R'))
		return;

	Actor *actor = getactor(actorObj);
	Scene *scene = g_grim->getCurrScene();
	if (!scene)
		return;

	Graphics::Vector3d p = scene->getCurrSetup()->_interest;
	Graphics::Vector3d resultPt = p;
	float minDist = -1.f;

	for (int i = 0; i < scene->getSectorCount(); ++i) {
		Sector *sector = scene->getSectorBase(i);
		if (sector->getType() != Sector::WalkType || !sector->isVisible())
			continue;

		Graphics::Vector3d closestPt = sector->getClosestPoint(p);
		if (scene->findPointSector(closestPt, Sector::HotType))
			continue;
		float thisDist = (closestPt - p).magnitude();
		if (minDist < 0 || thisDist < minDist) {
			resultPt = closestPt;
			minDist = thisDist;
		}
	}

	actor->setPos(resultPt);
}

void L1_SetActorFollowBoxes() {
	lua_Object actorObj = lua_getparam(1);
	lua_Object modeObj = lua_getparam(2);
	bool mode = true;

	if (!lua_isuserdata(actorObj) || lua_tag(actorObj) != MKTAG('A','C','T','R'))
		return;

	Actor *actor = getactor(actorObj);

	if (modeObj == LUA_NOOBJECT || lua_isnil(modeObj))
		mode = false;

	warning("SetActorFollowBoxes() not implemented");
	// TODO that is not walkbox walking, but temporary hack
	// actor->enableWalkbox(mode);
	actor->setConstrain(mode);
}

void L1_SetActorConstrain() {
	lua_Object actorObj = lua_getparam(1);
//	lua_Object constrainObj = lua_getparam(2);

	if (!lua_isuserdata(actorObj) || lua_tag(actorObj) != MKTAG('A','C','T','R'))
		return;

//	Actor *actor = static_cast<Actor *>(lua_getuserdata(actorObj));
//	bool constrain = !lua_isnil(constrainObj);

	// FIXME that below should be enabled, but for now it's disabled realated to
	// above func SetActorFollowBoxes.
//	actor->setConstrain(constrain);
}

void L1_GetVisibleThings() {
	lua_Object actorObj = lua_getparam(1);
	Actor *actor = NULL;
	if (lua_isnil(actorObj)) {
		actor = g_grim->getSelectedActor();
		if (!actor)
			return;
	} else if (lua_isuserdata(actorObj) && lua_tag(actorObj) == MKTAG('A','C','T','R')) {
		actor = getactor(actorObj);
	}
	assert(actor);

	lua_Object result = lua_createtable();

	// TODO verify code below
	for (Actor::Pool::Iterator i = Actor::getPool()->getBegin(); i != Actor::getPool()->getEnd(); ++i) {
		Actor *a = i->_value;
		if (!i->_value->isInSet(g_grim->getSceneName()))
			continue;
		// Consider the active actor visible
		if (actor == a || actor->getYawTo(*a) < 90) {
			lua_pushobject(result);
			lua_pushusertag(i->_key, MKTAG('A','C','T','R'));
			lua_pushnumber(1);
			lua_settable();
		}
	}
	lua_pushobject(result);
}

void L1_SetShadowColor() {
	int r = (int)lua_getnumber(lua_getparam(1));
	int g = (int)lua_getnumber(lua_getparam(2));
	int b = (int)lua_getnumber(lua_getparam(3));

	g_driver->setShadowColor(r, g, b);
}

void L1_KillActorShadows() {
	lua_Object actorObj = lua_getparam(1);

	if (!lua_isuserdata(actorObj) || lua_tag(actorObj) != MKTAG('A','C','T','R')) {
		lua_pushnil();
		return;
	}
	Actor *actor = getactor(actorObj);
	actor->clearShadowPlanes();
}

void L1_SetActiveShadow() {
	lua_Object actorObj = lua_getparam(1);
	lua_Object shadowIdObj = lua_getparam(2);

	if (!lua_isuserdata(actorObj) || actorObj == LUA_NOOBJECT) {
		lua_pushnil();
		return;
	}
	Actor *actor = getactor(actorObj);
	int shadowId = (int)lua_getnumber(shadowIdObj);
	actor->setActiveShadow(shadowId);
}

void L1_SetActorShadowPoint() {
	lua_Object actorObj = lua_getparam(1);
	lua_Object xObj = lua_getparam(2);
	lua_Object yObj = lua_getparam(3);
	lua_Object zObj = lua_getparam(4);

	if (!lua_isuserdata(actorObj) || actorObj == LUA_NOOBJECT) {
		lua_pushnil();
		return;
	}
	Actor *actor = getactor(actorObj);
	float x = lua_getnumber(xObj);
	float y = lua_getnumber(yObj);
	float z = lua_getnumber(zObj);

	actor->setShadowPoint(Graphics::Vector3d(x, y, z));
}

void L1_SetActorShadowPlane() {
	lua_Object actorObj = lua_getparam(1);
	lua_Object nameObj = lua_getparam(2);

	if (!lua_isuserdata(actorObj) || actorObj == LUA_NOOBJECT) {
		lua_pushnil();
		return;
	}
	Actor *actor = getactor(actorObj);
	const char *name = lua_getstring(nameObj);

	actor->setShadowPlane(name);
}

void L1_AddShadowPlane() {
	lua_Object actorObj = lua_getparam(1);
	lua_Object nameObj = lua_getparam(2);

	if (!lua_isuserdata(actorObj) || actorObj == LUA_NOOBJECT) {
		lua_pushnil();
		return;
	}
	Actor *actor = getactor(actorObj);
	const char *name = lua_getstring(nameObj);

	actor->addShadowPlane(name);
}

void L1_ActivateActorShadow() {
	lua_Object actorObj = lua_getparam(1);
	lua_Object shadowIdObj = lua_getparam(2);
	lua_Object stateObj = lua_getparam(3);

	if (!lua_isuserdata(actorObj) || actorObj == LUA_NOOBJECT) {
		lua_pushnil();
		return;
	}
	Actor *actor = getactor(actorObj);
	int shadowId = (int)lua_getnumber(shadowIdObj);
	bool state = !lua_isnil(stateObj);

	actor->setActivateShadow(shadowId, state);
	g_grim->flagRefreshShadowMask(true);
}

void L1_SetActorShadowValid() {
	lua_Object actorObj = lua_getparam(1);
	lua_Object numObj = lua_getparam(2);

	if (!lua_isuserdata(actorObj) || actorObj == LUA_NOOBJECT) {
		lua_pushnil();
		return;
	}
	Actor *actor = getactor(actorObj);
	int valid = (int)lua_getnumber(numObj);

	warning("SetActorShadowValid(%d) unknown purpose", valid);

	actor->setShadowValid(valid);
}

} // end of namespace Grim
