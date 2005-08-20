// Residual - Virtual machine to run LucasArts' 3D adventure games
// Copyright (C) 2003-2005 The ScummVM-Residual Team (www.scummvm.org)
//
//  This library is free software; you can redistribute it and/or
//  modify it under the terms of the GNU Lesser General Public
//  License as published by the Free Software Foundation; either
//  version 2.1 of the License, or (at your option) any later version.
//
//  This library is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//  Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public
//  License along with this library; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA

#include "stdafx.h"
#include "debug.h"
#include "lua.h"
#include "resource.h"
#include "actor.h"
#include "registry.h"
#include "localize.h"
#include "color.h"
#include "costume.h"
#include "engine.h"
#include "smush.h"
#include "textobject.h"
#include "objectstate.h"
#include "colormap.h"
#include "font.h"
#include "primitives.h"

#include "imuse/imuse.h"

#include <cstdio>
#include <cmath>
#include <SDL_keysym.h>
#include <SDL_keyboard.h>
#include <zlib.h>

extern Imuse *g_imuse;

#define strmatch(src, dst)     (strlen(src) == strlen(dst) && strcmp(src, dst) == 0)
#define DEBUG_FUNCTION()       debugFunction("Function", __FUNCTION__)

static void debugFunction(char *debugMessage, const char *funcName);
static void stubWarning(char *funcName);

static inline bool isObject(int num) {
	lua_Object param = lua_getparam(num);
	if (lua_isuserdata(param) && lua_tag(param) == MKID('STAT'))
		return true;
	return false;
}

static inline bool isActor(int num) {
	lua_Object param = lua_getparam(num);
	if (lua_isuserdata(param) && lua_tag(param) == MKID('ACTR'))
		return true;
	return false;
}

static inline bool isColor(int num) {
	lua_Object param = lua_getparam(num);
	if (lua_isuserdata(param) && lua_tag(param) == MKID('COLR'))
		return true;
	return false;
}

static inline bool isFont(int num) {
	lua_Object param = lua_getparam(num);
	if (lua_isuserdata(param) && lua_tag(param) == MKID('FONT'))
		return true;
	return false;
}

static inline bool isBitmapObject(int num) {
	lua_Object param = lua_getparam(num);
	if (lua_isuserdata(param) && lua_tag(param) == MKID('VBUF'))
		return true;
	return false;
}

// Helper functions to ensure the arguments we get are what we expect
static inline ObjectState *check_object(int num) {
	lua_Object param = lua_getparam(num);
	if (lua_isuserdata(param) && lua_tag(param) == MKID('STAT'))
		return static_cast<ObjectState *>(lua_getuserdata(param));
	luaL_argerror(num, "objectstate expected");
	return NULL;
}

static inline Actor *check_actor(int num) {
	lua_Object param = lua_getparam(num);
	if (lua_isuserdata(param) && lua_tag(param) == MKID('ACTR'))
		return static_cast<Actor *>(lua_getuserdata(param));
	luaL_argerror(num, "actor expected");
	return NULL;
}

static inline Color *check_color(int num) {
	lua_Object param = lua_getparam(num);
	if (lua_isuserdata(param) && lua_tag(param) == MKID('COLR'))
		return static_cast<Color *>(lua_getuserdata(param));
	luaL_argerror(num, "color expected");
	return NULL;
}

static inline Font *check_font(int num) {
	lua_Object param = lua_getparam(num);
	if (lua_isuserdata(param) && lua_tag(param) == MKID('FONT'))
		return static_cast<Font *>(lua_getuserdata(param));
	luaL_argerror(num, "font expected");
	return NULL;
}

static inline PrimitiveObject *check_primobject(int num) {
	lua_Object param = lua_getparam(num);
	if (lua_isuserdata(param) && lua_tag(param) == MKID('PRIM'))
		return static_cast<PrimitiveObject *>(lua_getuserdata(param));
	luaL_argerror(num, "primitive (rectangle) expected");
	return NULL;
} 

static inline TextObject *check_textobject(int num) {
	lua_Object param = lua_getparam(num);
	if (lua_isuserdata(param) && lua_tag(param) == MKID('TEXT'))
		return static_cast<TextObject *>(lua_getuserdata(param));
	luaL_argerror(num, "textobject expected");
	return NULL;
}

static inline Bitmap *check_bitmapobject(int num) {
	lua_Object param = lua_getparam(num);
	if (lua_isuserdata(param) && lua_tag(param) == MKID('VBUF'))
		return static_cast<Bitmap *>(lua_getuserdata(param));
	luaL_argerror(num, "image object expected");
	return NULL;
}

static inline double check_double(int num) {
	// Have found some instances, such as in Rubacava if you jump there,
	// where doubles of "zero" are called as nil
	if(lua_isnil(lua_getparam(num)))
		return 0.0;
	
	return luaL_check_number(num);
}

static inline int check_int(int num) {
	double val;
	
	// Have found some instances, such as in Rubacava and the tube-switcher
	// room, where integers of "zero" are called as nil
	if(lua_isnil(lua_getparam(num)))
		return 0;
	
	val = luaL_check_number(num);
	return int(round(val));
}

static inline int check_control(int num) {
	int val = check_int(num);
	if (val < 0 || val >= SDLK_EXTRA_LAST)
		luaL_argerror(num, "control identifier out of range");
	return val;
}

static inline ObjectState::Position check_objstate_pos(int num) {
	int val = check_int(num);
	if (val < 1 || val > 3)
		luaL_argerror(num, "object state position out of range");
	return (ObjectState::Position) val;
}

static inline bool getbool(int num) {
	return ! lua_isnil(lua_getparam(num));
}

static inline void pushbool(bool val) {
	if (val)
		lua_pushnumber(1);
	else
		lua_pushnil();
}

static Costume *get_costume(Actor *a, int param, char *called_from) {
	Costume *result;
	if (lua_isnil(lua_getparam(param))) {
		result = a->currentCostume();
		if (result == NULL && (debugLevel == DEBUG_WARN || debugLevel == DEBUG_ALL))
			warning("Actor %s has no costume [%s]\n", a->name(), called_from);
	} else {
		result = a->findCostume(luaL_check_string(param));
		if (result == NULL && (debugLevel == DEBUG_WARN || debugLevel == DEBUG_ALL))
			warning("Actor %s has no costume %s [%s]\n", a->name(), lua_getstring(lua_getparam(param)), called_from);
	}
	return result;
}

// Lua interface to bundle_dofile

static void new_dofile() {
	char *fname_str = luaL_check_string(1);
	bundle_dofile(fname_str);
}

// Debugging message functions

static void PrintDebug() {
	DEBUG_FUNCTION();
	if (debugLevel == DEBUG_NORMAL || debugLevel == DEBUG_ALL) {
		std::string msg = luaL_check_string(1);
		
		msg.insert(0, "Debug: ");
		std::fputs(msg.c_str(), stderr);
		msg.append("\n");
	}
}

static void PrintError() {
	DEBUG_FUNCTION();
	if (debugLevel == DEBUG_ERROR || debugLevel == DEBUG_ALL) {
		std::string msg = luaL_check_string(1);
		
		msg.insert(0, "Error: ");
		// don't do 'error()' so we can stay alive if possible
		std::fputs(msg.c_str(), stderr);
	}
}

static void PrintWarning() {
	DEBUG_FUNCTION();
	if (debugLevel == DEBUG_WARN || debugLevel == DEBUG_ALL) {
		std::string msg = luaL_check_string(1);
		
		msg.insert(0, "Warning: ");
		warning(msg.c_str());
	}
}

static void FunctionName() {
	char *name;
	char buf[256];
	char *filename;
	int line;

	DEBUG_FUNCTION();
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
				sprintf(buf, "function (%.100s:%d)", filename, line);
				filename = NULL;
			}
		}
	}
	int curr_line = lua_currentline(lua_getparam(1));
	if (curr_line > 0)
		sprintf(buf + strlen(buf), " at line %d", curr_line);
	if (filename != NULL)
		sprintf(buf + strlen(buf), " [in file %.100s]", filename);
	lua_pushstring(buf);
}

// File functions

static void CheckForFile() {
	char *filename = luaL_check_string(1);
	
	DEBUG_FUNCTION();
	pushbool(g_resourceloader->fileExists(filename));
}

// Color functions

static unsigned char clamp_color(int c) {
	if (c < 0)
		return 0;
	else if (c > 255)
		return 255;
	else
		return c;
}

static void MakeColor() {
	Color *c;
	
	DEBUG_FUNCTION();
	c = new Color (clamp_color(check_int(1)), clamp_color(check_int(2)), clamp_color(check_int(3)));
	lua_pushusertag(c, MKID('COLR'));
}

static void GetColorComponents() {
	Color *c;
	
	DEBUG_FUNCTION();
	c = check_color(1);
	lua_pushnumber(c->red());
	lua_pushnumber(c->green());
	lua_pushnumber(c->blue());
}

// Registry functions

static void ReadRegistryValue() {
	char *key;
	const char *val;
	
	DEBUG_FUNCTION();
	key = luaL_check_string(1);
	val = g_registry->get(key, NULL);
	lua_pushstring(const_cast<char *>(val));
}

static void WriteRegistryValue() {
	char *key;
	char *val;
	
	DEBUG_FUNCTION();
	key = luaL_check_string(1);
	val = luaL_check_string(2);
	g_registry->set(key, val);
}

// Actor functions

static void LoadActor() {
	const char *name;
	
	DEBUG_FUNCTION();
	if (lua_isnil(lua_getparam(1)))
		name = "<unnamed>";
	else
		name = luaL_check_string(1);
	lua_pushusertag(new Actor(name), MKID('ACTR'));
}

static void GetActorTimeScale() {
	DEBUG_FUNCTION();
	// return 1 so the game doesn't halt when Manny attempts
	// to pick up the fire extinguisher
	lua_pushnumber(1);
}

static void SetSelectedActor() {
	Actor *act;
	
	DEBUG_FUNCTION();
	act = check_actor(1);
	g_engine->setSelectedActor(act);
}

/* Get the currently selected actor, this is used in
 * "Camera-Relative" mode to handle the appropriate
 * actor for movement
 */
static void GetCameraActor() {
	Actor *act;
	
	DEBUG_FUNCTION();
	stubWarning("VERIFY: GetCameraActor");
	act = g_engine->selectedActor();
	lua_pushusertag(act, MKID('ACTR'));
}

static void SetSayLineDefaults() {
	char *key_text = NULL;
	lua_Object table_obj;
	lua_Object key;

	DEBUG_FUNCTION();
	table_obj = lua_getparam(1);
	for (;;) {
		lua_pushobject(table_obj);
		if (key_text)
			lua_pushobject(key);
		else
			lua_pushnil();

		// If the call to "next" fails then register an error
		if (lua_call("next") != 0) {
			error("SetSayLineDefaults failed to get next key!\n");
			return;
		}
		key = lua_getresult(1);
		if (lua_isnil(key)) 
			break;

		key_text = lua_getstring(key);
		if (strmatch(key_text, "font"))
			sayLineDefaults.font = check_font(2);
		else
			error("Unknown SetSayLineDefaults key %s\n", key_text);
	}
}

static void SetActorTalkColor() {
	Actor *act;
	Color *c;
	
	DEBUG_FUNCTION();
	act = check_actor(1);
	c = check_color(2);
	act->setTalkColor(*c);
}

static void GetActorTalkColor() {
	Actor *act;
	Color *c;

	DEBUG_FUNCTION();
	act = check_actor(1);
	c = new Color(act->talkColor());
	lua_pushusertag(c, MKID('COLR'));
}

