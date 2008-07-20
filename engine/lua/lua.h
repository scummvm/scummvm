/*
** $Id$
** Lua - An Extensible Extension Language
** TeCGraf: Grupo de Tecnologia em Computacao Grafica, PUC-Rio, Brazil
** e-mail: lua@tecgraf.puc-rio.br
** www: http://www.tecgraf.puc-rio.br/lua/
** See Copyright Notice at the end of this file
*/

#include "common\sys.h"

#ifndef lua_h
#define lua_h

#define LUA_VERSION	"Lua 3.1"
#define LUA_COPYRIGHT	"Copyright (C) 1994-1998 TeCGraf, PUC-Rio"
#define LUA_AUTHORS 	"W. Celes, R. Ierusalimschy & L. H. de Figueiredo"

#define LUA_NOOBJECT  0

#define LUA_ANYTAG    (-1)

typedef void (*lua_CFunction)(void);
typedef uint32 lua_Object;

typedef struct lua_State lua_State;
extern lua_State *lua_state;

typedef void (*SaveRestoreFunc)(void *, int32);
typedef int32 (*SaveRestoreCallback)(int32, int32, SaveRestoreFunc);

extern SaveRestoreCallback saveCallback;
extern SaveRestoreCallback restoreCallback;

void lua_Save(SaveRestoreFunc);
void lua_Restore(SaveRestoreFunc);

void lua_removelibslists(void);

void	       lua_open			(void);
void           lua_close		(void);
lua_State      *lua_setstate		(lua_State *st);

lua_Object     lua_settagmethod	(int32 tag, const char *event); /* In: new method */
lua_Object     lua_gettagmethod	(int32 tag, const char *event);
lua_Object     lua_seterrormethod (void);  /* In: new method */

int32            lua_newtag		(void);
int32            lua_copytagmethods	(int32 tagto, int32 tagfrom);
void           lua_settag		(int32 tag); /* In: object */

void           lua_error		(const char *s);
int32            lua_dofile 		(const char *filename); /* Out: returns */
int32            lua_dostring 		(const char *string); /* Out: returns */
int32            lua_dobuffer		(const char *buff, int32 size, const char *name);
					  /* Out: returns */
int32            lua_callfunction		(lua_Object f);
					  /* In: parameters; Out: returns */

void	       lua_beginblock		(void);
void	       lua_endblock		(void);

lua_Object     lua_lua2C 		(int32 number);
#define	       lua_getparam(_)		lua_lua2C(_)
#define	       lua_getresult(_)		lua_lua2C(_)

int32            lua_isnil                (lua_Object object);
int32            lua_istable              (lua_Object object);
int32            lua_isuserdata           (lua_Object object);
int32            lua_iscfunction          (lua_Object object);
int32            lua_isnumber             (lua_Object object);
int32            lua_isstring             (lua_Object object);
int32            lua_isfunction           (lua_Object object);

double         lua_getnumber 		(lua_Object object);
const char          *lua_getstring 		(lua_Object object);
int32           lua_strlen 		(lua_Object object);
lua_CFunction  lua_getcfunction 	(lua_Object object);
void	      *lua_getuserdata		(lua_Object object);


void 	       lua_pushnil 		(void);
void           lua_pushnumber 		(double n);
void           lua_pushlstring		(const char *s, int32 len);
void           lua_pushstring 		(const char *s);
void           lua_pushcclosure		(lua_CFunction fn, int32 n);
void           lua_pushusertag          (void *u, int32 tag);
void           lua_pushobject       	(lua_Object object);

lua_Object     lua_pop			(void);

lua_Object     lua_getglobal 		(const char *name);
lua_Object     lua_rawgetglobal		(const char *name);
void           lua_setglobal		(const char *name); /* In: value */
void           lua_rawsetglobal		(const char *name); /* In: value */

void           lua_settable	(void); /* In: table, index, value */
void           lua_rawsettable	(void); /* In: table, index, value */
lua_Object     lua_gettable 		(void); /* In: table, index */
lua_Object     lua_rawgettable		(void); /* In: table, index */

int32            lua_tag			(lua_Object object);


int32            lua_ref			(int32 lock); /* In: value */
lua_Object     lua_getref		(int32 ref);
void	       lua_unref		(int32 ref);

lua_Object     lua_createtable		(void);

int32	       lua_collectgarbage	(int32 limit);

void	       lua_runtasks		(void);

void current_script (void);

/* =============================================================== */
/* some useful macros/derived functions */

int32     (lua_call) (char *name);
#define lua_call(name)		lua_callfunction(lua_getglobal(name))

void    (lua_pushref) (int32 ref);
#define lua_pushref(ref)	lua_pushobject(lua_getref(ref))

int32     (lua_refobject) (lua_Object o, int32 l);
#define lua_refobject(o,l)	(lua_pushobject(o), lua_ref(l))

void    (lua_register) (char *n, lua_CFunction f);
#define lua_register(n,f)	(lua_pushcfunction(f), lua_setglobal(n))

void    (lua_pushuserdata) (void *u);
#define lua_pushuserdata(u)     lua_pushusertag(u, 0)

void    (lua_pushcfunction) (lua_CFunction f);
#define lua_pushcfunction(f)	lua_pushcclosure(f, 0)

int32     (lua_clonetag) (int32 t);
#define lua_clonetag(t)		lua_copytagmethods(lua_newtag(), (t))


/* ==========================================================================
** for compatibility with old versions. Avoid using these macros/functions
** If your program does need any of these, define LUA_COMPAT2_5
*/
//#define LUA_COMPAT2_5

#ifdef LUA_COMPAT2_5


lua_Object     lua_setfallback		(char *event, lua_CFunction fallback);

#define lua_storeglobal		lua_setglobal
#define lua_type		lua_tag

#define lua_lockobject(o)  lua_refobject(o,1)
#define	lua_lock() lua_ref(1)
#define lua_getlocked lua_getref
#define	lua_pushlocked lua_pushref
#define	lua_unlock lua_unref

#define lua_pushliteral(o)  lua_pushstring(o)

#define lua_getindexed(o,n) (lua_pushobject(o), lua_pushnumber(n), lua_gettable())
#define lua_getfield(o,f)   (lua_pushobject(o), lua_pushstring(f), lua_gettable())

#define lua_copystring(o) (strdup(lua_getstring(o)))

#define lua_getsubscript  lua_gettable
#define lua_storesubscript  lua_settable

#endif

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
* This implementation contains no third-party code.
******************************************************************************/
