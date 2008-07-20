/*
** $Id$
** Auxiliary functions for building Lua libraries
** See Copyright Notice in lua.h
*/


/* Please Notice: This file uses only the official API of Lua
** Any function declared here could be written as an application
** function. With care, these functions can be used by other libraries.
*/
#include "lauxlib.h"
#include "lua.h"
#include "luadebug.h"
#include "lmem.h"


int32 luaL_findstring (const char *name, const char *list[]) {
  int32 i;
  for (i=0; list[i]; i++)
    if (strcmp(list[i], name) == 0)
      return i;
  return -1;  /* name not found */
}

void luaL_argerror (int32 numarg, const char *extramsg)
{
  const char *funcname;
  lua_getobjname(lua_stackedfunction(0), &funcname);
  if (funcname == NULL)
    funcname = "???";
  if (extramsg == NULL)
    luaL_verror("bad argument #%d to function `%.50s'", numarg, funcname);
  else
    luaL_verror("bad argument #%d to function `%.50s' (%.100s)",
                    numarg, funcname, extramsg);
}

const char *luaL_check_lstr (int32 numArg, int32 *len)
{
  lua_Object o = lua_getparam(numArg);
  luaL_arg_check(lua_isstring(o), numArg, "string expected");
  if (len) *len = lua_strlen(o);
  return lua_getstring(o);
}

const char *luaL_opt_lstr (int32 numArg, const char *def, int32 *len)
{
  return (lua_getparam(numArg) == LUA_NOOBJECT) ? def :
                              luaL_check_lstr(numArg, len);
}

double luaL_check_number (int32 numArg)
{
  lua_Object o = lua_getparam(numArg);
  luaL_arg_check(lua_isnumber(o), numArg, "number expected");
  return lua_getnumber(o);
}


double luaL_opt_number (int32 numArg, double def)
{
  return (lua_getparam(numArg) == LUA_NOOBJECT) ? def :
                              luaL_check_number(numArg);
}  


lua_Object luaL_tablearg (int32 arg)
{
  lua_Object o = lua_getparam(arg);
  luaL_arg_check(lua_istable(o), arg, "table expected");
  return o;
}

lua_Object luaL_functionarg (int32 arg)
{
  lua_Object o = lua_getparam(arg);
  luaL_arg_check(lua_isfunction(o), arg, "function expected");
  return o;
}

lua_Object luaL_nonnullarg (int32 numArg)
{
  lua_Object o = lua_getparam(numArg);
  luaL_arg_check(o != LUA_NOOBJECT, numArg, "value expected");
  return o;
}

luaL_libList *list_of_libs = NULL;

void luaL_addlibtolist(luaL_reg *l, int32 n) {
  luaL_libList *list = (luaL_libList *)luaM_malloc(sizeof(luaL_libList));
  list->list = l;
  list->number = n;
  list->next = list_of_libs;
  list_of_libs = list;
}

void lua_removelibslists(void) {
  luaL_libList *list = list_of_libs;
  while (list) {
    luaL_libList *nextList = list->next;
    luaM_free(list);
    list = nextList;
  }
}

void luaL_openlib (struct luaL_reg *l, int32 n)
{
  int32 i;
  lua_open();  /* make sure lua is already open */
  for (i=0; i<n; i++)
    lua_register(l[i].name, l[i].func);
  luaL_addlibtolist(l, n);
}


void luaL_verror (const char *fmt, ...)
{
  char buff[500];
  va_list argp;
  va_start(argp, fmt);
  vsprintf(buff, fmt, argp);
  va_end(argp);
  lua_error(buff);
}

