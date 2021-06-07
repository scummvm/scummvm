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
 *
 * Based on the original sources
 *   Faery Tale II -- The Halls of the Dead
 *   (c) 1993-1996 The Wyrmkeep Entertainment Co.
 */

#include "saga2/std.h"
#include "saga2/config.h"

namespace Saga2 {

char iniFile[] = "FTA2.INI";
configuration globalConfig;

static char   originalDir[PATH_STR_SIZE];

//-----------------------------------------------------------------------
//	This doesn't belong here, oh well...

inline bool file_exists(const char f[]) {
	warning("STUB: file_exists()");
	return false;
	//return (!access(f, F_OK));
}
inline void deletefile(char n[]) {
	if (file_exists(n)) remove(n);
}

void KillLogs(void) {
	deletefile("AUDIOERR.LOG");
	deletefile("SYSERRS.LOG");
	deletefile("ERRORS.LOG");
	deletefile("MEMDUMP.LOG");
	deletefile("WINERROR.LOG");
	deletefile("MEMORY.LOG");
}

//-----------------------------------------------------------------------
//	Sets default values in config file

static void defaultConfig(configuration &config) {
	config.musicVolume = 127;
	config.soundVolume = 127;
	config.voiceVolume = 127;
	config.autoAggression = TRUE;
	config.autoWeapon = TRUE;
	config.speechText = TRUE;
	config.showNight = TRUE;
#ifdef _WIN32
	config.windowed = FALSE;
#endif
	strcpy(config.soundResfilePath, ".\\");
	strcpy(config.voiceResfilePath, ".\\");
	strcpy(config.imageResfilePath, ".\\");
	strcpy(config.videoFilePath, ".\\");
	strcpy(config.mainResfilePath, ".\\");
	strcpy(config.dataResfilePath, ".\\");
	strcpy(config.scriptResfilePath, ".\\");
	strcpy(config.savedGamePath, ".\\");
}

//-----------------------------------------------------------------------
//	saves settings to INI file

void writeConfig(void) {
#if 0
	WritePrivateProfileInt("Sound", "MusicVolume", globalConfig.musicVolume, iniFile);
	WritePrivateProfileInt("Sound", "SoundVolume", globalConfig.soundVolume, iniFile);
	WritePrivateProfileInt("Sound", "VoiceVolume", globalConfig.voiceVolume, iniFile);
	WritePrivateProfileInt("Options", "AutoAggression", globalConfig.autoAggression, iniFile);
	WritePrivateProfileInt("Options", "AutoWeapon", globalConfig.autoWeapon, iniFile);
	WritePrivateProfileInt("Options", "SpeechText", globalConfig.speechText, iniFile);
	WritePrivateProfileInt("Options", "Night", globalConfig.showNight, iniFile);
#endif
	warning("STUB: writeConfig()");
}

/* ===================================================================== *
   Functions to save the original program directory, and to return to
   that directory whenever needed.
 * ===================================================================== */

//-----------------------------------------------------------------------
//	Determine's the programs home directory


void findProgramDir(char *argv) {
	char        *sep;
	sprintf(originalDir, "%s", argv);
	if ((sep = strrchr(originalDir, '\\')) != NULL) * sep = '\0';
}

//-----------------------------------------------------------------------
//	Does a CD back to the program's directory

void restoreProgramDir(void) {
	warning("STUB: restoreProgramDir()");
	//chdir(originalDir);
}

} // end of namespace Saga2
