/* Residual - A 3D game interpreter
 *
 * Residual is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
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
#include "engines/grim/lua/luadebug.h"
#include "engines/grim/lua/lauxlib.h"
#include "engines/grim/imuse/imuse.h"

namespace Grim {

extern Imuse *g_imuse;

Common::StringList g_listfiles;
Common::StringList::const_iterator g_filesiter;

int refSystemTable;
static int refTypeOverride;
static int refOldConcatFallback;
static int refTextObjectX;
static int refTextObjectY;
static int refTextObjectFont;
static int refTextObjectWidth;
static int refTextObjectHeight;
static int refTextObjectFGColor;
static int refTextObjectBGColor;
static int refTextObjectFXColor;
static int refTextObjectHIColor;
static int refTextObjectDuration;
static int refTextObjectCenter;
static int refTextObjectLJustify;
static int refTextObjectRJustify;
static int refTextObjectVolume;
static int refTextObjectBackground;
static int refTextObjectPan;

#define strmatch(src, dst)		(strlen(src) == strlen(dst) && strcmp(src, dst) == 0)

static inline bool getbool(int num) {
	return !lua_isnil(lua_getparam(num));
}

static inline void pushbool(bool val) {
	if (val)
		lua_pushnumber(1);
	else
		lua_pushnil();
}

// Lua interface to bundle_dofile

int luaA_passresults();

static void new_dofile() {
	const char *fname_str = luaL_check_string(1);
	if (bundle_dofile(fname_str) == 0)
		if (luaA_passresults() == 0)
			lua_pushuserdata(NULL);
}

// Debugging message functions

static void PrintDebug() {
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

static void PrintError() {
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

static void PrintWarning() {
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

static void FunctionName() {
	const char *name;
	char buf[256];
	const char *filename;
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

static void CheckForFile() {
	lua_Object strObj = lua_getparam(1);

	if (!lua_isstring(strObj))
		return;

	const char *filename = lua_getstring(strObj);
	pushbool(g_resourceloader->fileExists(filename));
}

static byte clamp_color(int c) {
	if (c < 0)
		return 0;
	else if (c > 255)
		return 255;
	else
		return c;
}

static void MakeColor() {
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
	lua_pushusertag(c, MKID_BE('COLR'));
}

static void GetColorComponents() {
	lua_Object colorObj = lua_getparam(1);
	Color *c = static_cast<Color *>(lua_getuserdata(colorObj));
	lua_pushnumber(c->red());
	lua_pushnumber(c->green());
	lua_pushnumber(c->blue());
}

static void ReadRegistryValue() {
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

static void WriteRegistryValue() {
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

// Actor functions

static void LoadActor() {
	lua_Object nameObj = lua_getparam(1);
	const char *name;

	if (lua_isnil(nameObj) || !lua_isstring(nameObj))
		name = "<unnamed>";
	else
		name = lua_getstring(nameObj);
	lua_pushusertag(new Actor(name), MKID_BE('ACTR'));
}

static void GetActorTimeScale() {
	lua_Object actorObj = lua_getparam(1);
	if (!lua_isuserdata(actorObj) || lua_tag(actorObj) != MKID_BE('ACTR'))
		return;
/*	Actor *actor = static_cast<Actor *>(lua_getuserdata(actorObj));*/
	// TODO lua_pushnumber(actor->getTimeScale());
	// return 1 so the game doesn't halt when Manny attempts
	// to pick up the fire extinguisher
	lua_pushnumber(1);
}

static void SetSelectedActor() {
	lua_Object actorObj = lua_getparam(1);
	if (!lua_isuserdata(actorObj) || lua_tag(actorObj) != MKID_BE('ACTR'))
		return;
	Actor *actor = static_cast<Actor *>(lua_getuserdata(actorObj));
	// TODO there is missing some check here: else lua_pushnil()
	g_grim->setSelectedActor(actor);
}

/* Get the currently selected actor, this is used in
 * "Camera-Relative" mode to handle the appropriate
 * actor for movement
 */
static void GetCameraActor() {
	// TODO verify what is going on with selected actor
	Actor *actot = g_grim->selectedActor();
	lua_pushusertag(actot, MKID_BE('ACTR'));
}


