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
#include "common/hashmap.h"
#include "common/hash-str.h"

#include <stdarg.h>	// For va_list etc.


#ifdef __PLAYSTATION2__
	// for those replaced fopen/fread/etc functions
	#include "backends/platform/ps2/fileio.h"

	#define fputs(str, file)	ps2_fputs(str, file)
	#define fflush(a)			ps2_fflush(a)
#endif

#ifdef __DS__
	#include "backends/fs/ds/ds-fs.h"

	#define fputs(str, file)	DS::std_fwrite(str, strlen(str), 1, file)
	#define fflush(file)		DS::std_fflush(file)
#endif

// TODO: Move gDebugLevel into namespace Common.
int gDebugLevel = -1;

namespace Common {

namespace {

typedef HashMap<String, DebugChannel, IgnoreCase_Hash, IgnoreCase_EqualTo> DebugChannelMap;

static DebugChannelMap gDebugChannels;
static uint32 gDebugChannelsEnabled = 0;

struct DebugLevelComperator {
	bool operator()(const DebugChannel &l, const DebugChannel &r) {
		return (l.name.compareToIgnoreCase(r.name) < 0);
	}
};

} // end of anonymous namespace

bool addDebugChannel(uint32 channel, const String &name, const String &description) {
	if (gDebugChannels.contains(name))
		warning("Duplicate declaration of engine debug channel '%s'", name.c_str());

	gDebugChannels[name] = DebugChannel(channel, name, description);

	return true;
}

void clearAllDebugChannels() {
	gDebugChannelsEnabled = 0;
	gDebugChannels.clear();
}

bool enableDebugChannel(const String &name) {
	DebugChannelMap::iterator i = gDebugChannels.find(name);

	if (i != gDebugChannels.end()) {
		gDebugChannelsEnabled |= i->_value.channel;
		i->_value.enabled = true;

		return true;
	} else {
		return false;
	}
}

bool disableDebugChannel(const String &name) {
	DebugChannelMap::iterator i = gDebugChannels.find(name);

	if (i != gDebugChannels.end()) {
		gDebugChannelsEnabled &= ~i->_value.channel;
		i->_value.enabled = false;

		return true;
	} else {
		return false;
	}
}


DebugChannelList listDebugChannels() {
	DebugChannelList tmp;
	for (DebugChannelMap::iterator i = gDebugChannels.begin(); i != gDebugChannels.end(); ++i)
		tmp.push_back(i->_value);
	sort(tmp.begin(), tmp.end(), DebugLevelComperator());

	return tmp;
}

bool isDebugChannelEnabled(uint32 channel) {
	// Debug level 11 turns on all special debug level messages
	if (gDebugLevel == 11)
		return true;
	else
		return (gDebugChannelsEnabled & channel) != 0;
}



static OutputFormatter s_debugOutputFormatter = 0;

void setDebugOutputFormatter(OutputFormatter f) {
	s_debugOutputFormatter = f;
}

}	// End of namespace Common


#ifndef DISABLE_TEXT_CONSOLE

static void debugHelper(const char *s, va_list va, bool caret = true) {
	char in_buf[STRINGBUFLEN];
	char buf[STRINGBUFLEN];
	vsnprintf(in_buf, STRINGBUFLEN, s, va);

	// Next, give the active engine (if any) a chance to augment the message,
	// but only if not used from debugN.
	if (caret && Common::s_debugOutputFormatter) {
		(*Common::s_debugOutputFormatter)(buf, in_buf, STRINGBUFLEN);
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

void debugC(int level, uint32 debugChannels, const char *s, ...) {
	va_list va;

	// Debug level 11 turns on all special debug level messages
	if (gDebugLevel != 11)
		if (level > gDebugLevel || !(Common::gDebugChannelsEnabled & debugChannels))
			return;

	va_start(va, s);
	debugHelper(s, va);
	va_end(va);
}

void debugCN(int level, uint32 debugChannels, const char *s, ...) {
	va_list va;

	// Debug level 11 turns on all special debug level messages
	if (gDebugLevel != 11)
		if (level > gDebugLevel || !(Common::gDebugChannelsEnabled & debugChannels))
			return;

	va_start(va, s);
	debugHelper(s, va, false);
	va_end(va);
}

void debugC(uint32 debugChannels, const char *s, ...) {
	va_list va;

	// Debug level 11 turns on all special debug level messages
	if (gDebugLevel != 11)
		if (!(Common::gDebugChannelsEnabled & debugChannels))
			return;

	va_start(va, s);
	debugHelper(s, va);
	va_end(va);
}

void debugCN(uint32 debugChannels, const char *s, ...) {
	va_list va;

	// Debug level 11 turns on all special debug level messages
	if (gDebugLevel != 11)
		if (!(Common::gDebugChannelsEnabled & debugChannels))
			return;

	va_start(va, s);
	debugHelper(s, va, false);
	va_end(va);
}

#endif
