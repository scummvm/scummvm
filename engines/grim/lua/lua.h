/*
** Lua - An Extensible Extension Language
** TeCGraf: Grupo de Tecnologia em Computacao Grafica, PUC-Rio, Brazil
** e-mail: lua@tecgraf.puc-rio.br
** www: http://www.tecgraf.puc-rio.br/lua/
** See Copyright Notice at the end of this file
*/

#ifndef GRIM_LUA_H
#define GRIM_LUA_H

#include "common/scummsys.h"
#include "common/str.h"

namespace Common {
	class String;
	class SeekableReadStream;
	class WriteStream;
	class File;
}

namespace Grim {

#define LUA_VERSION		"Lua 3.1 (alpha)"
#define LUA_COPYRIGHT	"Copyright (C) 1994-1998 TeCGraf, PUC-Rio"
#define LUA_AUTHORS 	"W. Celes, R. Ierusalimschy & L. H. de Figueiredo"

#define LUA_NOOBJECT  0

#define LUA_ANYTAG    (-1)

typedef void (*lua_CFunction)();
typedef uint32 lua_Object;

struct PointerId {
	uint64 id;
};

PointerId makeIdFromPointer(void *ptr);
void *makePointerFromId(PointerId ptr);

class LuaFile {
public:
	Common::String _name;
	Common::String _filename;
	Common::SeekableReadStream *_in;
	Common::WriteStream *_out;
	bool _stdin, _stdout, _stderr;

public:
	LuaFile();
	~LuaFile();

	void close();
	bool isOpen() const;
	uint32 read(void *buf, uint32 len);
	uint32 write(const char *buf, uint32 len);
	void seek(int32 pos, int whence = 0);
};

class SaveGame;
void lua_Save(SaveGame *state);
void lua_Restore(SaveGame *state);

void lua_removelibslists();

void lua_open();
void lua_close();
bool lua_isopen();

lua_Object lua_settagmethod(int32 tag, const char *event); // In: new method
lua_Object lua_gettagmethod(int32 tag, const char *event);
lua_Object lua_seterrormethod();  // In: new method

int32 lua_newtag();
int32 lua_copytagmethods(int32 tagto, int32 tagfrom);
void lua_settag(int32 tag); // In: object

void lua_error(const char *s);
int32 lua_dostring(const char *string); // Out: returns
int32 lua_dobuffer(const char *buff, int32 size, const char *name);
int32 lua_callfunction(lua_Object f);
// In: parameters; Out: returns */

void lua_beginblock();
void lua_endblock();

lua_Object lua_lua2C(int32 number);

#define lua_getparam(_)		lua_lua2C(_)
#define	lua_getresult(_)	lua_lua2C(_)

int32 lua_isnil                (lua_Object object);
int32 lua_istable              (lua_Object object);
int32 lua_isuserdata           (lua_Object object);
int32 lua_iscfunction          (lua_Object object);
int32 lua_isnumber             (lua_Object object);
int32 lua_isstring             (lua_Object object);
int32 lua_isfunction           (lua_Object object);

float lua_getnumber 		(lua_Object object);
const char *lua_getstring 		(lua_Object object);
lua_CFunction lua_getcfunction 	(lua_Object object);
int32 lua_getuserdata		(lua_Object object);

void lua_pushnil();
void lua_pushnumber(float n);
void lua_pushstring(const char *s);
void lua_pushCclosure(lua_CFunction fn, int32 n);
void lua_pushusertag(int32 id, int32 tag);
void lua_pushobject(lua_Object object);

lua_Object lua_pop();
lua_Object lua_getglobal(const char *name);
lua_Object lua_rawgetglobal(const char *name);
void lua_setglobal(const char *name); // In: value
void lua_rawsetglobal(const char *name); // In: value

void lua_settable(); // In: table, index, value
void lua_rawsettable(); // In: table, index, value
lua_Object lua_gettable(); // In: table, index
lua_Object lua_rawgettable(); // In: table, index

int32 lua_tag(lua_Object object);

int32 lua_ref(int32 lock); // In: value
lua_Object lua_getref(int32 ref);
void lua_unref(int32 ref);

lua_Object lua_createtable();
int32 lua_collectgarbage(int32 limit);

void lua_runtasks();
void current_script();

/* some useful macros/derived functions */

#define lua_call(name)		lua_callfunction(lua_getglobal(name))

#define lua_pushref(ref)	lua_pushobject(lua_getref(ref))

#define lua_refobject(o,l)	(lua_pushobject(o), lua_ref(l))

#define lua_register(n, f)	(lua_pushcfunction(f), lua_setglobal(n))

#define lua_pushuserdata(u)	lua_pushusertag(u, 0)

#define lua_pushcfunction(f)	lua_pushCclosure(f, 0)

#define lua_clonetag(t)		lua_copytagmethods(lua_newtag(), (t))

/* ==========================================================================
** for compatibility with old versions. Avoid using these macros/functions
** If your program does need any of these, define LUA_COMPAT2_5
*/
#define LUA_COMPAT2_5

#ifdef LUA_COMPAT2_5

lua_Object lua_setfallback(const char *event, lua_CFunction fallback);

#define lua_storeglobal		lua_setglobal
#define lua_type			lua_tag

#define lua_lockobject(o)	lua_refobject(o,1)
#define	lua_lock()			lua_ref(1)
#define lua_getlocked		lua_getref
#define	lua_pushlocked		lua_pushref
#define	lua_unlock			lua_unref
#define lua_pushliteral(o)  lua_pushstring(o)
#define lua_getindexed(o, n) (lua_pushobject(o), lua_pushnumber(n), lua_gettable())
#define lua_getfield(o, f)  (lua_pushobject(o), lua_pushstring(f), lua_gettable())
#define lua_copystring(o)	(strdup(lua_getstring(o)))
#define lua_getsubscript	lua_gettable
#define lua_storesubscript  lua_settable

#endif


/******************************************************************************
* Copyright (c) 1994-1998 TeCGraf, PUC-Rio.  All rights reserved.
*
* Permission is hereby granted, without written agreement and without license
* or royalty fees, to use, copy, modify, and distribute this software and its
* documentation for any purpose, including commercial applications, subject to
* the following conditions:
*
*  - The above copyright notice and this permission notice shall appear in all
*    copies or substantial portions of this software.
*
*  - The origin of this software must not be misrepresented; you must not
*    claim that you wrote the original software. If you use this software in a
*    product, an acknowledgment in the product documentation would be greatly
*    appreciated (but it is not required).
*
*  - Altered source versions must be plainly marked as such, and must not be
*    misrepresented as being the original software.
*
* The authors specifically disclaim any warranties, including, but not limited
* to, the implied warranties of merchantability and fitness for a particular
* purpose.  The software provided hereunder is on an "as is" basis, and the
* authors have no obligation to provide maintenance, support, updates,
* enhancements, or modifications.  In no event shall TeCGraf, PUC-Rio, or the
* authors be held liable to any party for direct, indirect, special,
* incidental, or consequential damages arising out of the use of this software
* and its documentation.
*
* The Lua language and this implementation have been entirely designed and
* written by Waldemar Celes Filho, Roberto Ierusalimschy and
* Luiz Henrique de Figueiredo at TeCGraf, PUC-Rio.
*
* NOTE: This implementation of Lua contains additional code and
* modifications added only for ScummVM project.
* Look into the source code file "Changelog" for more info.
******************************************************************************/

} // end of namespace Grim

#endif
