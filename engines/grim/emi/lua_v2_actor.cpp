/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
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

#include "common/debug-channels.h"

#include "engines/grim/emi/lua_v2.h"
#include "engines/grim/lua/lua.h"

#include "engines/grim/actor.h"
#include "engines/grim/debug.h"
#include "engines/grim/grim.h"
#include "engines/grim/costume.h"
#include "engines/grim/set.h"

#include "engines/grim/emi/emi.h"
#include "engines/grim/emi/costumeemi.h"
#include "engines/grim/emi/skeleton.h"
#include "engines/grim/emi/costume/emichore.h"
#include "engines/grim/emi/costume/emiskel_component.h"

#include "engines/grim/lua/lauxlib.h"

namespace Grim {

void Lua_V2::SetActorLocalAlpha() {
	lua_Object actorObj = lua_getparam(1);
	lua_Object vertexObj= lua_getparam(2);
	lua_Object alphaObj = lua_getparam(3);
	// lua_Object unknownObj = lua_getparam(4);

	if (!lua_isuserdata(actorObj) || lua_tag(actorObj) != MKTAG('A','C','T','R'))
		return;

	Actor *actor = getactor(actorObj);
	if (!actor)
		return;

	if (!lua_isnumber(vertexObj))
		return;

	if (!lua_isnumber(alphaObj))
		return;

	int vertex = lua_getnumber(vertexObj);
	float alpha = lua_getnumber(alphaObj);

	Actor::AlphaMode mode = (Actor::AlphaMode)(int)alpha;

	if (mode == Actor::AlphaOff || mode == Actor::AlphaReplace || mode == Actor::AlphaModulate) {
		actor->setLocalAlphaMode(vertex, mode);
	} else {
		actor->setLocalAlpha(vertex, alpha);
	}
}


void Lua_V2::SetActorGlobalAlpha() {
	lua_Object actorObj = lua_getparam(1);
	lua_Object alphaObj = lua_getparam(2);
	lua_Object meshObj = lua_getparam(3);

	if (!lua_isuserdata(actorObj) || lua_tag(actorObj) != MKTAG('A','C','T','R'))
		return;

	Actor *actor = getactor(actorObj);
	if (!actor)
		return;

	if (!lua_isnumber(alphaObj))
		return;

	const char *mesh = nullptr;
	if (lua_isstring(meshObj)) {
		mesh = lua_getstring(meshObj);
	}
	float alpha = lua_getnumber(alphaObj);
	if (alpha == Actor::AlphaOff ||
	    alpha == Actor::AlphaReplace ||
	    alpha == Actor::AlphaModulate) {
			actor->setAlphaMode((Actor::AlphaMode) (int) alpha, mesh);
	} else {
		actor->setGlobalAlpha(alpha, mesh);
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

	g_grim->invalidateActiveActorsList();
	g_grim->immediatelyRemoveActor(actor);
	delete actor;
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

void Lua_V2::SetActorTurnRate() {
	lua_Object actorObj = lua_getparam(1);
	lua_Object rateObj = lua_getparam(2);

	if (!lua_isuserdata(actorObj) || lua_tag(actorObj) != MKTAG('A','C','T','R'))
		return;
	if (!lua_isnumber(rateObj))
		return;

	Actor *actor = getactor(actorObj);
	float rate = lua_getnumber(rateObj); // FIXME verify negate values of rate

	// special handling of value 1 only used for voodoo chair
	actor->setTurnRate((rate == 1) ? 100 : rate);
}

void Lua_V2::LockChoreSet() {
	lua_Object choreObj = lua_getparam(1);

	const char *choreName = lua_getstring(choreObj);
	warning("Lua_V2::LockChoreSet: chore: %s", choreName);
}

void Lua_V2::UnlockChoreSet() {
	lua_Object choreObj = lua_getparam(1);

	const char *choreName = lua_getstring(choreObj);
	warning("Lua_V2::UnlockChoreSet: chore: %s", choreName);
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

void Lua_V2::IsActorChoring() {
	lua_Object actorObj = lua_getparam(1);
	bool excludeLoop = getbool(2);

	if (!lua_isuserdata(actorObj) || lua_tag(actorObj) != MKTAG('A','C','T','R'))
		return;

	Actor *actor = getactor(actorObj);
	const Common::List<Costume *> &costumes = actor->getCostumes();

	for (Common::List<Costume *>::const_iterator it = costumes.begin(); it != costumes.end(); ++it) {
		Costume *costume = *it;
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

void Lua_V2::IsChoreValid() {
	lua_Object choreObj = lua_getparam(1);

	if (!lua_isuserdata(choreObj) || lua_tag(choreObj) != MKTAG('C','H','O','R'))
		return;

	int chore = lua_getuserdata(choreObj);
	Chore *c = EMIChore::getPool().getObject(chore);

	if (c) {
		pushbool(c != nullptr);
	} else {
		lua_pushnil();
	}
}

void Lua_V2::IsChorePlaying() {
	lua_Object choreObj = lua_getparam(1);

	if (!lua_isuserdata(choreObj) || lua_tag(choreObj) != MKTAG('C','H','O','R'))
		return;

	int chore = lua_getuserdata(choreObj);
	Chore *c = EMIChore::getPool().getObject(chore);

	if (c) {
		pushbool(c->isPlaying() && !c->isPaused());
	} else {
		lua_pushnil();
	}
}

void Lua_V2::IsChoreLooping() {
	lua_Object choreObj = lua_getparam(1);

	if (!lua_isuserdata(choreObj) || lua_tag(choreObj) != MKTAG('C','H','O','R'))
		return;

	int chore = lua_getuserdata(choreObj);
	Chore *c = EMIChore::getPool().getObject(chore);

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
	Chore *c = EMIChore::getPool().getObject(chore);

	if (c) {
		c->setLooping(false);
	}
	lua_pushnil();
}

void Lua_V2::PlayChore() {
	lua_Object choreObj = lua_getparam(1);

	if (!lua_isuserdata(choreObj) || lua_tag(choreObj) != MKTAG('C','H','O','R'))
		return;
	int chore = lua_getuserdata(choreObj);

	Chore *c = EMIChore::getPool().getObject(chore);
	if (c) {
		c->setPaused(false);
	}
}

void Lua_V2::PauseChore() {
	lua_Object choreObj = lua_getparam(1);

	if (!lua_isuserdata(choreObj) || lua_tag(choreObj) != MKTAG('C','H','O','R'))
		return;
	int chore = lua_getuserdata(choreObj);

	Chore *c = EMIChore::getPool().getObject(chore);
	if (c) {
		c->setPaused(true);
	}
}

void Lua_V2::StopChore() {
	lua_Object choreObj = lua_getparam(1);
	lua_Object fadeTimeObj = lua_getparam(2);

	if (!lua_isuserdata(choreObj) || lua_tag(choreObj) != MKTAG('C','H','O','R'))
		return;

	int chore = lua_getuserdata(choreObj);
	float fadeTime = 0.0f;

	if (!lua_isnil(fadeTimeObj)) {
		if (lua_isnumber(fadeTimeObj))
			fadeTime = lua_getnumber(fadeTimeObj);
	}

	// There are a few cases in the scripts where StopChore is called with an excessively
	// large fadeTime value. The original engine ignores fade times greater or equal
	// to 0.6 seconds, so we replicate that behavior here.
	if (fadeTime >= 0.6f) {
		fadeTime = 0.0f;
	}

	Chore *c = EMIChore::getPool().getObject(chore);
	if (c) {
		c->stop((int)(fadeTime * 1000));
	}
}

void Lua_V2::AdvanceChore() {
	lua_Object choreObj = lua_getparam(1);
	lua_Object timeObj = lua_getparam(2);

	if (!lua_isuserdata(choreObj) || lua_tag(choreObj) != MKTAG('C','H','O','R') || !lua_isnumber(timeObj))
		return;

	int chore = lua_getuserdata(choreObj);
	float time = lua_getnumber(timeObj);
	Chore *c = EMIChore::getPool().getObject(chore);
	if (c) {
		if (!c->isPlaying()) {
			warning("AdvanceChore() called on stopped chore %s (%s)",
					c->getName(), c->getOwner()->getFilename().c_str());
			if (c->isLooping()) {
				c->getOwner()->playChoreLooping(c->getName());
			} else {
				c->getOwner()->playChore(c->getName());
			}
		}
		c->advance(time * 1000);
	}
}

// TODO: Implement, verify, and rename parameters
void Lua_V2::CompleteChore() {
	lua_Object param1 = lua_getparam(1);
	lua_Object param2 = lua_getparam(2);

	if (!lua_isuserdata(param1) || !lua_isnumber(param2))
		error("Lua_V2::CompleteChore - Unknown params");

	// Guesswork based on StopChore:
	int chore = lua_getuserdata(param1);
	float time = lua_getnumber(param2);

	error("Lua_V2::CompleteChore(%d, %f) - TODO: Implement opcode", chore, time);
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

	g_emi->invalidateSortOrder();
}

void Lua_V2::GetActorSortOrder() {
	lua_Object actorObj = lua_getparam(1);

	if (!lua_isuserdata(actorObj) || lua_tag(actorObj) != MKTAG('A','C','T','R'))
		return;

	Actor *actor = getactor(actorObj);
	lua_pushnumber(actor->getEffectiveSortOrder());
}

void Lua_V2::ActorActivateShadow() {
	lua_Object actorObj = lua_getparam(1);
	lua_Object activeObj = lua_getparam(2);
	lua_Object planeObj = lua_getparam(3);

	if (!lua_isuserdata(actorObj) || lua_tag(actorObj) != MKTAG('A','C','T','R'))
		return;

	Actor *actor = getactor(actorObj);
	if (!actor)
		return;
	bool active = (int)lua_getnumber(activeObj) == 1;
	const char *plane = nullptr;
	if (lua_isstring(planeObj))
		plane = lua_getstring(planeObj);
	actor->activateShadow(active, plane);
}

void Lua_V2::ActorStopMoving() {
	lua_Object actorObj = lua_getparam(1);

	if (!lua_isuserdata(actorObj) || lua_tag(actorObj) != MKTAG('A','C','T','R'))
		return;

	Actor *actor = getactor(actorObj);

	actor->stopWalking();
	actor->stopTurning();

	warning("Lua_V2::ActorStopMoving, actor: %s", actor->getName().c_str());
	// FIXME: Inspect the rest of the code to see if there's anything else missing
}

void Lua_V2::ActorLookAt() {
	lua_Object actorObj = lua_getparam(1);
	lua_Object xObj = lua_getparam(2);
	lua_Object yObj = lua_getparam(3);
	lua_Object zObj = lua_getparam(4);
	lua_Object rateObj = lua_getparam(5);

	if (!lua_isuserdata(actorObj) || lua_tag(actorObj) != MKTAG('A', 'C', 'T', 'R'))
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

		Math::Vector3d vector;
		vector.set(fX, fY, fZ);
		actor->setLookAtVector(vector);

		if (lua_isnumber(rateObj))
			actor->setLookAtRate(lua_getnumber(rateObj));
	} else if (lua_isuserdata(xObj) && lua_tag(xObj) == MKTAG('A', 'C', 'T', 'R')) { // look at another actor
		Actor *lookedAct = getactor(xObj);
		actor->setLookAtActor(lookedAct);

		if (lua_isnumber(yObj))
			actor->setLookAtRate(lua_getnumber(yObj));
	} else {
		return;
	}

	actor->setLooking(true);
}

void Lua_V2::GetActorWorldPos() {
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
	Costume *costume = nullptr;
	int chore = -1;

	if (!lua_isuserdata(actorObj) || lua_tag(actorObj) != MKTAG('A','C','T','R') ||
			(!lua_isstring(choreObj) && !lua_isnil(choreObj))) {
		return;
	}

	Actor *actor = getactor(actorObj);

	setChoreAndCostume(choreObj, costumeObj, actor, costume, chore);

	actor->setRestChore(chore, costume);
}

void Lua_V2::SetActorWalkChore() {
	lua_Object actorObj = lua_getparam(1);
	lua_Object choreObj = lua_getparam(2);
	lua_Object costumeObj = lua_getparam(3);
	Costume *costume = nullptr;
	int chore = -1;

	if (!lua_isuserdata(actorObj) || lua_tag(actorObj) != MKTAG('A','C','T','R') ||
			(!lua_isstring(choreObj) && !lua_isnil(choreObj))) {
		return;
	}

	Actor *actor = getactor(actorObj);

	setChoreAndCostume(choreObj, costumeObj, actor, costume, chore);

	actor->setWalkChore(chore, costume);
}

void Lua_V2::SetActorTurnChores() {
	lua_Object actorObj = lua_getparam(1);
	lua_Object leftChoreObj = lua_getparam(2);
	lua_Object rightChoreObj = lua_getparam(3);
	lua_Object costumeObj = lua_getparam(4);
	Costume *costume;

	if (!lua_isuserdata(actorObj) || lua_tag(actorObj) != MKTAG('A','C','T','R')) {
		return;
	} else if (!lua_isnil(leftChoreObj) && !lua_isstring(leftChoreObj)) {
		return;
	} else if (!lua_isnil(rightChoreObj) && !lua_isstring(rightChoreObj)) {
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
	Costume *costume = nullptr;
	int chore = -1;

	if (!lua_isuserdata(actorObj) || lua_tag(actorObj) != MKTAG('A','C','T','R') ||
			!lua_isnumber(indexObj) ||
			(!lua_isstring(choreObj) && !lua_isnil(choreObj))) {
		return;
	}

	int index = (int)lua_getnumber(indexObj);
	if (index < 0 || index >= 16)
		return;

	Actor *actor = getactor(actorObj);

	setChoreAndCostume(choreObj, costumeObj, actor, costume, chore);

	actor->setTalkChore(index + 1, chore, costume);
}

void Lua_V2::SetActorMumblechore() {
	lua_Object actorObj = lua_getparam(1);
	lua_Object choreObj = lua_getparam(2);
	lua_Object costumeObj = lua_getparam(3);
	Costume *costume = nullptr;
	int chore = -1;

	if (!lua_isuserdata(actorObj) || lua_tag(actorObj) != MKTAG('A','C','T','R') ||
			(!lua_isstring(choreObj) && !lua_isnil(choreObj))) {
		return;
	}

	Actor *actor = getactor(actorObj);

	setChoreAndCostume(choreObj, costumeObj, actor, costume, chore);

	actor->setMumbleChore(chore, costume);
}

void Lua_V2::GetActorChores() {
	lua_Object actorObj = lua_getparam(1);
	if (!lua_isuserdata(actorObj) || lua_tag(actorObj) != MKTAG('A','C','T','R'))
		return;
	Actor *actor = getactor(actorObj);
	const Common::List<Costume *> &costumes = actor->getCostumes();

	lua_Object result = lua_createtable();
	int count = 0;
	for (Common::List<Costume *>::const_iterator it = costumes.begin(); it != costumes.end(); ++it) {
		const Common::List<Chore *> &playingChores = (*it)->getPlayingChores();
		for (Common::List<Chore *>::const_iterator cit = playingChores.begin(); cit != playingChores.end(); ++cit) {
			lua_pushobject(result);
			lua_pushnumber(count++);
			lua_pushusertag(((EMIChore *)*cit)->getId(), MKTAG('C', 'H', 'O', 'R'));
			lua_settable();
		}
	}
	lua_pushobject(result);
	lua_pushstring("count");
	lua_pushnumber(count);
	lua_settable();

	lua_pushobject(result);
}

// Helper function, not called from LUA directly
bool Lua_V2::findCostume(lua_Object costumeObj, Actor *actor, Costume **costume) {
	*costume = nullptr;
	if (lua_isnil(costumeObj)) {
		*costume = actor->getCurrentCostume();
	} else {
		if (lua_isstring(costumeObj)) {
			const char *costumeName = lua_getstring(costumeObj);
			*costume = actor->findCostume(costumeName);
			if (*costume == nullptr) {
				actor->pushCostume(costumeName);
				*costume = actor->findCostume(costumeName);
			}
		}
	}
	return (*costume != nullptr);
}

void Lua_V2::PlayActorChore() {
	lua_Object actorObj = lua_getparam(1);
	lua_Object choreObj = lua_getparam(2);
	lua_Object costumeObj = lua_getparam(3);
	lua_Object modeObj = lua_getparam(4);
	lua_Object fadeTimeObj = lua_getparam(5);

	if (!lua_isuserdata(actorObj) || lua_tag(actorObj) != MKTAG('A','C','T','R'))
		return;

	Actor *actor = getactor(actorObj);

	if (!lua_isstring(choreObj) || !lua_isstring(costumeObj))
		lua_pushnil();

	bool mode = false;
	float fadeTime = 0.0f;

	if (!lua_isnil(modeObj)) {
		if (lua_getnumber(modeObj) != 0.0)
			mode = true;
	}

	if (!lua_isnil(fadeTimeObj)) {
		if (lua_isnumber(fadeTimeObj))
			fadeTime = lua_getnumber(fadeTimeObj);
	}

	const char *choreName = lua_getstring(choreObj);

	Costume *costume;
	if (!findCostume(costumeObj, actor, &costume))
		return;

	EMIChore *chore = (EMIChore *)costume->getChore(choreName);
	if (mode) {
		costume->playChoreLooping(choreName, (int)(fadeTime * 1000));
	} else {
		costume->playChore(choreName, (int)(fadeTime * 1000));
	}
	if (chore) {
		lua_pushusertag(chore->getId(), MKTAG('C','H','O','R'));
	} else {
		lua_pushnil();
	}

}

void Lua_V2::StopActorChores() {
	lua_Object actorObj = lua_getparam(1);
	// Guesswork for boolean parameter
	bool ignoreLoopingChores = getbool(2);

	if (!lua_isuserdata(actorObj) || lua_tag(actorObj) != MKTAG('A','C','T','R'))
		return;

	Actor *actor = getactor(actorObj);
	if (!actor)
		return;

	actor->stopAllChores(ignoreLoopingChores);
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
	actor->setLightMode((Actor::LightMode)lightMode);
}

void Lua_V2::SetActorCollisionMode() {
	lua_Object actorObj = lua_getparam(1);
	lua_Object modeObj = lua_getparam(2);

	if (!lua_isuserdata(actorObj) || lua_tag(actorObj) != MKTAG('A','C','T','R'))
		return;

	Actor *actor = getactor(actorObj);
	assert(actor);
	int mode = (int)lua_getnumber(modeObj);

	Actor::CollisionMode m;
	switch (mode) {
		case Actor::CollisionOff:
			m = Actor::CollisionOff;
			break;
		case Actor::CollisionBox:
			m = Actor::CollisionBox;
			break;
		case Actor::CollisionSphere:
			m = Actor::CollisionSphere;
			break;
		default:
			warning("Lua_V2::SetActorCollisionMode(): wrong collisionmode: %d, using default 0", mode);
			m = Actor::CollisionOff;
	}
	actor->setCollisionMode(m);
}

void Lua_V2::SetActorCollisionScale() {
	lua_Object actorObj = lua_getparam(1);
	lua_Object scaleObj = lua_getparam(2);

	if (!lua_isuserdata(actorObj) || lua_tag(actorObj) != MKTAG('A','C','T','R'))
		return;

	Actor *actor = getactor(actorObj);
	assert(actor);

	float scale = lua_getnumber(scaleObj);
	actor->setCollisionScale(scale);
}

void Lua_V2::GetActorPuckVector() {
	lua_Object actorObj = lua_getparam(1);
	lua_Object addObj = lua_getparam(2);

	if (!lua_isuserdata(actorObj) || lua_tag(actorObj) != MKTAG('A','C','T','R')) {
		lua_pushnil();
		return;
	}

	Actor *actor = getactor(actorObj);
	// Note: The wear chore of dumbshadow.cos is only started from Lua if
	// GetActorPuckVector returns a non-nil value. The original engine seems
	// to return nil for all actors that have never followed walkboxes.
	if (!actor || !actor->hasFollowedBoxes()) {
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
	lua_Object yawObj = lua_getparam(2);
	lua_Object maxPitchObj = lua_getparam(3);
	lua_Object minPitchObj = lua_getparam(4);

	if (!lua_isuserdata(actorObj) || lua_tag(actorObj) != MKTAG('A','C','T','R'))
		return;

	Actor *actor = getactor(actorObj);
	if (!actor)
		return;

	if (lua_isnumber(yawObj) && lua_isnumber(minPitchObj) && lua_isnumber(maxPitchObj)) {
		float yaw = lua_getnumber(yawObj);
		float maxPitch = lua_getnumber(maxPitchObj);
		float minPitch = lua_getnumber(minPitchObj);
		actor->setHeadLimits(yaw / 2, maxPitch, -minPitch);
	}
}

void Lua_V2::SetActorHead() {
	lua_Object actorObj = lua_getparam(1);
	lua_Object jointObj = lua_getparam(2);
	lua_Object xObj = lua_getparam(3);
	lua_Object yObj = lua_getparam(4);
	lua_Object zObj = lua_getparam(5);

	if (!lua_isuserdata(actorObj) || lua_tag(actorObj) != MKTAG('A', 'C', 'T', 'R'))
		return;

	Actor *actor = getactor(actorObj);
	if (!actor)
		return;

	if (lua_isstring(jointObj)) {
		const char *joint = lua_getstring(jointObj);
		Math::Vector3d offset;
		offset.x() = lua_getnumber(xObj);
		offset.y() = lua_getnumber(yObj);
		offset.z() = lua_getnumber(zObj);
		actor->setHead(joint, offset);
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

	if (!lua_isuserdata(actorObj) || lua_tag(actorObj) != MKTAG('A', 'C', 'T', 'R'))
		return;

	Actor *actor = getactor(actorObj);
	if (!actor)
		return;

	if (!lua_isuserdata(attachedObj) || lua_tag(attachedObj) != MKTAG('A', 'C', 'T', 'R'))
		return;

	Actor *attached = getactor(attachedObj);
	if (!attached)
		return;

	const char *joint = nullptr;
	if (!lua_isnil(jointObj)) {
		joint = lua_getstring(jointObj);
	}

	attached->attachToActor(actor, joint);
	Debug::debug(Debug::Actors | Debug::Scripts, "Lua_V2::AttachActor: attaching %s to %s (on %s)", attached->getName().c_str(), actor->getName().c_str(), joint ? joint : "(none)");

	g_emi->invalidateSortOrder();
}

void Lua_V2::DetachActor() {
	// Missing lua parts
	lua_Object attachedObj = lua_getparam(1);

	if (!lua_isuserdata(attachedObj) || lua_tag(attachedObj) != MKTAG('A','C','T','R'))
		return;

	Actor *attached = getactor(attachedObj);
	if (!attached)
		return;

	Debug::debug(Debug::Actors | Debug::Scripts, "Lua_V2::DetachActor: detaching %s from parent actor", attached->getName().c_str());
	attached->detach();

	g_emi->invalidateSortOrder();
}

void Lua_V2::WalkActorToAvoiding() {
	lua_Object actorObj = lua_getparam(1);
	lua_Object actor2Obj = lua_getparam(2);
	lua_Object xObj = lua_getparam(3);
	lua_Object yObj = lua_getparam(4);
	lua_Object zObj = lua_getparam(5);

	if (!lua_isuserdata(actorObj) || lua_tag(actorObj) != MKTAG('A','C','T','R'))
		return;

	if (!lua_isuserdata(actor2Obj) || lua_tag(actor2Obj) != MKTAG('A','C','T','R'))
		return;

	Math::Vector3d destVec;
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

	// TODO: Make this actually avoid the second actor

	actor->walkTo(destVec);
}

void Lua_V2::WalkActorVector() {
	lua_Object actorObj = lua_getparam(1);
	//	lua_Object xObj = lua_getparam(3);
	//	lua_Object yObj = lua_getparam(4);
	//	lua_Object zObj = lua_getparam(5);
	//	lua_Object param6Obj = lua_getparam(6);

	if (!lua_isuserdata(actorObj) || lua_tag(actorObj) != MKTAG('A', 'C', 'T', 'R'))
		return;

	//	Actor *actor = static_cast<Actor *>(lua_getuserdata(actorObj));
	Actor *actor2 = getactor(actorObj);

	// TODO whole below part need rewrote to much original
	float moveHoriz, moveVert;

	// Third option is the "left/right" movement
	moveHoriz = luaL_check_number(2);
	// Fourth Option is the "up/down" movement
	moveVert = luaL_check_number(4);

	// Get the direction the camera is pointing
	Set::Setup *setup = g_grim->getCurrSet()->getCurrSetup();
	Math::Vector3d cameraVector(0, 0, 1);

	setup->_rot.transform(&cameraVector, false);

	// find the angle the camera direction is around the unit circle
	Math::Angle cameraYaw = Math::Angle::arcTangent2(cameraVector.x(), cameraVector.z());

	// Handle the turning
	Math::Vector3d adjustVector(moveHoriz, 0, moveVert);
	// find the angle the adjust vector is around the unit circle
	Math::Angle adjustYaw = Math::Angle::arcTangent2(adjustVector.x(), adjustVector.z());

	Math::Angle yaw = cameraYaw + adjustYaw;

	// set the new direction or walk forward
	if (actor2->getYaw() != yaw)
		actor2->turnTo(0, yaw, 0, true);
	actor2->walkForward();
}

void Lua_V2::EnableActorPuck() {
	lua_Object actorObj = lua_getparam(1);

	if (!lua_isuserdata(actorObj) || lua_tag(actorObj) != MKTAG('A','C','T','R'))
		return;

	Actor *actor = getactor(actorObj);
	if (!actor)
		return;

	bool enable = getbool(2);

	// FIXME: Implement.
	warning("Lua_V2::EnableActorPuck: stub, actor: %s enable: %s", actor->getName().c_str(), enable ? "TRUE" : "FALSE");
}

// Helper function, not called from LUA directly
void Lua_V2::setChoreAndCostume(lua_Object choreObj, lua_Object costumeObj, Actor *actor, Costume *&costume, int &chore) {
	if (lua_isnil(choreObj)) {
		return;
	}

	if (!findCostume(costumeObj, actor, &costume))
		return;

	const char *choreStr = lua_getstring(choreObj);
	chore = costume->getChoreId(choreStr);
}

} // end of namespace Grim
