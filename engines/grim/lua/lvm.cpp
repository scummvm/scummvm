/*
** Lua virtual machine
** See Copyright Notice in lua.h
*/

#define FORBIDDEN_SYMBOL_EXCEPTION_setjmp
#define FORBIDDEN_SYMBOL_EXCEPTION_longjmp

#include "engines/grim/lua/lauxlib.h"
#include "engines/grim/lua/ldo.h"
#include "engines/grim/lua/lfunc.h"
#include "engines/grim/lua/lgc.h"
#include "engines/grim/lua/lmem.h"
#include "engines/grim/lua/lopcodes.h"
#include "engines/grim/lua/lstate.h"
#include "engines/grim/lua/lstring.h"
#include "engines/grim/lua/ltable.h"
#include "engines/grim/lua/ltask.h"
#include "engines/grim/lua/ltm.h"
#include "engines/grim/lua/luadebug.h"
#include "engines/grim/lua/lvm.h"

namespace Grim {

#define skip_word(pc)	(pc += 2)
#define get_word(pc)	((*((pc) + 1) << 8)|(*(pc)))
#define next_word(pc)   (pc += 2, get_word(pc - 2))

#define	EXTRA_STACK	5

static TaggedString *strconc(char *l, char *r) {
	size_t nl = strlen(l);
	char *buffer = luaL_openspace(nl + strlen(r) + 1);
	strcpy(buffer, l);
	strcpy(buffer + nl, r);
	return luaS_new(buffer);
}

int32 luaV_tonumber (TObject *obj) { // LUA_NUMBER
	double t;
	char c;

	if (ttype(obj) != LUA_T_STRING)
		return 1;
	else if (sscanf(svalue(obj), "%lf %c", &t, &c) == 1) {
		nvalue(obj) = (float)t;
		ttype(obj) = LUA_T_NUMBER;
		return 0;
	} else
		return 2;
}

int32 luaV_tostring (TObject *obj) { // LUA_NUMBER
	if (ttype(obj) != LUA_T_NUMBER)
		return 1;
	else {
		char s[60];
		float f = nvalue(obj);
		int32 i;
		if ((float)(-MAX_INT) <= f && f <= (float)MAX_INT && (float)(i = (int32)f) == f)
			sprintf (s, "%d", (int)i);
		else
			sprintf (s, "%g", (double)nvalue(obj));
		tsvalue(obj) = luaS_new(s);
		ttype(obj) = LUA_T_STRING;
		return 0;
	}
}

void luaV_closure(int32 nelems) {
	if (nelems > 0) {
		Stack *S = &lua_state->stack;
		Closure *c = luaF_newclosure(nelems);
		c->consts[0] = *(S->top - 1);
		memcpy(&c->consts[1], S->top - (nelems + 1), nelems * sizeof(TObject));
		S->top -= nelems;
		ttype(S->top - 1) = LUA_T_CLOSURE;
		(S->top - 1)->value.cl = c;
	}
}

/*
** Function to index a table.
** Receives the table at top-2 and the index at top-1.
*/
void luaV_gettable() {
	Stack *S = &lua_state->stack;
	TObject *im;
	if (ttype(S->top - 2) != LUA_T_ARRAY)  // not a table, get "gettable" method
		im = luaT_getimbyObj(S->top - 2, IM_GETTABLE);
	else {  // object is a table...
		int32 tg = (S->top - 2)->value.a->htag;
		im = luaT_getim(tg, IM_GETTABLE);
		if (ttype(im) == LUA_T_NIL) {  // and does not have a "gettable" method
			TObject *h = luaH_get(avalue(S->top - 2), S->top - 1);
			if (h && ttype(h) != LUA_T_NIL) {
				--S->top;
				*(S->top - 1) = *h;
			} else if (ttype(im = luaT_getim(tg, IM_INDEX)) != LUA_T_NIL)
				luaD_callTM(im, 2, 1);
			else {
				--S->top;
				ttype(S->top - 1) = LUA_T_NIL;
			}
			return;
		}
		// else it has a "gettable" method, go through to next command
	}
	// object is not a table, or it has a "gettable" method
	if (ttype(im) != LUA_T_NIL)
		luaD_callTM(im, 2, 1);
	else
		lua_error("indexed expression not a table");
}

/*
** Function to store indexed based on values at the stack.top
** mode = 0: raw store (without tag methods)
** mode = 1: normal store (with tag methods)
** mode = 2: "deep lua_state->stack.stack" store (with tag methods)
*/
void luaV_settable(TObject *t, int32 mode) {
	struct Stack *S = &lua_state->stack;
	TObject *im = (mode == 0) ? nullptr : luaT_getimbyObj(t, IM_SETTABLE);
	if (ttype(t) == LUA_T_ARRAY && (!im || ttype(im) == LUA_T_NIL)) {
		TObject *h = luaH_set(avalue(t), t + 1);
		*h = *(S->top - 1);
		S->top -= (mode == 2) ? 1 : 3;
	} else {  // object is not a table, and/or has a specific "settable" method
		if (im && ttype(im) != LUA_T_NIL) {
			if (mode == 2) {
				*(S->top + 1) = *(lua_state->stack.top - 1);
				*(S->top) = *(t + 1);
				*(S->top - 1) = *t;
				S->top += 2;  // WARNING: caller must assure stack space
			}
			luaD_callTM(im, 3, 0);
		} else
			lua_error("indexed expression not a table");
	}
}

void luaV_getglobal(TaggedString *ts) {
	// WARNING: caller must assure stack space
	TObject *value = &ts->globalval;
	TObject *im = luaT_getimbyObj(value, IM_GETGLOBAL);
	if (ttype(im) == LUA_T_NIL) {  // default behavior
		*lua_state->stack.top++ = *value;
	} else {
		Stack *S = &lua_state->stack;
		ttype(S->top) = LUA_T_STRING;
		tsvalue(S->top) = ts;
		S->top++;
		*S->top++ = *value;
		luaD_callTM(im, 2, 1);
	}
}

void luaV_setglobal(TaggedString *ts) {
	TObject *oldvalue = &ts->globalval;
	TObject *im = luaT_getimbyObj(oldvalue, IM_SETGLOBAL);
	if (ttype(im) == LUA_T_NIL)  // default behavior */
		luaS_rawsetglobal(ts, --lua_state->stack.top);
	else {
		// WARNING: caller must assure stack space
		Stack *S = &lua_state->stack;
		TObject newvalue = *(S->top - 1);
		ttype(S->top - 1) = LUA_T_STRING;
		tsvalue(S->top - 1) = ts;
		*S->top++ = *oldvalue;
		*S->top++ = newvalue;
		luaD_callTM(im, 3, 0);
	}
}

static void call_binTM(IMS event, const char *msg) {
	TObject *im = luaT_getimbyObj(lua_state->stack.top - 2, event); // try first operand
	if (ttype(im) == LUA_T_NIL) {
		im = luaT_getimbyObj(lua_state->stack.top - 1, event);  // try second operand
		if (ttype(im) == LUA_T_NIL) {
			im = luaT_getim(0, event);  // try a 'global' i.m.
			if (ttype(im) == LUA_T_NIL)
				lua_error(msg);
		}
	}
	lua_pushstring(luaT_eventname[event]);
	luaD_callTM(im, 3, 1);
}


static void call_arith(IMS event) {
	call_binTM(event, "unexpected type in arithmetic operation");
}

static void comparison(lua_Type ttype_less, lua_Type ttype_equal, lua_Type ttype_great, IMS op) {
	Stack *S = &lua_state->stack;
	TObject *l = S->top-2;
	TObject *r = S->top-1;
	int32 result;
	if (ttype(l) == LUA_T_NUMBER && ttype(r) == LUA_T_NUMBER)
		result = (nvalue(l) < nvalue(r)) ? -1 : (nvalue(l) == nvalue(r)) ? 0 : 1;
	else if (ttype(l) == LUA_T_STRING && ttype(r) == LUA_T_STRING)
		result = strcoll(svalue(l), svalue(r));
	else {
		call_binTM(op, "unexpected type in comparison");
		return;
	}
	S->top--;
	nvalue(S->top - 1) = 1;
	ttype(S->top - 1) = (result < 0) ? ttype_less : (result == 0) ? ttype_equal : ttype_great;
}

void luaV_pack(StkId firstel, int32 nvararg, TObject *tab) {
	TObject *firstelem = lua_state->stack.stack + firstel;
	int32 i;
	if (nvararg < 0)
		nvararg = 0;
	avalue(tab) = luaH_new(nvararg + 1);  // +1 for field 'n'
	ttype(tab) = LUA_T_ARRAY;
	for (i = 0; i < nvararg; i++) {
		TObject index;
		ttype(&index) = LUA_T_NUMBER;
		nvalue(&index) = (float)i + 1;
		*(luaH_set(avalue(tab), &index)) = *(firstelem + i);
	}
	// store counter in field "n" */
	{
		TObject index, extra;
		ttype(&index) = LUA_T_STRING;
		tsvalue(&index) = luaS_new("n");
		ttype(&extra) = LUA_T_NUMBER;
		nvalue(&extra) = (float)nvararg;
		*(luaH_set(avalue(tab), &index)) = extra;
	}
}

static void adjust_varargs(StkId first_extra_arg) {
	TObject arg;
	luaV_pack(first_extra_arg, (lua_state->stack.top - lua_state->stack.stack) - first_extra_arg, &arg);
	luaD_adjusttop(first_extra_arg);
	*lua_state->stack.top++ = arg;
}

StkId luaV_execute(lua_Task *task) {
	if (!task->some_flag) {
		luaD_checkstack((*task->pc++) + EXTRA_STACK);
		if (*task->pc < ZEROVARARG) {
			luaD_adjusttop(task->base + *(task->pc++));
		} else {
			luaC_checkGC();
			adjust_varargs(task->base + (*task->pc++) - ZEROVARARG);
		}
		task->some_flag = 1;
	}
	lua_state->state_counter2++;

	while (1) {
		switch ((OpCode)(task->aux = *task->pc++)) {
		case PUSHNIL0:
			ttype(task->S->top++) = LUA_T_NIL;
			break;
		case PUSHNIL:
			task->aux = *task->pc++;
			do {
				ttype(task->S->top++) = LUA_T_NIL;
			} while (task->aux--);
			break;
		case PUSHNUMBER:
			task->aux = *task->pc++;
			goto pushnumber;
		case PUSHNUMBERW:
			task->aux = next_word(task->pc);
			goto pushnumber;
		case PUSHNUMBER0:
		case PUSHNUMBER1:
		case PUSHNUMBER2:
			task->aux -= PUSHNUMBER0;
pushnumber:
			ttype(task->S->top) = LUA_T_NUMBER;
			nvalue(task->S->top) = (float)task->aux;
			task->S->top++;
			break;
		case PUSHLOCAL:
			task->aux = *task->pc++;
			goto pushlocal;
		case PUSHLOCAL0:
		case PUSHLOCAL1:
		case PUSHLOCAL2:
		case PUSHLOCAL3:
		case PUSHLOCAL4:
		case PUSHLOCAL5:
		case PUSHLOCAL6:
		case PUSHLOCAL7:
			task->aux -= PUSHLOCAL0;
pushlocal:
			*task->S->top++ = *((task->S->stack + task->base) + task->aux);
			break;
		case GETGLOBALW:
			task->aux = next_word(task->pc);
			goto getglobal;
		case GETGLOBAL:
			task->aux = *task->pc++;
			goto getglobal;
		case GETGLOBAL0:
		case GETGLOBAL1:
		case GETGLOBAL2:
		case GETGLOBAL3:
		case GETGLOBAL4:
		case GETGLOBAL5:
		case GETGLOBAL6:
		case GETGLOBAL7:
			task->aux -= GETGLOBAL0;
getglobal:
			luaV_getglobal(tsvalue(&task->consts[task->aux]));
			break;
		case GETTABLE:
			luaV_gettable();
			break;
		case GETDOTTEDW:
			task->aux = next_word(task->pc); goto getdotted;
		case GETDOTTED:
			task->aux = *task->pc++;
			goto getdotted;
		case GETDOTTED0:
		case GETDOTTED1:
		case GETDOTTED2:
		case GETDOTTED3:
		case GETDOTTED4:
		case GETDOTTED5:
		case GETDOTTED6:
		case GETDOTTED7:
			task->aux -= GETDOTTED0;
getdotted:
			*task->S->top++ = task->consts[task->aux];
			luaV_gettable();
			break;
		case PUSHSELFW:
			task->aux = next_word(task->pc);
			goto pushself;
		case PUSHSELF:
			task->aux = *task->pc++;
			goto pushself;
		case PUSHSELF0:
		case PUSHSELF1:
		case PUSHSELF2:
		case PUSHSELF3:
		case PUSHSELF4:
		case PUSHSELF5:
		case PUSHSELF6:
		case PUSHSELF7:
			task->aux -= PUSHSELF0;
pushself:
			{
				TObject receiver = *(task->S->top - 1);
				*task->S->top++ = task->consts[task->aux];
				luaV_gettable();
				*task->S->top++ = receiver;
				break;
			}
		case PUSHCONSTANTW:
			task->aux = next_word(task->pc);
			goto pushconstant;
		case PUSHCONSTANT:
			task->aux = *task->pc++; goto pushconstant;
		case PUSHCONSTANT0:
		case PUSHCONSTANT1:
		case PUSHCONSTANT2:
		case PUSHCONSTANT3:
		case PUSHCONSTANT4:
		case PUSHCONSTANT5:
		case PUSHCONSTANT6:
		case PUSHCONSTANT7:
			task->aux -= PUSHCONSTANT0;
pushconstant:
			*task->S->top++ = task->consts[task->aux];
			break;
		case PUSHUPVALUE:
			task->aux = *task->pc++;
			goto pushupvalue;
		case PUSHUPVALUE0:
		case PUSHUPVALUE1:
			task->aux -= PUSHUPVALUE0;
pushupvalue:
			*task->S->top++ = task->cl->consts[task->aux + 1];
			break;
		case SETLOCAL:
			task->aux = *task->pc++;
			goto setlocal;
		case SETLOCAL0:
		case SETLOCAL1:
		case SETLOCAL2:
		case SETLOCAL3:
		case SETLOCAL4:
		case SETLOCAL5:
		case SETLOCAL6:
		case SETLOCAL7:
			task->aux -= SETLOCAL0;
setlocal:
			*((task->S->stack + task->base) + task->aux) = *(--task->S->top);
			break;
		case SETGLOBALW:
			task->aux = next_word(task->pc);
			goto setglobal;
		case SETGLOBAL:
			task->aux = *task->pc++;
			goto setglobal;
		case SETGLOBAL0:
		case SETGLOBAL1:
		case SETGLOBAL2:
		case SETGLOBAL3:
		case SETGLOBAL4:
		case SETGLOBAL5:
		case SETGLOBAL6:
		case SETGLOBAL7:
			task->aux -= SETGLOBAL0;
setglobal:
			luaV_setglobal(tsvalue(&task->consts[task->aux]));
			break;
		case SETTABLE0:
			luaV_settable(task->S->top - 3, 1);
			break;
		case SETTABLE:
			luaV_settable(task->S->top - 3 - (*task->pc++), 2);
			break;
		case SETLISTW:
			task->aux = next_word(task->pc);
			task->aux *= LFIELDS_PER_FLUSH;
			goto setlist;
		case SETLIST:
			task->aux = *(task->pc++) * LFIELDS_PER_FLUSH;
			goto setlist;
		case SETLIST0:
			task->aux = 0;
setlist:
			{
				int32 n = *(task->pc++);
				TObject *arr = task->S->top - n - 1;
				for (; n; n--) {
					ttype(task->S->top) = LUA_T_NUMBER;
					nvalue(task->S->top) = (float)(n + task->aux);
					*(luaH_set(avalue(arr), task->S->top)) = *(task->S->top - 1);
					task->S->top--;
			}
			break;
		}
		case SETMAP0:
			task->aux = 0;
			goto setmap;
		case SETMAP:
			task->aux = *task->pc++;
setmap:
			{
				TObject *arr = task->S->top - (2 * task->aux) - 3;
				do {
					*(luaH_set(avalue(arr), task->S->top - 2)) = *(task->S->top - 1);
					task->S->top -= 2;
				} while (task->aux--);
				break;
			}
		case POP:
			task->aux = *task->pc++;
			goto pop;
		case POP0:
		case POP1:
			task->aux -= POP0;
pop:
			task->S->top -= (task->aux + 1);
			break;
		case CREATEARRAYW:
			task->aux = next_word(task->pc);
			goto createarray;
		case CREATEARRAY0:
		case CREATEARRAY1:
			task->aux -= CREATEARRAY0;
			goto createarray;
		case CREATEARRAY:
			task->aux = *task->pc++;
createarray:
			luaC_checkGC();
			avalue(task->S->top) = luaH_new(task->aux);
			ttype(task->S->top) = LUA_T_ARRAY;
			task->S->top++;
			break;
		case EQOP:
		case NEQOP:
			{
				int32 res = luaO_equalObj(task->S->top - 2, task->S->top - 1);
				task->S->top--;
				if (task->aux == NEQOP)
					res = !res;
				ttype(task->S->top - 1) = res ? LUA_T_NUMBER : LUA_T_NIL;
				nvalue(task->S->top - 1) = 1;
				break;
			}
		case LTOP:
			comparison(LUA_T_NUMBER, LUA_T_NIL, LUA_T_NIL, IM_LT);
			break;
		case LEOP:
			comparison(LUA_T_NUMBER, LUA_T_NUMBER, LUA_T_NIL, IM_LE);
			break;
		case GTOP:
			comparison(LUA_T_NIL, LUA_T_NIL, LUA_T_NUMBER, IM_GT);
			break;
		case GEOP:
			comparison(LUA_T_NIL, LUA_T_NUMBER, LUA_T_NUMBER, IM_GE);
			break;
		case ADDOP:
			{
				TObject *l = task->S->top - 2;
				TObject *r = task->S->top - 1;
				if (tonumber(r) || tonumber(l))
					call_arith(IM_ADD);
				else {
					nvalue(l) += nvalue(r);
					--task->S->top;
				}
			break;
			}
		case SUBOP:
			{
				TObject *l = task->S->top - 2;
				TObject *r = task->S->top - 1;
				if (tonumber(r) || tonumber(l))
					call_arith(IM_SUB);
				else {
					nvalue(l) -= nvalue(r);
					--task->S->top;
				}
				break;
			}
		case MULTOP:
			{
				TObject *l = task->S->top - 2;
				TObject *r = task->S->top - 1;
				if (tonumber(r) || tonumber(l))
					call_arith(IM_MUL);
				else {
					nvalue(l) *= nvalue(r);
					--task->S->top;
				}
				break;
			}
		case DIVOP:
			{
				TObject *l = task->S->top - 2;
				TObject *r = task->S->top - 1;
				if (tonumber(r) || tonumber(l))
					call_arith(IM_DIV);
				else {
					nvalue(l) /= nvalue(r);
					--task->S->top;
				}
				break;
			}
		case POWOP:
			call_arith(IM_POW);
			break;
		case CONCOP:
			{
				TObject *l = task->S->top - 2;
				TObject *r = task->S->top - 1;
				if (tostring(l) || tostring(r))
					call_binTM(IM_CONCAT, "unexpected type for concatenation");
				else {
					tsvalue(l) = strconc(svalue(l), svalue(r));
					--task->S->top;
				}
				luaC_checkGC();
				break;
			}
		case MINUSOP:
			if (tonumber(task->S->top - 1)) {
				ttype(task->S->top) = LUA_T_NIL;
				task->S->top++;
				call_arith(IM_UNM);
			} else
				nvalue(task->S->top - 1) = -nvalue(task->S->top - 1);
			break;
		case NOTOP:
			ttype(task->S->top - 1) = (ttype(task->S->top - 1) == LUA_T_NIL) ? LUA_T_NUMBER : LUA_T_NIL;
			nvalue(task->S->top - 1) = 1;
			break;
		case ONTJMPW:
			task->aux = next_word(task->pc);
			goto ontjmp;
		case ONTJMP:
			task->aux = *task->pc++;
ontjmp:
			if (ttype(task->S->top - 1) != LUA_T_NIL)
				task->pc += task->aux;
			else
				task->S->top--;
			break;
		case ONFJMPW:
			task->aux = next_word(task->pc);
			goto onfjmp;
		case ONFJMP:
			task->aux = *task->pc++;
onfjmp:
			if (ttype(task->S->top - 1) == LUA_T_NIL)
				task->pc += task->aux;
			else
				task->S->top--;
			break;
		case JMPW:
			task->aux = next_word(task->pc);
			goto jmp;
		case JMP:
			task->aux = *task->pc++;
jmp:
			task->pc += task->aux;
			break;
		case IFFJMPW:
			task->aux = next_word(task->pc);
			goto iffjmp;
		case IFFJMP:
			task->aux = *task->pc++;
iffjmp:
			if (ttype(--task->S->top) == LUA_T_NIL)
				task->pc += task->aux;
			break;
		case IFTUPJMPW:
			task->aux = next_word(task->pc);
			goto iftupjmp;
		case IFTUPJMP:
			task->aux = *task->pc++;
iftupjmp:
			if (ttype(--task->S->top) != LUA_T_NIL)
				task->pc -= task->aux;
			break;
		case IFFUPJMPW:
			task->aux = next_word(task->pc);
			goto iffupjmp;
		case IFFUPJMP:
			task->aux = *task->pc++;
iffupjmp:
			if (ttype(--task->S->top) == LUA_T_NIL)
				task->pc -= task->aux;
			break;
		case CLOSURE:
			task->aux = *task->pc++;
			goto closure;
		case CLOSURE0:
		case CLOSURE1:
			task->aux -= CLOSURE0;
closure:
			luaV_closure(task->aux);
			luaC_checkGC();
			break;
	  case CALLFUNC:
			task->aux = *task->pc++;
			goto callfunc;
	  case CALLFUNC0:
	  case CALLFUNC1:
			task->aux -= CALLFUNC0;
callfunc:
			lua_state->state_counter2--;
			return -((task->S->top - task->S->stack) - (*task->pc++));
		case ENDCODE:
			task->S->top = task->S->stack + task->base;
			// fall through
		case RETCODE:
			lua_state->state_counter2--;
			return (task->base + ((task->aux == 123) ? *task->pc : 0));
		case SETLINEW:
			task->aux = next_word(task->pc);
			goto setline;
		case SETLINE:
			task->aux = *task->pc++;
setline:
			if ((task->S->stack + task->base - 1)->ttype != LUA_T_LINE) {
				// open space for LINE value */
				luaD_openstack((task->S->top - task->S->stack) - task->base);
				task->base++;
				(task->S->stack + task->base - 1)->ttype = LUA_T_LINE;
			}
			(task->S->stack + task->base - 1)->value.i = task->aux;
			if (lua_linehook)
				luaD_lineHook(task->aux);
			break;
#ifdef LUA_DEBUG
		default:
			LUA_INTERNALERROR("internal error - opcode doesn't match");
#endif
		}
	}
}

} // end of namespace Grim
