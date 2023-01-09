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

#define FORBIDDEN_SYMBOL_EXCEPTION_time_h

#ifdef WIN32
#define DISCORD_CALL __cdecl
#else
#define DISCORD_CALL
#endif

#include "backends/presence/discord/discord.h"

#ifdef USE_DISCORD
#include "common/config-manager.h"
#include "common/translation.h"

#include <discord_rpc.h>
#include <time.h>

#ifdef WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#endif

#define DISCORD_CLIENT_ID "714287866464698470"

static void (DISCORD_CALL *Discord_Initialize_dyn)(const char* applicationId,
                                       DiscordEventHandlers* handlers,
                                       int autoRegister,
                                       const char* optionalSteamId);
static void (DISCORD_CALL *Discord_Shutdown_dyn)(void);
static void (DISCORD_CALL *Discord_UpdatePresence_dyn)(const DiscordRichPresence* presence);
static void (DISCORD_CALL *Discord_ClearPresence_dyn)(void);

void loadDiscordLib() {
	static bool attempted = false;
	if (attempted)
		return;
	attempted = true;
#ifdef WIN32
	HINSTANCE handle = LoadLibrary(TEXT("discord-rpc.dll"));
	if (!handle) {
		warning("Failed to load discord-rpc.dll");
		return;
	}
	*(void **) (&Discord_Initialize_dyn) = GetProcAddress(handle, "Discord_Initialize");
	*(void **) (&Discord_Shutdown_dyn) = GetProcAddress(handle, "Discord_Shutdown");
	*(void **) (&Discord_UpdatePresence_dyn) = GetProcAddress(handle, "Discord_UpdatePresence");
	*(void **) (&Discord_ClearPresence_dyn) = GetProcAddress(handle, "Discord_ClearPresence");
#else
	void *handle = dlopen("discord-rpc.so", RTLD_NOW | RTLD_GLOBAL);
	if (!handle) {
		warning("Failed to load discord-rpc.so: %s", dlerror());
		return;
	}
	dlerror(); // Clear error
	*(void **) (&Discord_Initialize_dyn) = dlsym(handle, "Discord_Initialize");
	*(void **) (&Discord_Shutdown_dyn) = dlsym(handle, "Discord_Shutdown");
	*(void **) (&Discord_UpdatePresence_dyn) = dlsym(handle, "Discord_UpdatePresence");
	*(void **) (&Discord_ClearPresence_dyn) = dlsym(handle, "Discord_ClearPresence");
#endif

	// We need either all symbols or none
	if (!Discord_Initialize_dyn || !Discord_Shutdown_dyn
	    || !Discord_UpdatePresence_dyn || !Discord_ClearPresence_dyn) {
#ifdef WIN32
		warning("Failed to load discord-rpc.dll");
#else
		warning("Failed to load discord-rpc.so: %s", dlerror());
#endif
		Discord_Initialize_dyn = nullptr;
		Discord_Shutdown_dyn = nullptr;
		Discord_UpdatePresence_dyn = nullptr;
		Discord_ClearPresence_dyn = nullptr;
	}
}


DiscordPresence::DiscordPresence() {
	loadDiscordLib();
	if (Discord_Initialize_dyn == nullptr) {
		return;
	}
	Discord_Initialize_dyn(DISCORD_CLIENT_ID, nullptr, 0, nullptr);
	updateStatus("", "");
}

DiscordPresence::~DiscordPresence() {
	if (Discord_ClearPresence_dyn == nullptr)
		return;
	Discord_ClearPresence_dyn();
	Discord_Shutdown_dyn();
}

void DiscordPresence::updateStatus(const Common::String &name, const Common::String &description) {
	if (Discord_UpdatePresence_dyn == nullptr)
		return;
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
	if (ConfMan.getBool("discord_rpc", Common::ConfigManager::kApplicationDomain)) {
		Discord_UpdatePresence_dyn(&presence);
	} else {
		Discord_ClearPresence_dyn();
	}
}

#endif
