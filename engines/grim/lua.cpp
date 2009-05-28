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

static int refSystemTable;
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
static int refTextObjvVolume;
static int refTextObjectBackground;

#define strmatch(src, dst)		(strlen(src) == strlen(dst) && strcmp(src, dst) == 0)

static inline bool isObject(int num) {
	lua_Object param = lua_getparam(num);
	if (lua_isuserdata(param) && lua_tag(param) == MKID_BE('STAT'))
		return true;
	return false;
}

static inline bool isActor(int num) {
	lua_Object param = lua_getparam(num);
	if (lua_isuserdata(param) && lua_tag(param) == MKID_BE('ACTR'))
		return true;
	return false;
}

static inline bool isColor(int num) {
	lua_Object param = lua_getparam(num);
	if (lua_isuserdata(param) && lua_tag(param) == MKID_BE('COLR'))
		return true;
	return false;
}

static inline bool isFont(int num) {
	lua_Object param = lua_getparam(num);
	if (lua_isuserdata(param) && lua_tag(param) == MKID_BE('FONT'))
		return true;
	return false;
}

static inline bool isBitmapObject(int num) {
	lua_Object param = lua_getparam(num);
	if (lua_isuserdata(param) && lua_tag(param) == MKID_BE('VBUF'))
		return true;
	return false;
}

// Helper functions to ensure the arguments we get are what we expect
static inline ObjectState *check_object(int num) {
	lua_Object param = lua_getparam(num);
	if (lua_isuserdata(param) && lua_tag(param) == MKID_BE('STAT'))
		return static_cast<ObjectState *>(lua_getuserdata(param));
	luaL_argerror(num, "objectstate expected");
	return NULL;
}

Actor *check_actor(int num) {
	lua_Object param = lua_getparam(num);
	if (lua_isuserdata(param) && lua_tag(param) == MKID_BE('ACTR'))
		return static_cast<Actor *>(lua_getuserdata(param));
	luaL_argerror(num, "actor expected");
	return NULL;
}

Color *check_color(int num) {
	lua_Object param = lua_getparam(num);
	if (lua_isuserdata(param) && lua_tag(param) == MKID_BE('COLR'))
		return static_cast<Color *>(lua_getuserdata(param));
	luaL_argerror(num, "color expected");
	return NULL;
}

static inline Font *check_font(int num) {
	lua_Object param = lua_getparam(num);
	if (lua_isuserdata(param) && lua_tag(param) == MKID_BE('FONT'))
		return static_cast<Font *>(lua_getuserdata(param));
	luaL_argerror(num, "font expected");
	return NULL;
}

static inline PrimitiveObject *check_primobject(int num) {
	lua_Object param = lua_getparam(num);
	if (lua_isuserdata(param) && lua_tag(param) == MKID_BE('PRIM'))
		return static_cast<PrimitiveObject *>(lua_getuserdata(param));
	luaL_argerror(num, "primitive expected");
	return NULL;
}

static inline TextObject *check_textobject(int num) {
	lua_Object param = lua_getparam(num);
	if (lua_isuserdata(param) && lua_tag(param) == MKID_BE('TEXT'))
		return static_cast<TextObject *>(lua_getuserdata(param));
	luaL_argerror(num, "textobject expected");
	return NULL;
}

static inline Bitmap *check_bitmapobject(int num) {
	lua_Object param = lua_getparam(num);
	if (lua_isuserdata(param) && lua_tag(param) == MKID_BE('VBUF'))
		return static_cast<Bitmap *>(lua_getuserdata(param));
	luaL_argerror(num, "image object expected");
	return NULL;
}

static inline bool getbool(int num) {
	return !lua_isnil(lua_getparam(num));
}

static inline void pushbool(bool val) {
	if (val)
		lua_pushnumber(1);
	else
		lua_pushnil();
}

static Costume *get_costume(Actor *a, int param, const char *called_from) {
	Costume *result;
	if (lua_isnil(lua_getparam(param))) {
		result = a->currentCostume();
		if (!result && (gDebugLevel == DEBUG_WARN || gDebugLevel == DEBUG_ALL))
			warning("Actor %s has no costume [%s]", a->name(), called_from);
	} else {
		result = a->findCostume(lua_getstring(lua_getparam(param)));
		if (!result && (gDebugLevel == DEBUG_WARN || gDebugLevel == DEBUG_ALL))
			warning("Actor %s has no costume %s [%s]", a->name(), lua_getstring(lua_getparam(param)), called_from);
	}
	return result;
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
		msg += Common::String(luaL_check_string(1)) + "\n";
		printf(msg.c_str());
	}
}

static void PrintError() {
	if (gDebugLevel == DEBUG_ERROR || gDebugLevel == DEBUG_ALL) {
		Common::String msg("Error: ");
		msg += Common::String(luaL_check_string(1)) + "\n";
		printf(msg.c_str());
	}
}

static void PrintWarning() {
	if (gDebugLevel == DEBUG_WARN || gDebugLevel == DEBUG_ALL) {
		Common::String msg("Warning: ");
		msg += Common::String(luaL_check_string(1)) + "\n";
		printf(msg.c_str());
	}
}