static void SetActorRestChore() {
	Actor *act;
	int chore;
	Costume *costume;

	DEBUG_FUNCTION();
	act = check_actor(1);
	if (lua_isnil(lua_getparam(2))) {
		chore = -1;
		costume = NULL;
	} else {
		chore = check_int(2);
		costume = get_costume(act, 3, "SetActorRestChore");
	}

	act->setRestChore(chore, costume);
}

static void SetActorWalkChore() {
	Actor *act;
	int chore;
	Costume *costume;

	DEBUG_FUNCTION();
	act = check_actor(1);
	chore = check_int(2);
	costume = get_costume(act, 3, "SetActorWalkChore");
	if (costume == NULL) {
		if (debugLevel == DEBUG_CHORES || debugLevel == DEBUG_WARN || debugLevel == DEBUG_ALL)
			warning("SetActorWalkChore() could not find the requested costume, attempting to load...");
		act->pushCostume(lua_getstring(lua_getparam(3)));
		costume = get_costume(act, 3, "SetActorWalkChore");
		if (costume == NULL) {
			if (debugLevel == DEBUG_CHORES || debugLevel == DEBUG_ERROR || debugLevel == DEBUG_ALL)
				error("SetActorWalkChore() could not find the requested costume!");
			return;
		}
	}
	act->setWalkChore(chore, costume);
}

static void SetActorTurnChores() {
	Actor *act;
	int left_chore;
	int right_chore;
	Costume *costume;

	DEBUG_FUNCTION();
	act = check_actor(1);
	left_chore = check_int(2);
	right_chore = check_int(3);
	costume = get_costume(act, 4, "SetActorTurnChores");
	act->setTurnChores(left_chore, right_chore, costume);
}

static void SetActorTalkChore() {
	Actor *act;
	int index;
	int chore;

	DEBUG_FUNCTION();
	act = check_actor(1);
	index = check_int(2);
	if (lua_isnumber(lua_getparam(3)))
		chore = check_int(3);
	else
		chore = -1;

	Costume *costume = get_costume(act, 4, "setActorTalkChore");

	act->setTalkChore(index, chore, costume);
}

static void SetActorMumblechore() {
	Actor *act;
	int chore;
	Costume *costume;

	DEBUG_FUNCTION();
	act = check_actor(1);
	chore = check_int(2);
	costume = get_costume(act, 3, "SetActorMumblechore");
	act->setMumbleChore(chore, costume);
}

static void SetActorVisibility() {
	Actor *act;
	bool val;
	
	DEBUG_FUNCTION();
	act = check_actor(1);
	val = getbool(2);
	act->setVisibility(val);
}

static void PutActorAt() {
	Actor *act;

	DEBUG_FUNCTION();
	act = check_actor(1);
	act->setPos(Vector3d(luaL_check_number(2), luaL_check_number(3), luaL_check_number(4)));
}

static void GetActorPos() {
	Actor *act;
	Vector3d pos;
	
	DEBUG_FUNCTION();
	act = check_actor(1);
	pos = act->pos();
	// It is important to process this request for all actors,
	// even for actors not within the active scene
	lua_pushnumber(pos.x());
	lua_pushnumber(pos.y());
	lua_pushnumber(pos.z());
}

static void SetActorRot() {
	float pitch, yaw, roll;
	Actor *act;

	DEBUG_FUNCTION();
	act = check_actor(1);
	pitch = check_double(2);
	yaw = check_double(3);
	roll = check_double(4);
	if (getbool(5))
		act->turnTo(pitch, yaw, roll);
	else
		act->setRot(pitch, yaw, roll);
}

static void GetActorRot() {
	Actor *act;

	DEBUG_FUNCTION();
	act = check_actor(1);
	lua_pushnumber(act->pitch());
	lua_pushnumber(act->yaw());
	lua_pushnumber(act->roll());
}

static void IsActorTurning() {
	Actor *act;
	
	DEBUG_FUNCTION();
	act = check_actor(1);
	pushbool(act->isTurning());
}

static void GetAngleBetweenActors() {
	Actor *act1;
	Actor *act2;
	
	DEBUG_FUNCTION();
	act1 = check_actor(1);
	act2 = check_actor(2);
	lua_pushnumber(act1->angleTo(*act2));
}