static void setDefaultObjectParams(TextObjectDefaults *defaults, lua_Object tableObj) {
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
		if (lua_isuserdata(keyObj) && lua_tag(keyObj) == MKID_BE('FONT')) {
			defaults->font = static_cast<Font *>(lua_getuserdata(keyObj));
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
		if (lua_isuserdata(keyObj) && lua_tag(keyObj) == MKID_BE('COLR')) {
			defaults->fgColor = static_cast<Color *>(lua_getuserdata(keyObj));
		}
	}

	lua_pushobject(tableObj);
	lua_pushobject(lua_getref(refTextObjectBGColor));
	keyObj = lua_gettable();
	if (keyObj) {
		if (lua_isuserdata(keyObj) && lua_tag(keyObj) == MKID_BE('COLR')) {
			//defaults->bgColor = static_cast<Color *>(lua_getuserdata(keyObj));
			warning("setDefaultObjectParams: dummy BGColor");
		}
	}

	lua_pushobject(tableObj);
	lua_pushobject(lua_getref(refTextObjectFXColor));
	keyObj = lua_gettable();
	if (keyObj) {
		if (lua_isuserdata(keyObj) && lua_tag(keyObj) == MKID_BE('COLR')) {
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

static void SetSayLineDefaults() {
	lua_Object tableObj = lua_getparam(1);
	if (tableObj && lua_istable(tableObj))
		setDefaultObjectParams(&sayLineDefaults, tableObj);
}

static void SetActorTalkColor() {
	lua_Object actorObj = lua_getparam(1);
	lua_Object colorObj = lua_getparam(2);

	if (!lua_isuserdata(actorObj) || lua_tag(actorObj) != MKID_BE('ACTR'))
		return;
	if (!lua_isuserdata(colorObj) && lua_tag(colorObj) != MKID_BE('COLR'))
		return;
	Actor *actor = static_cast<Actor *>(lua_getuserdata(actorObj));
	Color *color = static_cast<Color *>(lua_getuserdata(colorObj));
	actor->setTalkColor(*color);
}

static void GetActorTalkColor() {
	lua_Object actorObj = lua_getparam(1);
	if (!lua_isuserdata(actorObj) || lua_tag(actorObj) != MKID_BE('ACTR')) {
		lua_pushnil();
		return;
	}
	Actor *actor = static_cast<Actor *>(lua_getuserdata(actorObj));
	Color *color = new Color(actor->talkColor());
	lua_pushusertag(color, MKID_BE('COLR'));
}

static bool findCostume(lua_Object costumeObj, Actor *actor, Costume **costume) {
	*costume = actor->currentCostume(); // should be root of list I think
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

static void SetActorRestChore() {
	lua_Object actorObj = lua_getparam(1);
	lua_Object choreObj = lua_getparam(2);
	lua_Object costumeObj = lua_getparam(3);
	Costume *costume;
	int chore;

	if (!lua_isuserdata(actorObj) || lua_tag(actorObj) != MKID_BE('ACTR') ||
			(!lua_isnumber(choreObj) && !lua_isnil(choreObj))) {
		return;
	}

	Actor *actor = static_cast<Actor *>(lua_getuserdata(actorObj));

	if (lua_isnil(choreObj)) {
		chore = -1;
	} else {
		chore = (int)lua_getnumber(choreObj);
	}
	if (!findCostume(costumeObj, actor, &costume))
		return;

	actor->setRestChore(chore, costume);
}

static void SetActorWalkChore() {
	lua_Object actorObj = lua_getparam(1);
	lua_Object choreObj = lua_getparam(2);
	lua_Object costumeObj = lua_getparam(3);
	Costume *costume;
	int chore;

	if (!lua_isuserdata(actorObj) || lua_tag(actorObj) != MKID_BE('ACTR') ||
			(!lua_isnumber(choreObj) && !lua_isnil(choreObj))) {
		return;
	}

	Actor *actor = static_cast<Actor *>(lua_getuserdata(actorObj));

	if (lua_isnil(choreObj)) {
		chore = -1;
	} else {
		chore = (int)lua_getnumber(choreObj);
	}
	if (!findCostume(costumeObj, actor, &costume))
		return;

	actor->setWalkChore(chore, costume);
}

static void SetActorTurnChores() {
	lua_Object actorObj = lua_getparam(1);
	lua_Object leftChoreObj = lua_getparam(2);
	lua_Object rightChoreObj = lua_getparam(3);
	lua_Object costumeObj = lua_getparam(4);
	Costume *costume;

	if (!lua_isuserdata(actorObj) || lua_tag(actorObj) != MKID_BE('ACTR') ||
			(!lua_isnumber(leftChoreObj) && !lua_isnumber(rightChoreObj))) {
		return;
	}

	Actor *actor = static_cast<Actor *>(lua_getuserdata(actorObj));
	int leftChore = (int)lua_getnumber(leftChoreObj);
	int rightChore = (int)lua_getnumber(rightChoreObj);

	if (!findCostume(costumeObj, actor, &costume))
		return;

	actor->setTurnChores(leftChore, rightChore, costume);
}

static void SetActorTalkChore() {
	lua_Object actorObj = lua_getparam(1);
	lua_Object indexObj = lua_getparam(2);
	lua_Object choreObj = lua_getparam(3);
	lua_Object costumeObj = lua_getparam(4);
	Costume *costume;
	int chore;

	if (!lua_isuserdata(actorObj) || lua_tag(actorObj) != MKID_BE('ACTR') ||
			!lua_isnumber(indexObj) || (!lua_isnumber(choreObj) && !lua_isnil(choreObj))) {
		return;
	}

	int index = (int)lua_getnumber(indexObj);
	if (index < 1 || index > 16)
		return;

	Actor *actor = static_cast<Actor *>(lua_getuserdata(actorObj));

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

static void SetActorMumblechore() {
	lua_Object actorObj = lua_getparam(1);
	lua_Object choreObj = lua_getparam(2);
	lua_Object costumeObj = lua_getparam(3);
	Costume *costume;
	int chore;

	if (!lua_isuserdata(actorObj) || lua_tag(actorObj) != MKID_BE('ACTR') ||
			(!lua_isnumber(choreObj) && !lua_isnil(choreObj))) {
		return;
	}

	Actor *actor = static_cast<Actor *>(lua_getuserdata(actorObj));

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

static void SetActorVisibility() {
	lua_Object actorObj = lua_getparam(1);
	if (!lua_isuserdata(actorObj) || lua_tag(actorObj) != MKID_BE('ACTR'))
		return;

	Actor *actor = static_cast<Actor *>(lua_getuserdata(actorObj));

	bool val = getbool(2);
	actor->setVisibility(val);
}

static void PutActorAt() {
	lua_Object actorObj = lua_getparam(1);
	lua_Object xObj = lua_getparam(2);
	lua_Object yObj = lua_getparam(3);
	lua_Object zObj = lua_getparam(4);
	if (!lua_isuserdata(actorObj) || lua_tag(actorObj) != MKID_BE('ACTR'))
		return;

	if (!lua_isnumber(xObj) || !lua_isnumber(yObj) || !lua_isnumber(zObj))
		return;

	Actor *actor = static_cast<Actor *>(lua_getuserdata(actorObj));
	float x = lua_getnumber(xObj);
	float y = lua_getnumber(yObj);
	float z = lua_getnumber(zObj);
	actor->setPos(Graphics::Vector3d(x, y, z));
}

static void GetActorPos() {
	lua_Object actorObj = lua_getparam(1);

	if (!lua_isuserdata(actorObj) || lua_tag(actorObj) != MKID_BE('ACTR'))
		return;

	Actor *actor = static_cast<Actor *>(lua_getuserdata(actorObj));
	Graphics::Vector3d pos = actor->pos();
	lua_pushnumber(pos.x());
	lua_pushnumber(pos.y());
	lua_pushnumber(pos.z());
}

static void SetActorRot() {
	lua_Object actorObj = lua_getparam(1);

	if (!lua_isuserdata(actorObj) || lua_tag(actorObj) != MKID_BE('ACTR'))
		return;

	Actor *actor = static_cast<Actor *>(lua_getuserdata(actorObj));
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

static void GetActorRot() {
	lua_Object actorObj = lua_getparam(1);

	if (!lua_isuserdata(actorObj) || lua_tag(actorObj) != MKID_BE('ACTR'))
		return;

	Actor *actor = static_cast<Actor *>(lua_getuserdata(actorObj));
	lua_pushnumber(actor->pitch());
	lua_pushnumber(actor->yaw());
	lua_pushnumber(actor->roll());
}

static void IsActorTurning() {
	lua_Object actorObj = lua_getparam(1);

	if (!lua_isuserdata(actorObj) || lua_tag(actorObj) != MKID_BE('ACTR'))
		return;

	Actor *actor = static_cast<Actor *>(lua_getuserdata(actorObj));
	pushbool(actor->isTurning());
}

static void GetAngleBetweenActors() {
	lua_Object actor1Obj = lua_getparam(1);
	lua_Object actor2Obj = lua_getparam(2);

	if (!lua_isuserdata(actor1Obj) || lua_tag(actor1Obj) != MKID_BE('ACTR')) {
		lua_pushnil();
		return;
	}
	if (!lua_isuserdata(actor2Obj) || lua_tag(actor2Obj) != MKID_BE('ACTR')) {
		lua_pushnil();
		return;
	}
	Actor *actor1 = static_cast<Actor *>(lua_getuserdata(actor1Obj));
	Actor *actor2 = static_cast<Actor *>(lua_getuserdata(actor2Obj));

	if (!actor1 || !actor2) {
		lua_pushnil();
		return;
	}
	// TODO implement proper calculations here
	lua_pushnumber(actor1->angleTo(*actor2));
}

static void GetActorYawToPoint() {
	lua_Object actorObj = lua_getparam(1);
	lua_Object pointObj = lua_getparam(2);
	lua_Object xObj, yObj, zObj;

	if (!lua_isuserdata(actorObj) || lua_tag(actorObj) != MKID_BE('ACTR')) {
		lua_pushnil();
		return;
	}

	Actor *actor = static_cast<Actor *>(lua_getuserdata(actorObj));
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

	lua_pushnumber(actor->yawTo(yawVector));
}

/* Changes the set that an actor is associated with,
 * by changing the set to "nil" an actor is disabled
 * but should still not be destroyed.
 */
static void PutActorInSet() {
	lua_Object actorObj = lua_getparam(1);
	lua_Object setObj = lua_getparam(2);

	if (!lua_isuserdata(actorObj) || lua_tag(actorObj) != MKID_BE('ACTR'))
		return;

	Actor *actor = static_cast<Actor *>(lua_getuserdata(actorObj));

	if (!lua_isstring(setObj) && !lua_isnil(setObj))
		return;

	const char *set = lua_getstring(setObj);

	// FIXME verify adding actor to set
	if (!set)
		set = "";
	if (!actor->inSet(set))
		actor->putInSet(set);
}

static void SetActorWalkRate() {
	lua_Object actorObj = lua_getparam(1);
	lua_Object rateObj = lua_getparam(2);

	if (!lua_isuserdata(actorObj) || lua_tag(actorObj) != MKID_BE('ACTR'))
		return;
	if (!lua_isnumber(rateObj))
		return;

	Actor *actor = static_cast<Actor *>(lua_getuserdata(actorObj));
	float rate = lua_getnumber(rateObj);
	actor->setWalkRate(rate);
}

static void GetActorWalkRate() {
	lua_Object actorObj = lua_getparam(1);
	if (!lua_isuserdata(actorObj) || lua_tag(actorObj) != MKID_BE('ACTR'))
		return;

	Actor *actor = static_cast<Actor *>(lua_getuserdata(actorObj));
	lua_pushnumber(actor->walkRate());
}

static void SetActorTurnRate() {
	lua_Object actorObj = lua_getparam(1);
	lua_Object rateObj = lua_getparam(2);

	if (!lua_isuserdata(actorObj) || lua_tag(actorObj) != MKID_BE('ACTR'))
		return;
	if (!lua_isnumber(rateObj))
		return;

	Actor *actor = static_cast<Actor *>(lua_getuserdata(actorObj));
	float rate = lua_getnumber(rateObj); // FIXME verify negate values of rate
	actor->setTurnRate(rate);
}

static void WalkActorForward() {
	lua_Object actorObj = lua_getparam(1);
	if (!lua_isuserdata(actorObj) || lua_tag(actorObj) != MKID_BE('ACTR'))
		return;
	Actor *actor = static_cast<Actor *>(lua_getuserdata(actorObj));
	actor->walkForward();
}

static void SetActorReflection() {
	lua_Object actorObj = lua_getparam(1);
	lua_Object angleObj = lua_getparam(2);

	if (!lua_isuserdata(actorObj) || lua_tag(actorObj) != MKID_BE('ACTR'))
		return;

	Actor *actor = static_cast<Actor *>(lua_getuserdata(actorObj));
	float angle = lua_getnumber(angleObj);
	actor->setReflection(angle);
}

static void GetActorPuckVector() {
	lua_Object actorObj = lua_getparam(1);
	lua_Object addObj = lua_getparam(2);

	if (!lua_isuserdata(actorObj) || lua_tag(actorObj) != MKID_BE('ACTR')) {
		lua_pushnil();
		return;
	}

	Actor *actor = static_cast<Actor *>(lua_getuserdata(actorObj));
	if (!actor) {
		lua_pushnil();
		return;
	}

	Graphics::Vector3d result = actor->puckVector();
	if (!lua_isnil(addObj))
		result += actor->pos();

	lua_pushnumber(result.x());
	lua_pushnumber(result.y());
	lua_pushnumber(result.z());
}

static void WalkActorTo() {
	lua_Object actorObj = lua_getparam(1);
	lua_Object xObj = lua_getparam(2);
	lua_Object yObj = lua_getparam(3);
	lua_Object zObj = lua_getparam(4);

	lua_Object txObj = lua_getparam(5);
	lua_Object tyObj = lua_getparam(6);
	lua_Object tzObj = lua_getparam(7);

	if (!lua_isuserdata(actorObj) || lua_tag(actorObj) != MKID_BE('ACTR'))
		return;

	Graphics::Vector3d destVec;
	Actor *actor = static_cast<Actor *>(lua_getuserdata(actorObj));
	if (!lua_isnumber(xObj)) {
		if (!lua_isuserdata(xObj) || lua_tag(xObj) != MKID_BE('ACTR'))
			return;
		Actor *destActor = static_cast<Actor *>(lua_getuserdata(xObj));
		destVec = destActor->pos();
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

static void IsActorMoving() {
	lua_Object actorObj = lua_getparam(1);
	if (!lua_isuserdata(actorObj) || lua_tag(actorObj) != MKID_BE('ACTR'))
		return;

	Actor *actor = static_cast<Actor *>(lua_getuserdata(actorObj));
	pushbool(actor->isWalking());
}

static void Is3DHardwareEnabled() {
	pushbool(g_driver->isHardwareAccelerated());
}

static void SetHardwareState() {
	// changing only in config setup (software/hardware rendering)
	bool accel = getbool(1);
	if (accel)
		g_registry->set("soft_renderer", "FALSE");
	else
		g_registry->set("soft_renderer", "TRUE");
}

static void SetVideoDevices() {
	int devId;
	int modeId;

	devId = (int)lua_getnumber(lua_getparam(1));
	modeId = (int)lua_getnumber(lua_getparam(2));
	// ignore setting video devices
}

static void GetVideoDevices() {
	lua_pushnumber(0.0);
	lua_pushnumber(-1.0);
}

static void EnumerateVideoDevices() {
	lua_Object result = lua_createtable();
	lua_pushobject(result);
	lua_pushnumber(0.0); // id of device
	lua_pushstring(g_driver->getVideoDeviceName()); // name of device
	lua_settable();
	lua_pushobject(result);
}

static void Enumerate3DDevices() {
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

static void IsActorResting() {
	lua_Object actorObj = lua_getparam(1);
	if (!lua_isuserdata(actorObj) || lua_tag(actorObj) != MKID_BE('ACTR'))
		return;

	Actor *actor = static_cast<Actor *>(lua_getuserdata(actorObj));
	// FIXME verify if exist one flag for resting mode
	pushbool(!(actor->isWalking() || actor->isTurning()));
}

/* Get the location of one of the actor's nodes, this is
 * used by Glottis to watch where Manny is located in
 * order to hand him the work order.  This function is
 * also important for when Velasco hands Manny the logbook
 * in Rubacava
 */
static void GetActorNodeLocation() {
	lua_Object actorObj = lua_getparam(1);
	lua_Object nodeObj = lua_getparam(2);

	if (!lua_isuserdata(actorObj) || lua_tag(actorObj) != MKID_BE('ACTR'))
		return;

	if (!lua_isnumber(nodeObj))
		return;

	Actor *actor = static_cast<Actor *>(lua_getuserdata(actorObj));
	if (!actor->currentCostume() || !actor->currentCostume()->getModelNodes())
		return;

	int node = (int)lua_getnumber(nodeObj);

	Model::HierNode *allNodes = actor->currentCostume()->getModelNodes();

	// TODO implement missing calculations

	lua_pushnumber(allNodes[node]._pos.x());
	lua_pushnumber(allNodes[node]._pos.y());
	lua_pushnumber(allNodes[node]._pos.z());
}

/* This function is called to stop walking actions that
 * are in progress, it is unknown whether it should start
 * walking actions that are not in progress.
 */
static void SetActorWalkDominate() {
	lua_Object actorObj = lua_getparam(1);
//	lua_Object modeObj = lua_getparam(2);

	if (!lua_isuserdata(actorObj) || lua_tag(actorObj) != MKID_BE('ACTR'))
		return;

/*	bool mode = lua_isnil(modeObj) != 0;
	Actor *actor = static_cast<Actor *>(lua_getuserdata(actorObj));
*/	// TODO implement missing function
	//actor->setWalkDominate(mode);
}

static void SetActorColormap() {
	lua_Object actorObj = lua_getparam(1);
	lua_Object nameObj = lua_getparam(2);

	if (!lua_isuserdata(actorObj) || lua_tag(actorObj) != MKID_BE('ACTR'))
		return;

	Actor *actor = static_cast<Actor *>(lua_getuserdata(actorObj));
	if (lua_isstring(nameObj)) {
		const char *name = lua_getstring(nameObj);
		g_resourceloader->loadColormap(name);
		actor->setColormap(name);
	} else if (lua_isnil(nameObj)) {
		error("SetActorColormap: implement remove cmap");
		// remove ?
	}
}

static void TurnActor() {
	lua_Object actorObj = lua_getparam(1);
	lua_Object dirObj = lua_getparam(2);

	if (!lua_isuserdata(actorObj) || lua_tag(actorObj) != MKID_BE('ACTR'))
		return;

	if (!lua_isnumber(dirObj))
		return;

	Actor *actor = static_cast<Actor *>(lua_getuserdata(actorObj));
	int dir = (int)lua_getnumber(dirObj);
	// TODO verify. need clear mode walking
	actor->turn(dir);
}

static void PushActorCostume() {
	lua_Object actorObj = lua_getparam(1);
	lua_Object nameObj = lua_getparam(2);

	if (!lua_isuserdata(actorObj) || lua_tag(actorObj) != MKID_BE('ACTR'))
		return;

	if (!lua_isstring(nameObj))
		return;

	Actor *actor = static_cast<Actor *>(lua_getuserdata(actorObj));
	const char *costumeName = lua_getstring(nameObj);
	actor->pushCostume(costumeName);
}

static void SetActorCostume() {
	lua_Object actorObj = lua_getparam(1);
	lua_Object costumeObj = lua_getparam(2);

	if (!lua_isuserdata(actorObj) || lua_tag(actorObj) != MKID_BE('ACTR'))
		return;

	Actor *actor = static_cast<Actor *>(lua_getuserdata(actorObj));
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

static void GetActorCostume() {
	lua_Object actorObj = lua_getparam(1);
	lua_Object costumeObj = lua_getparam(2);

	if (!lua_isuserdata(actorObj) || lua_tag(actorObj) != MKID_BE('ACTR')) {
		lua_pushnil();
		return;
	}

	Actor *actor = static_cast<Actor *>(lua_getuserdata(actorObj));
	Costume *costume = actor->currentCostume();
	if (lua_isnil(costumeObj)) {
		// dummy
	} else if (lua_isnumber(costumeObj)) {
/*		int num = (int)lua_getnumber(costumeObj);*/
		error("GetActorCostume: implement number Id");
	} else
		return;

	if (costume)
		lua_pushstring(const_cast<char *>(costume->filename()));
	else
		lua_pushnil();
}

static void PopActorCostume() {
	lua_Object actorObj = lua_getparam(1);
	if (!lua_isuserdata(actorObj) || lua_tag(actorObj) != MKID_BE('ACTR'))
		return;

	Actor *actor = static_cast<Actor *>(lua_getuserdata(actorObj));
	if (actor->currentCostume()) {
		lua_pushstring(const_cast<char *>(actor->currentCostume()->filename()));
		actor->popCostume();
	} else
		lua_pushnil();
}

static void GetActorCostumeDepth() {
	lua_Object actorObj = lua_getparam(1);
	if (!lua_isuserdata(actorObj) || lua_tag(actorObj) != MKID_BE('ACTR')) {
		lua_pushnil();
		return;
	}

	Actor *actor = static_cast<Actor *>(lua_getuserdata(actorObj));
	lua_pushnumber(actor->costumeStackDepth());
}

static void PrintActorCostumes() {
	// dummy
}

static void LoadCostume() {
	lua_Object nameObj = lua_getparam(1);
	if (lua_isstring(nameObj)) {
		const char *name = lua_getstring(nameObj);
		g_resourceloader->loadCostume(name, NULL);
	} else
		lua_pushnil();
}

static void PlayActorChore() {
	lua_Object actorObj = lua_getparam(1);
	lua_Object choreObj = lua_getparam(2);
	lua_Object costumeObj = lua_getparam(3);

	if (!lua_isuserdata(actorObj) || lua_tag(actorObj) != MKID_BE('ACTR'))
		return;

	Actor *actor = static_cast<Actor *>(lua_getuserdata(actorObj));

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

static void CompleteActorChore() {
	lua_Object actorObj = lua_getparam(1);
	lua_Object choreObj = lua_getparam(2);
	lua_Object costumeObj = lua_getparam(3);

	if (!lua_isuserdata(actorObj) || lua_tag(actorObj) != MKID_BE('ACTR'))
		return;

	Actor *actor = static_cast<Actor *>(lua_getuserdata(actorObj));

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

static void PlayActorChoreLooping() {
	lua_Object actorObj = lua_getparam(1);
	lua_Object choreObj = lua_getparam(2);
	lua_Object costumeObj = lua_getparam(3);

	if (!lua_isuserdata(actorObj) || lua_tag(actorObj) != MKID_BE('ACTR'))
		return;

	Actor *actor = static_cast<Actor *>(lua_getuserdata(actorObj));

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

static void SetActorChoreLooping() {
	lua_Object actorObj = lua_getparam(1);
	lua_Object choreObj = lua_getparam(2);
	lua_Object costumeObj = lua_getparam(4);

	if (!lua_isuserdata(actorObj) || lua_tag(actorObj) != MKID_BE('ACTR'))
		return;

	Actor *actor = static_cast<Actor *>(lua_getuserdata(actorObj));

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

static void StopActorChore() {
	lua_Object actorObj = lua_getparam(1);
	lua_Object choreObj = lua_getparam(2);
	lua_Object costumeObj = lua_getparam(3);

	if (!lua_isuserdata(actorObj) || lua_tag(actorObj) != MKID_BE('ACTR'))
		return;

	Actor *actor = static_cast<Actor *>(lua_getuserdata(actorObj));

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

static void IsActorChoring() {
	lua_Object actorObj = lua_getparam(1);
	lua_Object choreObj = lua_getparam(2);
	lua_Object costumeObj = lua_getparam(4);

	if (!lua_isuserdata(actorObj) || lua_tag(actorObj) != MKID_BE('ACTR'))
		return;

	Actor *actor = static_cast<Actor *>(lua_getuserdata(actorObj));

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

static void ActorLookAt() {
	lua_Object actorObj = lua_getparam(1);
	lua_Object xObj = lua_getparam(2);
	lua_Object yObj = lua_getparam(3);
	lua_Object zObj = lua_getparam(4);
	lua_Object rateObj = lua_getparam(5);

	if (!lua_isuserdata(actorObj) || lua_tag(actorObj) != MKID_BE('ACTR'))
		return;
	Actor *actor = static_cast<Actor *>(lua_getuserdata(actorObj));
	if (!actor->currentCostume())
		return;

	if (lua_isnumber(rateObj))
		actor->setLookAtRate(lua_getnumber(rateObj));

	// Look at nothing
	if (lua_isnil(xObj)) {
		if (actor->isLookAtVectorZero())
			return;

		actor->setLookAtVectorZero();
		if (lua_isnumber(yObj))
			actor->setLookAtRate(lua_getnumber(yObj));
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
	} else if (lua_isuserdata(xObj) && lua_tag(xObj) == MKID_BE('ACTR')) { // look at another actor
		Actor *lookedAct = static_cast<Actor *>(lua_getuserdata(xObj));

		actor->setLookAtVector(lookedAct->pos());

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
static void TurnActorTo() {
	lua_Object actorObj = lua_getparam(1);
	lua_Object xObj = lua_getparam(2);
	lua_Object yObj = lua_getparam(3);
	lua_Object zObj = lua_getparam(4);
	float x, y, z;

	if (!lua_isuserdata(actorObj) || lua_tag(actorObj) != MKID_BE('ACTR')) {
		lua_pushnil();
		return;
	}
	Actor *actor = static_cast<Actor *>(lua_getuserdata(actorObj));

	if (lua_isuserdata(xObj) && lua_tag(xObj) == MKID_BE('ACTR')) {
		Actor *destActor = static_cast<Actor *>(lua_getuserdata(xObj));
		x = destActor->pos().x();
		y = destActor->pos().y();
		z = destActor->pos().z();
	} else {
		x = lua_getnumber(xObj);
		y = lua_getnumber(yObj);
		z = lua_getnumber(zObj);
	}

	// TODO turning stuff below is not complete
	
	// Find the vector pointing from the actor to the desired location
	Graphics::Vector3d turnToVector(x, y, z);
	Graphics::Vector3d lookVector = turnToVector - actor->pos();
	// find the angle the requested position is around the unit circle
	float yaw = lookVector.unitCircleAngle();
	// yaw is offset from forward by 90 degrees
	yaw -= 90.0f;
	actor->turnTo(0, yaw, 0);

	// Game will lock in elevator if this doesn't return false
	pushbool(false);
}

static void PointActorAt() {
	lua_Object actorObj = lua_getparam(1);
	lua_Object xObj = lua_getparam(2);
	lua_Object yObj = lua_getparam(3);
	lua_Object zObj = lua_getparam(4);
	float x, y, z;

	if (!lua_isuserdata(actorObj) || lua_tag(actorObj) != MKID_BE('ACTR')) {
		lua_pushnil();
		return;
	}
	Actor *actor = static_cast<Actor *>(lua_getuserdata(actorObj));

	if (lua_isuserdata(xObj) && lua_tag(xObj) == MKID_BE('ACTR')) {
		Actor *destActor = static_cast<Actor *>(lua_getuserdata(xObj));
		x = destActor->pos().x();
		y = destActor->pos().y();
		z = destActor->pos().z();
	} else {
		x = lua_getnumber(xObj);
		y = lua_getnumber(yObj);
		z = lua_getnumber(zObj);
	}

	// TODO turning stuff below is not complete
	
	// Find the vector pointing from the actor to the desired location
	Graphics::Vector3d turnToVector(x, y, z);
	Graphics::Vector3d lookVector = turnToVector - actor->pos();
	// find the angle the requested position is around the unit circle
	float yaw = lookVector.unitCircleAngle();
	// yaw is offset from forward by 90 degrees
	yaw -= 90.0f;
	actor->turnTo(0, yaw, 0);

	// Game will lock in elevator if this doesn't return false
	pushbool(false);
}

static void WalkActorVector() {
	lua_Object actorObj = lua_getparam(1);
	lua_Object actor2Obj = lua_getparam(2);
//	lua_Object xObj = lua_getparam(3);
//	lua_Object yObj = lua_getparam(4);
//	lua_Object zObj = lua_getparam(5);
//	lua_Object param6Obj = lua_getparam(6);

	if (!lua_isuserdata(actorObj) || lua_tag(actorObj) != MKID_BE('ACTR') ||
			!lua_isuserdata(actor2Obj) || lua_tag(actor2Obj) != MKID_BE('ACTR'))
		return;

//	Actor *actor = static_cast<Actor *>(lua_getuserdata(actorObj));
	Actor *actor2 = static_cast<Actor *>(lua_getuserdata(actor2Obj));

	// TODO whole below part need rewrote to much original
	float moveHoriz, moveVert, yaw;

	// Third option is the "left/right" movement
	moveHoriz = luaL_check_number(3);
	// Fourth Option is the "up/down" movement
	moveVert = luaL_check_number(4);

	// Get the direction the camera is pointing
	Graphics::Vector3d cameraVector = g_grim->currScene()->_currSetup->_interest - g_grim->currScene()->_currSetup->_pos;
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
	if (actor2->yaw() != yaw)
		actor2->turnTo(0, yaw, 0);
	else
		actor2->walkForward();
}

/* RotateVector takes a vector and rotates it around
 * the point (0,0,0) by the requested number of degrees.
 * This function is used to calculate the locations for
 * getting on and off of the Bone Wagon.
 */
static void RotateVector() {
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

	// TODO implement proper code
	float rotate, currAngle, newAngle;
	rotate = rot.y();
	Graphics::Vector3d baseVector(sin(0.0f), cos(0.0f), 0);
	currAngle = angle(baseVector, vec) * (180 / LOCAL_PI);
	newAngle = (currAngle - rotate) * (LOCAL_PI / 180);
	Graphics::Vector3d vec2(sin(newAngle), cos(newAngle), 0);
	vec2 *= vec.magnitude();
	vec = vec2;


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
static void SetActorPitch() {
	lua_Object actorObj = lua_getparam(1);
	lua_Object pitchObj = lua_getparam(2);

	if (!lua_isuserdata(actorObj) || lua_tag(actorObj) != MKID_BE('ACTR'))
		return;

	Actor *actor = static_cast<Actor *>(lua_getuserdata(actorObj));
	float pitch = lua_getnumber(pitchObj);
	actor->setRot(pitch, actor->yaw(), actor->roll());
}

static void SetActorLookRate() {
	lua_Object actorObj = lua_getparam(1);
	lua_Object rateObj = lua_getparam(2);

	if (!lua_isuserdata(actorObj) || lua_tag(actorObj) != MKID_BE('ACTR'))
		return;

	if (!lua_isnumber(rateObj))
		return;

	Actor *actor = static_cast<Actor *>(lua_getuserdata(actorObj));
	if (!actor->currentCostume())
		return;

	float rate = lua_getnumber(rateObj);
	actor->setLookAtRate(rate);
}

static void GetActorLookRate() {
	lua_Object actorObj = lua_getparam(1);

	if (!lua_isuserdata(actorObj) || lua_tag(actorObj) != MKID_BE('ACTR'))
		return;

	Actor *actor = static_cast<Actor *>(lua_getuserdata(actorObj));
	if (!actor->currentCostume())
		lua_pushnil();
	else
		lua_pushnumber(actor->lookAtRate());
}

static void SetActorHead() {
	lua_Object actorObj = lua_getparam(1);
	lua_Object joint1Obj = lua_getparam(2);
	lua_Object joint2Obj = lua_getparam(3);
	lua_Object joint3Obj = lua_getparam(4);
	lua_Object maxRollObj = lua_getparam(5);
	lua_Object maxPitchObj = lua_getparam(6);
	lua_Object maxYawObj = lua_getparam(7);

	if (!lua_isuserdata(actorObj) || lua_tag(actorObj) != MKID_BE('ACTR'))
		return;
	if (!lua_isnumber(joint1Obj) || !lua_isnumber(joint2Obj) || !lua_isnumber(joint3Obj) ||
			!lua_isnumber(maxRollObj) || !lua_isnumber(maxPitchObj) || !lua_isnumber(maxYawObj))
		return;

	Actor *actor = static_cast<Actor *>(lua_getuserdata(actorObj));
	int joint1 = (int)lua_getnumber(joint1Obj);
	int joint2 = (int)lua_getnumber(joint2Obj);
	int joint3 = (int)lua_getnumber(joint3Obj);
	float maxRoll = lua_getnumber(maxRollObj);
	float maxPitch = lua_getnumber(maxPitchObj);
	float maxYaw = lua_getnumber(maxYawObj);

	actor->setHead(joint1, joint2, joint3, maxRoll, maxPitch, maxYaw);
}

static void PutActorAtInterest() {
	lua_Object actorObj = lua_getparam(1);

	if (!lua_isuserdata(actorObj) || lua_tag(actorObj) != MKID_BE('ACTR'))
		return;

	Actor *actor = static_cast<Actor *>(lua_getuserdata(actorObj));
	if (!g_grim->currScene())
		return;

	actor->setPos(g_grim->currScene()->_currSetup->_interest);
}

static void SetActorFollowBoxes() {
	lua_Object actorObj = lua_getparam(1);
	lua_Object modeObj = lua_getparam(2);
	bool mode = true;

	if (!lua_isuserdata(actorObj) || lua_tag(actorObj) != MKID_BE('ACTR'))
		return;

	Actor *actor = static_cast<Actor *>(lua_getuserdata(actorObj));

	if (modeObj == LUA_NOOBJECT || lua_isnil(modeObj))
		mode = false;

	warning("SetActorFollowBoxes() not implemented");
	// TODO that is not walkbox walking, but temporary hack
	// actor->enableWalkbox(mode);
	actor->setConstrain(mode);
}

static void SetActorConstrain() {
	lua_Object actorObj = lua_getparam(1);
//	lua_Object constrainObj = lua_getparam(2);

	if (!lua_isuserdata(actorObj) || lua_tag(actorObj) != MKID_BE('ACTR'))
		return;

//	Actor *actor = static_cast<Actor *>(lua_getuserdata(actorObj));
//	bool constrain = !lua_isnil(constrainObj);

	// FIXME that below should be enabled, but for now it's disabled realated to
	// above func SetActorFollowBoxes.
//	actor->setConstrain(constrain);
}

static void GetVisibleThings() {
	lua_Object actorObj = lua_getparam(1);
	Actor *actor;
	if (lua_isnil(actorObj)) {
		if (g_grim->selectedActor())
			actor = g_grim->selectedActor();
		else
			return;
	} else if (lua_isuserdata(actorObj) && lua_tag(actorObj) == MKID_BE('ACTR')) {
		actor = static_cast<Actor *>(lua_getuserdata(actorObj));
	}

	lua_Object result = lua_createtable();

	// TODO verify code below
	for (GrimEngine::ActorListType::const_iterator i = g_grim->actorsBegin(); i != g_grim->actorsEnd(); i++) {
		if (!(*i)->inSet(g_grim->sceneName()))
			continue;
		// Consider the active actor visible
		if (actor == (*i) || actor->angleTo(*(*i)) < 90) {
			lua_pushobject(result);
			lua_pushusertag(*i, MKID_BE('ACTR'));
			lua_pushnumber(1);
			lua_settable();
		}
	}
	lua_pushobject(result);
}

static void SetShadowColor() {
	int r = (int)lua_getnumber(lua_getparam(1));
	int g = (int)lua_getnumber(lua_getparam(2));
	int b = (int)lua_getnumber(lua_getparam(3));

	g_driver->setShadowColor(r, g, b);
}

static void KillActorShadows() {
	lua_Object actorObj = lua_getparam(1);

	if (!lua_isuserdata(actorObj) || lua_tag(actorObj) != MKID_BE('ACTR')) {
		lua_pushnil();
		return;
	}
	Actor *actor = static_cast<Actor *>(lua_getuserdata(actorObj));
	actor->clearShadowPlanes();
}

static void SetActiveShadow() {
	lua_Object actorObj = lua_getparam(1);
	lua_Object shadowIdObj = lua_getparam(2);

	if (!lua_isuserdata(actorObj) || actorObj == LUA_NOOBJECT) {
		lua_pushnil();
		return;
	}
	Actor *actor = static_cast<Actor *>(lua_getuserdata(actorObj));
	int shadowId = (int)lua_getnumber(shadowIdObj);
	actor->setActiveShadow(shadowId);
}

static void SetActorShadowPoint() {
	lua_Object actorObj = lua_getparam(1);
	lua_Object xObj = lua_getparam(2);
	lua_Object yObj = lua_getparam(3);
	lua_Object zObj = lua_getparam(4);

	if (!lua_isuserdata(actorObj) || actorObj == LUA_NOOBJECT) {
		lua_pushnil();
		return;
	}
	Actor *actor = static_cast<Actor *>(lua_getuserdata(actorObj));
	float x = lua_getnumber(xObj);
	float y = lua_getnumber(yObj);
	float z = lua_getnumber(zObj);

	actor->setShadowPoint(Graphics::Vector3d(x, y, z));
}

static void SetActorShadowPlane() {
	lua_Object actorObj = lua_getparam(1);
	lua_Object nameObj = lua_getparam(2);

	if (!lua_isuserdata(actorObj) || actorObj == LUA_NOOBJECT) {
		lua_pushnil();
		return;
	}
	Actor *actor = static_cast<Actor *>(lua_getuserdata(actorObj));
	const char *name = lua_getstring(nameObj);

	actor->setShadowPlane(name);
}

static void AddShadowPlane() {
	lua_Object actorObj = lua_getparam(1);
	lua_Object nameObj = lua_getparam(2);

	if (!lua_isuserdata(actorObj) || actorObj == LUA_NOOBJECT) {
		lua_pushnil();
		return;
	}
	Actor *actor = static_cast<Actor *>(lua_getuserdata(actorObj));
	const char *name = lua_getstring(nameObj);

	actor->addShadowPlane(name);
}

static void ActivateActorShadow() {
	lua_Object actorObj = lua_getparam(1);
	lua_Object shadowIdObj = lua_getparam(2);
	lua_Object stateObj = lua_getparam(3);

	if (!lua_isuserdata(actorObj) || actorObj == LUA_NOOBJECT) {
		lua_pushnil();
		return;
	}
	Actor *actor = static_cast<Actor *>(lua_getuserdata(actorObj));
	int shadowId = (int)lua_getnumber(shadowIdObj);
	bool state = !lua_isnil(stateObj);

	actor->setActivateShadow(shadowId, state);
	g_grim->flagRefreshShadowMask(true);
}

static void SetActorShadowValid() {
	lua_Object actorObj = lua_getparam(1);
	lua_Object numObj = lua_getparam(2);

	if (!lua_isuserdata(actorObj) || actorObj == LUA_NOOBJECT) {
		lua_pushnil();
		return;
	}
	Actor *actor = static_cast<Actor *>(lua_getuserdata(actorObj));
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

static void TextFileGetLine() {
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
	file->readLine_NEW(textBuf, 1000);
	delete file;

	lua_pushstring(textBuf);
}

static void TextFileGetLineCount() {
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
		file->readLine_NEW(textBuf, 1000);
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

static void LocalizeString() {
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

static void parseSayLineTable(lua_Object paramObj, bool *background, int *vol, int *pan, int *x, int *y) {
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

static void SayLine() {
	int vol = 127, buffer = 64, paramId = 1, x = -1, y = -1;
	bool background = true;
	char msgId[50];
	Common::String msg;
	lua_Object paramObj = lua_getparam(paramId++);

	if ((lua_isuserdata(paramObj) && lua_tag(paramObj) == MKID_BE('ACTR'))
			|| lua_isstring(paramObj) || lua_istable(paramObj)) {
		Actor *actor = NULL;//some_Actor, maybe some current actor
		if (lua_isuserdata(paramObj) && lua_tag(paramObj) == MKID_BE('ACTR')) {
			actor = static_cast<Actor *>(lua_getuserdata(paramObj));
			paramObj = lua_getparam(paramId++);
		}
		if (actor) {
			if (lua_isnil(paramObj)) {
loop:
				if (!msg.empty()) {
					actor->sayLine(msg.c_str(), msgId); //background, vol, pan, x, y
				}
			} else {
				while (1) {
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
					if (lua_isnil(paramObj))
						goto loop;
				}
			}
		}
	}
}

static void InputDialog() {
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

static void IsMessageGoing() {
	lua_Object actorObj = lua_getparam(1);

	if (!actorObj || (lua_isuserdata(actorObj) && lua_tag(actorObj) == MKID_BE('ACTR')) || lua_isnil(actorObj)) {
		if (lua_isuserdata(actorObj) && lua_tag(actorObj) == MKID_BE('ACTR')) {
			Actor *actor = static_cast<Actor *>(lua_getuserdata(actorObj));
			if (actor) {
				pushbool(actor->talking());
			}
		} else {
			// TODO
			// this part code check something more
			pushbool(g_imuse->isVoicePlaying());
		}
	} else
		lua_pushnil();
}

static void ShutUpActor() {
	lua_Object actorObj = lua_getparam(1);

	if (!lua_isuserdata(actorObj) || lua_tag(actorObj) != MKID_BE('ACTR'))
		return;
	Actor *actor = static_cast<Actor *>(lua_getuserdata(actorObj));
	if (actor)
		actor->shutUp();
}

// Sector functions
/* Find the sector (of any type) which contains
 * the requested coordinate (x,y,z).
 */
static void GetPointSector() {
	lua_Object xObj = lua_getparam(1);
	lua_Object yObj = lua_getparam(2);
	lua_Object zObj = lua_getparam(3);
	lua_Object typeObj = lua_getparam(4);
	int sectorType;

	if (!lua_isnumber(xObj) || !lua_isnumber(yObj) || !lua_isnumber(zObj)) {
		lua_pushnil();
		return;
	}
	if (lua_isnil(typeObj))
		sectorType = 0x2000;
	else
		sectorType = (int)lua_getnumber(typeObj);

	float x = lua_getnumber(xObj);
	float y = lua_getnumber(yObj);
	float z = lua_getnumber(zObj);

	Graphics::Vector3d point(x, y, z);
	Sector *result = g_grim->currScene()->findPointSector(point, sectorType);
	if (result) {
		lua_pushnumber(result->id());
		lua_pushstring(const_cast<char *>(result->name()));
		lua_pushnumber(result->type());
	} else {
		lua_pushnil();
	}
}

static void GetActorSector() {
	lua_Object actorObj = lua_getparam(1);
	lua_Object typeObj = lua_getparam(2);

	if (!lua_isuserdata(actorObj) || lua_tag(actorObj) != MKID_BE('ACTR'))
		return;
	if (!lua_isnumber(typeObj))
		return;

	Actor *actor = static_cast<Actor *>(lua_getuserdata(actorObj));
	int sectorType = (int)lua_getnumber(typeObj);
	Sector *result = g_grim->currScene()->findPointSector(actor->pos(), sectorType);
	if (result) {
		lua_pushnumber(result->id());
		lua_pushstring(const_cast<char *>(result->name()));
		lua_pushnumber(result->type());
	} else {
		lua_pushnil();
	}
}

static void IsActorInSector() {
	lua_Object actorObj = lua_getparam(1);
	lua_Object nameObj = lua_getparam(2);
	if (!lua_isuserdata(actorObj) || lua_tag(actorObj) != MKID_BE('ACTR'))
		return;
	if (!lua_isstring(nameObj)) {
		lua_pushnil();
		return;
	}

	Actor *actor = static_cast<Actor *>(lua_getuserdata(actorObj));
	const char *name = lua_getstring(nameObj);

	int numSectors = g_grim->currScene()->getSectorCount();
	for (int i = 0; i < numSectors; i++) {
		Sector *sector = g_grim->currScene()->getSectorBase(i);
		if (strmatch(sector->name(), name)) {
			if (sector->isPointInSector(actor->pos())) {
				lua_pushnumber(sector->id());
				lua_pushstring((char *)sector->name());
				lua_pushnumber(sector->type());
				return;
			}
		}
	}
	lua_pushnil();
}

static void MakeSectorActive() {
	lua_Object sectorObj = lua_getparam(1);

	if (!lua_isnumber(sectorObj) && !lua_isstring(sectorObj))
		return;

	// FIXME: This happens on initial load. Are we initting something in the wrong order?
	if (!g_grim->currScene()) {
		warning("!!!! Trying to call MakeSectorActive without a scene");
		return;
	}

	bool visible = !lua_isnil(lua_getparam(2));
	int numSectors = g_grim->currScene()->getSectorCount();
	if (lua_isstring(sectorObj)) {
		const char *name = lua_getstring(sectorObj);
		for (int i = 0; i < numSectors; i++) {
			Sector *sector = g_grim->currScene()->getSectorBase(i);
			if (strmatch(sector->name(), name)) {
				sector->setVisible(visible);
				return;
			}
		}
	} else if (lua_isnumber(sectorObj)) {
		int id = (int)lua_getnumber(sectorObj);
		for (int i = 0; i < numSectors; i++) {
			Sector *sector = g_grim->currScene()->getSectorBase(i);
			if (sector->id() == id) {
				sector->setVisible(visible);
				return;
			}
		}
	}
}

// Scene functions
static void LockSet() {
	lua_Object nameObj = lua_getparam(1);
	if (!lua_isstring(nameObj))
		return;

	const char *name = lua_getstring(nameObj);
	// TODO implement proper locking
	g_grim->setSceneLock(name, true);
}

static void UnLockSet() {
	lua_Object nameObj = lua_getparam(1);
	if (!lua_isstring(nameObj))
		return;

	const char *name = lua_getstring(nameObj);
	// TODO implement proper unlocking
	g_grim->setSceneLock(name, false);
}

static void MakeCurrentSet() {
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

static void MakeCurrentSetup() {
	lua_Object setupObj = lua_getparam(1);
	if (!lua_isnumber(setupObj))
		return;

	int num = (int)lua_getnumber(lua_getparam(1));
	g_grim->makeCurrentSetup(num);
}

/* Find the requested scene and return the current setup
 * id number.  This function cannot just use the current
 * scene or else when Manny opens his inventory information
 * gets lost, such as the position for the demon beavors
 * in the Petrified Forest.
 */
static void GetCurrentSetup() {
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
	lua_pushnumber(scene->setup());
}

static void GetShrinkPos() {
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

static void ImStartSound() {
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

static void ImStopSound() {
	lua_Object nameObj = lua_getparam(1);
	if (lua_isnumber(nameObj))
		error("ImStopsound: name from value not supported");

	const char *soundName = lua_getstring(nameObj);
	g_imuse->stopSound(soundName);
}

static void ImStopAllSounds() {
	g_imuse->stopAllSounds();
}

static void ImPause() {
	g_imuse->pause(true);
}

static void ImResume() {
	g_imuse->pause(false);
}

static void ImSetVoiceEffect() {
	const char *effectName;

	effectName = luaL_check_string(1);
	if (gDebugLevel == DEBUG_WARN || gDebugLevel == DEBUG_ALL)
		warning("ImSetVoiceEffect(%s) Voice effects are not yet supported", effectName);
}

static void ImSetMusicVol() {
	lua_Object volObj = lua_getparam(1);
	if (!lua_isnumber(volObj))
		return;
	g_system->getMixer()->setVolumeForSoundType(Audio::Mixer::kMusicSoundType, (int)lua_getnumber(volObj));
}

static void ImGetMusicVol() {
	lua_pushnumber(g_system->getMixer()->getVolumeForSoundType(Audio::Mixer::kMusicSoundType));
}

static void ImSetVoiceVol() {
	lua_Object volObj = lua_getparam(1);
	if (!lua_isnumber(volObj))
		return;
	g_system->getMixer()->setVolumeForSoundType(Audio::Mixer::kSpeechSoundType, (int)lua_getnumber(volObj));
}

static void ImGetVoiceVol() {
	lua_pushnumber(g_system->getMixer()->getVolumeForSoundType(Audio::Mixer::kSpeechSoundType));
}

static void ImSetSfxVol() {
	lua_Object volObj = lua_getparam(1);
	if (!lua_isnumber(volObj))
		return;
	g_system->getMixer()->setVolumeForSoundType(Audio::Mixer::kSFXSoundType, (int)lua_getnumber(volObj));
}

static void ImGetSfxVol() {
	lua_pushnumber(g_system->getMixer()->getVolumeForSoundType(Audio::Mixer::kSFXSoundType));
}

static void ImSetParam() {
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

void ImGetParam() {
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

static void ImFadeParam() {
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

static void ImSetState() {
	lua_Object stateObj = lua_getparam(1);
	if (!lua_isnumber(stateObj))
		return;

	g_imuseState = (int)lua_getnumber(stateObj);
}

static void ImSetSequence() {
	lua_Object stateObj = lua_getparam(1);
	if (!lua_isnumber(stateObj))
		return;

	int state = (int)lua_getnumber(stateObj);
	lua_pushnumber(g_imuse->setMusicSequence(state));
}

static void SaveIMuse() {
	SaveGame *savedIMuse = new SaveGame("grim.tmp", true);
	if (!savedIMuse)
		return;
	g_imuse->saveState(savedIMuse);
	delete savedIMuse;
}

static void RestoreIMuse() {
	SaveGame *savedIMuse = new SaveGame("grim.tmp", false);
	if (!savedIMuse)
		return;
	g_imuse->stopAllSounds();
	g_imuse->resetState();
	g_imuse->restoreState(savedIMuse);
	delete savedIMuse;
	g_system->getSavefileManager()->removeSavefile("grim.tmp");
}

static void SetSoundPosition() {
	Graphics::Vector3d pos;
	int minVolume = 10;
	int maxVolume = 127;
	float someParam = 0;
	int argId = 1;
	lua_Object paramObj;

	if (g_grim->currScene()) {
		g_grim->currScene()->getSoundParameters(&minVolume, &maxVolume);
	}

	lua_Object nameObj = lua_getparam(argId++);
	if (!lua_isnumber(nameObj) && !lua_isstring(nameObj))
		return;

	lua_Object actorObj = lua_getparam(argId++);
	if (lua_isuserdata(actorObj) && lua_tag(actorObj) == MKID_BE('ACTR')) {
		Actor *actor = static_cast<Actor *>(lua_getuserdata(actorObj));
		if (!actor)
			return;
		pos = actor->pos();
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

	if (g_grim->currScene()) {
		if (lua_isnumber(nameObj))
			error("SetSoundPosition: number is not yet supported");
		else {
			const char *soundName = lua_getstring(nameObj);
			g_grim->currScene()->setSoundParameters(minVolume, maxVolume);
			g_grim->currScene()->setSoundPosition(soundName, pos);
		}
	}
}

static void IsSoundPlaying() {
	// dummy
}

static void PlaySoundAt() {
	// dummy
}

static void FileFindDispose() {
	if (g_filesiter)
		g_filesiter->begin();
	g_listfiles.clear();
}

static void luaFileFindNext() {
	if (g_filesiter == g_listfiles.end()) {
		lua_pushnil();
		FileFindDispose();
	} else {
		lua_pushstring(g_filesiter->c_str());
		g_filesiter++;
	}
}

static void luaFileFindFirst() {
	lua_Object extObj = lua_getparam(1);
	if (!lua_isstring(extObj)) {
		lua_pushnil();
		return;
	}
		
	FileFindDispose();

	const char *extension = lua_getstring(extObj);
	Common::SaveFileManager *saveFileMan = g_system->getSavefileManager();
	g_listfiles = saveFileMan->listSavefiles(extension);
	g_filesiter = g_listfiles.begin();

	if (g_filesiter == g_listfiles.end())
		lua_pushnil();
	else
		luaFileFindNext();
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

void PerSecond() {
	lua_Object rateObj = lua_getparam(1);

	if (!lua_isnumber(rateObj)) {
		lua_pushnil();
		return;
	}
	float rate = lua_getnumber(rateObj);
	lua_pushnumber(g_grim->perSecond(rate));
}

void EnableControl() {
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

void DisableControl() {
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

void GetControlState() {
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

static void killBitmapPrimitives(Bitmap *bitmap) {
	for (GrimEngine::PrimitiveListType::const_iterator i = g_grim->primitivesBegin(); i != g_grim->primitivesEnd(); i++) {
		PrimitiveObject *p = *i;
		if (p->isBitmap() && p->getBitmapHandle() == bitmap) {
			g_grim->killPrimitiveObject(p);
			break;
		}
	}
}

static void GetImage() {
	lua_Object nameObj = lua_getparam(1);
	if (!lua_isstring(nameObj)) {
		lua_pushnil();
		return;
	}
	const char *bitmapName = lua_getstring(nameObj);
	Bitmap *image = g_resourceloader->loadBitmap(bitmapName);
	image->luaRef();
	lua_pushusertag(image, MKID_BE('VBUF'));
}

static void FreeImage() {
	lua_Object param = lua_getparam(1);
	if (!lua_isuserdata(param) || lua_tag(param) != MKID_BE('VBUF'))
		return;
	Bitmap *bitmap = static_cast<Bitmap *>(lua_getuserdata(param));
	killBitmapPrimitives(bitmap);
}

static void BlastImage() {
	lua_Object param = lua_getparam(1);
	if (!lua_isuserdata(param) || lua_tag(param) != MKID_BE('VBUF'))
		return;
	Bitmap *bitmap = static_cast<Bitmap *>(lua_getuserdata(param));
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
		if (lua_isuserdata(keyObj) && lua_tag(keyObj) == MKID_BE('FONT')) {
			textObject->setFont(static_cast<Font *>(lua_getuserdata(keyObj)));
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
		if (lua_isuserdata(keyObj) && lua_tag(keyObj) == MKID_BE('COLR')) {
			textObject->setFGColor(static_cast<Color *>(lua_getuserdata(keyObj)));
		}
	}

	lua_pushobject(tableObj);
	lua_pushobject(lua_getref(refTextObjectBGColor));
	keyObj = lua_gettable();
	if (keyObj) {
		if (lua_isuserdata(keyObj) && lua_tag(keyObj) == MKID_BE('COLR')) {
			//textObject->setBGColor(static_cast<Color *>(lua_getuserdata(keyObj)));
			warning("setTextObjectParams: dummy BGColor");
		}
	}

	lua_pushobject(tableObj);
	lua_pushobject(lua_getref(refTextObjectFXColor));
	keyObj = lua_gettable();
	if (keyObj) {
		if (lua_isuserdata(keyObj) && lua_tag(keyObj) == MKID_BE('COLR')) {
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

static void CleanBuffer() {
	g_driver->copyStoredToDisplay();
}

static void Exit() {
	exit(0);
}

/* Destroy a text object since we don't need it anymore
 * note that the menu creates more objects than it needs,
 * so it deletes some objects right after creating them
 */
static void KillTextObject() {
	lua_Object textObj = lua_getparam(1);

	if (lua_isuserdata(textObj) && lua_tag(textObj) == MKID_BE('TEXT')) {
		TextObject *textObject = static_cast<TextObject *>(lua_getuserdata(textObj));
		// TODO rewrote removing, but change only status, I guess disable or flag to removal
		// as it's done in original engine
		g_grim->killTextObject(textObject);
	}
}

/* Make changes to a text object based on the parameters passed
 * in the table in the LUA parameter 2.
 */
static void ChangeTextObject() {
	const char *line;
	lua_Object textObj = lua_getparam(1);
	int paramId = 2;
	if (lua_isuserdata(textObj) && lua_tag(textObj) == MKID_BE('TEXT')) {
		TextObject *textObject = static_cast<TextObject *>(lua_getuserdata(textObj));
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
				textObject->setText((char *)line);
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
static void GetTextSpeed() {
	lua_pushnumber(g_grim->getTextSpeed());
}

static void SetTextSpeed() {
	lua_Object speedObj = lua_getparam(1);
	if (!lua_isnumber(speedObj))
		return;

	int speed = (int)lua_getnumber(speedObj);
	g_grim->setTextSpeed(speed);
}

static void MakeTextObject() {
	TextObject *textObject = new TextObject(false);
	lua_Object textObj = lua_getparam(1);
	if (!lua_isstring(textObj))
		return;

	const char *line = lua_getstring(textObj);
	Common::String text = line;

	textObject->setDefaults(&blastTextDefaults);
	lua_Object tableObj = lua_getparam(2);
	if (lua_istable(tableObj))
		setTextObjectParams(textObject, tableObj);

	textObject->setText((char *)text.c_str());
	textObject->createBitmap();
	g_grim->registerTextObject(textObject);

	lua_pushusertag(textObject, MKID_BE('TEXT'));
	lua_pushnumber(textObject->getBitmapWidth());
	lua_pushnumber(textObject->getBitmapHeight());
}

static void GetTextObjectDimensions() {
	lua_Object textObj = lua_getparam(1);

	if (lua_isuserdata(textObj) && lua_tag(textObj) == MKID_BE('TEXT')) {
		TextObject *textObject = static_cast<TextObject *>(lua_getuserdata(textObj));
		lua_pushnumber(textObject->getBitmapWidth());
		lua_pushnumber(textObject->getBitmapHeight());
	}
}

static void ExpireText() {
	// Expire all the text objects
	g_grim->killTextObjects();
	// Cleanup actor references to deleted text objects
	for (GrimEngine::ActorListType::const_iterator i = g_grim->actorsBegin(); i != g_grim->actorsEnd(); i++)
		(*i)->lineCleanup();
}

static void GetTextCharPosition() {
	lua_Object textObj = lua_getparam(1);
	if (lua_isuserdata(textObj) && lua_tag(textObj) == MKID_BE('TEXT')) {
		TextObject *textObject = static_cast<TextObject *>(lua_getuserdata(textObj));
		int pos = (int)lua_getnumber(lua_getparam(2));
		lua_pushnumber(textObject->getTextCharPosition(pos));
	}
}

static void BlastText() {
	TextObject *textObject = new TextObject(true);
	lua_Object textObj = lua_getparam(1);
	if (!lua_isstring(textObj))
		return;

	const char *line = lua_getstring(textObj);
	Common::String text = line;

	textObject->setDefaults(&blastTextDefaults);
	lua_Object tableObj = lua_getparam(2);
	if (lua_istable(tableObj))
		setTextObjectParams(textObject, tableObj);

	textObject->setText((char *)text.c_str());
	textObject->createBitmap();
	textObject->draw();
	delete textObject;
}

static void SetOffscreenTextPos() {
	// this sets where we shouldn't put dialog maybe?
}

static void SetSpeechMode() {
	int mode;

	mode = (int)lua_getnumber(lua_getparam(1));
	if (mode >= 1 && mode <= 3)
		g_grim->setSpeechMode(mode);
}

static void GetSpeechMode() {
	int mode;

	mode = g_grim->getSpeechMode();
	lua_pushnumber(mode);
}

static void StartFullscreenMovie() {
	bool looping = getbool(2);

	lua_Object name = lua_getparam(1);
	if (!lua_isstring(name)) {
		lua_pushnil();
		return;
	}
	CleanBuffer();
	g_grim->setMode(ENGINE_MODE_SMUSH);
	pushbool(g_smush->play(lua_getstring(name), looping, 0, 0));
}

static void StartMovie() {
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
static void IsFullscreenMoviePlaying() {
	pushbool(g_smush->isPlaying() && g_grim->getMode() == ENGINE_MODE_SMUSH);
}

static void IsMoviePlaying() {
	pushbool(g_smush->isPlaying() && g_grim->getMode() == ENGINE_MODE_NORMAL);
}

static void StopMovie() {
	g_smush->stop();
}

static void PauseMovie() {
	g_smush->pause(lua_isnil(lua_getparam(1)) != 0);
}

static void PurgePrimitiveQueue() {
	g_grim->killPrimitiveObjects();
}

static void DrawPolygon() {
	lua_Object pointObj;
	Common::Point p1, p2, p3, p4;
	Color color;

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
		if (lua_isuserdata(colorObj) && lua_tag(colorObj) == MKID_BE('COLR')) {
			color = static_cast<Color *>(lua_getuserdata(colorObj));
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
	lua_pushusertag(p, MKID_BE('PRIM'));
}

static void DrawLine() {
	Common::Point p1, p2;
	Color color;
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
		if (lua_isuserdata(colorObj) && lua_tag(colorObj) == MKID_BE('COLR')) {
			color = static_cast<Color *>(lua_getuserdata(colorObj));
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
	lua_pushusertag(p, MKID_BE('PRIM'));
}

static void ChangePrimitive() {
	PrimitiveObject *psearch, *pmodify = NULL;
	Color color;

	lua_Object param1 = lua_getparam(1);
	if (!lua_isuserdata(param1) || lua_tag(param1) != MKID_BE('PRIM'))
		return;

	lua_Object tableObj = lua_getparam(2);
	if (!lua_istable(tableObj))
		return;

	psearch = static_cast<PrimitiveObject *>(lua_getuserdata(param1));

	for (GrimEngine::PrimitiveListType::const_iterator i = g_grim->primitivesBegin(); i != g_grim->primitivesEnd(); i++) {
		PrimitiveObject *p = *i;
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
	if (lua_isuserdata(colorObj) && lua_tag(colorObj) == MKID_BE('COLR')) {
		color = static_cast<Color *>(lua_getuserdata(colorObj));
		pmodify->setColor(color);
	}

	lua_pushobject(tableObj);
	lua_pushstring("layer");
	lua_Object layer = lua_gettable();
	if (lua_isnumber(layer)) {
		// TODO pmodify->setLayer(lua_getnumber(layer));
		assert(0);
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

static void DrawRectangle() {
	Common::Point p1, p2;
	Color color;
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
		if (lua_isuserdata(colorObj) && lua_tag(colorObj) == MKID_BE('COLR')) {
			color = static_cast<Color *>(lua_getuserdata(colorObj));
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
	lua_pushusertag(p, MKID_BE('PRIM')); // FIXME: we use PRIM usetag here
}

static void BlastRect() {
	Common::Point p1, p2;
	Color color;
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
		if (lua_isuserdata(colorObj) && lua_tag(colorObj) == MKID_BE('COLR')) {
			color = static_cast<Color *>(lua_getuserdata(colorObj));
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

static void DimScreen() {
	g_driver->dimScreen();
}

static void DimRegion() {
	int x = (int)lua_getnumber(lua_getparam(1));
	int y = (int)lua_getnumber(lua_getparam(2));
	int w = (int)lua_getnumber(lua_getparam(3));
	int h = (int)lua_getnumber(lua_getparam(4));
	float level = lua_getnumber(lua_getparam(5));
	g_driver->dimRegion(x, y, w, h, level);
}

static void GetDiskFreeSpace() {
	// amount of free space in MB, used for creating saves
	lua_pushnumber(50);
}

static void NewObjectState() {
	int setupID = (int)lua_getnumber(lua_getparam(1));
	int val = (int)lua_getnumber(lua_getparam(2));
	ObjectState::Position pos = (ObjectState::Position)val;
	const char *bitmap = lua_getstring(lua_getparam(3));
	const char *zbitmap = NULL;
	if (!lua_isnil(lua_getparam(4)))
		zbitmap = lua_getstring(lua_getparam(4));
	bool transparency = getbool(5);

	ObjectState *state = new ObjectState(setupID, pos, bitmap, zbitmap, transparency);
	g_grim->currScene()->addObjectState(state);
	lua_pushusertag(state, MKID_BE('STAT'));
}

static void FreeObjectState() {
	lua_Object param = lua_getparam(1);
	if (!lua_isuserdata(param) || lua_tag(param) != MKID_BE('STAT'))
		return;
	ObjectState *state =  static_cast<ObjectState *>(lua_getuserdata(param));
	g_grim->currScene()->deleteObjectState(state);
}

static void SendObjectToBack() {
	lua_Object param = lua_getparam(1);
	if (lua_isuserdata(param) && lua_tag(param) == MKID_BE('STAT')) {
		ObjectState *state = static_cast<ObjectState *>(lua_getuserdata(param));
		g_grim->currScene()->moveObjectStateToFirst(state);
	}
}

static void SendObjectToFront() {
	lua_Object param = lua_getparam(1);
	if (lua_isuserdata(param) && lua_tag(param) == MKID_BE('STAT')) {
		ObjectState *state = static_cast<ObjectState *>(lua_getuserdata(param));
		g_grim->currScene()->moveObjectStateToLast(state);
	}
}

static void SetObjectType() {
	lua_Object param = lua_getparam(1);
	if (!lua_isuserdata(param) || lua_tag(param) != MKID_BE('STAT'))
		return;
	ObjectState *state = static_cast<ObjectState *>(lua_getuserdata(param));
	int val = (int)lua_getnumber(lua_getparam(2));
	ObjectState::Position pos = (ObjectState::Position)val;
	state->setPos(pos);
}

static void GetCurrentScript() {
	current_script();
}

static void ScreenShot() {
	int width = (int)lua_getnumber(lua_getparam(1));
	int height = (int)lua_getnumber(lua_getparam(2));
	int mode = g_grim->getMode();
	g_grim->setMode(ENGINE_MODE_NORMAL);
	g_grim->updateDisplayScene();
	Bitmap *screenshot = g_driver->getScreenshot(width, height);
	g_grim->setMode(mode);
	if (screenshot) {
		screenshot->luaRef();
		lua_pushusertag(screenshot, MKID_BE('VBUF'));
	} else {
		lua_pushnil();
	}
}

static void GetSaveGameImage() {
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
	screenshot = new Bitmap(data, width, height, "screenshot");
	if (screenshot) {
		screenshot->luaRef();
		lua_pushusertag(screenshot, MKID_BE('VBUF'));
	} else {
		lua_pushnil();
		warning("Could not restore screenshot from file");
		return;
	}
	savedState->endSection();
	delete savedState;
}

static void SubmitSaveGameData() {
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

static void GetSaveGameData() {
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

static void Load() {
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

static void Save() {
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

static void lua_remove() {
	if (g_system->getSavefileManager()->removeSavefile(luaL_check_string(1)))
		lua_pushuserdata(NULL);
	else {
		lua_pushnil();
		lua_pushstring(g_system->getSavefileManager()->getErrorDesc().c_str());
	}
}

static PointerId saveCallback(int32 /*tag*/, PointerId ptr, SaveSint32 /*savedState*/) {
	return ptr;
}

static PointerId restoreCallback(int32 /*tag*/, PointerId ptr, RestoreSint32 /*savedState*/) {
	return ptr;
}

static void LockFont() {
	lua_Object param1 = lua_getparam(1);
	if (lua_isstring(param1)) {
		const char *fontName = lua_getstring(param1);
		Font *result = g_resourceloader->loadFont(fontName);
		if (result) {
			result->luaRef();
			lua_pushusertag(result, MKID_BE('FONT'));
			return;
		}
	}

	lua_pushnil();
}

static void EnableDebugKeys() {
}

static void LightMgrSetChange() {
	// that seems only used when some control panel is opened
}

static void SetAmbientLight() {
	int mode = (int)lua_getnumber(lua_getparam(1));
	if (mode == 0) {
		if (g_grim->currScene()) {
			g_grim->currScene()->setLightEnableState(true);
		}
	} else if (mode == 1) {
		if (g_grim->currScene()) {
			g_grim->currScene()->setLightEnableState(false);
		}
	}
}

static void RenderModeUser() {
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

static void SetGamma() {
}

static void Display() {
	if (g_grim->getFlipEnable()) {
		g_driver->flipBuffer();
	}
}

static void EngineDisplay() {
	// it enable/disable updating display
	bool mode = (int)lua_getnumber(lua_getparam(1)) != 0;
	if (mode) {
		g_grim->setFlipEnable(true);
	} else {
		g_grim->setFlipEnable(false);
	}
}

static void ForceRefresh() {
	g_grim->refreshDrawMode();
}

static void JustLoaded() {
	if (gDebugLevel == DEBUG_ERROR || gDebugLevel == DEBUG_ALL)
		error("OPCODE USAGE VERIFICATION: JustLoaded");
}

static void PlaySound() {
	if (gDebugLevel == DEBUG_ERROR || gDebugLevel == DEBUG_ALL)
		error("OPCODE USAGE VERIFICATION: PlaySound");
}

static void SetEmergencyFont() {
	if (gDebugLevel == DEBUG_ERROR || gDebugLevel == DEBUG_ALL)
		error("OPCODE USAGE VERIFICATION: SetEmergencyFont");
}

static void LoadBundle() {
	// loading grimdemo.mus is allready handled
}

void typeOverride() {
	lua_Object data = lua_getparam(1);

	if (lua_isuserdata(data)) {
		switch (lua_tag(data)) {
		case MKID_BE('ACTR'):
			lua_pushstring("actor");
			lua_pushnumber(lua_tag(data));
			return;
		case MKID_BE('COST'):
			lua_pushstring("costume");
			lua_pushnumber(lua_tag(data));
			return;
		case MKID_BE('SET '):
			lua_pushstring("set");
			lua_pushnumber(lua_tag(data));
			return;
		case MKID_BE('KEYF'):
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

void concatFallback() {
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
		else if (lua_tag(params[i]) == MKID_BE('ACTR')) {
			Actor *a = static_cast<Actor *>(lua_getuserdata(params[i]));
			sprintf(strPtr, "(actor%p:%s)", (void *)a,
				(a->currentCostume() && a->currentCostume()->getModelNodes()) ?
				a->currentCostume()->getModelNodes()->_name : "");
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

// Stub function for builtin functions not yet implemented
static void stubWarning(const char *funcName) {
	warning("Stub function: %s", funcName);
}

#define STUB_FUNC(name) static void name() { stubWarning(#name); }
STUB_FUNC(SetActorInvClipNode)
STUB_FUNC(NukeResources)
STUB_FUNC(UnShrinkBoxes)
STUB_FUNC(ShrinkBoxes)
STUB_FUNC(ResetTextures)
STUB_FUNC(AttachToResources)
STUB_FUNC(DetachFromResources)
STUB_FUNC(IrisUp)
STUB_FUNC(IrisDown)
STUB_FUNC(FadeInChore)
STUB_FUNC(FadeOutChore)
STUB_FUNC(SetActorClipPlane)
STUB_FUNC(SetActorClipActive)
STUB_FUNC(SetActorCollisionScale)
STUB_FUNC(SetActorCollisionMode)
STUB_FUNC(FlushControls)
STUB_FUNC(ActorToClean)
STUB_FUNC(LightMgrStartup)
STUB_FUNC(SetLightIntensity)
STUB_FUNC(SetLightPosition)
STUB_FUNC(TurnLightOn)
STUB_FUNC(GetAngleBetweenVectors)
STUB_FUNC(GetCameraLookVector)
STUB_FUNC(SetCameraRoll)
STUB_FUNC(SetCameraInterest)
STUB_FUNC(GetCameraPosition)
STUB_FUNC(SpewStartup)
STUB_FUNC(PreRender)
STUB_FUNC(GetSectorOppositeEdge)
STUB_FUNC(PreviousSetup)
STUB_FUNC(NextSetup)
STUB_FUNC(WorldToScreen)
STUB_FUNC(SetActorRoll)
STUB_FUNC(IsPointInSector)
STUB_FUNC(SetActorFrustrumCull)
STUB_FUNC(DriveActorTo)
STUB_FUNC(GetActorRect)
STUB_FUNC(SetActorTimeScale)
STUB_FUNC(SetActorScale)
STUB_FUNC(GetTranslationMode)
STUB_FUNC(SetTranslationMode)
STUB_FUNC(PrintLine)
STUB_FUNC(KillPrimitive)
STUB_FUNC(WalkActorToAvoiding)
STUB_FUNC(GetActorChores)
STUB_FUNC(SetCameraPosition)
STUB_FUNC(GetCameraFOV)
STUB_FUNC(SetCameraFOV)
STUB_FUNC(GetCameraRoll)
STUB_FUNC(ActorPuckOrient)
STUB_FUNC(GetMemoryUsage)
STUB_FUNC(GetFontDimensions)
STUB_FUNC(PurgeText)

static void dummyHandler() {
}

// Entries in the system table
static struct {
	const char *name;
	int key;
} system_defaults[] = {
	{ "frameTime", 0 },
	{ "movieTime", 0 }
};

struct luaL_reg mainOpcodes[] = {
	{ "EngineDisplay", EngineDisplay },
	{ "CheckForFile", CheckForFile },
	{ "Load", Load },
	{ "Save", Save },
	{ "remove", lua_remove },
	{ "SetActorColormap", SetActorColormap },
	{ "GetActorCostume", GetActorCostume },
	{ "SetActorCostume", SetActorCostume },
	{ "SetActorScale", SetActorScale },
	{ "GetActorTimeScale", GetActorTimeScale },
	{ "SetActorTimeScale", SetActorTimeScale },
	{ "GetActorNodeLocation", GetActorNodeLocation },
	{ "SetActorWalkChore", SetActorWalkChore },
	{ "SetActorTurnChores", SetActorTurnChores },
	{ "SetActorRestChore", SetActorRestChore },
	{ "SetActorMumblechore", SetActorMumblechore },
	{ "SetActorTalkChore", SetActorTalkChore },
	{ "SetActorWalkRate", SetActorWalkRate },
	{ "GetActorWalkRate", GetActorWalkRate },
	{ "SetActorTurnRate", SetActorTurnRate },
	{ "SetSelectedActor", SetSelectedActor },
	{ "LoadActor", LoadActor },
	{ "GetActorPos", GetActorPos },
	{ "GetActorRect", GetActorRect },
	{ "GetActorPuckVector", GetActorPuckVector },
	{ "GetActorYawToPoint", GetActorYawToPoint },
	{ "SetActorReflection", SetActorReflection },
	{ "PutActorAtInterest", PutActorAtInterest },
	{ "PutActorAt", PutActorAt },
	{ "PutActorInSet", PutActorInSet },
	{ "WalkActorVector", WalkActorVector },
	{ "WalkActorForward", WalkActorForward },
	{ "DriveActorTo", DriveActorTo },
	{ "WalkActorTo", WalkActorTo },
	{ "WalkActorToAvoiding", WalkActorToAvoiding },
	{ "ActorLookAt", ActorLookAt },
	{ "SetActorLookRate", SetActorLookRate },
	{ "GetActorLookRate", GetActorLookRate },
	{ "GetVisibleThings", GetVisibleThings },
	{ "GetCameraActor", GetCameraActor },
	{ "SetActorHead", SetActorHead },
	{ "SetActorVisibility", SetActorVisibility },
	{ "SetActorFollowBoxes", SetActorFollowBoxes },
	{ "ShutUpActor", ShutUpActor },
	{ "SetActorFrustrumCull", SetActorFrustrumCull },
	{ "IsActorInSector", IsActorInSector },
	{ "GetActorSector", GetActorSector },
	{ "IsPointInSector", IsPointInSector },
	{ "GetPointSector", GetPointSector },
	{ "TurnActor", TurnActor },
	{ "GetActorRot", GetActorRot },
	{ "SetActorRot", SetActorRot },
	{ "SetActorPitch", SetActorPitch },
	{ "SetActorRoll", SetActorRoll },
	{ "IsActorTurning", IsActorTurning },
	{ "PlayActorChore", PlayActorChore },
	{ "PlayActorChoreLooping", PlayActorChoreLooping },
	{ "StopActorChore", StopActorChore },
	{ "CompleteActorChore", CompleteActorChore },
	{ "IsActorMoving", IsActorMoving },
	{ "IsActorChoring", IsActorChoring },
	{ "IsActorResting", IsActorResting },
	{ "SetActorChoreLooping", SetActorChoreLooping },
	{ "GetActorChores", GetActorChores },
	{ "GetActorCostumeDepth", GetActorCostumeDepth },
	{ "WorldToScreen", WorldToScreen },
	{ "exit", Exit },
	{ "FunctionName", FunctionName },
	{ "EnableDebugKeys", EnableDebugKeys },
	{ "LockFont", LockFont },
	{ "EnableControl", EnableControl },
	{ "DisableControl", DisableControl },
	{ "GetControlState", GetControlState },
	{ "PrintError", PrintError },
	{ "PrintWarning", PrintWarning },
	{ "PrintDebug", PrintDebug },
	{ "MakeCurrentSet", MakeCurrentSet },
	{ "LockSet", LockSet },
	{ "UnLockSet", UnLockSet },
	{ "MakeCurrentSetup", MakeCurrentSetup },
	{ "GetCurrentSetup", GetCurrentSetup },
	{ "NextSetup", NextSetup },
	{ "PreviousSetup", PreviousSetup },
	{ "StartFullscreenMovie", StartFullscreenMovie },
	{ "IsFullscreenMoviePlaying", IsFullscreenMoviePlaying },
	{ "StartMovie", StartMovie },
	{ "StopMovie", StopMovie },
	{ "PauseMovie", PauseMovie },
	{ "IsMoviePlaying", IsMoviePlaying },
	{ "PlaySound", PlaySound },
	{ "PlaySoundAt", PlaySoundAt },
	{ "IsSoundPlaying", IsSoundPlaying },
	{ "SetSoundPosition", SetSoundPosition },
	{ "FileFindFirst", luaFileFindFirst },
	{ "FileFindNext", luaFileFindNext },
	{ "FileFindDispose", FileFindDispose },
	{ "InputDialog", InputDialog },
	{ "WriteRegistryValue", WriteRegistryValue },
	{ "ReadRegistryValue", ReadRegistryValue },
	{ "GetSectorOppositeEdge", GetSectorOppositeEdge },
	{ "MakeSectorActive", MakeSectorActive },
	{ "PreRender", PreRender },
	{ "SpewStartup", SpewStartup },
	{ "GetCurrentScript", GetCurrentScript },
	{ "PrintActorCostumes", PrintActorCostumes },
	{ "PushActorCostume", PushActorCostume },
	{ "PopActorCostume", PopActorCostume },
	{ "LoadCostume", LoadCostume },
	{ "RotateVector", RotateVector },
	{ "GetCameraPosition", GetCameraPosition },
	{ "SetCameraPosition", SetCameraPosition },
	{ "SetCameraInterest", SetCameraInterest },
	{ "GetCameraFOV", GetCameraFOV },
	{ "SetCameraFOV", SetCameraFOV },
	{ "GetCameraRoll", GetCameraRoll },
	{ "SetCameraRoll", SetCameraRoll },
	{ "GetCameraLookVector", GetCameraLookVector },
	{ "PointActorAt", PointActorAt },
	{ "TurnActorTo", TurnActorTo },
	{ "PerSecond", PerSecond },
	{ "GetAngleBetweenVectors", GetAngleBetweenVectors },
	{ "GetAngleBetweenActors", GetAngleBetweenActors },
	{ "SetAmbientLight", SetAmbientLight },
	{ "TurnLightOn", TurnLightOn },
	{ "SetLightPosition", SetLightPosition },
	{ "SetLightIntensity", SetLightIntensity },
	{ "LightMgrSetChange", LightMgrSetChange },
	{ "LightMgrStartup", LightMgrStartup },
	{ "ImStartSound", ImStartSound },
	{ "ImStopSound", ImStopSound },
	{ "ImStopAllSounds", ImStopAllSounds },
	{ "ImGetParam", ImGetParam },
	{ "ImSetParam", ImSetParam },
	{ "ImFadeParam", ImFadeParam },
	{ "ImGetSfxVol", ImGetSfxVol },
	{ "ImSetSfxVol", ImSetSfxVol },
	{ "ImGetVoiceVol", ImGetVoiceVol },
	{ "ImSetVoiceVol", ImSetVoiceVol },
	{ "ImGetMusicVol", ImGetMusicVol },
	{ "ImSetMusicVol", ImSetMusicVol },
	{ "ImSetState", ImSetState },
	{ "ImSetSequence", ImSetSequence },
	{ "ImPause", ImPause },
	{ "ImResume", ImResume },
	{ "ImSetVoiceEffect", ImSetVoiceEffect },
	{ "LoadBundle", LoadBundle },
	{ "SetGamma", SetGamma },
	{ "SetActorWalkDominate", SetActorWalkDominate },
	{ "SetActorConstrain", SetActorConstrain },
	{ "RenderModeUser", RenderModeUser },
	{ "ForceRefresh", ForceRefresh },
	{ "DimScreen", DimScreen },
	{ "DimRegion", DimRegion },
	{ "CleanBuffer", CleanBuffer },
	{ "Display", Display },
	{ "SetSpeechMode", SetSpeechMode },
	{ "GetSpeechMode", GetSpeechMode },
	{ "SetShadowColor", SetShadowColor },
	{ "ActivateActorShadow", ActivateActorShadow },
	{ "SetActorShadowPlane", SetActorShadowPlane },
	{ "SetActorShadowPoint", SetActorShadowPoint },
	{ "SetActiveShadow", SetActiveShadow },
	{ "KillActorShadows", KillActorShadows },
	{ "AddShadowPlane", AddShadowPlane },
	{ "SetActorShadowValid", SetActorShadowValid },
	{ "FreeObjectState", FreeObjectState },
	{ "NewObjectState", NewObjectState },
	{ "SetObjectType", SetObjectType },
	{ "SendObjectToBack", SendObjectToBack },
	{ "SendObjectToFront", SendObjectToFront },
	{ "ActorToClean", ActorToClean },
	{ "FlushControls", FlushControls },
	{ "SetActorCollisionMode", SetActorCollisionMode },
	{ "SetActorCollisionScale", SetActorCollisionScale },
	{ "SetActorClipActive", SetActorClipActive },
	{ "SetActorClipPlane", SetActorClipPlane },
	{ "FadeOutChore", FadeOutChore },
	{ "FadeInChore", FadeInChore },
	{ "IrisDown", IrisDown },
	{ "IrisUp", IrisUp },
	{ "TextFileGetLineCount", TextFileGetLineCount },
	{ "TextFileGetLine", TextFileGetLine },
	{ "ScreenShot", ScreenShot },
	{ "GetSaveGameImage", GetSaveGameImage },
	{ "GetImage", GetImage },
	{ "FreeImage", FreeImage },
	{ "BlastImage", BlastImage },
	{ "BlastRect", BlastRect },
	{ "SubmitSaveGameData", SubmitSaveGameData },
	{ "GetSaveGameData", GetSaveGameData },
	{ "SetTextSpeed", SetTextSpeed },
	{ "GetTextSpeed", GetTextSpeed },
	{ "DetachFromResources", DetachFromResources },
	{ "AttachToResources", AttachToResources },
	{ "ActorPuckOrient", ActorPuckOrient },
	{ "JustLoaded", JustLoaded },
	{ "ResetTextures", ResetTextures },
	{ "ShrinkBoxes", ShrinkBoxes },
	{ "UnShrinkBoxes", UnShrinkBoxes },
	{ "GetShrinkPos", GetShrinkPos },
	{ "NukeResources", NukeResources },
	{ "SetActorInvClipNode", SetActorInvClipNode },
	{ "GetDiskFreeSpace", GetDiskFreeSpace },
	{ "SaveIMuse", SaveIMuse },
	{ "RestoreIMuse", RestoreIMuse },
	{ "GetMemoryUsage", GetMemoryUsage },
	{ "dofile", new_dofile },
};

struct luaL_reg textOpcodes[] = {
	{ "IsMessageGoing", IsMessageGoing },
	{ "SetSayLineDefaults", SetSayLineDefaults },
	{ "SetActorTalkColor", SetActorTalkColor },
	{ "GetActorTalkColor", GetActorTalkColor },
	{ "SayLine", SayLine },
	{ "PrintLine", PrintLine },
	{ "MakeTextObject", MakeTextObject },
	{ "GetTextObjectDimensions", GetTextObjectDimensions },
	{ "GetFontDimensions", GetFontDimensions },
	{ "ChangeTextObject", ChangeTextObject },
	{ "KillTextObject", KillTextObject },
	{ "BlastText", BlastText },
	{ "ExpireText", ExpireText },
	{ "PurgeText", PurgeText },
	{ "MakeColor", MakeColor },
	{ "GetColorComponents", GetColorComponents },
	{ "SetTranslationMode", SetTranslationMode },
	{ "GetTranslationMode", GetTranslationMode },
	{ "GetTextCharPosition", GetTextCharPosition },
	{ "LocalizeString", LocalizeString },
	{ "SetEmergencyFont", SetEmergencyFont },
	{ "SetOffscreenTextPos", SetOffscreenTextPos }
};

struct luaL_reg primitivesOpcodes[] = {
	{ "DrawLine", DrawLine },
	{ "DrawPolygon", DrawPolygon },
	{ "DrawRectangle", DrawRectangle },
	{ "ChangePrimitive", ChangePrimitive },
	{ "KillPrimitive", KillPrimitive },
	{ "PurgePrimitiveQueue", PurgePrimitiveQueue }
};

struct luaL_reg hardwareOpcodes[] = {
	{ "Is3DHardwareEnabled", Is3DHardwareEnabled },
	{ "GetVideoDevices", GetVideoDevices },
	{ "SetVideoDevices", SetVideoDevices },
	{ "SetHardwareState", SetHardwareState },
	{ "Enumerate3DDevices", Enumerate3DDevices },
	{ "EnumerateVideoDevices", EnumerateVideoDevices }
};

struct luaL_reg miscOpcodes[] = {
	{ "  concatfallback", concatFallback },
	{ "  typeoverride", typeOverride },
	{ "  dfltcamera", dummyHandler },
	{ "  dfltcontrol", dummyHandler },
};

void register_lua() {
	// Register main opcodes functions
	luaL_openlib(mainOpcodes, ARRAYSIZE(mainOpcodes));

	// Register text opcodes functions
	luaL_openlib(textOpcodes, ARRAYSIZE(textOpcodes));

	// Register primitives opcodes functions
	luaL_openlib(primitivesOpcodes, ARRAYSIZE(primitivesOpcodes));

	// Register hardware opcodes functions
	luaL_openlib(hardwareOpcodes, ARRAYSIZE(hardwareOpcodes));

	// Register miscOpcodes opcodes functions
	luaL_openlib(miscOpcodes, ARRAYSIZE(miscOpcodes));

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
	lua_pushCclosure(typeOverride, 0);
	lua_setglobal("type");

	// Register constants for box types
	lua_pushnumber(0);
	lua_setglobal("NONE");
	lua_pushnumber(0x1000);
	lua_setglobal("WALK");
	lua_pushnumber(0x2000);
	lua_setglobal("CAMERA");
	lua_pushnumber(0x4000);
	lua_setglobal("SPECIAL");
	lua_pushnumber(0x8000);
	lua_setglobal("HOT");

	lua_pushobject(lua_setfallback("concat", concatFallback));
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

int bundle_dofile(const char *filename) {
	Block *b = g_resourceloader->getFileBlock(filename);
	if (!b) {
		delete b;
		// Don't print warnings on Scripts\foo.lua,
		// d:\grimFandango\Scripts\foo.lua
		if (!strstr(filename, "Scripts\\") && (gDebugLevel == DEBUG_WARN || gDebugLevel == DEBUG_ALL))
			warning("Cannot find script %s", filename);

		return 2;
	}

	int result = lua_dobuffer(const_cast<char *>(b->data()), b->len(), const_cast<char *>(filename));
	delete b;
	return result;
}

} // end of namespace Grim
