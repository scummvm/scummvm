/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "common/debug.h"
#include "common/debug-channels.h"
#include "common/system.h"
#include "common/textconsole.h"
#include "common/algorithm.h"

#include <stdarg.h>	// For va_list etc.

// TODO: Move gDebugLevel into namespace Common.
int gDebugLevel = -1;
bool gDebugChannelsOnly = false;

const DebugChannelDef gDebugChannels[] = {
	{ kDebugLevelEventRec,   "eventrec",  "Event recorder debug level" },
	{ kDebugGlobalDetection, "detection", "debug messages for advancedDetector" },
	{ kDebugLevelMainGUI,    "maingui",   "debug messages for GUI" },
	{ kDebugLevelMacGUI,     "macgui",    "debug messages for MacGUI" },
	DEBUG_CHANNEL_END
};
namespace Common {

DECLARE_SINGLETON(DebugManager);

namespace {

struct DebugLevelComperator {
	bool operator()(const DebugManager::DebugChannel &l, const DebugManager::DebugChannel &r) {
		return (l.name.compareToIgnoreCase(r.name) < 0);
	}
};

} // end of anonymous namespace

DebugManager::DebugManager() {
	addDebugChannels(gDebugChannels);

	// Create global debug channels mask
	_globalChannelsMask = 0;
	for (uint i = 0; gDebugChannels[i].channel != 0; ++i) {
		_globalChannelsMask |= gDebugChannels[i].channel;
	}
}

bool DebugManager::addDebugChannel(uint32 channel, const String &name, const String &description) {
	if (name.equalsIgnoreCase("all")) {
		warning("Debug channel 'all' is reserved for internal use");
		return false;
	}

	if (_debugChannels.contains(name))
		warning("Duplicate declaration of engine debug channel '%s'", name.c_str());

	for (DebugChannelMap::iterator i = _debugChannels.begin(); i != _debugChannels.end(); ++i)
		if (i->_value.channel == channel)
			error("Duplicate engine debug channel id '%d' for flag '%s'", channel, name.c_str());

	_debugChannels[name] = DebugChannel(channel, name, description);

	return true;
}

void DebugManager::addAllDebugChannels(const DebugChannelDef *channels) {
	removeAllDebugChannels();

	if (channels) {
		addDebugChannels(channels);
	}
}

void DebugManager::removeAllDebugChannels() {
	EnabledChannelsMap oldMap = _debugChannelsEnabled;

	_debugChannelsEnabled.clear();
	_debugChannels.clear();
	addDebugChannels(gDebugChannels);

	for (DebugChannelMap::iterator i = _debugChannels.begin(); i != _debugChannels.end(); ++i)
		if (oldMap.contains(i->_value.channel))
			_debugChannelsEnabled[i->_value.channel] = oldMap[i->_value.channel];
}

bool DebugManager::enableDebugChannel(const String &name) {
	DebugChannelMap::iterator i = _debugChannels.find(name);

	if (i != _debugChannels.end()) {
		_debugChannelsEnabled[i->_value.channel] = true;

		return true;
	} else {
		return false;
	}
}

bool DebugManager::enableDebugChannel(uint32 channel) {
	_debugChannelsEnabled[channel] = true;
	return true;
}

bool DebugManager::disableDebugChannel(const String &name) {
	DebugChannelMap::iterator i = _debugChannels.find(name);

	if (i != _debugChannels.end()) {
		_debugChannelsEnabled[i->_value.channel] = false;

		return true;
	} else {
		return false;
	}
}

bool DebugManager::disableDebugChannel(uint32 channel) {
	_debugChannelsEnabled[channel] = false;
	return true;
}

DebugManager::DebugChannelList DebugManager::getDebugChannels() {
	DebugChannelList tmp;
	for (DebugChannelMap::iterator i = _debugChannels.begin(); i != _debugChannels.end(); ++i)
		tmp.push_back(i->_value);
	sort(tmp.begin(), tmp.end(), DebugLevelComperator());

	return tmp;
}

void DebugManager::enableAllDebugChannels() {
	for (DebugChannelMap::iterator i = _debugChannels.begin(); i != _debugChannels.end(); ++i)
		enableDebugChannel(i->_value.name);
}

void DebugManager::disableAllDebugChannels() {
	for (DebugChannelMap::iterator i = _debugChannels.begin(); i != _debugChannels.end(); ++i)
		disableDebugChannel(i->_value.name);
}

bool DebugManager::isDebugChannelEnabled(uint32 channel, bool enforce) {
	// Debug level 11 turns on all special debug level messages
	if (gDebugLevel == 11 && enforce == false)
		return true;
	else
		return (_debugChannelsEnabled.contains(channel) && _debugChannelsEnabled[channel] == true);
}

void DebugManager::addDebugChannels(const DebugChannelDef *channels) {
	int added = 0;
	for (uint i = 0; channels[i].channel != 0; ++i) {
		addDebugChannel(channels[i].channel, channels[i].name, channels[i].description);
		added++;
	}

	if (!added)
		warning("DebugManager::addDebugChannels(): No debug channels were added, list is empty");
}

} // End of namespace Common

