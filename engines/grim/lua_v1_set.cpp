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

#include "math/matrix3.h"

#include "engines/grim/debug.h"
#include "engines/grim/lua_v1.h"
#include "engines/grim/actor.h"
#include "engines/grim/grim.h"
#include "engines/grim/set.h"

#include "engines/grim/lua/lauxlib.h"

namespace Grim {

// Sector functions
/* Find the sector (of any type) which contains
 * the requested coordinate (x,y,z).
 */
void Lua_V1::GetPointSector() {
	lua_Object xObj = lua_getparam(1);
	lua_Object yObj = lua_getparam(2);
	lua_Object zObj = lua_getparam(3);
	lua_Object typeObj = lua_getparam(4);
	Sector::SectorType sectorType;

	if (!lua_isnumber(xObj) || !lua_isnumber(yObj) || !lua_isnumber(zObj)) {
		lua_pushnil();
		return;
	}
	if (lua_isnil(typeObj))
		sectorType = Sector::WalkType;
	else
		sectorType = (Sector::SectorType)(int)lua_getnumber(typeObj);

	float x = lua_getnumber(xObj);
	float y = lua_getnumber(yObj);
	float z = lua_getnumber(zObj);

	Math::Vector3d point(x, y, z);
	Sector *result = g_grim->getCurrSet()->findPointSector(point, sectorType);
	if (result) {
		lua_pushnumber(result->getSectorId());
		lua_pushstring(result->getName().c_str());
		lua_pushnumber(result->getType());
	} else {
		lua_pushnil();
	}
}

void Lua_V1::GetActorSector() {
	lua_Object actorObj = lua_getparam(1);
	lua_Object typeObj = lua_getparam(2);

	if (!lua_isuserdata(actorObj) || lua_tag(actorObj) != MKTAG('A','C','T','R'))
		return;
	if (!lua_isnumber(typeObj))
		return;

	Actor *actor = getactor(actorObj);
	Sector::SectorType sectorType = (Sector::SectorType)(int)lua_getnumber(typeObj);
	Math::Vector3d pos = actor->getWorldPos();
	Sector *result = g_grim->getCurrSet()->findPointSector(pos, sectorType);
	if (result) {
		lua_pushnumber(result->getSectorId());
		lua_pushstring(result->getName().c_str());
		lua_pushnumber(result->getType());
	} else {
		lua_pushnil();
	}
}

void Lua_V1::IsActorInSector() {
	lua_Object actorObj = lua_getparam(1);
	lua_Object nameObj = lua_getparam(2);
	if (!lua_isuserdata(actorObj) || lua_tag(actorObj) != MKTAG('A','C','T','R'))
		return;
	if (!lua_isstring(nameObj)) {
		lua_pushnil();
		return;
	}

	Actor *actor = getactor(actorObj);
	const char *name = lua_getstring(nameObj);

	Sector *sector;
	if (g_grim->getGameType() == GType_GRIM) {
		sector = g_grim->getCurrSet()->getSectorBySubstring(name, actor->getPos());
	} else {
		sector = g_grim->getCurrSet()->getSectorByName(name);
		if (!(sector && sector->isPointInSector(actor->getPos()))) {
			sector = nullptr;
		}
	}

	if (sector) {
		lua_pushnumber(sector->getSectorId());
		lua_pushstring(sector->getName().c_str());
		lua_pushnumber(sector->getType());
	} else {
		lua_pushnil();
	}
}

void Lua_V1::IsPointInSector() {
	lua_Object xObj = lua_getparam(1);
	lua_Object yObj = lua_getparam(2);
	lua_Object zObj = lua_getparam(3);
	lua_Object nameObj = lua_getparam(4);

	if (!lua_isstring(nameObj)) {
		lua_pushnil();
		return;
	}

	const char *name = lua_getstring(nameObj);
	float x = lua_getnumber(xObj);
	float y = lua_getnumber(yObj);
	float z = lua_getnumber(zObj);
	Math::Vector3d pos(x, y, z);

	Sector *sector = g_grim->getCurrSet()->getSectorBySubstring(name, pos);
	if (sector) {
		lua_pushnumber(sector->getSectorId());
		lua_pushstring(sector->getName().c_str());
		lua_pushnumber(sector->getType());
	} else {
		lua_pushnil();
	}
}

void Lua_V1::GetSectorOppositeEdge() {
	lua_Object actorObj = lua_getparam(1);
	lua_Object nameObj = lua_getparam(2);

	if (!lua_isuserdata(actorObj) || lua_tag(actorObj) != MKTAG('A','C','T','R'))
		return;

	if (!lua_isstring(nameObj)) {
		lua_pushnil();
		return;
	}

	Actor *actor = getactor(actorObj);
	const char *name = lua_getstring(nameObj);

	Sector *sector = g_grim->getCurrSet()->getSectorBySubstring(name);
	if (sector) {
		if (sector->getNumVertices() != 4)
			warning("GetSectorOppositeEdge(): cheat box with %d (!= 4) edges!", sector->getNumVertices());
		Math::Vector3d *vertices = sector->getVertices();
		Sector::ExitInfo e;

		sector->getExitInfo(actor->getPos(), -actor->getPuckVector(), &e);
		float frac = (e.exitPoint - vertices[e.edgeVertex + 1]).getMagnitude() / e.edgeDir.getMagnitude();
		e.edgeVertex -= 2;
		if (e.edgeVertex < 0)
			e.edgeVertex += sector->getNumVertices();
		Math::Vector3d edge = vertices[e.edgeVertex + 1] - vertices[e.edgeVertex];
		Math::Vector3d p = vertices[e.edgeVertex] + edge * frac;
		lua_pushnumber(p.x());
		lua_pushnumber(p.y());
		lua_pushnumber(p.z());
	} else {
		lua_pushnil();
	}
}

void Lua_V1::MakeSectorActive() {
	lua_Object sectorObj = lua_getparam(1);

	if (!lua_isnumber(sectorObj) && !lua_isstring(sectorObj))
		return;

	// FIXME: This happens on initial load. Are we initting something in the wrong order?
	if (!g_grim->getCurrSet()) {
		warning("!!!! Trying to call MakeSectorActive without a scene");
		return;
	}

	bool visible = !lua_isnil(lua_getparam(2));

	if (lua_isnumber(sectorObj)) {
		int numSectors = g_grim->getCurrSet()->getSectorCount();
		int id = (int)lua_getnumber(sectorObj);
		for (int i = 0; i < numSectors; i++) {
			Sector *sector = g_grim->getCurrSet()->getSectorBase(i);
			if (sector->getSectorId() == id) {
				sector->setVisible(visible);
				return;
			}
		}
	} else if (lua_isstring(sectorObj)) {
		const char *name = lua_getstring(sectorObj);
		// a search by name here is needed for set bv, since it calls MakeSectorActive with sectors
		// "bw_gone" and "bw_gone2", and a substring search would return "bw_gone2" for both.
		Sector *sector = g_grim->getCurrSet()->getSectorByName(name);
		if (sector) {
			sector->setVisible(visible);
		}
	}
}

// Set functions
void Lua_V1::LockSet() {
	lua_Object nameObj = lua_getparam(1);
	if (!lua_isstring(nameObj))
		return;

	const char *name = lua_getstring(nameObj);
	// TODO implement proper locking
	g_grim->setSetLock(name, true);
}

void Lua_V1::UnLockSet() {
	lua_Object nameObj = lua_getparam(1);
	if (!lua_isstring(nameObj))
		return;

	const char *name = lua_getstring(nameObj);
	// TODO implement proper unlocking
	g_grim->setSetLock(name, false);
}

void Lua_V1::MakeCurrentSet() {
	lua_Object nameObj = lua_getparam(1);
	if (!lua_isstring(nameObj)) {
		// TODO setting current set null
		warning("Lua_V1::MakeCurrentSet: implement missing case");
		return;
	}

	const char *name = lua_getstring(nameObj);
	Debug::debug(Debug::Engine, "Entered new scene '%s'.", name);
	g_grim->setSet(name);
}

void Lua_V1::MakeCurrentSetup() {
	lua_Object setupObj = lua_getparam(1);
	if (!lua_isnumber(setupObj))
		return;

	int num = (int)lua_getnumber(setupObj);
	g_grim->makeCurrentSetup(num);
}

/* Find the requested scene and return the current setup
 * id number.  This function cannot just use the current
 * scene or else when Manny opens his inventory information
 * gets lost, such as the position for the demon beavors
 * in the Petrified Forest.
 */
void Lua_V1::GetCurrentSetup() {
	lua_Object nameObj = lua_getparam(1);
	if (!lua_isstring(nameObj))
		return;

	const char *name = lua_getstring(nameObj);

	// FIXME there are some big difference here !
	Set *scene = g_grim->loadSet(name);
	if (!scene) {
		warning("GetCurrentSetup() Requested scene (%s) is not loaded", name);
		lua_pushnil();
		return;
	}
	lua_pushnumber(scene->getSetup());
}

void Lua_V1::PreviousSetup() {
	int num = g_grim->getCurrSet()->getSetup() - 1;
	if (num < 0) {
		num = g_grim->getCurrSet()->getNumSetups() - 1;
	}
	g_grim->makeCurrentSetup(num);
}

void Lua_V1::NextSetup() {
	int num = g_grim->getCurrSet()->getSetup() + 1;
	if (num >= g_grim->getCurrSet()->getNumSetups()) {
		num = 0;
	}
	g_grim->makeCurrentSetup(num);
}

/* This function makes the walkplane sectors smaller by the
 * given size. This is used when manny is holding some big
 * thing, like his scythe, that is likely to clip with the
 * things around him. The sectors are still connected, but they
 * have a margin which prevents manny to go too close to the
 * sectors edges.
 */
void Lua_V1::ShrinkBoxes() {
	lua_Object sizeObj = lua_getparam(1);

	if (lua_isnumber(sizeObj)) {
		float size = lua_getnumber(sizeObj);
		g_grim->getCurrSet()->shrinkBoxes(size);
	}
}

void Lua_V1::UnShrinkBoxes() {
	g_grim->getCurrSet()->unshrinkBoxes();
}

/* Given a position and a size this function calculates and pushes
 * the nearest point to that which will be valid if the boxes are
 * shrunk by the amount specified.
 */
void Lua_V1::GetShrinkPos() {
	lua_Object xObj = lua_getparam(1);
	lua_Object yObj = lua_getparam(2);
	lua_Object zObj = lua_getparam(3);
	lua_Object rObj = lua_getparam(4);

	if (!lua_isnumber(xObj) || !lua_isnumber(yObj) || !lua_isnumber(zObj) || !lua_isnumber(rObj))
		return;

	float x = lua_getnumber(xObj);
	float y = lua_getnumber(yObj);
	float z = lua_getnumber(zObj);
	float r = lua_getnumber(rObj);
	Math::Vector3d pos;
	pos.set(x, y, z);

	Sector *sector;
	g_grim->getCurrSet()->shrinkBoxes(r);
	g_grim->getCurrSet()->findClosestSector(pos, &sector, &pos);
	g_grim->getCurrSet()->unshrinkBoxes();

	if (sector) {
		lua_pushnumber(pos.x());
		lua_pushnumber(pos.y());
		lua_pushnumber(pos.z());
	} else {
		lua_pushnil();
	}
}

void Lua_V1::NewObjectState() {
	int setupID = (int)lua_getnumber(lua_getparam(1));
	int val = (int)lua_getnumber(lua_getparam(2));
	ObjectState::Position pos = (ObjectState::Position)val;
	const char *bitmap = lua_getstring(lua_getparam(3));
	const char *zbitmap = nullptr;
	if (!lua_isnil(lua_getparam(4)))
		zbitmap = lua_getstring(lua_getparam(4));
	bool transparency = getbool(5);

	ObjectState *state = g_grim->getCurrSet()->addObjectState(setupID, pos, bitmap, zbitmap, transparency);
	lua_pushusertag(state->getId(), MKTAG('S','T','A','T'));
}

void Lua_V1::FreeObjectState() {
	lua_Object param = lua_getparam(1);
	if (!lua_isuserdata(param) || lua_tag(param) != MKTAG('S','T','A','T'))
		return;
	ObjectState *state = getobjectstate(param);
	g_grim->getCurrSet()->deleteObjectState(state);
	delete state;
}

void Lua_V1::SendObjectToBack() {
	lua_Object param = lua_getparam(1);
	if (lua_isuserdata(param) && lua_tag(param) == MKTAG('S','T','A','T')) {
		ObjectState *state =  getobjectstate(param);
		g_grim->getCurrSet()->moveObjectStateToBack(state);
	}
}

void Lua_V1::SendObjectToFront() {
	lua_Object param = lua_getparam(1);
	if (lua_isuserdata(param) && lua_tag(param) == MKTAG('S','T','A','T')) {
		ObjectState *state =  getobjectstate(param);
		g_grim->getCurrSet()->moveObjectStateToFront(state);
	}
}

void Lua_V1::SetObjectType() {
	lua_Object param = lua_getparam(1);
	if (!lua_isuserdata(param) || lua_tag(param) != MKTAG('S','T','A','T'))
		return;
	ObjectState *state =  getobjectstate(param);
	int val = (int)lua_getnumber(lua_getparam(2));
	ObjectState::Position pos = (ObjectState::Position)val;
	state->setPos(pos);
}

void Lua_V1::SetAmbientLight() {
	int mode = (int)lua_getnumber(lua_getparam(1));
	Set *set = g_grim->getCurrSet();

	if (set == nullptr)
		return;

	if (mode == 0) {
		set->setLightEnableState(true);
	} else if (mode == 1) {
		set->setLightEnableState(false);
	}
}

void Lua_V1::SetLightIntensity() {
	lua_Object lightObj = lua_getparam(1);
	lua_Object intensityObj = lua_getparam(2);

	if (!lua_isnumber(intensityObj))
		return;

	float intensity = lua_getnumber(intensityObj);

	if (lua_isnumber(lightObj)) {
		int light = (int)lua_getnumber(lightObj);
		g_grim->getCurrSet()->setLightIntensity(light, intensity);
	} else if (lua_isstring(lightObj)) {
		const char *light = lua_getstring(lightObj);
		g_grim->getCurrSet()->setLightIntensity(light, intensity);
	}
}

void Lua_V1::SetLightPosition() {
	lua_Object lightObj = lua_getparam(1);
	lua_Object xObj = lua_getparam(2);
	lua_Object yObj = lua_getparam(3);
	lua_Object zObj = lua_getparam(4);

	if (!lua_isnumber(xObj) || !lua_isnumber(yObj) || !lua_isnumber(zObj))
		return;

	float x = lua_getnumber(xObj);
	float y = lua_getnumber(yObj);
	float z = lua_getnumber(zObj);
	Math::Vector3d vec(x, y, z);

	if (lua_isnumber(lightObj)) {
		int light = (int)lua_getnumber(lightObj);
		g_grim->getCurrSet()->setLightPosition(light, vec);
	} else if (lua_isstring(lightObj)) {
		const char *light = lua_getstring(lightObj);
		g_grim->getCurrSet()->setLightPosition(light, vec);
	}
}

void Lua_V1::TurnLightOn() {
	lua_Object lightObj = lua_getparam(1);

	Set *scene = g_grim->getCurrSet();
	bool isOn = getbool(2);
	if (lua_isnumber(lightObj)) {
		scene->setLightEnabled((int)lua_getnumber(lightObj), isOn);
	} else if (lua_isstring(lightObj)) {
		scene->setLightEnabled(lua_getstring(lightObj), isOn);
	}
}

} // end of namespace Grim
