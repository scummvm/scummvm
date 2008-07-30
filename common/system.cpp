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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#include "backends/events/default/default-events.h"
#include "backends/fs/fs-factory.h"

#include "common/config-manager.h"
#include "common/system.h"
#include "common/timer.h"
#include "common/util.h"

#include "graphics/colormasks.h"
#include "gui/message.h"
#include "sound/mixer.h"

OSystem *g_system = 0;

OSystem::OSystem() {
}

OSystem::~OSystem() {
}

bool OSystem::setGraphicsMode(const char *name) {
	if (!name)
		return false;

	// Special case for the 'default' filter
	if (!scumm_stricmp(name, "normal") || !scumm_stricmp(name, "default")) {
		return setGraphicsMode(getDefaultGraphicsMode());
	}

	const GraphicsMode *gm = getSupportedGraphicsModes();

	while (gm->name) {
		if (!scumm_stricmp(gm->name, name)) {
			return setGraphicsMode(gm->id);
		}
		gm++;
	}

	return false;
}

OverlayColor OSystem::RGBToColor(uint8 r, uint8 g, uint8 b) {
	return ::RGBToColor<ColorMasks<565> >(r, g, b);
}

void OSystem::colorToRGB(OverlayColor color, uint8 &r, uint8 &g, uint8 &b) {
	::colorToRGB<ColorMasks<565> >(color, r, g, b);
}

OverlayColor OSystem::ARGBToColor(uint8 a, uint8 r, uint8 g, uint8 b) {
	return RGBToColor(r, g, b);
}

void OSystem::colorToARGB(OverlayColor color, uint8 &a, uint8 &r, uint8 &g, uint8 &b) {
	colorToRGB(color, r, g, b);
	a = 255;
}

void OSystem::displayMessageOnOSD(const char *msg) {
	// Display the message for 1.5 seconds
	GUI::TimedMessageDialog dialog(msg, 1500);
	dialog.runModal();
}


bool OSystem::openCD(int drive) {
	return false;
}

bool OSystem::pollCD() {
	return false;
}

void OSystem::playCD(int track, int num_loops, int start_frame, int duration) {
}

void OSystem::stopCD() {
}

void OSystem::updateCD() {
}

static Common::EventManager *s_eventManager = 0;

Common::EventManager *OSystem::getEventManager() {
	// FIXME/TODO: Eventually this method should be turned into an abstract one,
	// to force backends to implement this conciously (even if they
	// end up returning the default event manager anyway).
	if (!s_eventManager)
		s_eventManager = new DefaultEventManager(this);
	return s_eventManager;
}

void OSystem::clearScreen() {
	Graphics::Surface *screen = lockScreen();
	memset(screen->pixels, 0, screen->h * screen->pitch);
	unlockScreen();
}


/*
FIXME: The config file loading code below needs to be cleaned up.
 Port specific variants should be pushed into the respective ports.

 Ideally, the default OSystem::openConfigFileForReading/Writing methods
 should be removed completely. 
*/

#include "common/file.h"

#ifdef __PLAYSTATION2__
#include "backends/platform/ps2/systemps2.h"
#endif

#ifdef IPHONE
#include "backends/platform/iphone/osys_iphone.h"
#endif


#if defined(UNIX)
#ifdef MACOSX
#define DEFAULT_CONFIG_FILE "Library/Preferences/ScummVM Preferences"
#else
#define DEFAULT_CONFIG_FILE ".scummvmrc"
#endif
#else
#define DEFAULT_CONFIG_FILE "scummvm.ini"
#endif

