// Residual - Virtual machine to run LucasArts' 3D adventure games
// Copyright (C) 2003 The ScummVM-Residual Team (www.scummvm.org)
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
#include "lua.h"
#include "resource.h"
#include "debug.h"
#include "actor.h"
#include "registry.h"
#include "localize.h"
#include "color.h"
#include "costume.h"
#include "engine.h"
#include "mixer.h"
#include "sound.h"
#include "textobject.h"
#include <SDL_keysym.h>
#include <SDL_keyboard.h>
#include <cstdio>
#include <cmath>

static int actor_tag, color_tag, sound_tag;

// Helper functions to ensure the arguments we get are what we expect

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

static inline Sound *check_sound(int num) {
  if (lua_tag(lua_getparam(num)) != sound_tag)
    luaL_argerror(num, "sound expected");
  return static_cast<Sound *>(lua_getuserdata(lua_getparam(num)));
}

static inline int check_int(int num) {
  double val = luaL_check_number(num);
#ifndef _MSC_VER
  return int(round(val));
#else
  return int(/*round*/(val));
#endif
}

static inline int check_control(int num) {
  int val = check_int(num);
  if (val < 0 || val >= SDLK_EXTRA_LAST)
    luaL_argerror(num, "control identifier out of range");
  return val;
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
  }
  else {
    result = a->findCostume(luaL_check_string(param));
    if (result == NULL)
      warning("Actor %s has no costume %s [%s]\n", a->name(),
	    lua_getstring(lua_getparam(param)), called_from);
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

  if (! lua_isfunction(lua_getparam(1)))
    luaL_argerror(1, "expected function");
  lua_funcinfo(lua_getparam(1), &filename, &line);
  switch (*lua_getobjname(lua_getparam(1), &name)) {
  case 'g':
    sprintf(buf, "function %.100s", name);
    break;
  case 't':
    sprintf(buf, "`%.100s' tag method", name);
    break;
  default: {
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
  pushbool(ResourceLoader::instance()->fileExists(filename));
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
  Color *c = new Color
    (clamp_color(check_int(1)),
     clamp_color(check_int(2)),
     clamp_color(check_int(3)));
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
  const char *val = Registry::instance()->get(key);
  lua_pushstring(const_cast<char *>(val));
}

static void WriteRegistryValue() {
  char *key = luaL_check_string(1);
  char *val = luaL_check_string(2);
  Registry::instance()->set(key, val);
}

// Localization function

static void LocalizeString() {
  char *str = luaL_check_string(1);
  std::string result = Localizer::instance()->localize(str);
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
  Engine::instance()->setSelectedActor(act);
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

static void SetActorTalkChore() {
  Actor *act = check_actor(1);
  int index = check_int(2);
  int chore = check_int(3);
  Costume *costume = get_costume(act, 4, "setActorTalkChore");

  if (!costume)
    return;
  costume->setTalkChore(index, chore);
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

static void WalkActorTo() {
  Actor *act = check_actor(1);
  double x = luaL_check_number(2);
  double y = luaL_check_number(3);
  double z = luaL_check_number(4);
  act->walkTo(Vector3d(x, y, z));
}

static void IsActorMoving() {
  Actor *act = check_actor(1);
  pushbool(act->isWalking());
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

static void GetVisibleThings() {
  lua_Object result = lua_createtable();
  Actor *sel = Engine::instance()->selectedActor();
  for (Engine::actor_list_type::const_iterator i =
	 Engine::instance()->actorsBegin();
       i != Engine::instance()->actorsEnd(); i++) {
    if (! (*i)->inSet(Engine::instance()->sceneName()))
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
  if (lua_getparam(1) == LUA_NOOBJECT)
    pushbool(Mixer::instance()->voicePlaying());
  else {
    act = check_actor(1);
    pushbool(act->talking());
  }
}

static void ShutUpActor() {
 Actor *act = check_actor(1);
 if (act)
  act->shutUp();
}

// Sector functions
static void GetActorSector(void) {
  Actor *act = check_actor(1);
  int sectorType = check_int(2);

  int sectorFlag;
  switch (sectorType) {
  case 1:
    sectorFlag = 0x1000;
    break;
  case 2:
    sectorFlag = 0x2000;
    break;
  case 3:
    sectorFlag = 0x8000;
    break;
  default:
    error("Invalid sector type %d\n", sectorType);
  }

  int numSectors = Engine::instance()->currScene()->getSectorCount();
  for (int i = 0; i < numSectors; i++) {
    Sector *sector = Engine::instance()->currScene()->getSectorBase(i);
    if (sector->visible() && (sector->type() & sectorFlag)) {
      if (sector->isPointInSector(act->pos())) {
	lua_pushnumber(sector->id());
	lua_pushstring(const_cast<char *>(sector->name()));
	lua_pushnumber(sector->type());
	return;
      }
    }
  }

  lua_pushnil();
}

static void IsActorInSector(void) {
  Actor *act = check_actor(1);
  const char *name = luaL_check_string(2);
  int i, numSectors = Engine::instance()->currScene()->getSectorCount();

  for (i=0; i<numSectors; i++) {
   Sector *sector = Engine::instance()->currScene()->getSectorBase(i);

   if (sector->visible() && strstr(sector->name(), name)) {
    if (sector->isPointInSector(act->pos())) {
      lua_pushnumber(sector->id());
      lua_pushstring((char*)sector->name());
      lua_pushnumber(sector->type());
    }
   }
  } 

  lua_pushnil();
}

// Scene functions

static void MakeCurrentSet() {
  const char *name = luaL_check_string(1);
  Engine::instance()->setScene(name);
}

static void MakeCurrentSetup() {
  int num = check_int(1);
  int prevSetup = Engine::instance()->currScene()->setup();

  Engine::instance()->currScene()->setSetup(num);

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
  if (std::strcmp(name, Engine::instance()->sceneName()) == 0)
    lua_pushnumber(Engine::instance()->currScene()->setup());
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
  warning("Stub function GetShrinkPos(%g,%g,%g,%g) called",
	  x, y, z, r);
}

// Sound functions

enum ImuseParam {
  IM_SOUND_PLAY_COUNT = 256,
  IM_SOUND_PEND_COUNT = 512,
  IM_SOUND_GROUP = 1024,
  IM_SOUND_PRIORITY = 1280,
  IM_SOUND_VOL = 1536,
  IM_SOUND_PAN = 1792
};

void ImStartSound() {
  const char *name = luaL_check_string(1);
  Sound *s = ResourceLoader::instance()->loadSound(name);
  if (s != NULL) {
    Mixer::instance()->playSfx(s);
    s->luaRef();
    lua_pushusertag(s, sound_tag);
  }
  else
    lua_pushnil();
}

void gc_Sound() {
  Sound *s = check_sound(1);
  s->luaGc();
}

void ImStopSound() {
  Sound *s;
  if (lua_isstring(lua_getparam(1))) {
    s = Mixer::instance()->findSfx(lua_getstring(lua_getparam(1)));
    if (s == NULL)
      return;
  }
  else
    s = check_sound(1);
  Mixer::instance()->stopSfx(s);
}

void ImGetParam() {
  int param = check_int(2);
  switch (param) {
  case IM_SOUND_PLAY_COUNT:
    if (lua_isstring(lua_getparam(1))) {
      Sound *s = Mixer::instance()->findSfx(lua_getstring(lua_getparam(1)));
      if (s != NULL)
	lua_pushnumber(1);
      else
	lua_pushnumber(0);
    }
    else {
      Sound *s = check_sound(1);
      if (s->done())
	lua_pushnumber(0);
      else
	lua_pushnumber(1);
    }
    break;
  case IM_SOUND_VOL:
    lua_pushnumber(127);
    break;
  default:
    warning("Unimplemented ImGetParam with %d\n", param);
    lua_pushnil();
  }
}

void ImSetState() {
  int state = check_int(1);
  Mixer::instance()->setImuseState(state);
}

void ImSetSequence() {
  int seq = check_int(1);
  Mixer::instance()->setImuseSeq(seq);
}

// Timing functions
void set_frameTime(float frameTime) {
  lua_pushobject(lua_getglobal("system"));
  lua_pushstring("frameTime");
  lua_pushnumber(frameTime);
  lua_settable();
}

void PerSecond() {
  float rate = luaL_check_number(1);
  lua_pushnumber(Engine::instance()->perSecond(rate));
}

// Game control functions
void EnableControl() {
  int num = check_control(1);
  Engine::instance()->enableControl(num);
}

void DisableControl() {
  int num = check_control(1);
  Engine::instance()->disableControl(num);
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

// Text functions
static void MakeTextObject() {
 char *line = lua_getstring(lua_getparam(1)), *key_text = NULL;
 lua_Object table_obj = lua_getparam(2), key;
 int x = 0, y = 0, height = 0, width = 0;
 Color *fgColor = NULL;
 TextObject *textObject;

 while(1) {
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
	width  = atoi(lua_getstring(lua_getresult(2)));
   else
	error("Unknown MakeTextObject key %s\n", key_text);
 }

 textObject = new TextObject((const char *)line, x, y, *fgColor);
}

static void KillTextObject() {
 char *line;

 if (lua_isnil(lua_getparam(1))) { // FIXME: check this.. nil is kill all lines?
  Engine::instance()->killTextObjects();
  return;
 }

  line = lua_getstring(lua_getparam(1));

 error("killTextObject(%s)", line);
 for (Engine::text_list_type::const_iterator i = Engine::instance()->textsBegin();
      i != Engine::instance()->textsEnd(); i++) {
   TextObject *textO = *i;
   const char *name = textO->name();
   if (name==NULL)
    error("Text object with null name!");

   warning("Comparing **%s** to **%s**", line, name);
   if (strstr(name, line)) {
     error("Wanting to destroy text object %s ", line);
     break;
   }
 }
}

static void ChangeTextObject() {
 char *line = lua_getstring(lua_getparam(1)), *key_text = NULL, *val_text = NULL;
 lua_Object table_obj = lua_getparam(2), key;

 // FIXME: Disable this for now, as it seems to go into an infinite loop
 // when using Don's computer?!?
 warning("STUB: ChangeTextObject(%s)", line);
 return;

 printf("STUB: ChangeTextObject(%s, ", line);

 while(1) {
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
   val_text=lua_getstring(lua_getresult(2));
   printf(" %s=%s ", key_text, val_text);
 }

 printf(")\n");
}

static void GetTextObjectDimensions() {
 warning("STUB: GetTextObjectDimensions()");
 lua_pushnumber(100);	// Dummy X
 lua_pushnumber(100);	// Dummy Y
}

// Stub function for builtin functions not yet implemented

static void stubWarning() {
  char *name = lua_getstring(lua_getparam(1));
  fprintf(stderr, "WARNING: Stub function %s(", name);
  for (int i = 2; ; i++) {
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
      }
      else if (lua_tag(lua_getparam(i)) == color_tag) {
	Color *c = check_color(i);
	fprintf(stderr, "<color #%02x%02x%02x>",
		c->red(), c->green(), c->blue());
      }
      else if (lua_tag(lua_getparam(i)) == sound_tag) {
	Sound *s = check_sound(i);
	fprintf(stderr, "<sound %s>", s->filename());
      }
      else
	fprintf(stderr, "<userdata %p>", lua_getuserdata(lua_getparam(i)));
    }
    else if (lua_isfunction(lua_getparam(i)))
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

static char *stubFuncs[] = {
  "RestoreIMuse",
  "SaveIMuse",
  "Is3DHardwareEnabled",
  "SetActorInvClipNode",
  "NukeResources",
  "UnShrinkBoxes",
  "ShrinkBoxes",
  "ResetTextures",
  "JustLoaded",
  "AttachToResources",
  "DetachFromResources",
  "GetTextSpeed",
  "SetTextSpeed",
  "GetSaveGameData",
  "SubmitSaveGameData",
  "BlastRect",
  "BlastImage",
  "FreeImage",
  "GetImage",
  "GetSaveGameImage",
  "ScreenShot",
  "TextFileGetLine",
  "TextFileGetLineCount",
  "IrisUp",
  "IrisDown",
  "FadeInChore",
  "FadeOutChore",
  "SetActorClipPlane",
  "SetActorClipActive",
  "SetActorCollisionScale",
  "SetActorCollisionMode",
  "FlushControls",
  "ActorToClean",
  "SendObjectToFront",
  "SendObjectToBack",
  "SetObjectType",
  "FreeObjectState",
  "NewObjectState",
  "SetActorShadowValid",
  "AddShadowPlane",
  "KillActorShadows",
  "SetActiveShadow",
  "SetActorShadowPoint",
  "SetActorShadowPlane",
  "ActivateActorShadow",
  "SetShadowColor",
  "GetSpeechMode",
  "SetSpeechMode",
  "Display",
  "CleanBuffer",
  "DimRegion",
  "DimScreen",
  "ForceRefresh",
  "RenderModeUser",
  "SetActorConstrain",
  "SetActorWalkDominate",
  "SetGamma",
  "ImSetVoiceEffect",
  "ImResume",
  "ImPause",
  "ImSetMusicVol",
  "ImGetMusicVol",
  "ImSetVoiceVol",
  "ImGetVoiceVol",
  "ImSetSfxVol",
  "ImGetSfxVol",
  "ImFadeParam",
  "ImSetParam",
  "ImStopAllSounds",
  "LightMgrSetChange",
  "LightMgrStartup",
  "SetLightIntensity",
  "SetLightPosition",
  "TurnLightOn",
  "SetAmbientLight",
  "GetAngleBetweenVectors",
  "TurnActorTo",
  "PointActorAt",
  "GetCameraLookVector",
  "SetCameraRoll",
  "SetCameraInterest",
  "GetCameraPosition",
  "RotateVector",
  "LoadCostume",
  "PrintActorCostumes",
  "SpewStartup",
  "PreRender",
  "MakeSectorActive",
  "GetSectorOppositeEdge",
  "FileFindDispose",
  "FileFindNext",
  "FileFindFirst",
  "SetSoundPosition",
  "IsSoundPlaying",
  "PlaySoundAt",
  "IsMoviePlaying",
  "PauseMovie",
  "StopMovie",
  "StartMovie",
  "IsFullscreenMoviePlaying",
  "StartFullscreenMovie",
  "PreviousSetup",
  "NextSetup",
  "UnLockSet",
  "LockSet",
  "LockFont",
  "EnableDebugKeys",
  "WorldToScreen",
  "IsActorResting",
  "CompleteActorChore",
  "SetActorRoll",
  "SetActorPitch",
  "GetPointSector",
  "IsPointInSector",
  "SetActorFrustrumCull",
  "SetActorFollowBoxes",
  "SetActorHead",
  "GetCameraActor",
  "GetActorLookRate",
  "SetActorLookRate",
  "ActorLookAt",
  "DriveActorTo",
  "WalkActorVector",
  "PutActorAtInterest",
  "SetActorReflection",
  "GetActorPuckVector",
  "GetActorRect",
  "SetActorMumblechore",
  "SetActorRestChore",
  "SetActorTurnChores",
  "SetActorWalkChore",
  "GetActorNodeLocation",
  "SetActorTimeScale",
  "GetActorTimeScale",
  "SetActorScale",
  "SetActorColormap",
  "Save",
  "Load",
  "SearchForFileOrSwapCDs",
  "EngineDisplay",
  "SetOffscreenTextPos",
  "SetEmergencyFont",
  "GetTextCharPosition",
  "GetTranslationMode",
  "SetTranslationMode",
  "ExpireText",
  "BlastText",
  "PrintLine",
  "SetSayLineDefaults",
  "PurgePrimitiveQueue",
  "KillPrimitive",
  "ChangePrimitive",
  "DrawRectangle",
  "DrawPolygon",
  "DrawLine",
  "pause_scripts",
  "unpause_scripts",
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

struct luaL_reg builtins[] = {
  { "dofile", new_dofile },
  { "PrintDebug", PrintDebug },
  { "PrintWarning", PrintWarning },
  { "FunctionName", FunctionName },
  { "CheckForFile", CheckForFile },
  { "MakeColor", MakeColor },
  { "GetColorComponents", GetColorComponents },
  { "ReadRegistryValue", ReadRegistryValue },
  { "WriteRegistryValue", WriteRegistryValue },
  { "LocalizeString", LocalizeString },
  { "LoadActor", LoadActor },
  { "SetSelectedActor", SetSelectedActor },
  { "SetActorTalkColor", SetActorTalkColor },
  { "GetActorTalkColor", GetActorTalkColor },
  { "SetActorVisibility", SetActorVisibility },
  { "PutActorAt", PutActorAt },
  { "GetActorPos", GetActorPos },
  { "SetActorRot", SetActorRot },
  { "GetActorRot", GetActorRot },
  { "IsActorTurning", IsActorTurning },
  { "GetAngleBetweenActors", GetAngleBetweenActors },
  { "GetActorYawToPoint", GetActorYawToPoint },
  { "PutActorInSet", PutActorInSet },
  { "SetActorWalkRate", SetActorWalkRate },
  { "GetActorWalkRate", GetActorWalkRate },
  { "SetActorTurnRate", SetActorTurnRate },
  { "WalkActorForward", WalkActorForward },
  { "WalkActorTo", WalkActorTo },
  { "IsActorMoving", IsActorMoving },
  { "TurnActor", TurnActor },
  { "PushActorCostume", PushActorCostume },
  { "SetActorTalkChore", SetActorTalkChore },
  { "SetActorCostume", SetActorCostume },
  { "GetActorCostume", GetActorCostume },
  { "PopActorCostume", PopActorCostume },
  { "GetActorCostumeDepth", GetActorCostumeDepth },
  { "PlayActorChore", PlayActorChore },
  { "PlayActorChoreLooping", PlayActorChoreLooping },
  { "SetActorChoreLooping", SetActorChoreLooping },
  { "StopActorChore", StopActorChore },
  { "IsActorChoring", IsActorChoring },
  { "GetVisibleThings", GetVisibleThings },
  { "SayLine", SayLine },
  { "IsMessageGoing", IsMessageGoing },
  { "GetActorSector", GetActorSector },
  { "IsActorInSector", IsActorInSector },
  { "MakeCurrentSet", MakeCurrentSet },
  { "MakeCurrentSetup", MakeCurrentSetup },
  { "GetCurrentSetup", GetCurrentSetup },
  { "GetShrinkPos", GetShrinkPos },
  { "ImStartSound", ImStartSound },
  { "ImStopSound", ImStopSound },
  { "ImGetParam", ImGetParam },
  { "ImSetState", ImSetState },
  { "ImSetSequence", ImSetSequence },
  { "PerSecond", PerSecond },
  { "EnableControl", EnableControl },
  { "DisableControl", DisableControl },
  { "GetControlState", GetControlState },
  { "InputDialog", InputDialog },
  { "ChangeTextObject", ChangeTextObject },
  { "GetTextObjectDimensions", GetTextObjectDimensions },
  { "MakeTextObject", MakeTextObject },
  { "KillTextObject", KillTextObject },
  { "ShutUpActor", ShutUpActor }
};

void register_lua() {
  // Create various LUA tags
  actor_tag = lua_newtag();
  color_tag = lua_newtag();
  sound_tag = lua_newtag();

  // Register GC methods
  lua_pushcfunction(gc_Color);
  lua_settagmethod(color_tag, "gc");
  lua_pushcfunction(gc_Sound);
  lua_settagmethod(sound_tag, "gc");

  // Register system table
  lua_Object system_table = lua_createtable();
  lua_pushobject(system_table);
  lua_setglobal("system");

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

  // Register new version of dofile, and other builtin functions
  luaL_openlib(builtins, sizeof(builtins) / sizeof(builtins[0]));

  // Register stubs
  for (unsigned i = 0; i < sizeof(stubFuncs) / sizeof(char *); i++) {
    lua_pushstring(stubFuncs[i]);
    lua_pushcclosure(stubWarning, 1);
    lua_setglobal(stubFuncs[i]);
  }

  // Register constants for box types
  lua_pushnumber(1);
  lua_setglobal("WALK");	// Set to this value by Grim Fandango
				// scripts anyway...
  lua_pushnumber(2);
  lua_setglobal("CAMERA");
  lua_pushnumber(3);
  lua_setglobal("HOT");
}

int bundle_dofile(const char *filename) {
  Block *b = ResourceLoader::instance()->getFileBlock(filename);
  if (b == NULL) {
    delete b;
    // Don't print warnings on Scripts\foo.lua,
    // d:\grimFandango\Scripts\foo.lua
    if (std::strstr(filename, "Scripts\\") == NULL)
      warning("Cannot find script %s\n", filename);
    return 2;
  }

  int result = lua_dobuffer(const_cast<char *>(b->data()), b->len(),
			    const_cast<char *>(filename));
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
  if (! lua_isfunction(handler)) {
    warning("Invalid event handler %s", name);
    return LUA_NOOBJECT;
  }
  return handler;
}
