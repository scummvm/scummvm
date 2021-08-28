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

#include "ags/shared/core/platform.h"
#include "ags/ags.h"

#if AGS_PLATFORM_SCUMMVM

#include "ags/lib/allegro.h"
#include "ags/engine/ac/runtime_defines.h"
#include "ags/engine/gfx/gfx_defines.h"
#include "ags/engine/platform/base/ags_platform_driver.h"
#include "ags/plugins/ags_plugin.h"
#include "ags/shared/util/string.h"

namespace AGS3 {

using AGS::Shared::String;

struct ScummVMPlatformDriver : AGSPlatformDriver {
	virtual ~ScummVMPlatformDriver() {}

	int  CDPlayerCommand(int cmdd, int datt) override;
	void DisplayAlert(const char *, ...) override;
	const char *GetAllUsersDataDirectory() override;
	const char *GetUserSavedgamesDirectory() override;
	const char *GetUserConfigDirectory() override;
	const char *GetUserGlobalConfigDirectory() override;
	const char *GetAppOutputDirectory() override;
	unsigned long GetDiskFreeSpaceMB() override;
	const char *GetNoMouseErrorString() override;
	const char *GetAllegroFailUserHint() override;
	eScriptSystemOSID GetSystemOSID() override;
	int  InitializeCDPlayer() override;
	void PostAllegroExit() override;
	void SetGameWindowIcon() override;
	void ShutdownCDPlayer() override;
	bool LockMouseToWindow() override;
	void UnlockMouse() override;
	void GetSystemDisplayModes(std::vector<Engine::DisplayMode> &dms) override;
};


int ScummVMPlatformDriver::CDPlayerCommand(int cmdd, int datt) {
	warning("CDPlayerCommand(%d,%d)", cmdd, datt);
	//return cd_player_control(cmdd, datt);
	return 0;
}

void ScummVMPlatformDriver::DisplayAlert(const char *text, ...) {
	va_list ap;
	va_start(ap, text);
	Common::String msg = Common::String::vformat(text, ap);
	va_end(ap);

	if (_logToStdErr)
		debug("ERROR: %s\n", msg.c_str());
	else
		::AGS::g_vm->GUIError(msg);
}

const char *ScummVMPlatformDriver::GetAllUsersDataDirectory() {
	return "";
}

const char *ScummVMPlatformDriver::GetUserSavedgamesDirectory() {
	return "";
}

const char *ScummVMPlatformDriver::GetUserConfigDirectory() {
	return GetUserSavedgamesDirectory();
}

const char *ScummVMPlatformDriver::GetUserGlobalConfigDirectory() {
	return GetUserSavedgamesDirectory();
}

const char *ScummVMPlatformDriver::GetAppOutputDirectory() {
	return "";
}

unsigned long ScummVMPlatformDriver::GetDiskFreeSpaceMB() {
	// placeholder
	return 100;
}

const char *ScummVMPlatformDriver::GetNoMouseErrorString() {
	return "This game requires a mouse. You need to configure and setup your mouse to play this game.\n";
}

const char *ScummVMPlatformDriver::GetAllegroFailUserHint() {
	return nullptr;
}

eScriptSystemOSID ScummVMPlatformDriver::GetSystemOSID() {
	return eOS_Win;
}

int ScummVMPlatformDriver::InitializeCDPlayer() {
	//return cd_player_init();
	return 0;
}

void ScummVMPlatformDriver::PostAllegroExit() {
	// do nothing
}

void ScummVMPlatformDriver::SetGameWindowIcon() {
	// do nothing
}

void ScummVMPlatformDriver::ShutdownCDPlayer() {
	//cd_exit();
}

AGSPlatformDriver *AGSPlatformDriver::GetDriver() {
	if (instance == nullptr)
		instance = new ScummVMPlatformDriver();
	return instance;
}

bool ScummVMPlatformDriver::LockMouseToWindow() {
	return false;
}

void ScummVMPlatformDriver::UnlockMouse() {
}

void ScummVMPlatformDriver::GetSystemDisplayModes(std::vector<Engine::DisplayMode> &dms) {
	dms.clear();
	GFX_MODE_LIST *gmlist = get_gfx_mode_list(GFX_SCUMMVM_FULLSCREEN);
	for (int i = 0; i < gmlist->num_modes; ++i) {
		const GFX_MODE &m = gmlist->mode[i];
		dms.push_back(Engine::DisplayMode(Engine::GraphicResolution(m.width, m.height, m.bpp)));
	}
	destroy_gfx_mode_list(gmlist);
}

} // namespace AGS3

#endif
