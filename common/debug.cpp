/* Residual - A 3D game interpreter
 *
 * Residual is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
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
 *
 */

#include "common/debug.h"
#include "common/util.h"
#include "common/hashmap.h"

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

typedef HashMap<String, DebugChannel, IgnoreCase_Hash, IgnoreCase_EqualTo> DebugLevelMap;

static DebugLevelMap gDebugLevels;
static uint32 gDebugLevelsEnabled = 0;

struct DebugLevelComperator {
	bool operator()(const DebugChannel &l, const DebugChannel &r) {
		return (l.name.compareToIgnoreCase(r.name) < 0);
	}
};

}

bool addDebugChannel(uint32 level, const String &name, const String &description) {
	if (gDebugLevels.contains(name)) {
		warning("Duplicate declaration of engine debug level '%s'", name.c_str());
	}
	gDebugLevels[name] = DebugChannel(level, name, description);

	return true;
}

void clearAllDebugChannels() {
	gDebugLevelsEnabled = 0;
	gDebugLevels.clear();
}

bool enableDebugChannel(const String &name) {
	DebugLevelMap::iterator i = gDebugLevels.find(name);

	if (i != gDebugLevels.end()) {
		gDebugLevelsEnabled |= i->_value.level;
		i->_value.enabled = true;

		return true;
	} else {
		return false;
	}
}

bool disableDebugChannel(const String &name) {
	DebugLevelMap::iterator i = gDebugLevels.find(name);

	if (i != gDebugLevels.end()) {
		gDebugLevelsEnabled &= ~i->_value.level;
		i->_value.enabled = false;

		return true;
	} else {
		return false;
	}
}


DebugChannelList listDebugChannels() {
	DebugChannelList tmp;
	for (DebugLevelMap::iterator i = gDebugLevels.begin(); i != gDebugLevels.end(); ++i)
		tmp.push_back(i->_value);
	sort(tmp.begin(), tmp.end(), DebugLevelComperator());

	return tmp;
}

bool isDebugChannelEnabled(uint32 level) {
	// Debug level 11 turns on all special debug level messages
	if (gDebugLevel == 11)
		return true;
//	return gDebugLevelsEnabled & (1 << level);
	return gDebugLevelsEnabled & level;
}

bool isDebugChannelEnabled(const String &name) {
	// Debug level 11 turns on all special debug level messages
	if (gDebugLevel == 11)
		return true;

	// Search for the debug level with the given name and check if it is enabled
	DebugLevelMap::iterator i = gDebugLevels.find(name);
	if (i != gDebugLevels.end())
		return i->_value.enabled;
	return false;
}


}	// End of namespace Common


int gDebugLevel = -1;

#ifndef DISABLE_TEXT_CONSOLE

static void debugHelper(const char *s, va_list va, bool caret = true) {
	char in_buf[STRINGBUFLEN];
	char buf[STRINGBUFLEN];
	vsnprintf(in_buf, STRINGBUFLEN, s, va);

	strncpy(buf, in_buf, STRINGBUFLEN);

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

void debugC(int level, uint32 debugChannels, const char *s, ...) {
	va_list va;

	// Debug level 11 turns on all special debug level messages
	if (gDebugLevel != 11)
		if (level > gDebugLevel || !(Common::gDebugLevelsEnabled & debugChannels))
			return;

	va_start(va, s);
	debugHelper(s, va);
	va_end(va);
}

void debugCN(int level, uint32 debugChannels, const char *s, ...) {
	va_list va;

	// Debug level 11 turns on all special debug level messages
	if (gDebugLevel != 11)
		if (level > gDebugLevel || !(Common::gDebugLevelsEnabled & debugChannels))
			return;

	va_start(va, s);
	debugHelper(s, va, false);
	va_end(va);
}

void debugC(uint32 debugChannels, const char *s, ...) {
	va_list va;

	// Debug level 11 turns on all special debug level messages
	if (gDebugLevel != 11)
		if (!(Common::gDebugLevelsEnabled & debugChannels))
			return;

	va_start(va, s);
	debugHelper(s, va);
	va_end(va);
}

void debugCN(uint32 debugChannels, const char *s, ...) {
	va_list va;

	// Debug level 11 turns on all special debug level messages
	if (gDebugLevel != 11)
		if (!(Common::gDebugLevelsEnabled & debugChannels))
			return;

	va_start(va, s);
	debugHelper(s, va, false);
	va_end(va);
}

#endif
