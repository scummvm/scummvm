/*
** $Id$
** Standard I/O (and system) library
** See Copyright Notice in lua.h
*/


#include "common/savefile.h"
#include "common/fs.h"

#include "engine/lua/lauxlib.h"
#include "engine/lua/lua.h"
#include "engine/lua/luadebug.h"
#include "engine/lua/lualib.h"

#include "engine/resource.h"
#include "engine/cmd_line.h"
#include "engine/engine.h"
#include "engine/savegame.h"
#include "engine/backend/platform/driver.h"

#if defined(UNIX) || defined(__SYMBIAN32__)
#include <sys/stat.h>
#endif

#ifdef _WIN32
#include <direct.h>
#endif

#define CLOSEDTAG	2
#define IOTAG		1

#define FIRSTARG      3  // 1st and 2nd are upvalues

#define FINPUT		"_INPUT"
#define FOUTPUT		"_OUTPUT"

LuaFile *g_fin;
LuaFile *g_fout;
LuaFile *g_stdin;
LuaFile *g_stdout;
LuaFile *g_stderr;

LuaFile::LuaFile() : _in(NULL), _out(NULL), _file(NULL), _stdin(false), _stdout(false), _stderr(false) {
}

LuaFile::~LuaFile() {
	close();
}

void LuaFile::close() {
	delete _in;
	delete _out;
	delete _file;
	_in = NULL;
	_out = NULL;
	_file = NULL;
	_stdin = _stdout = _stderr = false;
}

bool LuaFile::isOpen() const {
	return _in || _out || _stdin || stdout || stderr;
}

uint32 LuaFile::read(void *buf, uint32 len) {
	if (_stdin) {
		return fread(buf, len, 1, stdin);
	} else if (_in) {
		return _in->read(buf, len);
	} else if (_file) {
		return _file->read(buf, len);
	} else
		assert(0);
	return 0;
}

uint32 LuaFile::write(const char *buf, uint32 len) {
	if (_stdout) {
		return fwrite(buf, len, 1, stdout);
	} else if (_stderr) {
		return fwrite(buf, len, 1, stderr);
	} else if (_out) {
		return _out->write(buf, len);
	} else
		assert(0);
	return 0;
}

