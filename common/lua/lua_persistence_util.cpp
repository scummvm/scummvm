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
 * This program is distri8buted in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

/**
 * This code is heavily based on the pluto code base. Copyright below
 */

/* Tamed Pluto - Heavy-duty persistence for Lua
 * Copyright (C) 2004 by Ben Sunshine-Hill, and released into the public
 * domain. People making use of this software as part of an application
 * are politely requested to email the author at sneftel@gmail.com
 * with a brief description of the application, primarily to satisfy his
 * curiosity.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * Instrumented by Stefan Reich (info@luaos.net)
 * for Mobile Lua (http://luaos.net/pages/mobile-lua.php)
 */

/**
 * This code is heavily based on the Pluto code base. Copyright below
 */

/* Tamed Pluto - Heavy-duty persistence for Lua
 * Copyright (C) 2004 by Ben Sunshine-Hill, and released into the public
 * domain. People making use of this software as part of an application
 * are politely requested to email the author at sneftel@gmail.com
 * with a brief description of the application, primarily to satisfy his
 * curiosity.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * Instrumented by Stefan Reich (info@luaos.net)
 * for Mobile Lua (http://luaos.net/pages/mobile-lua.php)
 */


#include "lua_persistence_util.h"

#include "common/scummsys.h"

#include "lobject.h"
#include "lstate.h"
#include "lgc.h"
#include "lopcodes.h"


