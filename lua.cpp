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

#include "imuse/imuse.h"

#include <cstdio>
#include <cmath>
#include <SDL_keysym.h>
#include <SDL_keyboard.h>

static int actor_tag, color_tag, text_tag, vbuffer_tag, object_tag;

extern Imuse *g_imuse;

static inline bool isObject(int num) {
	if (lua_tag(lua_getparam(num)) != object_tag)
		return false;
	return true;
}

static inline bool isActor(int num) {
	if (lua_tag(lua_getparam(num)) != actor_tag)
		return false;
	return true;
}

static inline bool isColor(int num) {
	if (lua_tag(lua_getparam(num)) != color_tag)
		return false;
	return true;
}

// Helper functions to ensure the arguments we get are what we expect
static inline ObjectState *check_object(int num) {
	if (lua_tag(lua_getparam(num)) != object_tag)
		luaL_argerror(num, "objectstate expected");
	return static_cast<ObjectState *>(lua_getuserdata(lua_getparam(num)));
}

static inline Actor *check_actor(int num) {
	if (lua_tag(lua_getparam(num)) != actor_tag)
		luaL_argerror(num, "actor expected");
	return static_cast<Actor *>(lua_getuserdata(lua_getparam(num)));
}

static inline Color *check_color(int num) {
	if (lua_tag(lua_getparam(num)) != color_tag)
		luaL_argerror(num, "color expected");
	return static_cast<Color *>(lua_getuserdata(lua_getparam(num)));
}

static inline int check_int(int num) {
	double val = luaL_check_number(num);

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
		if (result == NULL)
			warning("Actor %s has no costume [%s]\n", a->name(), called_from);
	} else {
		result = a->findCostume(luaL_check_string(param));
		if (result == NULL)
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
	char *msg_str = luaL_check_string(1);
	std::fputs(msg_str, stderr);
}

static void PrintWarning() {
	char *msg = luaL_check_string(1);
	warning(msg);
}

static void FunctionName() {
	char *name;
	char buf[256];
	char *filename;
	int line;

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
	Color *c = new Color (clamp_color(check_int(1)), clamp_color(check_int(2)), clamp_color(check_int(3)));
	lua_pushusertag(c, color_tag);
}

static void GetColorComponents() {
	Color *c = check_color(1);
	lua_pushnumber(c->red());
	lua_pushnumber(c->green());
	lua_pushnumber(c->blue());
}

static void gc_Color() {
	Color *c = static_cast<Color *>(lua_getuserdata(lua_getparam(1)));
	delete c;
}

// Registry functions

static void ReadRegistryValue() {
	char *key = luaL_check_string(1);
	const char *val = g_registry->get(key);
	lua_pushstring(const_cast<char *>(val));
}

static void WriteRegistryValue() {
	char *key = luaL_check_string(1);
	char *val = luaL_check_string(2);
	g_registry->set(key, val);
}

// Localization function

static void LocalizeString() {
	char *str = luaL_check_string(1);
	std::string result = g_localizer->localize(str);
	lua_pushstring(const_cast<char *>(result.c_str()));
}

// Actor functions

static void LoadActor() {
	const char *name;
	if (lua_isnil(lua_getparam(1)))
		name = "<unnamed>";
	else
		name = luaL_check_string(1);
	lua_pushusertag(new Actor(name), actor_tag);
}

static void SetSelectedActor() {
	Actor *act = check_actor(1);
	g_engine->setSelectedActor(act);
}

static void SetActorTalkColor() {
	Actor *act = check_actor(1);
	Color *c = check_color(2);
	act->setTalkColor(*c);
}

static void GetActorTalkColor() {
	Actor *act = check_actor(1);
	Color *c = new Color(act->talkColor());
	lua_pushusertag(c, color_tag);
}

static void SetActorRestChore() {
	Actor *act = check_actor(1);
	int chore;
	Costume *costume;

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
	Actor *act = check_actor(1);
	int chore = check_int(2);
	Costume *costume = get_costume(act, 3, "SetActorWalkChore");

	act->setWalkChore(chore, costume);
}

static void SetActorTurnChores() {
	Actor *act = check_actor(1);
	int left_chore = check_int(2);
	int right_chore = check_int(3);
	Costume *costume = get_costume(act, 4, "SetActorTurnChores");

	act->setTurnChores(left_chore, right_chore, costume);
}

static void SetActorTalkChore() {
	Actor *act = check_actor(1);
	int index = check_int(2);
	int chore;

	if (lua_isnumber(lua_getparam(3)))
		chore = check_int(3);
	else
		chore = -1;

	Costume *costume = get_costume(act, 4, "setActorTalkChore");

	act->setTalkChore(index, chore, costume);
}

