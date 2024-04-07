#include "StdAfx.h"
#include "LuaExec.h"

extern "C" {
#include "lua\src\lua.h"
#include "lua\src\lualib.h"
#include "lua\src\lauxlib.h"
};

LuaExec::LuaExec()
{
	lua=lua_open();
	luaL_openlibs(lua);
}

LuaExec::~LuaExec()
{
	lua_close(lua);
}

bool LuaExec::open(const char* buffer,int size,const char* chunk_name)
{
	if(luaL_loadbuffer(lua, buffer, size, chunk_name))
	{
		printf(lua_tostring(lua, -1));
		return false;
	}
	lua_pushvalue(lua,1);
	return true;
}

void LuaExec::set(const char* name,float value)
{
	lua_pushnumber(lua,value); 
	lua_setglobal (lua,name);
}

void LuaExec::set(const char* name,int value)
{
	lua_pushinteger(lua,value); 
	lua_setglobal (lua,name);
}

void LuaExec::set(const char* name,const char* value)
{
	lua_pushstring(lua,value); 
	lua_setglobal (lua,name);
}

string LuaExec::run()
{
	int old_idx=lua_gettop(lua);

	if(lua_pcall(lua, 0, 1, 0))
	{
		printf(lua_tostring(lua, -1));
		exit(1);
		return "";
	}

	string s = lua_tostring(lua,-1);
	lua_pop(lua,1);
	lua_pushvalue(lua,1);
	int idx=lua_gettop(lua);
	if(idx!=old_idx)
	{
		printf("Stack not correct\n");
		exit(1);
	}
	return s;
}
