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

#ifndef BACKENDS_DISCORD_H
#define BACKENDS_DISCORD_H

#include "common/scummsys.h"

#ifdef USE_DISCORD
#include "common/str.h"

/**
 * Manager for interacting with the Discord Rich Presence API.
 */
class DiscordPresence {
public:
	DiscordPresence();
	~DiscordPresence();
	/**
	 * Updates the Discord presence status with game information.
	 * Blank parameters default to no game running (Launcher).
	 *
	 * @param name Game ID and icon to display.
	 * @param description Game name to display.
	 */
	void updateStatus(const Common::String &name, const Common::String &description);
};

#endif

#endif
