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

#define FORBIDDEN_SYMBOL_EXCEPTION_time_h
#include "backends/presence/discord/discord.h"

#ifdef USE_DISCORD
#include "common/translation.h"

#include <discord_rpc.h>
#include <time.h>

#define DISCORD_CLIENT_ID "714287866464698470"

DiscordPresence::DiscordPresence() {
	Discord_Initialize(DISCORD_CLIENT_ID, nullptr, 0, nullptr);
	updateStatus("", "");
}

DiscordPresence::~DiscordPresence() {
	Discord_ClearPresence();
	Discord_Shutdown();
}

void DiscordPresence::updateStatus(const Common::String &name, const Common::String &description) {
	Common::String gameName = name.empty() ? "scummvm" : name;
	Common::String gameDesc = description.empty() ? _("Launcher").encode() : description;

	DiscordRichPresence presence;
	memset(&presence, 0, sizeof(presence));
	presence.largeImageKey = gameName.c_str();
	presence.largeImageText = gameDesc.c_str();
	presence.details = gameDesc.c_str();
	presence.smallImageKey = "scummvm";
	presence.smallImageText = "ScummVM";
	presence.startTimestamp = time(0);
	Discord_UpdatePresence(&presence);
}

#endif