bool debugLevelSet(int level) {
	return level <= gDebugLevel;
}

bool debugChannelSet(int level, uint32 debugChannels) {
	if (gDebugLevel != 11 || level == -1)
		if ((level != -1 && level > gDebugLevel) || !(DebugMan.isDebugChannelEnabled(debugChannels, level == -1)))
			return false;

	return true;
}


#ifndef DISABLE_TEXT_CONSOLE

static void debugHelper(const char *s, va_list va, int level, uint32 debugChannels, bool caret = true) {
	Common::String buf = Common::String::vformat(s, va);

	if (caret)
		buf += '\n';

	Common::LogWatcher logWatcher = Common::getLogWatcher();
	if (logWatcher)
		(*logWatcher)(LogMessageType::kDebug, level, debugChannels, buf.c_str());

	if (g_system)
		g_system->logMessage(LogMessageType::kDebug, buf.c_str());
	// TODO: Think of a good fallback in case we do not have
	// any OSystem yet.
}

void debug(const char *s, ...) {
	va_list va;

	if (gDebugChannelsOnly)
		return;

	va_start(va, s);
	debugHelper(s, va, 0, 0);
	va_end(va);
}

void debug(int level, const char *s, ...) {
	va_list va;

	if (level > gDebugLevel || gDebugChannelsOnly)
		return;

	va_start(va, s);
	debugHelper(s, va, level, 0);
	va_end(va);

}

void debugN(const char *s, ...) {
	va_list va;

	if (gDebugChannelsOnly)
		return;

	va_start(va, s);
	debugHelper(s, va, 0, 0, false);
	va_end(va);
}

void debugN(int level, const char *s, ...) {
	va_list va;

	if (level > gDebugLevel || gDebugChannelsOnly)
		return;

	va_start(va, s);
	debugHelper(s, va, level, 0, false);
	va_end(va);
}

void debugC(int level, uint32 debugChannels, const char *s, ...) {
	va_list va;

	// Debug level 11 turns on all special debug level messages
	if (gDebugLevel != 11)
		if (level > gDebugLevel || !(DebugMan.isDebugChannelEnabled(debugChannels)))
			return;

	va_start(va, s);
	debugHelper(s, va, level, debugChannels);
	va_end(va);
}

void debugCN(int level, uint32 debugChannels, const char *s, ...) {
	va_list va;

	// Debug level 11 turns on all special debug level messages
	if (gDebugLevel != 11)
		if (level > gDebugLevel || !(DebugMan.isDebugChannelEnabled(debugChannels)))
			return;

	va_start(va, s);
	debugHelper(s, va, level, debugChannels, false);
	va_end(va);
}

void debugC(uint32 debugChannels, const char *s, ...) {
	va_list va;

	// Debug level 11 turns on all special debug level messages
	if (gDebugLevel != 11)
		if (!(DebugMan.isDebugChannelEnabled(debugChannels)))
			return;

	va_start(va, s);
	debugHelper(s, va, 0, debugChannels);
	va_end(va);
}

void debugCN(uint32 debugChannels, const char *s, ...) {
	va_list va;

	// Debug level 11 turns on all special debug level messages
	if (gDebugLevel != 11)
		if (!(DebugMan.isDebugChannelEnabled(debugChannels)))
			return;

	va_start(va, s);
	debugHelper(s, va, 0, debugChannels, false);
	va_end(va);
}

#endif
