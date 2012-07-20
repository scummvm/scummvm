/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
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

#include "engines/grim/emi/lua_v2.h"
#include "engines/grim/lua/lua.h"

#include "engines/grim/actor.h"
#include "engines/grim/grim.h"
#include "engines/grim/costume.h"
#include "engines/grim/costume/chore.h"

#include "engines/grim/emi/costumeemi.h"
#include "engines/grim/emi/skeleton.h"
#include "engines/grim/emi/costume/emiskel_component.h"

namespace Grim {

void Lua_V2::SetActorLocalAlpha() {
	lua_Object actorObj = lua_getparam(1);

	if (!lua_isuserdata(actorObj) || lua_tag(actorObj) != MKTAG('A','C','T','R'))
		return;

	Actor *actor = getactor(actorObj);
	if (!actor)
		return;

	// FIXME: implement missing code
	warning("Lua_V2::SetActorLocalAlpha: stub, actor: %s", actor->getName().c_str());
}


void Lua_V2::SetActorGlobalAlpha() {
	lua_Object actorObj = lua_getparam(1);
	lua_Object alphaObj = lua_getparam(2);
//  lua_Object meshObj = lua_getparam(3);

	if (!lua_isuserdata(actorObj) || lua_tag(actorObj) != MKTAG('A','C','T','R'))
		return;

	Actor *actor = getactor(actorObj);
	if (!actor)
		return;

	if (!lua_isnumber(alphaObj))
			return;

	float alpha = lua_getnumber(alphaObj);
	if (alpha == Actor::AlphaOff ||
	    alpha == Actor::AlphaReplace ||
	    alpha == Actor::AlphaModulate) {
			actor->setAlphaMode((Actor::AlphaMode) (int) alpha);
	} else {
		actor->setGlobalAlpha(alpha);
	}
}

void Lua_V2::PutActorInOverworld() {
	lua_Object actorObj = lua_getparam(1);

	if (!lua_isuserdata(actorObj) || lua_tag(actorObj) != MKTAG('A','C','T','R'))
		return;

	Actor *actor = getactor(actorObj);

	actor->setInOverworld(true);
}

void Lua_V2::RemoveActorFromOverworld() {
	lua_Object actorObj = lua_getparam(1);

	if (!lua_isuserdata(actorObj) || lua_tag(actorObj) != MKTAG('A','C','T','R'))
		return;

	Actor *actor = getactor(actorObj);
	if (!actor)
		return;

	actor->setInOverworld(false);
}

void Lua_V2::UnloadActor() {
	lua_Object actorObj = lua_getparam(1);

	if (!lua_isuserdata(actorObj) || lua_tag(actorObj) != MKTAG('A','C','T','R'))
		return;

	Actor *actor = getactor(actorObj);
	if (!actor)
		return;

	// This should be safe.
	delete actor;
	g_grim->invalidateActiveActorsList();
}

void Lua_V2::SetActorWalkRate() {
	lua_Object actorObj = lua_getparam(1);
	lua_Object rateObj = lua_getparam(2);

	if (!lua_isuserdata(actorObj) || lua_tag(actorObj) != MKTAG('A','C','T','R'))
		return;
	if (!lua_isnumber(rateObj))
		return;

	Actor *actor = getactor(actorObj);
	float rate = lua_getnumber(rateObj);
	// const below only differ from grim
	actor->setWalkRate(rate * 3.279999971389771);
}

void Lua_V2::GetActorWalkRate() {
	lua_Object actorObj = lua_getparam(1);
	if (!lua_isuserdata(actorObj) || lua_tag(actorObj) != MKTAG('A','C','T','R'))
		return;

	Actor *actor = getactor(actorObj);
	// const below only differ from grim
	lua_pushnumber(actor->getWalkRate() * 0.3048780560493469);
}

void Lua_V2::LockChore() {
	lua_Object nameObj = lua_getparam(1);
	lua_Object filenameObj = lua_getparam(2);

	if (!lua_isstring(nameObj) || !lua_isstring(filenameObj)) {
		lua_pushnil();
		return;
	}

	const char *name = lua_getstring(nameObj);
	const char *filename = lua_getstring(filenameObj);
	warning("Lua_V2::LockChore, name: %s, filename: %s", name, filename);
	// FIXME: implement missing rest part of code
}

void Lua_V2::UnlockChore() {
	lua_Object nameObj = lua_getparam(1);
	lua_Object filenameObj = lua_getparam(2);

	if (!lua_isstring(nameObj) || !lua_isstring(filenameObj)) {
		lua_pushnil();
		return;
	}

	const char *name = lua_getstring(nameObj);
	const char *filename = lua_getstring(filenameObj);
	warning("Lua_V2::UnlockChore, name: %s, filename: %s", name, filename);
	// FIXME: implement missing rest part of code
}

void Lua_V2::IsChoreValid() {
	lua_Object choreObj = lua_getparam(1);

	if (!lua_isuserdata(choreObj) || lua_tag(choreObj) != MKTAG('C','H','O','R'))
		return;

	int chore = lua_getuserdata(choreObj);
	Chore *c = PoolChore::getPool().getObject(chore);

	if (c) {
		pushbool(c != NULL);
	} else {
		lua_pushnil();
	}
}

void Lua_V2::IsChorePlaying() {
	lua_Object choreObj = lua_getparam(1);

	if (!lua_isuserdata(choreObj) || lua_tag(choreObj) != MKTAG('C','H','O','R'))
		return;

	int chore = lua_getuserdata(choreObj);
	Chore *c = PoolChore::getPool().getObject(chore);

	if (c) {
		pushbool(c->isPlaying());
	} else {
		lua_pushnil();
	}
}

void Lua_V2::IsChoreLooping() {
	lua_Object choreObj = lua_getparam(1);

	if (!lua_isuserdata(choreObj) || lua_tag(choreObj) != MKTAG('C','H','O','R'))
		return;

	int chore = lua_getuserdata(choreObj);
	Chore *c = PoolChore::getPool().getObject(chore);

	if (c) {
		pushbool(c->isLooping());
	} else {
		lua_pushnil();
	}
}

void Lua_V2::SetChoreLooping() {
	lua_Object choreObj = lua_getparam(1);
	if (!lua_isuserdata(choreObj) || lua_tag(choreObj) != MKTAG('C','H','O','R'))
		return;

	int chore = lua_getuserdata(choreObj);
	Chore *c = PoolChore::getPool().getObject(chore);

	if (c) {
		c->setLooping(false);
	}
	lua_pushnil();
}

void Lua_V2::StopChore() {
	lua_Object choreObj = lua_getparam(1);
	lua_Object timeObj = lua_getparam(2);

	if (!lua_isuserdata(choreObj) || lua_tag(choreObj) != MKTAG('C','H','O','R') || !lua_isnumber(timeObj))
		return;

	int chore = lua_getuserdata(choreObj);
	float time = lua_getnumber(timeObj);
	// FIXME: implement missing rest part of code
	warning("Lua_V2::StopChore: stub, chore: %d time: %f", chore, time);
	Chore *c = PoolChore::getPool().getObject(chore);
	if (c) {
		c->stop();
	}
}

void Lua_V2::AdvanceChore() {
	lua_Object choreObj = lua_getparam(1);
	lua_Object timeObj = lua_getparam(2);

	if (!lua_isuserdata(choreObj) || lua_tag(choreObj) != MKTAG('C','H','O','R') || !lua_isnumber(timeObj))
		return;

	int chore = lua_getuserdata(choreObj);
	float time = lua_getnumber(timeObj);
	// FIXME: implement missong code
	warning("Lua_V2::AdvanceChore: stub, chore: %d time: %f", chore, time);
	Chore *c = PoolChore::getPool().getObject(chore);
	if (c) {
		// this is kind of wrong, but it at least gets stuff to draw.
		c->getOwner()->playChoreLooping(c->getChoreId());
	}
}

void Lua_V2::SetActorSortOrder() {
	lua_Object actorObj = lua_getparam(1);
	lua_Object orderObj = lua_getparam(2);

	if (!lua_isuserdata(actorObj) || lua_tag(actorObj) != MKTAG('A','C','T','R'))
		return;

	if (!lua_isnumber(orderObj))
		return;

	Actor *actor = getactor(actorObj);
	int order = (int)lua_getnumber(orderObj);
	actor->setSortOrder(order);
}

void Lua_V2::GetActorSortOrder() {
	lua_Object actorObj = lua_getparam(1);

	if (!lua_isuserdata(actorObj) || lua_tag(actorObj) != MKTAG('A','C','T','R'))
		return;

	Actor *actor = getactor(actorObj);
	lua_pushnumber(actor->getSortOrder());
}

void Lua_V2::ActorActivateShadow() {
	lua_Object actorObj = lua_getparam(1);
	lua_Object qualityObj = lua_getparam(2);
	lua_Object planeObj = lua_getparam(3);

	if (!lua_isuserdata(actorObj) || lua_tag(actorObj) != MKTAG('A','C','T','R'))
		return;

	Actor *actor = getactor(actorObj);
	if (!actor)
		return;
	int quality = (int)lua_getnumber(qualityObj);
	const char *plane = "NULL";
	if (lua_isstring(planeObj))
		plane = lua_getstring(planeObj);
	warning("Lua_V2::ActorActivateShadow, actor: %s, aquality: %d, plane: %s", actor->getName().c_str(), quality, plane);
	// FIXME: implement missing rest part of code
}

void Lua_V2::ActorStopMoving() {
	lua_Object actorObj = lua_getparam(1);

	if (!lua_isuserdata(actorObj) || lua_tag(actorObj) != MKTAG('A','C','T','R'))
		return;

	Actor *actor = getactor(actorObj);

	warning("Lua_V2::ActorStopMoving, actor: %s", actor->getName().c_str());
	// FIXME: implement missing rest part of code
}

void Lua_V2::GetActorWorldPos() {
	Lua_V1::GetActorPos();
	lua_Object actorObj = lua_getparam(1);

	if (!lua_isuserdata(actorObj) || lua_tag(actorObj) != MKTAG('A','C','T','R'))
		return;

	Actor *actor = getactor(actorObj);
	if (!actor)
		return;

	Math::Vector3d pos = actor->getWorldPos();
	lua_pushnumber(pos.x());
	lua_pushnumber(pos.y());
	lua_pushnumber(pos.z());
}

void Lua_V2::PutActorInSet() {
	lua_Object actorObj = lua_getparam(1);
	lua_Object setObj = lua_getparam(2);

	if (!lua_isuserdata(actorObj) || lua_tag(actorObj) != MKTAG('A','C','T','R'))
		return;

	Actor *actor = getactor(actorObj);

	if (!lua_isstring(setObj) && !lua_isnil(setObj)) {
		lua_pushnil();
		return;
	}

	const char *set = lua_getstring(setObj);

	// FIXME verify adding actor to set
	if (!set) {
		actor->putInSet("");
		lua_pushnil();
	} else {
		if (!actor->isInSet(set)) {
			actor->putInSet(set);
		}
		lua_pushnumber(1.0);
	}
}

void Lua_V2::SetActorRestChore() {
	lua_Object actorObj = lua_getparam(1);
	lua_Object choreObj = lua_getparam(2);
	lua_Object costumeObj = lua_getparam(3);
	Costume *costume;
	int chore;

	if (!lua_isuserdata(actorObj) || lua_tag(actorObj) != MKTAG('A','C','T','R') ||
			(!lua_isstring(choreObj) && !lua_isnil(choreObj))) {
		return;
	}

	Actor *actor = getactor(actorObj);

	if (!findCostume(costumeObj, actor, &costume))
		return;

	if (lua_isnil(choreObj)) {
		chore = -1;
	} else {
		const char * choreStr = lua_getstring(choreObj);
		chore = costume->getChoreId(choreStr);
	}

	actor->setRestChore(chore, costume);
}

void Lua_V2::SetActorWalkChore() {
	lua_Object actorObj = lua_getparam(1);
	lua_Object choreObj = lua_getparam(2);
	lua_Object costumeObj = lua_getparam(3);
	Costume *costume;
	int chore;

	if (!lua_isuserdata(actorObj) || lua_tag(actorObj) != MKTAG('A','C','T','R') ||
			(!lua_isstring(choreObj) && !lua_isnil(choreObj))) {
		return;
	}

	Actor *actor = getactor(actorObj);

	if (!findCostume(costumeObj, actor, &costume))
		return;

	if (lua_isnil(choreObj)) {
		chore = -1;
	} else {
		const char * choreStr = lua_getstring(choreObj);
		chore = costume->getChoreId(choreStr);
	}

	actor->setWalkChore(chore, costume);
}

void Lua_V2::SetActorTurnChores() {
	lua_Object actorObj = lua_getparam(1);
	lua_Object leftChoreObj = lua_getparam(2);
	lua_Object rightChoreObj = lua_getparam(3);
	lua_Object costumeObj = lua_getparam(4);
	Costume *costume;

	if (!lua_isuserdata(actorObj) || lua_tag(actorObj) != MKTAG('A','C','T','R') ||
			(!lua_isstring(leftChoreObj) && !lua_isstring(rightChoreObj))) {
		return;
	}

	Actor *actor = getactor(actorObj);

	if (!findCostume(costumeObj, actor, &costume))
		return;

	int leftChore = costume->getChoreId(lua_getstring(leftChoreObj));
	int rightChore = costume->getChoreId(lua_getstring(rightChoreObj));

	actor->setTurnChores(leftChore, rightChore, costume);
}

void Lua_V2::SetActorTalkChore() {
	lua_Object actorObj = lua_getparam(1);
	lua_Object indexObj = lua_getparam(2);
	lua_Object choreObj = lua_getparam(3);
	lua_Object costumeObj = lua_getparam(4);
	Costume *costume;
	int chore;

	if (!lua_isuserdata(actorObj) || lua_tag(actorObj) != MKTAG('A','C','T','R') ||
			!lua_isnumber(indexObj) ||
			(!lua_isstring(choreObj) && !lua_isnil(choreObj))) {
		return;
	}

	int index = (int)lua_getnumber(indexObj);
	if (index < 1 || index > 16)
		return;

	Actor *actor = getactor(actorObj);

	if (!findCostume(costumeObj, actor, &costume))
		return;

	if (lua_isnil(choreObj)) {
		chore = -1;
	} else {
		const char * choreStr = lua_getstring(choreObj);
		chore = costume->getChoreId(choreStr);
	}

	actor->setTalkChore(index, chore, costume);
}

void Lua_V2::SetActorMumblechore() {
	lua_Object actorObj = lua_getparam(1);
	lua_Object choreObj = lua_getparam(2);
	lua_Object costumeObj = lua_getparam(3);
	Costume *costume;
	int chore;

	if (!lua_isuserdata(actorObj) || lua_tag(actorObj) != MKTAG('A','C','T','R') ||
			(!lua_isstring(choreObj) && !lua_isnil(choreObj))) {
		return;
	}

	Actor *actor = getactor(actorObj);

	if (!findCostume(costumeObj, actor, &costume))
		return;

	if (lua_isnil(choreObj)) {
		chore = -1;
	} else {
		const char * choreStr = lua_getstring(choreObj);
		chore = costume->getChoreId(choreStr);
	}

	actor->setMumbleChore(chore, costume);
}

void Lua_V2::GetActorChores() {
	lua_Object actorObj = lua_getparam(1);
	if (!lua_isuserdata(actorObj) || lua_tag(actorObj) != MKTAG('A','C','T','R'))
		return;
	Actor *actor = getactor(actorObj);
	Costume *costume = actor->getCurrentCostume();

	lua_Object result = lua_createtable();
	lua_pushobject(result);

	if (!costume) {
		lua_pushstring("count");
		lua_pushnumber(0);
		lua_settable();
		lua_pushobject(result);
		return;
	}

	int num = costume->getNumChores();

	lua_pushstring("count");
	lua_pushnumber(num);
	lua_settable();

	for (int i = 0; i < num; ++i) {
		lua_pushobject(result);
		lua_pushnumber(i);
		lua_pushusertag(((PoolChore *)costume->getChore(i))->getId(), MKTAG('C','H','O','R'));
		lua_settable();
	}

	lua_pushobject(result);
}

void Lua_V2::PlayActorChore() {
	lua_Object actorObj = lua_getparam(1);
	lua_Object choreObj = lua_getparam(2);
	lua_Object costumeObj = lua_getparam(3);
	lua_Object modeObj = lua_getparam(4);
	lua_Object paramObj = lua_getparam(5);

	if (!lua_isuserdata(actorObj) || lua_tag(actorObj) != MKTAG('A','C','T','R'))
		return;

	Actor *actor = getactor(actorObj);

	if (!lua_isstring(choreObj) || !lua_isstring(costumeObj))
		lua_pushnil();

	bool mode = false;
	float param = 0.0;

	if (!lua_isnil(modeObj)) {
		if (lua_getnumber(modeObj) != 0.0)
			mode = true;
		if (!lua_isnil(paramObj))
			if (lua_isnumber(paramObj))
				param = lua_getnumber(paramObj);
	}

	const char *choreName = lua_getstring(choreObj);
	const char *costumeName = lua_getstring(costumeObj);
	Costume *costume = actor->findCostume(costumeName);

	if (costume == NULL) {
		actor->pushCostume(costumeName);
		costume = actor->findCostume(costumeName);
	}

	PoolChore *chore = (PoolChore *)costume->getChore(choreName);
	if (mode) {
		costume->playChoreLooping(choreName);
	} else {
		costume->playChore(choreName);
	}
	if (chore) {
		lua_pushusertag(chore->getId(), MKTAG('C','H','O','R'));
	} else {
		lua_pushnil();
	}

}

void Lua_V2::StopActorChores() {
	lua_Object actorObj = lua_getparam(1);
	/*lua_Object paramObj = */lua_getparam(2);

	if (!lua_isuserdata(actorObj) || lua_tag(actorObj) != MKTAG('A','C','T','R'))
		return;

	Actor *actor = getactor(actorObj);
	if (!actor)
		return;

	//FIXME: What does the second param actually do
//	bool p = lua_isnil(paramObj) != 0;
	Costume *costume = actor->getCurrentCostume();
	if (costume) {
		costume->stopChores();
	}
}

void Lua_V2::SetActorLighting() {
	lua_Object actorObj = lua_getparam(1);
	lua_Object lightModeObj = lua_getparam(2);

	if (!lua_isuserdata(actorObj) || lua_tag(actorObj) != MKTAG('A','C','T','R'))
		return;

	Actor *actor = getactor(actorObj);
	if (!actor)
		return;

	if (lua_isnil(lightModeObj) || !lua_isnumber(lightModeObj))
		return;

	int lightMode = (int)lua_getnumber(lightModeObj);
	if (lightMode != 0) {
		if (lightMode == 1) {
			//FIXME actor->
			warning("Lua_V2::SetActorLighting: case param 1(LIGHT_FASTDYN), actor: %s", actor->getName().c_str());
		} else if (lightMode == 2) {
			//FIXME actor->
			warning("Lua_V2::SetActorLighting: case param 2(LIGHT_NORMDYN), actor: %s", actor->getName().c_str());
		} else {
			actor->setGlobalAlpha(0.0f);
			actor->setAlphaMode(Grim::Actor::AlphaReplace);
		}
	} else {
		//FIXME actor->
		warning("Lua_V2::SetActorLighting: case param 0(LIGHT_STATIC), actor: %s", actor->getName().c_str());
	}
}

void Lua_V2::SetActorCollisionMode() {
	lua_Object actorObj = lua_getparam(1);
	lua_Object modeObj = lua_getparam(2);

	if (!lua_isuserdata(actorObj) || lua_tag(actorObj) != MKTAG('A','C','T','R'))
		return;

	Actor *actor = getactor(actorObj);
	assert(actor);
	int mode = (int)lua_getnumber(modeObj);
	// From _actors.lua
	// COLLISION_OFF = 0
	// COLLISION_BOX = 1
	// COLLISION_SPHERE = 2

	// FIXME: set collision mode
	//actor->func(mode);
	warning("Lua_V2::SetActorCollisionMode: implement opcode. Mode %d", mode);
}

void Lua_V2::SetActorCollisionScale() {
	lua_Object actorObj = lua_getparam(1);
	lua_Object scaleObj = lua_getparam(2);

	if (!lua_isuserdata(actorObj) || lua_tag(actorObj) != MKTAG('A','C','T','R'))
		return;

	Actor *actor = getactor(actorObj);
	assert(actor);

	float scale = lua_getnumber(scaleObj);
	// FIXME: set collision scale
	//actor->func(scale);
	warning("Lua_V2::SetActorCollisionScale: implement opcode. Scale %f", scale);
}

void Lua_V2::GetActorPuckVector() {
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

	Math::Vector3d result = actor->getPuckVector();
	if (!lua_isnil(addObj))
		result += actor->getPos();

	lua_pushnumber(result.x());
	lua_pushnumber(result.y());
	lua_pushnumber(result.z());
}

void Lua_V2::SetActorHeadLimits() {
	lua_Object actorObj = lua_getparam(1);
	lua_Object param2Obj = lua_getparam(2);
	lua_Object param3Obj = lua_getparam(3);
	lua_Object param4Obj = lua_getparam(4);

	if (!lua_isuserdata(actorObj) || lua_tag(actorObj) != MKTAG('A','C','T','R'))
		return;

	Actor *actor = getactor(actorObj);
	if (!actor)
		return;

	if (lua_isnumber(param2Obj) && lua_isnumber(param3Obj) && lua_isnumber(param4Obj)) {
		float param2 = lua_getnumber(param2Obj); // belows needs multiply by some runtime value
		float param3 = lua_getnumber(param3Obj);
		float param4 = lua_getnumber(param4Obj);
		// FIXME: implement missing func
		//actor->func(param2, param3, param4);
		warning("Lua_V2::SetActorHeadLimits: implement opcode. actor: %s, params: %f, %f, %f", actor->getName().c_str(), param2, param3, param4);
	}
}

void Lua_V2::SetActorFOV() {
	lua_Object actorObj = lua_getparam(1);
	lua_Object fovObj = lua_getparam(2);

	if (!lua_isuserdata(actorObj) || lua_tag(actorObj) != MKTAG('A','C','T','R'))
		return;

	Actor *actor = getactor(actorObj);
	if (!actor)
		return;

	if (lua_isnumber(fovObj)) {
		float fov = lua_getnumber(fovObj);
		// FIXME: implement missing code
		//actor->func(fov); // cos(fov * some tuntime val * 0.5)
		warning("Lua_V2::SetActorFOV: implement opcode. actor: %s, param: %f", actor->getName().c_str(), fov);
	}
}

void Lua_V2::AttachActor() {
	// Missing lua parts
	lua_Object attachedObj = lua_getparam(1);
	lua_Object actorObj = lua_getparam(2);
	lua_Object jointObj = lua_getparam(3);

	if (!lua_isuserdata(actorObj) || lua_tag(actorObj) != MKTAG('A','C','T','R'))
		return;

	Actor *actor = getactor(actorObj);
	if (!actor)
		return;

	if (!lua_isuserdata(attachedObj) || lua_tag(attachedObj) != MKTAG('A','C','T','R'))
		return;

	Actor *attached = getactor(attachedObj);
	if (!attached)
		return;

	const char * joint = NULL;
	if (!lua_isnil(jointObj)) {
		joint = lua_getstring(jointObj);
	}

	attached->attachToActor(actor, joint);
	warning("Lua_V2::AttachActor: attaching %s to %s (on %s)", attached->getName().c_str(), actor->getName().c_str(), joint ? joint : "(none)");
}

void Lua_V2::DetachActor() {
	// Missing lua parts
	lua_Object attachedObj = lua_getparam(1);

	if (!lua_isuserdata(attachedObj) || lua_tag(attachedObj) != MKTAG('A','C','T','R'))
		return;

	Actor *attached = getactor(attachedObj);
	if (!attached)
		return;

	warning("Lua_V2::DetachActor: detaching %s from parent actor", attached->getName().c_str());
	attached->detach();
}

} // end of namespace Grim
