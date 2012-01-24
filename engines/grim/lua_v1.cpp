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

#define FORBIDDEN_SYMBOL_EXCEPTION_printf

#include "common/endian.h"
#include "common/system.h"

#include "math/matrix3.h"

#include "engines/grim/debug.h"
#include "engines/grim/lua_v1.h"
#include "engines/grim/actor.h"
#include "engines/grim/lipsync.h"
#include "engines/grim/costume.h"
#include "engines/grim/registry.h"
#include "engines/grim/localize.h"
#include "engines/grim/colormap.h"
#include "engines/grim/grim.h"
#include "engines/grim/savegame.h"
#include "engines/grim/resource.h"
#include "engines/grim/bitmap.h"
#include "engines/grim/font.h"
#include "engines/grim/set.h"
#include "engines/grim/gfx_base.h"
#include "engines/grim/model.h"
#include "engines/grim/primitives.h"

#include "engines/grim/lua/lauxlib.h"
#include "engines/grim/lua/luadebug.h"

// Windows.h badness: Remove #defines to the following Win32 API MultiByte/Unicode functions.
#ifdef GetDiskFreeSpace
#undef GetDiskFreeSpace
#endif

#ifdef PlaySound
#undef PlaySound
#endif

namespace Grim {

#define strmatch(src, dst)		(strlen(src) == strlen(dst) && strcmp(src, dst) == 0)

byte clamp_color(int c) {
	if (c < 0)
		return 0;
	else if (c > 255)
		return 255;
	else
		return c;
}

int luaA_passresults();

void Lua_V1::new_dofile() {
	const char *fname_str = luaL_check_string(1);
	if (dofile(fname_str) == 0)
		if (luaA_passresults() == 0)
			lua_pushuserdata(0);
}

// Debugging message functions

void Lua_V1::PrintDebug() {
	if (Debug::isChannelEnabled(Debug::Scripts | Debug::Info)) {
		Common::String msg("Debug: ");
		lua_Object strObj = lua_getparam(1);
		if (lua_isnil(strObj))
			msg += "(nil)";
		if (!lua_isstring(strObj))
			return;
		msg += Common::String(lua_getstring(strObj));
		debug(msg.c_str());
	}
}

void Lua_V1::PrintError() {
	if (Debug::isChannelEnabled(Debug::Scripts | Debug::Info)) {
		Common::String msg("Error: ");
		lua_Object strObj = lua_getparam(1);
		if (lua_isnil(strObj))
			msg += "(nil)";
		if (!lua_isstring(strObj))
			return;
		msg += Common::String(lua_getstring(strObj));
		debug(msg.c_str());
	}
}

void Lua_V1::PrintWarning() {
	if (Debug::isChannelEnabled(Debug::Scripts | Debug::Info)) {
		Common::String msg("Warning: ");
		lua_Object strObj = lua_getparam(1);
		if (lua_isnil(strObj))
			msg += "(nil)";
		if (!lua_isstring(strObj))
			return;
		msg += Common::String(lua_getstring(strObj));
		debug(msg.c_str());
	}
}

void Lua_V1::FunctionName() {
	const char *name;
	char buf[256];
	const char *filename = 0;
	int32 line;
	lua_Object param1 = lua_getparam(1);

	if (!lua_isfunction(param1)) {
		sprintf(buf, "function InvalidArgsToFunctionName");
		lua_pushstring(buf);
		return;
	}

	lua_funcinfo(param1, &filename, &line);
	switch (*lua_getobjname(param1, &name)) {
	case 'g':
		sprintf(buf, "function %.100s", name);
		break;
	case 't':
		sprintf(buf, "`%.100s' tag method", name);
		break;
	default:
		{
//             cout<<(void*)filename<<endl;
			if (line == 0)
				sprintf(buf, "main of %.100s", filename);
			else if (line < 0)
				sprintf(buf, "%.100s", filename);
			else {
				sprintf(buf, "function (%.100s:%d)", filename, (int)line);
				filename = NULL;
			}
		}
	}
	int curr_line = lua_currentline(param1);
	if (curr_line > 0)
		sprintf(buf + strlen(buf), " at line %d", curr_line);
	if (filename)
		sprintf(buf + strlen(buf), " [in file %.100s]", filename);
	lua_pushstring(buf);
}

void Lua_V1::CheckForFile() {
	lua_Object strObj = lua_getparam(1);

	if (!lua_isstring(strObj))
		return;

	const char *filename = lua_getstring(strObj);
	pushbool(g_resourceloader->getFileExists(filename));
}

void Lua_V1::MakeColor() {
	lua_Object rObj = lua_getparam(1);
	lua_Object gObj = lua_getparam(2);
	lua_Object bObj = lua_getparam(3);
	int r, g, b;

	if (!lua_isnumber(rObj))
		r = 0;
	else
		r = clamp_color((int)lua_getnumber(rObj));

	if (!lua_isnumber(gObj))
		g = 0;
	else
		g = clamp_color((int)lua_getnumber(gObj));

	if (!lua_isnumber(bObj))
		b = 0;
	else
		b = clamp_color((int)lua_getnumber(bObj));

	PoolColor *c = new PoolColor (r, g ,b);
	lua_pushusertag(c->getId(), MKTAG('C','O','L','R'));
}

void Lua_V1::GetColorComponents() {
	lua_Object colorObj = lua_getparam(1);
	Color *c = getcolor(colorObj);
	lua_pushnumber(c->getRed());
	lua_pushnumber(c->getGreen());
	lua_pushnumber(c->getBlue());
}

void Lua_V1::ReadRegistryValue() {
	lua_Object keyObj = lua_getparam(1);

	if (!lua_isstring(keyObj)) {
		lua_pushnil();
		return;
	}
	const char *key = lua_getstring(keyObj);
	const char *val = g_registry->get(key, "");
	// this opcode can return lua_pushnumber due binary nature of some registry entries, but not implemented
	if (val[0] == 0)
		lua_pushnil();
	else
		lua_pushstring(const_cast<char *>(val));
}

void Lua_V1::WriteRegistryValue() {
	lua_Object keyObj = lua_getparam(1);
	lua_Object valObj = lua_getparam(2);

	if (!lua_isstring(keyObj))
		return;

	if (!lua_isstring(valObj))
		return;

	// this opcode can get lua_getnumber due binary nature of some registry entries, but not implemented
	const char *key = lua_getstring(keyObj);
	const char *val = lua_getstring(valObj);
	g_registry->set(key, val);
}

void Lua_V1::GetAngleBetweenVectors() {
	lua_Object vec1Obj = lua_getparam(1);
	lua_Object vec2Obj = lua_getparam(2);

	if (!lua_istable(vec1Obj) || !lua_istable(vec2Obj)) {
		lua_pushnil();
		return;
	}

	lua_pushobject(vec1Obj);
	lua_pushstring("x");
	lua_Object table = lua_gettable();
	float x1 = lua_getnumber(table);
	lua_pushobject(vec1Obj);
	lua_pushstring("y");
	table = lua_gettable();
	float y1 = lua_getnumber(table);
	lua_pushobject(vec1Obj);
	lua_pushstring("z");
	table = lua_gettable();
	float z1 = lua_getnumber(table);
	lua_pushobject(vec2Obj);
	lua_pushstring("x");
	table = lua_gettable();
	float x2 = lua_getnumber(table);
	lua_pushobject(vec2Obj);
	lua_pushstring("y");
	table = lua_gettable();
	float y2 = lua_getnumber(table);
	lua_pushobject(vec2Obj);
	lua_pushstring("z");
	table = lua_gettable();
	float z2 = lua_getnumber(table);

	Math::Vector3d vec1(x1, y1, z1);
	Math::Vector3d vec2(x2, y2, z2);
	vec1.normalize();
	vec2.normalize();

	float dot = vec1.getDotProduct(vec2);
	float angle = 90.0f - (180.0f * asin(dot)) / LOCAL_PI;
	if (angle < 0)
		angle = -angle;
	lua_pushnumber(angle);
}

void Lua_V1::Is3DHardwareEnabled() {
	pushbool(g_driver->isHardwareAccelerated());
}

void Lua_V1::SetHardwareState() {
	// changing only in config setup (software/hardware rendering)
	bool accel = getbool(1);
	if (accel)
		g_registry->set("soft_renderer", "false");
	else
		g_registry->set("soft_renderer", "true");
	g_grim->changeHardwareState();
}

void Lua_V1::SetVideoDevices() {
	int devId;
	int modeId;

	devId = (int)lua_getnumber(lua_getparam(1));
	modeId = (int)lua_getnumber(lua_getparam(2));
	// ignore setting video devices
}

void Lua_V1::GetVideoDevices() {
	lua_pushnumber(0.0);
	lua_pushnumber(-1.0);
}

void Lua_V1::EnumerateVideoDevices() {
	lua_Object result = lua_createtable();
	lua_pushobject(result);
	lua_pushnumber(0.0); // id of device
	lua_pushstring(g_driver->getVideoDeviceName()); // name of device
	lua_settable();
	lua_pushobject(result);
}

void Lua_V1::Enumerate3DDevices() {
	lua_Object result = lua_createtable();
	lua_Object numObj = lua_getparam(1);
	if (!lua_isnumber(numObj))
		return;
/*	int num = (int)lua_getnumber(numObj);*/
	lua_pushobject(result);
	lua_pushnumber(-1.0);
	if (g_driver->isHardwareAccelerated()) {
		lua_pushstring("OpenGL"); // type of 3d renderer
	} else {
		lua_pushstring("/engn003/Software"); // type of 3d renderer
	}
	lua_settable();
	lua_pushobject(result);
}

/* RotateVector takes a vector and rotates it around
 * the point (0,0,0) by the requested number of degrees.
 * This function is used to calculate the locations for
 * getting on and off of the Bone Wagon and for going up
 * and down the slide with the chain at the end of the world.
 */
void Lua_V1::RotateVector() {
	lua_Object vecObj = lua_getparam(1);
	lua_Object rotObj = lua_getparam(2);

	if (!lua_istable(vecObj) || !lua_istable(rotObj)) {
		lua_pushnil();
		return;
	}

	lua_pushobject(vecObj);
	lua_pushstring("x");
	float x = lua_getnumber(lua_gettable());
	lua_pushobject(vecObj);
	lua_pushstring("y");
	float y = lua_getnumber(lua_gettable());
	lua_pushobject(vecObj);
	lua_pushstring("z");
	float z = lua_getnumber(lua_gettable());
	Math::Vector3d vec(x, y, z);

	lua_pushobject(rotObj);
	lua_pushstring("x");
	Math::Angle pitch = lua_getnumber(lua_gettable());
	lua_pushobject(rotObj);
	lua_pushstring("y");
	Math::Angle yaw = lua_getnumber(lua_gettable());
	lua_pushobject(rotObj);
	lua_pushstring("z");
	Math::Angle roll = lua_getnumber(lua_gettable());

	Math::Matrix3 mat;
	mat.buildFromPitchYawRoll(pitch, yaw, roll);
	mat.transformVector(&vec);

	lua_Object resObj = lua_createtable();
	lua_pushobject(resObj);
	lua_pushstring("x");
	lua_pushnumber(vec.x());
	lua_settable();
	lua_pushobject(resObj);
	lua_pushstring("y");
	lua_pushnumber(vec.y());
	lua_settable();
	lua_pushobject(resObj);
	lua_pushstring("z");
	lua_pushnumber(vec.z());
	lua_settable();

	lua_pushobject(resObj);
}

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
		lua_pushstring(const_cast<char *>(result->getName()));
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
	Math::Vector3d pos = actor->getPos();
	Sector *result = g_grim->getCurrSet()->findPointSector(pos, sectorType);
	if (result) {
		lua_pushnumber(result->getSectorId());
		lua_pushstring(const_cast<char *>(result->getName()));
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

	int numSectors = g_grim->getCurrSet()->getSectorCount();
	for (int i = 0; i < numSectors; i++) {
		Sector *sector = g_grim->getCurrSet()->getSectorBase(i);
		if (strstr(sector->getName(), name)) {
			if (sector->isPointInSector(actor->getPos())) {
				lua_pushnumber(sector->getSectorId());
				lua_pushstring(sector->getName());
				lua_pushnumber(sector->getType());
				return;
			}
		}
	}
	lua_pushnil();
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

	int numSectors = g_grim->getCurrSet()->getSectorCount();
	for (int i = 0; i < numSectors; i++) {
		Sector *sector = g_grim->getCurrSet()->getSectorBase(i);
		if (strstr(sector->getName(), name)) {
			if (sector->isPointInSector(pos)) {
				lua_pushnumber(sector->getSectorId());
				lua_pushstring(sector->getName());
				lua_pushnumber(sector->getType());
				return;
			}
		}
	}
	lua_pushnil();
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

	int numSectors = g_grim->getCurrSet()->getSectorCount();
	for (int i = 0; i < numSectors; i++) {
		Sector *sector = g_grim->getCurrSet()->getSectorBase(i);
		if (strmatch(sector->getName(), name)) {
			if (sector->getNumVertices() != 4)
				warning("GetSectorOppositeEdge(): cheat box with %d (!= 4) edges!", sector->getNumVertices());
			Math::Vector3d* vertices = sector->getVertices();
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

			return;
		}
	}

	lua_pushnil();
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
	int numSectors = g_grim->getCurrSet()->getSectorCount();
	if (lua_isstring(sectorObj)) {
		const char *name = lua_getstring(sectorObj);
		for (int i = 0; i < numSectors; i++) {
			Sector *sector = g_grim->getCurrSet()->getSectorBase(i);
			if (strmatch(sector->getName(), name)) {
				sector->setVisible(visible);
				return;
			}
		}
	} else if (lua_isnumber(sectorObj)) {
		int id = (int)lua_getnumber(sectorObj);
		for (int i = 0; i < numSectors; i++) {
			Sector *sector = g_grim->getCurrSet()->getSectorBase(i);
			if (sector->getSectorId() == id) {
				sector->setVisible(visible);
				return;
			}
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

	Sector* sector;
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

void Lua_V1::FileFindDispose() {
	g_grim->_listFiles.clear();
	g_grim->_listFilesIter = NULL;
}

void Lua_V1::FileFindNext() {
	if (g_grim->_listFilesIter == g_grim->_listFiles.end()) {
		lua_pushnil();
		Lua_V1::FileFindDispose();
	} else {
		lua_pushstring(g_grim->_listFilesIter->c_str());
		g_grim->_listFilesIter++;
	}
}

void Lua_V1::FileFindFirst() {
	lua_Object extObj = lua_getparam(1);
	if (!lua_isstring(extObj)) {
		lua_pushnil();
		return;
	}

	Lua_V1::FileFindDispose();

	const char *extension = lua_getstring(extObj);
	Common::SaveFileManager *saveFileMan = g_system->getSavefileManager();
	g_grim->_listFiles = saveFileMan->listSavefiles(extension);
	g_grim->_listFilesIter = g_grim->_listFiles.begin();

	if (g_grim->_listFilesIter == g_grim->_listFiles.end())
		lua_pushnil();
	else
		Lua_V1::FileFindNext();
}

void Lua_V1::PerSecond() {
	lua_Object rateObj = lua_getparam(1);

	if (!lua_isnumber(rateObj)) {
		lua_pushnil();
		return;
	}
	float rate = lua_getnumber(rateObj);
	lua_pushnumber(g_grim->getPerSecond(rate));
}

void Lua_V1::EnableControl() {
	lua_Object numObj = lua_getparam(1);

	if (!lua_isnumber(numObj)) {
		lua_pushnil();
		return;
	}
	int num = (int)lua_getnumber(numObj);
	if (num < 0 || num >= KEYCODE_EXTRA_LAST)
		error("control identifier out of range");

	g_grim->enableControl(num);
}

void Lua_V1::DisableControl() {
	lua_Object numObj = lua_getparam(1);

	if (!lua_isnumber(numObj)) {
		lua_pushnil();
		return;
	}
	int num = (int)lua_getnumber(numObj);
	if (num < 0 || num >= KEYCODE_EXTRA_LAST)
		error("control identifier out of range");

	g_grim->disableControl(num);
}

void Lua_V1::GetControlState() {
	lua_Object numObj = lua_getparam(1);

	if (!lua_isnumber(numObj))
		return;

	int num = (int)lua_getnumber(numObj);
	if (num < 0 || num >= KEYCODE_EXTRA_LAST)
		error("control identifier out of range");
	if (num >= KEYCODE_AXIS_JOY1_X && num <= KEYCODE_AXIS_MOUSE_Z)
		lua_pushnumber(g_grim->getControlAxis(num));
	else {
		pushbool(g_grim->getControlState(num)); // key down, originaly it push number if key down
		//pushnil or number, what is is ?
	}
}

void Lua_V1::Exit() {
	g_grim->quitGame();
}

void Lua_V1::SetSpeechMode() {
	GrimEngine::SpeechMode mode = (GrimEngine::SpeechMode)((int)lua_getnumber(lua_getparam(1)));
	if (mode >= 1 && mode <= 3)
		g_grim->setSpeechMode(mode);
}

void Lua_V1::GetSpeechMode() {
	lua_pushnumber(g_grim->getSpeechMode());
}

void Lua_V1::GetDiskFreeSpace() {
	// amount of free space in MB, used for creating saves
	lua_pushnumber(50);
}

void Lua_V1::NewObjectState() {
	int setupID = (int)lua_getnumber(lua_getparam(1));
	int val = (int)lua_getnumber(lua_getparam(2));
	ObjectState::Position pos = (ObjectState::Position)val;
	const char *bitmap = lua_getstring(lua_getparam(3));
	const char *zbitmap = NULL;
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

void Lua_V1::GetCurrentScript() {
	current_script();
}

void Lua_V1::GetSaveGameImage() {
	int width = 250, height = 188;
	Bitmap *screenshot;
	int dataSize;

	lua_Object param = lua_getparam(1);
	if (!lua_isstring(param)) {
		lua_pushnil();
		return;
	}
	const char *filename = lua_getstring(param);
	SaveGame *savedState = SaveGame::openForLoading(filename);
	if (!savedState || savedState->saveVersion() != SaveGame::SAVEGAME_VERSION) {
		lua_pushnil();
		return;
	}
	dataSize = savedState->beginSection('SIMG');
	uint16 *data = new uint16[dataSize / 2];
	for (int l = 0; l < dataSize / 2; l++) {
		data[l] = savedState->readLEUint16();
	}
	screenshot = new Bitmap((char *)data, width, height, 16, "screenshot");
	delete[] data;
	if (screenshot) {
		lua_pushusertag(screenshot->getId(), MKTAG('V','B','U','F'));
	} else {
		lua_pushnil();
		warning("Could not restore screenshot from file");
		delete savedState;
		return;
	}
	savedState->endSection();
	delete savedState;
}

void Lua_V1::SubmitSaveGameData() {
	lua_Object table, table2;
	SaveGame *savedState;
	const char *str;
	table = lua_getparam(1);

	savedState = g_grim->savedState();
	if (!savedState)
		error("Cannot obtain saved game");
	savedState->beginSection('SUBS');
	int count = 0;
	for (;;) {
		lua_pushobject(table);
		lua_pushnumber(count);
		count++;
		table2 = lua_gettable();
		if (lua_isnil(table2))
			break;
		str = lua_getstring(table2);
		int32 len = strlen(str) + 1;
		savedState->writeLESint32(len);
		savedState->write(str, len);
	}
	savedState->endSection();
}

void Lua_V1::GetSaveGameData() {
	lua_Object param = lua_getparam(1);
	if (!lua_isstring(param))
		return;
	const char *filename = lua_getstring(param);
	SaveGame *savedState = SaveGame::openForLoading(filename);
	lua_Object result = lua_createtable();

	if (!savedState || savedState->saveVersion() != SaveGame::SAVEGAME_VERSION) {
		lua_pushobject(result);
		lua_pushnumber(2);
		lua_pushstring("mo.set"); // Just a placeholder to not make it throw a lua error
		lua_settable();
		lua_pushobject(result);

		if (!savedState) {
			warning("Savegame %s is invalid", filename);
		} else {
			warning("Savegame %s is incompatible with this Residual build. Save version: %d; current version: %d", filename, savedState->saveVersion(), SaveGame::SAVEGAME_VERSION);
		}
		delete savedState;
		return;
	}
	int32 dataSize = savedState->beginSection('SUBS');

	char str[200];
	int32 strSize;
	int count = 0;

	for (;;) {
		if (dataSize <= 0)
			break;
		strSize = savedState->readLESint32();
		savedState->read(str, strSize);
		lua_pushobject(result);
		lua_pushnumber(count);
		lua_pushstring(str);
		lua_settable();
		dataSize -= strSize;
		dataSize -= 4;
		count++;
	}
	lua_pushobject(result);

	savedState->endSection();
	delete savedState;
}

void Lua_V1::Load() {
	lua_Object fileName = lua_getparam(1);
	if (lua_isnil(fileName)) {
		g_grim->loadGame("");
	} else if (lua_isstring(fileName)) {
		g_grim->loadGame(lua_getstring(fileName));
	} else {
		warning("Load() fileName is wrong");
		return;
	}
}

void Lua_V1::Save() {
	lua_Object fileName = lua_getparam(1);
	if (lua_isnil(fileName)) {
		g_grim->saveGame("");
	} else if (lua_isstring(fileName)) {
		g_grim->saveGame(lua_getstring(fileName));
	} else {
		warning("Save() fileName is wrong");
		return;
	}
}

void Lua_V1::Remove() {
	if (g_system->getSavefileManager()->removeSavefile(luaL_check_string(1)))
		lua_pushuserdata(0);
	else {
		lua_pushnil();
		lua_pushstring(g_system->getSavefileManager()->getErrorDesc().c_str());
	}
}

void Lua_V1::LockFont() {
	lua_Object param1 = lua_getparam(1);
	if (lua_isstring(param1)) {
		const char *fontName = lua_getstring(param1);
		Font *result = g_resourceloader->loadFont(fontName);
		if (result) {
			lua_pushusertag(result->getId(), MKTAG('F','O','N','T'));
			return;
		}
	}

	lua_pushnil();
}

void Lua_V1::EnableDebugKeys() {
}

void Lua_V1::LightMgrSetChange() {
	// that seems only used when some control panel is opened
}

void Lua_V1::SetAmbientLight() {
	int mode = (int)lua_getnumber(lua_getparam(1));
	if (mode == 0) {
		if (g_grim->getCurrSet()) {
			g_grim->getCurrSet()->setLightEnableState(true);
		}
	} else if (mode == 1) {
		if (g_grim->getCurrSet()) {
			g_grim->getCurrSet()->setLightEnableState(false);
		}
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

void Lua_V1::LightMgrStartup() {
	// we will not implement this opcode
}

void Lua_V1::JustLoaded() {
	Debug::error("OPCODE USAGE VERIFICATION: JustLoaded");
}

void Lua_V1::SetEmergencyFont() {
	Debug::error("OPCODE USAGE VERIFICATION: SetEmergencyFont");
}


// Stub function for builtin functions not yet implemented
static void stubWarning(const char *funcName) {
	warning("Stub function: %s", funcName);
}

#define STUB_FUNC(name) void name() { stubWarning(#name); }

STUB_FUNC(Lua_V1::SetActorInvClipNode)
STUB_FUNC(Lua_V1::NukeResources)
STUB_FUNC(Lua_V1::ResetTextures)
STUB_FUNC(Lua_V1::AttachToResources)
STUB_FUNC(Lua_V1::DetachFromResources)
STUB_FUNC(Lua_V1::SetActorClipPlane)
STUB_FUNC(Lua_V1::SetActorClipActive)
STUB_FUNC(Lua_V1::FlushControls)
STUB_FUNC(Lua_V1::GetCameraLookVector)
STUB_FUNC(Lua_V1::SetCameraRoll)
STUB_FUNC(Lua_V1::SetCameraInterest)
STUB_FUNC(Lua_V1::GetCameraPosition)
STUB_FUNC(Lua_V1::SpewStartup)
STUB_FUNC(Lua_V1::PreviousSetup)
STUB_FUNC(Lua_V1::NextSetup)
STUB_FUNC(Lua_V1::WorldToScreen)
STUB_FUNC(Lua_V1::SetActorRoll)
STUB_FUNC(Lua_V1::SetActorFrustrumCull)
STUB_FUNC(Lua_V1::DriveActorTo)
STUB_FUNC(Lua_V1::GetActorRect)
STUB_FUNC(Lua_V1::GetTranslationMode)
STUB_FUNC(Lua_V1::SetTranslationMode)
STUB_FUNC(Lua_V1::WalkActorToAvoiding)
STUB_FUNC(Lua_V1::GetActorChores)
STUB_FUNC(Lua_V1::SetCameraPosition)
STUB_FUNC(Lua_V1::GetCameraFOV)
STUB_FUNC(Lua_V1::SetCameraFOV)
STUB_FUNC(Lua_V1::GetCameraRoll)
STUB_FUNC(Lua_V1::GetMemoryUsage)
STUB_FUNC(Lua_V1::GetFontDimensions)
STUB_FUNC(Lua_V1::PurgeText)

struct luaL_reg grimMainOpcodes[] = {
	{ "EngineDisplay", LUA_OPCODE(Lua_V1, EngineDisplay) },
	{ "CheckForFile", LUA_OPCODE(Lua_V1, CheckForFile) },
	{ "Load", LUA_OPCODE(Lua_V1, Load) },
	{ "Save", LUA_OPCODE(Lua_V1, Save) },
	{ "remove", LUA_OPCODE(Lua_V1, Remove) },
	{ "SetActorColormap", LUA_OPCODE(Lua_V1, SetActorColormap) },
	{ "GetActorCostume", LUA_OPCODE(Lua_V1, GetActorCostume) },
	{ "SetActorCostume", LUA_OPCODE(Lua_V1, SetActorCostume) },
	{ "SetActorScale", LUA_OPCODE(Lua_V1, SetActorScale) },
	{ "GetActorTimeScale", LUA_OPCODE(Lua_V1, GetActorTimeScale) },
	{ "SetActorTimeScale", LUA_OPCODE(Lua_V1, SetActorTimeScale) },
	{ "GetActorNodeLocation", LUA_OPCODE(Lua_V1, GetActorNodeLocation) },
	{ "SetActorWalkChore", LUA_OPCODE(Lua_V1, SetActorWalkChore) },
	{ "SetActorTurnChores", LUA_OPCODE(Lua_V1, SetActorTurnChores) },
	{ "SetActorRestChore", LUA_OPCODE(Lua_V1, SetActorRestChore) },
	{ "SetActorMumblechore", LUA_OPCODE(Lua_V1, SetActorMumblechore) },
	{ "SetActorTalkChore", LUA_OPCODE(Lua_V1, SetActorTalkChore) },
	{ "SetActorWalkRate", LUA_OPCODE(Lua_V1, SetActorWalkRate) },
	{ "GetActorWalkRate", LUA_OPCODE(Lua_V1, GetActorWalkRate) },
	{ "SetActorTurnRate", LUA_OPCODE(Lua_V1, SetActorTurnRate) },
	{ "SetSelectedActor", LUA_OPCODE(Lua_V1, SetSelectedActor) },
	{ "LoadActor", LUA_OPCODE(Lua_V1, LoadActor) },
	{ "GetActorPos", LUA_OPCODE(Lua_V1, GetActorPos) },
	{ "GetActorRect", LUA_OPCODE(Lua_V1, GetActorRect) },
	{ "GetActorPuckVector", LUA_OPCODE(Lua_V1, GetActorPuckVector) },
	{ "GetActorYawToPoint", LUA_OPCODE(Lua_V1, GetActorYawToPoint) },
	{ "SetActorReflection", LUA_OPCODE(Lua_V1, SetActorReflection) },
	{ "PutActorAtInterest", LUA_OPCODE(Lua_V1, PutActorAtInterest) },
	{ "PutActorAt", LUA_OPCODE(Lua_V1, PutActorAt) },
	{ "PutActorInSet", LUA_OPCODE(Lua_V1, PutActorInSet) },
	{ "WalkActorVector", LUA_OPCODE(Lua_V1, WalkActorVector) },
	{ "WalkActorForward", LUA_OPCODE(Lua_V1, WalkActorForward) },
	{ "DriveActorTo", LUA_OPCODE(Lua_V1, DriveActorTo) },
	{ "WalkActorTo", LUA_OPCODE(Lua_V1, WalkActorTo) },
	{ "WalkActorToAvoiding", LUA_OPCODE(Lua_V1, WalkActorToAvoiding) },
	{ "ActorLookAt", LUA_OPCODE(Lua_V1, ActorLookAt) },
	{ "SetActorLookRate", LUA_OPCODE(Lua_V1, SetActorLookRate) },
	{ "GetActorLookRate", LUA_OPCODE(Lua_V1, GetActorLookRate) },
	{ "GetVisibleThings", LUA_OPCODE(Lua_V1, GetVisibleThings) },
	{ "GetCameraActor", LUA_OPCODE(Lua_V1, GetCameraActor) },
	{ "SetActorHead", LUA_OPCODE(Lua_V1, SetActorHead) },
	{ "SetActorVisibility", LUA_OPCODE(Lua_V1, SetActorVisibility) },
	{ "SetActorFollowBoxes", LUA_OPCODE(Lua_V1, SetActorFollowBoxes) },
	{ "ShutUpActor", LUA_OPCODE(Lua_V1, ShutUpActor) },
	{ "SetActorFrustrumCull", LUA_OPCODE(Lua_V1, SetActorFrustrumCull) },
	{ "IsActorInSector", LUA_OPCODE(Lua_V1, IsActorInSector) },
	{ "GetActorSector", LUA_OPCODE(Lua_V1, GetActorSector) },
	{ "IsPointInSector", LUA_OPCODE(Lua_V1, IsPointInSector) },
	{ "GetPointSector", LUA_OPCODE(Lua_V1, GetPointSector) },
	{ "TurnActor", LUA_OPCODE(Lua_V1, TurnActor) },
	{ "GetActorRot", LUA_OPCODE(Lua_V1, GetActorRot) },
	{ "SetActorRot", LUA_OPCODE(Lua_V1, SetActorRot) },
	{ "SetActorPitch", LUA_OPCODE(Lua_V1, SetActorPitch) },
	{ "SetActorRoll", LUA_OPCODE(Lua_V1, SetActorRoll) },
	{ "IsActorTurning", LUA_OPCODE(Lua_V1, IsActorTurning) },
	{ "PlayActorChore", LUA_OPCODE(Lua_V1, PlayActorChore) },
	{ "PlayActorChoreLooping", LUA_OPCODE(Lua_V1, PlayActorChoreLooping) },
	{ "StopActorChore", LUA_OPCODE(Lua_V1, StopActorChore) },
	{ "CompleteActorChore", LUA_OPCODE(Lua_V1, CompleteActorChore) },
	{ "IsActorMoving", LUA_OPCODE(Lua_V1, IsActorMoving) },
	{ "IsActorChoring", LUA_OPCODE(Lua_V1, IsActorChoring) },
	{ "IsActorResting", LUA_OPCODE(Lua_V1, IsActorResting) },
	{ "SetActorChoreLooping", LUA_OPCODE(Lua_V1, SetActorChoreLooping) },
	{ "GetActorChores", LUA_OPCODE(Lua_V1, GetActorChores) },
	{ "GetActorCostumeDepth", LUA_OPCODE(Lua_V1, GetActorCostumeDepth) },
	{ "WorldToScreen", LUA_OPCODE(Lua_V1, WorldToScreen) },
	{ "exit", LUA_OPCODE(Lua_V1, Exit) },
	{ "FunctionName", LUA_OPCODE(Lua_V1, FunctionName) },
	{ "EnableDebugKeys", LUA_OPCODE(Lua_V1, EnableDebugKeys) },
	{ "LockFont", LUA_OPCODE(Lua_V1, LockFont) },
	{ "EnableControl", LUA_OPCODE(Lua_V1, EnableControl) },
	{ "DisableControl", LUA_OPCODE(Lua_V1, DisableControl) },
	{ "GetControlState", LUA_OPCODE(Lua_V1, GetControlState) },
	{ "PrintError", LUA_OPCODE(Lua_V1, PrintError) },
	{ "PrintWarning", LUA_OPCODE(Lua_V1, PrintWarning) },
	{ "PrintDebug", LUA_OPCODE(Lua_V1, PrintDebug) },
	{ "MakeCurrentSet", LUA_OPCODE(Lua_V1, MakeCurrentSet) },
	{ "LockSet", LUA_OPCODE(Lua_V1, LockSet) },
	{ "UnLockSet", LUA_OPCODE(Lua_V1, UnLockSet) },
	{ "MakeCurrentSetup", LUA_OPCODE(Lua_V1, MakeCurrentSetup) },
	{ "GetCurrentSetup", LUA_OPCODE(Lua_V1, GetCurrentSetup) },
	{ "NextSetup", LUA_OPCODE(Lua_V1, NextSetup) },
	{ "PreviousSetup", LUA_OPCODE(Lua_V1, PreviousSetup) },
	{ "StartFullscreenMovie", LUA_OPCODE(Lua_V1, StartFullscreenMovie) },
	{ "IsFullscreenMoviePlaying", LUA_OPCODE(Lua_V1, IsFullscreenMoviePlaying) },
	{ "StartMovie", LUA_OPCODE(Lua_V1, StartMovie) },
	{ "StopMovie", LUA_OPCODE(Lua_V1, StopMovie) },
	{ "PauseMovie", LUA_OPCODE(Lua_V1, PauseMovie) },
	{ "IsMoviePlaying", LUA_OPCODE(Lua_V1, IsMoviePlaying) },
	{ "PlaySound", LUA_OPCODE(Lua_V1, PlaySound) },
	{ "PlaySoundAt", LUA_OPCODE(Lua_V1, PlaySoundAt) },
	{ "IsSoundPlaying", LUA_OPCODE(Lua_V1, IsSoundPlaying) },
	{ "SetSoundPosition", LUA_OPCODE(Lua_V1, SetSoundPosition) },
	{ "FileFindFirst", LUA_OPCODE(Lua_V1, FileFindFirst) },
	{ "FileFindNext", LUA_OPCODE(Lua_V1, FileFindNext) },
	{ "FileFindDispose", LUA_OPCODE(Lua_V1, FileFindDispose) },
	{ "InputDialog", LUA_OPCODE(Lua_V1, InputDialog) },
	{ "WriteRegistryValue", LUA_OPCODE(Lua_V1, WriteRegistryValue) },
	{ "ReadRegistryValue", LUA_OPCODE(Lua_V1, ReadRegistryValue) },
	{ "GetSectorOppositeEdge", LUA_OPCODE(Lua_V1, GetSectorOppositeEdge) },
	{ "MakeSectorActive", LUA_OPCODE(Lua_V1, MakeSectorActive) },
	{ "PreRender", LUA_OPCODE(Lua_V1, PreRender) },
	{ "SpewStartup", LUA_OPCODE(Lua_V1, SpewStartup) },
	{ "GetCurrentScript", LUA_OPCODE(Lua_V1, GetCurrentScript) },
	{ "PrintActorCostumes", LUA_OPCODE(Lua_V1, PrintActorCostumes) },
	{ "PushActorCostume", LUA_OPCODE(Lua_V1, PushActorCostume) },
	{ "PopActorCostume", LUA_OPCODE(Lua_V1, PopActorCostume) },
	{ "LoadCostume", LUA_OPCODE(Lua_V1, LoadCostume) },
	{ "RotateVector", LUA_OPCODE(Lua_V1, RotateVector) },
	{ "GetCameraPosition", LUA_OPCODE(Lua_V1, GetCameraPosition) },
	{ "SetCameraPosition", LUA_OPCODE(Lua_V1, SetCameraPosition) },
	{ "SetCameraInterest", LUA_OPCODE(Lua_V1, SetCameraInterest) },
	{ "GetCameraFOV", LUA_OPCODE(Lua_V1, GetCameraFOV) },
	{ "SetCameraFOV", LUA_OPCODE(Lua_V1, SetCameraFOV) },
	{ "GetCameraRoll", LUA_OPCODE(Lua_V1, GetCameraRoll) },
	{ "SetCameraRoll", LUA_OPCODE(Lua_V1, SetCameraRoll) },
	{ "GetCameraLookVector", LUA_OPCODE(Lua_V1, GetCameraLookVector) },
	{ "PointActorAt", LUA_OPCODE(Lua_V1, PointActorAt) },
	{ "TurnActorTo", LUA_OPCODE(Lua_V1, TurnActorTo) },
	{ "PerSecond", LUA_OPCODE(Lua_V1, PerSecond) },
	{ "GetAngleBetweenVectors", LUA_OPCODE(Lua_V1, GetAngleBetweenVectors) },
	{ "GetAngleBetweenActors", LUA_OPCODE(Lua_V1, GetAngleBetweenActors) },
	{ "SetAmbientLight", LUA_OPCODE(Lua_V1, SetAmbientLight) },
	{ "TurnLightOn", LUA_OPCODE(Lua_V1, TurnLightOn) },
	{ "SetLightPosition", LUA_OPCODE(Lua_V1, SetLightPosition) },
	{ "SetLightIntensity", LUA_OPCODE(Lua_V1, SetLightIntensity) },
	{ "LightMgrSetChange", LUA_OPCODE(Lua_V1, LightMgrSetChange) },
	{ "LightMgrStartup", LUA_OPCODE(Lua_V1, LightMgrStartup) },
	{ "ImStartSound", LUA_OPCODE(Lua_V1, ImStartSound) },
	{ "ImStopSound", LUA_OPCODE(Lua_V1, ImStopSound) },
	{ "ImStopAllSounds", LUA_OPCODE(Lua_V1, ImStopAllSounds) },
	{ "ImGetParam", LUA_OPCODE(Lua_V1, ImGetParam) },
	{ "ImSetParam", LUA_OPCODE(Lua_V1, ImSetParam) },
	{ "ImFadeParam", LUA_OPCODE(Lua_V1, ImFadeParam) },
	{ "ImGetSfxVol", LUA_OPCODE(Lua_V1, ImGetSfxVol) },
	{ "ImSetSfxVol", LUA_OPCODE(Lua_V1, ImSetSfxVol) },
	{ "ImGetVoiceVol", LUA_OPCODE(Lua_V1, ImGetVoiceVol) },
	{ "ImSetVoiceVol", LUA_OPCODE(Lua_V1, ImSetVoiceVol) },
	{ "ImGetMusicVol", LUA_OPCODE(Lua_V1, ImGetMusicVol) },
	{ "ImSetMusicVol", LUA_OPCODE(Lua_V1, ImSetMusicVol) },
	{ "ImSetState", LUA_OPCODE(Lua_V1, ImSetState) },
	{ "ImSetSequence", LUA_OPCODE(Lua_V1, ImSetSequence) },
	{ "ImPause", LUA_OPCODE(Lua_V1, ImPause) },
	{ "ImResume", LUA_OPCODE(Lua_V1, ImResume) },
	{ "ImSetVoiceEffect", LUA_OPCODE(Lua_V1, ImSetVoiceEffect) },
	{ "LoadBundle", LUA_OPCODE(Lua_V1, LoadBundle) },
	{ "SetGamma", LUA_OPCODE(Lua_V1, SetGamma) },
	{ "SetActorWalkDominate", LUA_OPCODE(Lua_V1, SetActorWalkDominate) },
	{ "SetActorConstrain", LUA_OPCODE(Lua_V1, SetActorConstrain) },
	{ "RenderModeUser", LUA_OPCODE(Lua_V1, RenderModeUser) },
	{ "ForceRefresh", LUA_OPCODE(Lua_V1, ForceRefresh) },
	{ "DimScreen", LUA_OPCODE(Lua_V1, DimScreen) },
	{ "DimRegion", LUA_OPCODE(Lua_V1, DimRegion) },
	{ "CleanBuffer", LUA_OPCODE(Lua_V1, CleanBuffer) },
	{ "Display", LUA_OPCODE(Lua_V1, Display) },
	{ "SetSpeechMode", LUA_OPCODE(Lua_V1, SetSpeechMode) },
	{ "GetSpeechMode", LUA_OPCODE(Lua_V1, GetSpeechMode) },
	{ "SetShadowColor", LUA_OPCODE(Lua_V1, SetShadowColor) },
	{ "ActivateActorShadow", LUA_OPCODE(Lua_V1, ActivateActorShadow) },
	{ "SetActorShadowPlane", LUA_OPCODE(Lua_V1, SetActorShadowPlane) },
	{ "SetActorShadowPoint", LUA_OPCODE(Lua_V1, SetActorShadowPoint) },
	{ "SetActiveShadow", LUA_OPCODE(Lua_V1, SetActiveShadow) },
	{ "KillActorShadows", LUA_OPCODE(Lua_V1, KillActorShadows) },
	{ "AddShadowPlane", LUA_OPCODE(Lua_V1, AddShadowPlane) },
	{ "SetActorShadowValid", LUA_OPCODE(Lua_V1, SetActorShadowValid) },
	{ "FreeObjectState", LUA_OPCODE(Lua_V1, FreeObjectState) },
	{ "NewObjectState", LUA_OPCODE(Lua_V1, NewObjectState) },
	{ "SetObjectType", LUA_OPCODE(Lua_V1, SetObjectType) },
	{ "SendObjectToBack", LUA_OPCODE(Lua_V1, SendObjectToBack) },
	{ "SendObjectToFront", LUA_OPCODE(Lua_V1, SendObjectToFront) },
	{ "ActorToClean", LUA_OPCODE(Lua_V1, ActorToClean) },
	{ "FlushControls", LUA_OPCODE(Lua_V1, FlushControls) },
	{ "SetActorCollisionMode", LUA_OPCODE(Lua_V1, SetActorCollisionMode) },
	{ "SetActorCollisionScale", LUA_OPCODE(Lua_V1, SetActorCollisionScale) },
	{ "SetActorClipActive", LUA_OPCODE(Lua_V1, SetActorClipActive) },
	{ "SetActorClipPlane", LUA_OPCODE(Lua_V1, SetActorClipPlane) },
	{ "FadeOutChore", LUA_OPCODE(Lua_V1, FadeOutChore) },
	{ "FadeInChore", LUA_OPCODE(Lua_V1, FadeInChore) },
	{ "IrisDown", LUA_OPCODE(Lua_V1, IrisDown) },
	{ "IrisUp", LUA_OPCODE(Lua_V1, IrisUp) },
	{ "TextFileGetLineCount", LUA_OPCODE(Lua_V1, TextFileGetLineCount) },
	{ "TextFileGetLine", LUA_OPCODE(Lua_V1, TextFileGetLine) },
	{ "ScreenShot", LUA_OPCODE(Lua_V1, ScreenShot) },
	{ "GetSaveGameImage", LUA_OPCODE(Lua_V1, GetSaveGameImage) },
	{ "GetImage", LUA_OPCODE(Lua_V1, GetImage) },
	{ "FreeImage", LUA_OPCODE(Lua_V1, FreeImage) },
	{ "BlastImage", LUA_OPCODE(Lua_V1, BlastImage) },
	{ "BlastRect", LUA_OPCODE(Lua_V1, BlastRect) },
	{ "SubmitSaveGameData", LUA_OPCODE(Lua_V1, SubmitSaveGameData) },
	{ "GetSaveGameData", LUA_OPCODE(Lua_V1, GetSaveGameData) },
	{ "SetTextSpeed", LUA_OPCODE(Lua_V1, SetTextSpeed) },
	{ "GetTextSpeed", LUA_OPCODE(Lua_V1, GetTextSpeed) },
	{ "DetachFromResources", LUA_OPCODE(Lua_V1, DetachFromResources) },
	{ "AttachToResources", LUA_OPCODE(Lua_V1, AttachToResources) },
	{ "ActorPuckOrient", LUA_OPCODE(Lua_V1, ActorPuckOrient) },
	{ "JustLoaded", LUA_OPCODE(Lua_V1, JustLoaded) },
	{ "ResetTextures", LUA_OPCODE(Lua_V1, ResetTextures) },
	{ "ShrinkBoxes", LUA_OPCODE(Lua_V1, ShrinkBoxes) },
	{ "UnShrinkBoxes", LUA_OPCODE(Lua_V1, UnShrinkBoxes) },
	{ "GetShrinkPos", LUA_OPCODE(Lua_V1, GetShrinkPos) },
	{ "NukeResources", LUA_OPCODE(Lua_V1, NukeResources) },
	{ "SetActorInvClipNode", LUA_OPCODE(Lua_V1, SetActorInvClipNode) },
	{ "GetDiskFreeSpace", LUA_OPCODE(Lua_V1, GetDiskFreeSpace) },
	{ "SaveIMuse", LUA_OPCODE(Lua_V1, SaveIMuse) },
	{ "RestoreIMuse", LUA_OPCODE(Lua_V1, RestoreIMuse) },
	{ "GetMemoryUsage", LUA_OPCODE(Lua_V1, GetMemoryUsage) },
	{ "dofile", LUA_OPCODE(Lua_V1, new_dofile) },
};

static struct luaL_reg grimTextOpcodes[] = {
	{ "IsMessageGoing", LUA_OPCODE(Lua_V1, IsMessageGoing) },
	{ "SetSayLineDefaults", LUA_OPCODE(Lua_V1, SetSayLineDefaults) },
	{ "SetActorTalkColor", LUA_OPCODE(Lua_V1, SetActorTalkColor) },
	{ "GetActorTalkColor", LUA_OPCODE(Lua_V1, GetActorTalkColor) },
	{ "SayLine", LUA_OPCODE(Lua_V1, SayLine) },
	{ "PrintLine", LUA_OPCODE(Lua_V1, PrintLine) },
	{ "MakeTextObject", LUA_OPCODE(Lua_V1, MakeTextObject) },
	{ "GetTextObjectDimensions", LUA_OPCODE(Lua_V1, GetTextObjectDimensions) },
	{ "GetFontDimensions", LUA_OPCODE(Lua_V1, GetFontDimensions) },
	{ "ChangeTextObject", LUA_OPCODE(Lua_V1, ChangeTextObject) },
	{ "KillTextObject", LUA_OPCODE(Lua_V1, KillTextObject) },
	{ "BlastText", LUA_OPCODE(Lua_V1, BlastText) },
	{ "ExpireText", LUA_OPCODE(Lua_V1, ExpireText) },
	{ "PurgeText", LUA_OPCODE(Lua_V1, PurgeText) },
	{ "MakeColor", LUA_OPCODE(Lua_V1, MakeColor) },
	{ "GetColorComponents", LUA_OPCODE(Lua_V1, GetColorComponents) },
	{ "SetTranslationMode", LUA_OPCODE(Lua_V1, SetTranslationMode) },
	{ "GetTranslationMode", LUA_OPCODE(Lua_V1, GetTranslationMode) },
	{ "GetTextCharPosition", LUA_OPCODE(Lua_V1, GetTextCharPosition) },
	{ "LocalizeString", LUA_OPCODE(Lua_V1, LocalizeString) },
	{ "SetEmergencyFont", LUA_OPCODE(Lua_V1, SetEmergencyFont) },
	{ "SetOffscreenTextPos", LUA_OPCODE(Lua_V1, SetOffscreenTextPos) }
};

struct luaL_reg grimPrimitivesOpcodes[] = {
	{ "DrawLine", LUA_OPCODE(Lua_V1, DrawLine) },
	{ "DrawPolygon", LUA_OPCODE(Lua_V1, DrawPolygon) },
	{ "DrawRectangle", LUA_OPCODE(Lua_V1, DrawRectangle) },
	{ "ChangePrimitive", LUA_OPCODE(Lua_V1, ChangePrimitive) },
	{ "KillPrimitive", LUA_OPCODE(Lua_V1, KillPrimitive) },
	{ "PurgePrimitiveQueue", LUA_OPCODE(Lua_V1, PurgePrimitiveQueue) }
};

struct luaL_reg grimHardwareOpcodes[] = {
	{ "Is3DHardwareEnabled", LUA_OPCODE(Lua_V1, Is3DHardwareEnabled) },
	{ "GetVideoDevices", LUA_OPCODE(Lua_V1, GetVideoDevices) },
	{ "SetVideoDevices", LUA_OPCODE(Lua_V1, SetVideoDevices) },
	{ "SetHardwareState", LUA_OPCODE(Lua_V1, SetHardwareState) },
	{ "Enumerate3DDevices", LUA_OPCODE(Lua_V1, Enumerate3DDevices) },
	{ "EnumerateVideoDevices", LUA_OPCODE(Lua_V1, EnumerateVideoDevices) }
};

void Lua_V1::registerOpcodes() {
	// Register main opcodes functions
	luaL_openlib(grimMainOpcodes, ARRAYSIZE(grimMainOpcodes));

	// Register text opcodes functions
	luaL_openlib(grimTextOpcodes, ARRAYSIZE(grimTextOpcodes));

	// Register primitives opcodeEs functions
	luaL_openlib(grimPrimitivesOpcodes, ARRAYSIZE(grimPrimitivesOpcodes));

	// Register hardware opcode functions
	luaL_openlib(grimHardwareOpcodes, ARRAYSIZE(grimHardwareOpcodes));

	LuaBase::registerOpcodes();
}

void Lua_V1::postRestoreHandle() {
	// Apply the patch, only if it wasn't applied already.
	if (lua_isnil(lua_getglobal("  service_release.lua")))
		dofile("patch05.bin");

	lua_beginblock();
	// Set the developerMode, since the save contains the value of
	// the installation it was made with.
	lua_pushobject(lua_getglobal("developerMode"));
	const char *devMode = g_registry->get("good_times", "");
	if (devMode[0] == 0)
		lua_pushnil();
	else
		lua_pushstring(devMode);
	lua_setglobal("developerMode");
	lua_endblock();
}

} // end of namespace Grim
