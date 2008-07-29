/*
** $Id$
** Standard I/O (and system) library
** See Copyright Notice in lua.h
*/


#include "engine/lua/lauxlib.h"
#include "engine/lua/lua.h"
#include "engine/lua/luadebug.h"
#include "engine/lua/lualib.h"

#include "engine/resource.h"
#include "engine/backend/driver.h"

#include <errno.h>

#define CLOSEDTAG	2
#define IOTAG		1

#define FIRSTARG      3  // 1st and 2nd are upvalues

#define FINPUT		"_INPUT"
#define FOUTPUT		"_OUTPUT"

static int32 gettag(int32 i) {
	return (int32)lua_getnumber(lua_getparam(i));
}

static void pushresult(int32 i) {
	if (i)
		lua_pushuserdata(NULL);
	else {
		lua_pushnil();
		lua_pushstring(strerror(errno));
	}
}


static int32 ishandler(lua_Object f) {
	if (lua_isuserdata(f)) {
		if (lua_tag(f) == gettag(CLOSEDTAG))
			lua_error("cannot access a closed file");
		return lua_tag(f) == gettag(IOTAG);
	}
	else return 0;
}

static FILE *getfile(const char *name) {
	lua_Object f = lua_getglobal(name);
	if (!ishandler(f))
		luaL_verror("global variable `%.50s' is not a file handle", name);
	return (FILE *)lua_getuserdata(f);
}

static FILE *getfileparam(const char *name, int32 *arg) {
	lua_Object f = lua_getparam(*arg);
	if (ishandler(f)) {
		(*arg)++;
		return (FILE *)lua_getuserdata(f);
	} else
		return getfile(name);
}

static void closefile (const char *name) {
	FILE *f = getfile(name);
	if (f == stdin || f == stdout)
		return;
	fclose(f);
	lua_pushobject(lua_getglobal(name));
	lua_settag(gettag(CLOSEDTAG));
}


static void setfile(FILE *f, const char *name, int32 tag) {
	lua_pushusertag(f, tag);
	lua_setglobal(name);
}

static void setreturn(FILE *f, const char *name) {
	int32 tag = gettag(IOTAG);
	setfile(f, name, tag);
	lua_pushusertag(f, tag);
}

static void io_readfrom() {
	FILE *current;
	lua_Object f = lua_getparam(FIRSTARG);
	if (f == LUA_NOOBJECT) {
		closefile(FINPUT);
		current = stdin;
	} else if (lua_tag(f) == gettag(IOTAG)) {
		current = (FILE *)lua_getuserdata(f);
	} else {
		const char *s = luaL_check_string(FIRSTARG);
		current = fopen(s, "r");
		//      if (current == NULL)
		//	    current = g_resourceloader->openNewStream(s);
	}
	if (!current) {
		pushresult(0);
		return;
	}
	setreturn(current, FINPUT);
}

static void io_writeto() {
	FILE *current;
	lua_Object f = lua_getparam(FIRSTARG);
	if (f == LUA_NOOBJECT) {
		closefile(FOUTPUT);
		current = stdout;
	} else if (lua_tag(f) == gettag(IOTAG))
		current = (FILE *)lua_getuserdata(f);
	else {
		const char *s = luaL_check_string(FIRSTARG);
		current = fopen(s, "w");
		if (!current) {
			pushresult(0);
			return;
		}
	}
	setreturn(current, FOUTPUT);
}

static void io_appendto() {
	const char *s = luaL_check_string(FIRSTARG);
	FILE *fp = fopen (s, "a");
	if (fp)
		setreturn(fp, FOUTPUT);
	else
		pushresult(0);
}

#define NEED_OTHER (EOF - 1)  // just some flag different from EOF

static void read_until(FILE *f, int32 lim) {
	int32 l = 0;
	int32 c;
	for (c = getc(f); c != EOF && c != lim; c = getc(f)) {
		luaL_addchar(c);
		l++;
	}
	if (l > 0 || c == lim)  // read anything?
		lua_pushlstring(luaL_buffer(), l);
}

