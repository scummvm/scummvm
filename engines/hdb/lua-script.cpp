/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
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

#include "common/lua/lua.h"
#include "common/lua/lauxlib.h"
#include "common/lua/lualib.h"
#include "common/debug.h"

#include "hdb/lua-script.h"

namespace HDB {

struct ScriptPatch {
	const char *scriptName;
	const char *search;
	const char *replace;
} scriptPatches[] = {
	{"GLOBAL.LUA", "for i,npc in npcs do", "for i,npc in pairs(npcs) do"},
	{"GLOBAL.LUA", "setglobal( npcdef.codename..\"_init\", function() return NPC_Init( %npcdef ) end )", "_G[npcdef.codename .. \"_init\"] = function() return NPC_Init( npcdef ) end"},
	{"GLOBAL.LUA", "setglobal( npcdef.codename..\"_use\", function(x, y, v1, v2) return NPC_Use( %npcdef, x, y, v1, v2 ) end )", "_G[npcdef.codename .. \"_use\"] = function(x, y, v1, v2) return NPC_Use( npcdef, x, y, v1, v2 ) end"},
	{"GLOBAL.LUA", "if( dtable.counter < getn(dtable) ) then", "if( dtable.counter < #dtable ) then"},
	{"GLOBAL.LUA", "if( getglobal( \"map\"..tostring(v1)..\"_complete\" ) ) then", "if( _G[\"map\"..tostring(v1)..\"_complete\"] ) then"},
	{"GLOBAL.LUA", "closefunc = getglobal( npcdef.codename..\"_use\" )", "closefunc = _G[npcdef.codename..\"_use\"]"},
	{"GLOBAL.LUA", "strsub(", "string.sub("}, // line 15
	{"GLOBAL.LUA", "for i,v in globals() do", "for i,v in pairs(_G) do"},
	{"GLOBAL.LUA", "return gsub( s, \"\\n\", \"\\\\\\n\" )", "return string.gsub( s, \"\\n\", \"\\\\\\n\" )"},
	{"GLOBAL.LUA", "for i,v in t do", "for i,v in pairs(t) do"},
	{"GLOBAL.LUA", "if type(v) == 'userdata' or type(v) == 'function' then return end", "if type(v) == 'userdata' or type(v) == 'function' or i == 'package' then return end" }, // Line 16
	{"GLOBAL.LUA", "for npcname,npcdata in npcs do", "for npcname,npcdata in pairs(npcs) do"}, // Line 66
	{"GLOBAL.LUA", "for dlgname,dlgdata in npcdata.dialog do", "for dlgname,dlgdata in pairs(npcdata.dialog) do"}, // Line 67
	{"GLOBAL.LUA", "s = format( \"npcs.%s.dialog.%s.counter", "s = string.format( \"npcs.%s.dialog.%s.counter"}, // Line 68
	{"GLOBAL.LUA", "s = format( \"npcs.%s.dialog.%s.finished = ", "s = string.format( \"npcs.%s.dialog.%s.finished = "}, // line 70
	{"MAP00.LUA", "tempfunc = function() emptybed_use( %x, %y, %v1, %v2 ) end", "tempfunc = function() emptybed_use(x, y, v1, v2) end"},
	{"MAP00.LUA", "if( getn( beds ) == 0 ) then", "if( #beds == 0 ) then"},
	{"MAP01.LUA", "if( covert_index < getn(covert_dialog) ) then", "if( covert_index < #covert_dialog ) then"},
	{"MAP01.LUA", "if( chiste_index < getn( chiste_dialog ) )then", "if( covert_index < #covert_dialog ) then"},
	{"MAP01.LUA", "strsub(", "string.sub("}, // line 23
	{NULL, NULL, NULL},
};

LuaScript::LuaScript() {
	_state = NULL;
	_systemInit = false;
}

LuaScript::~LuaScript() {
	if (_state) {
		lua_close(_state);
	}
}

bool LuaScript::init() {
	// Load Global Lua Code
	_globalLuaStream = g_hdb->_fileMan->findFirstData("GLOBAL.LUA", TYPE_BINARY);
	_globalLuaLength = g_hdb->_fileMan->getLength("GLOBAL.LUA", TYPE_BINARY);
	if (_globalLuaStream == NULL || _globalLuaLength == 0) {
		error("LuaScript::initScript: 'global code' failed to load");
		return false;
	}

	return true;
}

bool LuaScript::loadLua(const char *name) {
	Common::SeekableReadStream *luaStream = g_hdb->_fileMan->findFirstData(name, TYPE_BINARY);
	int32 luaLength = g_hdb->_fileMan->getLength(name, TYPE_BINARY);
	if (luaStream == NULL) {
		warning("The %s MPC entry can't be found", name);

		return false;
	}

	initScript(luaStream, name, luaLength);

	return true;
}

void LuaScript::saveGlobalNumber(const char *global, double value) {
	// see if global already exists; if so, overwrite it.
	for (uint i = 0; i < _globals.size(); i++) {
		if (!scumm_stricmp(global, _globals[i]->global)) {
			_globals[i]->valueOrString = 0;
			_globals[i]->value = value;
			return;
		}
	}

	Global *g = new Global;
	strcpy(g->global, global);
	g->valueOrString = 0;
	g->value = value;

	_globals.push_back(g);
}

void LuaScript::saveGlobalString(const char *global, const char *string) {
	if (!string)
		return;

	// see if global already exists; if so, overwrite it.
	for (uint i = 0; i < _globals.size(); i++) {
		if (!scumm_stricmp(global, _globals[i]->global)) {
			_globals[i]->valueOrString = 1;
			strcpy(_globals[i]->string, string);
			return;
		}
	}

	Global *g = new Global;
	strcpy(g->global, global);
	g->valueOrString = 1;
	strcpy(g->string, string);

	_globals.push_back(g);
}

void LuaScript::loadGlobal(const char *global) {
	for (uint i = 0; i < _globals.size(); i++) {
		if (!scumm_stricmp(global, _globals[i]->global)) {
			if (_globals[i]->valueOrString) {
				lua_pushstring(_state, _globals[i]->string);
				lua_setglobal(_state, _globals[i]->global);
			} else {
				lua_pushnumber(_state, _globals[i]->value);
				lua_setglobal(_state, _globals[i]->global);
			}
			return;
		}
	}
}

void LuaScript::purgeGlobals() {
	_globals.clear();
}

void LuaScript::save(Common::OutSaveFile *out, const char *targetName, int slot) {
	out->writeUint32LE(_globals.size());

	// Save Globals
	for (uint i = 0; i < _globals.size(); i++) {
		out->write(_globals[i]->global, 32);
		out->writeSint32LE(_globals[i]->valueOrString);
		out->writeDoubleLE(_globals[i]->value);
		out->write(_globals[i]->string, 32);
	}

	Common::String saveLuaName = Common::String::format("%s.l.%03d", targetName, slot);
	lua_printstack(_state);
	lua_getglobal(_state, "SaveState");

	lua_pushstring(_state, saveLuaName.c_str());
	lua_call(_state, 1, 0);
}

void LuaScript::loadSaveFile(Common::InSaveFile *in, const char *fName) {
	// Clear out all globals
	_globals.clear();

	// Start reading globals
	_globals.resize(in->readUint32LE());
	for (uint i = 0; i < _globals.size(); i++) {
		in->read(_globals[i]->global, 32);
		_globals[i]->valueOrString = in->readSint32LE();
		_globals[i]->value = in->readDoubleLE();
		in->read(_globals[i]->string, 32);
	}

	// Error handling function to be executed after the function is put on the stack
	lua_rawgeti(_state, LUA_REGISTRYINDEX, _pcallErrorhandlerRegistryIndex);
	lua_insert(_state, -2);

	lua_getglobal(_state, "LoadState");
	lua_pushstring(_state, fName);

	lua_call(_state, 1, 0);
}

void LuaScript::setLuaGlobalValue(const char *name, int value) {
	if (!_state)
		return;

	lua_pushnumber(_state, value);
	lua_setglobal(_state, name);
}

/*
	Called from Lua, this will pop into the menu
*/

static int cineStart(lua_State *L) {
	double abortable = lua_tonumber(L, 1);
	const char *abortFunc = lua_tostring(L, 2);

	g_hdb->_lua->checkParameters("cineStart", 2);

	lua_pop(L, 2);

	g_hdb->_ai->cineStart((bool)abortable, abortFunc);

	return 0;
}

static int cineStop(lua_State *L) {
	const char *funcNext = NULL;

	int stackTop = lua_gettop(L);
	if (stackTop) {
		funcNext = lua_tostring(L, 1);
		lua_pop(L, 1);
	}

	g_hdb->_ai->cineStop(funcNext);
	return 0;
}

static int cineFadeInBlack(lua_State *L) {
	double steps = lua_tonumber(L, 1);

	g_hdb->_lua->checkParameters("cineFadeInBlack", 1);

	lua_pop(L, 1);

	g_hdb->_ai->cineFadeIn(false, (int) steps);

	return 0;
}

static int cineFadeOutBlack(lua_State *L) {
	double steps = lua_tonumber(L, 1);

	g_hdb->_lua->checkParameters("cineFadeOutBlack", 1);

	lua_pop(L, 1);

	g_hdb->_ai->cineFadeOut(false, (int)steps);

	return 0;
}

static int cineFadeInWhite(lua_State *L) {
	double steps = lua_tonumber(L, 1);

	g_hdb->_lua->checkParameters("cineFadeInWhite", 1);

	lua_pop(L, 1);

	g_hdb->_ai->cineFadeIn(true, (int)steps);

	return 0;
}

static int cineFadeOutWhite(lua_State *L) {
	double steps = lua_tonumber(L, 1);

	g_hdb->_lua->checkParameters("cineFadeOutWhite", 1);

	lua_pop(L, 1);

	g_hdb->_ai->cineFadeOut(true, (int)steps);

	return 0;
}

static int cineStartMap(lua_State *L) {
	const char *mapName = lua_tostring(L, 1);

	g_hdb->_lua->checkParameters("cineStartMap", 1);

	lua_pop(L, 1);

	g_hdb->_ai->cineStartMap(mapName);

	return 0;
}

static int cineLockPlayer(lua_State *L) {
	g_hdb->_ai->cineLockPlayer();
	return 0;
}

static int cineUnlockPlayer(lua_State *L) {
	g_hdb->_ai->cineUnlockPlayer();
	return 0;
}

static int cineSetCamera(lua_State *L) {
	double x = lua_tonumber(L, 1);
	double y = lua_tonumber(L, 2);

	g_hdb->_lua->checkParameters("cineSetCamera", 2);

	lua_pop(L, 2);

	g_hdb->_ai->cineSetCamera((int) x, (int) y);

	return 0;
}

static int cineResetCamera(lua_State *L) {
	g_hdb->_ai->cineResetCamera();
	return 0;
}

static int cineMoveCamera(lua_State *L) {
	double x = lua_tonumber(L, 1);
	double y = lua_tonumber(L, 2);
	double speed = lua_tonumber(L, 3);

	g_hdb->_lua->checkParameters("cineMoveCamera", 3);

	lua_pop(L, 3);
	g_hdb->_ai->cineMoveCamera((int) x, (int) y, (int) speed);

	return 0;
}

static int cineWait(lua_State *L) {
	double seconds = lua_tonumber(L, 1);

	g_hdb->_lua->checkParameters("cineWait", 1);

	lua_pop(L, 1);
	g_hdb->_ai->cineWait((int) seconds);
	return 0;
}

static int cineWaitUntilDone(lua_State *L) {
	g_hdb->_ai->cineWaitUntilDone();
	return 0;
}

static int cinePlaySound(lua_State *L) {
	double index = lua_tonumber(L, 1);

	g_hdb->_lua->checkParameters("cinePlaySound", 1);

	lua_pop(L, 1);

	g_hdb->_ai->cinePlaySound((int)index);
	return 0;
}

static int cinePlayVoice(lua_State *L) {
	double	index = lua_tonumber(L, 1);
	double	actor = lua_tonumber(L, 2);

	g_hdb->_lua->checkParameters("cinePlayVoice", 2);

	lua_pop(L, 2);

	g_hdb->_ai->cinePlayVoice((int)index, (int)actor);
	return 0;
}

static int cineUseEntity(lua_State *L) {
	const char *string = lua_tostring(L, 1);

	g_hdb->_lua->checkParameters("cineUseEntity", 1);

	lua_pop(L, 1);
	g_hdb->_ai->cineUse(string);
	return 0;
}

static int cineSetEntity(lua_State *L) {
	const char *entName = lua_tostring(L, 1);
	double x = lua_tonumber(L, 2);
	double y = lua_tonumber(L, 3);
	double level = lua_tonumber(L, 4);

	g_hdb->_lua->checkParameters("cineSetEntity", 4);

	lua_pop(L, 4);
	g_hdb->_ai->cineSetEntity(entName, (int)x, (int)y, (int)level);
	return 0;
}

static int cineRemoveEntity(lua_State *L) {
	warning("STUB: CINE REMOVE ENTITY");
	return 0;
}

static int cineMoveEntity(lua_State *L) {
	const char *entName = lua_tostring(L, 1);
	double x = lua_tonumber(L, 2);
	double y = lua_tonumber(L, 3);
	double level = lua_tonumber(L, 4);
	double speed = lua_tonumber(L, 5);

	g_hdb->_lua->checkParameters("cineMoveEntity", 5);

	lua_pop(L, 5);
	g_hdb->_ai->cineMoveEntity(entName, (int)x, (int)y, (int)level, (int)speed);
	return 0;
}

static int cineAnimEntity(lua_State *L) {
	const char *entName = lua_tostring(L, 1);
	double state = lua_tonumber(L, 2);
	double loop = lua_tonumber(L, 3);

	g_hdb->_lua->checkParameters("cineAnimEntity", 3);

	lua_pop(L, 3);
	int s = (int)state;
	g_hdb->_ai->cineAnimEntity(entName, (AIState)s, (int)loop);
	return 0;
}

static int cineSetAnimFrame(lua_State *L) {
	const char *entName = lua_tostring(L, 1);
	double state = lua_tonumber(L, 2);
	double frame = lua_tonumber(L, 3);

	g_hdb->_lua->checkParameters("cineSetAnimFrame", 3);

	lua_pop(L, 3);
	int s = (int)state;
	g_hdb->_ai->cineSetAnimFrame(entName, (AIState)s, (int)frame);
	return 0;
}

static int cineDialog(lua_State *L) {
	const char *title = lua_tostring(L, 1);
	const char *string = lua_tostring(L, 2);
	double seconds = lua_tonumber(L, 3);

	g_hdb->_lua->checkParameters("cineDialog", 3);

	lua_pop(L, 3);
	g_hdb->_ai->cineDialog(title, string, (int)seconds);
	return 0;
}

static int cineDrawPic(lua_State *L) {
	warning("STUB: CINE DRAW PIC");
	return 0;
}

static int cineDrawMaskedPic(lua_State *L) {
	warning("STUB: CINE DRAW MASKED PIC");
	return 0;
}

static int cineMovePic(lua_State *L) {
	warning("STUB: CINE MOVE PIC");
	return 0;
}

static int cineMoveMaskedPic(lua_State *L) {
	const char *id = lua_tostring(L, 1);
	const char *pic = lua_tostring(L, 2);
	double	x1 = lua_tonumber(L, 3);
	double	y1 = lua_tonumber(L, 4);
	double	x2 = lua_tonumber(L, 5);
	double	y2 = lua_tonumber(L, 6);
	double	speed = lua_tonumber(L, 7);

	g_hdb->_lua->checkParameters("cineMoveMaskedPic", 7);

	x1 += kCameraXOff;
	y1 += kCameraYOff;
	x2 += kCameraXOff;
	y2 += kCameraYOff;

	lua_pop(L, 7);

	g_hdb->_ai->cineMoveMaskedPic(id, pic, (int)x1, (int)y1, (int)x2, (int)y2, (int)speed);

	return 0;
}

static int cineSpawnEntity(lua_State *L) {
	double type = lua_tonumber( L, 1 );
	double dir = lua_tonumber( L, 2 );
	double x = lua_tonumber( L, 3 );
	double y = lua_tonumber( L, 4 );
	const char *func_init = lua_tostring( L, 5 );
	const char *func_action = lua_tostring( L, 6 );
	const char *func_use = lua_tostring( L, 7 );
	double dir2 =  lua_tonumber( L, 8 );
	double level =  lua_tonumber( L, 9 );
	double value1 = lua_tonumber( L, 10 );
	double value2 = lua_tonumber( L, 11 );

	int t = (int) type;
	int d = (int) dir;
	int d2 = (int) dir2;

	g_hdb->_lua->checkParameters("Cine_SpawnEntity", 11);

	lua_pop(L, 11);
	g_hdb->_ai->cineSpawnEntity((AIType)t, (AIDir)d, (int)x, (int)y, func_init, func_action, func_use,
			(AIDir)d2, (int)level, (int)value1, (int)value2);
	return 0;
}

static int cineClearForeground(lua_State *L) {
	double x = lua_tonumber(L, 1);
	double y = lua_tonumber(L, 2);

	g_hdb->_lua->checkParameters("cineClearForeground", 2);

	lua_pop(L, 2);
	g_hdb->_ai->cineClearForeground((int)x, (int)y);
	return 0;
}

static int cineSetForeground(lua_State *L) {
	double x = lua_tonumber(L, 1);
	double y = lua_tonumber(L, 2);
	const char *tileName = lua_tostring(L, 3);

	g_hdb->_lua->checkParameters("cineSetForeground", 3);

	lua_pop(L, 3);
	int index = g_hdb->_gfx->getTileIndex(tileName);
	g_hdb->_gfx->getTile(index);
	g_hdb->_ai->cineSetForeground((int)x, (int)y, index);
	return 0;
}

static int cineSetBackground(lua_State *L) {
	double x = lua_tonumber(L, 1);
	double y = lua_tonumber(L, 2);
	const char *tileName = lua_tostring(L, 3);

	g_hdb->_lua->checkParameters("cineSetBackground", 3);

	lua_pop(L, 3);
	int index = g_hdb->_gfx->getTileIndex(tileName);
	g_hdb->_gfx->getTile(index);
	g_hdb->_ai->cineSetBackground((int)x, (int)y, index);
	return 0;
}

static int cineFunction(lua_State *L) {
	warning("STUB: CINE FUNCTION");
	return 0;
}

static int cineEntityFace(lua_State *L) {
	const char *luaName = lua_tostring(L, 1);
	double dir = lua_tonumber(L, 2);

	g_hdb->_lua->checkParameters("CineEntityFace", 2);

	lua_pop(L, 2);
	g_hdb->_ai->cineEntityFace(luaName, dir);
	return 0;
}

static int cineTextOut(lua_State *L) {
	const char *string = lua_tostring(L, 1);
	double x = lua_tonumber(L, 2);
	double y = lua_tonumber(L, 3);
	double timer = lua_tonumber(L, 4);

	g_hdb->_lua->checkParameters("cineTextOut", 4);

	x += kCameraXOff;
	y += kCameraYOff;

	lua_pop(L, 4);
	g_hdb->_ai->cineTextOut(string, (int)x, (int)y, (int)timer);
	return 0;
}

static int cineCenterTextOut(lua_State *L) {
	const char *string = lua_tostring(L, 1);
	double y = lua_tonumber(L, 2);
	double timer = lua_tonumber(L, 3);

	g_hdb->_lua->checkParameters("cineCenterTextOut", 3);

	y += kCameraYOff;

	lua_pop(L, 3);
	g_hdb->_ai->cineCenterTextOut(string, (int)y, (int)timer);
	return 0;
}

static int newDelivery(lua_State *L) {
	const char *itemTextName, *itemGfxName;
	const char *destTextName, *destGfxName, *id;

	itemTextName = lua_tostring(L, 1);
	itemGfxName = lua_tostring(L, 2);
	destTextName = lua_tostring(L, 3);
	destGfxName = lua_tostring(L, 4);
	id = lua_tostring(L, 5);

	g_hdb->_lua->checkParameters("newDelivery", 5);

	lua_pop(L, 5);

	g_hdb->_ai->newDelivery(itemTextName, itemGfxName, destTextName, destGfxName, id);

	return 0;
}

static int completeDelivery(lua_State *L) {
	const char *id = lua_tostring(L, 1);
	double rtn = g_hdb->_ai->completeDelivery(id);

	g_hdb->_lua->checkParameters("completeDelivery", 1);

	lua_pop(L, 1);
	lua_pushnumber(L, rtn);
	return 1;
}

static int deliveriesLeft(lua_State *L) {
	double	value = (double)g_hdb->_ai->getDeliveriesAmount();
	lua_pushnumber(L, value);
	return 1;
}

static int getEntityXY(lua_State *L) {
	int x, y;
	const char *initName = lua_tostring(L, 1);

	g_hdb->_lua->checkParameters("getEntityXY", 1);

	lua_pop(L, 1);

	g_hdb->_ai->getEntityXY(initName, &x, &y);

	lua_pushnumber(L, x);
	lua_pushnumber(L, y);
	return 2;
}

static int setEntity(lua_State *L) {
	AIEntity *e = NULL;

	const char *entName = lua_tostring(L, 1);
	double x = lua_tonumber(L, 2);
	double y = lua_tonumber(L, 3);
	double level = lua_tonumber(L, 4);

	g_hdb->_lua->checkParameters("setEntity", 4);

	lua_pop(L, 4);
	e = g_hdb->_ai->locateEntity(entName);
	if (e) {
		e->x = (int)x * kTileWidth;
		e->tileX = (int)x;
		e->y = (int)y * kTileHeight;
		e->tileY = (int)y;
		e->level = (int)level;
		e->goalX = e->goalY = e->xVel = e->yVel = 0;
		e->state = STATE_STANDDOWN;
	} else {
		warning("Couldn't SetEntity on '%s'", entName);
	}
	return 0;
}

static int setEntDir(lua_State *L) {
	AIEntity *e;
	char buff[64];

	const char *entName = lua_tostring(L, 1);
	double d = lua_tonumber(L, 2);

	g_hdb->_lua->checkParameters("setEntDir", 2);

	lua_pop(L, 2);
	e = g_hdb->_ai->locateEntity(entName);

	if (e) {
		int	dd = (int)d;
		e->dir = (AIDir)dd;
	} else {
		sprintf(buff, "Could not SetEntDir on '%s'", entName);
		g_hdb->_window->openMessageBar(buff, 10);
	}

	return 0;
}

static int removeEntity(lua_State *L) {
	const char *entName = lua_tostring(L, 1);

	g_hdb->_lua->checkParameters("removeEntity", 1);

	lua_pop(L, 1);

	g_hdb->_ai->removeLuaEntity(entName);

	return 0;
}

static int animEntity(lua_State *L) {
	const char *entName = lua_tostring(L, 1);
	double state = lua_tonumber(L, 2);

	g_hdb->_lua->checkParameters("animEntity", 2);

	lua_pop(L, 2);
	int s = (int)state;
	g_hdb->_ai->animLuaEntity(entName, (AIState)s);
	return 0;
}

static int setAnimFrame(lua_State *L) {
	const char *entName = lua_tostring(L, 1);
	double	state = lua_tonumber(L, 2);
	double	frame = lua_tonumber(L, 3);

	g_hdb->_lua->checkParameters("setAnimFrame", 3);

	lua_pop(L, 3);

	int	s = (int)state;
	g_hdb->_ai->setLuaAnimFrame(entName, (AIState)s, (int)frame);

	return 0;
}

static int useEntity(lua_State *L) {
	const char *initName = lua_tostring(L, 1);

	g_hdb->_lua->checkParameters("useEntity", 1);

	lua_pop(L, 1);

	g_hdb->_ai->useLuaEntity(initName);

	return 0;
}

static int entityFace(lua_State *L) {
	const char *initName = lua_tostring(L, 1);
	double dir = lua_tonumber(L, 2);

	g_hdb->_lua->checkParameters("entityFace", 2);

	lua_pop(L, 2);

	g_hdb->_ai->entityFace(initName, (int)dir);

	return 0;
}

static int clearForeground(lua_State *L) {
	double x = lua_tonumber(L, 1);
	double y = lua_tonumber(L, 2);

	g_hdb->_lua->checkParameters("clearForegnd", 2);

	lua_pop(L, 2);

	g_hdb->_map->setMapFGTileIndex((int)x, (int)y, -1);
	g_hdb->_map->removeFGTileAnimation((int)x, (int)y);

	return 0;
}

static int setForeground(lua_State *L) {
	double x = lua_tonumber(L, 1);
	double y = lua_tonumber(L, 2);
	const char *tileName = lua_tostring(L, 3);

	g_hdb->_lua->checkParameters("setForeground", 3);

	lua_pop(L, 3);
	int index = g_hdb->_gfx->getTileIndex(tileName);
	g_hdb->_gfx->getTile(index);
	g_hdb->_map->setMapFGTileIndex((int)x, (int)y, index);
	g_hdb->_map->addFGTileAnimation((int)x, (int)y);
	return 0;
}

static int setBackground(lua_State *L) {
	double x = lua_tonumber(L, 1);
	double y = lua_tonumber(L, 2);
	const char *tileName = lua_tostring(L, 3);

	g_hdb->_lua->checkParameters("setBackground", 3);

	lua_pop(L, 3);
	int index = g_hdb->_gfx->getTileIndex(tileName);
	g_hdb->_gfx->getTile(index);
	g_hdb->_map->setMapBGTileIndex((int)x, (int)y, index);
	g_hdb->_map->addBGTileAnimation((int)x, (int)y);
	return 0;
}

static int dialog(lua_State *L) {
	const char *title, *string, *more;
	double tileIndex;

	title = lua_tostring(L, 1);
	tileIndex = lua_tonumber(L, 2);
	string = lua_tostring(L, 3);
	more = lua_tostring(L, 4);

	if (!more || more[0] == '0')
		more = NULL;

	g_hdb->_lua->checkParameters("dialog", 4);

	lua_pop(L, 4);
	if (string)
		g_hdb->_window->openDialog(title, (int)tileIndex, string, more ? 1 : 0, more);
	return 0;
}

static int dialogChoice(lua_State *L) {
	const char *title = lua_tostring(L, 1);
	const char *text = lua_tostring(L, 2);
	const char *func = lua_tostring(L, 3);
	const char *choice[10] = {0,0,0,0,0,0,0,0,0,0};

	int	i, amount = lua_gettop(L) - 3;
	if (amount > 9)
		amount = 9;

	for (i = 0; i < amount; i++)
		choice[i] = lua_tostring(L, 4 + i);

	lua_pop(L, amount + 3);

	g_hdb->_window->openDialogChoice(title, text, func, amount, &choice[0]);
	return 0;
}

static int message(lua_State *L) {
	const char *title;
	double	delay;

	title = lua_tostring(L, 1);
	delay = lua_tonumber(L, 2);


	g_hdb->_lua->checkParameters("message", 2);

	lua_pop(L, 2);

	g_hdb->_window->openMessageBar(title, (int)delay);
	return 0;
}

static int animation(lua_State *L) {
	double	x = lua_tonumber(L, 1);
	double	y = lua_tonumber(L, 2);
	double	which = lua_tonumber(L, 3);
	double	playsnd = lua_tonumber(L, 4);

	g_hdb->_lua->checkParameters("animation", 4);

	x *= kTileWidth;
	y *= kTileHeight;

	switch ((int)which) {
	case 0:
		g_hdb->_ai->addAnimateTarget((int)x, (int)y, 0, 3, ANIM_NORMAL, false, false, GROUP_WATER_SPLASH_SIT);
		if (playsnd)
			g_hdb->_sound->playSound(SND_SPLASH);
		break;
	case 1:
		g_hdb->_ai->addAnimateTarget((int)x, (int)y, 0, 3, ANIM_NORMAL, false, false, GROUP_EXPLOSION_BOOM_SIT);
		if (playsnd)
			g_hdb->_sound->playSound(SND_BARREL_EXPLODE);
		break;
	case 2:
		g_hdb->_ai->addAnimateTarget((int)x, (int)y, 0, 3, ANIM_FAST, false, false, GROUP_STEAM_PUFF_SIT);
		if (playsnd)
			g_hdb->_sound->playSound(SND_BARREL_MELTING);
		break;
	case 3:
		g_hdb->_ai->addAnimateTarget((int)x, (int)y, 0, 7, ANIM_NORMAL, false, false, TELEPORT_FLASH);
		if (playsnd)
			g_hdb->_sound->playSound(SND_TELEPORT);
		break;
	case 4:
		g_hdb->_ai->addAnimateTarget((int)x, (int)y, 0, 3, ANIM_NORMAL, false, false, GEM_FLASH);
		if (playsnd)
			g_hdb->_sound->playSound(SND_GET_GEM);
		break;
	case 5:
		g_hdb->_ai->addAnimateTarget((int)x, (int)y, 0, 2, ANIM_NORMAL, false, false, GROUP_ENT_CHICKEN_DIE);
		if (playsnd)
			g_hdb->_sound->playSound(SND_CHICKEN_DEATH);
		break;
	}

	lua_pop(L, 3);
	return 0;
}

static int spawnEntity(lua_State *L) {
	double type = lua_tonumber(L, 1);
	double	dir = lua_tonumber(L, 2);
	double	x = lua_tonumber(L, 3);
	double	y = lua_tonumber(L, 4);
	const char *funcInit = lua_tostring(L, 5);
	const char *funcAction = lua_tostring(L, 6);
	const char *funcUse = lua_tostring(L, 7);
	double	dir2 = lua_tonumber(L, 8);
	double	level = lua_tonumber(L, 9);
	double	value1 = lua_tonumber(L, 10);
	double	value2 = lua_tonumber(L, 11);

	int t = (int)type;
	int d = (int)dir;
	int d2 = (int)dir2;

	g_hdb->_lua->checkParameters("spawnEntity", 11);

	lua_pop(L, 11);

	g_hdb->_ai->spawn((AIType)t, (AIDir)d, (int)x, (int)y, funcInit, funcAction, funcUse, (AIDir)d2, (int)level, (int)value1, (int)value2, 1);
	return 0;
}

static int addInvItem(lua_State *L) {
	double	type = lua_tonumber(L, 1);
	double	amount = lua_tonumber(L, 2);
	const char *funcInit = lua_tostring(L, 3);
	const char *funcAction = lua_tostring(L, 4);
	const char *funcUse = lua_tostring(L, 5);

	int t = (int)type;

	g_hdb->_lua->checkParameters("addInvItem", 5);

	lua_pop(L, 5);

	bool rtn = g_hdb->_ai->addItemToInventory((AIType)t, (int)amount, funcInit, funcAction, funcUse);

	lua_pushnumber(L, rtn);
	return 1;
}

static int keepInvItem(lua_State *L) {
	double type = lua_tonumber(L, 1);
	int t = (int)type;

	g_hdb->_lua->checkParameters("keepInvItem", 1);

	lua_pop(L, 1);

	g_hdb->_ai->keepInvItem((AIType)t);
	return 0;
}

static int queryInv(lua_State *L) {
	const char *search;
	int	result;

	search = lua_tostring(L, 1);		// get the passed-in search string

	g_hdb->_lua->checkParameters("queryInv", 1);

	lua_pop(L, 1);

	result = g_hdb->_ai->queryInventory(search);			// call the function & get return value
	lua_pushnumber(L, result);					// send the return value back to Lua
	return 1;
}

static int purgeInv(lua_State *L) {
	g_hdb->_ai->purgeInventory();
	return 0;
}

static int queryInvItem(lua_State *L) {
	double	search;
	int	result, s1;

	search = lua_tonumber(L, 1);					// get the passed-in search string
	s1 = (int)search;

	g_hdb->_lua->checkParameters("queryInvItem", 1);

	lua_pop(L, 1);

	result = g_hdb->_ai->queryInventoryType((AIType)s1);	// call the function & get return value
	lua_pushnumber(L, result);					// send the return value back to Lua
	return 1;
}

static int removeInv(lua_State *L) {
	const char *search;
	int	result;

	search = lua_tostring(L, 1);		// get the passed-in search string
	double number = lua_tonumber(L, 2);

	g_hdb->_lua->checkParameters("removeInv", 2);

	lua_pop(L, 2);

	result = (int)g_hdb->_ai->removeInvItem(search, (int)number);	// call the function & get return value
	lua_pushnumber(L, result);					// send the return value back to Lua
	return 1;
}

static int removeInvItem(lua_State *L) {
	int	result;

	double	search = lua_tonumber(L, 1);		// get the passed-in type value
	double	number = lua_tonumber(L, 2);

	g_hdb->_lua->checkParameters("removeInvItem", 2);

	lua_pop(L, 2);

	int	s = (int)search;
	result = (int)g_hdb->_ai->removeInvItemType((AIType)s, (int)number);	// call the function & get return value
	lua_pushnumber(L, result);					// send the return value back to Lua
	return 1;
}

static int killTrigger(lua_State *L) {
	const char *id = lua_tostring(L, 1);

	g_hdb->_lua->checkParameters("killTrigger", 1);

	lua_pop(L, 1);
	g_hdb->_ai->killTrigger(id);
	return 0;
}

static int startMusic(lua_State *L) {
	warning("STUB: START MUSIC");
	return 0;
}

static int fadeInMusic(lua_State *L) {
	warning("STUB: FADE IN MUSIC");
	return 0;
}

static int stopMusic(lua_State *L) {
	warning("STUB: STOP MUSIC");
	return 0;
}

static int fadeOutMusic(lua_State *L) {
	warning("STUB: FADE OUT MUSIC");
	return 0;
}

static int registerSound(lua_State *L) {
	warning("STUB: REGISTER SOUND");
	return 0;
}

static int playSound(lua_State *L) {
	warning("STUB: PLAY SOUND");
	return 0;
}

static int freeSound(lua_State *L) {
	warning("STUB: FREE SOUND");
	return 0;
}

static int startMap(lua_State *L) {
	const char *string = lua_tostring(L, 1);

	g_hdb->_lua->checkParameters("startMap", 1);

	lua_pop(L, 1);
	g_hdb->changeLevel(string);

	return 0;
}

static int saveGlobal(lua_State *L) {
	const char *global = lua_tostring(L, 1);
	int type;

	g_hdb->_lua->checkParameters("saveGlobal", 1);

	lua_pop(L, 1);

	lua_getglobal(L, global);
	type = lua_type(L, 1);
	if (type == LUA_TNUMBER) {
		double value = lua_tonumber(L, 1);
		g_hdb->_lua->saveGlobalNumber(global, value);
	} else if (type == LUA_TSTRING) {
		const char *string = lua_tostring(L, 1);
		g_hdb->_lua->saveGlobalString(global, string);
	}

	return 0;
}

static int loadGlobal(lua_State *L) {
	const char *global = lua_tostring(L, 1);

	g_hdb->_lua->checkParameters("loadGlobal", 1);

	lua_pop(L, 1);

	g_hdb->_lua->loadGlobal(global);

	return 0;
}

static int purgeGlobals(lua_State *L) {
	g_hdb->_lua->purgeGlobals();
	return 0;
}

static int textOut(lua_State *L) {
	const char *string = lua_tostring(L, 1);
	double	x = lua_tonumber(L, 2);
	double	y = lua_tonumber(L, 3);
	double	timer = lua_tonumber(L, 4);

	g_hdb->_lua->checkParameters("textOut", 4);

	lua_pop(L, 4);
	g_hdb->_window->textOut(string, (int)x, (int)y, (int)timer);
	return 0;
}

static int centerTextOut(lua_State *L) {
	const char *string = lua_tostring(L, 1);
	double	y = lua_tonumber(L, 2);
	double	timer = lua_tonumber(L, 3);

	g_hdb->_lua->checkParameters("centerTextOut", 3);

	lua_pop(L, 3);
	g_hdb->_window->centerTextOut(string, (int)y, (int)timer);
	return 0;
}

static int turnOnSnow(lua_State *L) {
	g_hdb->_gfx->turnOnSnow();
	return 0;
}

static int turnOffSnow(lua_State *L) {
	g_hdb->_gfx->turnOffSnow();
	return 0;
}

static int gotoMenu(lua_State *L) {
	g_hdb->changeGameState();
	return 0;
}

static int setInfobarDark(lua_State *L) {
	double	value = lua_tonumber(L, 1);

	g_hdb->_lua->checkParameters("setInfobarDark", 1);

	lua_pop(L, 1);

	g_hdb->_window->setInfobarDark((int)value);

	return 0;
}

static int setPointerState(lua_State *L) {
	double	value = lua_tonumber(L, 1);

	g_hdb->_lua->checkParameters("setPointerState", 1);

	lua_pop(L, 1);

	g_hdb->_gfx->setPointerState((int)value);
	return 0;
}

static int playVoice(lua_State *L) {
	warning("STUB: PLAY VOICE");
	return 0;
}

static int openFile(lua_State *L) {

	const char *fName = lua_tostring(L, 1);
	const char *mode = lua_tostring(L, 2);

	g_hdb->_lua->checkParameters("openFile", 2);

	lua_pop(L, 2);

	if (!scumm_stricmp(mode, "wt")) {
		// Delete Lua Save File
		Common::InSaveFile *inLua = g_system->getSavefileManager()->openForLoading(fName);
		if (inLua)
			delete inLua;

		Common::OutSaveFile *outLua = g_system->getSavefileManager()->openForSaving(fName);
		if (!outLua)
			error("Cannot open %s", fName);
		lua_pushlightuserdata(L, outLua);
	}

	return 1;
}

static int write(lua_State *L) {
	Common::OutSaveFile *out = (Common::OutSaveFile *)lua_topointer(L, 1);
	const char *data = lua_tostring(L, 2);

	g_hdb->_lua->checkParameters("write", 2);

	lua_pop(L, 2);

	out->write(data, strlen(data));
	debugN(3, "%s", data);

	return 0;
}

static int closeFile(lua_State *L) {
	Common::OutSaveFile *out = (Common::OutSaveFile *)lua_topointer(L, 1);

	g_hdb->_lua->checkParameters("closeFile", 1);

	lua_pop(L, 1);

	out->finalize();

	delete out;

	return 0;
}

/*
	Lua Initialization Code
*/

struct VarInit {
	const char *realName;
	const char *luaName;
} luaGlobalStrings[] = {
	{ "Map00",	"MAP00"},
	{ TILE_TELEPORT1,					"TILE_TELEPORT1"	},
	{ TILE_SKY_STARS,					"TILE_SKY_STARS"	},
	{ TILE_HOLDER1,						"TILE_HOLDER1"		},
	{ TILE_HOLDERFULL,					"TILE_HOLDERFULL"	},
	{ TILE_SWITCH1,						"TILE_SWITCH1"		},
	{ TILE_1SWITCH1,					"TILE_1SWITCH1"		},
	{ TILE_DOOR_N,						"TILE_DOOR_N"		},
	{ TILE_DOOR_P,						"TILE_DOOR_P"		},
	{ TILE_DOOR_S,						"TILE_DOOR_S"		},
	{ TILE_ASKCOMP1,					"TILE_ASKCOMP1"		},
	{ TILE_SWITCH_ON,					"TILE_SWITCH_ON"	},
	{ TILE_1SWITCH_ON,					"TILE_1SWITCH_ON"	},
	{ TILE_FLOORSHOCK,					"TILE_FLOORSHOCK"	},
	{ TILE_DOOR_NV,						"TILE_DOOR_NV"		},
	{ TILE_DOOR_PV,						"TILE_DOOR_PV"		},
	{ TILE_DOOR_SV,						"TILE_DOOR_SV"		},
	{ TILE_MAILSORT,					"TILE_MAILSORT"		},
	{ TILE_BEDR_OPEN,					"TILE_BEDR_OPEN"	},
	{ TILE_BEDL_OPEN,					"TILE_BEDL_OPEN"	},
	{ TILE_BEDR_CLOSED,					"TILE_BEDR_CLOSED"	},
	{ TILE_BEDL_CLOSED,					"TILE_BEDL_CLOSED"	},
	{ TILE_FERRET,						"TILE_FERRET"		},
	{ TILE_CABOPEN_T,					"TILE_CABOPEN_T"	},
	{ TILE_CABOPEN_B,					"TILE_CABOPEN_B"	},
	{ GROUP_ENT_ENVELOPE_WHITE_SIT"01",	"TILE_ENV_WHITE"	},
	{ GROUP_ENT_ENVELOPE_BLUE_SIT"01",	"TILE_ENV_BLUE"		},
	{ GROUP_ENT_ENVELOPE_RED_SIT"01",	"TILE_ENV_RED" 		},
	{ GROUP_ENT_ENVELOPE_GREEN_SIT"01",	"TILE_ENV_GREEN"	},
	{ GROUP_ENT_TRANSCEIVER_SIT"01",	"TILE_TRANSCEIVER"	},
	{ TILE_HEAD_SCIENTIST,				"TILE_HEAD_SCIENTIST" },
	{ TILE_HEAD_SPACEDUDE,				"TILE_HEAD_SPACEDUDE" },
	{ TILE_HEAD_WORKER,					"TILE_HEAD_WORKER" },
	{ TILE_HEAD_ACCOUNTANT,				"TILE_HEAD_ACCOUNTANT" },
	{ TILE_HEAD_DOLLY,					"TILE_HEAD_DOLLY" },
	{ TILE_HEAD_SARGE,					"TILE_HEAD_SARGE" },
	{ TILE_BTUNNEL_BLOCK_B,				"TILE_BTUNNEL_BLOCK_B" },
	{ TILE_BTUNNEL_BLOCK_L,				"TILE_BTUNNEL_BLOCK_L" },
	{ TILE_BTUNNEL_BLOCK_R,				"TILE_BTUNNEL_BLOCK_R" },
	{ TILE_BTUNNEL_BLOCK_T,				"TILE_BTUNNEL_BLOCK_T" },
	{ TILE_BTUNNEL_DONE_B,				"TILE_BTUNNEL_DONE_B" },
	{ TILE_BTUNNEL_DONE_L,				"TILE_BTUNNEL_DONE_L" },
	{ TILE_BTUNNEL_DONE_R,				"TILE_BTUNNEL_DONE_R" },
	{ TILE_BTUNNEL_DONE_T,				"TILE_BTUNNEL_DONE_T" },
	{ GROUP_ENT_ROUTER_SIT"01",			"TILE_ROUTER" },
	{ GROUP_ENT_NOTE_SIT"01",			"TILE_NOTE" },
	{ TILE_INBOX,						"TILE_INBOX" },
	{ TILE_MEMOS,						"TILE_MEMOS" },
	{ TILE_A,							"TILE_A" },
	{ TILE_B,							"TILE_B" },
	{ TILE_C,							"TILE_C" },
	{ TILE_D,							"TILE_D" },
	{ TILE_E,							"TILE_E" },
	{ TILE_F,							"TILE_F" },
	{ TILE_G,							"TILE_G" },
	{ TILE_H,							"TILE_H" },
	{ TILE_I,							"TILE_I" },
	{ TILE_J,							"TILE_J" },
	{ TILE_K,							"TILE_K" },
	{ TILE_L,							"TILE_L" },
	{ TILE_M,							"TILE_M" },
	{ TILE_N,							"TILE_N" },
	{ TILE_O,							"TILE_O" },
	{ TILE_P,							"TILE_P" },
	{ TILE_Q,							"TILE_Q" },
	{ TILE_R,							"TILE_R" },
	{ TILE_S,							"TILE_S" },
	{ TILE_T,							"TILE_T" },
	{ TILE_U,							"TILE_U" },
	{ TILE_V,							"TILE_V" },
	{ TILE_W,							"TILE_W" },
	{ TILE_X,							"TILE_X" },
	{ TILE_Y,							"TILE_Y" },
	{ TILE_Z,							"TILE_Z" },
	{ TILE_SNOWDOOR_B,					"TILE_SNOWDOOR_B" },
	{ TILE_PACKAGE,						"TILE_PACKAGE" },
	{ TILE_TYRANIUM,					"TILE_TYRANIUM" },
	{ TILE_CHAIR,						"TILE_CHAIR" },
	{ TILE_ROUTER_HOLDER,				"TILE_ROUTER_HOLDER" },
	{ TILE_PHAR_CARPET,					"TILE_PHAR_CARPET" },
	{ TILE_WASTEBASKET,					"TILE_WASTEBASKET" },
	{ TILE_PIZZA,						"TILE_PIZZA" },
	{ TILE_HEADSTATUE,					"TILE_HEADSTATUE" },
	{ TILE_SLOT_GOOD1,					"TILE_SLOT_GOOD1" },
	{ TILE_SLOT_GOOD2,					"TILE_SLOT_GOOD2" },
	{ TILE_SLOT_GOOD3,					"TILE_SLOT_GOOD3" },
	{ TILE_SLOT_GOOD4,					"TILE_SLOT_GOOD4" },
	{ TILE_SLOT_GOOD5,					"TILE_SLOT_GOOD5" },
	{ TILE_SLOT_GOOD6,					"TILE_SLOT_GOOD6" },
	{ TILE_SLOT_BAD,					"TILE_SLOT_BAD" },
	{ GEM_BLUE_GFX,						"TILE_GEM_BLUE" },
	{ GEM_RED_GFX,						"TILE_GEM_RED" },
	{ GEM_GREEN_GFX,					"TILE_GEM_GREEN" },
	{ TITLE_EPISODE1,					"PIC_TITLE_EPISODE1"	},
	{ TITLE_EPISODE2,					"PIC_TITLE_EPISODE2"	},
	{ TITLE_EPISODE3,					"PIC_TITLE_EPISODE3"	},
	{ PIC_FROG_STATUE,					"PIC_FROG_STATUE" },
	{ PIC_JENKINS,						"PIC_JENKINS" },
	{ GEM_WHITE,						"TILE_GEM_WHITE" },

	{ CINE_INTROSHIP,					"PIC_CINE_INTROSHIP"	},
	{ CINE_LITTLE_MBOT,					"PIC_CINE_LITTLE_MBOT"	},
	{ CINE_CLOCK,						"PIC_CINE_CLOCK"		},
	{ MONKEYLOGOSCREEN,					"PIC_MONKEYLOGOSCREEN"	},
	{ MONKEYLOGO_OOHOOH,				"PIC_MONKEYLOGO_OOHOOH" },
	{ MONKEYLOGO_SMILE,					"PIC_MONKEYLOGO_SMILE"	},
	{ PIC_ENV_WHITE,					"PIC_ENV_WHITE" },
	{ PIC_TEAM_BRIAN,					"PIC_TEAM_BRIAN" },
	{ PIC_TEAM_STEVIE,					"PIC_TEAM_STEVIE" },
	{ PIC_TEAM_JOSH,					"PIC_TEAM_JOSH" },
	{ PIC_TEAM_TOM,						"PIC_TEAM_TOM" },
	{ PIC_TEAM_JOHN,					"PIC_TEAM_JOHN" },
	{ PIC_TEAM_LUCAS,					"PIC_TEAM_LUCAS" },

	{ PIC_RANK1,						"PIC_RANK1" },
	{ PIC_RANK2,						"PIC_RANK2" },
	{ PIC_RANK3,						"PIC_RANK3" },
	{ PIC_RANK4,						"PIC_RANK4" },
	{ PIC_RANK5,						"PIC_RANK5" },
	{NULL, NULL}
};

// For AI States, to be implemented
struct NumberInit {
	int value;
	const char *luaName;
} luaGlobalValues[] = {
	{ DIR_NONE,			"DIR_NONE"	},
	{ DIR_DOWN,			"DIR_DOWN"	},
	{ DIR_UP,			"DIR_UP"	},
	{ DIR_LEFT,			"DIR_LEFT"	},
	{ DIR_RIGHT,		"DIR_RIGHT"	},

	{ STATE_NONE,			"STATE_NONE"			},
	{ STATE_STANDDOWN,		"STATE_STANDDOWN"		},
	{ STATE_STANDUP,		"STATE_STANDUP"			},
	{ STATE_STANDLEFT,		"STATE_STANDLEFT"		},
	{ STATE_STANDRIGHT,		"STATE_STANDRIGHT"		},
	{ STATE_BLINK,			"STATE_BLINK"			},
	{ STATE_MOVEUP,			"STATE_MOVEUP"			},
	{ STATE_MOVEDOWN,		"STATE_MOVEDOWN"		},
	{ STATE_MOVELEFT,		"STATE_MOVELEFT"		},
	{ STATE_MOVERIGHT,		"STATE_MOVERIGHT"		},
	{ STATE_DYING,			"STATE_DYING"			},
	{ STATE_DEAD,			"STATE_DEAD"			},
	{ STATE_GOODJOB,		"STATE_GOODJOB"			},
	{ STATE_HORRIBLE1,		"STATE_HORRIBLE1"		},
	{ STATE_HORRIBLE2,		"STATE_HORRIBLE2"		},
	{ STATE_HORRIBLE3,		"STATE_HORRIBLE3"		},
	{ STATE_HORRIBLE4,		"STATE_HORRIBLE4"		},
	{ STATE_PUSHUP,			"STATE_PUSHUP"			},
	{ STATE_PUSHDOWN,		"STATE_PUSHDOWN"		},
	{ STATE_PUSHLEFT,		"STATE_PUSHLEFT"		},
	{ STATE_PUSHRIGHT,		"STATE_PUSHRIGHT"		},
	{ STATE_GRABUP,			"STATE_GRABUP"			},
	{ STATE_GRABDOWN,		"STATE_GRABDOWN"		},
	{ STATE_GRABLEFT,		"STATE_GRABLEFT"		},
	{ STATE_GRABRIGHT,		"STATE_GRABRIGHT"		},
	{ STATE_ATK_CLUB_UP,	"STATE_ATK_CLUB_UP"		},
	{ STATE_ATK_CLUB_DOWN,	"STATE_ATK_CLUB_DOWN"	},
	{ STATE_ATK_CLUB_LEFT,	"STATE_ATK_CLUB_LEFT"	},
	{ STATE_ATK_CLUB_RIGHT,	"STATE_ATK_CLUB_RIGHT"	},
	{ STATE_ATK_STUN_DOWN,	"STATE_ATK_STUN_DOWN"	},
	{ STATE_ATK_STUN_UP,	"STATE_ATK_STUN_UP"		},
	{ STATE_ATK_STUN_LEFT,	"STATE_ATK_STUN_LEFT"	},
	{ STATE_ATK_STUN_RIGHT,	"STATE_ATK_STUN_RIGHT"	},
	{ STATE_ATK_SLUG_DOWN,	"STATE_ATK_SLUG_DOWN"	},
	{ STATE_ATK_SLUG_UP,	"STATE_ATK_SLUG_UP"		},
	{ STATE_ATK_SLUG_LEFT,	"STATE_ATK_SLUG_LEFT"	},
	{ STATE_ATK_SLUG_RIGHT,	"STATE_ATK_SLUG_RIGHT"	},
	{ STATE_FLOATING,		"STATE_FLOATING"		},
	{ STATE_FLOATDOWN,		"STATE_FLOATDOWN"		},
	{ STATE_FLOATUP,		"STATE_FLOATUP"			},
	{ STATE_FLOATLEFT,		"STATE_FLOATLEFT"		},
	{ STATE_FLOATRIGHT,		"STATE_FLOATRIGHT"		},
	{ STATE_MELTED,			"STATE_MELTED"			},
	{ STATE_SLIDING,		"STATE_SLIDING"			},
	{ STATE_SHOCKING,		"STATE_SHOCKING"		},
	{ STATE_EXPLODING,		"STATE_EXPLODING"		},
	{ STATE_USEDOWN,		"STATE_USEDOWN"			},
	{ STATE_USEUP,			"STATE_USEUP"			},
	{ STATE_USELEFT,		"STATE_USELEFT"			},
	{ STATE_USERIGHT,		"STATE_USERIGHT"		},
	{ STATE_MEER_MOVE,		"STATE_MEER_MOVE"		},
	{ STATE_MEER_APPEAR,	"STATE_MEER_APPEAR"		},
	{ STATE_MEER_BITE,		"STATE_MEER_BITE"		},
	{ STATE_MEER_DISAPPEAR,	"STATE_MEER_DISAPPEAR"	},
	{ STATE_MEER_LOOK,		"STATE_MEER_LOOK"		},
	{ STATE_ICEP_PEEK,		"STATE_ICEP_PEEK"		},
	{ STATE_ICEP_APPEAR,	"STATE_ICEP_APPEAR"		},
	{ STATE_ICEP_THROWDOWN,	"STATE_ICEP_THROWDOWN"	},
	{ STATE_ICEP_THROWRIGHT,"STATE_ICEP_THROWRIGHT"	},
	{ STATE_ICEP_THROWLEFT,	"STATE_ICEP_THROWLEFT"	},
	{ STATE_ICEP_DISAPPEAR,	"STATE_ICEP_DISAPPEAR"	},
	{ STATE_ANGRY,			"STATE_ANGRY"			},
	{ STATE_PANIC,			"STATE_PANIC"			},
	{ STATE_LAUGH,			"STATE_LAUGH"			},
	{ STATE_KISSLEFT,		"STATE_KISSLEFT"		},
	{ STATE_KISSRIGHT,		"STATE_KISSRIGHT"		},
	{ STATE_DOLLYUSERIGHT,	"STATE_DOLLYUSERIGHT"	},
	{ STATE_YELL,			"STATE_YELL"			},

	{ NULL, NULL }
};

struct FuncInit {
	const char *luaName;
	int (*function) (lua_State *L);
} luaFuncs[] = {
	{  "Dialog",				dialog				},
	{  "DialogChoice",			dialogChoice		},
	{  "Message",				message				},
	{  "QueryInv",				queryInv			},
	{  "QueryInvItem",			queryInvItem		},
	{  "RemoveInv",				removeInv			},
	{  "RemoveInvItem",			removeInvItem		},
	{  "AddInvItem",			addInvItem			},
	{  "KeepInvItem",			keepInvItem			},
	{  "PurgeInv",				purgeInv			},
	{  "StartMusic",			startMusic			},
	{  "StopMusic",				stopMusic			},
	{  "FadeInMusic",			fadeInMusic			},
	{  "FadeOutMusic",			fadeOutMusic		},
	{  "RegisterSound",			registerSound		},
	{  "PlaySound",				playSound			},
	{  "FreeSound",				freeSound			},
	{  "StartMap",				startMap			},
	{  "Tile_ClearFG",			clearForeground		},
	{  "Tile_SetFG",			setForeground		},
	{  "Tile_SetBG",			setBackground		},
	{  "GetEntityXY",			getEntityXY			},
	{  "UseEntity",				useEntity			},
	{  "SetEntity",				setEntity			},
	{  "SetEntDir",				setEntDir			},
	{  "RemoveEntity",			removeEntity		},
	{  "AnimEntity",			animEntity			},
	{  "SetAnimFrame",			setAnimFrame		},
	{  "EntityFace",			entityFace			},
	{  "KillTrigger",			killTrigger			},
	{  "SpawnEntity",			spawnEntity			},
	{  "Animation",				animation			},
	{  "NewDelivery",			newDelivery			},
	{  "CompleteDelivery",		completeDelivery	},
	{  "DeliveriesLeft",		deliveriesLeft		},
	{  "SaveGlobal",			saveGlobal			},
	{  "LoadGlobal",			loadGlobal			},
	{  "PurgeGlobals",			purgeGlobals		},
	{  "TextOut",				textOut				},
	{  "CenterTextOut",			centerTextOut		},
	{  "SnowOn",				turnOnSnow			},
	{  "SnowOff",				turnOffSnow			},
	{  "GotoMenu",				gotoMenu			},
	{  "SetInfobarDark",		setInfobarDark		},
	{  "SetPointerState",		setPointerState		},
	{  "PlayVoice",				playVoice			},

	{  "Cine_StartCine",		cineStart			},
	{  "Cine_StopCine",			cineStop			},
	{  "Cine_StartMap",			cineStartMap		},
	{  "Cine_LockPlayer",		cineLockPlayer		},
	{  "Cine_UnlockPlayer",		cineUnlockPlayer	},
	{  "Cine_SetCamera",		cineSetCamera		},
	{  "Cine_ResetCamera",		cineResetCamera		},
	{  "Cine_MoveCamera",		cineMoveCamera		},
	{  "Cine_Wait",				cineWait			},
	{  "Cine_WaitUntilDone",	cineWaitUntilDone	},
	{  "Cine_UseEntity",		cineUseEntity		},
	{  "Cine_SetEntity",		cineSetEntity		},
	{  "Cine_RemoveEntity",		cineRemoveEntity	},
	{  "Cine_EntityFace",		cineEntityFace		},
	{  "Cine_MoveEntity",		cineMoveEntity		},
	{  "Cine_AnimEntity",		cineAnimEntity		},
	{  "Cine_SetAnimFrame",		cineSetAnimFrame	},
	{  "Cine_Dialog",			cineDialog			},
	{  "Cine_DrawPic",			cineDrawPic			},
	{  "Cine_DrawMaskedPic",	cineDrawMaskedPic	},
	{  "Cine_MovePic",			cineMovePic			},
	{  "Cine_MoveMaskedPic",	cineMoveMaskedPic	},
	{  "Cine_FadeOutBlack",		cineFadeOutBlack	},
	{  "Cine_FadeInBlack",		cineFadeInBlack		},
	{  "Cine_FadeOutWhite",		cineFadeOutWhite	},
	{  "Cine_FadeInWhite",		cineFadeInWhite		},
	{  "Cine_SpawnEntity",		cineSpawnEntity		},
	{  "Cine_PlaySound",		cinePlaySound		},
	{  "Cine_Tile_ClearFG",		cineClearForeground },
	{  "Cine_Tile_SetFG",		cineSetForeground	},
	{  "Cine_Tile_SetBG",		cineSetBackground	},
	{  "Cine_Function",			cineFunction		},
	{  "Cine_TextOut",			cineTextOut			},
	{  "Cine_CenterTextOut",	cineCenterTextOut	},
	{  "Cine_PlayVoice",		cinePlayVoice		},

	{	"openfile",				openFile,			},
	{	"write",				write,			},
	{	"closefile",			closeFile,			},
	{ NULL, NULL }
};

namespace {
int panicCB(lua_State *L) {
	error("Lua panic. Error message: %s", lua_isnil(L, -1) ? "" : lua_tostring(L, -1));
	return 0;
}

void debugHook(lua_State *L, lua_Debug *ar) {
	if (!lua_getinfo(L, "Sn", ar))
		return;

	debug("LUA: %s %s: %s %d", ar->namewhat, ar->name, ar->short_src, ar->currentline);
}
}

bool LuaScript::initScript(Common::SeekableReadStream *stream, const char *scriptName, int32 length) {

	if (_systemInit) {
		return false;
	}

	// Initialize Lua Environment
	_state = lua_open();
	if (_state == NULL) {
		error("Couldn't initialize Lua script.");
		return false;
	}
	luaL_openlibs(_state);

	_systemInit = true;

	// Register Extensions
	for (int i = 0; luaFuncs[i].luaName; i++) {
		lua_register(_state, luaFuncs[i].luaName, luaFuncs[i].function);
	}

	// Register Lua Globals

	for (int i = 0; luaGlobalStrings[i].realName; i++) {
		lua_pushstring(_state, luaGlobalStrings[i].realName);
		lua_setglobal(_state, luaGlobalStrings[i].luaName);
	}

	for (int j = 0; luaGlobalValues[j].luaName; j++) {
		lua_pushnumber(_state, luaGlobalValues[j].value);
		lua_setglobal(_state, luaGlobalValues[j].luaName);
	}

	// set the last mapname as a global
	lua_pushstring(_state, g_hdb->lastMapName());
	lua_setglobal(_state, "LASTMAP");

	// Set the lowest printable line
	lua_pushnumber(_state, 480 - 14);
	lua_setglobal(_state, "BOTTOM_Y");

	/*
		TODO: Load the sound names and entity
		spawn names into Lua once they are implemented.
	*/

	// Set the Entity Spawn Names  in Lua
	int j = 0;
	while (aiEntList[j].luaName) {
		lua_pushnumber(_state, aiEntList[j].type);
		lua_setglobal(_state, aiEntList[j].luaName);
		j++;
	}

	// Register panic callback function
	lua_atpanic(_state, panicCB);

	// Error handler for lua_pcall calls
	// The code below contains a local error handler function
	const char errorHandlerCode[] =
		"local function ErrorHandler(message) "
		"   return message .. '\\n' .. debug.traceback('', 2) "
		"end "
		"return ErrorHandler";

	// Compile the code
	if (luaL_loadbuffer(_state, errorHandlerCode, strlen(errorHandlerCode), "PCALL ERRORHANDLER") != 0) {
		// An error occurred, so dislay the reason and exit
		error("Couldn't compile luaL_pcall errorhandler:\n%s", lua_tostring(_state, -1));
		lua_pop(_state, 1);

		return false;
	}

	// Running the code, the error handler function sets the top of the stack
	if (lua_pcall(_state, 0, 1, 0) != 0) {
		// An error occurred, so dislay the reason and exit
		error("Couldn't prepare luaL_pcall errorhandler:\n%s", lua_tostring(_state, -1));
		lua_pop(_state, 1);

		return false;
	}

	// Place the error handler function in the Lua registry, and remember the index
	_pcallErrorhandlerRegistryIndex = luaL_ref(_state, LUA_REGISTRYINDEX);

	if (gDebugLevel >= 8) {
		// Initialize debugging callback
		lua_sethook(_state, debugHook, LUA_MASKCALL | LUA_MASKLINE, 0);
	}

	// Load GLOBAL.LUA and execute it

	if (!executeMPC(_globalLuaStream, "global code", "GLOBAL.LUA", _globalLuaLength)) {
		error("LuaScript::initScript: 'global code' failed to execute");
		return false;
	}

	// Load script and execute it

	if (!executeMPC(stream, "level code", scriptName, length)) {
		error("LuaScript::initScript: 'level code' failed to execute");
		return false;
	}

	lua_getglobal(_state, "level_init");

	// Error handling function to be executed after the function is put on the stack
	lua_rawgeti(_state, LUA_REGISTRYINDEX, _pcallErrorhandlerRegistryIndex);
	lua_insert(_state, -2);

	if (lua_pcall(_state, 0, 0, -2)) {
		error("An error occured while executing \"%s\": %s.", "level_init", lua_tostring(_state, -1));
		lua_pop(_state, -1);

		return false;
	}

	// Remove the error handler function from the stack
	lua_pop(_state, 1);

	return true;
}

void LuaScript::pushInt(int value) {
	if (!_systemInit)
		return;
	lua_pushnumber(_state, (double)value);
}

void LuaScript::pushString(char *string) {
	if (!_systemInit)
		return;
	lua_pushstring(_state, string);
}

void LuaScript::pushFunction(char *func) {
	int type;

	if (!_systemInit)
		return;

	lua_getglobal(_state, func);
	type = lua_type(_state, 1);
	if (type != LUA_TFUNCTION && type != LUA_TNUMBER) {
		warning("pushFunction: Function '%s' doesn't exists", func);
	}
}

void LuaScript::call(int args, int returns) {
	if (!_systemInit)
		return;

	if (lua_pcall(_state, args, returns, -2)) {
		error("An error occured while executing: %s.", lua_tostring(_state, -1));
		lua_pop(_state, -1);
	}
}

bool LuaScript::callFunction(const char *name, int returns) {
	if (!_systemInit) {
		return false;
	}

	lua_getglobal(_state, name);

	if (lua_pcall(_state, 0, returns, -2)) {
		error("An error occured while executing \"%s\": %s.", name, lua_tostring(_state, -1));
		lua_pop(_state, -1);

		return false;
	}

	return true;
}

void LuaScript::invokeLuaFunction(char *luaFunc, int x, int y, int value1, int value2) {
	int type;

	if (!_systemInit)
		return;

	lua_getglobal(_state, luaFunc);
	type = lua_type(_state, 1);
	if (type != LUA_TFUNCTION) {
		warning("Function '%s' doesn't exist", luaFunc);
	} else {
		lua_pushnumber(_state, x);
		lua_pushnumber(_state, y);
		lua_pushnumber(_state, value1);
		lua_pushnumber(_state, value2);
		lua_call(_state, 4, 0);
	}
}

bool LuaScript::executeMPC(Common::SeekableReadStream *stream, const char *name, const char *scriptName, int32 length) {

	if (!_systemInit) {
		return false;
	}

	char *chunk = new char[length + 1];
	stream->read((void *)chunk, length);
	chunk[length] = '\0'; // be on the safe side

	stripComments(chunk);

	/*
			Remove C-style comments from the script
			and update the upvalue syntax for Lua 5.1.3
	*/
	Common::String chunkString(chunk, length);

	addPatches(chunkString, scriptName);

	if (!executeChunk(chunkString, name)) {
		delete[] chunk;

		return false;
	}

	delete[] chunk;

	return true;
}

bool LuaScript::executeFile(const Common::String &filename) {

	if (!_systemInit) {
		return false;
	}

	Common::File *file = new Common::File;

	if (!file->open(filename)) {
		error("Cannot find \"%s\"", filename.c_str());
	}

	uint fileSize = file->size();
	char *fileData = new char[fileSize + 1];
	file->read((void *)fileData, fileSize);
	fileData[fileSize] = '\0'; // be on the safe side

	stripComments(fileData);

	Common::String fileDataString(fileData);

	addPatches(fileDataString, filename.c_str());

	if (!executeChunk(fileDataString, filename)) {
		delete[] fileData;
		delete file;

		return false;
	}

	delete[] fileData;
	delete file;

	return true;
}

bool LuaScript::executeChunk(Common::String &chunk, const Common::String &chunkName) const {

	if (!_systemInit) {
		return false;
	}

	// Compile Chunk
	if (luaL_loadbuffer(_state, chunk.c_str(), chunk.size(), chunkName.c_str())) {
		error("Couldn't compile \"%s\": %s", chunkName.c_str(), lua_tostring(_state, -1));
		lua_pop(_state, -1);

		return false;
	}

	// Error handling function to be executed after the function is put on the stack
	lua_rawgeti(_state, LUA_REGISTRYINDEX, _pcallErrorhandlerRegistryIndex);
	lua_insert(_state, -2);

	// Execute Chunk
	if (lua_pcall(_state, 0, 0, -2)) {
		error("An error occured while executing \"%s\": %s.", chunkName.c_str(), lua_tostring(_state, -1));
		lua_pop(_state, -1);

		return false;
	}

	// Remove the error handler function from the stack
	lua_pop(_state, 1);

	return true;
}

void LuaScript::stripComments(char *chunk) {
	uint32 offset = 0;

	while (chunk[offset]) {
		// Strip C-Style comments
		if (chunk[offset] == '/' && chunk[offset + 1] == '/') {
			while (chunk[offset] != 0x0d) {
				chunk[offset++] = ' ';
			}
		}
		offset++;
	}
}

void LuaScript::addPatches(Common::String &chunk, const char *scriptName) {
	ScriptPatch *patch = scriptPatches;
	int applied = 0;

	while (patch->scriptName) {
		if (!strcmp(scriptName, patch->scriptName)) {
			Common::String searchString(patch->search);
			Common::String replaceString(patch->replace);
			Common::replace(chunk, searchString, replaceString);
			applied++;
		}
		patch++;
	}

	if (applied)
		debug(1, "Applied %d patches to %s", applied, scriptName);
}

void LuaScript::checkParameters(const char *func, int params) {
	int stackTop = lua_gettop(_state);
	if (stackTop < params) {
		warning("%s: Not Enough Parameters", func);
	} else if (stackTop > params) {
		warning("%s: Too Many Parameters", func);
	}
}

void lua_printstack(lua_State *L) {
	int n = lua_gettop(L);
	for (int i = 1; i <= n; i++)  {
		debug(1, "STACK %d %s %s", i, lua_tostring(L, i), luaL_typename(L, i));
	}
}

const char *LuaScript::getStringOffStack() {
	if (!_systemInit) {
		return NULL;
	}

	const char *string = lua_tostring(_state, 1);
	lua_remove(_state, 1);
	return string;
}

}