void LuaFile::seek(int32 pos, int whence) {
	if (_stdin) {
		fseek(stdin, pos, whence);
	} else if (_in) {
		_in->seek(pos, whence);
	} else if (_file) {
		_file->seek(pos, whence);
	} else
		assert(0);
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

static LuaFile *getfile(const char *name) {
	lua_Object f = lua_getglobal(name);
	if (!ishandler(f))
		luaL_verror("global variable `%.50s' is not a file handle", name);
	return (LuaFile *)lua_getuserdata(f);
}

static LuaFile *getfileparam(const char *name, int32 *arg) {
	lua_Object f = lua_getparam(*arg);
	if (ishandler(f)) {
		(*arg)++;
		return (LuaFile *)lua_getuserdata(f);
	} else
		return getfile(name);
}

static void closefile(const char *name) {
	LuaFile *f = getfile(name);
	f->close();
	lua_pushobject(lua_getglobal(name));
	lua_settag(gettag(CLOSEDTAG));
}

static void setfile(LuaFile *f, const char *name, int32 tag) {
	lua_pushusertag(f, tag);
	lua_setglobal(name);
}

static void setreturn(LuaFile *f, const char *name) {
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
		LuaFile *current = (LuaFile *)lua_getuserdata(f);
		if (!current) {
			pushresult(0);
			return;
		}
		setreturn(current, FINPUT);
	} else {
		const char *s = luaL_check_string(FIRSTARG);
		LuaFile *current;
		Common::SeekableReadStream *inFile = NULL;
		Common::SaveFileManager *saveFileMan = g_driver->getSavefileManager();
		inFile = saveFileMan->openForLoading(s);
		if (!inFile)
			current = g_resourceloader->openNewStreamLua(s);
		else {
			current = new LuaFile();
			current->_in = inFile;
		}
		if (!current) {
			delete current;
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
		LuaFile *current = (LuaFile *)lua_getuserdata(f);
		if (!current->isOpen()) {
			pushresult(0);
			return;
		}
		setreturn(current, FOUTPUT);
	} else {
		const char *s = luaL_check_string(FIRSTARG);
		LuaFile *current;
		Common::WriteStream *outFile = NULL;
		Common::SaveFileManager *saveFileMan = g_driver->getSavefileManager();
		outFile = saveFileMan->openForSaving(s);
		if (!outFile) {
			pushresult(0);
			return;
		}
		current = new LuaFile();
		current->_out = outFile;
		setreturn(current, FOUTPUT);
	}
}

static void io_appendto() {
	const char *s = luaL_check_string(FIRSTARG);
	Common::SeekableReadStream *inFile = NULL;
	Common::SaveFileManager *saveFileMan = g_driver->getSavefileManager();
	inFile = saveFileMan->openForLoading(s);
	if (!inFile) {
		pushresult(0);
		return;
	}
	int size = inFile->size();
	byte *buf = new byte[size];
	inFile->read(buf, size);
	delete inFile;

	Common::WriteStream *outFile = NULL;
	outFile = saveFileMan->openForSaving(s);
	if (!outFile)
		pushresult(0);
	else {
		outFile->write(buf, size);
		LuaFile *current = new LuaFile();
		current->_out = outFile;
		setreturn(current, FOUTPUT);
	}
	delete[] buf;
}

#define NEED_OTHER (EOF - 1)  // just some flag different from EOF

static void read_until(LuaFile *f, int32 lim) {
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
	LuaFile *f = (LuaFile *)getfileparam(FINPUT, &arg);
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
		if (c >= 0) // not EOF nor NEED_OTHER?
			f->seek(-1, SEEK_CUR);
		if (l > 0 || *p == 0)  // read something or did not fail?
			lua_pushlstring(luaL_buffer(), l);
	}
}

static void io_write() {
	int32 arg = FIRSTARG;
	LuaFile *f = (LuaFile *)getfileparam(FOUTPUT, &arg);
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
		sprintf(buf, (level == 2) ? "Active Stack:\n\t" : "\t");
		g_stderr->write(buf, strlen(buf));
		switch (*lua_getobjname(func, &name)) {
		case 'g':
			sprintf(buf, "function %s", name);
			break;
		case 't':
			sprintf(buf, "`%s' tag method", name);
			break;
		default: 
			{
				if (linedefined == 0)
					sprintf(buf, "main of %s", filename);
				else if (linedefined < 0)
					sprintf(buf, "%s", filename);
				else
					sprintf(buf, "function (%s:%d)", filename, (int)linedefined);
				filename = NULL;
			}
		}
		g_stderr->write(buf, strlen(buf));

		if ((currentline = lua_currentline(func)) > 0) {
			sprintf(buf, " at line %d", (int)currentline);
			g_stderr->write(buf, strlen(buf));
		}
		if (filename) {
			sprintf(buf, " [in file %s]", filename);
			g_stderr->write(buf, strlen(buf));
		}
		sprintf(buf, "\n");
		g_stderr->write(buf, strlen(buf));
	}
}

static void errorfb() {
	char buf[256];
	sprintf(buf, "lua: %s\n", lua_getstring(lua_getparam(1)));
	g_stderr->write(buf, strlen(buf));
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

	g_fin = new LuaFile();
	g_fin->_stdin = true;
	setfile(g_fin, FINPUT, iotag);

	g_fout = new LuaFile();
	g_fout->_stdout = true;
	setfile(g_fout, FOUTPUT, iotag);

	g_stdin = new LuaFile();
	g_stdin->_stdin = true;
	setfile(g_stdin, "_STDIN", iotag);

	g_stdout = new LuaFile();
	g_stdout->_stdout = true;
	setfile(g_stdout, "_STDOUT", iotag);

	g_stderr = new LuaFile();
	g_stderr->_stderr = true;
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