static void FunctionName() {
	const char *name;
	char buf[256];
	const char *filename;
	int32 line;

	if (!lua_isfunction(lua_getparam(1))) {
		sprintf(buf, "function InvalidArgsToFunctionName");
		lua_pushstring(buf);
		return;
	}

	lua_funcinfo(lua_getparam(1), &filename, &line);
	switch (*lua_getobjname(lua_getparam(1), &name)) {
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
	int curr_line = lua_currentline(lua_getparam(1));
	if (curr_line > 0)
		sprintf(buf + strlen(buf), " at line %d", curr_line);
	if (filename)
		sprintf(buf + strlen(buf), " [in file %.100s]", filename);
	lua_pushstring(buf);
}

static void CheckForFile() {
	const char *filename = luaL_check_string(1);

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
	Color *c = new Color (clamp_color(lua_getnumber(lua_getparam(1))),
			clamp_color(lua_getnumber(lua_getparam(2))), clamp_color(lua_getnumber(lua_getparam(3))));
	lua_pushusertag(c, MKID_BE('COLR'));
}

static void GetColorComponents() {
	Color *c = check_color(1);
	lua_pushnumber(c->red());
	lua_pushnumber(c->green());
	lua_pushnumber(c->blue());
}

static void ReadRegistryValue() {
	const char *key = luaL_check_string(1);
	const char *val = g_registry->get(key, "");
	if (val[0] == 0)
		lua_pushnil();
	else
		lua_pushstring(const_cast<char *>(val));
}

static void WriteRegistryValue() {
	const char *key = luaL_check_string(1);
	const char *val = luaL_check_string(2);
	g_registry->set(key, val);
}

// Actor functions

static void LoadActor() {
	const char *name;

	if (lua_isnil(lua_getparam(1)))
		name = "<unnamed>";
	else
		name = luaL_check_string(1);
	lua_pushusertag(new Actor(name), MKID_BE('ACTR'));
}

static void GetActorTimeScale() {
	// return 1 so the game doesn't halt when Manny attempts
	// to pick up the fire extinguisher
	lua_pushnumber(1);
}

static void SetSelectedActor() {
	Actor *act = check_actor(1);
	g_grim->setSelectedActor(act);
}

/* Get the currently selected actor, this is used in
 * "Camera-Relative" mode to handle the appropriate
 * actor for movement
 */
static void GetCameraActor() {
	Actor *act = g_grim->selectedActor();
	lua_pushusertag(act, MKID_BE('ACTR'));
}

static void SetSayLineDefaults() {
	const char *key_text = NULL;
	lua_Object table_obj;
	lua_Object key = LUA_NOOBJECT;

	table_obj = lua_getparam(1);
	for (;;) {
		lua_pushobject(table_obj);
		if (key_text)
			lua_pushobject(key);
		else
			lua_pushnil();

		// If the call to "next" fails then register an error
		if (lua_call("next") != 0) {
			error("SetSayLineDefaults failed to get next key!");
			return;
		}
		key = lua_getresult(1);
		if (lua_isnil(key))
			break;

		key_text = lua_getstring(key);
		if (strmatch(key_text, "font"))
			sayLineDefaults.font = check_font(2);
		else
			error("Unknown SetSayLineDefaults key %s", key_text);
	}
}

static void SetActorTalkColor() {
	Actor *act = check_actor(1);
	Color *c = check_color(2);
	act->setTalkColor(*c);
}

static void GetActorTalkColor() {
	Actor *act = check_actor(1);
	Color *c = new Color(act->talkColor());
	lua_pushusertag(c, MKID_BE('COLR'));
}

static void SetActorRestChore() {
	int chore;
	Costume *costume;

	Actor *act = check_actor(1);
	if (lua_isnil(lua_getparam(2))) {
		chore = -1;
		costume = NULL;
	} else {
		chore = lua_getnumber(lua_getparam(2));
		costume = get_costume(act, 3, "SetActorRestChore");
	}

	act->setRestChore(chore, costume);
}

static void SetActorWalkChore() {
	Actor *act = check_actor(1);
	int chore = lua_getnumber(lua_getparam(2));
	Costume *costume = get_costume(act, 3, "SetActorWalkChore");
	if (!costume) {
		if (gDebugLevel == DEBUG_CHORES || gDebugLevel == DEBUG_WARN || gDebugLevel == DEBUG_ALL)
			warning("SetActorWalkChore() could not find the requested costume, attempting to load...");
		act->pushCostume(lua_getstring(lua_getparam(3)));
		costume = get_costume(act, 3, "SetActorWalkChore");
		if (!costume) {
			if (gDebugLevel == DEBUG_CHORES || gDebugLevel == DEBUG_ERROR || gDebugLevel == DEBUG_ALL)
				error("SetActorWalkChore() could not find the requested costume!");
			return;
		}
	}
	act->setWalkChore(chore, costume);
}

static void SetActorTurnChores() {
	Actor *act = check_actor(1);
	int left_chore = lua_getnumber(lua_getparam(2));
	int right_chore = lua_getnumber(lua_getparam(3));
	Costume *costume = get_costume(act, 4, "SetActorTurnChores");
	act->setTurnChores(left_chore, right_chore, costume);
}

static void SetActorTalkChore() {
	int chore;

	Actor *act = check_actor(1);
	int index = lua_getnumber(lua_getparam(2));
	if (lua_isnumber(lua_getparam(3)))
		chore = lua_getnumber(lua_getparam(3));
	else
		chore = -1;

	Costume *costume = get_costume(act, 4, "setActorTalkChore");

	act->setTalkChore(index, chore, costume);
}

static void SetActorMumblechore() {
	Actor *act = check_actor(1);
	int chore = lua_getnumber(lua_getparam(2));
	Costume *costume = get_costume(act, 3, "SetActorMumblechore");
	act->setMumbleChore(chore, costume);
}

static void SetActorVisibility() {
	Actor *act = check_actor(1);
	bool val = getbool(2);
	act->setVisibility(val);
}

static void PutActorAt() {
	Actor *act = check_actor(1);
	act->setPos(Graphics::Vector3d(luaL_check_number(2), luaL_check_number(3), luaL_check_number(4)));
}

static void GetActorPos() {
	Actor *act = check_actor(1);
	Graphics::Vector3d pos = act->pos();
	// It is important to process this request for all actors,
	// even for actors not within the active scene
	lua_pushnumber(pos.x());
	lua_pushnumber(pos.y());
	lua_pushnumber(pos.z());
}

static void SetActorRot() {
	Actor *act = check_actor(1);
	lua_Object p = lua_getparam(2);
	lua_Object y = lua_getparam(3);
	lua_Object r = lua_getparam(4);
	if (!lua_isnumber(p) || !lua_isnumber(y) || !lua_isnumber(r))
		return;
	float pitch = lua_getnumber(p);
	float yaw = lua_getnumber(y);
	float roll = lua_getnumber(r);
	if (getbool(5))
		act->turnTo(pitch, yaw, roll);
	else
		act->setRot(pitch, yaw, roll);
}

static void GetActorRot() {
	Actor *act = check_actor(1);
	lua_pushnumber(act->pitch());
	lua_pushnumber(act->yaw());
	lua_pushnumber(act->roll());
}

static void IsActorTurning() {
	Actor *act = check_actor(1);
	pushbool(act->isTurning());
}

static void GetAngleBetweenActors() {
	Actor *act1 = check_actor(1);
	Actor *act2 = check_actor(2);
	lua_pushnumber(act1->angleTo(*act2));
}

static void GetActorYawToPoint() {
	Graphics::Vector3d yawVector;

	Actor *act = check_actor(1);
	lua_Object param2 = lua_getparam(2);
	// when this gets called by the tube-switcher guy it's sending
	// only two things: an actor and a table with components x, y, z
	if (lua_isnumber(param2)) {
		yawVector = Graphics::Vector3d(luaL_check_number(2), luaL_check_number(3), luaL_check_number(4));
	} else if (lua_istable(param2)) {
		yawVector = tableToVector(param2);
	} else {
		if (gDebugLevel == DEBUG_ERROR || gDebugLevel == DEBUG_ALL)
			error("Unhandled data type for GetActorYawToPoint!");
		lua_pushnil();
		return;
	}
	lua_pushnumber(act->yawTo(yawVector));
}

/* Changes the set that an actor is associated with,
 * by changing the set to "nil" an actor is disabled
 * but should still not be destroyed.
 */
static void PutActorInSet() {
	const char *set = "";
	Actor *act = check_actor(1);
	// If the set is "nil" then set to an empty string, we should not render
	// objects in the empty set or bad things will happen like the Bone
	// Wagon not changing scenes correctly.
	lua_Object param2 = lua_getparam(2);
	if (!lua_isnil(param2))
		set = luaL_check_string(2);
	// Make sure the actor isn't already in the set
	if (!act->inSet(set))
		act->putInSet(set);
}

static void SetActorWalkRate() {
	Actor *act = check_actor(1);
	float rate = luaL_check_number(2);
	act->setWalkRate(rate);
}

static void GetActorWalkRate() {
	Actor *act = check_actor(1);
	lua_pushnumber(act->walkRate());
}

static void SetActorTurnRate() {
	Actor *act = check_actor(1);
	float rate = luaL_check_number(2);
	act->setTurnRate(rate);
}

static void WalkActorForward() {
	Actor *act = check_actor(1);
	act->walkForward();
}

static void SetActorReflection() {
	Actor *act = check_actor(1);
	float angle = luaL_check_number(2);
	act->setReflection(angle);
}

static void GetActorPuckVector() {
	Actor *act = check_actor(1);
	Graphics::Vector3d result = act->puckVector();
	lua_pushnumber(result.x());
	lua_pushnumber(result.y());
	lua_pushnumber(result.z());
}

static void WalkActorTo() {
	Actor *act = check_actor(1);
	act->walkTo(Graphics::Vector3d(luaL_check_number(2), luaL_check_number(3), luaL_check_number(4)));
}

static void IsActorMoving() {
	Actor *act = check_actor(1);
	pushbool(act->isWalking());
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

	devId = lua_getnumber(lua_getparam(1));
	modeId = lua_getnumber(lua_getparam(2));
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
	int num = lua_getnumber(lua_getparam(1));
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
	Actor *act = check_actor(1);
	pushbool(!(act->isWalking() || act->isTurning()));
}

/* Get the location of one of the actor's nodes, this is
 * used by Glottis to watch where Manny is located in
 * order to hand him the work order.  This function is
 * also important for when Velasco hands Manny the logbook
 * in Rubacava
 */
static void GetActorNodeLocation() {
	Model::HierNode *allNodes;
	Costume *c;
	Actor *act;
	int node;

	act = check_actor(1);
	node = lua_getnumber(lua_getparam(2));
	c = act->currentCostume();
	if (!c) {
		lua_pushnil();
		lua_pushnil();
		lua_pushnil();
		if (gDebugLevel == DEBUG_WARN || gDebugLevel == DEBUG_ALL)
			warning("GetActorNodeLocation() when actor has no costume (which means no nodes)!");
		return;
	}
	allNodes = c->getModelNodes();
	if (!allNodes) {
		lua_pushnil();
		lua_pushnil();
		lua_pushnil();
		if (gDebugLevel == DEBUG_WARN || gDebugLevel == DEBUG_ALL)
			warning("GetActorNodeLocation() when actor has no nodes!");
		return;
	}
	lua_pushnumber(allNodes[node]._pos.x());
	lua_pushnumber(allNodes[node]._pos.y());
	lua_pushnumber(allNodes[node]._pos.z());
}

/* This function is called to stop walking actions that
 * are in progress, it is unknown whether it should start
 * walking actions that are not in progress.
 */
static void SetActorWalkDominate() {
	lua_Object param2;
	Actor *act;

	act = check_actor(1);
	if (!act) {
		lua_pushnil();
		return;
	}
	param2 = lua_getparam(2);
	if (lua_isnil(param2)) {
		lua_pushnil();
	} else if (lua_isnumber(param2)) {
		int walkcode = lua_getnumber(lua_getparam(2));

		if (walkcode != 1) {
			warning("Unknown SetActorWalkDominate 'walking code' value: %d", walkcode);
			lua_pushnil();
			return;
		}
		// When Manny is pursued out of the dam area by a demon
		// beaver he needs to stop his walk chore
		act->stopWalking();
		lua_pushnumber(walkcode);
	} else {
		warning("Unknown SetActorWalkDominate parameter!");
		lua_pushnil();
	}
}

static void SetActorColormap() {
	const char *mapname;
	CMap *_cmap;
	Actor *act;

	act = check_actor(1);
	mapname = luaL_check_string(2);
	_cmap = g_resourceloader->loadColormap(mapname);
	act->setColormap(mapname);
}

static void TurnActor() {
	Actor *act;
	int dir;

	act = check_actor(1);
	dir = lua_getnumber(lua_getparam(2));
	act->turn(dir);
}

static void PushActorCostume() {
	Actor *act;
	const char *costumeName;

	act = check_actor(1);
	costumeName = luaL_check_string(2);
	act->pushCostume(costumeName);
}

static void SetActorCostume() {
	Actor *act;

	act = check_actor(1);
	if (lua_isnil(lua_getparam(2)))
		act->clearCostumes();
	else {
		const char *costumeName = luaL_check_string(2);
		act->setCostume(costumeName);
	}
}

static void GetActorCostume() {
	Actor *act;
	Costume *c;

	act = check_actor(1);
	c = act->currentCostume();
	if (!c) {
		lua_pushnil();
		if (gDebugLevel == DEBUG_NORMAL || gDebugLevel == DEBUG_ALL)
			printf("GetActorCostume() on '%s' when actor has no costume!", act->name());
		return;
	}
	lua_pushstring(const_cast<char *>(c->filename()));
}

static void PopActorCostume() {
	Actor *act;

	act = check_actor(1);
	act->popCostume();
}

static void GetActorCostumeDepth() {
	Actor *act;

	act = check_actor(1);
	lua_pushnumber(act->costumeStackDepth());
}

static void PlayActorChore() {
	Actor *act;
	int num;
	Costume *cost;

	act = check_actor(1);
	num = lua_getnumber(lua_getparam(2));
	cost = get_costume(act, 3, "playActorChore");
	if (!cost) {
		if (gDebugLevel == DEBUG_CHORES || gDebugLevel == DEBUG_WARN || gDebugLevel == DEBUG_ALL)
			warning("Actor costume not found, unable to perform chore.");
		return;
	}

	cost->playChore(num);
}

static void CompleteActorChore() {
	Costume *cost;
	Actor *act;
	int num;
	// CompleteActorChore appears to be an alias for PlayActorChore
	// Except that we should jump to the last frame of the chore
	//
	// Example: When Manny puts the message tube back in his office
	// the animation automatically puts the tube back into place
	// and then calls this function to show the closed graphic
	//
	// Note: This does not appear to function entirely as it should
	// TODO: Make this operation work better

	act = check_actor(1);
	num = lua_getnumber(lua_getparam(2));
	cost = get_costume(act, 3, "completeActorChore");
	if (!cost) {
		if (gDebugLevel == DEBUG_CHORES || gDebugLevel == DEBUG_WARN || gDebugLevel == DEBUG_ALL)
			warning("Actor costume not found, unable to perform chore.");
		return;
	}

	cost->setChoreLastFrame(num);
}

static void PlayActorChoreLooping() {
	Actor *act;
	int num;
	Costume *cost;

	act = check_actor(1);
	num = lua_getnumber(lua_getparam(2));
	cost = get_costume(act, 3, "playActorChoreLooping");
	if (!cost)
		return;

	cost->playChoreLooping(num);
}

static void SetActorChoreLooping() {
	Actor *act;
	int num;
	bool val;
	Costume *cost;

	act = check_actor(1);
	num = lua_getnumber(lua_getparam(2));
	val = getbool(3);
	cost = get_costume(act, 4, "setActorChoreLooping");
	if (!cost)
		return;

	cost->setChoreLooping(num, val);
}

static void StopActorChore() {
	Actor *act;
	Costume *cost;

	act = check_actor(1);
	cost = get_costume(act, 3, "stopActorChore");
	if (!cost)
		return;

	if (lua_isnil(lua_getparam(2)))
		cost->stopChores();
	else
		cost->stopChore(lua_getnumber(lua_getparam(2)));
}

static void IsActorChoring() {
	bool excludeLooping;
	lua_Object param2;
	Costume *cost;
	Actor *act;
	int result = -1;

	act = check_actor(1);
	excludeLooping = getbool(3);
	cost = get_costume(act, 4, "isActorChoring");
	if (!cost) {
		lua_pushnil();
		return;
	}

	// This function can be called with "nil" to get the current
	// chore, a number to check to see if that chore ID is
	// running, or a string that will check to see if the
	// chore of a particular name is running
	param2 = lua_getparam(2);
	if (lua_isnil(param2))
		result = cost->isChoring(excludeLooping);
	else if (lua_isnumber(param2))
		result = cost->isChoring(lua_getnumber(lua_getparam(2)), excludeLooping);
	else if (lua_isstring(param2))
		result = cost->isChoring(lua_getstring(param2), excludeLooping);
	else {
		if (gDebugLevel == DEBUG_WARN || gDebugLevel == DEBUG_ALL)
			warning("IsActorChoring: LUA Parameter 2 is of unhandled type!");
	}

	if (result < 0)
		lua_pushnil();
	else
		lua_pushnumber(result);
}

static void ActorLookAt() {
	lua_Object x, y, z, rate;
	Graphics::Vector3d vector;
	Actor *act;

	act = check_actor(1);
	x = lua_getparam(2);
	y = lua_getparam(3);
	z = lua_getparam(4);
	rate = lua_getparam(5);

	if (lua_isnumber(rate))
		act->setLookAtRate(lua_getnumber(rate));

	// Look at nothing
	if (lua_isnil(x)) {
		if (act->isLookAtVectorZero())
			return;

		act->setLookAtVectorZero();
		act->setLooking(true);
		if (lua_isnumber(y))
			act->setLookAtRate(lua_getnumber(y));
	} else if (lua_isnumber(x)) { // look at xyz
		float fX;
		float fY;
		float fZ;

		if (lua_isnumber(x))
			fX = lua_getnumber(x);
		else
			fX = 0.f;

		if (lua_isnumber(y))
			fY = lua_getnumber(y);
		else
			fY = 0.f;

		if (lua_isnumber(z))
			fZ = lua_getnumber(z);
		else
			fZ = 0.f;

		vector.set(fX, fY, fZ);
		act->setLookAtVector(vector);
	} else if (isActor(x)) { // look at another actor
		Actor *lookedAct = check_actor(x);

		act->setLookAtVector(lookedAct->pos());

		if (lua_isnumber(y))
			act->setLookAtRate(lua_getnumber(y));
	} else {
		if (gDebugLevel == DEBUG_WARN || gDebugLevel == DEBUG_ALL)
			warning("ActorLookAt: Don't know what to look at!");
		return;
	}

	act->setLooking(true);
}

/* Turn the actor to a point specified in the 3D space,
 * this should not have the actor look toward the point
 * but should rotate the entire actor toward it.
 *
 * This function must use a yaw value around the unit
 * circle and not just a difference in angles.
 */
static void TurnActorTo() {
	float x, y, z, yaw;
	Actor *act;

	act = check_actor(1);
	if (lua_isnumber(lua_getparam(2))) {
		x = luaL_check_number(2);
		y = luaL_check_number(3);
		z = luaL_check_number(4);
	} else if (isActor(2)) {
		Actor *destActor;

		destActor = check_actor(2);
		x = destActor->pos().x();
		y = destActor->pos().y();
		z = destActor->pos().z();
	} else {
		if (gDebugLevel == DEBUG_WARN || gDebugLevel == DEBUG_ALL)
			warning("TurnActorTo() parameter type not understood");
		return;
	}

	// Find the vector pointing from the actor to the desired location
	Graphics::Vector3d turnToVector(x, y, z);
	Graphics::Vector3d lookVector = turnToVector - act->pos();
	// find the angle the requested position is around the unit circle
	yaw = lookVector.unitCircleAngle();
	// yaw is offset from forward by 90 degrees
	yaw -= 90.0f;
	act->turnTo(0, yaw, 0);

	// Game will lock in elevator if this doesn't return false
	pushbool(false);
}

/* PointActorAt seems to be an alias for TurnActorTo
 */
static void PointActorAt() {
	TurnActorTo();
}

/* WalkActorVector handles the movement of the selected actor
 * when the game is in "Camera-Relative Movement Mode"
 *
 * NOTE: The usage for paramaters 1 and 5 are as-yet unknown.
 */
static void WalkActorVector() {
	float moveHoriz, moveVert, yaw;
	Actor *act;

	// Second option is the actor returned by GetCameraActor
	act = check_actor(2);
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
	if (act->yaw() != yaw)
		act->turnTo(0, yaw, 0);
	else
		act->walkForward();
}

/* RotateVector takes a vector and rotates it around
 * the point (0,0,0) by the requested number of degrees.
 * This function is used to calculate the locations for
 * getting on and off of the Bone Wagon.
 */
static void RotateVector() {
	lua_Object param1, param2, result;

	param1 = lua_getparam(1);
	param2 = lua_getparam(2);
	if (lua_istable(param1) && lua_istable(param2)) {
		Graphics::Vector3d vec1 = tableToVector(param1);
		lua_Object rotateObject = getTableValue(param2, "y");
		float rotate, currAngle, newAngle;

		// The signpost uses an indexed table (1,2,3) instead of
		// a value-based table (x,y,z)
		if (rotateObject == 0)
			rotateObject = getIndexedTableValue(param2, 2);
		rotate = lua_getnumber(rotateObject);
		Graphics::Vector3d baseVector(sin(0.0f), cos(0.0f), 0);
		currAngle = angle(baseVector, vec1) * (180 / LOCAL_PI);
		newAngle = (currAngle - rotate) * (LOCAL_PI / 180);
		Graphics::Vector3d vec2(sin(newAngle), cos(newAngle), 0);
		vec2 *= vec1.magnitude();

		result = lua_createtable();
		lua_pushobject(result);
		lua_pushstring("x");
		lua_pushnumber(vec2.x());
		lua_settable();
		lua_pushobject(result);
		lua_pushstring("y");
		lua_pushnumber(vec2.y());
		lua_settable();
		lua_pushobject(result);
		lua_pushstring("z");
		lua_pushnumber(vec2.z());
		lua_settable();
	} else {
		if (gDebugLevel == DEBUG_WARN || gDebugLevel == DEBUG_ALL)
			warning("RotateVector() parameter type not understood!");
		// This will likely cause a crash since LUA is expecting
		// a table out of this function
		lua_pushnil();
		return;
	}
	lua_pushobject(result);
}

/* Set the pitch of the actor to the requested value,
 * this will rotate an actor toward/away from the ground.
 * This is used when Glottis runs over the signpost in
 * the Petrified Forest
 */
static void SetActorPitch() {
	lua_Object param2;
	Actor *act;

	act = check_actor(1);
	param2 = lua_getparam(2);
	if (lua_isnumber(param2)) {
		float pitch = lua_getnumber(param2);

		act->setRot(pitch, act->yaw(), act->roll());
	} else {
		if (gDebugLevel == DEBUG_WARN || gDebugLevel == DEBUG_ALL)
			warning("SetActorPitch() parameter type not understood!");
		return;
	}
}

static void SetActorLookRate() {
	Actor *act;
	float rate;

	act = check_actor(1);
	rate = luaL_check_number(2);
	act->setLookAtRate(rate);
}

static void GetActorLookRate() {
	Actor *act;

	act = check_actor(1);
	lua_pushnumber(act->lookAtRate());
}

static void SetActorHead() {
	float maxRoll, maxPitch, maxYaw;
	int joint1, joint2, joint3;
	Actor *act;

	act = check_actor(1);
	joint1 = lua_getnumber(lua_getparam(2));
	joint2 = lua_getnumber(lua_getparam(3));
	joint3 = lua_getnumber(lua_getparam(4));
	maxRoll = luaL_check_number(5);
	maxPitch = luaL_check_number(6);
	maxYaw = luaL_check_number(7);
	act->setHead(joint1, joint2, joint3, maxRoll, maxPitch, maxYaw);
}

static void PutActorAtInterest() {
	Actor *act;

	act = check_actor(1);
	if (!g_grim->currScene())
		return;

	act->setPos(g_grim->currScene()->_currSetup->_interest);
}

static void SetActorFollowBoxes() {
	Actor *act;
	bool mode;

	act = check_actor(1);
	mode = !lua_isnil(lua_getparam(2));
	if (gDebugLevel == DEBUG_WARN || gDebugLevel == DEBUG_ALL)
		warning("SetActorFollowBoxes() not implemented");
	// that is not walkbox walking, but temporary hack
	// act->enableWalkbox(mode);
	act->setConstrain(mode);
}

static void SetActorConstrain() {
//	Actor *act = check_actor(1);
//	bool constrain = !lua_isnil(lua_getparam(2));

	// that below should be enabled, but for now it's disabled realated to
	// above func SetActorFollowBoxes.
//	act->setConstrain(constrain);
}

static void GetVisibleThings() {
	lua_Object result = lua_createtable();
	Actor *sel;

	sel = g_grim->selectedActor();
	for (GrimEngine::ActorListType::const_iterator i = g_grim->actorsBegin(); i != g_grim->actorsEnd(); i++) {
		if (!(*i)->inSet(g_grim->sceneName()))
			continue;
		// Consider the active actor visible
		if (sel == (*i) || sel->angleTo(*(*i)) < 90) {
			lua_pushobject(result);
			lua_pushusertag(*i, MKID_BE('ACTR'));
			lua_pushnumber(1);
			lua_settable();
		}
	}
	lua_pushobject(result);
}

static void SetShadowColor() {
	int r = lua_getnumber(lua_getparam(1));
	int g = lua_getnumber(lua_getparam(2));
	int b = lua_getnumber(lua_getparam(3));

	g_driver->setShadowColor(r, g, b);
}

static void KillActorShadows() {
	Actor *act = check_actor(1);

	act->clearShadowPlanes();
}

static void SetActiveShadow() {
	Actor *act = check_actor(1);
	int shadowId = lua_getnumber(lua_getparam(2));

	act->setActiveShadow(shadowId);
}

static void SetActorShadowPoint() {
	Actor *act = check_actor(1);
	float x = luaL_check_number(2);
	float y = luaL_check_number(3);
	float z = luaL_check_number(4);

	act->setShadowPoint(Graphics::Vector3d(x, y, z));
}

static void SetActorShadowPlane() {
	Actor *act = check_actor(1);
	const char *name = lua_getstring(lua_getparam(2));

	act->setShadowPlane(name);
}

static void AddShadowPlane() {
	Actor *act = check_actor(1);
	const char *name = lua_getstring(lua_getparam(2));

	act->addShadowPlane(name);
}

static void ActivateActorShadow() {
	Actor *act = check_actor(1);
	int shadowId = lua_getnumber(lua_getparam(2));
	bool state = getbool(3);

	act->setActivateShadow(shadowId, state);
	g_grim->flagRefreshShadowMask(true);
}

static void SetActorShadowValid() {
	Actor *act = check_actor(1);
	int valid = lua_getnumber(lua_getparam(2));

	warning("SetActorShadowValid(%d) unknown purpose", valid);

	act->setShadowValid(valid);
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
	char textBuf[512];
	textBuf[0] = 0;
	const char *filename;
	Common::SeekableReadStream *file;

	filename = luaL_check_string(1);
	Common::SaveFileManager *saveFileMan = g_system->getSavefileManager();
	file = saveFileMan->openForLoading(filename);
	if (!file) {
		lua_pushnil();
		return;
	}

	int pos = lua_getnumber(lua_getparam(2));
	file->seek(pos, SEEK_SET);
	file->readLine_NEW(textBuf, 512);
	delete file;

	lua_pushstring(textBuf);
}

static void TextFileGetLineCount() {
	char textBuf[512];
	const char *filename;
	Common::SeekableReadStream *file;

	filename = luaL_check_string(1);
	Common::SaveFileManager *saveFileMan = g_system->getSavefileManager();
	file = saveFileMan->openForLoading(filename);
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
		file->readLine_NEW(textBuf, 512);
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
	char msgId[32], buf[640];
	const char *str, *result;

	str = luaL_check_string(1);
	// If the string that we're passed isn't localized yet then
	// construct the localized string, otherwise spit back what
	// we've been given
	if (str[0] == '/' && str[strlen(str)-1] == '/') {
		Common::String msg = parseMsgText(str, msgId);
		sprintf(buf, "/%s/%s", msgId, msg.c_str());
		result = buf;
	} else {
		result = str;
	}
	lua_pushstring(result);
}

static void SayLine() {
	int /*vol = 64, */paramId = 2;
	char msgId[32];
	Common::String msg;
	lua_Object paramObj;
	Actor *act;

	paramObj = lua_getparam(paramId++);
	act = check_actor(1);
	if (!lua_isnil(paramObj)) {
		do {
			if (lua_isstring(paramObj)) {
				const char *tmpstr = lua_getstring(paramObj);
				msg = parseMsgText(tmpstr, msgId);
			} else if (lua_isnumber(paramObj)) {
				// ignore
			} else if (lua_istable(paramObj)) {
				const char *key_text = NULL;
				lua_Object key = LUA_NOOBJECT;
				for (;;) {
					lua_pushobject(paramObj);
					if (key_text)
						lua_pushobject(key);
					else
						lua_pushnil();
					// If the call to "next" fails then register an error
					if (lua_call("next") != 0) {
						error("SayLine failed to get next key!");
						return;
					}
					key = lua_getresult(1);
					if (lua_isnil(key))
						break;
					warning("SayLine() not null table");
					key_text = lua_getstring(key);
					if (strmatch(key_text, "x"))
						/*int x = */atoi(lua_getstring(lua_getresult(2)));
					else if (strmatch(key_text, "y"))
						/*int y = */atoi(lua_getstring(lua_getresult(2)));
					else if (strmatch(key_text, "background"))
						/*const char *backgorund = */lua_getstring(lua_getresult(2));
					else if (strmatch(key_text, "skip_log"))
						/*int skip_log = */atoi(lua_getstring(lua_getresult(2)));
					else
						error("Unknown SayLine key '%s' = '%s'", key_text, lua_getstring(lua_getresult(2)));
				}
			} else {
					error("SayLine() unknown type of param");
			}
			paramObj = lua_getparam(paramId++);
		} while (!lua_isnil(paramObj));
		if (msg.empty()) {
			if (gDebugLevel == DEBUG_WARN || gDebugLevel == DEBUG_ALL)
				warning("SayLine: Did not find a message ID!");
			return;
		}
		act->sayLine(msg.c_str(), msgId);
	}
}

static void InputDialog() {
	int c, i = 0;
	char buf[512];

	fprintf(stderr, "%s %s: ", luaL_check_string(1), luaL_check_string(2));
	while (i < 512 && (c = fgetc(stdin)) != EOF && c != '\n')
		buf[i++] = c;
	buf[i] = '\0';

	lua_pushstring(buf);
}

static void IsMessageGoing() {
	Actor *act;

	if (lua_getparam(1) == LUA_NOOBJECT) {
		pushbool(g_imuse->isVoicePlaying());
	} else {
		act = check_actor(1);
		pushbool(act->talking());
	}
}

static void ShutUpActor() {
	Actor *act;

	act = check_actor(1);
	if (act)
		act->shutUp();
}

// Sector functions
/* Find the sector (of any type) which contains
 * the requested coordinate (x,y,z).
 */
static void GetPointSector() {
	lua_Object xparam, yparam, zparam;
	Sector *result;
	float x = 0.0f, y = 0.0f, z = 0.0f;

	xparam = lua_getparam(1);
	yparam = lua_getparam(2);
	zparam = lua_getparam(3);
	if (lua_isnumber(xparam) && lua_isnumber(yparam) && lua_isnumber(zparam)) {
		Graphics::Vector3d point(x, y, z);

		// Find the point in any available sector
		result = g_grim->currScene()->findPointSector(point, 0xFFFF);
	} else {
		result = NULL;
	}
	if (!result) {
		if (gDebugLevel == DEBUG_ERROR || gDebugLevel == DEBUG_ALL)
			error("GetPointSector() passed an unhandled type or failed to find any matching sector!");
		lua_pushnil();
		lua_pushnil();
		lua_pushnil();
		return;
	}
	lua_pushnumber(result->id());
	lua_pushstring(const_cast<char *>(result->name()));
	lua_pushnumber(result->type());
}

static void GetActorSector() {
	Actor *act;
	int sectorType;

	act = check_actor(1);
	sectorType = lua_getnumber(lua_getparam(2));
	Sector *result = g_grim->currScene()->findPointSector(act->pos(), sectorType);
	if (result) {
		lua_pushnumber(result->id());
		lua_pushstring(const_cast<char *>(result->name()));
		lua_pushnumber(result->type());
	} else {
		lua_pushnil();
		lua_pushnil();
		lua_pushnil();
	}
}

static void IsActorInSector() {
	int i, numSectors;
	const char *name;
	Actor *act;

	act = check_actor(1);
	name = luaL_check_string(2);
	numSectors = g_grim->currScene()->getSectorCount();
	for (i = 0; i < numSectors; i++) {
		Sector *sector = g_grim->currScene()->getSectorBase(i);
		if (sector->visible() && strmatch(sector->name(), name)) {
			if (sector->isPointInSector(act->pos())) {
				lua_pushnumber(sector->id());
				lua_pushstring((char *)sector->name());
				lua_pushnumber(sector->type());
				return;
			}
		}
	}
	lua_pushnil();
	lua_pushnil();
	lua_pushnil();
}

static void MakeSectorActive() {
	lua_Object sectorName;
	bool visible;
	int i = 0, numSectors;

	
	sectorName = lua_getparam(1);
	visible = !lua_isnil(lua_getparam(2));
	// FIXME: This happens on initial load. Are we initting something in the wrong order?
	if (!g_grim->currScene() && (gDebugLevel == DEBUG_WARN || gDebugLevel == DEBUG_ALL)) {
		warning("!!!! Trying to call MakeSectorActive without a scene!");
		return;
	}

	if (g_grim->currScene())
		numSectors = g_grim->currScene()->getSectorCount();
	else
		numSectors = 0;

	if (lua_isstring(sectorName)) {
		const char *name = luaL_check_string(1);

		for (i = 0; i < numSectors; i++) {
			Sector *sector = g_grim->currScene()->getSectorBase(i);
			if (strmatch(sector->name(), name)) {
				sector->setVisible(visible);
				return;
			}
		}
	} else if (lua_isnumber(sectorName)) {
		int id = lua_getnumber(lua_getparam(1));

		for (i = 0; i < numSectors; i++) {
			Sector *sector = g_grim->currScene()->getSectorBase(i);
			if (sector->id() == id) {
				sector->setVisible(visible);
				return;
			}
		}
	} else if (gDebugLevel == DEBUG_WARN || gDebugLevel == DEBUG_ALL){
		warning("MakeSectorActive Parameter is not a sector ID or Name");
		return;
	} else
		return;
}

// Scene functions
static void LockSet() {
	const char *name;

	name = luaL_check_string(1);
	// We should lock the set so it isn't destroyed
	g_grim->setSceneLock(name, true);
}

static void UnLockSet() {
	const char *name;

	name = luaL_check_string(1);
	// We should unlock the set so it can be destroyed again
	g_grim->setSceneLock(name, false);
}

static void MakeCurrentSet() {
	const char *name;

	name = luaL_check_string(1);
	if (gDebugLevel == DEBUG_NORMAL || gDebugLevel == DEBUG_ALL)
		printf("Entered new scene '%s'.\n", name);
	g_grim->setScene(name);
}

static void MakeCurrentSetup() {
	int num, prevSetup;

	num = lua_getnumber(lua_getparam(1));
	prevSetup = g_grim->currScene()->setup();
	g_grim->currScene()->setSetup(num);

	lua_beginblock();
	lua_Object camChangeHandler = getEventHandler("camChangeHandler");
	if (camChangeHandler != LUA_NOOBJECT) {
		lua_pushnumber(prevSetup);
		lua_pushnumber(num);
		lua_callfunction(camChangeHandler);
	}
	lua_endblock();

	lua_beginblock();
	lua_Object postCamChangeHandler = getEventHandler("postCamChangeHandler");
	if (postCamChangeHandler != LUA_NOOBJECT) {
		lua_pushnumber(num);
		lua_callfunction(postCamChangeHandler);
	}
	lua_endblock();
}

/* Find the requested scene and return the current setup
 * id number.  This function cannot just use the current
 * scene or else when Manny opens his inventory information
 * gets lost, such as the position for the demon beavors
 * in the Petrified Forest.
 */
static void GetCurrentSetup() {
	const char *name;
	Scene *scene;

	name = luaL_check_string(1);
	scene = g_grim->findScene(name);
	if (!scene) {
		if (gDebugLevel == DEBUG_WARN || gDebugLevel == DEBUG_ALL)
			warning("GetCurrentSetup() Requested scene (%s) is not loaded!", name);
		lua_pushnil();
		return;
	}
	lua_pushnumber(scene->setup());
}

// FIXME: Function only spits back what it's given
static void GetShrinkPos() {
	double x, y, z, r;

	x = luaL_check_number(1);
	y = luaL_check_number(2);
	z = luaL_check_number(3);
	r = luaL_check_number(4);
	lua_pushnumber(x);
	lua_pushnumber(y);
	lua_pushnumber(z);
	if (gDebugLevel == DEBUG_WARN || gDebugLevel == DEBUG_ALL)
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
	int priority, group;
	const char *soundName;

	soundName = luaL_check_string(1);
	priority = lua_getnumber(lua_getparam(2));
	group = lua_getnumber(lua_getparam(3));

	// Start the sound with the appropriate settings
	if (g_imuse->startSound(soundName, group, 0, 127, 0, priority, NULL)) {
		lua_pushstring(soundName);
	} else {
		// Allow soft failing when loading sounds, hard failing when not
		if (priority == 127) {
			if (gDebugLevel == DEBUG_IMUSE || gDebugLevel == DEBUG_WARN || gDebugLevel == DEBUG_ALL)
				warning("ImStartSound failed to load '%s'", soundName);
		} else {
			if (gDebugLevel == DEBUG_IMUSE || gDebugLevel == DEBUG_ERROR || gDebugLevel == DEBUG_ALL)
				error("ImStartSound failed to start '%s'", soundName);
		}
		lua_pushnil();
	}
}

static void ImStopSound() {
	const char *soundName;

	soundName = luaL_check_string(1);
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
	g_system->getMixer()->setVolumeForSoundType(Audio::Mixer::kMusicSoundType, lua_getnumber(lua_getparam(1)));
}

static void ImGetMusicVol() {
	lua_pushnumber(g_system->getMixer()->getVolumeForSoundType(Audio::Mixer::kMusicSoundType));
}

static void ImSetVoiceVol() {
	g_system->getMixer()->setVolumeForSoundType(Audio::Mixer::kSpeechSoundType, lua_getnumber(lua_getparam(1)));
}

static void ImGetVoiceVol() {
	lua_pushnumber(g_system->getMixer()->getVolumeForSoundType(Audio::Mixer::kSpeechSoundType));
}

static void ImSetSfxVol() {
	g_system->getMixer()->setVolumeForSoundType(Audio::Mixer::kSFXSoundType, lua_getnumber(lua_getparam(1)));
}

static void ImGetSfxVol() {
	lua_pushnumber(g_system->getMixer()->getVolumeForSoundType(Audio::Mixer::kSFXSoundType));
}

static void ImSetParam() {
	int param, value;
	const char *soundName;

	soundName = luaL_check_string(1);
	param = lua_getnumber(lua_getparam(2));
	value = lua_getnumber(lua_getparam(3));
	switch (param) {
	case IM_SOUND_VOL:
		g_imuse->setVolume(soundName, value);
		break;
	case IM_SOUND_PAN:
		g_imuse->setPan(soundName, value);
		break;
	default:
		lua_pushnil();
		if (gDebugLevel == DEBUG_WARN || gDebugLevel == DEBUG_ALL)
			warning("ImSetParam() Unimplemented %d, %d", param, value);
	}
}

void ImGetParam() {
	const char *soundName;
	int param;

	soundName = luaL_check_string(1);
	param = lua_getnumber(lua_getparam(2));
	switch (param) {
	case IM_SOUND_PLAY_COUNT:
		lua_pushnumber(g_imuse->getCountPlayedTracks(soundName));
		break;
	case IM_SOUND_VOL:
		lua_pushnumber(g_imuse->getVolume(soundName));
		break;
	default:
		lua_pushnil();
		if (gDebugLevel == DEBUG_WARN || gDebugLevel == DEBUG_ALL)
			warning("ImGetParam() Unimplemented %d", param);
	}
}

static void ImFadeParam() {
	int opcode, value, duration;
	const char *soundName;

	soundName = luaL_check_string(1);
	opcode = lua_getnumber(lua_getparam(2));
	value = lua_getnumber(lua_getparam(3));
	duration = lua_getnumber(lua_getparam(4));
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
	g_imuseState = lua_getnumber(lua_getparam(1));
}

static void ImSetSequence() {
	int state;

	state = lua_getnumber(lua_getparam(1));
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

	paramObj = lua_getparam(argId++);
	if (lua_isnumber(paramObj)) {
		minVolume = lua_getnumber(paramObj);
		if (minVolume > 127)
			minVolume = 127;
	}
	paramObj = lua_getparam(argId++);
	if (lua_isnumber(paramObj)) {
		maxVolume = lua_getnumber(paramObj);
		if (maxVolume > 127)
			maxVolume = 127;
		else if (maxVolume < minVolume)
			maxVolume = minVolume;
	}

	paramObj = lua_getparam(argId++);
	if (lua_isnumber(paramObj)) {
		someParam = lua_getnumber(paramObj);
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
	int num = lua_getnumber(numObj);
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
	int num = lua_getnumber(numObj);
	if (num < 0 || num >= KEYCODE_EXTRA_LAST)
		error("control identifier out of range");

	g_grim->disableControl(num);
}

void GetControlState() {
	lua_Object numObj = lua_getparam(1);

	if (!lua_isnumber(numObj))
		return;

	int num = lua_getnumber(numObj);
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
	Bitmap *bitmap = check_bitmapobject(1);
	killBitmapPrimitives(bitmap);
}

static void BlastImage() {
	Bitmap *bitmap = check_bitmapobject(1);
	lua_Object xObj = lua_getparam(2);
	lua_Object yObj = lua_getparam(3);
	if (!lua_isnumber(xObj) || !lua_isnumber(yObj))
		return;

	int x = lua_getnumber(xObj);
	int y = lua_getnumber(yObj);
	bool transparent = getbool(4); // TODO transparent/masked copy into display
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
			textObject->setX(lua_getnumber(keyObj));
		}
	}

	lua_pushobject(tableObj);
	lua_pushobject(lua_getref(refTextObjectY));
	keyObj = lua_gettable();
	if (keyObj) {
		if (lua_isnumber(keyObj)) {
			textObject->setY(lua_getnumber(keyObj));
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
			textObject->setWidth(lua_getnumber(keyObj));
		}
	}

	lua_pushobject(tableObj);
	lua_pushobject(lua_getref(refTextObjectHeight));
	keyObj = lua_gettable();
	if (keyObj) {
		if (lua_isnumber(keyObj)) {
			textObject->setHeight(lua_getnumber(keyObj));
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
			warning("setTextObjectParams: dummy Duration: %d", lua_getnumber(keyObj));
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

	int speed = lua_getnumber(speedObj);
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
		int pos = lua_getnumber(lua_getparam(2));
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

	mode = lua_getnumber(lua_getparam(1));
	if (mode >= 1 && mode <= 3)
		g_grim->setSpeechMode(mode);
}

static void GetSpeechMode() {
	int mode;

	mode = g_grim->getSpeechMode();
	lua_pushnumber(mode);
}

static void StartFullscreenMovie() {
	bool mode = getbool(2);
	warning("StartFullscreenMovie() mode: %d", (int)mode);

	lua_Object name = lua_getparam(1);
	if (!lua_isstring(name)) {
		lua_pushnil();
		return;
	}
	CleanBuffer();
	g_grim->setMode(ENGINE_MODE_SMUSH);
	pushbool(g_smush->play(lua_getstring(name), 0, 0));
}

static void StartMovie() {
	bool mode = getbool(2);
	int x = 0, y = 0;

	warning("StartMovie() mode: %d", (int)mode);

	lua_Object name = lua_getparam(1);
	if (!lua_isstring(name)) {
		lua_pushnil();
		return;
	}
	if (!lua_isnil(lua_getparam(3)))
		x = lua_getnumber(lua_getparam(3));

	if (!lua_isnil(lua_getparam(4)))
		y = lua_getnumber(lua_getparam(4));

	g_grim->setMode(ENGINE_MODE_NORMAL);
	pushbool(g_smush->play(lua_getstring(name), x, y));
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
			layer = lua_getnumber(layerObj);
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

	p1.x = lua_getnumber(x1Obj);
	p1.y = lua_getnumber(y1Obj);
	p2.x = lua_getnumber(x2Obj);
	p2.y = lua_getnumber(y2Obj);

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
			layer = lua_getnumber(layerObj);
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
	if (!lua_isuserdata(param1))
		return;

	lua_Object tableObj = lua_getparam(2);
	if (!lua_istable(tableObj))
		return;

	psearch = check_primobject(1);
	
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
			x = lua_getnumber(xoffset);
		if (lua_isnumber(yoffset))
			y = lua_getnumber(yoffset);
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
			x = lua_getnumber(xobj);
		if (lua_isnumber(yobj))
			y = lua_getnumber(yobj);
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
			x = lua_getnumber(x2);
		if (lua_isnumber(y2))
			y = lua_getnumber(y2);
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
			x = lua_getnumber(width);
		if (lua_isnumber(height))
			y = lua_getnumber(height);
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
	p1.x = lua_getnumber(objX1);
	p1.y = lua_getnumber(objY1);
	p2.x = lua_getnumber(objX2);
	p2.y = lua_getnumber(objY2);
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
	p1.x = lua_getnumber(objX1);
	p1.y = lua_getnumber(objY1);
	p2.x = lua_getnumber(objX2);
	p2.y = lua_getnumber(objY2);
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
	int x = lua_getnumber(lua_getparam(1));
	int y = lua_getnumber(lua_getparam(2));
	int w = lua_getnumber(lua_getparam(3));
	int h = lua_getnumber(lua_getparam(4));
	float level = lua_getnumber(lua_getparam(5));
	g_driver->dimRegion(x, y, w, h, level);
}

static void GetDiskFreeSpace() {
	// amount of free space in MB, used for creating saves
	lua_pushnumber(50);
}

static void NewObjectState() {
	int setupID = lua_getnumber(lua_getparam(1));
	int val = (int)lua_getnumber(lua_getparam(2));
	ObjectState::Position pos = (ObjectState::Position)val;
	const char *bitmap = lua_getstring(lua_getparam(3));
	const char *zbitmap = NULL;
	if (!lua_isnil(lua_getparam(4)))
		zbitmap = lua_getstring(lua_getparam(4));
	bool visible = getbool(5);

	ObjectState *state = new ObjectState(setupID, pos, bitmap, zbitmap, visible);
	g_grim->currScene()->addObjectState(state);
	lua_pushusertag(state, MKID_BE('STAT'));
}

static void FreeObjectState() {
	ObjectState *state = check_object(1);
	g_grim->currScene()->deleteObjectState(state);
}

static void SendObjectToBack() {
	lua_Object param = lua_getparam(1);
	if (lua_isuserdata(param) && lua_tag(param) == MKID_BE('STAT')) {
		ObjectState *state = static_cast<ObjectState *>(lua_getuserdata(param));
		// moving object to top in list ?
		g_grim->currScene()->moveObjectStateToFirst(state);
	}
}

static void SendObjectToFront() {
	lua_Object param = lua_getparam(1);
	if (lua_isuserdata(param) && lua_tag(param) == MKID_BE('STAT')) {
		ObjectState *state = static_cast<ObjectState *>(lua_getuserdata(param));
		// moving object to last in list ?
		g_grim->currScene()->moveObjectStateToLast(state);
	}
}

static void SetObjectType() {
	ObjectState *state = check_object(1);
	int val = (int)lua_getnumber(lua_getparam(2));
	ObjectState::Position pos = (ObjectState::Position)val;
	state->setPos(pos);
}

static void GetCurrentScript() {
	current_script();
}

static void ScreenShot() {
	int width = lua_getnumber(lua_getparam(1));
	int height = lua_getnumber(lua_getparam(2));
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
		warning("Could not restore screenshot from file!");
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
		error("Cannot obtain saved game!");
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
	int mode = lua_getnumber(lua_getparam(1));
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
			sprintf(strPtr, lua_getstring(params[i]));
		else if (lua_tag(params[i]) == MKID_BE('ACTR')) {
			Actor *a = check_actor(params[i]);
			sprintf(strPtr, "(actor%d:%s)", (long)a,
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
STUB_FUNC(LoadCostume)
STUB_FUNC(PrintActorCostumes)
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

// new Monkey 4 opcodes:

STUB_FUNC(SetActiveCD)
STUB_FUNC(GetActiveCD)
STUB_FUNC(AreWeInternational)
STUB_FUNC(MakeScreenTextures)
STUB_FUNC(ThumbnailFromFile)
STUB_FUNC(ClearSpecialtyTexture)
STUB_FUNC(UnloadActor)
STUB_FUNC(PutActorInOverworld)
STUB_FUNC(RemoveActorFromOverworld)
STUB_FUNC(ClearOverworld)
STUB_FUNC(ToggleOverworld)
STUB_FUNC(ActorStopMoving)
STUB_FUNC(SetActorFOV)
STUB_FUNC(SetActorLighting)
STUB_FUNC(SetActorHeadLimits)
STUB_FUNC(ActorActivateShadow)
STUB_FUNC(EnableActorPuck)
STUB_FUNC(SetActorGlobalAlpha)
STUB_FUNC(SetActorLocalAlpha)
STUB_FUNC(SetActorSortOrder)
STUB_FUNC(GetActorSortOrder)
STUB_FUNC(AttachActor)
STUB_FUNC(DetachActor)
STUB_FUNC(IsChoreValid)
STUB_FUNC(IsChorePlaying)
STUB_FUNC(IsChoreLooping)
STUB_FUNC(StopActorChores)
STUB_FUNC(PlayChore)
STUB_FUNC(StopChore)
STUB_FUNC(PauseChore)
STUB_FUNC(AdvanceChore)
STUB_FUNC(CompleteChore)
STUB_FUNC(LockChore)
STUB_FUNC(UnlockChore)
STUB_FUNC(LockChoreSet)
STUB_FUNC(UnlockChoreSet)
STUB_FUNC(LockBackground)
STUB_FUNC(UnLockBackground)
STUB_FUNC(EscapeMovie)
STUB_FUNC(StopAllSounds)
STUB_FUNC(LoadSound)
STUB_FUNC(FreeSound)
STUB_FUNC(PlayLoadedSound)
STUB_FUNC(SetGroupVolume)
STUB_FUNC(GetSoundVolume)
STUB_FUNC(SetSoundVolume)
STUB_FUNC(EnableAudioGroup)
STUB_FUNC(EnableVoiceFX)
STUB_FUNC(PlaySoundFrom)
STUB_FUNC(PlayLoadedSoundFrom)
STUB_FUNC(SetReverb)
STUB_FUNC(UpdateSoundPosition)
STUB_FUNC(ImSelectSet)
STUB_FUNC(ImStateHasLooped)
STUB_FUNC(ImStateHasEnded)
STUB_FUNC(ImPushState)
STUB_FUNC(ImPopState)
STUB_FUNC(ImFlushStack)
STUB_FUNC(ImGetMillisecondPosition)
STUB_FUNC(GetSectorName)
STUB_FUNC(GetCameraYaw)
STUB_FUNC(YawCamera)
STUB_FUNC(GetCameraPitch)
STUB_FUNC(PitchCamera)
STUB_FUNC(RollCamera)
STUB_FUNC(UndimAll)
STUB_FUNC(UndimRegion)
STUB_FUNC(GetCPUSpeed)
STUB_FUNC(NewLayer)
STUB_FUNC(FreeLayer)
STUB_FUNC(SetLayerSortOrder)
STUB_FUNC(SetLayerFrame)
STUB_FUNC(AdvanceLayerFrame)
STUB_FUNC(PushText)
STUB_FUNC(PopText)
STUB_FUNC(NukeAllScriptLocks)
STUB_FUNC(ToggleDebugDraw)
STUB_FUNC(ToggleDrawCameras)
STUB_FUNC(ToggleDrawLights)
STUB_FUNC(ToggleDrawSectors)
STUB_FUNC(ToggleDrawBBoxes)
STUB_FUNC(ToggleDrawFPS)
STUB_FUNC(ToggleDrawPerformance)
STUB_FUNC(ToggleDrawActorStats)
STUB_FUNC(SectEditSelect)
STUB_FUNC(SectEditPlace)
STUB_FUNC(SectEditDelete)
STUB_FUNC(SectEditInsert)
STUB_FUNC(SectEditSortAdd)
STUB_FUNC(SectEditForgetIt)
STUB_FUNC(FRUTEY_Begin)
STUB_FUNC(FRUTEY_End)

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

	// new Monkey 4 opcodes:

	{ "SetActiveCD", SetActiveCD },
	{ "GetActiveCD", GetActiveCD },
	{ "AreWeInternational", AreWeInternational },
	{ "MakeScreenTextures", MakeScreenTextures },
	{ "ThumbnailFromFile", ThumbnailFromFile },
	{ "ClearSpecialtyTexture", ClearSpecialtyTexture },
	{ "UnloadActor", UnloadActor },
	{ "PutActorInOverworld", PutActorInOverworld },
	{ "RemoveActorFromOverworld", RemoveActorFromOverworld },
	{ "ClearOverworld", ClearOverworld },
	{ "ToggleOverworld", ToggleOverworld },
	{ "ActorStopMoving", ActorStopMoving },
	{ "SetActorFOV", SetActorFOV },
	{ "SetActorLighting", SetActorLighting },
	{ "SetActorHeadLimits", SetActorHeadLimits },
	{ "ActorActivateShadow", ActorActivateShadow },
	{ "EnableActorPuck", EnableActorPuck },
	{ "SetActorGlobalAlpha", SetActorGlobalAlpha },
	{ "SetActorLocalAlpha", SetActorLocalAlpha },
	{ "SetActorSortOrder", SetActorSortOrder },
	{ "GetActorSortOrder", GetActorSortOrder },
	{ "AttachActor", AttachActor },
	{ "DetachActor", DetachActor },
	{ "IsChoreValid", IsChoreValid },
	{ "IsChorePlaying", IsChorePlaying },
	{ "IsChoreLooping", IsChoreLooping },
	{ "StopActorChores", StopActorChores },
	{ "PlayChore", PlayChore },
	{ "StopChore", StopChore },
	{ "PauseChore", PauseChore },
	{ "AdvanceChore", AdvanceChore },
	{ "CompleteChore", CompleteChore },
	{ "LockChore", LockChore },
	{ "UnlockChore", UnlockChore },
	{ "LockChoreSet", LockChoreSet },
	{ "UnlockChoreSet", UnlockChoreSet },
	{ "LockBackground", LockBackground },
	{ "UnLockBackground", UnLockBackground },
	{ "EscapeMovie", EscapeMovie },
	{ "StopAllSounds", StopAllSounds },
	{ "LoadSound", LoadSound },
	{ "FreeSound", FreeSound },
	{ "PlayLoadedSound", PlayLoadedSound },
	{ "SetGroupVolume", SetGroupVolume },
	{ "GetSoundVolume", GetSoundVolume },
	{ "SetSoundVolume", SetSoundVolume },
	{ "EnableAudioGroup", EnableAudioGroup },
	{ "EnableVoiceFX", EnableVoiceFX },
	{ "PlaySoundFrom", PlaySoundFrom },
	{ "PlayLoadedSoundFrom", PlayLoadedSoundFrom },
	{ "SetReverb", SetReverb },
	{ "UpdateSoundPosition", UpdateSoundPosition },
	{ "ImSelectSet", ImSelectSet },
	{ "ImStateHasLooped", ImStateHasLooped },
	{ "ImStateHasEnded", ImStateHasEnded },
	{ "ImPushState", ImPushState },
	{ "ImPopState", ImPopState },
	{ "ImFlushStack", ImFlushStack },
	{ "ImGetMillisecondPosition", ImGetMillisecondPosition },
	{ "GetSectorName", GetSectorName },
	{ "GetCameraYaw", GetCameraYaw },
	{ "YawCamera", YawCamera },
	{ "GetCameraPitch", GetCameraPitch },
	{ "PitchCamera", PitchCamera },
	{ "RollCamera", RollCamera },
	{ "UndimAll", UndimAll },
	{ "UndimRegion", UndimRegion },
	{ "GetCPUSpeed", GetCPUSpeed },
	{ "NewLayer", NewLayer },
	{ "FreeLayer", FreeLayer },
	{ "SetLayerSortOrder", SetLayerSortOrder },
	{ "SetLayerFrame", SetLayerFrame },
	{ "AdvanceLayerFrame", AdvanceLayerFrame },
	{ "PushText", PushText },
	{ "PopText", PopText },
	{ "NukeAllScriptLocks", NukeAllScriptLocks },
	{ "ToggleDebugDraw", ToggleDebugDraw },
	{ "ToggleDrawCameras", ToggleDrawCameras },
	{ "ToggleDrawLights", ToggleDrawLights },
	{ "ToggleDrawSectors", ToggleDrawSectors },
	{ "ToggleDrawBBoxes", ToggleDrawBBoxes },
	{ "ToggleDrawFPS", ToggleDrawFPS },
	{ "ToggleDrawPerformance", ToggleDrawPerformance },
	{ "ToggleDrawActorStats", ToggleDrawActorStats },
	{ "SectEditSelect", SectEditSelect },
	{ "SectEditPlace", SectEditPlace },
	{ "SectEditDelete", SectEditDelete },
	{ "SectEditInsert", SectEditInsert },
	{ "SectEditSortAdd", SectEditSortAdd },
	{ "SectEditForgetIt", SectEditForgetIt },
	{ "FRUTEY_Begin", FRUTEY_Begin },
	{ "FRUTEY_End", FRUTEY_End }
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
	refTextObjvVolume = lua_ref(true);
//	lua_pushstring(&text_buffer);
//	refTextObjectBuffer = lua_ref(true);
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

lua_Object getTableFunction(lua_Object table, const char *name) {
	lua_pushobject(table);
	lua_pushstring(const_cast<char *>(name));
	lua_Object handler = lua_gettable();

	if (lua_isnil(handler))
		return LUA_NOOBJECT;

	if (lua_istable(handler)) {
		lua_pushobject(handler);	// Push handler object
		lua_pushobject(handler);	// For gettable
		lua_pushstring(const_cast<char *>(name));
		handler = lua_gettable();
		if (lua_isnil(handler))
			return LUA_NOOBJECT;
	}

	if (!lua_isfunction(handler)) {
		if (gDebugLevel == DEBUG_WARN || gDebugLevel == DEBUG_ALL)
			warning("Invalid event handler %s", name);
		return LUA_NOOBJECT;
	}

	return handler;
}

lua_Object getTableValue(lua_Object table, const char *name) {
	const char *key_text = NULL;
	lua_Object key = LUA_NOOBJECT;

	if (!lua_istable(table)) {
		error("getTableValue(): Parameter not a table!");
		return 0;
	}

	for (;;) {
		lua_pushobject(table);
		if (key_text)
			lua_pushobject(key);
		else
			lua_pushnil();

		// If getTableValue is called against a bad value
		// that it doesn't understand then an infinite loop
		// will be set up repeating the same error.
		if (lua_call("next") != 0) {
			error("getTableValue could not find the next key!");
			return 0;
		}
		key = lua_getresult(1);
		if (lua_isnil(key))
			break;
		// If this function is called against a table that is
		// indexed (rather than keyed) then return zero so
		// the indexed version can be called instead.  This
		// operation cannot be done automatically since the
		// index number needs to be known in order to obtain
		// the correct value.
		if (lua_isnumber(key))
			return 0;

		key_text = lua_getstring(key);
		if (strmatch(key_text, name))
			return lua_getresult(2);
	}

	return 0;
}

lua_Object getIndexedTableValue(lua_Object table, int index) {
	if (!lua_istable(table)) {
		error("getIndexedTableValue(): Parameter not a table!");
		return 0;
	}
	lua_pushobject(table);
	if (index == 1)
		lua_pushnil();
	else
		lua_pushnumber(index - 1);
	// If the call to "next" fails then register an error
	if (lua_call("next") != 0) {
		error("getIndexedTableValue failed to get next key!");
		return 0;
	}
	return lua_getresult(2);
}

void setTableValue(lua_Object table, const char *name, int newvalue) {
	lua_pushobject(table);
	lua_pushstring(name);
	lua_pushnumber(newvalue);
	lua_settable();
}

void setTableValue(lua_Object table, const char *name, lua_Object newvalue) {
	lua_pushobject(table);
	lua_pushstring(name);
	if (newvalue == 0)
		lua_pushnil();
	else
		lua_pushobject(newvalue);
	lua_settable();
}

/* Obtain the x, y, and z coordinates from a LUA table
 * and then create a Vector3d object with these values
 */
Graphics::Vector3d tableToVector(lua_Object table) {
	lua_Object xparam, yparam, zparam;
	float x, y, z;

	if (!lua_istable(table))
		error("tableToVector passed a LUA object that is not a table!");

	xparam = getTableValue(table, "x");
	yparam = getTableValue(table, "y");
	zparam = getTableValue(table, "z");
	if (!lua_isnumber(xparam) || !lua_isnumber(yparam) || !lua_isnumber(zparam))
		error("tableToVector passed a LUA table that does not contain vector coordinates!");

	x = lua_getnumber(xparam);
	y = lua_getnumber(yparam);
	z = lua_getnumber(zparam);
	return Graphics::Vector3d(x, y, z);
}

lua_Object getEventHandler(const char *name) {
	lua_Object system_table = lua_getglobal("system");
	return getTableFunction(system_table, name);
}

} // end of namespace Grim
