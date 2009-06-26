/*
** $Id$
** Standard I/O (and system) library
** See Copyright Notice in lua.h
*/


#include "common/savefile.h"
#include "common/fs.h"
#include "common/system.h"

#include "engines/grim/lua/lauxlib.h"
#include "engines/grim/lua/lua.h"
#include "engines/grim/lua/luadebug.h"
#include "engines/grim/lua/lualib.h"

#include "base/commandLine.h"

#include "engines/grim/resource.h"
#include "engines/grim/grim.h"
#include "engines/grim/savegame.h"

#if defined(UNIX) || defined(__SYMBIAN32__)
#include <sys/stat.h>
#endif

#include <time.h>

#ifdef _WIN32
#include <direct.h>
#endif

namespace Grim {

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

LuaFile::LuaFile() : _in(NULL), _out(NULL), _stdin(false), _stdout(false), _stderr(false) {
}

LuaFile::~LuaFile() {
	close();
}

void LuaFile::close() {
	delete _in;
	delete _out;
	_in = NULL;
	_out = NULL;
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
	} else
		assert(0);
	return 0;
}

uint32 LuaFile::write(const char *buf, uint32 len) {
	if (_stdin)
		error("LuaFile::write() not allowed on stdin");
	if (_in)
		error("LuaFile::write() not allowed on in");
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
		Common::SaveFileManager *saveFileMan = g_system->getSavefileManager();
		inFile = saveFileMan->openForLoading(s);
		if (!inFile)
			current = g_resourceloader->openNewStreamLuaFile(s);
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
		if (Common::String(s).hasSuffix("\\bino.txt")) {
			pushresult(0);
			return;
		}
		LuaFile *current;
		Common::WriteStream *outFile = NULL;
		Common::SaveFileManager *saveFileMan = g_system->getSavefileManager();
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
	Common::SaveFileManager *saveFileMan = g_system->getSavefileManager();
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

static void io_read() {
	int32 arg = FIRSTARG;
	LuaFile *f = (LuaFile *)getfileparam(FINPUT, &arg);
	char *buff;
	const char *p = luaL_opt_string(arg, "[^\n]*{\n}");
	int inskip = 0;  // to control {skips}
	int c = NEED_OTHER;
	luaL_resetbuffer();
	while (*p) {
		if (*p == '{') {
			inskip++;
			p++;
		} else if (*p == '}') {
			if (inskip == 0)
				lua_error("unbalanced braces in read pattern");
			inskip--;
			p++;
		} else {
			const char *ep;  // get what is next
			int m;  // match result
			if (c == NEED_OTHER) {
				char z;
				if (f->read(&z, 1) != 1)
					c = EOF;
				else
					c = z;
			}
			m = luaI_singlematch((c == EOF) ? 0 : (char)c, p, &ep);
			if (m) {
				if (inskip == 0)
					luaL_addchar(c);
				c = NEED_OTHER;
			}
			switch (*ep) {
			case '*':  // repetition
				if (!m)
					p = ep + 1;  // else stay in (repeat) the same item
				break;
			case '?':  // optional
				p = ep + 1;  // continues reading the pattern
				break;
			default:
				if (m)
					p = ep;  // continues reading the pattern
				else
					goto break_while;   // pattern fails
			}
		}
	}
break_while:
	if (c >= 0) // not EOF nor NEED_OTHER?
		f->seek(-1, SEEK_CUR);
	luaL_addchar(0);
	buff = luaL_buffer();
	if (*buff != 0 || *p == 0)  // read something or did not fail?
		lua_pushstring(buff);
}

static void io_write() {
	int32 arg = FIRSTARG;
	LuaFile *f = (LuaFile *)getfileparam(FOUTPUT, &arg);
	int32 status = 1;
	const char *s;
	while ((s = luaL_opt_string(arg++, NULL)) != NULL)
		status = status && ((int32)f->write(s, strlen(s)) != EOF);
	pushresult(status);
}

static void io_date() {
	tm t;
	char b[BUFSIZ];

	g_system->getTimeAndDate(t);
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
		lua_pushCclosure(iolibtag[i].func, 2);
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

} // end of namespace Grim
