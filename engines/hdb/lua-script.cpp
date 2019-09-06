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

#include "common/file.h"
#include "common/lua/lua.h"
#include "common/lua/lauxlib.h"
#include "common/lua/lualib.h"

#include "hdb/hdb.h"
#include "hdb/ai.h"
#include "hdb/file-manager.h"
#include "hdb/lua-script.h"
#include "hdb/gfx.h"
#include "hdb/map.h"
#include "hdb/mpc.h"
#include "hdb/sound.h"
#include "hdb/window.h"

namespace HDB {

struct ScriptPatch {
	const char *scriptName;
	const char *search;
	const char *replace;
} scriptPatches[] = {
	{"GLOBAL", "return gsub( s, \"\\n\", \"\\\\\\n\" )", "return string.gsub( s, \"\\n\", \"\\\\\\n\" )"}, // line 10
	{"GLOBAL", "strsub(", "string.sub("}, // line 15
	{"GLOBAL", "function save( i,v, nest )", "function func_namelookup(func)\nfor i,v in pairs(_G) do\nif type(v) == 'function' and v == func then\nreturn i\nend\nend\nend\nfunction save( i,v, nest )"}, // line 13
	{"GLOBAL", "if type(v) == 'userdata' or type(v) == 'function' then return end", "if type(v) == 'userdata' or i == 'package' or i == 'os' or i == 'io' or i == 'string' or i == 'table' or i == 'debug' or i == 'math' or i == 'coroutine' then return end" }, // Line 16
	{"GLOBAL", "local t=type(v)", "local t=type(v)\nif (t == 'function' and nest == 0) then return end"}, // line 18
	{"GLOBAL", "then write_record(v, nest + 1)", "then write_record(v, nest + 1)\nelseif t=='function'    then write(savefile, func_namelookup(v)) if nest > 0 then write(savefile, ',' ) end"}, // line 32
	{"GLOBAL", "for i,v in t do", "for i,v in pairs(t) do"},  // line 43
	{"GLOBAL", "for i,v in globals() do", "for i,v in pairs(_G) do"}, // line 52
	{"GLOBAL", "for npcname,npcdata in npcs do", "for npcname,npcdata in pairs(npcs) do"}, // Line 66
	{"GLOBAL", "for dlgname,dlgdata in npcdata.dialog do", "for dlgname,dlgdata in pairs(npcdata.dialog) do"}, // Line 67
	{"GLOBAL", "s = format( \"npcs.%s.dialog.%s.counter", "s = string.format( \"npcs.%s.dialog.%s.counter"}, // Line 68
	{"GLOBAL", "s = format( \"npcs.%s.dialog.%s.finished = ", "s = string.format( \"npcs.%s.dialog.%s.finished = "}, // line 70
	{"GLOBAL", "if( getglobal( \"map\"..tostring(v1)..\"_complete\" ) ) then", "if( _G[\"map\"..tostring(v1)..\"_complete\"] ) then"}, // line 219
	{"GLOBAL", "local mapname = format( \"MAP", "local mapname = string.format( \"MAP"}, // line 225
	{"GLOBAL", "if( dtable.counter < getn(dtable) ) then", "if( dtable.counter < #dtable ) then"}, // line 254
	{"GLOBAL", "closefunc = getglobal( npcdef.codename..\"_use\" )", "closefunc = _G[npcdef.codename..\"_use\"]"}, // line 272
	{"GLOBAL", "setglobal( npcdef.codename..\"_init\", function() return NPC_Init( %npcdef ) end )", "_G[npcdef.codename .. \"_init\"] = function() return NPC_Init( npcdef ) end"}, // line 317
	{"GLOBA", "setglobal( npcdef.codename..\"_use\", function(x, y, v1, v2) return NPC_Use( %npcdef, x, y, v1, v2 ) end )", "_G[npcdef.codename .. \"_use\"] = function(x, y, v1, v2) return NPC_Use( npcdef, x, y, v1, v2 ) end"}, // line 318
	{"GLOBAL", "for i,npc in npcs do", "for i,npc in pairs(npcs) do"}, // 323

	{"MAP00", "if( getn( beds ) == 0 ) then", "if( #beds == 0 ) then"}, // line 924
	{"MAP00", "tempfunc = function() emptybed_use( %x, %y, %v1, %v2 ) end", "tempfunc = function() emptybed_use(x, y, v1, v2) end"}, // line 926

	{"MAP01", "strsub(", "string.sub("}, // line 23
	{"MAP01", "if( covert_index < getn(covert_dialog) ) then", "if( covert_index < #covert_dialog ) then"}, // line 766
	{"MAP01", "if( chiste_index < getn( chiste_dialog ) )then", "if( chiste_index < #chiste_dialog ) then"}, // 845

	{"MAP10", "if( getn( beds ) == 0 ) then", "if( #beds == 0 ) then"}, // line 119
	{"MAP10", "emptybed_use( %x, %y, %v1, %v2 )", "emptybed_use( x, y, v1, v2 )"}, // line 121

	{"MAP15", "function cop5_5b()", "function cop5_4b()"}, // line 112
	{"MAP15", "function cop5_5c()", "function cop5_4c()"}, // line 115
	{"MAP15", "function cop5_5d()", "function cop5_4d()"}, // line 118
	{"MAP15", "function cop5_5e()", "function cop5_4e()"}, // line 121
	{"MAP15", "function cop5_5f()", "function cop5_4f()"}, // line 124

	{"MAP19", "func = getglobal( \"frustrato_nice\"..tostring(frustrato_stage) )", "func = _G[ \"frustrato_nice\"..tostring(frustrato_stage) ]"}, // line 296
	{"MAP19", "func = getglobal( \"frustrato_choice\"..tostring(frustrato_stage + 1) )", "func = _G[ \"frustrato_choice\"..tostring(frustrato_stage + 1) ]"}, // line 298

	{"MAP11", "mapstring = strsub( LASTMAP, 1, 5 )", "mapstring = string.sub( LASTMAP, 1, 5 )"}, // line 51

	{"MAP21", "mapstring = strsub( LASTMAP, 1, 5 )", "mapstring = string.sub( LASTMAP, 1, 5 )"}, // line 105

	{"MAP29", "Message( format( \"", "Message( string.format( \""}, // line 195

	{"MAP30", "rots = 19 + random( 6 )", "rots = 19 + math.random( 6 )"}, // line 208
	{"MAP30", "if i1 > getn(tiles)", "if i1 > #tiles"}, // line 211
	{"MAP30", "if i2 > getn(tiles)", "if i2 > #tiles"}, // line 212
	{"MAP30", "if i3 > getn(tiles)", "if i3 > #tiles"}, // line 213
	{"MAP30", "rots = 13 + random( 4 )", "rots = 13 + math.random( 4 )"}, // line 234
	{"MAP30", "if i2 > getn(tiles)", "if i2 > #tiles"}, // line 237
	{"MAP30", "if i3 > getn(tiles)", "if i3 > #tiles"}, // line 238
	{"MAP30", "rots = 13 + random( 4 )", "rots = 13 + math.random( 4 )"}, // line 258
	{"MAP30", "if i3 > getn(tiles)", "if i3 > #tiles"}, // line 261

	// Jump straight to credits
	//{"CINE_OUTRO", "-- delegate", "-- delegate\nCine_FadeOutBlack( 40 )\nCredits()"},

	{NULL, NULL, NULL}
};

LuaScript::LuaScript() {

	if (g_hdb->isPPC()) {
		_cameraXOff = 0;
		_cameraYOff = 0;
	} else {
		_cameraXOff = (32 * 3 + 24);	// 3.75 Tiles Extra
		_cameraYOff = (32 * 2 + 16);	// 2.50 Tiles Extra
	}

	_state = NULL;
	_systemInit = false;


	_pcallErrorhandlerRegistryIndex = 0;
	_globalLuaStream = nullptr;
	_globalLuaLength = 0;
}

LuaScript::~LuaScript() {
	if (_state)
		lua_close(_state);

	if (_globalLuaStream)
		delete _globalLuaStream;
}

void LuaScript::init() {
	// Load Global Lua Code
	_globalLuaStream = g_hdb->_fileMan->findFirstData("GLOBAL.LUA", TYPE_BINARY);
	_globalLuaLength = g_hdb->_fileMan->getLength("GLOBAL.LUA", TYPE_BINARY);
	if (_globalLuaStream == NULL || _globalLuaLength == 0) {
		error("LuaScript::initScript: 'global code' failed to load");
	}
}

bool LuaScript::loadLua(const char *name) {
	Common::SeekableReadStream *luaStream = g_hdb->_fileMan->findFirstData(name, TYPE_BINARY);
	int32 luaLength = g_hdb->_fileMan->getLength(name, TYPE_BINARY);
	if (luaStream == NULL) {
		warning("The %s MPC entry can't be found", name);

		_systemInit = false;

		delete luaStream;
		return false;
	}

	_systemInit = initScript(luaStream, name, luaLength);
	delete luaStream;

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

void LuaScript::save(Common::OutSaveFile *out) {
	out->writeUint32LE(_globals.size());

	// Save Globals
	for (uint i = 0; i < _globals.size(); i++) {
		out->write(_globals[i]->global, 32);
		out->writeSint32LE(_globals[i]->valueOrString);
		out->writeDoubleLE(_globals[i]->value);
		out->write(_globals[i]->string, 32);
	}

	g_hdb->_currentOutSaveFile = out;

	lua_printstack(_state);
	lua_getglobal(_state, "SaveState");

	// the save file will be ignored
	lua_pushstring(_state, "tempSave");
	lua_call(_state, 1, 0);

	g_hdb->_currentOutSaveFile = NULL;
}

void LuaScript::loadSaveFile(Common::InSaveFile *in) {
	// Clear out all globals
	_globals.clear();

	// Start reading globals
	uint32 globalsSize = in->readUint32LE();
	for (uint i = 0; i < globalsSize; i++) {
		Global *g = new Global;

		in->read(g->global, 32);
		g->valueOrString = in->readSint32LE();
		g->value = in->readDoubleLE();
		in->read(g->string, 32);

		_globals.push_back(g);
	}

	g_hdb->_currentInSaveFile = in;

	lua_getglobal(_state, "LoadState");
	// it will be ignored
	lua_pushstring(_state, "tempSave");

	lua_call(_state, 1, 0);

	g_hdb->_currentInSaveFile = NULL;
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
	double index = lua_tonumber(L, 1);
	double actor = lua_tonumber(L, 2);

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
	const char *entName = lua_tostring(L, 1);

	g_hdb->_lua->checkParameters("cineRemoveEntity", 1);

	lua_pop(L, 1);

	g_hdb->_ai->cineRemoveEntity(entName);
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
	const char *id = lua_tostring(L, 1);
	const char *pic = lua_tostring(L, 2);
	double x = lua_tonumber(L, 3);
	double y = lua_tonumber(L, 4);

	g_hdb->_lua->checkParameters("cineDrawPic", 4);

	x += g_hdb->_lua->_cameraXOff;
	y += g_hdb->_lua->_cameraYOff;

	lua_pop(L, 4);
	g_hdb->_ai->cineDrawPic(id, pic, (int)x, (int)y);

	return 0;
}

static int cineDrawMaskedPic(lua_State *L) {
	const char *id = lua_tostring(L, 1);
	const char *pic = lua_tostring(L, 2);
	double x = lua_tonumber(L, 3);
	double y = lua_tonumber(L, 4);

	g_hdb->_lua->checkParameters("cineDrawMaskedPic", 4);

	x += g_hdb->_lua->_cameraXOff;
	y += g_hdb->_lua->_cameraYOff;

	lua_pop(L, 4);
	g_hdb->_ai->cineDrawMaskedPic(id, pic, (int)x, (int)y);

	return 0;
}

static int cineMovePic(lua_State *L) {
	const char *id = lua_tostring(L, 1);
	const char *pic = lua_tostring(L, 2);
	double	x1 = lua_tonumber(L, 3);
	double	y1 = lua_tonumber(L, 4);
	double	x2 = lua_tonumber(L, 5);
	double	y2 = lua_tonumber(L, 6);
	double	speed = lua_tonumber(L, 7);

	g_hdb->_lua->checkParameters("cineMovePic", 7);

	x1 += g_hdb->_lua->_cameraXOff;
	y1 += g_hdb->_lua->_cameraYOff;
	x2 += g_hdb->_lua->_cameraXOff;
	y2 += g_hdb->_lua->_cameraYOff;

	lua_pop(L, 7);

	g_hdb->_ai->cineMovePic(id, pic, (int)x1, (int)y1, (int)x2, (int)y2, (int)speed);

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

	x1 += g_hdb->_lua->_cameraXOff;
	y1 += g_hdb->_lua->_cameraYOff;
	x2 += g_hdb->_lua->_cameraXOff;
	y2 += g_hdb->_lua->_cameraYOff;

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
	const char *func = lua_tostring(L, 1);

	g_hdb->_lua->checkParameters("cineFunction", 1);

	lua_pop(L, 1);

	g_hdb->_ai->cineFunction(func);

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

	x += g_hdb->_lua->_cameraXOff;
	y += g_hdb->_lua->_cameraYOff;

	lua_pop(L, 4);
	g_hdb->_ai->cineTextOut(string, (int)x, (int)y, (int)timer);
	return 0;
}

static int cineCenterTextOut(lua_State *L) {
	const char *string = lua_tostring(L, 1);
	double y = lua_tonumber(L, 2);
	double timer = lua_tonumber(L, 3);

	g_hdb->_lua->checkParameters("cineCenterTextOut", 3);

	y += g_hdb->_lua->_cameraYOff;

	lua_pop(L, 3);
	g_hdb->_ai->cineCenterTextOut(string, (int)y, (int)timer);
	return 0;
}

static int newDelivery(lua_State *L) {
	const char *itemTextName = lua_tostring(L, 1);
	const char *itemGfxName = lua_tostring(L, 2);
	const char *destTextName = lua_tostring(L, 3);
	const char *destGfxName = lua_tostring(L, 4);
	const char *id = lua_tostring(L, 5);

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
	const char *initName = lua_tostring(L, 1);

	g_hdb->_lua->checkParameters("getEntityXY", 1);

	lua_pop(L, 1);

	int x, y;
	g_hdb->_ai->getEntityXY(initName, &x, &y);

	lua_pushnumber(L, x);
	lua_pushnumber(L, y);
	return 2;
}

static int setEntity(lua_State *L) {
	const char *entName = lua_tostring(L, 1);
	double x = lua_tonumber(L, 2);
	double y = lua_tonumber(L, 3);
	double level = lua_tonumber(L, 4);

	g_hdb->_lua->checkParameters("setEntity", 4);

	lua_pop(L, 4);
	AIEntity *e = g_hdb->_ai->locateEntity(entName);
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
	const char *entName = lua_tostring(L, 1);
	double d = lua_tonumber(L, 2);

	g_hdb->_lua->checkParameters("setEntDir", 2);

	lua_pop(L, 2);
	AIEntity *e = g_hdb->_ai->locateEntity(entName);

	if (e) {
		int	dd = (int)d;
		e->dir = (AIDir)dd;
	} else {
		char buff[64];
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
	const char *title = lua_tostring(L, 1);
	double tileIndex = lua_tonumber(L, 2);
	const char *string = lua_tostring(L, 3);
	const char *more = lua_tostring(L, 4);

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

	int	amount = lua_gettop(L) - 3;
	if (amount > 9)
		amount = 9;

	for (int i = 0; i < amount; i++)
		choice[i] = lua_tostring(L, 4 + i);

	lua_pop(L, amount + 3);

	g_hdb->_window->openDialogChoice(title, text, func, amount, &choice[0]);
	return 0;
}

static int message(lua_State *L) {
	const char *title = lua_tostring(L, 1);
	double delay = lua_tonumber(L, 2);

	g_hdb->_lua->checkParameters("message", 2);

	lua_pop(L, 2);

	g_hdb->_window->openMessageBar(title, (int)delay);
	return 0;
}

static int animation(lua_State *L) {
	double x = lua_tonumber(L, 1);
	double y = lua_tonumber(L, 2);
	double which = lua_tonumber(L, 3);
	double playsnd = lua_tonumber(L, 4);

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
		if (!g_hdb->isDemo())
			if (playsnd)
				g_hdb->_sound->playSound(SND_BARREL_EXPLODE);
		break;
	case 2:
		g_hdb->_ai->addAnimateTarget((int)x, (int)y, 0, 3, ANIM_FAST, false, false, GROUP_STEAM_PUFF_SIT);
		if (!g_hdb->isDemo())
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
	double dir = lua_tonumber(L, 2);
	double x = lua_tonumber(L, 3);
	double y = lua_tonumber(L, 4);
	const char *funcInit = lua_tostring(L, 5);
	const char *funcAction = lua_tostring(L, 6);
	const char *funcUse = lua_tostring(L, 7);
	double dir2 = lua_tonumber(L, 8);
	double level = lua_tonumber(L, 9);
	double value1 = lua_tonumber(L, 10);
	double value2 = lua_tonumber(L, 11);

	int t = (int)type;
	int d = (int)dir;
	int d2 = (int)dir2;

	g_hdb->_lua->checkParameters("spawnEntity", 11);

	lua_pop(L, 11);

	g_hdb->_ai->spawn((AIType)t, (AIDir)d, (int)x, (int)y, funcInit, funcAction, funcUse, (AIDir)d2, (int)level, (int)value1, (int)value2, 1);
	return 0;
}

static int addInvItem(lua_State *L) {
	double type = lua_tonumber(L, 1);
	double amount = lua_tonumber(L, 2);
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
	// get the passed-in search string
	const char *search = lua_tostring(L, 1);

	g_hdb->_lua->checkParameters("queryInv", 1);

	lua_pop(L, 1);

	// call the function & get return value
	int result = g_hdb->_ai->queryInventory(search);
	// send the return value back to Lua
	lua_pushnumber(L, result);
	return 1;
}

static int purgeInv(lua_State *L) {
	g_hdb->_ai->purgeInventory();
	return 0;
}

static int queryInvItem(lua_State *L) {
	// get the passed-in search string
	double search = lua_tonumber(L, 1);
	int s1 = (int)search;

	g_hdb->_lua->checkParameters("queryInvItem", 1);

	lua_pop(L, 1);

	// call the function & get return value
	int result = g_hdb->_ai->queryInventoryType((AIType)s1);
	// send the return value back to Lua
	lua_pushnumber(L, result);
	return 1;
}

static int removeInv(lua_State *L) {
	// get the passed-in search string
	const char *search = lua_tostring(L, 1);
	double number = lua_tonumber(L, 2);

	g_hdb->_lua->checkParameters("removeInv", 2);

	lua_pop(L, 2);

	// call the function & get return value
	int result = (int)g_hdb->_ai->removeInvItem(search, (int)number);
	// send the return value back to Lua
	lua_pushnumber(L, result);
	return 1;
}

static int removeInvItem(lua_State *L) {
	// get the passed-in type value
	double	search = lua_tonumber(L, 1);
	double	number = lua_tonumber(L, 2);

	g_hdb->_lua->checkParameters("removeInvItem", 2);

	lua_pop(L, 2);

	int	s = (int)search;
	// call the function & get return value
	int result = (int)g_hdb->_ai->removeInvItemType((AIType)s, (int)number);
	// send the return value back to Lua
	lua_pushnumber(L, result);
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
	double song = lua_tonumber(L, 1);
	int s1 = (int)song;

	g_hdb->_lua->checkParameters("startMusic", 1);

	lua_pop(L, 1);
	g_hdb->_sound->startMusic((SoundType)s1);

	return 0;
}

static int fadeInMusic(lua_State *L) {
	double song = lua_tonumber(L, 1);
	int s1 = (int)song;
	int ramp = (int)lua_tonumber(L, 2);
	if (!ramp)
		ramp = 1;

	g_hdb->_lua->checkParameters("fadeInMusic", 2);

	lua_pop(L, 2);
	g_hdb->_sound->fadeInMusic((SoundType)s1, ramp);

	return 0;
}

static int stopMusic(lua_State *L) {
	g_hdb->_sound->stopMusic();
	return 0;
}

static int fadeOutMusic(lua_State *L) {
	int ramp = (int)lua_tonumber(L, 1);
	if (!ramp)
		ramp = 1;

	g_hdb->_lua->checkParameters("fadeOutMusic", 1);

	lua_pop(L, 1);
	g_hdb->_sound->fadeOutMusic(ramp);

	return 0;
}

static int registerSound(lua_State *L) {
	const char *name = lua_tostring(L, 1);

	g_hdb->_lua->checkParameters("registerSound", 1);

	lua_pop(L, 1);
	int index = g_hdb->_sound->registerSound(name);
	lua_pushnumber(L, index);

	return 1;
}

static int playSound(lua_State *L) {
	int index = (int)lua_tonumber(L, 1);

	g_hdb->_lua->checkParameters("playSound", 1);

	lua_pop(L, 1);
	g_hdb->_sound->playSound(index);

	return 0;
}

static int freeSound(lua_State *L) {
	int index = (int)lua_tonumber(L, 1);

	g_hdb->_lua->checkParameters("freeSound", 1);

	lua_pop(L, 1);
	g_hdb->_sound->freeSound(index);

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

	g_hdb->_lua->checkParameters("saveGlobal", 1);

	lua_pop(L, 1);

	lua_getglobal(L, global);
	int type = lua_type(L, 1);
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
	double index = lua_tonumber(L, 1);
	double actor = lua_tonumber(L, 2);

	g_hdb->_lua->checkParameters("playVoice", 2);

	lua_pop(L, 2);

	g_hdb->_sound->playVoice((int)index, (int)actor);
	return 0;
}

static int openFile(lua_State *L) {
	g_hdb->_lua->checkParameters("openFile", 2);

	lua_pop(L, 2); // drop 2 parameters

	lua_pushlightuserdata(L, 0);

	return 1;
}

static int writeto(lua_State *L) {
	g_hdb->_lua->checkParameters("writeto", 1);

	lua_pop(L, 1);

	return 1;
}

static int write(lua_State *L) {
	Common::OutSaveFile *out = g_hdb->_currentOutSaveFile;
	const char *data;

	if (g_hdb->isDemo() && g_hdb->isPPC()) {
		data = lua_tostring(L, 1);

		g_hdb->_lua->checkParameters("write", 1);

		lua_pop(L, 1);
	} else {
		data = lua_tostring(L, 2);

		g_hdb->_lua->checkParameters("write", 2);

		lua_pop(L, 2);
	}

	out->write(data, strlen(data));

	return 0;
}

static int closeFile(lua_State *L) {
	g_hdb->_lua->checkParameters("closeFile", 1);

	lua_pop(L, 1);

	// No op

	return 0;
}

static int dofile(lua_State *L) {
	g_hdb->_lua->checkParameters("dofile", 1);

	lua_pop(L, 1);

	Common::InSaveFile *in = g_hdb->_currentInSaveFile;

	int length = in->size() - in->pos();
	char *chunk = new char[length + 1];
	in->read((void *)chunk, length);

	chunk[length] = '\0'; // be on the safe side

	Common::String chunkString(chunk);
	delete[] chunk;

	if (!g_hdb->_lua->executeChunk(chunkString, "saveState")) {
		return 0;
	}

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
	{ PIC_DEMOSCREEN,					"PIC_DEMOSCREEN" },
	{ PIC_DEMOSCREEN2,					"PIC_DEMOSCREEN2" },
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

	{ STATE_NONE, NULL }
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
	{	"write",				write,				},
	{	"closefile",			closeFile,			},
	{	"dofile",				dofile,				},
	{	"writeto",				writeto,			},
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
	if (_state != NULL) {
		lua_close(_state);
	}

	// Initialize Lua Environment
	_state = lua_open();
	if (_state == NULL) {
		error("Couldn't initialize Lua script.");
		return false;
	}
	luaL_openlibs(_state);

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

	for (int j = 0; j < g_hdb->_sound->getNumSounds(); j++) {
		const char *name = g_hdb->_sound->getSNDLuaName(j);
		lua_pushnumber(_state, j);
		lua_setglobal(_state, name);
	}

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
		// An error occurred, so display the reason and exit
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

	// Make sure we start from the beginning
	_globalLuaStream->seek(0);
	if (!executeMPC(_globalLuaStream, "GLOBAL.LUA", "GLOBAL.LUA", _globalLuaLength)) {
		error("LuaScript::initScript: 'global code' failed to execute");
		return false;
	}

	// Load script and execute it

	if (!executeMPC(stream, scriptName, scriptName, length)) {
		error("LuaScript::initScript: %s failed to execute", scriptName);
		return false;
	}

	lua_getglobal(_state, "level_init");

	// Error handling function to be executed after the function is put on the stack
	lua_rawgeti(_state, LUA_REGISTRYINDEX, _pcallErrorhandlerRegistryIndex);
	lua_insert(_state, -2);

	if (lua_pcall(_state, 0, 0, -2)) {
		error("LuaScript::initScript: An error occured while executing \"%s\": %s.", "level_init", lua_tostring(_state, -1));
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
	if (!_systemInit)
		return;

	lua_getglobal(_state, func);
	int type = lua_type(_state, 1);
	if (type != LUA_TFUNCTION && type != LUA_TNUMBER) {
		warning("pushFunction: Function '%s' doesn't exists", func);
	}
}

void LuaScript::call(int args, int returns) {
	if (!_systemInit)
		return;

	if (lua_pcall(_state, args, returns, -2)) {
		error("LuaScript::call: An error occured while executing: %s.", lua_tostring(_state, -1));
		lua_pop(_state, -1);
	}
}

bool LuaScript::callFunction(const char *name, int returns) {
	if (!_systemInit) {
		return false;
	}

	lua_getglobal(_state, name);

	if (lua_pcall(_state, 0, returns, -2)) {
		error("LuaScript::callFunction: An error occured while executing \"%s\": %s.", name, lua_tostring(_state, -1));
		lua_pop(_state, -1);

		return false;
	}

	return true;
}

void LuaScript::invokeLuaFunction(char *luaFunc, int x, int y, int value1, int value2) {
	if (!_systemInit)
		return;

	lua_getglobal(_state, luaFunc);
	int type = lua_type(_state, 1);

#if 0
	if (!strcmp(luaFunc, "ferretbed_use")) {
		const char *t = lua_typename(_state, type);
		warning("type: %s", t);
		Common::String a("print(ferretbed_init(50, 30, 0, 0))");

		executeChunk(a, "debug");

//		a = "print('woo') for i,v in pairs(_G) do if type(v) == 'function' then print(i) end end";
//		a = "print(ferretbed_use())";

		executeChunk(a, "debug");

		lua_printstack(_state);
	}
#endif

	if (type != LUA_TFUNCTION) {
		warning("Function '%s' doesn't exist (%d)", luaFunc, type);
	} else {
		lua_pushnumber(_state, x);
		lua_pushnumber(_state, y);
		lua_pushnumber(_state, value1);
		lua_pushnumber(_state, value2);
		lua_call(_state, 4, 0);
	}
}

bool LuaScript::executeMPC(Common::SeekableReadStream *stream, const char *name, const char *scriptName, int32 length) {
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
		error("LuaScript::executeChunk: An error occured while executing \"%s\": %s.", chunkName.c_str(), lua_tostring(_state, -1));
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
		if (!scumm_strnicmp(scriptName, patch->scriptName, strlen(patch->scriptName))) {
			Common::String searchString(patch->search);
			Common::String replaceString(patch->replace);
			Common::replace(chunk, searchString, replaceString);
			applied++;
		}
		patch++;
	}

	if (applied)
		debug(1, "Applied %d patches to %s", applied, scriptName);

	if (gDebugLevel > 3) {
		warning(">>>>>>>>>>> SCRIPT: %s", scriptName);
		chunk += "\nfor i,v in pairs(_G) do if type(v) == 'function' then print(i) end end";
	}
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