static Common::String getDefaultConfigFileName() {
	char configFile[MAXPATHLEN];

#if defined (WIN32) && !defined(_WIN32_WCE) && !defined(__SYMBIAN32__)
	OSVERSIONINFO win32OsVersion;
	ZeroMemory(&win32OsVersion, sizeof(OSVERSIONINFO));
	win32OsVersion.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	GetVersionEx(&win32OsVersion);
	// Check for non-9X version of Windows.
	if (win32OsVersion.dwPlatformId != VER_PLATFORM_WIN32_WINDOWS) {
		// Use the Application Data directory of the user profile.
		if (win32OsVersion.dwMajorVersion >= 5) {
			if (!GetEnvironmentVariable("APPDATA", configFile, sizeof(configFile)))
				error("Unable to access application data directory");
		} else {
			if (!GetEnvironmentVariable("USERPROFILE", configFile, sizeof(configFile)))
				error("Unable to access user profile directory");

			strcat(configFile, "\\Application Data");
			CreateDirectory(configFile, NULL);
		}

		strcat(configFile, "\\ScummVM");
		CreateDirectory(configFile, NULL);
		strcat(configFile, "\\" DEFAULT_CONFIG_FILE);

		if (fopen(configFile, "r") == NULL) {
			// Check windows directory
			char oldConfigFile[MAXPATHLEN];
			GetWindowsDirectory(oldConfigFile, MAXPATHLEN);
			strcat(oldConfigFile, "\\" DEFAULT_CONFIG_FILE);
			if (fopen(oldConfigFile, "r")) {
				printf("The default location of the config file (scummvm.ini) in ScummVM has changed,\n");
				printf("under Windows NT4/2000/XP/Vista. You may want to consider moving your config\n");
				printf("file from the old default location:\n");
				printf("%s\n", oldConfigFile);
				printf("to the new default location:\n");
				printf("%s\n\n", configFile);
				strcpy(configFile, oldConfigFile);
			}
		}
	} else {
		// Check windows directory
		GetWindowsDirectory(configFile, MAXPATHLEN);
		strcat(configFile, "\\" DEFAULT_CONFIG_FILE);
	}

#elif defined(PALMOS_MODE)
	strcpy(configFile,"/PALM/Programs/ScummVM/" DEFAULT_CONFIG_FILE);
#elif defined(IPHONE)
	strcpy(configFile, OSystem_IPHONE::getConfigPath());	
#elif defined(__PLAYSTATION2__)
	((OSystem_PS2*)g_system)->makeConfigPath(configFile);
#elif defined(__PSP__)
	strcpy(configFile, "ms0:/" DEFAULT_CONFIG_FILE);
#elif defined (__SYMBIAN32__)
	strcpy(configFile, Symbian::GetExecutablePath());
	strcat(configFile, DEFAULT_CONFIG_FILE);
#elif defined(UNIX) && !defined(GP2X) && !defined(IPHONE)
	// On UNIX type systems, by default we store the config file inside
	// to the HOME directory of the user.
	//	
	// GP2X is Linux based but Home dir can be read only so do not use
	// it and put the config in the executable dir.
	//
	// On the iPhone, the home dir of the user when you launch the app
	// from the Springboard, is /. Which we don't want.
	const char *home = getenv("HOME");
	if (home != NULL && strlen(home) < MAXPATHLEN)
		snprintf(configFile, MAXPATHLEN, "%s/%s", home, DEFAULT_CONFIG_FILE);
	else
		strcpy(configFile, DEFAULT_CONFIG_FILE);

#else
	strcpy(configFile, DEFAULT_CONFIG_FILE);
#endif


	return configFile;
}

Common::SeekableReadStream *OSystem::openConfigFileForReading() {
	Common::File *confFile = new Common::File();
	assert(confFile);
	confFile->open(getDefaultConfigFileName());
	return confFile;
}

Common::WriteStream *OSystem::openConfigFileForWriting() {
#ifdef __DC__
	return 0;
#else
	Common::DumpFile *confFile = new Common::DumpFile();
	assert(confFile);
	confFile->open(getDefaultConfigFileName());
	return confFile;
#endif
}


/*
FIXME: The config file loading code below needs to be cleaned up.
 Port specific variants should be pushed into the respective ports.

 Ideally, the default OSystem::openConfigFileForReading/Writing methods
 should be removed completely. 
*/

#include "common/file.h"

#ifdef __PLAYSTATION2__
#include "backends/platform/ps2/systemps2.h"
#endif

#ifdef IPHONE
#include "backends/platform/iphone/osys_iphone.h"
#endif


#if defined(UNIX)
#ifdef MACOSX
#define DEFAULT_CONFIG_FILE "Library/Preferences/ScummVM Preferences"
#else
#define DEFAULT_CONFIG_FILE ".scummvmrc"
#endif
#else
#define DEFAULT_CONFIG_FILE "scummvm.ini"
#endif

