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

#ifndef COMMON_DEBUG_H
#define COMMON_DEBUG_H

#include "common/scummsys.h"
#include "common/list.h"
#include "common/str.h"


namespace Common {


struct DebugChannel {
	DebugChannel() : level(0), enabled(false) {}
	DebugChannel(uint32 l, const String &n, const String &d)
		: name(n), description(d), level(l), enabled(false) {}

	String name;
	String description;

	uint32 level;
	bool enabled;
};

/**
 * Adds a engine debug level.
 * @param level the level flag (should be OR-able i.e. first one should be 1 than 2,4,...)
 * @param name the option name which is used in the debugger/on the command line to enable
 *               this special debug level (case will be ignored)
 * @param description the description which shows up in the debugger
 * @return true on success false on failure
 */
bool addDebugChannel(uint32 level, const String &name, const String &description);

/**
 * Resets all engine debug levels.
 */
void clearAllDebugChannels();

/**
 * Enables an engine debug level.
 * @param name the name of the debug level to enable
 * @return true on success, false on failure
 */
bool enableDebugChannel(const String &name);

/**
 * Disables an engine debug level
 * @param name the name of the debug level to disable
 * @return true on success, false on failure
 */
bool disableDebugChannel(const String &name);



typedef List<DebugChannel> DebugChannelList;

/**
 * Lists all debug levels
 * @return returns a arry with all debug levels
 */
DebugChannelList listDebugChannels();


/**
 * Test whether the given debug level is enabled.
 */
bool isDebugChannelEnabled(uint32 level);

/**
 * Test whether the given debug level is enabled.
 */
bool isDebugChannelEnabled(const String &name);


}	// End of namespace Common


#ifdef DISABLE_TEXT_CONSOLE

inline void debug(const char *s, ...) {}
inline void debug(int level, const char *s, ...) {}
inline void debugN(int level, const char *s, ...) {}
inline void debugC(int level, uint32 engine_level, const char *s, ...) {}
inline void debugC(uint32 engine_level, const char *s, ...) {}
inline void debugCN(int level, uint32 engine_level, const char *s, ...) {}
inline void debugCN(uint32 engine_level, const char *s, ...) {}


#else


/**
 * Print a debug message to the text console (stdout).
 * Automatically appends a newline.
 */
void debug(const char *s, ...) GCC_PRINTF(1, 2);

/**
 * Print a debug message to the text console (stdout), but only if
 * the specified level does not exceed the value of gDebugLevel.
 * As a rule of thumb, the more important the message, the lower the level.
 * Automatically appends a newline.
 */
void debug(int level, const char *s, ...) GCC_PRINTF(2, 3);

/**
 * Print a debug message to the text console (stdout), but only if
 * the specified level does not exceed the value of gDebugLevel.
 * As a rule of thumb, the more important the message, the lower the level.
 * Does not append a newline.
 */
void debugN(int level, const char *s, ...) GCC_PRINTF(2, 3);

/**
 * Print a debug message to the text console (stdout), but only if
 * the specified level does not exceed the value of gDebugLevel OR
 * if the specified special debug level is active.
 * As a rule of thumb, the more important the message, the lower the level.
 * Automatically appends a newline.
 *
 * @see enableDebugChannel
 */
void debugC(int level, uint32 debugChannels, const char *s, ...) GCC_PRINTF(3, 4);

/**
 * Print a debug message to the text console (stdout), but only if
 * the specified level does not exceed the value of gDebugLevel OR
 * if the specified special debug level is active.
 * As a rule of thumb, the more important the message, the lower the level.
 * Does not append a newline automatically.
 *
 * @see enableDebugChannel
 */
void debugCN(int level, uint32 debugChannels, const char *s, ...) GCC_PRINTF(3, 4);

/**
 * Print a debug message to the text console (stdout), but only if
 * the specified special debug level is active.
 * Automatically appends a newline.
 *
 * @see enableDebugChannel
 */
void debugC(uint32 debugChannels, const char *s, ...) GCC_PRINTF(2, 3);

/**
 * Print a debug message to the text console (stdout), but only if
 * the specified special debug level is active.
 * Does not append a newline automatically.
 *
 * @see enableDebugChannel
 */
void debugCN(uint32 debugChannels, const char *s, ...) GCC_PRINTF(2, 3);

#endif

/**
 * The debug level. Initially set to -1, indicating that no debug output
 * should be shown. Positive values usually imply an increasing number of
 * debug output shall be generated, the higher the value, the more verbose the
 * information (although the exact semantics are up to the engines).
 */
extern int gDebugLevel;


#endif
