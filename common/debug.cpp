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
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 */

#include "common/debug.h"
#include "common/util.h"

#include "engines/engine.h"

#include <stdarg.h>	// For va_list etc.


#ifdef __PLAYSTATION2__
	// for those replaced fopen/fread/etc functions
	typedef unsigned long	uint64;
	typedef signed long	int64;
	#include "backends/platform/ps2/fileio.h"

	#define fprintf				ps2_fprintf
	#define fputs(str, file)	ps2_fputs(str, file)
	#define fflush(a)			ps2_fflush(a)
#endif

#ifdef __DS__
	#include "backends/fs/ds/ds-fs.h"

	void	std_fprintf(FILE* handle, const char* fmt, ...);
	void	std_fflush(FILE* handle);

	#define fprintf(file, fmt, ...)				do { char str[128]; sprintf(str, fmt, ##__VA_ARGS__); DS::std_fwrite(str, strlen(str), 1, file); } while(0)
	#define fputs(str, file)					DS::std_fwrite(str, strlen(str), 1, file)
	#define fflush(file)						DS::std_fflush(file)
#endif


namespace Common {

namespace {

DebugLevelContainer gDebugLevels;
uint32 gDebugLevelsEnabled = 0;

struct DebugLevelSort {
	bool operator()(const EngineDebugLevel &l, const EngineDebugLevel &r) {
		return (l.option.compareToIgnoreCase(r.option) < 0);
	}
};

struct DebugLevelSearch {
	const String &_option;

	DebugLevelSearch(const String &option) : _option(option) {}

	bool operator()(const EngineDebugLevel &l) {
		return _option.equalsIgnoreCase(l.option);
	}
};

}

bool addSpecialDebugLevel(uint32 level, const String &option, const String &description) {
	DebugLevelContainer::iterator i = find_if(gDebugLevels.begin(), gDebugLevels.end(), DebugLevelSearch(option));

	if (i != gDebugLevels.end()) {
		warning("Declared engine debug level '%s' again", option.c_str());
		*i = EngineDebugLevel(level, option, description);
	} else {
		gDebugLevels.push_back(EngineDebugLevel(level, option, description));
		sort(gDebugLevels.begin(), gDebugLevels.end(), DebugLevelSort());
	}

	return true;
}

void clearAllSpecialDebugLevels() {
	gDebugLevelsEnabled = 0;
	gDebugLevels.clear();
}

bool enableSpecialDebugLevel(const String &option) {
	DebugLevelContainer::iterator i = find_if(gDebugLevels.begin(), gDebugLevels.end(), DebugLevelSearch(option));

	if (i != gDebugLevels.end()) {
		gDebugLevelsEnabled |= i->level;
		i->enabled = true;

		return true;
	} else {
		return false;
	}
}

void enableSpecialDebugLevelList(const String &option) {
	StringTokenizer tokenizer(option, " ,");
	String token;

	while (!tokenizer.empty()) {
		token = tokenizer.nextToken();
		if (!enableSpecialDebugLevel(token))
			warning("Engine does not support debug level '%s'", token.c_str());
	}
}

bool disableSpecialDebugLevel(const String &option) {
	DebugLevelContainer::iterator i = find_if(gDebugLevels.begin(), gDebugLevels.end(), DebugLevelSearch(option));

	if (i != gDebugLevels.end()) {
		gDebugLevelsEnabled &= ~i->level;
		i->enabled = false;

		return true;
	} else {
		return false;
	}
}

const DebugLevelContainer &listSpecialDebugLevels() {
	return gDebugLevels;
}

uint32 getEnabledSpecialDebugLevels() {
	return gDebugLevelsEnabled;
}

}	// End of namespace Common


int gDebugLevel = -1;

#ifndef DISABLE_TEXT_CONSOLE

static void debugHelper(const char *s, va_list va, bool caret = true) {
	char in_buf[STRINGBUFLEN];
	char buf[STRINGBUFLEN];
	vsnprintf(in_buf, STRINGBUFLEN, s, va);

	// Next, give the active engine (if any) a chance to augment the message,
	// but only if not used from debugN.
	if (g_engine && caret) {
		g_engine->errorString(in_buf, buf, STRINGBUFLEN);
	} else {
		strncpy(buf, in_buf, STRINGBUFLEN);
	}
	buf[STRINGBUFLEN-1] = '\0';

	if (caret) {
		buf[STRINGBUFLEN-2] = '\0';
		strcat(buf, "\n");
	}

	fputs(buf, stdout);

#if defined( USE_WINDBG )
#if defined( _WIN32_WCE )
	TCHAR buf_unicode[1024];
	MultiByteToWideChar(CP_ACP, 0, buf, strlen(buf) + 1, buf_unicode, sizeof(buf_unicode));
	OutputDebugString(buf_unicode);
#else
	OutputDebugString(buf);
#endif
#endif

	fflush(stdout);
}

void debug(const char *s, ...) {
	va_list va;

	va_start(va, s);
	debugHelper(s, va);
	va_end(va);
}

void debug(int level, const char *s, ...) {
	va_list va;

	if (level > gDebugLevel)
		return;

	va_start(va, s);
	debugHelper(s, va);
	va_end(va);

}

void debugN(int level, const char *s, ...) {
	va_list va;

	if (level > gDebugLevel)
		return;

	va_start(va, s);
	debugHelper(s, va, false);
	va_end(va);
}

void debugC(int level, uint32 engine_level, const char *s, ...) {
	va_list va;

	if (gDebugLevel != 11)
		if (level > gDebugLevel || !(Common::gDebugLevelsEnabled & engine_level))
			return;

	va_start(va, s);
	debugHelper(s, va);
	va_end(va);
}

#endif
