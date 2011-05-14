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
 * $URL$
 * $Id$
 *
 */

#define FORBIDDEN_SYMBOL_EXCEPTION_printf

#include "common/endian.h"
#include "common/system.h"

#include "engines/grim/lua.h"
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

#include "engines/grim/lua/lauxlib.h"
#include "engines/grim/lua/luadebug.h"

namespace Grim {

int refSystemTable;
int refTypeOverride;
int refOldConcatFallback;
int refTextObjectX;
int refTextObjectY;
int refTextObjectFont;
int refTextObjectWidth;
int refTextObjectHeight;
int refTextObjectFGColor;
int refTextObjectBGColor;
int refTextObjectFXColor;
int refTextObjectHIColor;
int refTextObjectDuration;
int refTextObjectCenter;
int refTextObjectLJustify;
int refTextObjectRJustify;
int refTextObjectVolume;
int refTextObjectBackground;
int refTextObjectPan;

#define strmatch(src, dst)		(strlen(src) == strlen(dst) && strcmp(src, dst) == 0)

bool getbool(int num) {
	return !lua_isnil(lua_getparam(num));
}

void pushbool(bool val) {
	if (val)
		lua_pushnumber(1);
	else
		lua_pushnil();
}

Actor *getactor(lua_Object obj) {
	return g_grim->getActor(lua_getuserdata(obj));
}

TextObject *gettextobject(lua_Object obj) {
	return g_grim->getTextObject(lua_getuserdata(obj));
}

Font *getfont(lua_Object obj) {
	return g_grim->getFont(lua_getuserdata(obj));
}

Color *getcolor(lua_Object obj) {
	return g_grim->getColor(lua_getuserdata(obj));
}

PrimitiveObject *getprimitive(lua_Object obj) {
	return g_grim->getPrimitiveObject(lua_getuserdata(obj));
}

ObjectState *getobjectstate(lua_Object obj) {
	return g_grim->getObjectState(lua_getuserdata(obj));
}

byte clamp_color(int c) {
	if (c < 0)
		return 0;
	else if (c > 255)
		return 255;
	else
		return c;
}

bool findCostume(lua_Object costumeObj, Actor *actor, Costume **costume) {
	*costume = actor->getCurrentCostume(); // should be root of list I think
	if (lua_isnil(costumeObj))
		return true;
	if (lua_isnumber(costumeObj)) {
/*		int num = (int)lua_getnumber(costumeObj);*/
		error("findCostume: search by Id not implemented");
		// TODO get costume by ID ?
	}
	if (lua_isstring(costumeObj)) {
		*costume = actor->findCostume(lua_getstring(costumeObj));
		return *costume != 0;
	}

	return false;
}


int luaA_passresults();

void L1_new_dofile() {
	const char *fname_str = luaL_check_string(1);
	if (g_grim->bundle_dofile(fname_str) == 0)
		if (luaA_passresults() == 0)
			lua_pushuserdata(0);
}

// Debugging message functions

void L1_PrintDebug() {
	if (gDebugLevel == DEBUG_NORMAL || gDebugLevel == DEBUG_ALL) {
		Common::String msg("Debug: ");
		lua_Object strObj = lua_getparam(1);
		if (lua_isnil(strObj))
			msg += "(nil)";
		if (!lua_isstring(strObj))
			return;
		msg += Common::String(lua_getstring(strObj)) + "\n";
		printf("%s", msg.c_str());
	}
}

void L1_PrintError() {
	if (gDebugLevel == DEBUG_ERROR || gDebugLevel == DEBUG_ALL) {
		Common::String msg("Error: ");
		lua_Object strObj = lua_getparam(1);
		if (lua_isnil(strObj))
			msg += "(nil)";
		if (!lua_isstring(strObj))
			return;
		msg += Common::String(lua_getstring(strObj)) + "\n";
		printf("%s", msg.c_str());
	}
}

void L1_PrintWarning() {
	if (gDebugLevel == DEBUG_WARN || gDebugLevel == DEBUG_ALL) {
		Common::String msg("Warning: ");
		lua_Object strObj = lua_getparam(1);
		if (lua_isnil(strObj))
			msg += "(nil)";
		if (!lua_isstring(strObj))
			return;
		msg += Common::String(lua_getstring(strObj)) + "\n";
		printf("%s", msg.c_str());
	}
}

void L1_FunctionName() {
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

void L1_CheckForFile() {
	lua_Object strObj = lua_getparam(1);

	if (!lua_isstring(strObj))
		return;

	const char *filename = lua_getstring(strObj);
	pushbool(g_resourceloader->getFileExists(filename));
}

void L1_MakeColor() {
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

	Color *c = new Color (r, g ,b);
	g_grim->registerColor(c);
	lua_pushusertag(c->getId(), MKTAG('C','O','L','R'));
}

void L1_GetColorComponents() {
	lua_Object colorObj = lua_getparam(1);
	Color *c = getcolor(colorObj);
	lua_pushnumber(c->getRed());
	lua_pushnumber(c->getGreen());
	lua_pushnumber(c->getBlue());
}

void L1_ReadRegistryValue() {
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

void L1_WriteRegistryValue() {
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

void L1_GetAngleBetweenVectors() {
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

	Graphics::Vector3d vec1(x1, y1, z1);
	Graphics::Vector3d vec2(x2, y2, z2);
	vec1.normalize();
	vec2.normalize();

	float dot = vec1.dotProduct(vec2.x(), vec2.y(), vec2.z());
	float angle = 90.0f - (180.0f * asin(dot)) / LOCAL_PI;
	if (angle < 0)
		angle = -angle;
	lua_pushnumber(angle);
}

void L1_Is3DHardwareEnabled() {
	pushbool(g_driver->isHardwareAccelerated());
}

void L1_SetHardwareState() {
	// changing only in config setup (software/hardware rendering)
	bool accel = getbool(1);
	if (accel)
		g_registry->set("soft_renderer", "false");
	else
		g_registry->set("soft_renderer", "true");
}

void L1_SetVideoDevices() {
	int devId;
	int modeId;

	devId = (int)lua_getnumber(lua_getparam(1));
	modeId = (int)lua_getnumber(lua_getparam(2));
	// ignore setting video devices
}

void L1_GetVideoDevices() {
	lua_pushnumber(0.0);
	lua_pushnumber(-1.0);
}

void L1_EnumerateVideoDevices() {
	lua_Object result = lua_createtable();
	lua_pushobject(result);
	lua_pushnumber(0.0); // id of device
	lua_pushstring(g_driver->getVideoDeviceName()); // name of device
	lua_settable();
	lua_pushobject(result);
}

void L1_Enumerate3DDevices() {
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
void L1_RotateVector() {
	lua_Object vecObj = lua_getparam(1);
	lua_Object rotObj = lua_getparam(2);
	lua_Object resObj;
	Graphics::Vector3d vec, rot, resVec;
	float x, y, z;

	if (!lua_istable(vecObj) || !lua_istable(rotObj)) {
		lua_pushnil();
		return;
	}

	lua_pushobject(vecObj);
	lua_pushstring("x");
	x = lua_getnumber(lua_gettable());
	lua_pushobject(vecObj);
	lua_pushstring("y");
	y = lua_getnumber(lua_gettable());
	lua_pushobject(vecObj);
	lua_pushstring("z");
	z = lua_getnumber(lua_gettable());
	vec.set(x, y, z);

	lua_pushobject(rotObj);
	lua_pushstring("x");
	x = lua_getnumber(lua_gettable());
	lua_pushobject(rotObj);
	lua_pushstring("y");
	y = lua_getnumber(lua_gettable());
	lua_pushobject(rotObj);
	lua_pushstring("z");
	z = lua_getnumber(lua_gettable());
	rot.set(x, y, z);

	// FIXME: Is this really right?
	float xAngle = x * LOCAL_PI / 180.f;
	float yAngle = z * LOCAL_PI / 180.f;
	float zAngle = y * LOCAL_PI / 180.f;

	x = vec.x() * cos(zAngle) - vec.y() * sin(zAngle);
	y = vec.x() * sin(zAngle) + vec.y() * cos(zAngle);
	vec.x() = x;
	vec.y() = y;

	y = vec.y() * cos(xAngle) - vec.z() * sin(xAngle);
	z = vec.y() * sin(xAngle) + vec.z() * cos(xAngle);
	vec.y() = y;
	vec.z() = z;

	x = vec.x() * cos(yAngle) - vec.z() * sin(yAngle);
	z = vec.x() * sin(yAngle) + vec.z() * cos(yAngle);
	vec.x() = x;
	vec.z() = z;

	resObj = lua_createtable();
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
void L1_GetPointSector() {
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

	Graphics::Vector3d point(x, y, z);
	Sector *result = g_grim->getCurrScene()->findPointSector(point, sectorType);
	if (result) {
		lua_pushnumber(result->getSectorId());
		lua_pushstring(const_cast<char *>(result->getName()));
		lua_pushnumber(result->getType());
	} else {
		lua_pushnil();
	}
}

void L1_GetActorSector() {
	lua_Object actorObj = lua_getparam(1);
	lua_Object typeObj = lua_getparam(2);

	if (!lua_isuserdata(actorObj) || lua_tag(actorObj) != MKTAG('A','C','T','R'))
		return;
	if (!lua_isnumber(typeObj))
		return;

	Actor *actor = getactor(actorObj);
	Sector::SectorType sectorType = (Sector::SectorType)(int)lua_getnumber(typeObj);
	Graphics::Vector3d pos = actor->getDestPos();
	Sector *result = g_grim->getCurrScene()->findPointSector(pos, sectorType);
	if (result) {
		lua_pushnumber(result->getSectorId());
		lua_pushstring(const_cast<char *>(result->getName()));
		lua_pushnumber(result->getType());
	} else {
		lua_pushnil();
	}
}

void L1_IsActorInSector() {
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

	int numSectors = g_grim->getCurrScene()->getSectorCount();
	for (int i = 0; i < numSectors; i++) {
		Sector *sector = g_grim->getCurrScene()->getSectorBase(i);
		if (strmatch(sector->getName(), name)) {
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

void L1_IsPointInSector() {
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
	Graphics::Vector3d pos(x, y, z);

	int numSectors = g_grim->getCurrScene()->getSectorCount();
	for (int i = 0; i < numSectors; i++) {
		Sector *sector = g_grim->getCurrScene()->getSectorBase(i);
		if (strmatch(sector->getName(), name)) {
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

void L1_MakeSectorActive() {
	lua_Object sectorObj = lua_getparam(1);

	if (!lua_isnumber(sectorObj) && !lua_isstring(sectorObj))
		return;

	// FIXME: This happens on initial load. Are we initting something in the wrong order?
	if (!g_grim->getCurrScene()) {
		warning("!!!! Trying to call MakeSectorActive without a scene");
		return;
	}

	bool visible = !lua_isnil(lua_getparam(2));
	int numSectors = g_grim->getCurrScene()->getSectorCount();
	if (lua_isstring(sectorObj)) {
		const char *name = lua_getstring(sectorObj);
		for (int i = 0; i < numSectors; i++) {
			Sector *sector = g_grim->getCurrScene()->getSectorBase(i);
			if (strmatch(sector->getName(), name)) {
				sector->setVisible(visible);
				return;
			}
		}
	} else if (lua_isnumber(sectorObj)) {
		int id = (int)lua_getnumber(sectorObj);
		for (int i = 0; i < numSectors; i++) {
			Sector *sector = g_grim->getCurrScene()->getSectorBase(i);
			if (sector->getSectorId() == id) {
				sector->setVisible(visible);
				return;
			}
		}
	}
}

// Scene functions
void L1_LockSet() {
	lua_Object nameObj = lua_getparam(1);
	if (!lua_isstring(nameObj))
		return;

	const char *name = lua_getstring(nameObj);
	// TODO implement proper locking
	g_grim->setSceneLock(name, true);
}

void L1_UnLockSet() {
	lua_Object nameObj = lua_getparam(1);
	if (!lua_isstring(nameObj))
		return;

	const char *name = lua_getstring(nameObj);
	// TODO implement proper unlocking
	g_grim->setSceneLock(name, false);
}

void L1_MakeCurrentSet() {
	lua_Object nameObj = lua_getparam(1);
	if (!lua_isstring(nameObj)) {
		// TODO setting current set null
		return;
	}

	const char *name = lua_getstring(nameObj);
	if (gDebugLevel == DEBUG_NORMAL || gDebugLevel == DEBUG_ALL)
		printf("Entered new scene '%s'.\n", name);
	g_grim->setScene(name);
}

void L1_MakeCurrentSetup() {
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
void L1_GetCurrentSetup() {
	lua_Object nameObj = lua_getparam(1);
	if (!lua_isstring(nameObj))
		return;

	const char *name = lua_getstring(nameObj);

	// FIXME there are some big difference here !
	Scene *scene = g_grim->findScene(name);
	if (!scene) {
		warning("GetCurrentSetup() Requested scene (%s) is not loaded", name);
		lua_pushnil();
		return;
	}
	lua_pushnumber(scene->getSetup());
}

void L1_GetShrinkPos() {
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
	Graphics::Vector3d pos;
	pos.set(x, y, z);

	// TODO
	//UnShrinkBoxes();
	// lua_pusnumber 1, 2, 3 or lua_pushnil
	lua_pushnumber(x);
	lua_pushnumber(y);
	lua_pushnumber(z);

	warning("Stub function GetShrinkPos(%g,%g,%g,%g) called", x, y, z, r);
}

void L1_FileFindDispose() {
	if (g_grim->_listFilesIter)
		g_grim->_listFilesIter->begin();
	g_grim->_listFiles.clear();
	g_grim->_listFilesIter = NULL;
}

void L1_FileFindNext() {
	if (g_grim->_listFilesIter == g_grim->_listFiles.end()) {
		lua_pushnil();
		L1_FileFindDispose();
	} else {
		lua_pushstring(g_grim->_listFilesIter->c_str());
		g_grim->_listFilesIter++;
	}
}

void L1_FileFindFirst() {
	lua_Object extObj = lua_getparam(1);
	if (!lua_isstring(extObj)) {
		lua_pushnil();
		return;
	}

	L1_FileFindDispose();

	const char *extension = lua_getstring(extObj);
	Common::SaveFileManager *saveFileMan = g_system->getSavefileManager();
	g_grim->_listFiles = saveFileMan->listSavefiles(extension);
	g_grim->_listFilesIter = g_grim->_listFiles.begin();

	if (g_grim->_listFilesIter == g_grim->_listFiles.end())
		lua_pushnil();
	else
		L1_FileFindNext();
}

void L1_PerSecond() {
	lua_Object rateObj = lua_getparam(1);

	if (!lua_isnumber(rateObj)) {
		lua_pushnil();
		return;
	}
	float rate = lua_getnumber(rateObj);
	lua_pushnumber(g_grim->getPerSecond(rate));
}

void L1_EnableControl() {
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

void L1_DisableControl() {
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

void L1_GetControlState() {
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

void L1_Exit() {
	g_grim->quitGame();
}

void L1_SetSpeechMode() {
	int mode;

	mode = (int)lua_getnumber(lua_getparam(1));
	if (mode >= 1 && mode <= 3)
		g_grim->setSpeechMode(mode);
}

void L1_GetSpeechMode() {
	int mode;

	mode = g_grim->getSpeechMode();
	lua_pushnumber(mode);
}

void L1_GetDiskFreeSpace() {
	// amount of free space in MB, used for creating saves
	lua_pushnumber(50);
}

void L1_NewObjectState() {
	int setupID = (int)lua_getnumber(lua_getparam(1));
	int val = (int)lua_getnumber(lua_getparam(2));
	ObjectState::Position pos = (ObjectState::Position)val;
	const char *bitmap = lua_getstring(lua_getparam(3));
	const char *zbitmap = NULL;
	if (!lua_isnil(lua_getparam(4)))
		zbitmap = lua_getstring(lua_getparam(4));
	bool transparency = getbool(5);

	ObjectState *state = new ObjectState(setupID, pos, bitmap, zbitmap, transparency);
	g_grim->registerObjectState(state);
	g_grim->getCurrScene()->addObjectState(state);
	lua_pushusertag(state->getId(), MKTAG('S','T','A','T'));
}

void L1_FreeObjectState() {
	lua_Object param = lua_getparam(1);
	if (!lua_isuserdata(param) || lua_tag(param) != MKTAG('S','T','A','T'))
		return;
	ObjectState *state =  getobjectstate(param);
	g_grim->getCurrScene()->deleteObjectState(state);
	delete state;
}

void L1_SendObjectToBack() {
	lua_Object param = lua_getparam(1);
	if (lua_isuserdata(param) && lua_tag(param) == MKTAG('S','T','A','T')) {
		ObjectState *state =  getobjectstate(param);
		g_grim->getCurrScene()->moveObjectStateToFirst(state);
	}
}

void L1_SendObjectToFront() {
	lua_Object param = lua_getparam(1);
	if (lua_isuserdata(param) && lua_tag(param) == MKTAG('S','T','A','T')) {
		ObjectState *state =  getobjectstate(param);
		g_grim->getCurrScene()->moveObjectStateToLast(state);
	}
}

void L1_SetObjectType() {
	lua_Object param = lua_getparam(1);
	if (!lua_isuserdata(param) || lua_tag(param) != MKTAG('S','T','A','T'))
		return;
	ObjectState *state =  getobjectstate(param);
	int val = (int)lua_getnumber(lua_getparam(2));
	ObjectState::Position pos = (ObjectState::Position)val;
	state->setPos(pos);
}

void L1_GetCurrentScript() {
	current_script();
}

void L1_GetSaveGameImage() {
	int width = 250, height = 188;
	char *data;
	Bitmap *screenshot;
	int dataSize;

	lua_Object param = lua_getparam(1);
	if (!lua_isstring(param)) {
		lua_pushnil();
		return;
	}
	const char *filename = lua_getstring(param);
	SaveGame *savedState = new SaveGame(filename, false);
	if (!savedState || savedState->saveVersion() != SaveGame::SAVEGAME_VERSION) {
		lua_pushnil();
		return;
	}
	dataSize = savedState->beginSection('SIMG');
	data = new char[dataSize];
	savedState->read(data, dataSize);
	screenshot = g_grim->registerBitmap(data, width, height, "screenshot");
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

void L1_SubmitSaveGameData() {
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
		int len = strlen(str) + 1;
		savedState->write(&len, sizeof(int32));
		savedState->write(str, len);
	}
	savedState->endSection();
}

void L1_GetSaveGameData() {
	lua_Object param = lua_getparam(1);
	if (!lua_isstring(param))
		return;
	const char *filename = lua_getstring(param);
	SaveGame *savedState = new SaveGame(filename, false);
	lua_Object result = lua_createtable();

	if (!savedState || savedState->saveVersion() != SaveGame::SAVEGAME_VERSION) {
		lua_pushobject(result);
		lua_pushnumber(2);
		lua_pushstring("mo.set"); // Just a placeholder to not make it throw a lua error
		lua_settable();
		lua_pushobject(result);

		warning("Savegame %s is incompatible with this Residual build.", filename);
		delete savedState;
		return;
	}
	int32 dataSize = savedState->beginSection('SUBS');

	char str[200];
	int strSize;
	int count = 0;

	for (;;) {
		if (dataSize <= 0)
			break;
		savedState->read(&strSize, sizeof(int32));
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

void L1_Load() {
	lua_Object fileName = lua_getparam(1);
	if (lua_isnil(fileName)) {
		g_grim->_savegameFileName = NULL;
	} else if (lua_isstring(fileName)) {
		g_grim->_savegameFileName = lua_getstring(fileName);
	} else {
		warning("Load() fileName is wrong");
		return;
	}
	g_grim->_savegameLoadRequest = true;
}

void L1_Save() {
	lua_Object fileName = lua_getparam(1);
	if (lua_isnil(fileName)) {
		g_grim->_savegameFileName = NULL;
	} else if (lua_isstring(fileName)) {
		g_grim->_savegameFileName = lua_getstring(fileName);
	} else {
		warning("Save() fileName is wrong");
		return;
	}
	g_grim->_savegameSaveRequest = true;
}

void L1_Remove() {
	if (g_system->getSavefileManager()->removeSavefile(luaL_check_string(1)))
		lua_pushuserdata(0);
	else {
		lua_pushnil();
		lua_pushstring(g_system->getSavefileManager()->getErrorDesc().c_str());
	}
}

PointerId saveCallback(int32 /*tag*/, PointerId ptr, SaveSint32 /*savedState*/) {
	return ptr;
}

PointerId restoreCallback(int32 /*tag*/, PointerId ptr, RestoreSint32 /*savedState*/) {
	return ptr;
}

void L1_LockFont() {
	lua_Object param1 = lua_getparam(1);
	if (lua_isstring(param1)) {
		const char *fontName = lua_getstring(param1);
		Font *result = g_resourceloader->loadFont(fontName);
		if (result) {
			g_grim->registerFont(result);
			lua_pushusertag(result->getId(), MKTAG('F','O','N','T'));
			return;
		}
	}

	lua_pushnil();
}

void L1_EnableDebugKeys() {
}

void L1_LightMgrSetChange() {
	// that seems only used when some control panel is opened
}

void L1_SetAmbientLight() {
	int mode = (int)lua_getnumber(lua_getparam(1));
	if (mode == 0) {
		if (g_grim->getCurrScene()) {
			g_grim->getCurrScene()->setLightEnableState(true);
		}
	} else if (mode == 1) {
		if (g_grim->getCurrScene()) {
			g_grim->getCurrScene()->setLightEnableState(false);
		}
	}
}

void L1_SetLightIntensity() {
	lua_Object lightObj = lua_getparam(1);
	lua_Object intensityObj = lua_getparam(2);

	if (!lua_isnumber(intensityObj))
		return;

	float intensity = lua_getnumber(intensityObj);

	if (lua_isnumber(lightObj)) {
		int light = (int)lua_getnumber(lightObj);
		g_grim->getCurrScene()->setLightIntensity(light, intensity);
	} else if (lua_isstring(lightObj)) {
		const char *light = lua_getstring(lightObj);
		g_grim->getCurrScene()->setLightIntensity(light, intensity);
	}
}

void L1_SetLightPosition() {
	lua_Object lightObj = lua_getparam(1);
	lua_Object xObj = lua_getparam(2);
	lua_Object yObj = lua_getparam(3);
	lua_Object zObj = lua_getparam(4);

	if (!lua_isnumber(xObj) || !lua_isnumber(yObj) || !lua_isnumber(zObj))
		return;

	float x = lua_getnumber(xObj);
	float y = lua_getnumber(yObj);
	float z = lua_getnumber(zObj);
	Graphics::Vector3d vec(x, y, z);

	if (lua_isnumber(lightObj)) {
		int light = (int)lua_getnumber(lightObj);
		g_grim->getCurrScene()->setLightPosition(light, vec);
	} else if (lua_isstring(lightObj)) {
		const char *light = lua_getstring(lightObj);
		g_grim->getCurrScene()->setLightPosition(light, vec);
	}
}

void L1_LightMgrStartup() {
	// we will not implement this opcode
}

void L1_JustLoaded() {
	if (gDebugLevel == DEBUG_ERROR || gDebugLevel == DEBUG_ALL)
		error("OPCODE USAGE VERIFICATION: JustLoaded");
}

void L1_SetEmergencyFont() {
	if (gDebugLevel == DEBUG_ERROR || gDebugLevel == DEBUG_ALL)
		error("OPCODE USAGE VERIFICATION: SetEmergencyFont");
}

void L1_typeOverride() {
	lua_Object data = lua_getparam(1);

	if (lua_isuserdata(data)) {
		switch (lua_tag(data)) {
		case MKTAG('A','C','T','R'):
			lua_pushstring("actor");
			lua_pushnumber(lua_tag(data));
			return;
		case MKTAG('C','O','S','T'):
			lua_pushstring("costume");
			lua_pushnumber(lua_tag(data));
			return;
		case MKTAG('S','E','T',' '):
			lua_pushstring("set");
			lua_pushnumber(lua_tag(data));
			return;
		case MKTAG('K','E','Y','F'):
			lua_pushstring("keyframe");
			lua_pushnumber(lua_tag(data));
			return;
		default:
			break;
		}
	}

	lua_pushobject(data);
	lua_callfunction(lua_getref(refTypeOverride));
	lua_Object param1 = lua_getresult(1);
	lua_Object param2 = lua_getresult(2);
	lua_pushobject(param1);
	lua_pushobject(param2);
}

void L1_concatFallback() {
	lua_Object params[2];
	char result[200];
	char *strPtr;

	params[0] = lua_getparam(1);
	params[1] = lua_getparam(2);
	result[0] = 0;

	for (int i = 0; i < 2; i++) {
		if (!lua_isnil(params[i]) && !lua_isuserdata(params[i]) && !lua_isstring(params[i])) {
			lua_pushobject(params[0]);
			lua_pushobject(params[1]);
			lua_callfunction(lua_getref(refOldConcatFallback));
			lua_pushobject(lua_getresult(1));
			return;
		}

		int pos = strlen(result);
		strPtr = &result[pos];

		if (lua_isnil(params[i]))
			sprintf(strPtr, "(nil)");
		else if (lua_isstring(params[i]))
			sprintf(strPtr, "%s", lua_getstring(params[i]));
		else if (lua_tag(params[i]) == MKTAG('A','C','T','R')) {
			Actor *a = getactor(params[i]);
			sprintf(strPtr, "(actor%p:%s)", (void *)a,
				(a->getCurrentCostume() && a->getCurrentCostume()->getModelNodes()) ?
				a->getCurrentCostume()->getModelNodes()->_name : "");
		} else {
			lua_pushobject(params[0]);
			lua_pushobject(params[1]);
			lua_callfunction(lua_getref(refOldConcatFallback));
			lua_pushobject(lua_getresult(1));
			return;
		}
	}

	lua_pushstring(result);
}

void setFrameTime(float frameTime) {
	lua_pushobject(lua_getref(refSystemTable));
	lua_pushstring("frameTime");
	lua_pushnumber(frameTime);
	lua_settable();
}

void setMovieTime(float movieTime) {
	lua_pushobject(lua_getref(refSystemTable));
	lua_pushstring("movieTime");
	lua_pushnumber(movieTime);
	lua_settable();
}

void dummyHandler() {
}

// Stub function for builtin functions not yet implemented
static void stubWarning(const char *funcName) {
	warning("Stub function: %s", funcName);
}

#define STUB_FUNC(name) void name() { stubWarning(#name); }
STUB_FUNC(L1_SetActorInvClipNode)
STUB_FUNC(L1_NukeResources)
STUB_FUNC(L1_UnShrinkBoxes)
STUB_FUNC(L1_ShrinkBoxes)
STUB_FUNC(L1_ResetTextures)
STUB_FUNC(L1_AttachToResources)
STUB_FUNC(L1_DetachFromResources)
STUB_FUNC(L1_IrisUp)
STUB_FUNC(L1_IrisDown)
STUB_FUNC(L1_SetActorClipPlane)
STUB_FUNC(L1_SetActorClipActive)
STUB_FUNC(L1_SetActorCollisionScale)
STUB_FUNC(L1_SetActorCollisionMode)
STUB_FUNC(L1_FlushControls)
STUB_FUNC(L1_TurnLightOn)
STUB_FUNC(L1_GetCameraLookVector)
STUB_FUNC(L1_SetCameraRoll)
STUB_FUNC(L1_SetCameraInterest)
STUB_FUNC(L1_GetCameraPosition)
STUB_FUNC(L1_SpewStartup)
STUB_FUNC(L1_PreRender)
STUB_FUNC(L1_GetSectorOppositeEdge)
STUB_FUNC(L1_PreviousSetup)
STUB_FUNC(L1_NextSetup)
STUB_FUNC(L1_WorldToScreen)
STUB_FUNC(L1_SetActorRoll)
STUB_FUNC(L1_SetActorFrustrumCull)
STUB_FUNC(L1_DriveActorTo)
STUB_FUNC(L1_GetActorRect)
STUB_FUNC(L1_SetActorTimeScale)
STUB_FUNC(L1_GetTranslationMode)
STUB_FUNC(L1_SetTranslationMode)
STUB_FUNC(L1_WalkActorToAvoiding)
STUB_FUNC(L1_GetActorChores)
STUB_FUNC(L1_SetCameraPosition)
STUB_FUNC(L1_GetCameraFOV)
STUB_FUNC(L1_SetCameraFOV)
STUB_FUNC(L1_GetCameraRoll)
STUB_FUNC(L1_ActorPuckOrient)
STUB_FUNC(L1_GetMemoryUsage)
STUB_FUNC(L1_GetFontDimensions)
STUB_FUNC(L1_PurgeText)

// Entries in the system table
static struct {
	const char *name;
	int key;
} system_defaults[] = {
	{ "frameTime", 0 },
	{ "movieTime", 0 }
};

struct luaL_reg grimMainOpcodes[] = {
	{ "EngineDisplay", L1_EngineDisplay },
	{ "CheckForFile", L1_CheckForFile },
	{ "Load", L1_Load },
	{ "Save", L1_Save },
	{ "remove", L1_Remove },
	{ "SetActorColormap", L1_SetActorColormap },
	{ "GetActorCostume", L1_GetActorCostume },
	{ "SetActorCostume", L1_SetActorCostume },
	{ "SetActorScale", L1_SetActorScale },
	{ "GetActorTimeScale", L1_GetActorTimeScale },
	{ "SetActorTimeScale", L1_SetActorTimeScale },
	{ "GetActorNodeLocation", L1_GetActorNodeLocation },
	{ "SetActorWalkChore", L1_SetActorWalkChore },
	{ "SetActorTurnChores", L1_SetActorTurnChores },
	{ "SetActorRestChore", L1_SetActorRestChore },
	{ "SetActorMumblechore", L1_SetActorMumblechore },
	{ "SetActorTalkChore", L1_SetActorTalkChore },
	{ "SetActorWalkRate", L1_SetActorWalkRate },
	{ "GetActorWalkRate", L1_GetActorWalkRate },
	{ "SetActorTurnRate", L1_SetActorTurnRate },
	{ "SetSelectedActor", L1_SetSelectedActor },
	{ "LoadActor", L1_LoadActor },
	{ "GetActorPos", L1_GetActorPos },
	{ "GetActorRect", L1_GetActorRect },
	{ "GetActorPuckVector", L1_GetActorPuckVector },
	{ "GetActorYawToPoint", L1_GetActorYawToPoint },
	{ "SetActorReflection", L1_SetActorReflection },
	{ "PutActorAtInterest", L1_PutActorAtInterest },
	{ "PutActorAt", L1_PutActorAt },
	{ "PutActorInSet", L1_PutActorInSet },
	{ "WalkActorVector", L1_WalkActorVector },
	{ "WalkActorForward", L1_WalkActorForward },
	{ "DriveActorTo", L1_DriveActorTo },
	{ "WalkActorTo", L1_WalkActorTo },
	{ "WalkActorToAvoiding", L1_WalkActorToAvoiding },
	{ "ActorLookAt", L1_ActorLookAt },
	{ "SetActorLookRate", L1_SetActorLookRate },
	{ "GetActorLookRate", L1_GetActorLookRate },
	{ "GetVisibleThings", L1_GetVisibleThings },
	{ "GetCameraActor", L1_GetCameraActor },
	{ "SetActorHead", L1_SetActorHead },
	{ "SetActorVisibility", L1_SetActorVisibility },
	{ "SetActorFollowBoxes", L1_SetActorFollowBoxes },
	{ "ShutUpActor", L1_ShutUpActor },
	{ "SetActorFrustrumCull", L1_SetActorFrustrumCull },
	{ "IsActorInSector", L1_IsActorInSector },
	{ "GetActorSector", L1_GetActorSector },
	{ "IsPointInSector", L1_IsPointInSector },
	{ "GetPointSector", L1_GetPointSector },
	{ "TurnActor", L1_TurnActor },
	{ "GetActorRot", L1_GetActorRot },
	{ "SetActorRot", L1_SetActorRot },
	{ "SetActorPitch", L1_SetActorPitch },
	{ "SetActorRoll", L1_SetActorRoll },
	{ "IsActorTurning", L1_IsActorTurning },
	{ "PlayActorChore", L1_PlayActorChore },
	{ "PlayActorChoreLooping", L1_PlayActorChoreLooping },
	{ "StopActorChore", L1_StopActorChore },
	{ "CompleteActorChore", L1_CompleteActorChore },
	{ "IsActorMoving", L1_IsActorMoving },
	{ "IsActorChoring", L1_IsActorChoring },
	{ "IsActorResting", L1_IsActorResting },
	{ "SetActorChoreLooping", L1_SetActorChoreLooping },
	{ "GetActorChores", L1_GetActorChores },
	{ "GetActorCostumeDepth", L1_GetActorCostumeDepth },
	{ "WorldToScreen", L1_WorldToScreen },
	{ "exit", L1_Exit },
	{ "FunctionName", L1_FunctionName },
	{ "EnableDebugKeys", L1_EnableDebugKeys },
	{ "LockFont", L1_LockFont },
	{ "EnableControl", L1_EnableControl },
	{ "DisableControl", L1_DisableControl },
	{ "GetControlState", L1_GetControlState },
	{ "PrintError", L1_PrintError },
	{ "PrintWarning", L1_PrintWarning },
	{ "PrintDebug", L1_PrintDebug },
	{ "MakeCurrentSet", L1_MakeCurrentSet },
	{ "LockSet", L1_LockSet },
	{ "UnLockSet", L1_UnLockSet },
	{ "MakeCurrentSetup", L1_MakeCurrentSetup },
	{ "GetCurrentSetup", L1_GetCurrentSetup },
	{ "NextSetup", L1_NextSetup },
	{ "PreviousSetup", L1_PreviousSetup },
	{ "StartFullscreenMovie", L1_StartFullscreenMovie },
	{ "IsFullscreenMoviePlaying", L1_IsFullscreenMoviePlaying },
	{ "StartMovie", L1_StartMovie },
	{ "StopMovie", L1_StopMovie },
	{ "PauseMovie", L1_PauseMovie },
	{ "IsMoviePlaying", L1_IsMoviePlaying },
	{ "PlaySound", L1_PlaySound },
	{ "PlaySoundAt", L1_PlaySoundAt },
	{ "IsSoundPlaying", L1_IsSoundPlaying },
	{ "SetSoundPosition", L1_SetSoundPosition },
	{ "FileFindFirst", L1_FileFindFirst },
	{ "FileFindNext", L1_FileFindNext },
	{ "FileFindDispose", L1_FileFindDispose },
	{ "InputDialog", L1_InputDialog },
	{ "WriteRegistryValue", L1_WriteRegistryValue },
	{ "ReadRegistryValue", L1_ReadRegistryValue },
	{ "GetSectorOppositeEdge", L1_GetSectorOppositeEdge },
	{ "MakeSectorActive", L1_MakeSectorActive },
	{ "PreRender", L1_PreRender },
	{ "SpewStartup", L1_SpewStartup },
	{ "GetCurrentScript", L1_GetCurrentScript },
	{ "PrintActorCostumes", L1_PrintActorCostumes },
	{ "PushActorCostume", L1_PushActorCostume },
	{ "PopActorCostume", L1_PopActorCostume },
	{ "LoadCostume", L1_LoadCostume },
	{ "RotateVector", L1_RotateVector },
	{ "GetCameraPosition", L1_GetCameraPosition },
	{ "SetCameraPosition", L1_SetCameraPosition },
	{ "SetCameraInterest", L1_SetCameraInterest },
	{ "GetCameraFOV", L1_GetCameraFOV },
	{ "SetCameraFOV", L1_SetCameraFOV },
	{ "GetCameraRoll", L1_GetCameraRoll },
	{ "SetCameraRoll", L1_SetCameraRoll },
	{ "GetCameraLookVector", L1_GetCameraLookVector },
	{ "PointActorAt", L1_PointActorAt },
	{ "TurnActorTo", L1_TurnActorTo },
	{ "PerSecond", L1_PerSecond },
	{ "GetAngleBetweenVectors", L1_GetAngleBetweenVectors },
	{ "GetAngleBetweenActors", L1_GetAngleBetweenActors },
	{ "SetAmbientLight", L1_SetAmbientLight },
	{ "TurnLightOn", L1_TurnLightOn },
	{ "SetLightPosition", L1_SetLightPosition },
	{ "SetLightIntensity", L1_SetLightIntensity },
	{ "LightMgrSetChange", L1_LightMgrSetChange },
	{ "LightMgrStartup", L1_LightMgrStartup },
	{ "ImStartSound", L1_ImStartSound },
	{ "ImStopSound", L1_ImStopSound },
	{ "ImStopAllSounds", L1_ImStopAllSounds },
	{ "ImGetParam", L1_ImGetParam },
	{ "ImSetParam", L1_ImSetParam },
	{ "ImFadeParam", L1_ImFadeParam },
	{ "ImGetSfxVol", L1_ImGetSfxVol },
	{ "ImSetSfxVol", L1_ImSetSfxVol },
	{ "ImGetVoiceVol", L1_ImGetVoiceVol },
	{ "ImSetVoiceVol", L1_ImSetVoiceVol },
	{ "ImGetMusicVol", L1_ImGetMusicVol },
	{ "ImSetMusicVol", L1_ImSetMusicVol },
	{ "ImSetState", L1_ImSetState },
	{ "ImSetSequence", L1_ImSetSequence },
	{ "ImPause", L1_ImPause },
	{ "ImResume", L1_ImResume },
	{ "ImSetVoiceEffect", L1_ImSetVoiceEffect },
	{ "LoadBundle", L1_LoadBundle },
	{ "SetGamma", L1_SetGamma },
	{ "SetActorWalkDominate", L1_SetActorWalkDominate },
	{ "SetActorConstrain", L1_SetActorConstrain },
	{ "RenderModeUser", L1_RenderModeUser },
	{ "ForceRefresh", L1_ForceRefresh },
	{ "DimScreen", L1_DimScreen },
	{ "DimRegion", L1_DimRegion },
	{ "CleanBuffer", L1_CleanBuffer },
	{ "Display", L1_Display },
	{ "SetSpeechMode", L1_SetSpeechMode },
	{ "GetSpeechMode", L1_GetSpeechMode },
	{ "SetShadowColor", L1_SetShadowColor },
	{ "ActivateActorShadow", L1_ActivateActorShadow },
	{ "SetActorShadowPlane", L1_SetActorShadowPlane },
	{ "SetActorShadowPoint", L1_SetActorShadowPoint },
	{ "SetActiveShadow", L1_SetActiveShadow },
	{ "KillActorShadows", L1_KillActorShadows },
	{ "AddShadowPlane", L1_AddShadowPlane },
	{ "SetActorShadowValid", L1_SetActorShadowValid },
	{ "FreeObjectState", L1_FreeObjectState },
	{ "NewObjectState", L1_NewObjectState },
	{ "SetObjectType", L1_SetObjectType },
	{ "SendObjectToBack", L1_SendObjectToBack },
	{ "SendObjectToFront", L1_SendObjectToFront },
	{ "ActorToClean", L1_ActorToClean },
	{ "FlushControls", L1_FlushControls },
	{ "SetActorCollisionMode", L1_SetActorCollisionMode },
	{ "SetActorCollisionScale", L1_SetActorCollisionScale },
	{ "SetActorClipActive", L1_SetActorClipActive },
	{ "SetActorClipPlane", L1_SetActorClipPlane },
	{ "FadeOutChore", L1_FadeOutChore },
	{ "FadeInChore", L1_FadeInChore },
	{ "IrisDown", L1_IrisDown },
	{ "IrisUp", L1_IrisUp },
	{ "TextFileGetLineCount", L1_TextFileGetLineCount },
	{ "TextFileGetLine", L1_TextFileGetLine },
	{ "ScreenShot", L1_ScreenShot },
	{ "GetSaveGameImage", L1_GetSaveGameImage },
	{ "GetImage", L1_GetImage },
	{ "FreeImage", L1_FreeImage },
	{ "BlastImage", L1_BlastImage },
	{ "BlastRect", L1_BlastRect },
	{ "SubmitSaveGameData", L1_SubmitSaveGameData },
	{ "GetSaveGameData", L1_GetSaveGameData },
	{ "SetTextSpeed", L1_SetTextSpeed },
	{ "GetTextSpeed", L1_GetTextSpeed },
	{ "DetachFromResources", L1_DetachFromResources },
	{ "AttachToResources", L1_AttachToResources },
	{ "ActorPuckOrient", L1_ActorPuckOrient },
	{ "JustLoaded", L1_JustLoaded },
	{ "ResetTextures", L1_ResetTextures },
	{ "ShrinkBoxes", L1_ShrinkBoxes },
	{ "UnShrinkBoxes", L1_UnShrinkBoxes },
	{ "GetShrinkPos", L1_GetShrinkPos },
	{ "NukeResources", L1_NukeResources },
	{ "SetActorInvClipNode", L1_SetActorInvClipNode },
	{ "GetDiskFreeSpace", L1_GetDiskFreeSpace },
	{ "SaveIMuse", L1_SaveIMuse },
	{ "RestoreIMuse", L1_RestoreIMuse },
	{ "GetMemoryUsage", L1_GetMemoryUsage },
	{ "dofile", L1_new_dofile },
};

static struct luaL_reg grimTextOpcodes[] = {
	{ "IsMessageGoing", L1_IsMessageGoing },
	{ "SetSayLineDefaults", L1_SetSayLineDefaults },
	{ "SetActorTalkColor", L1_SetActorTalkColor },
	{ "GetActorTalkColor", L1_GetActorTalkColor },
	{ "SayLine", L1_SayLine },
	{ "PrintLine", L1_PrintLine },
	{ "MakeTextObject", L1_MakeTextObject },
	{ "GetTextObjectDimensions", L1_GetTextObjectDimensions },
	{ "GetFontDimensions", L1_GetFontDimensions },
	{ "ChangeTextObject", L1_ChangeTextObject },
	{ "KillTextObject", L1_KillTextObject },
	{ "BlastText", L1_BlastText },
	{ "ExpireText", L1_ExpireText },
	{ "PurgeText", L1_PurgeText },
	{ "MakeColor", L1_MakeColor },
	{ "GetColorComponents", L1_GetColorComponents },
	{ "SetTranslationMode", L1_SetTranslationMode },
	{ "GetTranslationMode", L1_GetTranslationMode },
	{ "GetTextCharPosition", L1_GetTextCharPosition },
	{ "LocalizeString", L1_LocalizeString },
	{ "SetEmergencyFont", L1_SetEmergencyFont },
	{ "SetOffscreenTextPos", L1_SetOffscreenTextPos }
};

struct luaL_reg grimPrimitivesOpcodes[] = {
	{ "DrawLine", L1_DrawLine },
	{ "DrawPolygon", L1_DrawPolygon },
	{ "DrawRectangle", L1_DrawRectangle },
	{ "ChangePrimitive", L1_ChangePrimitive },
	{ "KillPrimitive", L1_KillPrimitive },
	{ "PurgePrimitiveQueue", L1_PurgePrimitiveQueue }
};

struct luaL_reg grimHardwareOpcodes[] = {
	{ "Is3DHardwareEnabled", L1_Is3DHardwareEnabled },
	{ "GetVideoDevices", L1_GetVideoDevices },
	{ "SetVideoDevices", L1_SetVideoDevices },
	{ "SetHardwareState", L1_SetHardwareState },
	{ "Enumerate3DDevices", L1_Enumerate3DDevices },
	{ "EnumerateVideoDevices", L1_EnumerateVideoDevices }
};

struct luaL_reg grimMiscOpcodes[] = {
	{ "  concatfallback", L1_concatFallback },
	{ "  typeoverride", L1_typeOverride },
	{ "  dfltcamera", dummyHandler },
	{ "  dfltcontrol", dummyHandler },
};

void registerGrimOpcodes() {
	// Register main opcodes functions
	luaL_openlib(grimMainOpcodes, ARRAYSIZE(grimMainOpcodes));

	// Register text opcodes functions
	luaL_openlib(grimTextOpcodes, ARRAYSIZE(grimTextOpcodes));

	// Register primitives opcodes functions
	luaL_openlib(grimPrimitivesOpcodes, ARRAYSIZE(grimPrimitivesOpcodes));

	// Register hardware opcodes functions
	luaL_openlib(grimHardwareOpcodes, ARRAYSIZE(grimHardwareOpcodes));

	// Register miscOpcodes opcodes functions
	luaL_openlib(grimMiscOpcodes, ARRAYSIZE(grimMiscOpcodes));
}

void registerLua() {
	// Register system table
	lua_Object system_table = lua_createtable();
	lua_pushobject(system_table);
	lua_setglobal("system");

	lua_pushobject(system_table);
	refSystemTable = lua_ref(1);

	for (unsigned i = 0; i < ARRAYSIZE(system_defaults); i++) {
		lua_pushobject(lua_getref(refSystemTable));
		lua_pushstring(system_defaults[i].name);
		lua_pushnumber(system_defaults[i].key);
		lua_settable();
	}

	// Create and populate system.controls table
	lua_Object controls_table = lua_createtable();
	lua_pushobject(lua_getref(refSystemTable));
	lua_pushstring("controls");
	lua_pushobject(controls_table);
	lua_settable();

	for (int i = 0; controls[i].name; i++) {
		lua_pushobject(controls_table);
		lua_pushstring(controls[i].name);
		lua_pushnumber(controls[i].key);
		lua_settable();
	}

	lua_pushobject(lua_getref(refSystemTable));
	lua_pushstring("camChangeHandler");
	lua_pushcfunction(dummyHandler);
	lua_settable();

	lua_pushobject(lua_getref(refSystemTable));
	lua_pushstring("axisHandler");
	lua_pushcfunction(dummyHandler);
	lua_settable();

	lua_pushobject(lua_getref(refSystemTable));
	lua_pushstring("buttonHandler");
	lua_pushcfunction(dummyHandler);
	lua_settable();

	lua_pushobject(lua_getglobal("type"));
	refTypeOverride = lua_ref(true);
	lua_pushCclosure(L1_typeOverride, 0);
	lua_setglobal("type");

	// Register constants for box types
	lua_pushnumber(Sector::NoneType);
	lua_setglobal("NONE");
	lua_pushnumber(Sector::WalkType);
	lua_setglobal("WALK");
	lua_pushnumber(Sector::CameraType);
	lua_setglobal("CAMERA");
	lua_pushnumber(Sector::SpecialType);
	lua_setglobal("SPECIAL");
	lua_pushnumber(Sector::HotType);
	lua_setglobal("HOT");

	lua_pushobject(lua_setfallback("concat", L1_concatFallback));
	refOldConcatFallback = lua_ref(1);

	saveCallbackPtr = saveCallback;
	restoreCallbackPtr = restoreCallback;

	// initialize Text globals
	lua_pushstring("x");
	refTextObjectX = lua_ref(true);
	lua_pushstring("y");
	refTextObjectY = lua_ref(true);
	lua_pushstring("font");
	refTextObjectFont = lua_ref(true);
	lua_pushstring("width");
	refTextObjectWidth = lua_ref(true);
	lua_pushstring("height");
	refTextObjectHeight = lua_ref(true);
	lua_pushstring("fgcolor");
	refTextObjectFGColor = lua_ref(true);
	lua_pushstring("bgcolor");
	refTextObjectBGColor = lua_ref(true);
	lua_pushstring("fxcolor");
	refTextObjectFXColor = lua_ref(true);
	lua_pushstring("hicolor");
	refTextObjectHIColor = lua_ref(true);
	lua_pushstring("duration");
	refTextObjectDuration = lua_ref(true);
	lua_pushstring("center");
	refTextObjectCenter = lua_ref(true);
	lua_pushstring("ljustify");
	refTextObjectLJustify = lua_ref(true);
	lua_pushstring("rjustify");
	refTextObjectRJustify = lua_ref(true);
	lua_pushstring("volume");
	refTextObjectVolume = lua_ref(true);
	lua_pushstring("pan");
	refTextObjectPan = lua_ref(true);
	lua_pushstring("background");
	refTextObjectBackground = lua_ref(true);
}

} // end of namespace Grim
