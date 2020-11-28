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

#if AGS_PLATFORM_OS_LINUX

// ********* LINUX PLACEHOLDER DRIVER *********

//include <stdio.h>
//include <xalleg.h>
//include <libcda.h>
//include <pwd.h>
//include <sys/stat.h>
#include "ags/lib/allegro.h"
#include "ags/lib/opengl/opengl.h"
#include "ags/engine/ac/runtime_defines.h"
#include "ags/engine/gfx/gfxdefines.h"
#include "ags/engine/platform/base/agsplatformdriver.h"
#include "ags/engine/plugin/agsplugin.h"
#include "ags/shared/util/string.h"

namespace AGS3 {

using AGS::Shared::String;


// Replace the default Allegro icon. The original defintion is in the
// Allegro 4.4 source under "src/x/xwin.c".
//include "ags/shared/icon.xpm"
//void *allegro_icon = icon_xpm;
String CommonDataDirectory;
String UserDataDirectory;

struct AGSLinux : AGSPlatformDriver {

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


int AGSLinux::CDPlayerCommand(int cmdd, int datt) {
	warning("CDPlayerCommand(%d,%d)", cmdd, datt);
	//return cd_player_control(cmdd, datt);
	return 0;
}

void AGSLinux::DisplayAlert(const char *text, ...) {
	char displbuf[2000];
	va_list ap;
	va_start(ap, text);
	vsprintf(displbuf, text, ap);
	va_end(ap);
	if (_logToStdErr)
		debug("ERROR: %s\n", displbuf);
	else
		debug("ERROR: %s\n", displbuf);
}

size_t BuildXDGPath(char *destPath, size_t destSize) {
#ifdef TODO
	// Check to see if XDG_DATA_HOME is set in the enviroment
	const char *home_dir = getenv("XDG_DATA_HOME");
	size_t l = 0;
	if (home_dir) {
		l = snprintf(destPath, destSize, "%s", home_dir);
	} else {
		// No evironment variable, so we fall back to home dir in /etc/passwd
		struct passwd *p = getpwuid(getuid());
		l = snprintf(destPath, destSize, "%s/.local", p->pw_dir);
		if (mkdir(destPath, 0755) != 0 && errnum != EEXIST)
			return 0;
		l += snprintf(destPath + l, destSize - l, "/share");
		if (mkdir(destPath, 0755) != 0 && errnum != EEXIST)
			return 0;
	}
	return l;
#endif
	return 0;
}

void DetermineDataDirectories() {
#ifdef TODO
	if (!UserDataDirectory.IsEmpty())
		return;
	char xdg_path[256];
	if (BuildXDGPath(xdg_path, sizeof(xdg_path)) == 0)
		sprintf(xdg_path, "%s", "/tmp");
	UserDataDirectory.Format("%s/ags", xdg_path);
	mkdir(UserDataDirectory.GetCStr(), 0755);
	CommonDataDirectory.Format("%s/ags-common", xdg_path);
	mkdir(CommonDataDirectory.GetCStr(), 0755);
#endif
}

const char *AGSLinux::GetAllUsersDataDirectory() {
	DetermineDataDirectories();
	return CommonDataDirectory;
}

const char *AGSLinux::GetUserSavedgamesDirectory() {
	DetermineDataDirectories();
	return UserDataDirectory;
}

const char *AGSLinux::GetUserConfigDirectory() {
	return GetUserSavedgamesDirectory();
}

const char *AGSLinux::GetUserGlobalConfigDirectory() {
	return GetUserSavedgamesDirectory();
}

const char *AGSLinux::GetAppOutputDirectory() {
	DetermineDataDirectories();
	return UserDataDirectory;
}

unsigned long AGSLinux::GetDiskFreeSpaceMB() {
	// placeholder
	return 100;
}

const char *AGSLinux::GetNoMouseErrorString() {
	return "This game requires a mouse. You need to configure and setup your mouse to play this game.\n";
}

const char *AGSLinux::GetAllegroFailUserHint() {
	return "Make sure you have latest version of Allegro 4 libraries installed, and X server is running.";
}

eScriptSystemOSID AGSLinux::GetSystemOSID() {
	return eOS_Linux;
}

int AGSLinux::InitializeCDPlayer() {
	//return cd_player_init();
	return 0;
}

void AGSLinux::PostAllegroExit() {
	// do nothing
}

void AGSLinux::SetGameWindowIcon() {
	// do nothing
}

void AGSLinux::ShutdownCDPlayer() {
	//cd_exit();
}

AGSPlatformDriver *AGSPlatformDriver::GetDriver() {
	if (instance == nullptr)
		instance = new AGSLinux();
	return instance;
}

bool AGSLinux::LockMouseToWindow() {
#if 0
	return XGrabPointer(_xwin.display, _xwin.window, False,
	                    PointerMotionMask | ButtonPressMask | ButtonReleaseMask,
	                    GrabModeAsync, GrabModeAsync, _xwin.window, None, CurrentTime) == GrabSuccess;
#else
	return false;
#endif
}

void AGSLinux::UnlockMouse() {
#if 0
	XUngrabPointer(_xwin.display, CurrentTime);
#endif
}

void AGSLinux::GetSystemDisplayModes(std::vector<Engine::DisplayMode> &dms) {
	dms.clear();
	GFX_MODE_LIST *gmlist = get_gfx_mode_list(GFX_XWINDOWS_FULLSCREEN);
	for (int i = 0; i < gmlist->num_modes; ++i) {
		const GFX_MODE &m = gmlist->mode[i];
		dms.push_back(Engine::DisplayMode(Engine::GraphicResolution(m.width, m.height, m.bpp)));
	}
	destroy_gfx_mode_list(gmlist);
}

} // namespace AGS3

#endif
