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
 */

#ifndef COMMON_DEBUG_CHANNELS_H
#define COMMON_DEBUG_CHANNELS_H

#include "common/scummsys.h"

#include "common/hashmap.h"
#include "common/hash-str.h"
#include "common/list.h"
#include "common/singleton.h"
#include "common/str.h"


namespace Common {

/**
 * @defgroup common_debug_channels Debug channels
 * @ingroup common
 *
 * @brief  Functions for managing debug channels.
 * @{
 */

// TODO: Find a better name for this
class DebugManager : public Singleton<DebugManager> {
public:

	struct DebugChannel {
		DebugChannel() : channel(0), enabled(false) {}
		DebugChannel(uint32 c, const String &n, const String &d)
			: name(n), description(d), channel(c), enabled(false) {}

		String name; /*!< Name of the channel */
		String description; /*!< Description of the channel */

		uint32 channel; /*!< Channel number. */
		bool enabled; /*!< Whether the channel is enabled. */
	};

	/**
	 * Add a debug channel.
	 *
	 * A debug channel is considered roughly similar to what the debug levels described by
	 * gDebugLevel try to achieve:
	 *
	 * Debug channels should only affect the display of additional debug output, based on
	 * their state. That is, if they are enabled, channel-specific debug messages should
	 * be shown. If they are disabled on the other hand, those messages will be hidden.
	 *
	 * @see gDebugLevel
	 *
	 * Note that we have debug* functions that depend both on the debug level set and
	 * specific debug channels. Those functions will only show output, when *both* criteria
	 * are satisfied.
	 *
	 * @param channel     Channel flag (should be OR-able i.e. first one should be 1 then 2, 4, etc.).
	 * @param name        The option name that is used in the debugger/on the command line to enable
	 *                    this special debug level (case will be ignored).
	 * @param description The description that shows up in the debugger.
	 *
	 * @return True on success, false on failure.
	 */
	bool addDebugChannel(uint32 channel, const String &name, const String &description);

	/**
	 * Reset all engine-specific debug channels.
	 */
	void clearAllDebugChannels();

	/**
	 * Enable a debug channel.
	 *
	 * @param name Name of the debug channel to enable.
	 * @return True on success, false on failure.
	 */
	bool enableDebugChannel(const String &name);

	/**
	 * @overload enableDebugChannel(uint32 channel)
	 *
	 * @param channel Debug channel.
	 * @return True on success, false on failure.
	 */
	bool enableDebugChannel(uint32 channel);

	/**
	 * Disable a debug channel.
	 *
	 * @param name Name of the debug channel to disable.
	 * @return True on success, false on failure.
	 */
	bool disableDebugChannel(const String &name);

	/**
	 * @overload bool disableDebugChannel(uint32 channel)
	 *
	 * @param channel The debug channel
	 * @return true on success, false on failure
	 */
	bool disableDebugChannel(uint32 channel);

	typedef List<DebugChannel> DebugChannelList;

	/**
	 * Lists all engine specific debug channels.
	 *
	 * @return returns an array with all debug channels
	 */
	DebugChannelList listDebugChannels();

	/**
	 * Enable all debug channels.
	 */
	void enableAllDebugChannels();

	/**
	 * Disable all debug channels.
	 */
	void disableAllDebugChannels();

	/**
	 * Test whether the given debug channel is enabled.
	 */
	bool isDebugChannelEnabled(uint32 channel, bool enforce = false);

private:
	typedef HashMap<String, DebugChannel, IgnoreCase_Hash, IgnoreCase_EqualTo> DebugChannelMap;

	DebugChannelMap gDebugChannels;
	uint32 gDebugChannelsEnabled;

	friend class Singleton<SingletonBaseType>;
	DebugManager() : gDebugChannelsEnabled(0) {}
};

/** Shortcut for accessing the Debug Manager. */
#define DebugMan		Common::DebugManager::instance()

/** @} */

} // End of namespace Common

#endif
