/*
** Standard library for strings and pattern-matching
** See Copyright Notice in lua.h
*/

#define FORBIDDEN_SYMBOL_EXCEPTION_iscntrl
#define FORBIDDEN_SYMBOL_EXCEPTION_ispunct

#include "common/util.h"

#include "engines/grim/lua/lauxlib.h"
#include "engines/grim/lua/lua.h"
#include "engines/grim/lua/lualib.h"

namespace Grim {

static void addnchar(const char *s, int32 n) {
	char *b = luaL_openspace(n);
	strncpy(b, s, n);
	luaL_addsize(n);
}

static void addstr(const char *s) {
	addnchar(s, strlen(s));
}

static void str_len() {
	lua_pushnumber(strlen(luaL_check_string(1)));
}

static void closeandpush() {
	luaL_addchar(0);
	lua_pushstring(luaL_buffer());
}

static void str_sub() {
	const char *s = luaL_check_string(1);
	int l = strlen(s);
	int start = (int)luaL_check_number(2);
	int end = (int)luaL_opt_number(3, -1);
	if (start < 0)
		start = l + start + 1;
	if (end < 0)
		end = l + end + 1;
	if (1 <= start && start <= end && end <= l) {
		luaL_resetbuffer();
		addnchar(s + start - 1, end - start + 1);
		closeandpush();
	} else
		lua_pushstring("");
}

static void str_lower() {
	const char *s;
	luaL_resetbuffer();
	for (s = luaL_check_string(1); *s; s++)
		luaL_addchar(tolower((byte)*s));
	closeandpush();
}

static void str_upper() {
	const char *s;
	luaL_resetbuffer();
	for (s = luaL_check_string(1); *s; s++)
		luaL_addchar(toupper((byte)*s));
	closeandpush();
}

static void str_rep() {
	const char *s = luaL_check_string(1);
	int32 n = (int32)luaL_check_number(2);
	luaL_resetbuffer();
	while (n-- > 0)
		addstr(s);
	closeandpush();
}

#define MAX_CAPT 9

struct Capture {
	int32 level;  // total number of captures (finished or unfinished)
	struct {
		const char *init;
		int32 len;  // -1 signals unfinished capture
	} capture[MAX_CAPT];
};

#define ESC	'%'
#define SPECIALS  "^$*?.([%-"

static void push_captures(Capture *cap) {
	int i;

	for (i = 0; i < cap->level; i++) {
		int l = cap->capture[i].len;
		char *buff = luaL_openspace(l+1);
		if (l == -1)
			lua_error("unfinished capture");
		strncpy(buff, cap->capture[i].init, l);
		buff[l] = 0;
		lua_pushstring(buff);
	}
}

static int32 check_cap (int32 l, Capture *cap) {
	l -= '1';
	if (!(0 <= l && l < cap->level && cap->capture[l].len != -1))
		lua_error("invalid capture index");
	return l;
}

static int32 capture_to_close(Capture *cap) {
	int32 level = cap->level;
	for (level--; level >= 0; level--)
		if (cap->capture[level].len == -1) return level;
	lua_error("invalid pattern capture");
	return 0;  // to avoid warnings
}


static const char *bracket_end(const char *p) {
	return (*p == 0) ? nullptr : strchr((*p == '^') ? p + 2 : p + 1, ']');
}

static int32 matchclass(int32 c, int32 cl) {
	int32 res;
	if (c == 0)
		return 0;
	switch (tolower((int)cl)) {
	case 'a' :
		res = Common::isAlpha(c);
		break;
	case 'c' :
		res = iscntrl(c);
		break;
	case 'd' :
		res = Common::isDigit(c);
		break;
	case 'l' :
		res = Common::isLower(c);
		break;
	case 'p' :
		res = ispunct(c);
		break;
	case 's' :
		res = Common::isSpace(c);
		break;
	case 'u' :
		res = Common::isUpper(c);
		break;
	case 'w' :
		res = Common::isAlnum(c);
		break;
	default:
		return (cl == c);
	}
	return (Common::isLower((byte)cl) ? res : !res);
}

int32 luaI_singlematch(int32 c, const char *p, const char **ep) {
	switch (*p) {
	case '.':  // matches any char
		*ep = p + 1;
		return c != 0;
	case '\0':  // end of pattern; matches nothing
		*ep = p;
		return 0;
	case ESC:
		if (*(++p) == '\0')
			luaL_verror("incorrect pattern (ends with `%c')", ESC);
		*ep = p+1;
		return matchclass(c, (byte)*p);
	case '[': {
		const char *end = bracket_end(p + 1);
		int32 sig = *(p + 1) == '^' ? (p++, 0) : 1;
		if (!end)
			lua_error("incorrect pattern (missing `]')");
		*ep = end + 1;
		if (c == 0)
			return 0;
		while (++p < end) {
			if (*p == ESC) {
				if (((p + 1) < end) && matchclass(c, (byte)*++p))
					return sig;
			} else if ((*(p + 1) == '-') && (p + 2 < end)) {
				p += 2;
				if ((byte)*(p - 2) <= c && c <= (byte)*p)
					return sig;
			} else if ((byte)*p == c)
				return sig;
		}
		return !sig;
	}
	default:
		*ep = p+1;
		return ((byte)*p == c);
	}
}

static const char *matchbalance(const char *s, int32 b, int32 e) {
	if (*s != b)
		return nullptr;
	else {
		int32 cont = 1;
		while (*(++s)) {
			if (*s == e) {
				if (--cont == 0)
					return s + 1;
			} else if (*s == b)
				cont++;
		}
	}
	return nullptr;  // string ends out of balance
}

static const char *matchitem(const char *s, const char *p, Capture *cap, const char **ep) {
	if (*p == ESC) {
		p++;
		if (Common::isDigit((byte)*p)) {  // capture
			int32 l = check_cap(*p, cap);
			*ep = p + 1;
			if (strncmp(cap->capture[l].init, s, cap->capture[l].len) == 0)
				return s + cap->capture[l].len;
			else
				return nullptr;
		} else if (*p == 'b') {  // balanced string
			p++;
			if (*p == 0 || *(p + 1) == 0)
				lua_error("unbalanced pattern");
			*ep = p + 2;
			return matchbalance(s, *p, *(p + 1));
		} else
			p--;  // and go through
	}
	return (luaI_singlematch((byte)*s, p, ep) ? s + 1 : nullptr);
}

static const char *match(const char *s, const char *p, Capture *cap) {
init:
	// using goto's to optimize tail recursion
	switch (*p) {
	case '(':
		{  // start capture
			const char *res;
			if (cap->level >= MAX_CAPT)
				lua_error("too many captures");
			cap->capture[cap->level].init = s;
			cap->capture[cap->level].len = -1;
			cap->level++;
			res = match(s, p + 1, cap);
			if (!res)  // match failed?
				cap->level--;  // undo capture
			return res;
		}
	case ')':
		{  // end capture
			int32 l = capture_to_close(cap);
			const char *res;
			cap->capture[l].len = s - cap->capture[l].init;  // close capture
			res = match(s, p + 1, cap);
			if (!res)  // match failed?
				cap->capture[l].len = -1;  // undo capture
			return res;
		}
	case '\0':
	case '$':  // (possibly) end of pattern
		if (*p == 0 || (*(p + 1) == 0 && *s == 0))
			return s;
		// fall through
	default:
		{  // it is a pattern item
			const char *ep;  // get what is next
			const char *s1 = matchitem(s, p, cap, &ep);
			switch (*ep) {
			case '*':
				{  // repetition
					const char *res;
					if (s1) {
						res = match(s1, p, cap);
						return res;
					}
					p = ep + 1;
					goto init;
				}
			case '?':
				{  // optional
					const char *res;
					if (s1) {
						res = match(s1, ep + 1, cap);
						return res;
					}
					p = ep + 1;
					goto init;
				}
			case '-':
				{  // repetition
					const char *res = match(s, ep + 1, cap);
					if (res)
						return res;
					else if (s1) {
						s = s1;
						goto init;
					} else
						return nullptr;
				}
			default:
				if (s1) {
					s = s1;
					p = ep;
					goto init;
				} else
					return nullptr;
			}
		}
	}
}

static void str_find() {
	const char *s = luaL_check_string(1);
	const char *p = luaL_check_string(2);
	int32 init = (uint32)luaL_opt_number(3, 1) - 1;
	luaL_arg_check(0 <= init && init <= (int32)strlen(s), 3, "out of range");
	if (lua_getparam(4) != LUA_NOOBJECT || !strpbrk(p, SPECIALS)) {  // no special characters?
			const char *s2 = strstr(s + init, p);
			if (s2) {
				lua_pushnumber(s2 - s + 1);
				lua_pushnumber(s2 - s + strlen(p));
			}
	} else {
		int32 anchor = (*p == '^') ? (p++, 1) : 0;
		const char *s1 = s + init;
		do {
			struct Capture cap;
			const char *res;
			cap.level = 0;
			res = match(s1, p, &cap);
			if (res) {
				lua_pushnumber(s1 - s + 1);  // start
				lua_pushnumber(res - s);   // end
				push_captures(&cap);
				return;
			}
		} while (*s1++ && !anchor);
	}
}

static void add_s(lua_Object newp, Capture *cap) {
	if (lua_isstring(newp)) {
		const char *news = lua_getstring(newp);
		while (*news) {
			if (*news != ESC || !Common::isDigit((byte)*++news))
				luaL_addchar(*news++);
			else {
				int l = check_cap(*news++, cap);
				addnchar(cap->capture[l].init, cap->capture[l].len);
			}
		}
	} else if (lua_isfunction(newp)) {
		lua_Object res;
		int32 status;
		int32 oldbuff;
		lua_beginblock();
		push_captures(cap);
		// function may use buffer, so save it and create a new one
		oldbuff = luaL_newbuffer(0);
		status = lua_callfunction(newp);
		// restore old buffer
		luaL_oldbuffer(oldbuff);
		if (status) {
			lua_endblock();
			lua_error(nullptr);
		}
		res = lua_getresult(1);
		addstr(lua_isstring(res) ? lua_getstring(res) : "");
		lua_endblock();
	} else
		luaL_arg_check(0, 3, "string or function expected");
}

static void str_gsub() {
	const char *src = luaL_check_string(1);
	const char *p = luaL_check_string(2);
	lua_Object newp = lua_getparam(3);
	int32 max_s = (int32)luaL_opt_number(4, strlen(src) + 1);
	int32 anchor = (*p == '^') ? (p++, 1) : 0;
	int32 n = 0;
	luaL_resetbuffer();
	while (n < max_s) {
		struct Capture cap;
		const char *e;
		cap.level = 0;
		e = match(src, p, &cap);
		if (e) {
			n++;
			add_s(newp, &cap);
		}
		if (e && e > src) // non empty match?
			src = e;  // skip it
		else if (*src)
			luaL_addchar(*src++);
		else
			break;
		if (anchor)
			break;
	}
	addstr(src);
	closeandpush();
	lua_pushnumber(n);  // number of substitutions
}

static void luaI_addquoted(const char *s) {
	luaL_addchar('"');
	for (; *s; s++) {
		if (strchr("\"\\\n", *s))
			luaL_addchar('\\');
		luaL_addchar(*s);
	}
	luaL_addchar('"');
}

#define MAX_FORMAT 200

static void str_format() {
	int32 arg = 1;
	const char *strfrmt = luaL_check_string(arg);
	luaL_resetbuffer();
	while (*strfrmt) {
		if (*strfrmt != '%')
			luaL_addchar(*strfrmt++);
		else if (*++strfrmt == '%')
			luaL_addchar(*strfrmt++);  // %%
		else { // format item
			char form[MAX_FORMAT];      // store the format ('%...')
			struct Capture cap;
			char *buff;
			const char *initf = strfrmt;
			form[0] = '%';
			cap.level = 0;
			strfrmt = match(strfrmt, "%d?%$?[-+ #]*(%d*)%.?(%d*)", &cap);
			if (cap.capture[0].len > 3 || cap.capture[1].len > 3)  // < 1000?
				lua_error("invalid format (width or precision too long)");
			if (Common::isDigit((byte)initf[0]) && initf[1] == '$') {
				arg = initf[0] - '0';
				initf += 2;  // skip the 'n$'
			}
			arg++;
			strncpy(form+1, initf, strfrmt - initf + 1); // +1 to include convertion
			form[strfrmt-initf + 2] = 0;
			buff = luaL_openspace(1000);  // to store the formatted value
			switch (*strfrmt++) {
			case 'q':
				luaI_addquoted(luaL_check_string(arg));
				continue;
			case 's':
				{
					const char *s = luaL_check_string(arg);
					buff = luaL_openspace(strlen(s));
					sprintf(buff, form, s);
					break;
				}
			case 'c':
			case 'd':
			case 'o':
			case 'i':
			case 'u':
			case 'x':
			case 'X':
				sprintf(buff, form, (int)luaL_check_number(arg));
				break;
			case 'e':
			case 'E':
			case 'f':
			case 'g':
			case 'G':
				sprintf(buff, form, luaL_check_number(arg));
				break;
			default:  // also treat cases 'pnLlh'
				lua_error("invalid option in `format'");
			}
			luaL_addsize(strlen(buff));
		}
	}
	closeandpush();  // push the result
}

static struct luaL_reg strlib[] = {
	{"strlen", str_len},
	{"strsub", str_sub},
	{"strlower", str_lower},
	{"strupper", str_upper},
	{"strrep", str_rep},
	{"format", str_format},
	{"strfind", str_find},
	{"gsub", str_gsub}
};

/*
** Open string library
*/
void strlib_open() {
	luaL_openlib(strlib, (sizeof(strlib) / sizeof(strlib[0])));
}

} // end of namespace Grim