namespace Lua {

void *lua_realloc(lua_State *luaState, void *block, size_t osize, size_t nsize) {
	global_State *globalState = G(luaState);

	block = (*globalState->frealloc)(globalState->ud, block, osize, nsize);
	globalState->totalbytes = (globalState->totalbytes - osize) + nsize;

	return block;
}

void pushObject(lua_State *luaState, TValue *obj) {
	setobj2s(luaState, luaState->top, obj);

	api_check(luaState, luaState->top < luaState->ci->top);
	luaState->top++;
}

void pushProto(lua_State *luaState, Proto *proto) {
	TValue obj;
	setptvalue(luaState, &obj, proto);

	pushObject(luaState, &obj);
}

void pushUpValue(lua_State *luaState, UpVal *upval) {
	TValue obj;

	obj.value.gc = cast(GCObject *, upval);
	obj.tt = LUA_TUPVAL;
	checkliveness(G(L), obj);

	pushObject(luaState, &obj);
}

void pushString(lua_State *luaState, TString *str) {
	TValue o;
	setsvalue(luaState, &o, str);

	pushObject(luaState, &o);
}

/* A simple reimplementation of the unfortunately static function luaA_index.
 * Does not support the global table, registry, or upvalues. */
StkId getObject(lua_State *luaState, int stackpos) {
	if (stackpos > 0) {
		lua_assert(luaState->base + stackpos - 1 < luaState->top);
		return luaState->base + stackpos - 1;
	} else {
		lua_assert(L->top - stackpos >= L->base);
		return luaState->top + stackpos;
	}
}

void lua_linkObjToGC(lua_State *luaState, GCObject *obj, lu_byte type) {
	global_State *globalState = G(luaState);

	obj->gch.next = globalState->rootgc;
	globalState->rootgc = obj;
	obj->gch.marked = luaC_white(globalState);
	obj->gch.tt = type;
}

Closure *lua_newLclosure(lua_State *luaState, int numElements, Table *elementTable) {
	Closure *c = (Closure *)lua_malloc(luaState, sizeLclosure(numElements));
	lua_linkObjToGC(luaState, obj2gco(c), LUA_TFUNCTION);

	c->l.isC = 0;
	c->l.env = elementTable;
	c->l.nupvalues = cast_byte(numElements);

	while (numElements--) {
		c->l.upvals[numElements] = NULL;
	}

	return c;
}

void pushClosure(lua_State *luaState, Closure *closure) {
	TValue obj;
	setclvalue(luaState, &obj, closure);
	pushObject(luaState, &obj);
}

Proto *createProto(lua_State *luaState) {
	Proto *newProto = (Proto *)lua_malloc(luaState, sizeof(Proto));
	lua_linkObjToGC(luaState, obj2gco(newProto), LUA_TPROTO);

	newProto->k = NULL;
	newProto->sizek = 0;
	newProto->p = NULL;
	newProto->sizep = 0;
	newProto->code = NULL;
	newProto->sizecode = 0;
	newProto->sizelineinfo = 0;
	newProto->sizeupvalues = 0;
	newProto->nups = 0;
	newProto->upvalues = NULL;
	newProto->numparams = 0;
	newProto->is_vararg = 0;
	newProto->maxstacksize = 0;
	newProto->lineinfo = NULL;
	newProto->sizelocvars = 0;
	newProto->locvars = NULL;
	newProto->linedefined = 0;
	newProto->lastlinedefined = 0;
	newProto->source = NULL;

	return newProto;
}

TString *createString(lua_State *luaState, const char *str, size_t len) {
	TString *res;
	lua_pushlstring(luaState, str, len);

	res = rawtsvalue(luaState->top - 1);
	lua_pop(luaState, 1);

	return res;
}

Proto *makeFakeProto(lua_State *L, lu_byte nups) {
	Proto *p = createProto(L);

	p->sizelineinfo = 1;
	p->lineinfo = lua_newVector(L, 1, int);
	p->lineinfo[0] = 1;
	p->sizecode = 1;
	p->code = lua_newVector(L, 1, Instruction);
	p->code[0] = CREATE_ABC(OP_RETURN, 0, 1, 0);
	p->source = createString(L, "", 0);
	p->maxstacksize = 2;
	p->nups = nups;
	p->sizek = 0;
	p->sizep = 0;

	return p;
}

UpVal *createUpValue(lua_State *luaState, int stackpos) {
	UpVal *upValue = (UpVal *)lua_malloc(luaState, sizeof(UpVal));
	lua_linkObjToGC(luaState, (GCObject *)upValue, LUA_TUPVAL);
	upValue->tt = LUA_TUPVAL;
	upValue->v = &upValue->u.value;
	upValue->u.l.prev = NULL;
	upValue->u.l.next = NULL;

	const TValue *o2 = (TValue *)getObject(luaState, stackpos);
	upValue->v->value = o2->value;
	upValue->v->tt = o2->tt;
	checkliveness(G(L), upValue->v);

	return upValue;
}

void unboxUpValue(lua_State *luaState) {
	// >>>>> ...... func
	LClosure *lcl;
	UpVal *uv;

	lcl = (LClosure *)clvalue(getObject(luaState, -1));
	uv = lcl->upvals[0];

	lua_pop(luaState, 1);
	// >>>>> ......

	pushUpValue(luaState, uv);
	// >>>>> ...... upValue
}

size_t appendStackToStack_reverse(lua_State *from, lua_State *to) {
	for (StkId id = from->top - 1; id >= from->stack; --id) {
		setobj2s(to, to->top, id);
		to->top++;
	}

	return from->top - from->stack;
}

void correctStack(lua_State *L, TValue *oldstack) {
	CallInfo *ci;
	GCObject *up;
	L->top = (L->top - oldstack) + L->stack;
	for (up = L->openupval; up != NULL; up = up->gch.next)
		gco2uv(up)->v = (gco2uv(up)->v - oldstack) + L->stack;
	for (ci = L->base_ci; ci <= L->ci; ci++) {
		ci->top = (ci->top - oldstack) + L->stack;
		ci->base = (ci->base - oldstack) + L->stack;
		ci->func = (ci->func - oldstack) + L->stack;
	}
	L->base = (L->base - oldstack) + L->stack;
}

void lua_reallocstack(lua_State *L, int newsize) {
	TValue *oldstack = L->stack;
	int realsize = newsize + 1 + EXTRA_STACK;

	lua_reallocvector(L, L->stack, L->stacksize, realsize, TValue);
	L->stacksize = realsize;
	L->stack_last = L->stack + newsize;
	correctStack(L, oldstack);
}

void lua_reallocCallInfo(lua_State *lauState, int newsize) {
	CallInfo *oldci = lauState->base_ci;
	lua_reallocvector(lauState, lauState->base_ci, lauState->size_ci, newsize, CallInfo);

	lauState->size_ci = newsize;
	lauState->ci = (lauState->ci - oldci) + lauState->base_ci;
	lauState->end_ci = lauState->base_ci + lauState->size_ci - 1;
}

void GCUnlink(lua_State *luaState, GCObject *gco) {
	GCObject *prevslot;
	if (G(luaState)->rootgc == gco) {
		G(luaState)->rootgc = G(luaState)->rootgc->gch.next;
		return;
	}

	prevslot = G(luaState)->rootgc;
	while (prevslot->gch.next != gco) {
		prevslot = prevslot->gch.next;
	}

	prevslot->gch.next = prevslot->gch.next->gch.next;
}

TString *lua_newlstr(lua_State *luaState, const char *str, size_t len) {
	lua_pushlstring(luaState, str, len);
	TString *luaStr = &(luaState->top - 1)->value.gc->ts;

	lua_pop(luaState, 1);

	return luaStr;
}

void lua_link(lua_State *luaState, GCObject *o, lu_byte tt) {
	global_State *g = G(luaState);
	o->gch.next = g->rootgc;
	g->rootgc = o;
	o->gch.marked = luaC_white(g);
	o->gch.tt = tt;
}

Proto *lua_newproto(lua_State *luaState) {
	Proto *f = (Proto *)lua_malloc(luaState, sizeof(Proto));
	lua_link(luaState, obj2gco(f), LUA_TPROTO);
	f->k = NULL;
	f->sizek = 0;
	f->p = NULL;
	f->sizep = 0;
	f->code = NULL;
	f->sizecode = 0;
	f->sizelineinfo = 0;
	f->sizeupvalues = 0;
	f->nups = 0;
	f->upvalues = NULL;
	f->numparams = 0;
	f->is_vararg = 0;
	f->maxstacksize = 0;
	f->lineinfo = NULL;
	f->sizelocvars = 0;
	f->locvars = NULL;
	f->linedefined = 0;
	f->lastlinedefined = 0;
	f->source = NULL;
	return f;
}

UpVal *makeUpValue(lua_State *luaState, int stackPos) {
	UpVal *uv = lua_new(luaState, UpVal);
	lua_link(luaState, (GCObject *)uv, LUA_TUPVAL);
	uv->tt = LUA_TUPVAL;
	uv->v = &uv->u.value;
	uv->u.l.prev = NULL;
	uv->u.l.next = NULL;

	setobj(luaState, uv->v, getObject(luaState, stackPos));

	return uv;
}

void boxUpValue_start(lua_State *luaState) {
	LClosure *closure;
	closure = (LClosure *)lua_newLclosure(luaState, 1, hvalue(&luaState->l_gt));
	pushClosure(luaState, (Closure *)closure);
	// >>>>> ...... func
	closure->p = makeFakeProto(luaState, 1);

	// Temporarily initialize the upvalue to nil
	lua_pushnil(luaState);
	closure->upvals[0] = makeUpValue(luaState, -1);
	lua_pop(luaState, 1);
}

void boxUpValue_finish(lua_State *luaState) {
	// >>>>> ...... func obj
	LClosure *lcl = (LClosure *)clvalue(getObject(luaState, -2));

	lcl->upvals[0]->u.value = *getObject(luaState, -1);
	lua_pop(luaState, 1);
	// >>>>> ...... func
}

} // End of namespace Lua
