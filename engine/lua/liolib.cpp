/*
** $Id$
** Standard I/O (and system) library
** See Copyright Notice in lua.h
*/


#include "common/savefile.h"

#include "engine/lua/lauxlib.h"
#include "engine/lua/lua.h"
#include "engine/lua/luadebug.h"
#include "engine/lua/lualib.h"

#include "engine/resource.h"
#include "engine/cmd_line.h"
#include "engine/savegame.h"
#include "engine/backend/platform/driver.h"

#define CLOSEDTAG	2
#define IOTAG		1

#define FIRSTARG      3  // 1st and 2nd are upvalues

#define FINPUT		"_INPUT"
#define FOUTPUT		"_OUTPUT"

Common::File *g_fin;
Common::File *g_fout;
Common::File *g_stdin;
Common::File *g_stdout;
Common::File *g_stderr;


extern Common::SaveFileManager *g_saveFileMan;

static void join_paths(const char *filename, const char *directory,
								 char *buf, int bufsize) {
	buf[bufsize - 1] = '\0';
	strncpy(buf, directory, bufsize - 1);

#ifdef WIN32
	// Fix for Win98 issue related with game directory pointing to root drive ex. "c:\"
	if ((buf[0] != 0) && (buf[1] == ':') && (buf[2] == '\\') && (buf[3] == 0)) {
		buf[2] = 0;
	}
#endif

	const int dirLen = strlen(buf);

	if (dirLen > 0) {
#if defined(__MORPHOS__) || defined(__amigaos4__)
		if (buf[dirLen - 1] != ':' && buf[dirLen - 1] != '/')
#endif

#if !defined(__GP32__)
		strncat(buf, "/", bufsize - 1);	// prevent double /
#endif
	}
	strncat(buf, filename, bufsize - 1);
}


static int32 gettag(int32 i) {
	return (int32)lua_getnumber(lua_getparam(i));
}