static void GetActorYawToPoint() {
	Vector3d yawVector;
	lua_Object param2;
	Actor *act;
	
	DEBUG_FUNCTION();
	act = check_actor(1);
	param2 = lua_getparam(2);
	// when this gets called by the tube-switcher guy it's sending
	// only two things: an actor and a table with components x, y, z
	if (lua_isnumber(param2)) {
		yawVector = Vector3d(luaL_check_number(2), luaL_check_number(3), luaL_check_number(4));
	} else if (lua_istable(param2)) {
		yawVector = tableToVector(param2);
	} else {
		if (debugLevel == DEBUG_ERROR || debugLevel == DEBUG_ALL)
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
	Actor *act;
	
	DEBUG_FUNCTION();
	act = check_actor(1);
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
	Actor *act;
	float rate;

	DEBUG_FUNCTION();
	act = check_actor(1);
	rate = luaL_check_number(2);
	act->setWalkRate(rate);
}

static void GetActorWalkRate() {
	Actor *act;

	DEBUG_FUNCTION();
	act = check_actor(1);
	lua_pushnumber(act->walkRate());
}

static void SetActorTurnRate() {
	Actor *act;
	float rate;
	
	DEBUG_FUNCTION();
	act = check_actor(1);
	rate = luaL_check_number(2);
	act->setTurnRate(rate);
}

static void WalkActorForward() {
	Actor *act;
	
	DEBUG_FUNCTION();
	act = check_actor(1);
	act->walkForward();
}

static void SetActorReflection() {
	Actor *act;
	float angle;
	
	DEBUG_FUNCTION();
	act = check_actor(1);
	angle = luaL_check_number(2);
	act->setReflection(angle);
}

static void GetActorPuckVector() {
	Actor *act;
	Vector3d result;

	DEBUG_FUNCTION();
	act = check_actor(1);
	result = act->puckVector();
	lua_pushnumber(result.x());
	lua_pushnumber(result.y());
	lua_pushnumber(result.z());
}

static void WalkActorTo() {
	Actor *act;

	DEBUG_FUNCTION();
	act = check_actor(1);
	act->walkTo(Vector3d(luaL_check_number(2), luaL_check_number(3), luaL_check_number(4)));
}

static void IsActorMoving() {
	Actor *act;

	DEBUG_FUNCTION();
	act = check_actor(1);
	pushbool(act->isWalking());
}

static void Is3DHardwareEnabled() {
	DEBUG_FUNCTION();
	pushbool(g_driver->isHardwareAccelerated());
}

static void SetHardwareState() {
	// changing only in config setup (software/hardware rendering)
	bool accel;

	DEBUG_FUNCTION();
	accel = getbool(1);
	if (accel)
		g_registry->set("soft", "FALSE");
	else
		g_registry->set("soft", "TRUE");
	g_registry->save();
}

static void SetVideoDevices() {
	int devId;
	int modeId;

	DEBUG_FUNCTION();
	devId = check_int(1);
	modeId = check_int(2);
	// ignore setting video devices
}

static void GetVideoDevices() {
	DEBUG_FUNCTION();
	lua_pushnumber(0.0);
	lua_pushnumber(-1.0);
}

static void EnumerateVideoDevices() {
	lua_Object result = lua_createtable();
	
	DEBUG_FUNCTION();
	lua_pushobject(result);
	lua_pushnumber(0.0); // id of device
	lua_pushstring("SDL Video Device"); // name of device
	lua_settable();
	lua_pushobject(result);
}

static void Enumerate3DDevices() {
	lua_Object result = lua_createtable();
	int num;

	DEBUG_FUNCTION();
	num = check_int(1);
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
	Actor *act;

	DEBUG_FUNCTION();
	act = check_actor(1);
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
	
	DEBUG_FUNCTION();
	act = check_actor(1);
	node = check_int(2);
	c = act->currentCostume();
	if (c == NULL) {
		lua_pushnil();
		lua_pushnil();
		lua_pushnil();
		if (debugLevel == DEBUG_WARN || debugLevel == DEBUG_ALL)
			warning("GetActorNodeLocation() when actor has no costume (which means no nodes)!");
		return;
	}
	allNodes = c->getModelNodes();
	if (allNodes == NULL) {
		lua_pushnil();
		lua_pushnil();
		lua_pushnil();
		if (debugLevel == DEBUG_WARN || debugLevel == DEBUG_ALL)
			warning("GetActorNodeLocation() when actor has no nodes!");
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
	
	DEBUG_FUNCTION();
	act = check_actor(1);
	if (act == NULL) {
		lua_pushnil();
		return;
	}
	param2 = lua_getparam(2);
	if (lua_isnil(param2)) {
		lua_pushnil();
	} else if (lua_isnumber(param2)) {
		int walkcode = check_int(2);
		
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
	char *mapname;
	CMap *_cmap;
	Actor *act;
	
	DEBUG_FUNCTION();
	act = check_actor(1);
	mapname = luaL_check_string(2);
	_cmap = g_resourceloader->loadColormap(mapname);
	act->setColormap(mapname);
}

static void TurnActor() {
	Actor *act;
	int dir;
	
	DEBUG_FUNCTION();
	act = check_actor(1);
	dir = check_int(2);
	act->turn(dir);
}

static void PushActorCostume() {
	Actor *act;
	const char *costumeName;

	DEBUG_FUNCTION();
	act = check_actor(1);
	costumeName = luaL_check_string(2);
	act->pushCostume(costumeName);
}

static void SetActorCostume() {
	Actor *act;

	DEBUG_FUNCTION();
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

	DEBUG_FUNCTION();
	act = check_actor(1);
	c = act->currentCostume();
	if (c == NULL) {
		lua_pushnil();
		if (debugLevel == DEBUG_NORMAL || debugLevel == DEBUG_ALL)
			printf("GetActorCostume() on '%s' when actor has no costume!\n", act->name());
		return;
	}
	lua_pushstring(const_cast<char *>(c->filename()));
}

static void PopActorCostume() {
	Actor *act;
	
	DEBUG_FUNCTION();
	act = check_actor(1);
	act->popCostume();
}

static void GetActorCostumeDepth() {
	Actor *act;
	
	DEBUG_FUNCTION();
	act = check_actor(1);
	lua_pushnumber(act->costumeStackDepth());
}

static void PlayActorChore() {
	Actor *act;
	int num;
	Costume *cost;

	DEBUG_FUNCTION();
	act = check_actor(1);
	num = check_int(2);
	cost = get_costume(act, 3, "playActorChore");
	if (!cost) {
		if (debugLevel == DEBUG_CHORES || debugLevel == DEBUG_WARN || debugLevel == DEBUG_ALL)
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
	
	DEBUG_FUNCTION();
	act = check_actor(1);
	num = check_int(2);
	cost = get_costume(act, 3, "completeActorChore");
	if (!cost) {
		if (debugLevel == DEBUG_CHORES || debugLevel == DEBUG_WARN || debugLevel == DEBUG_ALL)
			warning("Actor costume not found, unable to perform chore.");
		return;
	}

	cost->setChoreLastFrame(num);
}

static void PlayActorChoreLooping() {
	Actor *act;
	int num;
	Costume *cost;

	DEBUG_FUNCTION();
	act = check_actor(1);
	num = check_int(2);
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

	DEBUG_FUNCTION();
	act = check_actor(1);
	num = check_int(2);
	val = getbool(3);
	cost = get_costume(act, 4, "setActorChoreLooping");
	if (!cost)
		return;

	cost->setChoreLooping(num, val);
}

static void StopActorChore() {
	Actor *act;
	Costume *cost;

	DEBUG_FUNCTION();
	act = check_actor(1);
	cost = get_costume(act, 3, "stopActorChore");
	if (!cost)
		return;

	if (lua_isnil(lua_getparam(2)))
		cost->stopChores();
	else
		cost->stopChore(check_int(2));
}

static void IsActorChoring() {
	bool excludeLooping;
	lua_Object param2;
	Costume *cost;
	Actor *act;
	int result = -1;

	DEBUG_FUNCTION();
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
		result = cost->isChoring(check_int(2), excludeLooping);
	else if (lua_isstring(param2))
		result = cost->isChoring(lua_getstring(param2), excludeLooping);
	else {
		if (debugLevel == DEBUG_WARN || debugLevel == DEBUG_ALL)
			warning("IsActorChoring: LUA Parameter 2 is of unhandled type!");
	}

	if (result < 0)
		lua_pushnil();
	else
		lua_pushnumber(result);
}

static void ActorLookAt() {
	float rate = 100.0; // Give a default rate
	lua_Object x, y, z;
	bool nullvector = false;
	Vector3d vector;
	Actor *act;

	DEBUG_FUNCTION();
	act = check_actor(1);
	x = lua_getparam(2);
	y = lua_getparam(3);
	z = lua_getparam(4);

	// Look at nothing
	if (lua_isnil(x)) {
		if (act->isLookAtVectorZero()) {
			if (debugLevel == DEBUG_WARN || debugLevel == DEBUG_ALL)
				warning("Actor requested to look at nothing, already looking at nothing!\n");
			return;
		}

		nullvector = true;
		if (lua_isnumber(y))
			rate = luaL_check_number(3);
	} else if ( lua_isnumber(x)) { // look at xyz
		float fX;
		float fY;
		float fZ;

		fX = luaL_check_number(2);

		if (lua_isnumber(y))
			fY = luaL_check_number(3);
		else
			fY = 0.f;

		if (lua_isnumber(z))
			fZ = luaL_check_number(4);
		else
			fZ = 0.f;

		vector.set(fX,fY,fZ);

		if (lua_isnumber(lua_getparam(5)))
			rate = luaL_check_number(5);
	} else if (isActor(2)) { // look at another actor
		Actor *lookedAct = check_actor(2);

		act->setLookAtVector(lookedAct->pos());

		if (lua_isnumber(y))
			rate = luaL_check_number(3);
	} else {
		if (debugLevel == DEBUG_WARN || debugLevel == DEBUG_ALL)
			warning("ActorLookAt: Don't know what to look at!");
		return;
	}

	act->setLookAtRate(rate);
	if (nullvector)
		act->setLookAtVectorZero();
	else
		act->setLookAtVector(vector);
	act->setLooking(true);
	pushbool(act->isTurning());
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

	DEBUG_FUNCTION();
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
		if (debugLevel == DEBUG_WARN || debugLevel == DEBUG_ALL)
			warning("TurnActorTo() parameter type not understood");
		return;
	}
	
	// Find the vector pointing from the actor to the desired location
	Vector3d turnToVector(x, y, z);
	Vector3d lookVector = turnToVector - act->pos();
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
	DEBUG_FUNCTION();
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
	
	DEBUG_FUNCTION();
	stubWarning("VERIFY: WalkActorVector");
	// Second option is the actor returned by GetCameraActor
	act = check_actor(2);
	// Third option is the "left/right" movement
	moveHoriz = luaL_check_number(3);
	// Fourth Option is the "up/down" movement
	moveVert = luaL_check_number(4);

	// Get the direction the camera is pointing
	Vector3d cameraVector = g_engine->currScene()->_currSetup->_interest - g_engine->currScene()->_currSetup->_pos;
	// find the angle the camera direction is around the unit circle
	float cameraYaw = cameraVector.unitCircleAngle();

	// Handle the turning
	Vector3d adjustVector(moveHoriz, moveVert, 0);
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
	
	DEBUG_FUNCTION();
	stubWarning("VERIFY: RotateVector");
	param1 = lua_getparam(1);
	param2 = lua_getparam(2);
	if (lua_istable(param1) && lua_istable(param2)) {
		Vector3d vec1 = tableToVector(param1);
		lua_Object rotateObject = getTableValue(param2, "y");
		float rotate, currAngle, newAngle;
		
		// The signpost uses an indexed table (1,2,3) instead of
		// a value-based table (x,y,z)
		if (rotateObject == 0)
			rotateObject = getIndexedTableValue(param2, 2);
		rotate = lua_getnumber(rotateObject);
		Vector3d baseVector(std::sin(0.0f), std::cos(0.0f), 0);
		currAngle = angle(baseVector, vec1) * (180 / M_PI);
		newAngle = (currAngle - rotate) * (M_PI / 180);
		Vector3d vec2(std::sin(newAngle), std::cos(newAngle), 0);
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
		if (debugLevel == DEBUG_WARN || debugLevel == DEBUG_ALL)
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

	DEBUG_FUNCTION();
	stubWarning("VERIFY: SetActorPitch");
	act = check_actor(1);
	param2 = lua_getparam(2);
	if (lua_isnumber(param2)) {
		float pitch = lua_getnumber(param2);
		
		act->setRot(pitch, act->yaw(), act->roll());
	} else {
		if (debugLevel == DEBUG_WARN || debugLevel == DEBUG_ALL)
			warning("SetActorPitch() parameter type not understood!");
		return;
	}
}

static void SetActorLookRate() {
	Actor *act;
	float rate;

	DEBUG_FUNCTION();
	act = check_actor(1);
	rate = luaL_check_number(2);
	act->setLookAtRate(rate);
}

static void GetActorLookRate() {
	Actor *act;

	DEBUG_FUNCTION();
	act = check_actor(1);
	lua_pushnumber(act->lookAtRate());
}

static void SetActorHead() {
	float maxRoll, maxPitch, maxYaw;
	int joint1, joint2, joint3;
	Actor *act;

	DEBUG_FUNCTION();
	act = check_actor(1);
	joint1 = check_int(2);
	joint2 = check_int(3);
	joint3 = check_int(4);
	maxRoll = luaL_check_number(5); // Yaz: recheck to see if it's really roll
	maxPitch = luaL_check_number(6);
	maxYaw = luaL_check_number(7);
	act->setHead(joint1, joint2, joint3, maxRoll, maxPitch, maxYaw);
}

static void PutActorAtInterest() {
	Actor *act;
	
	DEBUG_FUNCTION();
	act = check_actor(1);
	if (!g_engine->currScene())
		return;

	act->setPos(g_engine->currScene()->_currSetup->_interest);
}

static void SetActorFollowBoxes() {
	Actor *act;
	bool mode;

	DEBUG_FUNCTION();
	act = check_actor(1);
	mode = !lua_isnil(lua_getparam(2));
	if (debugLevel == DEBUG_WARN || debugLevel == DEBUG_ALL)
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
	stubWarning("SetActorConstrain");
}

static void GetVisibleThings() {
	lua_Object result = lua_createtable();
	Actor *sel;

	DEBUG_FUNCTION();
	sel = g_engine->selectedActor();
	for (Engine::ActorListType::const_iterator i = g_engine->actorsBegin(); i != g_engine->actorsEnd(); i++) {
		if (!(*i)->inSet(g_engine->sceneName()))
			continue;
		// Consider the active actor visible
		if (sel == (*i) || sel->angleTo(*(*i)) < 90) {
			lua_pushobject(result);
			lua_pushusertag(*i, MKID('ACTR'));
			lua_pushnumber(1);
			lua_settable();
		}
	}
	lua_pushobject(result);
}

// 0 - translate from '/msgId/'
// 1 - don't translate - message after '/msgId'
// 2 - return '/msgId/'
int translationMode = 0;

std::string parseMsgText(const char *msg, char *msgId) {
	std::string translation = g_localizer->localize(msg);
	const char *secondSlash = NULL;

	if ((msg[0] == '/') && (msgId)) {
		secondSlash = std::strchr(msg + 1, '/');
		if (secondSlash != NULL) {
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
	char *filename;
	FILE *file;
	
	DEBUG_FUNCTION();
	filename = luaL_check_string(1);
	file = fopen(filename, "r");
	if (!file) {
		lua_pushnil();
		return;
	}

	int pos = check_int(2);
	fseek(file, pos, SEEK_SET);
	fgets(textBuf, 512, file);
	fclose(file);

	lua_pushstring(textBuf);
}

static void TextFileGetLineCount() {
	char textBuf[512];
	char *filename;
	FILE *file;
	
	DEBUG_FUNCTION();
	filename = luaL_check_string(1);
	file = fopen(filename, "r");
	if (!file) {
		lua_pushnil();
		return;
	}

	lua_Object result = lua_createtable();

	int line = 0;
	for (;;) {
		if (feof(file))
			break;
		lua_pushobject(result);
		lua_pushnumber(line);
		int pos = ftell(file);
		lua_pushnumber(pos);
		lua_settable();
		fgets(textBuf, 512, file);
		line++;
	}
	fclose(file);

	lua_pushobject(result);
	lua_pushstring("count");
	lua_pushnumber(line);
	lua_settable();
	lua_pushobject(result);
}

// Localization function

static void LocalizeString() {
	char msgId[32], buf[640], *str;
	char *result;
	
	DEBUG_FUNCTION();
	str = luaL_check_string(1);
	// If the string that we're passed isn't localized yet then
	// construct the localized string, otherwise spit back what
	// we've been given
	if (str[0] == '/' && str[strlen(str)-1] == '/') {
		std::string msg = parseMsgText(str, msgId);
		sprintf(buf, "/%s/%s", msgId, msg.c_str());
		result = buf;
	} else {
		result = str;
	}
	lua_pushstring(const_cast<char *>(result));
}

static void SayLine() {
	int pan = 64, param_number = 2;
	char msgId[32], *str = NULL;
	lua_Object param2;
	Actor *act;
	
	DEBUG_FUNCTION();
	act = check_actor(1);
	param2 = lua_getparam(param_number++);
	if (!lua_isnil(param2)) {
		do {
			if (lua_isstring(param2)) {
				char *tmpstr = lua_getstring(param2);
				
				// Fix so Police Chief Bogen's text is interpretted correctly,
				// Bogen has a second text item that's just "1" which previously
				// over-wrote the actual message
				if (tmpstr[0] == '/' && tmpstr[strlen(tmpstr)-1] == '/') {
					parseMsgText(tmpstr, msgId);
					str = tmpstr;
				}
			} else if (lua_isnumber(param2)) {
				pan = 64;
			} else if (lua_istable(param2)) {
			} else {
				error("SayLine() unknown type of param");
			}
			param2 = lua_getparam(param_number++);
		} while (!lua_isnil(param2));
		if (str == NULL) {
			if (debugLevel == DEBUG_WARN || debugLevel == DEBUG_ALL)
				warning("SayLine: Did not find a message ID!");
			stubWarning("ERROR: SayLine");
			return;
		}
		act->sayLine(str, msgId);
	}
}

static void InputDialog() {
	int c, i = 0;
	char buf[512];

	DEBUG_FUNCTION();
	fprintf(stderr, "%s %s: ", luaL_check_string(1), luaL_check_string(2));
	while (i < 512 && (c = fgetc(stdin)) != EOF && c != '\n')
		buf[i++] = c;
	buf[i] = '\0';

	lua_pushstring(buf);
}

static void IsMessageGoing() {
	Actor *act;
	
	DEBUG_FUNCTION();
	if (lua_getparam(1) == LUA_NOOBJECT) {
		pushbool(g_imuse->isVoicePlaying());
	} else {
		act = check_actor(1);
		pushbool(act->talking());
	}
}

static void ShutUpActor() {
	Actor *act;
	
	DEBUG_FUNCTION();
	act = check_actor(1);
	if (act)
		act->shutUp();
}

// Sector functions
/* Find the sector (of any type) which contains
 * the requested coordinate (x,y,z).
 */
static void GetPointSector(void) {
	lua_Object xparam, yparam, zparam;
	Sector *result;
	float x = 0.0f, y = 0.0f, z = 0.0f;
	
	DEBUG_FUNCTION();
	stubWarning("VERIFY: GetPointSector");
	xparam = lua_getparam(1);
	yparam = lua_getparam(2);
	zparam = lua_getparam(3);
	if (lua_isnumber(xparam) && lua_isnumber(yparam) && lua_isnumber(zparam)) {
		Vector3d point(x, y, z);
		
		// Find the point in any available sector
		result = g_engine->currScene()->findPointSector(point, 0xFFFF);
	} else {
		result = NULL;
	}
	if (result == NULL) {
		if (debugLevel == DEBUG_ERROR || debugLevel == DEBUG_ALL)
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

static void GetActorSector(void) {
	Actor *act;
	int sectorType;

	DEBUG_FUNCTION();
	act = check_actor(1);
	sectorType = check_int(2);
	Sector *result = g_engine->currScene()->findPointSector(act->pos(), sectorType);
	if (result != NULL) {
		lua_pushnumber(result->id());
		lua_pushstring(const_cast<char *>(result->name()));
		lua_pushnumber(result->type());
	} else {
		lua_pushnil();
		lua_pushnil();
		lua_pushnil();
	}
}

static void IsActorInSector(void) {
	int i, numSectors;
	const char *name;
	Actor *act;

	DEBUG_FUNCTION();
	act = check_actor(1);
	name = luaL_check_string(2);
	numSectors = g_engine->currScene()->getSectorCount();
	for (i = 0; i < numSectors; i++) {
		Sector *sector = g_engine->currScene()->getSectorBase(i);
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

static void MakeSectorActive(void) {
	lua_Object sectorName;
	bool visible;
	int i = 0, numSectors;

	DEBUG_FUNCTION();
	sectorName = lua_getparam(1);
	visible = !lua_isnil(lua_getparam(2));
	// FIXME: This happens on initial load. Are we initting something in the wrong order?
	if (!g_engine->currScene() && (debugLevel == DEBUG_WARN || debugLevel == DEBUG_ALL)) {
		warning("!!!! Trying to call MakeSectorActive without a scene!");
		return;
	}

	numSectors = g_engine->currScene()->getSectorCount();

	if (lua_isstring(sectorName)) {
		char *name = luaL_check_string(1);

		for (i = 0; i < numSectors; i++) {
			Sector *sector = g_engine->currScene()->getSectorBase(i);
			if (strmatch(sector->name(), name)) {
				sector->setVisible(visible);
				return;
			}
		}
	} else if (lua_isnumber(sectorName)) {
		int id = check_int(1);

		for (i = 0; i < numSectors; i++) {
			Sector *sector = g_engine->currScene()->getSectorBase(i);
			if (sector->id() == id) {
				sector->setVisible(visible);
				return;
			}
		}
	} else if (debugLevel == DEBUG_WARN || debugLevel == DEBUG_ALL){
		warning("MakeSectorActive Parameter is not a sector ID or Name");
		return;
	} else
		return;
}

// Scene functions
static void LockSet() {
	const char *name;
	
	DEBUG_FUNCTION();
	name = luaL_check_string(1);
	// We should lock the set so it isn't destroyed
	g_engine->setSceneLock(name, true);
}

static void UnLockSet() {
	const char *name;
	
	DEBUG_FUNCTION();
	name = luaL_check_string(1);
	// We should unlock the set so it can be destroyed again
	g_engine->setSceneLock(name, false);
}

static void MakeCurrentSet() {
	const char *name;
	
	DEBUG_FUNCTION();
	name = luaL_check_string(1);
	if (debugLevel == DEBUG_NORMAL || debugLevel == DEBUG_ALL)
		printf("Entered new scene '%s'.\n", name);
	g_engine->setScene(name);
}

static void MakeCurrentSetup() {
	int num, prevSetup;

	DEBUG_FUNCTION();
	num = check_int(1);
	prevSetup = g_engine->currScene()->setup();
	g_engine->currScene()->setSetup(num);

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
	
	DEBUG_FUNCTION();
	name = luaL_check_string(1);
	scene = g_engine->findScene(name);
	if (scene == NULL) {
		if (debugLevel == DEBUG_WARN || debugLevel == DEBUG_ALL)
			warning("GetCurrentSetup() Requested scene (%s) is not loaded!", name);
		lua_pushnil();
		return;
	}
	lua_pushnumber(scene->setup());
}

// FIXME: Function only spits back what it's given
static void GetShrinkPos() {
	double x, y, z, r;
	
	DEBUG_FUNCTION();
	x = luaL_check_number(1);
	y = luaL_check_number(2);
	z = luaL_check_number(3);
	r = luaL_check_number(4);
	lua_pushnumber(x);
	lua_pushnumber(y);
	lua_pushnumber(z);
	if (debugLevel == DEBUG_WARN || debugLevel == DEBUG_ALL)
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
	char *soundName;

	DEBUG_FUNCTION();
	soundName = luaL_check_string(1);
	priority = check_int(2);
	group = check_int(3);
	
	// Start the sound with the appropriate settings
	if (g_imuse->startSound(soundName, group, 0, 127, 0, priority)) {
		lua_pushstring(soundName);
	} else {
		// Allow soft failing when loading sounds, hard failing when not
		if (priority == 127) {
			if (debugLevel == DEBUG_IMUSE || debugLevel == DEBUG_WARN || debugLevel == DEBUG_ALL)
				warning("ImStartSound failed to load '%s'", soundName);
		} else {
			if (debugLevel == DEBUG_IMUSE || debugLevel == DEBUG_ERROR || debugLevel == DEBUG_ALL)
				error("ImStartSound failed to start '%s'", soundName);
		}
		lua_pushnil();
	}
}

static void ImStopSound() {
	char *soundName;
	
	DEBUG_FUNCTION();
	soundName = luaL_check_string(1);
	g_imuse->stopSound(soundName);
}

static void ImStopAllSounds() {
	DEBUG_FUNCTION();
	g_imuse->stopAllSounds();
}

static void ImPause() {
	DEBUG_FUNCTION();
	g_imuse->pause(true);
}

static void ImResume() {
	DEBUG_FUNCTION();
	g_imuse->pause(false);
}

static void ImSetVoiceEffect() {
	char *effectName;
	
	DEBUG_FUNCTION();
	effectName = luaL_check_string(1);
	if (debugLevel == DEBUG_WARN || debugLevel == DEBUG_ALL)
		warning("ImSetVoiceEffect(%s) Voice effects are not yet supported", effectName);
}

static void ImSetMusicVol() {
	DEBUG_FUNCTION();
	g_imuse->setGroupMusicVolume(check_int(1));
}

static void ImGetMusicVol() {
	DEBUG_FUNCTION();
	lua_pushnumber(g_imuse->getGroupMusicVolume());
}

static void ImSetVoiceVol() {
	DEBUG_FUNCTION();
	g_imuse->setGroupVoiceVolume(check_int(1));
}

static void ImGetVoiceVol() {
	DEBUG_FUNCTION();
	lua_pushnumber(g_imuse->getGroupVoiceVolume());
}

static void ImSetSfxVol() {
	DEBUG_FUNCTION();
	g_imuse->setGroupSfxVolume(check_int(1));
}

static void ImGetSfxVol() {
	DEBUG_FUNCTION();
	lua_pushnumber(g_imuse->getGroupSfxVolume());
}

static void ImSetParam() {
	int param, value;
	char *soundName;

	DEBUG_FUNCTION();
	soundName = luaL_check_string(1);
	param = check_int(2);
	value = check_int(3);
	switch (param) {
	case IM_SOUND_VOL:
		g_imuse->setVolume(soundName, value);
		break;
	case IM_SOUND_PAN:
		g_imuse->setPan(soundName, value);
		break;
	default:
		lua_pushnil();
		if (debugLevel == DEBUG_WARN || debugLevel == DEBUG_ALL)
			warning("ImSetParam() Unimplemented %d, %d\n", param, value);
	}
}

void ImGetParam() {
	char *soundName;
	int param;

	DEBUG_FUNCTION();
	soundName = luaL_check_string(1);
	param = check_int(2);
	switch (param) {
	case IM_SOUND_PLAY_COUNT:
		lua_pushnumber(g_imuse->getCountPlayedTracks(soundName));
		break;
	case IM_SOUND_VOL:
		lua_pushnumber(g_imuse->getVolume(soundName));
		break;
	default:
		lua_pushnil();
		if (debugLevel == DEBUG_WARN || debugLevel == DEBUG_ALL)
			warning("ImGetParam() Unimplemented %d\n", param);
	}
}

static void ImFadeParam() {
	int opcode, value, duration;
	char *soundName;

	DEBUG_FUNCTION();
	soundName = luaL_check_string(1);
	opcode = check_int(2);
	value = check_int(3);
	duration = check_int(4);
	switch (opcode) {
	case IM_SOUND_PAN:
		g_imuse->setFadePan(soundName, value, duration);
		break;
	default:
		error("ImFadeParam(%s, %h, %d, %d)", soundName, opcode, value, duration);
		break;
	}
}

static void ImSetState() {
	DEBUG_FUNCTION();
	g_imuseState = check_int(1);
}

static void ImSetSequence() {
	int state;
	
	DEBUG_FUNCTION();
	state = check_int(1);
	lua_pushnumber(g_imuse->setMusicSequence(state));
}

static void SaveIMuse() {
	DEBUG_FUNCTION();
	gzFile file = gzopen("grim.tmp", "wb");
	if (file == NULL) {
		warning("SaveIMuse() Error creating temp savegame file");
		return;
	}
	g_engine->_savegameFileHandle = file;
	g_imuse->saveState(g_engine->savegameGzwrite);
	gzclose(file);
}

static void RestoreIMuse() {
	DEBUG_FUNCTION();
	gzFile file = gzopen("grim.tmp", "rb");
	if (file == NULL) {
		return;
	}
	g_engine->_savegameFileHandle = file;
	g_imuse->stopAllSounds();
	g_imuse->resetState();
	g_imuse->restoreState(g_engine->savegameGzread);
	gzclose(file);
	unlink("grim.tmp");
}

static void SetSoundPosition() {
	Vector3d pos;
	int minVolume = 10;
	int maxVolume = 127;
	
	DEBUG_FUNCTION();
	if (g_engine->currScene()) {
		g_engine->currScene()->getSoundParameters(&minVolume, &maxVolume);
	}

	char *soundName = lua_getstring(lua_getresult(1));

	if (isActor(lua_getparam(2))) {
		Actor *act = check_actor(2);
		if (act) {
			pos = act->pos();
		} else {
			return;
		}
		minVolume = check_int(3);
		maxVolume = check_int(4);
	} else if (lua_isnumber(lua_getparam(2))) {
		error("SetSoundPosition() Position x,y,z as params is not suported.");
	} else {
		return;
	}

	if (g_engine->currScene()) {
		g_engine->currScene()->setSoundParameters(minVolume, maxVolume);
		g_engine->currScene()->setSoundPosition(soundName, pos);
	}
}

static void IsSoundPlaying() {
	// dummy
	DEBUG_FUNCTION();
}

static void PlaySoundAt() {
	// dummy
	DEBUG_FUNCTION();
}

static void FileFindDispose() {
	DEBUG_FUNCTION();
	if (g_searchFile) {
#ifdef _WIN32
		FindClose(g_searchFile);
#else
		closedir(g_searchFile);
#endif
		g_searchFile = NULL;
	}
}

static void luaFileFindNext() {
	bool found = false;
#ifndef _WIN32
	dirent *de;
#endif
	
	DEBUG_FUNCTION();
	if (g_searchFile) {
#ifdef _WIN32
		if (g_firstFind) {
			found = true;
			g_firstFind = false;
		}
		while (found && (g_find_file_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
			if (FindNextFile(g_searchFile, &g_find_file_data) == 0)
				found = true;
		};
#else
		do {
			de = readdir(g_searchFile);
			if (de) {
				// If the 'extension' is a wildcard pattern then check only the extensions,
				// otherwise check the entire filename
				if (g_find_file_data[0] == '*' && g_find_file_data[1] == '.') {
					char *c = strrchr(de->d_name, '.');
					
					if (c != NULL && !strcasecmp(c, &g_find_file_data[1])) {
						found = true;
						break;
					}
				} else if (!strcasecmp(de->d_name, g_find_file_data)) {
					found = true;
					break;
				}
			}
		} while (de);
#endif

#ifdef _WIN32
		if (g_find_file_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
#else
		if (!de)
#endif
			found = false;
	}

	if (found) {
#ifdef _WIN32
		lua_pushstring(g_find_file_data.cFileName);
#else
		lua_pushstring(de->d_name);
#endif
	} else {
		lua_pushnil();
	}
}

static void luaFileFindFirst() {
	char *path, *extension;
	lua_Object pathObj;

	DEBUG_FUNCTION();
	extension = luaL_check_string(1);
	pathObj = lua_getparam(2);
	FileFindDispose();

	if (lua_isnil(pathObj))
		path = ".";
	else
		path = lua_getstring(pathObj);
	
#ifdef _WIN32
	std::string dir_strWin32 = path;
	dir_strWin32 += "/";
	dir_strWin32 += extension;
	g_searchFile = FindFirstFile(dir_strWin32.c_str(), &g_find_file_data);
	g_firstFind = true;
	if (g_searchFile == INVALID_HANDLE_VALUE)
		g_searchFile = NULL;
#else
	g_searchFile = opendir(path);
	strcpy(g_find_file_data, extension);
#endif

	if (g_searchFile) {
		luaFileFindNext();
	} else {
		lua_pushnil();
	}
}

void setFrameTime(float frameTime) {
	lua_pushobject(lua_getglobal("system"));
	lua_pushstring("frameTime");
	lua_pushnumber(frameTime);
	lua_settable();
}

void setMovieTime(float movieTime) {
	lua_pushobject(lua_getglobal("system"));
	lua_pushstring("movieTime");
	lua_pushnumber(movieTime);
	lua_settable();
}

void PerSecond() {
	float rate;
	
	DEBUG_FUNCTION();
	rate = luaL_check_number(1);
	lua_pushnumber(g_engine->perSecond(rate));
}

void EnableControl() {
	int num;
	
	DEBUG_FUNCTION();
	num = check_control(1);
	g_engine->enableControl(num);
}

void DisableControl() {
	int num;
	
	DEBUG_FUNCTION();
	num = check_control(1);
	g_engine->disableControl(num);
}

void GetControlState() {
	int num;
	
	DEBUG_FUNCTION();
	num = check_control(1);
	if (num >= SDLK_JOY1_B1 && num <= SDLK_MOUSE_B4)
		lua_pushnil();
	else if (num >= SDLK_AXIS_JOY1_X && num <= SDLK_AXIS_MOUSE_Z)
		lua_pushnumber(0);
	else {
		uint8 *keystate = SDL_GetKeyState(NULL);
		pushbool(keystate[num] != 0);
	}
}

static void GetImage() {
	char *bitmapName;
	
	DEBUG_FUNCTION();
	bitmapName = luaL_check_string(1);
	Bitmap *image = g_resourceloader->loadBitmap(bitmapName);
	lua_pushusertag(image, MKID('VBUF'));
}

static void FreeImage() {
	Bitmap *bitmap;

	DEBUG_FUNCTION();
	bitmap = check_bitmapobject(1);
	for (Engine::PrimitiveListType::const_iterator i = g_engine->primitivesBegin(); i != g_engine->primitivesEnd(); i++) {
		PrimitiveObject *p = *i;
		if (p->isBitmap() && p->getBitmapHandle() == bitmap) {
			g_engine->killPrimitiveObject(p);
			break;
		}
	}

	g_resourceloader->uncache(bitmap->getFilename());
}

static void BlastImage() {
	PrimitiveObject *p = new PrimitiveObject();
	bool transparent;
	Bitmap *bitmap;
	int x, y;
	
	DEBUG_FUNCTION();
	// Allow displaying a null image to fail gracefully.
	// Once main_menu:cancel is handled better this should
	// be unnecessary.
	if (lua_isnil(lua_getparam(1)))
		return;
	bitmap = check_bitmapobject(1);
	x = check_int(2);
	y = check_int(3);
	transparent = getbool(4);
	p->createBitmap(bitmap, x, y, transparent);
	g_engine->registerPrimitiveObject(p);
	lua_pushusertag(p, MKID('PRIM'));
}

void getTextObjectParams(TextObject *textObject, lua_Object table_obj) {
	char *key_text = NULL;
	lua_Object key;
	
	DEBUG_FUNCTION();
	for (;;) {
		lua_pushobject(table_obj);
		if (key_text)
			lua_pushobject(key);
		else
			lua_pushnil();

		// If the call to "next" fails then register an error
		if (lua_call("next") != 0) {
			error("getTextObjectParams failed to get next key!\n");
			return;
		}
		key = lua_getresult(1);
		if (lua_isnil(key)) 
			break;

		// printf("debug param: %s %s\n", lua_getstring(key), lua_getstring(lua_getresult(2)));

		key_text = lua_getstring(key);
		if (strmatch(key_text, "x"))
			textObject->setX(atoi(lua_getstring(lua_getresult(2))));
		else if (strmatch(key_text, "y"))
			textObject->setY(atoi(lua_getstring(lua_getresult(2))));
		else if (strmatch(key_text, "width"))
			textObject->setWidth(atoi(lua_getstring(lua_getresult(2))));
		else if (strmatch(key_text, "height"))
			textObject->setHeight(atoi(lua_getstring(lua_getresult(2))));
		else if (strmatch(key_text, "font"))
			textObject->setFont(check_font(2));
		else if (strmatch(key_text, "fgcolor"))
			textObject->setFGColor(check_color(2));
		else if (strmatch(key_text, "hicolor"))
			warning("getTextObjectParams 'hivolor'");
		else if (strmatch(key_text, "disabled"))
			textObject->setDisabled(atoi(lua_getstring(lua_getresult(2))) != 0);
		else if (strmatch(key_text, "center"))
			textObject->setJustify(1);
		else if (strmatch(key_text, "ljustify"))
			textObject->setJustify(2);
		else if (strmatch(key_text, "rjustify"))
			textObject->setJustify(3);
		else
			error("Unknown getTextObjectParams key '%s'\n", key_text);
	}
}

static void CleanBuffer() {
	DEBUG_FUNCTION();
	g_driver->copyStoredToDisplay();
}
 
/* This function sends the SDL signal to
 * go ahead and exit the game
 */
static void Exit() {
	SDL_Event event;
	
	DEBUG_FUNCTION();
	event.type = SDL_QUIT;
	if (SDL_PushEvent(&event) != 0)
		error("Unable to push exit event!");
}

/* Check for an existing object by a certain name
 * this function is used by several functions that look
 * for text objects to see if they need to be created/modified/destroyed.
 */
TextObject *TextObjectExists(char *name) {
	TextObject *modifyObject = NULL;
	
	DEBUG_FUNCTION();
	for (Engine::TextListType::const_iterator i = g_engine->textsBegin(); i != g_engine->textsEnd(); i++) {
		TextObject *textO = *i;
		if (strlen(name) == strlen(textO->name()) && strcmp(textO->name(), name) == 0) {
			modifyObject = textO;
			break;
		}
	}
	return modifyObject;
}
  
/* Destroy a text object since we don't need it anymore
 * note that the menu creates more objects than it needs,
 * so it deletes some objects right after creating them
 */
static void KillTextObject() {
	TextObject *textObjectParm, *delText;
	
	DEBUG_FUNCTION();
	if (lua_isnil(lua_getparam(1))) {
		if (debugLevel == DEBUG_ERROR || debugLevel == DEBUG_ALL)
			error("KillTextObject(NULL)");
		return;
	}

	textObjectParm = check_textobject(1);

	delText = TextObjectExists((char *) textObjectParm->name());        
	if (delText != NULL)
		g_engine->killTextObject(delText);
}

/* Make changes to a text object based on the parameters passed
 * in the table in the LUA parameter 2.
 */
static void ChangeTextObject() {
	TextObject *modifyObject, *textObject;
	lua_Object tableObj;

	DEBUG_FUNCTION();
	textObject = check_textobject(1);
	// when called in certain instances (such as don's computer)
	// the second parameter is the string and the third is the table
	if (lua_isstring(lua_getparam(2)))
		tableObj = lua_getparam(3);
	else
		tableObj = lua_getparam(2);

	modifyObject = TextObjectExists((char *)textObject->name());
	if (!modifyObject) {
		if (debugLevel == DEBUG_WARN || debugLevel == DEBUG_ALL)
			warning("ChangeTextObject(): Cannot find active text object");
		return;
	}

	if (lua_istable(tableObj))
		getTextObjectParams(modifyObject, tableObj);
	else if (debugLevel == DEBUG_WARN || debugLevel == DEBUG_ALL)
		warning("Expecting table parameter!");

	// to modify current bitmap it need recreate it
	modifyObject->destroyBitmap();
	modifyObject->createBitmap();

	lua_pushnumber(modifyObject->getBitmapWidth());
	lua_pushnumber(modifyObject->getBitmapHeight());
}

/* Return the "text speed", this option must be handled
 * to prevent errors in the "Options" menu even though
 * we're not currently using the value
 */
static void GetTextSpeed() {
	DEBUG_FUNCTION();
	lua_pushnumber(g_engine->getTextSpeed());
}

static void SetTextSpeed() {
	int speed;
	
	DEBUG_FUNCTION();
	speed = check_int(1);
	g_engine->setTextSpeed(speed);
}

/* Make a text object, known to be used by the menu
 * please note that if the same text is issued we will
 * add an additional space (TEXT_NULL) until the text
 * becomes unique
 * (otherwise we'll get identically named menu objects)
 */
static void MakeTextObject() {
	TextObject *textObject = new TextObject();
	lua_Object tableObj;
	char *line;

	DEBUG_FUNCTION();
	line = lua_getstring(lua_getparam(1));
	std::string text = line;
	tableObj = lua_getparam(2);
	textObject->setDefaults(&textObjectDefaults);
         
	if (lua_istable(tableObj))
		getTextObjectParams(textObject, tableObj);
         
	while (TextObjectExists((char *)text.c_str()) != NULL)
		text += TEXT_NULL;

	//printf("Make: %s\n", (char *)text.c_str());

	textObject->setText((char *)text.c_str());
	textObject->createBitmap();
	g_engine->registerTextObject(textObject);
         
	lua_pushusertag(textObject, MKID('TEXT'));
	lua_pushnumber(textObject->getBitmapWidth());
	lua_pushnumber(textObject->getBitmapHeight());
}

static void GetTextObjectDimensions() {
	TextObject *textObjectParam;
	
	DEBUG_FUNCTION();
	textObjectParam = check_textobject(1);
	lua_pushnumber(textObjectParam->getBitmapWidth());
	lua_pushnumber(textObjectParam->getBitmapHeight());
}

static void ExpireText() {
	DEBUG_FUNCTION();
	for (Engine::TextListType::const_iterator i = g_engine->textsBegin(); i != g_engine->textsEnd(); i++) {
		TextObject *textO = *i;
		g_engine->killTextObject(textO);
		delete textO;
	}
	// Cleanup references to deleted text objects
	for (Engine::ActorListType::const_iterator i = g_engine->actorsBegin(); i != g_engine->actorsEnd(); i++)
		(*i)->lineCleanup();
}

static void GetTextCharPosition() {
	TextObject *textObjectParam;
	int pos;
	
	DEBUG_FUNCTION();
	textObjectParam = check_textobject(1);
	pos = (int)lua_getnumber(lua_getparam(2));
	lua_pushnumber((double)textObjectParam->getTextCharPosition(pos));
}

static void BlastText() {
	DEBUG_FUNCTION();
	// there is some diffrence to MakeTextObject
	// it draw directly to gfx buffer from here, not from main loop
	MakeTextObject();
}

static void SetOffscreenTextPos() {
	// this sets where we shouldn't put dialog maybe?
	stubWarning("SetOffscreenTextPos");
}

static void SetSpeechMode() {
	int mode;
	
	DEBUG_FUNCTION();
	mode = check_int(1);
	if ((mode >= 1) && (mode <= 3))
 		g_engine->setSpeechMode(mode);
}

static void GetSpeechMode() {
	int mode;
	
	DEBUG_FUNCTION();
	mode = g_engine->getSpeechMode();
 	lua_pushnumber(mode);
}

static void StartFullscreenMovie() {
	/*bool mode = getbool(2);*/
	DEBUG_FUNCTION();
	// Clean out any text objects on the display before running the
	// movie, otherwise things like Bruno's "Nice bathrobe." will stay
	// on-screen the whole movie
	CleanBuffer();
	g_engine->setMode(ENGINE_MODE_SMUSH);
	pushbool(g_smush->play(luaL_check_string(1), 0, 0));
}

static void StartMovie() {
	/*bool mode = getbool(2);*/
	int x = 0, y = 0;

	DEBUG_FUNCTION();
	if (!lua_isnil(lua_getparam(3)))
		x = check_int(3);

	if (!lua_isnil(lua_getparam(4)))
		y = check_int(4);

	g_engine->setMode(ENGINE_MODE_NORMAL);
	pushbool(g_smush->play(luaL_check_string(1), x, y));
}

/* Fullscreen movie playing query and normal movie
 * query should actually detect correctly and not
 * just return true whenever ANY movie is playing
 */
static void IsFullscreenMoviePlaying() {
	DEBUG_FUNCTION();
	pushbool(g_smush->isPlaying() && g_engine->getMode() == ENGINE_MODE_SMUSH);
}

static void IsMoviePlaying() {
	DEBUG_FUNCTION();
	pushbool(g_smush->isPlaying() && g_engine->getMode() == ENGINE_MODE_NORMAL);
}

static void StopMovie() {
	DEBUG_FUNCTION();
	g_smush->stop();
}

static void PauseMovie() {
	DEBUG_FUNCTION();
	g_smush->pause(lua_isnil(lua_getparam(1)) != 0);
}

static void PurgePrimitiveQueue() {
	DEBUG_FUNCTION();
	g_engine->killPrimitiveObjects();
}

static void DrawPolygon() {
	stubWarning("DrawPolygon");
}

static void DrawLine() {
	int x1, y1, x2, y2;
	lua_Object tableObj;
	Color color;
	
	DEBUG_FUNCTION();
	x1 = check_int(1);
	y1 = check_int(2);
	x2 = check_int(3);
	y2 = check_int(4);
	tableObj = lua_getparam(5);
	color._vals[0] = 255;
	color._vals[1] = 255;
	color._vals[2] = 255;

	if (lua_istable(tableObj)){
		lua_pushobject(tableObj);
		lua_pushstring("color");
		lua_Object colorObj = lua_gettable();
		if (lua_isuserdata(colorObj) && lua_tag(colorObj) == MKID('COLR')) {
			color = static_cast<Color *>(lua_getuserdata(colorObj));
		}
	}

	PrimitiveObject *p = new PrimitiveObject();
	p->createLine(x1, x2, y1, y2, color);
	g_engine->registerPrimitiveObject(p);
	lua_pushusertag(p, MKID('PRIM'));
}

static void ChangePrimitive() {
	PrimitiveObject *psearch, *pmodify = NULL;
	lua_Object tableObj;
	Color color;

	DEBUG_FUNCTION();
	tableObj = lua_getparam(2);
	color._vals[0] = 255;
	color._vals[1] = 255;
	color._vals[2] = 255;

	if (lua_isnil(lua_getparam(1)) || !lua_istable(tableObj))
		return;

	psearch = check_primobject(1);
	for (Engine::PrimitiveListType::const_iterator i = g_engine->primitivesBegin(); i != g_engine->primitivesEnd(); i++) {
		PrimitiveObject *p = *i;
		if (p->getX1() == psearch->getX1() && p->getX2() == psearch->getX2() && p->getY1() == psearch->getY1() && p->getY2() == psearch->getY2()) {
			pmodify = p;
			break;
		}
	}
	if(!pmodify)
		error("Primitive object not found.");

	lua_pushobject(tableObj);
	lua_pushstring("color");
	lua_Object colorObj = lua_gettable();
	if (lua_isuserdata(colorObj) && lua_tag(colorObj) == MKID('COLR')) {
		color = static_cast<Color *>(lua_getuserdata(colorObj));
		pmodify->setColor(color);
	}
	lua_pushobject(tableObj);
	lua_pushstring("y");
	lua_Object yObj = lua_gettable();
	if (!lua_isnil(yObj)) {
		pmodify->setY1(atoi(lua_getstring(yObj)));
		pmodify->setY2(atoi(lua_getstring(yObj)));
	}
}

static void DrawRectangle() {
	int x1, y1, x2, y2;
	lua_Object tableObj;
	Color color;
	
	DEBUG_FUNCTION();
	x1 = check_int(1);
	y1 = check_int(2);
	x2 = check_int(3);
	y2 = check_int(4);
	tableObj = lua_getparam(5);
	color._vals[0] = 255;
	color._vals[1] = 255;
	color._vals[2] = 255;
	bool filled = false;

	if (lua_istable(tableObj)){
		lua_pushobject(tableObj);
		lua_pushstring("color");
		lua_Object colorObj = lua_gettable();
		if (lua_isuserdata(colorObj) && lua_tag(colorObj) == MKID('COLR')) {
			color = static_cast<Color *>(lua_getuserdata(colorObj));
		}

		lua_pushobject(tableObj);
		lua_pushstring("filled");
		lua_Object objFilled = lua_gettable();
		if (!lua_isnil(objFilled))
			filled = true;
	}

	PrimitiveObject *p = new PrimitiveObject();
	p->createRectangle(x1, x2, y1, y2, color, filled);
	g_engine->registerPrimitiveObject(p);
	lua_pushusertag(p, MKID('PRIM'));
}

static void BlastRect() {
	DEBUG_FUNCTION();
	// BlastRect is specifically for the menu thread;
	// however, we don't need to handle the menu in a 
	// separate thread so this works fine
	DrawRectangle();
}

static void DimScreen() {
	DEBUG_FUNCTION();
	if (debugLevel == DEBUG_WARN || debugLevel == DEBUG_ALL)
		warning("DimRegion()");
}

static void DimRegion() {
	DEBUG_FUNCTION();
	if (debugLevel == DEBUG_WARN || debugLevel == DEBUG_ALL)
		warning("DimRegion()");
}

static void GetDiskFreeSpace() {
	DEBUG_FUNCTION();
	// amount of free space in MB, used for creating saves
	lua_pushnumber(50);
}

// Objectstate functions
static void NewObjectState() {
	ObjectState *state = NULL;
	ObjectState::Position pos;
	char *bitmap, *zbitmap;
	bool visible;
	int setupID;

	DEBUG_FUNCTION();
	// Called with "nil" if you jump to zone "sp"
	setupID = check_int(1);					// Setup ID
	pos = check_objstate_pos(2); 		// When to draw
	bitmap = luaL_check_string(3);	// Bitmap
	zbitmap = NULL;									// Zbuffer Bitmap
	visible = getbool(5);						// Starts visible?
	if (!lua_isnil(lua_getparam(4)))
		zbitmap = luaL_check_string(4);

	state = new ObjectState(setupID, pos, bitmap, zbitmap, visible);
	g_engine->currScene()->addObjectState(state);
	lua_pushusertag(state, MKID('STAT'));
}

static void FreeObjectState() {
	ObjectState *state;
	
	DEBUG_FUNCTION();
	state = check_object(1);
	g_engine->currScene()->deleteObjectState(state);
}

static void SendObjectToBack() {
	lua_Object param;
	
	DEBUG_FUNCTION();
	param = lua_getparam(1);
	if (lua_isuserdata(param) && lua_tag(param) == MKID('STAT')) {
		ObjectState *state = static_cast<ObjectState *>(lua_getuserdata(param));
		// moving object to top in list ?
		g_engine->currScene()->moveObjectStateToFirst(state);
	}
}

static void SendObjectToFront() {
	lua_Object param;
	
	DEBUG_FUNCTION();
	param = lua_getparam(1);
	if (lua_isuserdata(param) && lua_tag(param) == MKID('STAT')) {
		ObjectState *state = static_cast<ObjectState *>(lua_getuserdata(param));
		// moving object to last in list ?
		g_engine->currScene()->moveObjectStateToLast(state);
	}
}

static void SetObjectType() {
	ObjectState::Position pos;
	ObjectState *state;
	
	DEBUG_FUNCTION();
	state = check_object(1);
	pos = check_objstate_pos(2);
	state->setPos(pos);
}

static void GetCurrentScript() {
	DEBUG_FUNCTION();
	current_script();
}

static void ScreenShot() {
	int width, height;
	
	DEBUG_FUNCTION();
	width = check_int(1);
	height = check_int(2);
	Bitmap *screenshot = g_driver->getScreenshot(width, height);
	if (screenshot) {
		lua_pushusertag(screenshot, MKID('VBUF'));
	} else {
		lua_pushnil();
	}
}

static void SubmitSaveGameData() {
	lua_Object table, table2;
	int dataSize = 0;
	int count = 0;
	char *str;
	
	DEBUG_FUNCTION();
	table = lua_getparam(1);
	for (;;) {
		lua_pushobject(table);
		lua_pushnumber(count);
		count++;
		table2 = lua_gettable();
		if (lua_isnil(table2))
			break;
		str = lua_getstring(table2);
		dataSize += strlen(str) + 1;
		dataSize += 4;
	}
	if (dataSize == 0)
		return;

	g_engine->savegameGzwrite(&dataSize, sizeof(int));
	count = 0;
	for (;;) {
		lua_pushobject(table);
		lua_pushnumber(count);
		count++;
		table2 = lua_gettable();
		if (lua_isnil(table2))
			break;
		str = lua_getstring(table2);
		int len = strlen(str) + 1;
		g_engine->savegameGzwrite(&len, sizeof(int));
		g_engine->savegameGzwrite(str, len);
	}
}

static void GetSaveGameData() {
	lua_Object result;
	char *filename;
	int dataSize;
	gzFile file;
	
	DEBUG_FUNCTION();
	filename = luaL_check_string(1);
	file = gzopen(filename, "rb");
	if (!file)
		return;

	result = lua_createtable();
	gzread(file, &dataSize, sizeof(int));

	char str[128];
	int strSize;
	int count = 0;

	// Get the name of the saved game
	lua_pushobject(result);
	lua_pushnumber(count++);
	lua_pushstring(filename); // TODO: Use an actual stored name
	lua_settable();
	
	for (;;) {
		if (dataSize <= 0)
			break;
		gzread(file, &strSize, sizeof(int));
		gzread(file, str, strSize);
		lua_pushobject(result);
		lua_pushnumber(count);
		lua_pushstring(str);
		lua_settable();
		dataSize -= strSize;
		dataSize -= 4;
		count++;
	}

	lua_pushobject(result);

	gzclose(file);
}

static void Load() {
	lua_Object fileName;
	
	DEBUG_FUNCTION();
	fileName = lua_getparam(1);
	if (lua_isnil(fileName)) {
		g_engine->_savegameFileName = NULL;
	} else if (lua_isstring(fileName)) {
		g_engine->_savegameFileName = lua_getstring(fileName);
	} else {
		if (debugLevel == DEBUG_WARN || debugLevel == DEBUG_ALL)
			warning("Load() fileName is wrong");
		return;
	}
	g_engine->_savegameLoadRequest = true;
}

static void Save() {
	lua_Object fileName;
	
	DEBUG_FUNCTION();
	fileName = lua_getparam(1);
	if (lua_isnil(fileName)) {
		g_engine->_savegameFileName = NULL;
	} else if (lua_isstring(fileName)) {
		g_engine->_savegameFileName = lua_getstring(fileName);
	} else {
		if (debugLevel == DEBUG_WARN || debugLevel == DEBUG_ALL)
			warning("Save() fileName is wrong");
		return;
	}
	g_engine->_savegameSaveRequest = true;
}

static int SaveCallback(int /*tag*/, int value, SaveRestoreFunc /*saveFunc*/) {
	DEBUG_FUNCTION();
	return value;
}

static int RestoreCallback(int /*tag*/, int value, SaveRestoreFunc /*saveFunc*/) {
	DEBUG_FUNCTION();
	return value;
}

static void LockFont() {
	lua_Object param1;
	
	DEBUG_FUNCTION();
	param1 = lua_getparam(1);
	if (lua_isstring(param1)) {
		char *fontName = lua_getstring(param1);
		Font *result = g_resourceloader->loadFont(fontName);
		if (result) {
			lua_pushusertag(result, MKID('FONT'));
		}
	}
}

static void EnableDebugKeys() {
	DEBUG_FUNCTION();
	// in residual all keys are handled/enabled
}

static void LightMgrSetChange() {
	DEBUG_FUNCTION();
	// that seems only used when some control panel is opened
}

static void SetAmbientLight() {
	int mode;
	
	DEBUG_FUNCTION();
	mode = check_int(1);
	if (mode == 0) {
		if (g_engine->currScene() != NULL) {
			g_engine->currScene()->setLightEnableState(true);
		}
	} else if (mode == 1) {
		if (g_engine->currScene() != NULL) {
			g_engine->currScene()->setLightEnableState(false);
		}
	} else {
		error("SetAmbientLight() Unknown param %d", mode);
	}
}

static void RenderModeUser() {
	lua_Object param1;

	DEBUG_FUNCTION();
	param1 = lua_getparam(1);
	if (lua_isnumber(param1)) {
		g_engine->setPreviousMode(g_engine->getMode());
		g_smush->pause(true);
		g_engine->setMode(ENGINE_MODE_DRAW);
	} else if (lua_isnil(param1)) {
		g_smush->pause(false);
		g_engine->refreshDrawMode();
		g_engine->setMode(g_engine->getPreviousMode());
	} else {
		error("RenderModeUser() Unknown type of param");
	}
}

static void SetGamma() {
	DEBUG_FUNCTION();

	stubWarning("SetGamma");
}

static void Display() {
	DEBUG_FUNCTION();
	if (g_engine->getFlipEnable()) {
		g_engine->drawPrimitives();
		g_driver->flipBuffer();
	}
}

static void EngineDisplay() {
	// it enable/disable updating display
	DEBUG_FUNCTION();
	bool mode = check_int(1) != 0;
	if (mode) {
		g_engine->setFlipEnable(true);
	} else {
		g_engine->setFlipEnable(false);
	}
}

static void ForceRefresh() {
	g_engine->refreshDrawMode();
}

static void JustLoaded() {
	DEBUG_FUNCTION();
	if(debugLevel == DEBUG_ERROR || debugLevel == DEBUG_ALL)
		error("OPCODE USAGE VERIFICATION: JustLoaded");
}

static void PlaySound() {
	DEBUG_FUNCTION();
	if(debugLevel == DEBUG_ERROR || debugLevel == DEBUG_ALL)
		error("OPCODE USAGE VERIFICATION: PlaySound");
}

static void SetEmergencyFont() {
	DEBUG_FUNCTION();
	if(debugLevel == DEBUG_ERROR || debugLevel == DEBUG_ALL)
		error("OPCODE USAGE VERIFICATION: SetEmergencyFont");
}

/* Generate debug information for all functions
 *
 * When the debug flag is set to debug everything or "Functions"
 * then this will be called to print out the information on a
 * function.  This is useful for finding problems in a LUA script
 * that are due to a few function calls leading up to a failure.
 *
 * This function is also used by the "Stub" handler which will 
 * generate warnings about missing functions when the debug flag 
 * is set to "Stub", warnings, or everything.
 */
static void debugFunction(char *debugMessage, const char *funcName) {
	bool stubFn = strcmp(debugMessage, "WARNING: Stub function") == 0;
	FILE *output;
	
	if (!stubFn && debugLevel != DEBUG_FUNC
	 && debugLevel != DEBUG_ALL)
		return;
	
	if (stubFn)
		output = stderr;
	else
		output = stdout;
	fprintf(output, "%s %s(", debugMessage, funcName);
	for (int i = 1; ; i++) {
		if (lua_getparam(i) == LUA_NOOBJECT)
			break;
		if (lua_isnil(lua_getparam(i)))
			fprintf(output, "nil");
		else if (lua_istable(lua_getparam(i)))
			fprintf(output, "{...}");
		else if (lua_isuserdata(lua_getparam(i))) {
			if (lua_tag(lua_getparam(i)) == MKID('ACTR')) {
				Actor *a = check_actor(i);
				fprintf(output, "<actor \"%s\">", a->name());
			} else if (lua_tag(lua_getparam(i)) == MKID('COLR')) {
				Color *c = check_color(i);
				fprintf(output, "<color #%02x%02x%02x>", c->red(), c->green(), c->blue());
			} else
				fprintf(output, "<userdata %p>", lua_getuserdata(lua_getparam(i)));
		} else if (lua_isfunction(lua_getparam(i)))
			fprintf(output, "<function>");
		else if (lua_isnumber(lua_getparam(i)))
			fprintf(output, "%g", lua_getnumber(lua_getparam(i)));
		else if (lua_isstring(lua_getparam(i)))
			fprintf(output, "\"%s\"", lua_getstring(lua_getparam(i)));
		else
			fprintf(output, "<unknown>");
		if (lua_getparam(i+1) != LUA_NOOBJECT)
			fprintf(output, ", ");
	}
	fprintf(output, ") called\n");
#if 0
	lua_call("print_stack");
#endif
}

// Stub function for builtin functions not yet implemented
static void stubWarning(char *funcName) {
	// If the user doesn't want these debug messages then don't print them
	if(debugLevel != DEBUG_WARN && debugLevel != DEBUG_STUB && debugLevel != DEBUG_FUNC
	 && debugLevel != DEBUG_ALL)
		return;
	
	debugFunction("WARNING: Stub function", funcName);
}

#define STUB_FUNC(name) static void name() { stubWarning(#name); }
STUB_FUNC(SetActorInvClipNode)
STUB_FUNC(NukeResources)
STUB_FUNC(UnShrinkBoxes)
STUB_FUNC(ShrinkBoxes)
STUB_FUNC(ResetTextures)
STUB_FUNC(AttachToResources)
STUB_FUNC(DetachFromResources)
STUB_FUNC(GetSaveGameImage)
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
STUB_FUNC(SetActorShadowValid)
STUB_FUNC(AddShadowPlane)
STUB_FUNC(KillActorShadows)
STUB_FUNC(SetActiveShadow)
STUB_FUNC(SetActorShadowPoint)
STUB_FUNC(SetActorShadowPlane)
STUB_FUNC(ActivateActorShadow)
STUB_FUNC(SetShadowColor)
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
STUB_FUNC(LoadBundle)
STUB_FUNC(ActorShadow)
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
	char *name;
	int key;
} system_defaults[] = {
	{ "frameTime", 0 },
	{ "movieTime", 0 }
};

// Entries in the system.controls table

static struct {
	char *name;
	int key;
} controls[] = {
	{ "KEY_ESCAPE", SDLK_ESCAPE },
	{ "KEY_1", SDLK_1 },
	{ "KEY_2", SDLK_2 },
	{ "KEY_3", SDLK_3 },
	{ "KEY_4", SDLK_4 },
	{ "KEY_5", SDLK_5 },
	{ "KEY_6", SDLK_6 },
	{ "KEY_7", SDLK_7 },
	{ "KEY_8", SDLK_8 },
	{ "KEY_9", SDLK_9 },
	{ "KEY_0", SDLK_0 },
	{ "KEY_MINUS", SDLK_MINUS },
	{ "KEY_EQUALS", SDLK_EQUALS },
	{ "KEY_BACK", SDLK_BACKSPACE },
	{ "KEY_TAB", SDLK_TAB },
	{ "KEY_Q", SDLK_q },
	{ "KEY_W", SDLK_w },
	{ "KEY_E", SDLK_e },
	{ "KEY_R", SDLK_r },
	{ "KEY_T", SDLK_t },
	{ "KEY_Y", SDLK_y },
	{ "KEY_U", SDLK_u },
	{ "KEY_I", SDLK_i },
	{ "KEY_O", SDLK_o },
	{ "KEY_P", SDLK_p },
	{ "KEY_LBRACKET", SDLK_LEFTBRACKET },
	{ "KEY_RBRACKET", SDLK_RIGHTBRACKET },
	{ "KEY_RETURN", SDLK_RETURN },
	{ "KEY_LCONTROL", SDLK_LCTRL },
	{ "KEY_A", SDLK_a },
	{ "KEY_S", SDLK_s },
	{ "KEY_D", SDLK_d },
	{ "KEY_F", SDLK_f },
	{ "KEY_G", SDLK_g },
	{ "KEY_H", SDLK_h },
	{ "KEY_J", SDLK_j },
	{ "KEY_K", SDLK_k },
	{ "KEY_L", SDLK_l },
	{ "KEY_SEMICOLON", SDLK_SEMICOLON },
	{ "KEY_APOSTROPHE", SDLK_QUOTE },
	{ "KEY_GRAVE", SDLK_BACKQUOTE },
	{ "KEY_LSHIFT", SDLK_LSHIFT },
	{ "KEY_BACKSLASH", SDLK_BACKSLASH },
	{ "KEY_Z", SDLK_z },
	{ "KEY_X", SDLK_x },
	{ "KEY_C", SDLK_c },
	{ "KEY_V", SDLK_v },
	{ "KEY_B", SDLK_b },
	{ "KEY_N", SDLK_n },
	{ "KEY_M", SDLK_m },
	{ "KEY_COMMA", SDLK_COMMA },
	{ "KEY_PERIOD", SDLK_PERIOD },
	{ "KEY_SLASH", SDLK_SLASH },
	{ "KEY_RSHIFT", SDLK_RSHIFT },
	{ "KEY_MULTIPLY", SDLK_KP_MULTIPLY },
	{ "KEY_LMENU", SDLK_LALT },
	{ "KEY_SPACE", SDLK_SPACE },
	{ "KEY_CAPITAL", SDLK_CAPSLOCK },
	{ "KEY_F1", SDLK_F1 },
	{ "KEY_F2", SDLK_F2 },
	{ "KEY_F3", SDLK_F3 },
	{ "KEY_F4", SDLK_F4 },
	{ "KEY_F5", SDLK_F5 },
	{ "KEY_F6", SDLK_F6 },
	{ "KEY_F7", SDLK_F7 },
	{ "KEY_F8", SDLK_F8 },
	{ "KEY_F9", SDLK_F9 },
	{ "KEY_F10", SDLK_F10 },
	{ "KEY_NUMLOCK", SDLK_NUMLOCK },
	{ "KEY_SCROLL", SDLK_SCROLLOCK },
	{ "KEY_NUMPAD7", SDLK_KP7 },
	{ "KEY_NUMPAD8", SDLK_KP8 },
	{ "KEY_NUMPAD9", SDLK_KP9 },
	{ "KEY_SUBTRACT", SDLK_KP_MINUS },
	{ "KEY_NUMPAD4", SDLK_KP4 },
	{ "KEY_NUMPAD5", SDLK_KP5 },
	{ "KEY_NUMPAD6", SDLK_KP6 },
	{ "KEY_ADD", SDLK_KP_PLUS },
	{ "KEY_NUMPAD1", SDLK_KP1 },
	{ "KEY_NUMPAD2", SDLK_KP2 },
	{ "KEY_NUMPAD3", SDLK_KP3 },
	{ "KEY_NUMPAD0", SDLK_KP0 },
	{ "KEY_DECIMAL", SDLK_KP_PERIOD },
	{ "KEY_F11", SDLK_F11 },
	{ "KEY_F12", SDLK_F12 },
	{ "KEY_F13", SDLK_F13 },
	{ "KEY_F14", SDLK_F14 },
	{ "KEY_F15", SDLK_F15 },
	{ "KEY_STOP", SDLK_BREAK },
	{ "KEY_NUMPADENTER", SDLK_KP_ENTER },
	{ "KEY_RCONTROL", SDLK_RCTRL },
	{ "KEY_DIVIDE", SDLK_KP_DIVIDE },
	{ "KEY_SYSRQ", SDLK_SYSREQ },
	{ "KEY_RMENU", SDLK_RALT },
	{ "KEY_HOME", SDLK_HOME },
	{ "KEY_UP", SDLK_UP },
	{ "KEY_PRIOR", SDLK_PAGEUP },
	{ "KEY_LEFT", SDLK_LEFT },
	{ "KEY_RIGHT", SDLK_RIGHT },
	{ "KEY_END", SDLK_END },
	{ "KEY_DOWN", SDLK_DOWN },
	{ "KEY_NEXT", SDLK_PAGEDOWN },
	{ "KEY_INSERT", SDLK_INSERT },
	{ "KEY_DELETE", SDLK_DELETE },
	{ "KEY_LWIN", SDLK_LSUPER },
	{ "KEY_RWIN", SDLK_RSUPER },
	{ "KEY_APPS", SDLK_MENU },
	{ "KEY_JOY1_B1", SDLK_JOY1_B1 },
	{ "KEY_JOY1_B2", SDLK_JOY1_B2 },
	{ "KEY_JOY1_B3", SDLK_JOY1_B3 },
	{ "KEY_JOY1_B4", SDLK_JOY1_B4 },
	{ "KEY_JOY1_B5", SDLK_JOY1_B5 },
	{ "KEY_JOY1_B6", SDLK_JOY1_B6 },
	{ "KEY_JOY1_B7", SDLK_JOY1_B7 },
	{ "KEY_JOY1_B8", SDLK_JOY1_B8 },
	{ "KEY_JOY1_B9", SDLK_JOY1_B9 },
	{ "KEY_JOY1_B10", SDLK_JOY1_B10 },
	{ "KEY_JOY1_HLEFT", SDLK_JOY1_HLEFT },
	{ "KEY_JOY1_HUP", SDLK_JOY1_HUP },
	{ "KEY_JOY1_HRIGHT", SDLK_JOY1_HRIGHT },
	{ "KEY_JOY1_HDOWN", SDLK_JOY1_HDOWN },
	{ "KEY_JOY2_B1", SDLK_JOY2_B1 },
	{ "KEY_JOY2_B2", SDLK_JOY2_B2 },
	{ "KEY_JOY2_B3", SDLK_JOY2_B3 },
	{ "KEY_JOY2_B4", SDLK_JOY2_B4 },
	{ "KEY_JOY2_B5", SDLK_JOY2_B5 },
	{ "KEY_JOY2_B6", SDLK_JOY2_B6 },
	{ "KEY_JOY2_B7", SDLK_JOY2_B7 },
	{ "KEY_JOY2_B8", SDLK_JOY2_B8 },
	{ "KEY_JOY2_B9", SDLK_JOY2_B9 },
	{ "KEY_JOY2_B10", SDLK_JOY2_B10 },
	{ "KEY_JOY2_HLEFT", SDLK_JOY2_HLEFT },
	{ "KEY_JOY2_HUP", SDLK_JOY2_HUP },
	{ "KEY_JOY2_HRIGHT", SDLK_JOY2_HRIGHT },
	{ "KEY_JOY2_HDOWN", SDLK_JOY2_HDOWN },
	{ "KEY_MOUSE_B1", SDLK_MOUSE_B1 },
	{ "KEY_MOUSE_B2", SDLK_MOUSE_B2 },
	{ "KEY_MOUSE_B3", SDLK_MOUSE_B3 },
	{ "KEY_MOUSE_B4", SDLK_MOUSE_B4 },
	{ "AXIS_JOY1_X", SDLK_AXIS_JOY1_X },
	{ "AXIS_JOY1_Y", SDLK_AXIS_JOY1_Y },
	{ "AXIS_JOY1_Z", SDLK_AXIS_JOY1_Z },
	{ "AXIS_JOY1_R", SDLK_AXIS_JOY1_R },
	{ "AXIS_JOY1_U", SDLK_AXIS_JOY1_U },
	{ "AXIS_JOY1_V", SDLK_AXIS_JOY1_V },
	{ "AXIS_JOY2_X", SDLK_AXIS_JOY2_X },
	{ "AXIS_JOY2_Y", SDLK_AXIS_JOY2_Y },
	{ "AXIS_JOY2_Z", SDLK_AXIS_JOY2_Z },
	{ "AXIS_JOY2_R", SDLK_AXIS_JOY2_R },
	{ "AXIS_JOY2_U", SDLK_AXIS_JOY2_U },
	{ "AXIS_JOY2_V", SDLK_AXIS_JOY2_V },
	{ "AXIS_MOUSE_X", SDLK_AXIS_MOUSE_X },
	{ "AXIS_MOUSE_Y", SDLK_AXIS_MOUSE_Y },
	{ "AXIS_MOUSE_Z", SDLK_AXIS_MOUSE_Z }
};

struct luaL_reg mainOpcodes[] = {
	{ "EngineDisplay", EngineDisplay },
	{ "CheckForFile", CheckForFile },
	{ "Load", Load },
	{ "Save", Save },
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
	{ "ActorShadow", ActorShadow },
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

void register_lua() {
	// Register main opcodes functions
	luaL_openlib(mainOpcodes, ARRAYSIZE(mainOpcodes));

	// Register text opcodes functions
	luaL_openlib(textOpcodes, ARRAYSIZE(textOpcodes));

	// Register primitives opcodes functions
	luaL_openlib(primitivesOpcodes, ARRAYSIZE(primitivesOpcodes));

	// Register hardware opcodes functions
	luaL_openlib(hardwareOpcodes, ARRAYSIZE(hardwareOpcodes));

	// Register system table
	lua_Object system_table = lua_createtable();
	lua_pushobject(system_table);
	lua_setglobal("system");

	for (unsigned i = 0; i < ARRAYSIZE(system_defaults); i++) {
		lua_pushobject(system_table);
		lua_pushstring(system_defaults[i].name);
		lua_pushnumber(system_defaults[i].key);
		lua_settable();
	}

	// Create and populate system.controls table
	lua_Object controls_table = lua_createtable();
	lua_pushobject(system_table);
	lua_pushstring("controls");
	lua_pushobject(controls_table);
	lua_settable();

	for (unsigned i = 0; i < ARRAYSIZE(controls); i++) {
		lua_pushobject(controls_table);
		lua_pushstring(controls[i].name);
		lua_pushnumber(controls[i].key);
		lua_settable();
	}

	lua_pushobject(system_table);
	lua_pushstring("camChangeHandler");
	lua_pushcfunction(dummyHandler);
	lua_settable();

	lua_pushobject(system_table);
	lua_pushstring("axisHandler");
	lua_pushcfunction(dummyHandler);
	lua_settable();

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

	saveCallback = SaveCallback;
	restoreCallback = RestoreCallback;
}

int bundle_dofile(const char *filename) {
	Block *b = g_resourceloader->getFileBlock(filename);
	if (b == NULL) {
		delete b;
		// Don't print warnings on Scripts\foo.lua,
		// d:\grimFandango\Scripts\foo.lua
		if (std::strstr(filename, "Scripts\\") == NULL && (debugLevel == DEBUG_WARN || debugLevel == DEBUG_ALL))
			warning("Cannot find script %s\n", filename);

		return 2;
	}

	int result = lua_dobuffer(const_cast<char *>(b->data()), b->len(), const_cast<char *>(filename));
	delete b;
	return result;
}

lua_Object getTableFunction(lua_Object table, char *name) {
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
		if (debugLevel == DEBUG_WARN || debugLevel == DEBUG_ALL)
			warning("Invalid event handler %s", name);
		return LUA_NOOBJECT;
	}

	return handler;
}

lua_Object getTableValue(lua_Object table, char *name) {
	char *key_text = NULL;
	lua_Object key;

	if (!lua_istable(table)) {
		error("getTableValue(): Parameter not a table!\n");
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
		if(lua_call("next") != 0) {
			error("getTableValue could not find the next key!\n");
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
		error("getIndexedTableValue(): Parameter not a table!\n");
		return 0;
	}
	lua_pushobject(table);
	if (index == 1)
		lua_pushnil();
	else
		lua_pushnumber(index - 1);
	// If the call to "next" fails then register an error
	if (lua_call("next") != 0) {
		error("getIndexedTableValue failed to get next key!\n");
		return 0;
	}
	return lua_getresult(2);
}

void setTableValue(lua_Object table, char *name, int newvalue) {
	lua_pushobject(table);
	lua_pushstring(name);
	lua_pushnumber(newvalue);
	lua_settable();
}

void setTableValue(lua_Object table, char *name, lua_Object newvalue) {
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
Vector3d tableToVector(lua_Object table) {
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
	return Vector3d(x, y, z);
}

lua_Object getEventHandler(const char *name) {
	lua_Object system_table = lua_getglobal("system");
	return getTableFunction(system_table, (char *)name);
}
