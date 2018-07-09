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

#include <discord_rpc.h>
#include "backends/discord/discord.h"
#include "common/str.h"

namespace Discord
{
	const char* ApplicationId = "465279538725912579";

	void Init()
	{
		DiscordEventHandlers handlers = {};
		Discord_Initialize(ApplicationId, &handlers, 1, nullptr);
		UpdateDiscordPresence("In launcher", "scummvm");
	}

	void UpdateDiscordPresence(Common::String gameName, Common::String gameId)
	{	
		DiscordRichPresence discord_presence = {};
		discord_presence.largeImageKey = gameId.c_str();
		discord_presence.largeImageText = gameName.c_str();
		discord_presence.smallImageKey = "scummvm";
		discord_presence.smallImageText = "ScummVM";
		discord_presence.details = gameName.c_str();
		discord_presence.startTimestamp = 0;
		Discord_UpdatePresence(&discord_presence);
	}

	void Shutdown()
	{
		Discord_ClearPresence();
		Discord_Shutdown();
	}	

} // namespace Discord