static void io_read (void) {
	int32 arg = FIRSTARG;
	FILE *f = getfileparam(FINPUT, &arg);
	const char *p = luaL_opt_string(arg, NULL);
	luaL_resetbuffer();
	if (p == NULL)  // default: read a line
		read_until(f, '\n');
	else if (p[0] == '.' && p[1] == '*' && p[2] == 0)  // p = ".*"
		read_until(f, EOF);
	else {
		int32 l = 0;  // number of chars read in buffer
		int32 inskip = 0;  // to control {skips}
		int32 c = NEED_OTHER;
		while (*p) {
			switch (*p) {
			case '{':
				inskip++;
				p++;
				continue;
			case '}':
				if (inskip == 0)
					lua_error("unbalanced braces in read pattern");
				inskip--;
				p++;
				continue;
			default:
				{
					const char *ep;  // get what is next
					int32 m;  // match result
					if (c == NEED_OTHER)
						c = getc(f);
					if (c == EOF) {
						luaI_singlematch(0, p, &ep);  // to set "ep"
						m = 0;
					} else {
						m = luaI_singlematch(c, p, &ep);
						if (m) {
							if (inskip == 0) {
								luaL_addchar(c);
								l++;
							}
							c = NEED_OTHER;
						}
					}
					switch (*ep) {
					case '*':  // repetition
						if (!m)
						p = ep + 1;  // else stay in (repeat) the same item
						continue;
					case '?':  // optional
						p = ep + 1;  // continues reading the pattern
						continue;
					default:
						if (m)
							p = ep;  // continues reading the pattern
						else
							goto break_while;   // pattern fails
					}
				}
			}
		}
break_while:
		if (c >= 0)  // not EOF nor NEED_OTHER?
			ungetc(c, f);
		if (l > 0 || *p == 0)  // read something or did not fail?
			lua_pushlstring(luaL_buffer(), l);
	}
}

static void io_write() {
	int32 arg = FIRSTARG;
	FILE *f = getfileparam(FOUTPUT, &arg);
	int32 status = 1;
	const char *s;
	int32 l;
	while ((s = luaL_opt_lstr(arg++, NULL, &l)))
		status = status && (fwrite(s, 1, l, f) == (size_t)l);
	pushresult(status);
}

static void io_date() {
	tm t;
	char b[BUFSIZ];

	g_driver->getTimeAndDate(t);
	sprintf(b, "%02d.%02d.%d %02d:%02d.%02d", t.tm_mday, t.tm_mon + 1, 1900 + t.tm_year, t.tm_hour, t.tm_min, t.tm_sec);
	lua_pushstring(b);
}

static void io_exit() {
	lua_Object o = lua_getparam(1);
	exit((int)lua_isnumber(o) ? (int)lua_getnumber(o) : 1);
}

static void lua_printstack(FILE *f) {
	int32 level = 1;  // skip level 0 (it's this function)
	lua_Object func;
	while ((func = lua_stackedfunction(level++)) != LUA_NOOBJECT) {
		const char *name;
		int32 currentline;
		const char *filename;
		int32 linedefined;
		lua_funcinfo(func, &filename, &linedefined);
		fprintf(f, (level == 2) ? "Active Stack:\n\t" : "\t");
		switch (*lua_getobjname(func, &name)) {
		case 'g':
			fprintf(f, "function %s", name);
			break;
		case 't':
			fprintf(f, "`%s' tag method", name);
			break;
		default: 
			{
				if (linedefined == 0)
					fprintf(f, "main of %s", filename);
				else if (linedefined < 0)
					fprintf(f, "%s", filename);
				else
					fprintf(f, "function (%s:%d)", filename, (int)linedefined);
				filename = NULL;
			}
		}

		if ((currentline = lua_currentline(func)) > 0)
			fprintf(f, " at line %d", (int)currentline);
		if (filename)
			fprintf(f, " [in file %s]", filename);
		fprintf(f, "\n");
	}
}

static void errorfb() {
	fprintf(stderr, "lua: %s\n", lua_getstring(lua_getparam(1)));
	lua_printstack(stderr);
}

static struct luaL_reg iolib[] = {
	{ "date",			io_date },
	{ "exit",			io_exit },
	{ "print_stack",	errorfb }
};

static struct luaL_reg iolibtag[] = {
	{ "readfrom",	io_readfrom },
	{ "writeto",	io_writeto },
	{ "appendto",	io_appendto },
	{ "read",		io_read },
	{ "write",		io_write }
};

static void openwithtags() {
	int32 iotag = lua_newtag();
	int32 closedtag = lua_newtag();
	uint32 i;
	for (i = 0; i < sizeof(iolibtag) / sizeof(iolibtag[0]); i++) {
		// put both tags as upvalues for these functions
		lua_pushnumber(iotag);
		lua_pushnumber(closedtag);
		lua_pushcclosure(iolibtag[i].func, 2);
		lua_setglobal(iolibtag[i].name);
	}
	setfile(stdin, FINPUT, iotag);
	setfile(stdout, FOUTPUT, iotag);
	setfile(stdin, "_STDIN", iotag);
	setfile(stdout, "_STDOUT", iotag);
	setfile(stderr, "_STDERR", iotag);
}

void lua_iolibopen() {
	luaL_openlib(iolib, (sizeof(iolib) / sizeof(iolib[0])));
	luaL_addlibtolist(iolibtag, (sizeof(iolib) / sizeof(iolib[0])));
	openwithtags();
	lua_pushcfunction(errorfb);
	lua_seterrormethod();
}
