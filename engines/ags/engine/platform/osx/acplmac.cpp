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

#include "core/platform.h"

#if AGS_PLATFORM_OS_MACOS

// ********* MacOS PLACEHOLDER DRIVER *********

//#include "util/wgt2allg.h"
//#include "gfx/ali3d.h"
//#include "ac/runtime_defines.h"
//#include "main/config.h"
//#include "plugin/agsplugin.h"
//#include <libcda.h>
//#include <pwd.h>
//#include <sys/stat.h>
#include "platform/base/agsplatformdriver.h"
#include "util/directory.h"
#include "ac/common.h"
#include "main/main.h"

void AGSMacInitPaths(char gamename[256], char appdata[PATH_MAX]);
void AGSMacGetBundleDir(char gamepath[PATH_MAX]);
//bool PlayMovie(char const *name, int skipType);

static char libraryApplicationSupport[PATH_MAX];
static char commonDataPath[PATH_MAX];

struct AGSMac : AGSPlatformDriver {
	AGSMac();

	virtual int  CDPlayerCommand(int cmdd, int datt) override;
	virtual void DisplayAlert(const char *, ...) override;
	virtual unsigned long GetDiskFreeSpaceMB() override;
	virtual const char *GetNoMouseErrorString() override;
	virtual eScriptSystemOSID GetSystemOSID() override;
	virtual int  InitializeCDPlayer() override;
	virtual void PostAllegroExit() override;
	virtual void SetGameWindowIcon() override;
	virtual void ShutdownCDPlayer() override;

	virtual const char *GetUserSavedgamesDirectory() override;
	virtual const char *GetAllUsersDataDirectory() override;
	virtual const char *GetUserConfigDirectory() override;
	virtual const char *GetAppOutputDirectory() override;
	virtual const char *GetIllegalFileChars() override;
};

AGSMac::AGSMac() {
	AGSMacInitPaths(psp_game_file_name, libraryApplicationSupport);

	snprintf(commonDataPath, PATH_MAX, "%s/uk.co.adventuregamestudio", libraryApplicationSupport);
	AGS::Common::Directory::CreateDirectory(commonDataPath);

	strcpy(psp_translation, "default");
}

int AGSMac::CDPlayerCommand(int cmdd, int datt) {
	return 0;//cd_player_control(cmdd, datt);
}

void AGSMac::DisplayAlert(const char *text, ...) {
	char displbuf[2000];
	va_list ap;
	va_start(ap, text);
	vsprintf(displbuf, text, ap);
	va_end(ap);
	if (_logToStdErr)
		fprintf(stderr, "%s\n", displbuf);
	else
		fprintf(stdout, "%s\n", displbuf);
}

unsigned long AGSMac::GetDiskFreeSpaceMB() {
	// placeholder
	return 100;
}

const char *AGSMac::GetNoMouseErrorString() {
	return "This game requires a mouse. You need to configure and setup your mouse to play this game.\n";
}

eScriptSystemOSID AGSMac::GetSystemOSID() {
	// override performed if `override.os` is set in config.
	return eOS_Mac;
}

int AGSMac::InitializeCDPlayer() {
	//return cd_player_init();
	return 0;
}

void AGSMac::PostAllegroExit() {
	// do nothing
}

void AGSMac::SetGameWindowIcon() {
	// do nothing
}

void AGSMac::ShutdownCDPlayer() {
	//cd_exit();
}

const char *AGSMac::GetAllUsersDataDirectory() {
	return commonDataPath;
}

const char *AGSMac::GetUserSavedgamesDirectory() {
	return libraryApplicationSupport;
}

const char *AGSMac::GetUserConfigDirectory() {
	return libraryApplicationSupport;
}

const char *AGSMac::GetAppOutputDirectory() {
	return commonDataPath;
}

const char *AGSMac::GetIllegalFileChars() {
	return "\\/:?\"<>|*"; // keep same as Windows so we can sync.
}

AGSPlatformDriver *AGSPlatformDriver::GetDriver() {
	if (instance == NULL)
		instance = new AGSMac();
	return instance;
}

#endif