static void pushresult(int32 i) {
	if (i)
		lua_pushuserdata(NULL);
	else {
		lua_pushnil();
		lua_pushstring("File I/O error.");
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

static Common::File *getfile(const char *name) {
	lua_Object f = lua_getglobal(name);
	if (!ishandler(f))
		luaL_verror("global variable `%.50s' is not a file handle", name);
	return (Common::File *)lua_getuserdata(f);
}

static Common::File *getfileparam(const char *name, int32 *arg) {
	lua_Object f = lua_getparam(*arg);
	if (ishandler(f)) {
		(*arg)++;
		return (Common::File *)lua_getuserdata(f);
	} else
		return getfile(name);
}

static void closefile(const char *name) {
	Common::File *f = getfile(name);
	f->close();
	lua_pushobject(lua_getglobal(name));
	lua_settag(gettag(CLOSEDTAG));
}

static void setfile(Common::File *f, const char *name, int32 tag) {
	lua_pushusertag(f, tag);
	lua_setglobal(name);
}

static void setreturn(Common::File *f, const char *name) {
	int32 tag = gettag(IOTAG);
	setfile(f, name, tag);
	lua_pushusertag(f, tag);
}

static void io_readfrom() {
	lua_Object f = lua_getparam(FIRSTARG);
	if (f == LUA_NOOBJECT) {
		closefile(FINPUT);
		setreturn(g_fin, FINPUT);
	} else if (lua_tag(f) == gettag(IOTAG)) {
		Common::File *current = (Common::File *)lua_getuserdata(f);
		if (!current) {
			pushresult(0);
			return;
		}
		setreturn(current, FINPUT);
	} else {
		const char *s = luaL_check_string(FIRSTARG);
		Common::File *current = new Common::File();
		Common::String dir = ConfMan.get("savepath");
#ifdef _WIN32_WCE
		if (dir.empty())
			dir = ConfMan.get("path");
#endif
		char buf[256];
		join_paths(s, dir.c_str(), buf, sizeof(buf));
		if (current->exists(buf))
			current->open(buf);
		if (!current->isOpen()) {
			delete current;
			current = g_resourceloader->openNewStream(s);
		}
		if (!current) {
			pushresult(0);
			return;
		}
		setreturn(current, FINPUT);
	}
}

static void io_writeto() {
	lua_Object f = lua_getparam(FIRSTARG);
	if (f == LUA_NOOBJECT) {
		closefile(FOUTPUT);
		setreturn(g_fout, FOUTPUT);
	} else if (lua_tag(f) == gettag(IOTAG)) {
		Common::File *current = (Common::File *)lua_getuserdata(f);
		if (!current->isOpen()) {
			pushresult(0);
			return;
		}
		setreturn(current, FOUTPUT);
	} else {
		const char *s = luaL_check_string(FIRSTARG);
		Common::File *current = new Common::File();
		Common::String dir = ConfMan.get("savepath");
#ifdef _WIN32_WCE
		if (dir.empty())
			dir = ConfMan.get("path");
#endif
		char buf[256];
		join_paths(s, dir.c_str(), buf, sizeof(buf));
		current->open(buf, Common::File::kFileWriteMode);
		if (!current->isOpen()) {
			delete current;
			pushresult(0);
			return;
		}
		setreturn(current, FOUTPUT);
	}
}

static void io_appendto() {
	const char *s = luaL_check_string(FIRSTARG);
	Common::File file;
	Common::String dir = ConfMan.get("savepath");
#ifdef _WIN32_WCE
	if (dir.empty())
		dir = ConfMan.get("path");
#endif
	char path[256];
	join_paths(s, dir.c_str(), path, sizeof(path));
	file.open(path);
	if (!file.isOpen()) {
		pushresult(0);
		return;
	}
	int size = file.size();
	byte *buf = new byte[size];
	file.read(buf, size);
	file.close();

	Common::File *fp = new Common::File();
	fp->open(path, Common::File::kFileWriteMode);
	if (fp->isOpen()) {
		fp->write(buf, size);
		setreturn(fp, FOUTPUT);
	} else {
		delete fp;
		pushresult(0);
	}
	delete buf;
}

#define NEED_OTHER (EOF - 1)  // just some flag different from EOF

static void read_until(Common::File *f, int32 lim) {
	int32 l = 0;
	int8 c;

	if (f->read(&c, 1) == 0)
		c = EOF;
	for (; c != EOF && c != lim; ) {
		luaL_addchar(c);
		l++;
		if (f->read(&c, 1) == 0)
			c = EOF;
	}
	if (l > 0 || c == lim)  // read anything?
		lua_pushlstring(luaL_buffer(), l);
}

static void io_read() {
	int32 arg = FIRSTARG;
	Common::File *f = (Common::File *)getfileparam(FINPUT, &arg);
	const char *p = luaL_opt_string(arg, NULL);
	luaL_resetbuffer();
	if (!p)  // default: read a line
		read_until(f, '\n');
	else if (p[0] == '.' && p[1] == '*' && p[2] == 0)  // p = ".*"
		read_until(f, EOF);
	else {
		int32 l = 0;  // number of chars read in buffer
		int32 inskip = 0;  // to control {skips}
		int8 c = NEED_OTHER;
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
					if (c == NEED_OTHER) {
						if (f->read(&c, 1) == 0)
							c = EOF;
					}
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
			f->seek(-1, SEEK_CUR);
		if (l > 0 || *p == 0)  // read something or did not fail?
			lua_pushlstring(luaL_buffer(), l);
	}
}

static void io_write() {
	int32 arg = FIRSTARG;
	Common::File *f = (Common::File *)getfileparam(FOUTPUT, &arg);
	int32 status = 1;
	const char *s;
	int32 l;
	while ((s = luaL_opt_lstr(arg++, NULL, &l)))
		status = status && (f->write(s, l) == (size_t)l);
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

static void lua_printstack() {
	int32 level = 1;  // skip level 0 (it's this function)
	lua_Object func;
	char buf[256];
	while ((func = lua_stackedfunction(level++)) != LUA_NOOBJECT) {
		const char *name;
		int32 currentline;
		const char *filename;
		int32 linedefined;
		lua_funcinfo(func, &filename, &linedefined);
		printf(buf, (level == 2) ? "Active Stack:\n\t" : "\t");
		g_stderr->write(buf, strlen(buf));
		switch (*lua_getobjname(func, &name)) {
		case 'g':
			printf(buf, "function %s", name);
			break;
		case 't':
			printf(buf, "`%s' tag method", name);
			break;
		default: 
			{
				if (linedefined == 0)
					printf(buf, "main of %s", filename);
				else if (linedefined < 0)
					printf(buf, "%s", filename);
				else
					printf(buf, "function (%s:%d)", filename, (int)linedefined);
				filename = NULL;
			}
		}
		g_stderr->write(buf, strlen(buf));

		if ((currentline = lua_currentline(func)) > 0) {
			printf(buf, " at line %d", (int)currentline);
			g_stderr->write(buf, strlen(buf));
		}
		if (filename) {
			printf(buf, " [in file %s]", filename);
			g_stderr->write(buf, strlen(buf));
		}
		printf(buf, "\n");
		g_stderr->write(buf, strlen(buf));
	}
}

static void errorfb() {
	fprintf(stderr, "lua: %s\n", lua_getstring(lua_getparam(1)));
	lua_printstack();
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

	g_fin = new Common::File();
	g_fin->open("(stdin)");
	if (!g_fin->isOpen())
		delete g_fin;
	else
		setfile(g_fin, FINPUT, iotag);
	g_fout = new Common::File();
	g_fout->open("(stdout)", Common::File::kFileWriteMode);
	if (!g_fout->isOpen())
		delete g_fout;
	else
		setfile(g_fout, FOUTPUT, iotag);
	g_stdin = new Common::File();
	g_stdin->open("(stdin)");
	if (!g_stdin->isOpen())
		delete g_stdin;
	else
		setfile(g_stdin, "_STDIN", iotag);
	g_stdout = new Common::File();
	g_stdout->open("(stdout)", Common::File::kFileWriteMode);
	if (!g_stdout->isOpen())
		delete g_stdout;
	else
		setfile(g_stdout, "_STDOUT", iotag);
	g_stderr = new Common::File();
	g_stderr->open("(stderr)", Common::File::kFileWriteMode);
	if (!g_stderr->isOpen())
		delete g_stderr;
	else
		setfile(g_stderr, "_STDERR", iotag);
}

void lua_iolibopen() {
	luaL_openlib(iolib, (sizeof(iolib) / sizeof(iolib[0])));
	luaL_addlibtolist(iolibtag, (sizeof(iolibtag) / sizeof(iolibtag[0])));
	openwithtags();
	lua_pushcfunction(errorfb);
	lua_seterrormethod();
}

void lua_iolibclose() {
	delete g_fin;
	delete g_fout;
	delete g_stdin;
	delete g_stdout;
	delete g_stderr;
}
