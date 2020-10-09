/*
** Lua standard mathematical library
** See Copyright Notice in lua.h
*/

#define FORBIDDEN_SYMBOL_EXCEPTION_rand
#define FORBIDDEN_SYMBOL_EXCEPTION_srand

#include "common/scummsys.h"

#include "engines/grim/lua/lauxlib.h"
#include "engines/grim/lua/lua.h"
#include "engines/grim/lua/lualib.h"

namespace Grim {

#define FROMRAD(a) ((a) * (180.0 / (float)M_PI))
#define TORAD(a)    ((a) * ((float)M_PI / 180.0))

static void math_abs() {
	float d = luaL_check_number(1);
	if (d < 0)
		d = -d;
	lua_pushnumber(d);
}

static void math_sin() {
	lua_pushnumber(sin(TORAD(luaL_check_number(1))));
}

static void math_cos() {
	lua_pushnumber(cos(TORAD(luaL_check_number(1))));
}

static void math_tan() {
	lua_pushnumber(tan(TORAD(luaL_check_number(1))));
}

static void math_asin() {
	lua_pushnumber(FROMRAD(asin(luaL_check_number(1))));
}

static void math_acos() {
	lua_pushnumber(FROMRAD(acos(luaL_check_number(1))));
}

static void math_atan() {
	lua_pushnumber(FROMRAD(atan(luaL_check_number(1))));
}

static void math_ceil() {
	lua_pushnumber(ceil(luaL_check_number(1)));
}

static void math_floor() {
	lua_pushnumber(floor(luaL_check_number(1)));
}

static void math_mod() {
	lua_pushnumber(fmod(luaL_check_number(1), luaL_check_number(2)));
}

static void math_sqrt() {
	lua_pushnumber(sqrt(luaL_check_number(1)));
}

static void math_pow() {
	lua_pushnumber(pow(luaL_check_number(1), luaL_check_number(2)));
}

static void math_deg() {
	lua_pushnumber(luaL_check_number(1) * (180.0 / (float)M_PI));
}

static void math_rad() {
	lua_pushnumber(luaL_check_number(1) * ((float)M_PI / 180.0));
}

static void math_min() {
	int32 i = 1;
	float dmin = luaL_check_number(i);
	while (lua_getparam(++i) != LUA_NOOBJECT) {
		float d = luaL_check_number(i);
		if (d < dmin)
			dmin = d;
	}
	lua_pushnumber(dmin);
}

static void math_max() {
	int32 i = 1;
	float dmax = luaL_check_number(i);
	while (lua_getparam(++i) != LUA_NOOBJECT) {
		float d = luaL_check_number(i);
		if (d > dmax)
			dmax = d;
	}
	lua_pushnumber(dmax);
}

static void math_random() {
	float r = (float)(rand() % RAND_MAX) / (float)RAND_MAX;
	float l = luaL_opt_number(1, 0);
	if (l == 0)
		lua_pushnumber(r);
	else
		lua_pushnumber((int32)(r * l) + 1);
}

static void math_randomseed() {
	srand((unsigned int)luaL_check_number(1));
}

static struct luaL_reg mathlib[] = {
	{ "abs",	math_abs},
	{ "sin",	math_sin},
	{ "cos",	math_cos},
	{ "tan",	math_tan},
	{ "asin",	math_asin},
	{ "acos",	math_acos},
	{ "atan",	math_atan},
	{ "ceil",	math_ceil},
	{ "floor",	math_floor},
	{ "mod",	math_mod},
	{ "sqrt",	math_sqrt},
	{ "min",	math_min},
	{ "max",	math_max},
	{ "deg",	math_deg},
	{ "rad",	math_rad},
	{ "random",		math_random},
	{ "randomseed",	math_randomseed}
};

static luaL_reg powFunc[] = {
	{ "math_pow",	math_pow }
};

/*
** Open math library
*/
void lua_mathlibopen() {
	luaL_openlib(mathlib, (sizeof(mathlib) / sizeof(mathlib[0])));
	luaL_addlibtolist(powFunc, (sizeof(powFunc) / sizeof(powFunc[0])));
	lua_pushstring("deg");
	lua_setglobal("_TRIGMODE");
	lua_pushcfunction(math_pow);
	lua_pushnumber(0);  // to get its tag
	lua_settagmethod(lua_tag(lua_pop()), "pow");
	lua_pushnumber((float)M_PI);
	lua_setglobal("PI");
}

} // end of namespace Grim
