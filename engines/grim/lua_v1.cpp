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

#include "common/endian.h"

#include "engines/grim/lua.h"
#include "engines/grim/actor.h"
#include "engines/grim/registry.h"
#include "engines/grim/localize.h"
#include "engines/grim/colormap.h"
#include "engines/grim/costume.h"
#include "engines/grim/grim.h"
#include "engines/grim/smush/smush.h"
#include "engines/grim/savegame.h"
#include "engines/grim/lipsync.h"

#include "engines/grim/lua/luadebug.h"
#include "engines/grim/lua/lauxlib.h"

#include "engines/grim/imuse/imuse.h"

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

extern Imuse *g_imuse;

#define strmatch(src, dst)		(strlen(src) == strlen(dst) && strcmp(src, dst) == 0)

inline bool getbool(int num) {
	return !lua_isnil(lua_getparam(num));
}

inline void pushbool(bool val) {
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

byte clamp_color(int c) {
	if (c < 0)
		return 0;
	else if (c > 255)
		return 255;
	else
		return c;
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
/*	Actor *actor = static_cast<Actor *>(lua_getuserdata(actorObj));*/
	// TODO lua_pushnumber(actor->getTimeScale());
	// return 1 so the game doesn't halt when Manny attempts
	// to pick up the fire extinguisher
	lua_pushnumber(1);
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

void setDefaultObjectParams(TextObjectDefaults *defaults, lua_Object tableObj) {
	lua_Object keyObj;

	lua_pushobject(tableObj);
	lua_pushobject(lua_getref(refTextObjectX));
	keyObj = lua_gettable();
	if (keyObj) {
		if (lua_isnumber(keyObj)) {
			defaults->x = (int)lua_getnumber(keyObj);
		}
	}

	lua_pushobject(tableObj);
	lua_pushobject(lua_getref(refTextObjectY));
	keyObj = lua_gettable();
	if (keyObj) {
		if (lua_isnumber(keyObj)) {
			defaults->y = (int)lua_getnumber(keyObj);
		}
	}

	lua_pushobject(tableObj);
	lua_pushobject(lua_getref(refTextObjectFont));
	keyObj = lua_gettable();
	if (keyObj) {
		if (lua_isuserdata(keyObj) && lua_tag(keyObj) == MKTAG('F','O','N','T')) {
			defaults->font = getfont(keyObj);
		}
	}

	lua_pushobject(tableObj);
	lua_pushobject(lua_getref(refTextObjectWidth));
	keyObj = lua_gettable();
	if (keyObj) {
		if (lua_isnumber(keyObj)) {
			defaults->width = (int)lua_getnumber(keyObj);
		}
	}

	lua_pushobject(tableObj);
	lua_pushobject(lua_getref(refTextObjectHeight));
	keyObj = lua_gettable();
	if (keyObj) {
		if (lua_isnumber(keyObj)) {
			defaults->height = (int)lua_getnumber(keyObj);
		}
	}

	lua_pushobject(tableObj);
	lua_pushobject(lua_getref(refTextObjectFGColor));
	keyObj = lua_gettable();
	if (keyObj) {
		if (lua_isuserdata(keyObj) && lua_tag(keyObj) == MKTAG('C','O','L','R')) {
			defaults->fgColor = getcolor(keyObj);
		}
	}

	lua_pushobject(tableObj);
	lua_pushobject(lua_getref(refTextObjectBGColor));
	keyObj = lua_gettable();
	if (keyObj) {
		if (lua_isuserdata(keyObj) && lua_tag(keyObj) == MKTAG('C','O','L','R')) {
			//defaults->bgColor = static_cast<Color *>(lua_getuserdata(keyObj));
			warning("setDefaultObjectParams: dummy BGColor");
		}
	}

	lua_pushobject(tableObj);
	lua_pushobject(lua_getref(refTextObjectFXColor));
	keyObj = lua_gettable();
	if (keyObj) {
		if (lua_isuserdata(keyObj) && lua_tag(keyObj) == MKTAG('C','O','L','R')) {
			//defaults->fxColor = static_cast<Color *>(lua_getuserdata(keyObj));
			warning("setDefaultObjectParams: dummy FXColor");
		}
	}

	lua_pushobject(tableObj);
	lua_pushobject(lua_getref(refTextObjectCenter));
	keyObj = lua_gettable();
	if (keyObj) {
		if (!lua_isnil(keyObj)) {
			defaults->justify = 1; //5
		}
	}

	lua_pushobject(tableObj);
	lua_pushobject(lua_getref(refTextObjectLJustify));
	keyObj = lua_gettable();
	if (keyObj) {
		if (!lua_isnil(keyObj)) {
			defaults->justify = 2; //4
		}
	}

	lua_pushobject(tableObj);
	lua_pushobject(lua_getref(refTextObjectRJustify));
	keyObj = lua_gettable();
	if (keyObj) {
		if (!lua_isnil(keyObj)) {
			defaults->justify = 3; //6
		}
	}

	lua_pushobject(tableObj);
	lua_pushobject(lua_getref(refTextObjectDuration));
	keyObj = lua_gettable();
	if (keyObj) {
		if (lua_isnumber(keyObj)) {
			//defaults->duration = lua_getnumber(key);
			warning("setDefaultObjectParams: dummy Duration: %f", lua_getnumber(keyObj));
		}
	}
}

void L1_SetSayLineDefaults() {
	lua_Object tableObj = lua_getparam(1);
	if (tableObj && lua_istable(tableObj))
		setDefaultObjectParams(&g_grim->_sayLineDefaults, tableObj);
}

void L1_SetActorTalkColor() {
	lua_Object actorObj = lua_getparam(1);
	lua_Object colorObj = lua_getparam(2);

	if (!lua_isuserdata(actorObj) || lua_tag(actorObj) != MKTAG('A','C','T','R'))
		return;
	if (!lua_isuserdata(colorObj) && lua_tag(colorObj) != MKTAG('C','O','L','R'))
		return;
	Actor *actor = getactor(actorObj);
	Color *color = getcolor(colorObj);
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

	Model::HierNode *allNodes = actor->getCurrentCostume()->getModelNodes();
	Model::HierNode *node = allNodes + nodeId - 1;

	Model::HierNode *root = node;
	while (root->_parent) {
		root = root->_parent;
	}

	Graphics::Matrix4 matrix;
	matrix._pos = actor->getPos();
	matrix._rot.buildFromPitchYawRoll(actor->getPitch(), actor->getYaw(), actor->getRoll());
	root->setMatrix(matrix);
	root->update();

	lua_pushnumber(node->_matrix._pos.x());
	lua_pushnumber(node->_matrix._pos.y());
	lua_pushnumber(node->_matrix._pos.z());
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
		lua_pushstring(const_cast<char *>(costume->getFilename()));
	else
		lua_pushnil();
}

void L1_PopActorCostume() {
	lua_Object actorObj = lua_getparam(1);
	if (!lua_isuserdata(actorObj) || lua_tag(actorObj) != MKTAG('A','C','T','R'))
		return;

	Actor *actor = getactor(actorObj);
	if (actor->getCurrentCostume()) {
		lua_pushstring(const_cast<char *>(actor->getCurrentCostume()->getFilename()));
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
		int chore = costume->isChoring(excludeLoop);
		if (chore != -1) {
			lua_pushnumber(chore);
			pushbool(true);
			return;
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
	Graphics::Vector3d cameraVector = g_grim->getCurrScene()->_currSetup->_interest - g_grim->getCurrScene()->_currSetup->_pos;
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
	if (!g_grim->getCurrScene())
		return;

	Graphics::Vector3d p = g_grim->getCurrScene()->_currSetup->_interest;
	Graphics::Vector3d resultPt = p;
	float minDist = -1.f;

	for (int i = 0; i < g_grim->getCurrScene()->getSectorCount(); ++i) {
		Sector *sector = g_grim->getCurrScene()->getSectorBase(i);
		if (sector->getType() != Sector::WalkType || !sector->isVisible())
			continue;

		Graphics::Vector3d closestPt = sector->getClosestPoint(p);
		if (g_grim->getCurrScene()->findPointSector(closestPt, Sector::HotType))
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
	for (GrimEngine::ActorListType::const_iterator i = g_grim->actorsBegin(); i != g_grim->actorsEnd(); ++i) {
		Actor *a = i->_value;
		if (!i->_value->isInSet(g_grim->getSceneName()))
			continue;
		// Consider the active actor visible
		if (actor == a || actor->getAngleTo(*a) < 90) {
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

// 0 - translate from '/msgId/'
// 1 - don't translate - message after '/msgId'
// 2 - return '/msgId/'
int translationMode = 0;

Common::String parseMsgText(const char *msg, char *msgId) {
	Common::String translation = g_localizer->localize(msg);
	const char *secondSlash = NULL;

	if (msg[0] == '/' && msgId) {
		secondSlash = strchr(msg + 1, '/');
		if (secondSlash) {
			strncpy(msgId, msg + 1, secondSlash - msg - 1);
			msgId[secondSlash - msg - 1] = 0;
		} else {
			msgId[0] = 0;
		}
	}

	if (translationMode == 1)
		return secondSlash;

	if (translationMode == 2)
		return msg;

	return translation;
}

void L1_TextFileGetLine() {
	char textBuf[1000];
	lua_Object nameObj = lua_getparam(1);
	lua_Object posObj = lua_getparam(2);
	Common::SeekableReadStream *file;

	if (lua_isnil(nameObj) || lua_isnil(posObj)) {
		lua_pushnil();
		return;
	}

	const char *filename = lua_getstring(nameObj);
	Common::SaveFileManager *saveFileMan = g_system->getSavefileManager();
	file = saveFileMan->openForLoading(filename);
	if (!file) {
		lua_pushnil();
		return;
	}

	int pos = (int)lua_getnumber(posObj);
	file->seek(pos, SEEK_SET);
	memset(textBuf, 0, 1000);
	file->readLine(textBuf, 1000);
	delete file;

	lua_pushstring(textBuf);
}

void L1_TextFileGetLineCount() {
	char textBuf[1000];
	lua_Object nameObj = lua_getparam(1);

	if (lua_isnil(nameObj)) {
		lua_pushnil();
		return;
	}

	const char *filename = luaL_check_string(1);
	Common::SaveFileManager *saveFileMan = g_system->getSavefileManager();
	Common::SeekableReadStream *file = saveFileMan->openForLoading(filename);
	if (!file) {
		lua_pushnil();
		return;
	}

	lua_Object result = lua_createtable();

	int line = 0;
	for (;;) {
		if (file->eos())
			break;
		lua_pushobject(result);
		lua_pushnumber(line);
		int pos = file->pos();
		lua_pushnumber(pos);
		lua_settable();
		file->readLine(textBuf, 1000);
		line++;
	}
	delete file;

	lua_pushobject(result);
	lua_pushstring("count");
	lua_pushnumber(line);
	lua_settable();
	lua_pushobject(result);
}

// Localization function

void L1_LocalizeString() {
	char msgId[50], buf[1000];
	lua_Object strObj = lua_getparam(1);

	if (lua_isstring(strObj)) {
		const char *str = lua_getstring(strObj);
		// If the string that we're passed isn't localized yet then
		// construct the localized string, otherwise spit back what
		// we've been given
		if (str[0] == '/' && str[strlen(str) - 1] == '/') {
			Common::String msg = parseMsgText(str, msgId);
			sprintf(buf, "/%s/%s", msgId, msg.c_str());
			str = buf;
		}
		lua_pushstring(str);
	}
}

void parseSayLineTable(lua_Object paramObj, bool *background, int *vol, int *pan, int *x, int *y) {
	lua_Object tableObj;

	lua_pushobject(paramObj);
	lua_pushobject(lua_getref(refTextObjectX));
	tableObj = lua_gettable();
	if (lua_isnumber(tableObj)) {
		if (*x)
			*x = (int)lua_getnumber(tableObj);
	}

	lua_pushobject(paramObj);
	lua_pushobject(lua_getref(refTextObjectY));
	tableObj = lua_gettable();
	if (lua_isnumber(tableObj)) {
		if (*y)
			*y = (int)lua_getnumber(tableObj);
	}

	lua_pushobject(paramObj);
	lua_pushobject(lua_getref(refTextObjectBackground));
	tableObj = lua_gettable();
	if (!lua_isnil(tableObj)) {
		if (*background)
			*background = 0;
	}

	lua_pushobject(paramObj);
	lua_pushobject(lua_getref(refTextObjectVolume));
	tableObj = lua_gettable();
	if (lua_isnumber(tableObj)) {
		if (*vol)
			*vol = (int)lua_getnumber(tableObj);
	}

	lua_pushobject(paramObj);
	lua_pushobject(lua_getref(refTextObjectPan));
	tableObj = lua_gettable();
	if (lua_isnumber(tableObj)) {
		if (*pan)
			*pan = (int)lua_getnumber(tableObj);
	}
}

void L1_SayLine() {
	int vol = 127, buffer = 64, paramId = 1, x = -1, y = -1;
	bool background = true;
	char msgId[50];
	Common::String msg;
	lua_Object paramObj = lua_getparam(paramId++);

	if ((lua_isuserdata(paramObj) && lua_tag(paramObj) == MKTAG('A','C','T','R'))
			|| lua_isstring(paramObj) || lua_istable(paramObj)) {
		Actor *actor = NULL;//some_Actor, maybe some current actor
		if (lua_isuserdata(paramObj) && lua_tag(paramObj) == MKTAG('A','C','T','R')) {
			actor = getactor(paramObj);
			paramObj = lua_getparam(paramId++);
		}
		if (actor) {
			while (!lua_isnil(paramObj)) {
				if (!lua_isstring(paramObj) && !lua_isnumber(paramObj) && !lua_istable(paramObj))
					break;
				if (lua_istable(paramObj))
					parseSayLineTable(paramObj, &background, &vol, &buffer, &x, &y);
				else {
					if (lua_isnumber(paramObj))
						background = false;
					else {
						const char *tmpstr = lua_getstring(paramObj);
						msg = parseMsgText(tmpstr, msgId);
					}
				}
				paramObj = lua_getparam(paramId++);
			}
			if (!msg.empty()) {
				actor->sayLine(msg.c_str(), msgId); //background, vol, pan, x, y
			}
		}
	}
}

void L1_PrintLine() {
	int vol = 127, buffer = 64, /*paramId = 1, */x = -1, y = -1;
	bool background = true;
	char msgId[50];
	Common::String msg;
	lua_Object param1Obj = lua_getparam(0);
	lua_Object param2Obj = lua_getparam(1);

	if ((lua_isstring(param1Obj) || lua_isnil(param1Obj)) && (lua_istable(param2Obj) || lua_isnil(param2Obj))) {
		if (lua_istable(param2Obj)) {
			setDefaultObjectParams(&g_grim->_printLineDefaults, param2Obj);
			parseSayLineTable(param2Obj, &background, &vol, &buffer, &x, &y);
		}
		if (lua_isstring(param1Obj)) {
			const char *tmpstr = lua_getstring(param1Obj);
			msg = parseMsgText(tmpstr, msgId);
		}
		if (!msg.empty()) {
//			actor->sayLine(msg.c_str(), msgId); //background, vol, pan, x, y
		}
	}
}

void L1_InputDialog() {
	lua_Object str1Obj = lua_getparam(1);
	lua_Object str2Obj = lua_getparam(2);
	int c, i = 0;
	char buf[512];

	if (!lua_isstring(str1Obj) || !lua_isstring(str2Obj)) {
		lua_pushnil();
		return;
	}
	fprintf(stderr, "%s %s: ", lua_getstring(str1Obj), lua_getstring(str2Obj));
	while (i < 512 && (c = fgetc(stdin)) != EOF && c != '\n')
		buf[i++] = c;
	buf[i] = '\0';

	lua_pushstring(buf);
}

void L1_IsMessageGoing() {
	lua_Object actorObj = lua_getparam(1);

	if (!actorObj || (lua_isuserdata(actorObj) && lua_tag(actorObj) == MKTAG('A','C','T','R')) || lua_isnil(actorObj)) {
		if (lua_isuserdata(actorObj) && lua_tag(actorObj) == MKTAG('A','C','T','R')) {
			Actor *actor = getactor(actorObj);
			if (actor) {
				pushbool(actor->isTalking());
			}
		} else {
			// TODO
			// this part code check something more
			pushbool(g_imuse->isVoicePlaying());
		}
	} else
		lua_pushnil();
}

void L1_ShutUpActor() {
	lua_Object actorObj = lua_getparam(1);

	if (!lua_isuserdata(actorObj) || lua_tag(actorObj) != MKTAG('A','C','T','R'))
		return;
	Actor *actor = getactor(actorObj);
	if (actor)
		actor->shutUp();
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

// Sound functions

enum ImuseParam {
	IM_SOUND_PLAY_COUNT = 0x100,
	IM_SOUND_PEND_COUNT = 0x200,
	IM_SOUND_GROUP = 0x400,
	IM_SOUND_PRIORITY = 0x500,
	IM_SOUND_VOL = 0x600,
	IM_SOUND_PAN = 0x700
};

void L1_ImStartSound() {
	lua_Object nameObj = lua_getparam(1);
	lua_Object priorityObj = lua_getparam(2);
	lua_Object groupObj = lua_getparam(3);

	if (!lua_isstring(nameObj) && !lua_isnumber(nameObj))
		return;
	if (!lua_isnumber(priorityObj) || !lua_isnumber(groupObj))
		return;

	const char *soundName = lua_getstring(nameObj);
	int priority = (int)lua_getnumber(priorityObj);
	int group = (int)lua_getnumber(groupObj);

	// Start the sound with the appropriate settings
	if (g_imuse->startSound(soundName, group, 0, 127, 0, priority, NULL)) {
		// FIXME actually it's pushnumber from result of startSound
		lua_pushstring(soundName);
	}
}

void L1_ImStopSound() {
	lua_Object nameObj = lua_getparam(1);
	if (lua_isnumber(nameObj))
		error("ImStopsound: name from value not supported");

	const char *soundName = lua_getstring(nameObj);
	g_imuse->stopSound(soundName);
}

void L1_ImStopAllSounds() {
	g_imuse->stopAllSounds();
}

void L1_ImPause() {
	g_imuse->pause(true);
}

void L1_ImResume() {
	g_imuse->pause(false);
}

void L1_ImSetVoiceEffect() {
	const char *effectName;

	effectName = luaL_check_string(1);
	if (gDebugLevel == DEBUG_WARN || gDebugLevel == DEBUG_ALL)
		warning("ImSetVoiceEffect(%s) Voice effects are not yet supported", effectName);
}

void L1_ImSetMusicVol() {
	lua_Object volObj = lua_getparam(1);
	if (!lua_isnumber(volObj))
		return;
	g_system->getMixer()->setVolumeForSoundType(Audio::Mixer::kMusicSoundType, (int)lua_getnumber(volObj));
}

void L1_ImGetMusicVol() {
	lua_pushnumber(g_system->getMixer()->getVolumeForSoundType(Audio::Mixer::kMusicSoundType));
}

void L1_ImSetVoiceVol() {
	lua_Object volObj = lua_getparam(1);
	if (!lua_isnumber(volObj))
		return;
	g_system->getMixer()->setVolumeForSoundType(Audio::Mixer::kSpeechSoundType, (int)lua_getnumber(volObj));
}

void L1_ImGetVoiceVol() {
	lua_pushnumber(g_system->getMixer()->getVolumeForSoundType(Audio::Mixer::kSpeechSoundType));
}

void L1_ImSetSfxVol() {
	lua_Object volObj = lua_getparam(1);
	if (!lua_isnumber(volObj))
		return;
	g_system->getMixer()->setVolumeForSoundType(Audio::Mixer::kSFXSoundType, (int)lua_getnumber(volObj));
}

void L1_ImGetSfxVol() {
	lua_pushnumber(g_system->getMixer()->getVolumeForSoundType(Audio::Mixer::kSFXSoundType));
}

void L1_ImSetParam() {
	lua_Object nameObj = lua_getparam(1);
	lua_Object paramObj = lua_getparam(2);
	lua_Object valueObj = lua_getparam(3);

	if (lua_isnumber(nameObj))
		error("ImSetParam: getting name from number is not supported");
	if (!lua_isstring(nameObj)) {
		lua_pushnumber(-1.0);
		return;
	}

	const char *soundName = lua_getstring(nameObj);
	int param = (int)lua_getnumber(paramObj);
	int value = (int)lua_getnumber(valueObj);
	if (value < 0)
		value = 0;
	switch (param) {
	case IM_SOUND_VOL:
		g_imuse->setVolume(soundName, value);
		break;
	case IM_SOUND_PAN:
		g_imuse->setPan(soundName, value);
		break;
	default:
		error("ImSetParam() Unimplemented %d", param);
	}
}

void L1_ImGetParam() {
	lua_Object nameObj = lua_getparam(1);
	lua_Object paramObj = lua_getparam(2);

	if (lua_isnumber(nameObj))
		error("ImGetParam: getting name from number is not supported");
	if (!lua_isstring(nameObj)) {
		lua_pushnumber(-1.0);
		return;
	}

	const char *soundName = lua_getstring(nameObj);
	int param = (int)lua_getnumber(paramObj);
	switch (param) {
	case IM_SOUND_PLAY_COUNT:
		lua_pushnumber(g_imuse->getCountPlayedTracks(soundName));
		break;
	case IM_SOUND_VOL:
		lua_pushnumber(g_imuse->getVolume(soundName));
		break;
	default:
		error("ImGetParam() Unimplemented %d", param);
	}
}

void L1_ImFadeParam() {
	lua_Object nameObj = lua_getparam(1);
	lua_Object opcodeObj = lua_getparam(2);
	lua_Object valueObj = lua_getparam(3);
	lua_Object durationObj = lua_getparam(4);

	if (!lua_isstring(nameObj) && !lua_isnumber(nameObj)) {
		lua_pushnumber(0);
		return;
	}
	if (!lua_isnumber(opcodeObj) || !lua_isnumber(valueObj) || !lua_isnumber(durationObj))
		return;

	if (lua_isnumber(nameObj)) {
		error("ImFadeParam: getting name from number is not supported");
	}
	const char *soundName = lua_getstring(nameObj);
	int opcode = (int)lua_getnumber(opcodeObj);
	int value = (int)lua_getnumber(valueObj);
	if (value < 0)
		value = 0;
	int duration = (int)lua_getnumber(durationObj);
	switch (opcode) {
	case IM_SOUND_PAN:
		g_imuse->setFadePan(soundName, value, duration);
		break;
	default:
		error("ImFadeParam(%s, %x, %d, %d)", soundName, opcode, value, duration);
		break;
	}
}

void L1_ImSetState() {
	lua_Object stateObj = lua_getparam(1);
	if (!lua_isnumber(stateObj))
		return;

	g_imuseState = (int)lua_getnumber(stateObj);
}

void L1_ImSetSequence() {
	lua_Object stateObj = lua_getparam(1);
	if (!lua_isnumber(stateObj))
		return;

	int state = (int)lua_getnumber(stateObj);
	lua_pushnumber(g_imuse->setMusicSequence(state));
}

void L1_SaveIMuse() {
	SaveGame *savedIMuse = new SaveGame("grim.tmp", true);
	if (!savedIMuse)
		return;
	g_imuse->saveState(savedIMuse);
	delete savedIMuse;
}

void L1_RestoreIMuse() {
	SaveGame *savedIMuse = new SaveGame("grim.tmp", false);
	if (!savedIMuse)
		return;
	g_imuse->stopAllSounds();
	g_imuse->resetState();
	g_imuse->restoreState(savedIMuse);
	delete savedIMuse;
	g_system->getSavefileManager()->removeSavefile("grim.tmp");
}

void L1_SetSoundPosition() {
	Graphics::Vector3d pos;
	int minVolume = 10;
	int maxVolume = 127;
	float someParam = 0;
	int argId = 1;
	lua_Object paramObj;

	if (g_grim->getCurrScene()) {
		g_grim->getCurrScene()->getSoundParameters(&minVolume, &maxVolume);
	}

	lua_Object nameObj = lua_getparam(argId++);
	if (!lua_isnumber(nameObj) && !lua_isstring(nameObj))
		return;

	lua_Object actorObj = lua_getparam(argId++);
	if (lua_isuserdata(actorObj) && lua_tag(actorObj) == MKTAG('A','C','T','R')) {
		Actor *actor = getactor(actorObj);
		if (!actor)
			return;
		pos = actor->getPos();
	} else if (lua_isnumber(actorObj)) {
		float x = lua_getnumber(actorObj);
		float y = lua_getnumber(argId++);
		float z = lua_getnumber(argId++);
		pos.set(x, y, z);
	}

	paramObj = (int)lua_getparam(argId++);
	if (lua_isnumber(paramObj)) {
		minVolume = (int)lua_getnumber(paramObj);
		if (minVolume > 127)
			minVolume = 127;
	}
	paramObj = lua_getparam(argId++);
	if (lua_isnumber(paramObj)) {
		maxVolume = (int)lua_getnumber(paramObj);
		if (maxVolume > 127)
			maxVolume = 127;
		else if (maxVolume < minVolume)
			maxVolume = minVolume;
	}

	paramObj = lua_getparam(argId++);
	if (lua_isnumber(paramObj)) {
		someParam = (int)lua_getnumber(paramObj);
		if (someParam < 0.0)
			someParam = 0.0;
	}

	if (g_grim->getCurrScene()) {
		if (lua_isnumber(nameObj))
			error("SetSoundPosition: number is not yet supported");
		else {
			const char *soundName = lua_getstring(nameObj);
			g_grim->getCurrScene()->setSoundPosition(soundName, pos, minVolume, maxVolume);
		}
	}
}

void L1_IsSoundPlaying() {
	// dummy
}

void L1_PlaySoundAt() {
	// dummy
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

#if 0
void L1_killBitmapPrimitives(Bitmap *bitmap) {
	for (GrimEngine::PrimitiveListType::const_iterator i = g_grim->primitivesBegin(); i != g_grim->primitivesEnd(); ++i) {
		PrimitiveObject *p = i->_value;
		if (p->isBitmap() && p->getBitmapHandle() == bitmap) {
			g_grim->killPrimitiveObject(p);
			break;
		}
	}
}
#endif

void L1_GetImage() {
	lua_Object nameObj = lua_getparam(1);
	if (!lua_isstring(nameObj)) {
		lua_pushnil();
		return;
	}
	const char *bitmapName = lua_getstring(nameObj);
	Bitmap *b = g_resourceloader->loadBitmap(bitmapName);
	g_grim->registerBitmap(b);
	lua_pushusertag(b->getId(), MKTAG('V','B','U','F'));
}

void L1_FreeImage() {
	lua_Object param = lua_getparam(1);
	if (!lua_isuserdata(param) || lua_tag(param) != MKTAG('V','B','U','F'))
		return;
	Bitmap *bitmap = g_grim->getBitmap(lua_getuserdata(param));
	g_grim->killBitmap(bitmap);
}

void L1_BlastImage() {
	lua_Object param = lua_getparam(1);
	if (!lua_isuserdata(param) || lua_tag(param) != MKTAG('V','B','U','F'))
		return;
	Bitmap *bitmap = g_grim->getBitmap(lua_getuserdata(param));
	lua_Object xObj = lua_getparam(2);
	lua_Object yObj = lua_getparam(3);
	if (!lua_isnumber(xObj) || !lua_isnumber(yObj))
		return;

	int x = (int)lua_getnumber(xObj);
	int y = (int)lua_getnumber(yObj);
//	bool transparent = getbool(4); // TODO transparent/masked copy into display
	bitmap->setX(x);
	bitmap->setY(y);
	g_driver->createBitmap(bitmap);
	g_driver->drawBitmap(bitmap);
}

void setTextObjectParams(TextObject *textObject, lua_Object tableObj) {
	lua_Object keyObj;

	lua_pushobject(tableObj);
	lua_pushobject(lua_getref(refTextObjectX));
	keyObj = lua_gettable();
	if (keyObj) {
		if (lua_isnumber(keyObj)) {
			textObject->setX((int)lua_getnumber(keyObj));
		}
	}

	lua_pushobject(tableObj);
	lua_pushobject(lua_getref(refTextObjectY));
	keyObj = lua_gettable();
	if (keyObj) {
		if (lua_isnumber(keyObj)) {
			textObject->setY((int)lua_getnumber(keyObj));
		}
	}

	lua_pushobject(tableObj);
	lua_pushobject(lua_getref(refTextObjectFont));
	keyObj = lua_gettable();
	if (keyObj) {
		if (lua_isuserdata(keyObj) && lua_tag(keyObj) == MKTAG('F','O','N','T')) {
			textObject->setFont(getfont(keyObj));
		}
	}

	lua_pushobject(tableObj);
	lua_pushobject(lua_getref(refTextObjectWidth));
	keyObj = lua_gettable();
	if (keyObj) {
		if (lua_isnumber(keyObj)) {
			textObject->setWidth((int)lua_getnumber(keyObj));
		}
	}

	lua_pushobject(tableObj);
	lua_pushobject(lua_getref(refTextObjectHeight));
	keyObj = lua_gettable();
	if (keyObj) {
		if (lua_isnumber(keyObj)) {
			textObject->setHeight((int)lua_getnumber(keyObj));
		}
	}

	lua_pushobject(tableObj);
	lua_pushobject(lua_getref(refTextObjectFGColor));
	keyObj = lua_gettable();
	if (keyObj) {
		if (lua_isuserdata(keyObj) && lua_tag(keyObj) == MKTAG('C','O','L','R')) {
			textObject->setFGColor(getcolor(keyObj));
		}
	}

	lua_pushobject(tableObj);
	lua_pushobject(lua_getref(refTextObjectBGColor));
	keyObj = lua_gettable();
	if (keyObj) {
		if (lua_isuserdata(keyObj) && lua_tag(keyObj) == MKTAG('C','O','L','R')) {
			//textObject->setBGColor(static_cast<Color *>(lua_getuserdata(keyObj)));
			warning("setTextObjectParams: dummy BGColor");
		}
	}

	lua_pushobject(tableObj);
	lua_pushobject(lua_getref(refTextObjectFXColor));
	keyObj = lua_gettable();
	if (keyObj) {
		if (lua_isuserdata(keyObj) && lua_tag(keyObj) == MKTAG('C','O','L','R')) {
			//textObject->setFXColor(static_cast<Color *>(lua_getuserdata(keyObj)));
			warning("setTextObjectParams: dummy FXColor");
		}
	}

	lua_pushobject(tableObj);
	lua_pushobject(lua_getref(refTextObjectCenter));
	keyObj = lua_gettable();
	if (keyObj) {
		if (!lua_isnil(keyObj)) {
			textObject->setJustify(1); //5
		}
	}

	lua_pushobject(tableObj);
	lua_pushobject(lua_getref(refTextObjectLJustify));
	keyObj = lua_gettable();
	if (keyObj) {
		if (!lua_isnil(keyObj)) {
			textObject->setJustify(2); //4
		}
	}

	lua_pushobject(tableObj);
	lua_pushobject(lua_getref(refTextObjectRJustify));
	keyObj = lua_gettable();
	if (keyObj) {
		if (!lua_isnil(keyObj)) {
			textObject->setJustify(3); //6
		}
	}

	lua_pushobject(tableObj);
	lua_pushobject(lua_getref(refTextObjectDuration));
	keyObj = lua_gettable();
	if (keyObj) {
		if (lua_isnumber(keyObj)) {
			//textObject->setDuration(lua_getnumber(key));
			warning("setTextObjectParams: dummy Duration: %d", (int)lua_getnumber(keyObj));
		}
	}
}

void L1_CleanBuffer() {
	g_driver->copyStoredToDisplay();
}

void L1_Exit() {
	g_grim->quitGame();
}

/* Destroy a text object since we don't need it anymore
 * note that the menu creates more objects than it needs,
 * so it deletes some objects right after creating them
 */
void L1_KillTextObject() {
	lua_Object textObj = lua_getparam(1);

	if (lua_isuserdata(textObj) && lua_tag(textObj) == MKTAG('T', 'E', 'X', 'T')) {
		TextObject *textObject = gettextobject(textObj);
		textObject->setDisabled(true);
	}
}

/* Make changes to a text object based on the parameters passed
 * in the table in the LUA parameter 2.
 */
void L1_ChangeTextObject() {
	const char *line;
	lua_Object textObj = lua_getparam(1);
	int paramId = 2;
	if (lua_isuserdata(textObj) && lua_tag(textObj) == MKTAG('T', 'E', 'X', 'T')) {
		TextObject *textObject = gettextobject(textObj);
		do {
			lua_Object paramObj = lua_getparam(paramId++);
			if (!paramObj)
				break;
			if (!lua_isstring(paramObj)) {
				if (!lua_istable(paramObj))
					break;
				setTextObjectParams(textObject, paramObj);
			} else {
				line = lua_getstring(paramObj);
				textObject->setText(line);
				lua_getstring(paramObj);

			}
			textObject->destroyBitmap();
			textObject->createBitmap();

			lua_pushnumber(textObject->getBitmapWidth());
			lua_pushnumber(textObject->getBitmapHeight());
		} while (false);
	}
}

/* Return the "text speed", this option must be handled
 * to prevent errors in the "Options" menu even though
 * we're not currently using the value
 */
void L1_GetTextSpeed() {
	lua_pushnumber(g_grim->getTextSpeed());
}

void L1_SetTextSpeed() {
	lua_Object speedObj = lua_getparam(1);
	if (!lua_isnumber(speedObj))
		return;

	int speed = (int)lua_getnumber(speedObj);
	g_grim->setTextSpeed(speed);
}

void L1_MakeTextObject() {
	lua_Object textObj = lua_getparam(1);
	if (!lua_isstring(textObj)) {
		return;
	}

	TextObject *textObject = new TextObject(false);
	const char *line = lua_getstring(textObj);
	Common::String text = line;

	textObject->setDefaults(&g_grim->_blastTextDefaults);
	lua_Object tableObj = lua_getparam(2);
	if (lua_istable(tableObj))
		setTextObjectParams(textObject, tableObj);

	textObject->setText(text.c_str());
	if (!(g_grim->getGameFlags() & GF_DEMO))
		textObject->createBitmap();
	g_grim->registerTextObject(textObject);

	lua_pushusertag(textObject->getId(), MKTAG('T', 'E', 'X', 'T'));
	if (!(g_grim->getGameFlags() & GF_DEMO)) {
		lua_pushnumber(textObject->getBitmapWidth());
		lua_pushnumber(textObject->getBitmapHeight());
	}
}

void L1_GetTextObjectDimensions() {
	lua_Object textObj = lua_getparam(1);

	if (lua_isuserdata(textObj) && lua_tag(textObj) == MKTAG('T', 'E', 'X', 'T')) {
		TextObject *textObject = gettextobject(textObj);
		lua_pushnumber(textObject->getBitmapWidth());
		lua_pushnumber(textObject->getBitmapHeight());
	}
}

void L1_ExpireText() {
	// Expire all the text objects
	for (GrimEngine::TextListType::const_iterator i = g_grim->textsBegin(); i != g_grim->textsEnd(); ++i)
		i->_value->setDisabled(true);

	// Cleanup actor references to deleted text objects
	for (GrimEngine::ActorListType::const_iterator i = g_grim->actorsBegin(); i != g_grim->actorsEnd(); ++i)
		i->_value->lineCleanup();
}

void L1_GetTextCharPosition() {
	lua_Object textObj = lua_getparam(1);
	if (lua_isuserdata(textObj) && lua_tag(textObj) == MKTAG('T', 'E', 'X', 'T')) {
		TextObject *textObject = gettextobject(textObj);
		int pos = (int)lua_getnumber(lua_getparam(2));
		lua_pushnumber(textObject->getTextCharPosition(pos));
	}
}

void L1_BlastText() {
	lua_Object textObj = lua_getparam(1);
	if (!lua_isstring(textObj)) {
		return;
	}

	const char *line = lua_getstring(textObj);
	Common::String text = line;

	TextObject *textObject = new TextObject(true);
	textObject->setDefaults(&g_grim->_blastTextDefaults);
	lua_Object tableObj = lua_getparam(2);
	if (lua_istable(tableObj))
		setTextObjectParams(textObject, tableObj);

	textObject->setText(text.c_str());
	textObject->createBitmap();
	textObject->draw();
	delete textObject;
}

void L1_SetOffscreenTextPos() {
	// this sets where we shouldn't put dialog maybe?
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

void L1_StartFullscreenMovie() {
	bool looping = getbool(2);

	lua_Object name = lua_getparam(1);
	if (!lua_isstring(name)) {
		lua_pushnil();
		return;
	}
	L1_CleanBuffer();
	g_grim->setMode(ENGINE_MODE_SMUSH);
	pushbool(g_smush->play(lua_getstring(name), looping, 0, 0));
}

void L1_StartMovie() {
	bool looping = getbool(2);
	int x = 0, y = 0;

	lua_Object name = lua_getparam(1);
	if (!lua_isstring(name)) {
		lua_pushnil();
		return;
	}
	if (!lua_isnil(lua_getparam(3)))
		x = (int)lua_getnumber(lua_getparam(3));

	if (!lua_isnil(lua_getparam(4)))
		y = (int)lua_getnumber(lua_getparam(4));

	g_grim->setMode(ENGINE_MODE_NORMAL);
	pushbool(g_smush->play(lua_getstring(name), looping, x, y));
}

/* Fullscreen movie playing query and normal movie
 * query should actually detect correctly and not
 * just return true whenever ANY movie is playing
 */
void L1_IsFullscreenMoviePlaying() {
	pushbool(g_smush->isPlaying() && g_grim->getMode() == ENGINE_MODE_SMUSH);
}

void L1_IsMoviePlaying() {
	if (g_grim->getGameFlags() & GF_DEMO)
		pushbool(g_smush->isPlaying());
	else
		pushbool(g_smush->isPlaying() && g_grim->getMode() == ENGINE_MODE_NORMAL);
}

void L1_StopMovie() {
	g_smush->stop();
}

void L1_PauseMovie() {
	g_smush->pause(lua_isnil(lua_getparam(1)) == 0);
}

void L1_PurgePrimitiveQueue() {
	g_grim->killPrimitiveObjects();
}

void L1_DrawPolygon() {
	lua_Object pointObj;
	Common::Point p1, p2, p3, p4;
	Color *color = NULL;

	lua_Object tableObj1 = lua_getparam(1);
	if (!lua_istable(tableObj1)) {
		lua_pushnil();
		return;
	}

	int layer = 2;
	lua_Object tableObj2 = lua_getparam(2);
	if (lua_istable(tableObj2)) {
		lua_pushobject(tableObj2);
		lua_pushstring("color");
		lua_Object colorObj = lua_gettable();
		if (lua_isuserdata(colorObj) && lua_tag(colorObj) == MKTAG('C','O','L','R')) {
			color = getcolor(colorObj);
		}
		lua_pushobject(tableObj2);
		lua_pushstring("layer");
		lua_Object layerObj = lua_gettable();
		if (lua_isnumber(layerObj))
			layer = (int)lua_getnumber(layerObj);
	}

	// This code support static 4 points polygon as game doesn't use other than that.
	// However original engine can support many points per polygon
	lua_pushobject(tableObj1);
	lua_pushnumber(1);
	pointObj = lua_gettable();
	p1.x = (int)lua_getnumber(pointObj);
	lua_pushobject(tableObj1);
	lua_pushnumber(2);
	pointObj = lua_gettable();
	p1.y = (int)lua_getnumber(pointObj);
	lua_pushobject(tableObj1);
	lua_pushnumber(3);
	pointObj = lua_gettable();
	p2.x = (int)lua_getnumber(pointObj);
	lua_pushobject(tableObj1);
	lua_pushnumber(4);
	pointObj = lua_gettable();
	p2.y = (int)lua_getnumber(pointObj);
	lua_pushobject(tableObj1);
	lua_pushnumber(5);
	pointObj = lua_gettable();
	p3.x = (int)lua_getnumber(pointObj);
	lua_pushobject(tableObj1);
	lua_pushnumber(6);
	pointObj = lua_gettable();
	p3.y = (int)lua_getnumber(pointObj);
	lua_pushobject(tableObj1);
	lua_pushnumber(7);
	pointObj = lua_gettable();
	p4.x = (int)lua_getnumber(pointObj);
	lua_pushobject(tableObj1);
	lua_pushnumber(8);
	pointObj = lua_gettable();
	p4.y = (int)lua_getnumber(pointObj);

	PrimitiveObject *p = new PrimitiveObject();
	p->createPolygon(p1, p2, p3, p4, color);
	g_grim->registerPrimitiveObject(p);
	lua_pushusertag(p->getId(), MKTAG('P','R','I','M'));
}

void L1_DrawLine() {
	Common::Point p1, p2;
	Color *color = NULL;;
	lua_Object x1Obj = lua_getparam(1);
	lua_Object y1Obj = lua_getparam(2);
	lua_Object x2Obj = lua_getparam(3);
	lua_Object y2Obj = lua_getparam(4);
	lua_Object tableObj = lua_getparam(5);

	if (!lua_isnumber(x1Obj) || !lua_isnumber(y1Obj) || !lua_isnumber(x2Obj) || !lua_isnumber(y2Obj)) {
		lua_pushnil();
		return;
	}

	p1.x = (int)lua_getnumber(x1Obj);
	p1.y = (int)lua_getnumber(y1Obj);
	p2.x = (int)lua_getnumber(x2Obj);
	p2.y = (int)lua_getnumber(y2Obj);

	int layer = 2;
	if (lua_istable(tableObj)) {
		lua_pushobject(tableObj);
		lua_pushstring("color");
		lua_Object colorObj = lua_gettable();
		if (lua_isuserdata(colorObj) && lua_tag(colorObj) == MKTAG('C','O','L','R')) {
			color = getcolor(colorObj);
		}
		lua_pushobject(tableObj);
		lua_pushstring("layer");
		lua_Object layerObj = lua_gettable();
		if (lua_isnumber(layerObj))
			layer = (int)lua_getnumber(layerObj);
	}

	PrimitiveObject *p = new PrimitiveObject();
	p->createLine(p1, p2, color); // TODO Add layer support
	g_grim->registerPrimitiveObject(p);
	lua_pushusertag(p->getId(), MKTAG('P','R','I','M'));
}

void L1_ChangePrimitive() {
	PrimitiveObject *psearch, *pmodify = NULL;
	Color *color = NULL;

	lua_Object param1 = lua_getparam(1);
	if (!lua_isuserdata(param1) || lua_tag(param1) != MKTAG('P','R','I','M'))
		return;

	lua_Object tableObj = lua_getparam(2);
	if (!lua_istable(tableObj))
		return;

	psearch = getprimitive(param1);

	for (GrimEngine::PrimitiveListType::const_iterator i = g_grim->primitivesBegin(); i != g_grim->primitivesEnd(); ++i) {
		PrimitiveObject *p = i->_value;
		if (p->getP1().x == psearch->getP1().x && p->getP2().x == psearch->getP2().x
				&& p->getP1().y == psearch->getP1().y && p->getP2().y == psearch->getP2().y) {
			pmodify = p;
			break;
		}
	}
	assert(pmodify);

	lua_pushobject(tableObj);
	lua_pushstring("color");
	lua_Object colorObj = lua_gettable();
	if (lua_isuserdata(colorObj) && lua_tag(colorObj) == MKTAG('C','O','L','R')) {
		color = getcolor(colorObj);
		pmodify->setColor(color);
	}

	lua_pushobject(tableObj);
	lua_pushstring("layer");
	lua_Object layer = lua_gettable();
	if (lua_isnumber(layer)) {
		// TODO pmodify->setLayer(lua_getnumber(layer));
		warning("Not implemented: PrimitiveObject::setLayer. Layer: %d", (int)lua_getnumber(layer));
	}

	lua_pushobject(tableObj);
	lua_pushstring("xoffset");
	lua_Object xoffset = lua_gettable();
	lua_pushobject(tableObj);
	lua_pushstring("yoffset");
	lua_Object yoffset = lua_gettable();
	if (lua_isnumber(xoffset) || lua_isnumber(yoffset)) {
		int x = 0;
		int y = 0;
		if (lua_isnumber(xoffset))
			x = (int)lua_getnumber(xoffset);
		if (lua_isnumber(yoffset))
			y = (int)lua_getnumber(yoffset);
		// TODO pmodify->setOffets(x, y);
		assert(0);
	}

	lua_pushobject(tableObj);
	lua_pushstring("x");
	lua_Object xobj = lua_gettable();
	lua_pushobject(tableObj);
	lua_pushstring("y");
	lua_Object yobj = lua_gettable();
	if (lua_isnumber(xobj) || lua_isnumber(yobj)) {
		int x = -1;
		int y = -1;
		if (lua_isnumber(xobj))
			x = (int)lua_getnumber(xobj);
		if (lua_isnumber(yobj))
			y = (int)lua_getnumber(yobj);
		pmodify->setPos(x, y);
	}

	lua_pushobject(tableObj);
	lua_pushstring("x2");
	lua_Object x2 = lua_gettable();
	lua_pushobject(tableObj);
	lua_pushstring("y2");
	lua_Object y2 = lua_gettable();
	if (lua_isnumber(x2) || lua_isnumber(y2)) {
		int x = -1;
		int y = -1;
		if (lua_isnumber(x2))
			x = (int)lua_getnumber(x2);
		if (lua_isnumber(y2))
			y = (int)lua_getnumber(y2);
		// TODO pmodify->setSize(x, y);
		assert(0);
	}

	lua_pushobject(tableObj);
	lua_pushstring("width");
	lua_Object width = lua_gettable();
	lua_pushobject(tableObj);
	lua_pushstring("height");
	lua_Object height = lua_gettable();
	if (lua_isnumber(width) || lua_isnumber(height)) {
		int x = -1;
		int y = -1;
		if (lua_isnumber(width))
			x = (int)lua_getnumber(width);
		if (lua_isnumber(height))
			y = (int)lua_getnumber(height);
		// TODO pmodify->setSize(x, y);
		assert(0);
	}
}

void L1_DrawRectangle() {
	Common::Point p1, p2;
	Color *color = NULL;
	lua_Object objX1 = lua_getparam(1);
	lua_Object objY1 = lua_getparam(2);
	lua_Object objX2 = lua_getparam(3);
	lua_Object objY2 = lua_getparam(4);
	lua_Object tableObj = lua_getparam(5);

	if (!lua_isnumber(objX1) || !lua_isnumber(objY1) || !lua_isnumber(objX2) || !lua_isnumber(objY2)) {
		lua_pushnil();
		return;
	}
	p1.x = (int)lua_getnumber(objX1);
	p1.y = (int)lua_getnumber(objY1);
	p2.x = (int)lua_getnumber(objX2);
	p2.y = (int)lua_getnumber(objY2);
	bool filled = false;

	if (lua_istable(tableObj)){
		lua_pushobject(tableObj);
		lua_pushstring("color");
		lua_Object colorObj = lua_gettable();
		if (lua_isuserdata(colorObj) && lua_tag(colorObj) == MKTAG('C','O','L','R')) {
			color = getcolor(colorObj);
		}

		lua_pushobject(tableObj);
		lua_pushstring("filled");
		lua_Object objFilled = lua_gettable();
		if (!lua_isnil(objFilled))
			filled = true;
	}

	PrimitiveObject *p = new PrimitiveObject();
	p->createRectangle(p1, p2, color, filled);
	g_grim->registerPrimitiveObject(p);
	lua_pushusertag(p->getId(), MKTAG('P','R','I','M')); // FIXME: we use PRIM usetag here
}

void L1_BlastRect() {
	Common::Point p1, p2;
	Color *color = NULL;
	lua_Object objX1 = lua_getparam(1);
	lua_Object objY1 = lua_getparam(2);
	lua_Object objX2 = lua_getparam(3);
	lua_Object objY2 = lua_getparam(4);
	lua_Object tableObj = lua_getparam(5);

	if (!lua_isnumber(objX1) || !lua_isnumber(objY1) || !lua_isnumber(objX2) || !lua_isnumber(objY2)) {
		lua_pushnil();
		return;
	}
	p1.x = (int)lua_getnumber(objX1);
	p1.y = (int)lua_getnumber(objY1);
	p2.x = (int)lua_getnumber(objX2);
	p2.y = (int)lua_getnumber(objY2);
	bool filled = false;

	if (lua_istable(tableObj)){
		lua_pushobject(tableObj);
		lua_pushstring("color");
		lua_Object colorObj = lua_gettable();
		if (lua_isuserdata(colorObj) && lua_tag(colorObj) == MKTAG('C','O','L','R')) {
			color = getcolor(colorObj);
		}

		lua_pushobject(tableObj);
		lua_pushstring("filled");
		lua_Object objFilled = lua_gettable();
		if (!lua_isnil(objFilled))
			filled = true;
	}

	PrimitiveObject *p = new PrimitiveObject();
	p->createRectangle(p1, p2, color, filled);
	p->draw();
	delete p;
}

void L1_KillPrimitive() {
	lua_Object primObj = lua_getparam(1);

	if (!lua_isuserdata(primObj) || lua_tag(primObj) != MKTAG('P','R','I','M'))
		return;

	PrimitiveObject *prim = getprimitive(primObj);
	g_grim->killPrimitiveObject(prim);
	delete prim;
}

void L1_DimScreen() {
	g_driver->dimScreen();
}

void L1_DimRegion() {
	int x = (int)lua_getnumber(lua_getparam(1));
	int y = (int)lua_getnumber(lua_getparam(2));
	int w = (int)lua_getnumber(lua_getparam(3));
	int h = (int)lua_getnumber(lua_getparam(4));
	float level = lua_getnumber(lua_getparam(5));
	g_driver->dimRegion(x, y, w, h, level);
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

void L1_ScreenShot() {
	int width = (int)lua_getnumber(lua_getparam(1));
	int height = (int)lua_getnumber(lua_getparam(2));
	int mode = g_grim->getMode();
	g_grim->setMode(ENGINE_MODE_NORMAL);
	g_grim->updateDisplayScene();
	Bitmap *screenshot = g_driver->getScreenshot(width, height);
	g_grim->setMode(mode);
	if (screenshot) {
		lua_pushusertag(screenshot->getId(), MKTAG('V','B','U','F'));
	} else {
		lua_pushnil();
	}
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
	if (!savedState) {
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
	int32 dataSize = savedState->beginSection('SUBS');
	lua_Object result = lua_createtable();

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

void L1_RenderModeUser() {
	lua_Object param1 = lua_getparam(1);
	if (!lua_isnil(param1) && g_grim->getMode() != ENGINE_MODE_DRAW) {
		g_grim->setPreviousMode(g_grim->getMode());
		g_smush->pause(true);
		g_grim->setMode(ENGINE_MODE_DRAW);
	} else if (lua_isnil(param1) && g_grim->getMode() == ENGINE_MODE_DRAW) {
		g_smush->pause(false);
		g_grim->refreshDrawMode();
		g_grim->setMode(g_grim->getPreviousMode());
	}
}

void L1_SetGamma() {
}

void L1_Display() {
	if (g_grim->getFlipEnable()) {
		g_driver->flipBuffer();
	}
}

void L1_EngineDisplay() {
	// it enable/disable updating display
	bool mode = (int)lua_getnumber(lua_getparam(1)) != 0;
	if (mode) {
		g_grim->setFlipEnable(true);
	} else {
		g_grim->setFlipEnable(false);
	}
}

void L1_ForceRefresh() {
	g_grim->refreshDrawMode();
}

void L1_JustLoaded() {
	if (gDebugLevel == DEBUG_ERROR || gDebugLevel == DEBUG_ALL)
		error("OPCODE USAGE VERIFICATION: JustLoaded");
}

void L1_PlaySound() {
	if (gDebugLevel == DEBUG_ERROR || gDebugLevel == DEBUG_ALL)
		error("OPCODE USAGE VERIFICATION: PlaySound");
}

void L1_SetEmergencyFont() {
	if (gDebugLevel == DEBUG_ERROR || gDebugLevel == DEBUG_ALL)
		error("OPCODE USAGE VERIFICATION: SetEmergencyFont");
}

void L1_LoadBundle() {
	// loading grimdemo.mus is allready handled
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

void L1_LightMgrStartup() {
	// we will not implement this opcode
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

void dummyHandler() {
}

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
