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

#ifndef SAGA2_CONFIG_H
#define SAGA2_CONFIG_H

namespace Saga2 {

// under WIN95 paths can get BIG
#define PATH_STR_SIZE 260

/* ===================================================================== *
   FTA2 INI file settings
 * ===================================================================== */

struct configuration {
	uint8   musicVolume;
	uint8   soundVolume;
	uint8   voiceVolume;
	bool    autoAggression;
	bool    autoWeapon;
	bool    speechText;
	bool    showNight;
#ifdef _WIN32
	bool    windowed;
#endif
	char    soundResfilePath[PATH_STR_SIZE];
	char    voiceResfilePath[PATH_STR_SIZE];
	char    imageResfilePath[PATH_STR_SIZE];
	char    videoFilePath[PATH_STR_SIZE];
	char    mainResfilePath[PATH_STR_SIZE];
	char    dataResfilePath[PATH_STR_SIZE];
	char    scriptResfilePath[PATH_STR_SIZE];
	char    savedGamePath[PATH_STR_SIZE];
};

/* ===================================================================== *
   FTA2 INI file code
 * ===================================================================== */

void writeConfig(void);
void findProgramDir(char *argv);
void restoreProgramDir(void);

extern configuration globalConfig;

} // end of namespace Saga2

#endif  // CONFIG_H