static Common::String getDefaultConfigFileName() {
	char configFile[MAXPATHLEN];
#if defined(UNIX) && !defined(GP2X) && !defined(IPHONE)
	// On UNIX type systems, by default we store the config file inside
	// to the HOME directory of the user.
	//	
	// GP2X is Linux based but Home dir can be read only so do not use
	// it and put the config in the executable dir.
	//
	// On the iPhone, the home dir of the user when you launch the app
	// from the Springboard, is /. Which we don't want.
	const char *home = getenv("HOME");
	if (home != NULL && strlen(home) < MAXPATHLEN)
		snprintf(configFile, MAXPATHLEN, "%s/%s", home, DEFAULT_CONFIG_FILE);
	else
		strcpy(configFile, DEFAULT_CONFIG_FILE);
#else
	#if defined (WIN32) && !defined(_WIN32_WCE) && !defined(__SYMBIAN32__)
		OSVERSIONINFO win32OsVersion;
		ZeroMemory(&win32OsVersion, sizeof(OSVERSIONINFO));
		win32OsVersion.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
		GetVersionEx(&win32OsVersion);
		// Check for non-9X version of Windows.
		if (win32OsVersion.dwPlatformId != VER_PLATFORM_WIN32_WINDOWS) {
			// Use the Application Data directory of the user profile.
			if (win32OsVersion.dwMajorVersion >= 5) {
				if (!GetEnvironmentVariable("APPDATA", configFile, sizeof(configFile)))
					error("Unable to access application data directory");
			} else {
				if (!GetEnvironmentVariable("USERPROFILE", configFile, sizeof(configFile)))
					error("Unable to access user profile directory");

				strcat(configFile, "\\Application Data");
				CreateDirectory(configFile, NULL);
			}

			strcat(configFile, "\\ScummVM");
			CreateDirectory(configFile, NULL);
			strcat(configFile, "\\" DEFAULT_CONFIG_FILE);

			if (fopen(configFile, "r") == NULL) {
				// Check windows directory
				char oldConfigFile[MAXPATHLEN];
				GetWindowsDirectory(oldConfigFile, MAXPATHLEN);
				strcat(oldConfigFile, "\\" DEFAULT_CONFIG_FILE);
				if (fopen(oldConfigFile, "r")) {
					printf("The default location of the config file (scummvm.ini) in ScummVM has changed,\n");
					printf("under Windows NT4/2000/XP/Vista. You may want to consider moving your config\n");
					printf("file from the old default location:\n");
					printf("%s\n", oldConfigFile);
					printf("to the new default location:\n");
					printf("%s\n\n", configFile);
					strcpy(configFile, oldConfigFile);
				}
			}
		} else {
			// Check windows directory
			GetWindowsDirectory(configFile, MAXPATHLEN);
			strcat(configFile, "\\" DEFAULT_CONFIG_FILE);
		}

	#elif defined(PALMOS_MODE)
		strcpy(configFile,"/PALM/Programs/ScummVM/" DEFAULT_CONFIG_FILE);
	#elif defined(IPHONE)
		strcpy(configFile, OSystem_IPHONE::getConfigPath());	
	#elif defined(__PLAYSTATION2__)
		((OSystem_PS2*)g_system)->makeConfigPath(configFile);
	#elif defined(__PSP__)
		strcpy(configFile, "ms0:/" DEFAULT_CONFIG_FILE);
	#elif defined (__SYMBIAN32__)
		strcpy(configFile, Symbian::GetExecutablePath());
		strcat(configFile, DEFAULT_CONFIG_FILE);
	#else
		strcpy(configFile, DEFAULT_CONFIG_FILE);
	#endif
#endif

	return configFile;
}

Common::SeekableReadStream *OSystem::openConfigFileForReading() {
	Common::File *confFile = new Common::File();
	assert(confFile);
	confFile->open(getDefaultConfigFileName());
	return confFile;
}

Common::WriteStream *OSystem::openConfigFileForWriting() {
#ifdef __DC__
	return 0;
#else
	Common::DumpFile *confFile = new Common::DumpFile();
	assert(confFile);
	confFile->open(getDefaultConfigFileName());
	return confFile;
#endif
}