static void SetActorMumblechore() {
	Actor *act = check_actor(1);
	int chore = check_int(2);
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
	float x = luaL_check_number(2);
	float y = luaL_check_number(3);
	float z = luaL_check_number(4);

	act->setPos(Vector3d(x, y, z));
}

static void GetActorPos() {
	Actor *act = check_actor(1);
	Vector3d pos = act->pos();
	lua_pushnumber(pos.x());
	lua_pushnumber(pos.y());
	lua_pushnumber(pos.z());
}

static void SetActorRot() {
	Actor *act = check_actor(1);
	float pitch = luaL_check_number(2);
	float yaw = luaL_check_number(3);
	float roll = luaL_check_number(4);

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
	Actor *act = check_actor(1);
	double x = luaL_check_number(2);
	double y = luaL_check_number(3);
	double z = luaL_check_number(4);
	lua_pushnumber(act->yawTo(Vector3d(x, y, z)));
}

static void PutActorInSet() {
	Actor *act = check_actor(1);
	const char *set = "";
	if (! lua_isnil(lua_getparam(2)))
		set = luaL_check_string(2);
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

static void GetActorPuckVector() {
	Actor *act = check_actor(1);
	Vector3d result = act->puckVector();
	lua_pushnumber(result.x());
	lua_pushnumber(result.y());
	lua_pushnumber(result.z());
}

static void WalkActorTo() {
	Actor *act = check_actor(1);
	float x = luaL_check_number(2);
	float y = luaL_check_number(3);
	float z = luaL_check_number(4);

	act->walkTo(Vector3d(x, y, z));
}

static void IsActorMoving() {
	Actor *act = check_actor(1);
	pushbool(act->isWalking());
}

static void Is3DHardwareEnabled() {
	pushbool(true);
}

static void IsActorResting() {
	Actor *act = check_actor(1);
	pushbool(!(act->isWalking() || act->isTurning()));
}

static void TurnActor() {
	Actor *act = check_actor(1);
	int dir = check_int(2);
	act->turn(dir);
}

static void PushActorCostume() {
	Actor *act = check_actor(1);
	const char *costumeName = luaL_check_string(2);
	act->pushCostume(costumeName);
}

static void SetActorCostume() {
	Actor *act = check_actor(1);
	if (lua_isnil(lua_getparam(2)))
		act->clearCostumes();
	else {
		const char *costumeName = luaL_check_string(2);
		act->setCostume(costumeName);
	}
}

static void GetActorCostume() {
	Actor *act = check_actor(1);
	Costume *c = act->currentCostume();
	if (c == NULL)
		lua_pushnil();
	else
		lua_pushstring(const_cast<char *>(c->filename()));
}

static void PopActorCostume() {
	Actor *act = check_actor(1);
	act->popCostume();
}

static void GetActorCostumeDepth() {
	Actor *act = check_actor(1);
	lua_pushnumber(act->costumeStackDepth());
}

static void PlayActorChore() {
	Actor *act = check_actor(1);
	int num = check_int(2);
	Costume *cost = get_costume(act, 3, "playActorChore");

	if (!cost)
		return;

	cost->playChore(num);
}

static void PlayActorChoreLooping() {
	Actor *act = check_actor(1);
	int num = check_int(2);
	Costume *cost = get_costume(act, 3, "playActorChoreLooping");

	if (!cost)
		return;

	cost->playChoreLooping(num);
}

static void SetActorChoreLooping() {
	Actor *act = check_actor(1);
	int num = check_int(2);
	bool val = getbool(3);
	Costume *cost = get_costume(act, 4, "setActorChoreLooping");

	if (!cost)
		return;

	cost->setChoreLooping(num, val);
}

static void StopActorChore() {
	Actor *act = check_actor(1);
	Costume *cost = get_costume(act, 3, "stopActorChore");

	if (!cost)
		return;

	if (lua_isnil(lua_getparam(2)))
		cost->stopChores();
	else
		cost->stopChore(check_int(2));
}

static void IsActorChoring() {
	Actor *act = check_actor(1);
	bool excludeLooping = getbool(3);
	Costume *cost = get_costume(act, 4, "isActorChoring");
	int result;

	if (!cost) {
		lua_pushnil();
		return;
	}

	if (lua_isnil(lua_getparam(2)))
		result = cost->isChoring(excludeLooping);
	else
		result = cost->isChoring(check_int(2), excludeLooping);

	if (result < 0)
		lua_pushnil();
	else
		lua_pushnumber(result);
}

static void ActorLookAt() {
	Actor *act = check_actor(1);
	lua_Object x = lua_getparam(2);
	lua_Object y = lua_getparam(3);
	lua_Object z = lua_getparam(4);
	lua_Object rate = lua_getparam(5);

	if (lua_isnumber(rate))
		act->setLookAtRate(luaL_check_number(5));

	// Look at nothing
	if (lua_isnil(x)) {
		if (act->isLookAtVectorZero()) // already looking at nothing
			return;

		act->setLookAtVectorZero();

		if (lua_isnumber(y))
			act->setLookAtRate(luaL_check_number(3));

		act->setLooking(true);
		return;
	} else if ( lua_isnumber(x)) { // look at xyz
		Vector3d vector;
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

		act->setLookAtVector( vector );
	} else if (isActor(2)) { // look at another actor
		Actor *lookedAct = check_actor(2);

		act->setLookAtVector(lookedAct->pos());

		if (lua_isnumber(y))
			act->setLookAtRate(luaL_check_number(3));
	}

	act->setLooking(true);
}

static void SetActorLookRate() {
	Actor *act = check_actor(1);
	float rate = luaL_check_number(2);

	act->setLookAtRate(rate);
}

static void GetActorLookRate() {
	Actor *act = check_actor(1);

	lua_pushnumber(act->lookAtRate());
}

static void SetActorHead() {
	Actor *act = check_actor(1);
	int joint1 = check_int(2);
	int joint2 = check_int(3);
	int joint3 = check_int(4);
	float maxRoll = luaL_check_number(5); // Yaz: recheck to see if it's really roll
	float maxPitch = luaL_check_number(6);
	float maxYaw = luaL_check_number(7);

	act->setHead(joint1, joint2, joint3, maxRoll, maxPitch, maxYaw);
}

static void SetActorFollowBoxes() {	// Constrain actor to walkplanes?
	Actor *act = check_actor(1);
	bool constrain = !lua_isnil(lua_getparam(2));

	act->setConstrain(constrain);
}

static void GetVisibleThings() {
	lua_Object result = lua_createtable();
	Actor *sel = g_engine->selectedActor();
	for (Engine::ActorListType::const_iterator i = g_engine->actorsBegin(); i != g_engine->actorsEnd(); i++) {
		if (!(*i)->inSet(g_engine->sceneName()))
			continue;
		if (sel->angleTo(*(*i)) < 90) {
			lua_pushobject(result);
			lua_pushusertag(*i, actor_tag);
			lua_pushnumber(1);
			lua_settable();
		}
	}
	lua_pushobject(result);
}

static void SayLine() {
	Actor *act = check_actor(1);
	const char *msg = luaL_check_string(2);
	act->sayLine(msg);
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
	Actor *act = check_actor(1);
	if (act)
		act->shutUp();
}

static void HardwareAccelerated() {
	// FIXME: Are we always in HW accelerated ?
	lua_pushnumber(true);
}

// Sector functions
static void GetActorSector(void) {
	Actor *act = check_actor(1);
	int sectorType = check_int(2);

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
	Actor *act = check_actor(1);
	const char *name = luaL_check_string(2);
	int i, numSectors = g_engine->currScene()->getSectorCount();

	for (i=0; i<numSectors; i++) {
		Sector *sector = g_engine->currScene()->getSectorBase(i);

		if (sector->visible() && strstr(sector->name(), name)) {
			if (sector->isPointInSector(act->pos())) {
				lua_pushnumber(sector->id());
				lua_pushstring((char *)sector->name());
				lua_pushnumber(sector->type());
			}
		}
	}

	lua_pushnil();
}

static void MakeSectorActive(void) {
	lua_Object sectorName = lua_getparam(1);
	bool visible = !lua_isnil(lua_getparam(2));
	int i = 0, numSectors;

	// FIXME: This happens on initial load. Are we initting something in the wrong order?
	if (!g_engine->currScene()) {
		warning("!!!! Trying to call MakeSectorActive without a scene!");
		return;
	}

	numSectors = g_engine->currScene()->getSectorCount();

	if (lua_isstring(sectorName)) {
		char *name = luaL_check_string(1);

		for (i = 0; i < numSectors; i++) {
			Sector *sector = g_engine->currScene()->getSectorBase(i);
			if (strstr(sector->name(), name)) {
				sector->setVisible(visible);
				return;
			}
		}
	} else if (lua_isnumber(sectorName)) {
		int id = check_int(1);

		for (i=0; i<numSectors; i++) {
			Sector *sector = g_engine->currScene()->getSectorBase(i);
			if (sector->id() == id) {
				sector->setVisible(visible);
				return;
			}
		}
	} else {
		warning("MakeSectorActive Parameter is not a sector ID or Name");
		return;
	}
}

// Scene functions

static void MakeCurrentSet() {
	const char *name = luaL_check_string(1);
	g_engine->setScene(name);
}

static void MakeCurrentSetup() {
	int num = check_int(1);
	int prevSetup = g_engine->currScene()->setup();

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

static void GetCurrentSetup() {
	const char *name = luaL_check_string(1);
	if (std::strcmp(name, g_engine->sceneName()) == 0)
		lua_pushnumber(g_engine->currScene()->setup());
	else
		lua_pushnil();
}

static void GetShrinkPos() {
	// FIXME
	double x = luaL_check_number(1);
	double y = luaL_check_number(2);
	double z = luaL_check_number(3);
	double r = luaL_check_number(4);
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
	char *soundName = luaL_check_string(1);
	int priority = check_int(2);
	int group = check_int(3);

	if (g_imuse->startSound(soundName, group, 0, 127, 0, priority)) {
		lua_pushstring(soundName);
	} else {
		lua_pushnil();
	}
}

static void ImStopSound() {
	char *soundName = luaL_check_string(1);
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
	char *effectName = luaL_check_string(1);
	warning("ImSetVoiceEffect(%s) Voice effects are not yet supported", effectName);
}

static void ImSetMusicVol() {
	g_imuse->setGroupMusicVolume(check_int(1));
}

static void ImGetMusicVol() {
	lua_pushnumber(g_imuse->getGroupMusicVolume());
}

static void ImSetVoiceVol() {
	g_imuse->setGroupVoiceVolume(check_int(1));
}

static void ImGetVoiceVol() {
	lua_pushnumber(g_imuse->getGroupVoiceVolume());
}

static void ImSetSfxVol() {
	g_imuse->setGroupSfxVolume(check_int(1));
}

static void ImGetSfxVol() {
	lua_pushnumber(g_imuse->getGroupSfxVolume());
}

static void ImSetParam() {
	char *soundName = luaL_check_string(1);
	int param = check_int(2);
	int value = check_int(3);

	switch (param) {
	case IM_SOUND_VOL:
		g_imuse->setVolume(soundName, value);
		break;
	case IM_SOUND_PAN:
		g_imuse->setPan(soundName, value);
		break;
	default:
		lua_pushnil();
		warning("ImSetParam() Unimplemented %d, %d\n", param, value);
	}
}

void ImGetParam() {
	char *soundName = luaL_check_string(1);
	int param = check_int(2);

	switch (param) {
	case IM_SOUND_PLAY_COUNT:
		lua_pushnumber(g_imuse->getCountPlayedTracks());
		break;
	case IM_SOUND_VOL:
		lua_pushnumber(g_imuse->getVolume(soundName));
		break;
	default:
		lua_pushnil();
		warning("ImGetParam() Unimplemented %d\n", param);
	}
}

static void ImFadeParam() {
	char *soundName = luaL_check_string(1);
	int opcode = check_int(2);
	int value = check_int(3);
	int fadeDelay = check_int(4);

	switch (opcode) {
	case IM_SOUND_PAN:
		// it should be fade panning really
		g_imuse->setPan(soundName, value);
		break;
	default:
		error("ImFadeParam(%s, %h, %d, %d)", soundName, opcode, value, fadeDelay);
		break;
	}
}

static void ImSetState() {
	int state = check_int(1);
	g_imuse->setMusicState(state);
}

static void ImSetSequence() {
	int state = check_int(1);
	g_imuse->setMusicSequence(state);
}

static void SaveIMuse() {
	warning("SaveIMuse() is not yet supported");
}

static void RestoreIMuse() {
	warning("RestoreIMuse() is not yet supported");
}

static void SetSoundPosition() {
	warning("SetSoundPosition() is not yet supported");
}

static void IsSoundPlaying() {
	error("IsSoundPlaying() is not supported");
}

static void PlaySoundAt() {
	error("PlaySoundAt() is not supported");
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
	float rate = luaL_check_number(1);
	lua_pushnumber(g_engine->perSecond(rate));
}

void EnableControl() {
	int num = check_control(1);
	g_engine->enableControl(num);
}

void DisableControl() {
	int num = check_control(1);
	g_engine->disableControl(num);
}

void GetControlState() {
	int num = check_control(1);
	if (num >= SDLK_JOY1_B1 && num <= SDLK_MOUSE_B4)
		lua_pushnil();
	else if (num >= SDLK_AXIS_JOY1_X && num <= SDLK_AXIS_MOUSE_Z)
		lua_pushnumber(0);
	else {
		Uint8 *keystate = SDL_GetKeyState(NULL);
		pushbool(keystate[num] != 0);
	}
}

static void MakeTextObject() {
	char *line = lua_getstring(lua_getparam(1)), *key_text = NULL;
	lua_Object table_obj = lua_getparam(2), key;
	int x = 0, y = 0, height = 0, width = 0;
	Color *fgColor = NULL;
	TextObject *textObject;

	for (;;) {
		lua_pushobject(table_obj);
		if (key_text)
			lua_pushobject(key);
		else
			lua_pushnil();

		lua_call("next");
		key=lua_getresult(1);
		if (lua_isnil(key)) 
			break;

		key_text=lua_getstring(key);
		//val_text=lua_getstring(lua_getresult(2));
		if (strstr(key_text, "x"))
			x = atoi(lua_getstring(lua_getresult(2)));
		else if (strstr(key_text, "y"))
			y = atoi(lua_getstring(lua_getresult(2)));
		else if (strstr(key_text, "fgcolor"))
			fgColor = check_color(2);
		else if (strstr(key_text, "height")) // Hm, do these just force clipping?
			height = atoi(lua_getstring(lua_getresult(2)));
		else if (strstr(key_text, "width"))
			width = atoi(lua_getstring(lua_getresult(2)));
		else if (strstr(key_text, "center"))
			warning("MakeTextObject key center not implemented");
		else if (strstr(key_text, "font"))
			warning("MakeTextObject key font not implemented");
		else
			error("Unknown MakeTextObject key %s\n", key_text);
	}

	textObject = new TextObject((const char *)line, x, y, *fgColor);
	lua_pushstring(line);	// FIXME: Register a LUA text object and pass that instead?
}

static void KillTextObject() {
	char *textID;

	if (lua_isnil(lua_getparam(1))) { // FIXME: check this.. nil is kill all lines?
		error("KillTextObject(NULL)");
		//g_engine->killTextObjects();
		return;
	}

	textID = lua_getstring(lua_getparam(1));

	for (Engine::TextListType::const_iterator i = g_engine->textsBegin(); i != g_engine->textsEnd(); i++) {
		TextObject *textO = *i;

		if (strstr(textO->name(), textID)) {
			g_engine->killTextObject(textO);
			delete textO;
			return;
		}
	}
}

// Called from both Callback and Main CTO functions. This routine is NOT
// thread safe.
static void ChangeTextObject_Real(char *keyName, void *data) {
	static TextObject *modifyObject = NULL; // Set by main CTO call 'object'
	lua_Object *keyValue = NULL;

	if (strstr(keyName, "object")) {
		modifyObject = (TextObject*)data;
		return;
	}

	if (!modifyObject)	// We *need* a modify object for remaining calls
		return;

	keyValue = (lua_Object*)data;

	// FIXME: X/Y sets depend on GetTextObjectDimensions

	if (strstr(keyName, "fgcolor")) {
		modifyObject->setColor(check_color(2));
	} else if (strstr(keyName, "x")) {
		//modifyObject->setX(atoi(lua_getstring(keyValue)));
	} else if (strstr(keyName, "y")) {
		//modifyObject->setY(atoi(lua_getstring(keyValue)));
	} else {
		printf("ChangeTextObject() - Unknown key %s\n", keyName);
	}
}

// Callback from table walk method in Main CTO function
static void ChangeTextObject_CB(void) {
	char *keyName = NULL;
	lua_Object keyValue;

	keyName = lua_getstring(lua_getparam(1));
	keyValue = lua_getresult(2);
	ChangeTextObject_Real(keyName, &keyValue);
};

// Main CTO handler and LUA interface
static void ChangeTextObject() {
	char *textID = lua_getstring(lua_getparam(1));
	lua_Object tableObj = lua_getparam(2);
	TextObject *modifyObject = NULL;

	for (Engine::TextListType::const_iterator i = g_engine->textsBegin(); i != g_engine->textsEnd(); i++) {
		TextObject *textO = *i;

		if (strstr(textO->name(), textID)) {
			modifyObject = textO;
			break;
		}
	}

	if (!modifyObject)
		error("ChangeTextObject(%s): Cannot find active text object", textID);

	if (!lua_istable(tableObj))
		return;

	ChangeTextObject_Real("object", modifyObject);
	lua_pushobject(tableObj);
	lua_pushcfunction(ChangeTextObject_CB);	// Callback handler
	lua_call("foreach");
}

static void GetTextObjectDimensions() {
	warning("STUB: GetTextObjectDimensions()");
	lua_pushnumber(100);	// Dummy X
	lua_pushnumber(100);	// Dummy Y
}

static void SetSpeechMode() {
	int mode = check_int(1);
	if ((mode >= 1) && (mode <= 3))
 		g_engine->setSpeechMode(mode);
}

static void GetSpeechMode() {
	int mode = g_engine->getSpeechMode();
 	lua_pushnumber(mode);
}

static void StartFullscreenMovie() {
	bool mode = getbool(2);
	g_engine->setMode(ENGINE_MODE_SMUSH);
	pushbool(g_smush->play(luaL_check_string(1), 0, 0));
}

static void StartMovie() {
	bool mode = getbool(2);
	int x = 0;
	int y = 0;

	if (!lua_isnil(lua_getparam(3)))
		x = check_int(3);

	if (!lua_isnil(lua_getparam(4)))
		y = check_int(4);

	g_engine->setMode(ENGINE_MODE_NORMAL);
	pushbool(g_smush->play(luaL_check_string(1), x, y));
}

static void IsFullscreenMoviePlaying() {
	pushbool(g_smush->isPlaying());
}

static void IsMoviePlaying() {
	pushbool(g_smush->isPlaying());
}

static void StopMovie() {
	g_smush->stop();
}

static void PauseMovie() {
	g_smush->pause(lua_isnil(lua_getparam(1)) != 0);
}

static void GetTextCharPosition() {
	warning("STUB GetTextCharPosition(\"%s\", %d)", lua_getstring(lua_getparam(1)), lua_getnumber(lua_getparam(2)));
	lua_pushnumber(0);
}

static void GetDiskFreeSpace() {
	// amount of free space in MB, used for creating saves
	lua_pushnumber(50);
}

// Objectstate functions
static void NewObjectState() {
	enum ObjectPosition {
		OBJSTATE_UNDERLAY = 1,
		OBJSTATE_OVERLAY = 2,
		OBJSTATE_STATE = 3
	};

	ObjectState *object = NULL;

	int setupID = check_int(1);		// Setup ID
	ObjectState::Position pos = check_objstate_pos(2); // When to draw
	char *bitmap = luaL_check_string(3);	// Bitmap
	char *zbitmap = NULL;			// Zbuffer Bitmap
	bool visible = getbool(5);		// Starts visible?

	if (!lua_isnil(lua_getparam(4)))
		zbitmap = luaL_check_string(4);

	object = new ObjectState(setupID, pos, bitmap, zbitmap, visible);
	g_engine->currScene()->addObjectState(object);
	lua_pushusertag(object, object_tag);
}

static void FreeObjectState() {
	ObjectState *object = check_object(1);
	warning("State: freeObjectState(...)");
	//scene.deleteObjectState(object);
}

static void GetCurrentScript() {
	current_script();
}

static void Load() {
	lua_Object fileName = lua_getparam(1);
	if (lua_isnil(fileName)) {
		g_engine->_savegameFileName = NULL;
	} else if (lua_isstring(fileName)) {
		g_engine->_savegameFileName = lua_getstring(fileName);
	} else {
		warning("Load() fileName is wrong");
		return;
	}
	g_engine->_savegameLoadRequest = true;
}

static void Save() {
	lua_Object fileName = lua_getparam(1);
	if (lua_isnil(fileName)) {
		g_engine->_savegameFileName = NULL;
	} else if (lua_isstring(fileName)) {
		g_engine->_savegameFileName = lua_getstring(fileName);
	} else {
		warning("Save() fileName is wrong");
		return;
	}
	g_engine->_savegameSaveRequest = true;
}

static int SaveCallback(int tag, int value, SaveRestoreFunc saveFunc) {
	return value;
}

static int RestoreCallback(int tag, int value, SaveRestoreFunc saveFunc) {
	return value;
}

// Stub function for builtin functions not yet implemented

static void stubWarning(char *funcName) {
	fprintf(stderr, "WARNING: Stub function %s(", funcName);
	for (int i = 1; ; i++) {
		if (lua_getparam(i) == LUA_NOOBJECT)
			break;
		if (lua_isnil(lua_getparam(i)))
			fprintf(stderr, "nil");
		else if (lua_istable(lua_getparam(i)))
			fprintf(stderr, "{...}");
		else if (lua_isuserdata(lua_getparam(i))) {
			if (lua_tag(lua_getparam(i)) == actor_tag) {
				Actor *a = check_actor(i);
				fprintf(stderr, "<actor \"%s\">", a->name());
			} else if (lua_tag(lua_getparam(i)) == color_tag) {
				Color *c = check_color(i);
				fprintf(stderr, "<color #%02x%02x%02x>", c->red(), c->green(), c->blue());
			} else
				fprintf(stderr, "<userdata %p>", lua_getuserdata(lua_getparam(i)));
		} else if (lua_isfunction(lua_getparam(i)))
			fprintf(stderr, "<function>");
		else if (lua_isnumber(lua_getparam(i)))
			fprintf(stderr, "%g", lua_getnumber(lua_getparam(i)));
		else if (lua_isstring(lua_getparam(i)))
			fprintf(stderr, "\"%s\"", lua_getstring(lua_getparam(i)));
		else
			fprintf(stderr, "<unknown>");
		if (lua_getparam(i+1) != LUA_NOOBJECT)
			fprintf(stderr, ", ");
	}
	fprintf(stderr, ") called\n");
#if 0
	lua_call("print_stack");
#endif
}

static void BlastText() {
	char * str = luaL_check_string(1), *key_text = NULL;
	lua_Object table_obj = lua_getparam(2), key;
	int x = 0, y = 0, height = 0, width = 0;
	Color *fgColor = NULL;

	for (;;) {
		lua_pushobject(table_obj);
		if (key_text)
			lua_pushobject(key);
		else
			lua_pushnil();

		lua_call("next");
		key = lua_getresult(1);

		if (lua_isnil(key)) 
			break;

		key_text = lua_getstring(key);
		//val_text=lua_getstring(lua_getresult(2));
		if (strstr(key_text, "x"))
			x = atoi(lua_getstring(lua_getresult(2)));
		else if (strstr(key_text, "y"))
			y = atoi(lua_getstring(lua_getresult(2)));
		else if (strstr(key_text, "fgcolor"))
			fgColor = check_color(2);
		else if (strstr(key_text, "font"))
			lua_getresult(2);
		else if (strstr(key_text, "center")) // TRUE or FALSE
			lua_getresult(2);
		else if (strstr(key_text, "disabled")) // TRUE or FALSE
			lua_getresult(2);
		else
			error("Unknown BlastText key %s\n", key_text);
	}

	warning("STUB: BlastText(\"%s\", x = %d, y = %d)\n", g_localizer->localize(str).c_str(), x, y);
}

#define STUB_FUNC(name) static void name() { stubWarning(#name); }
STUB_FUNC(SetActorInvClipNode)
STUB_FUNC(NukeResources)
STUB_FUNC(UnShrinkBoxes)
STUB_FUNC(ShrinkBoxes)
STUB_FUNC(ResetTextures)
STUB_FUNC(JustLoaded)
STUB_FUNC(AttachToResources)
STUB_FUNC(DetachFromResources)
STUB_FUNC(GetTextSpeed)
STUB_FUNC(SetTextSpeed)
STUB_FUNC(GetSaveGameData)
STUB_FUNC(SubmitSaveGameData)
STUB_FUNC(BlastRect)
STUB_FUNC(BlastImage)
STUB_FUNC(FreeImage)
STUB_FUNC(GetImage)
STUB_FUNC(GetSaveGameImage)
STUB_FUNC(ScreenShot)
STUB_FUNC(TextFileGetLine)
STUB_FUNC(TextFileGetLineCount)
STUB_FUNC(IrisUp)
STUB_FUNC(IrisDown)
STUB_FUNC(PlaySound)
STUB_FUNC(FadeInChore)
STUB_FUNC(FadeOutChore)
STUB_FUNC(SetActorClipPlane)
STUB_FUNC(SetActorClipActive)
STUB_FUNC(SetActorCollisionScale)
STUB_FUNC(SetActorCollisionMode)
STUB_FUNC(FlushControls)
STUB_FUNC(ActorToClean)
STUB_FUNC(SendObjectToFront)
STUB_FUNC(SendObjectToBack)
STUB_FUNC(SetObjectType)
STUB_FUNC(SetActorShadowValid)
STUB_FUNC(AddShadowPlane)
STUB_FUNC(KillActorShadows)
STUB_FUNC(SetActiveShadow)
STUB_FUNC(SetActorShadowPoint)
STUB_FUNC(SetActorShadowPlane)
STUB_FUNC(ActivateActorShadow)
STUB_FUNC(SetShadowColor)
STUB_FUNC(Display)
STUB_FUNC(CleanBuffer)
STUB_FUNC(DimRegion)
STUB_FUNC(DimScreen)
STUB_FUNC(ForceRefresh)
STUB_FUNC(RenderModeUser)
STUB_FUNC(SetGamma)
STUB_FUNC(LightMgrSetChange)
STUB_FUNC(LightMgrStartup)
STUB_FUNC(SetLightIntensity)
STUB_FUNC(SetLightPosition)
STUB_FUNC(TurnLightOn)
STUB_FUNC(SetAmbientLight)
STUB_FUNC(GetAngleBetweenVectors)
STUB_FUNC(TurnActorTo)
STUB_FUNC(PointActorAt)
STUB_FUNC(GetCameraLookVector)
STUB_FUNC(SetCameraRoll)
STUB_FUNC(SetCameraInterest)
STUB_FUNC(GetCameraPosition)
STUB_FUNC(RotateVector)
STUB_FUNC(LoadCostume)
STUB_FUNC(PrintActorCostumes)
STUB_FUNC(SpewStartup)
STUB_FUNC(PreRender)
STUB_FUNC(GetSectorOppositeEdge)
STUB_FUNC(FileFindDispose)
STUB_FUNC(FileFindNext)
STUB_FUNC(FileFindFirst)
STUB_FUNC(PreviousSetup)
STUB_FUNC(NextSetup)
STUB_FUNC(UnLockSet)
STUB_FUNC(LockSet)
STUB_FUNC(LockFont)
STUB_FUNC(EnableDebugKeys)
STUB_FUNC(WorldToScreen)
STUB_FUNC(CompleteActorChore)
STUB_FUNC(SetActorRoll)
STUB_FUNC(SetActorPitch)
STUB_FUNC(GetPointSector)
STUB_FUNC(IsPointInSector)
STUB_FUNC(SetActorFrustrumCull)
STUB_FUNC(SetActorWalkDominate)
STUB_FUNC(SetActorConstrain)
STUB_FUNC(GetCameraActor)
STUB_FUNC(DriveActorTo)
STUB_FUNC(WalkActorVector)
STUB_FUNC(PutActorAtInterest)
STUB_FUNC(SetActorReflection)
STUB_FUNC(GetActorRect)
STUB_FUNC(GetActorNodeLocation)
STUB_FUNC(SetActorTimeScale)
STUB_FUNC(GetActorTimeScale)
STUB_FUNC(SetActorScale)
STUB_FUNC(SetActorColormap)
STUB_FUNC(SearchForFileOrSwapCDs)
STUB_FUNC(EngineDisplay)
STUB_FUNC(SetOffscreenTextPos)
STUB_FUNC(SetEmergencyFont)
STUB_FUNC(GetTranslationMode)
STUB_FUNC(SetTranslationMode)
STUB_FUNC(ExpireText)
STUB_FUNC(PrintLine)
STUB_FUNC(SetSayLineDefaults)
STUB_FUNC(PurgePrimitiveQueue)
STUB_FUNC(KillPrimitive)
STUB_FUNC(ChangePrimitive)
STUB_FUNC(DrawRectangle)
STUB_FUNC(DrawPolygon)
STUB_FUNC(DrawLine)
STUB_FUNC(WalkActorToAvoiding)
STUB_FUNC(GetActorChores)
STUB_FUNC(Exit)
STUB_FUNC(PrintError)
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
STUB_FUNC(GetVideoDevices)
STUB_FUNC(SetVideoDevices)
STUB_FUNC(SetHardwareState)
STUB_FUNC(Enumerate3DDevices)
STUB_FUNC(EnumerateVideoDevices)

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
	{ "FileFindFirst", FileFindFirst },
	{ "FileFindNext", FileFindNext },
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
	// Create various LUA tags
	actor_tag = lua_newtag();
	color_tag = lua_newtag();
	text_tag = lua_newtag();
	vbuffer_tag = lua_newtag();
	object_tag = lua_newtag();

	// Register main opcodes functions
	luaL_openlib(mainOpcodes, sizeof(mainOpcodes) / sizeof(mainOpcodes[0]));

	// Register text opcodes functions
	luaL_openlib(textOpcodes, sizeof(textOpcodes) / sizeof(textOpcodes[0]));

	// Register primitives opcodes functions
	luaL_openlib(primitivesOpcodes, sizeof(primitivesOpcodes) / sizeof(primitivesOpcodes[0]));

	// Register hardware opcodes functions
	luaL_openlib(hardwareOpcodes, sizeof(hardwareOpcodes) / sizeof(hardwareOpcodes[0]));

	// Register system table
	lua_Object system_table = lua_createtable();
	lua_pushobject(system_table);
	lua_setglobal("system");

	for (unsigned i = 0; i < sizeof(system_defaults) / sizeof(system_defaults[0]); i++) {
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

	for (unsigned i = 0; i < sizeof(controls) / sizeof(controls[0]); i++) {
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
		if (std::strstr(filename, "Scripts\\") == NULL)
			warning("Cannot find script %s\n", filename);

		return 2;
	}

	int result = lua_dobuffer(const_cast<char *>(b->data()), b->len(), const_cast<char *>(filename));
	delete b;
	return result;
}

lua_Object getEventHandler(const char *name) {
	lua_Object system_table = lua_getglobal("system");
	lua_pushobject(system_table);
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
		warning("Invalid event handler %s", name);
		return LUA_NOOBJECT;
	}

	return handler;
}
