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

#ifndef ADL_DETECTION_H
#define ADL_DETECTION_H

#include "engines/advancedDetector.h"

namespace Adl {

#define SAVEGAME_VERSION 0
#define SAVEGAME_NAME_LEN 32

enum GameType {
	GAME_TYPE_NONE,
	GAME_TYPE_HIRES0,
	GAME_TYPE_HIRES1,
	GAME_TYPE_HIRES2,
	GAME_TYPE_HIRES3,
	GAME_TYPE_HIRES4,
	GAME_TYPE_HIRES5,
	GAME_TYPE_HIRES6
};

/*
 * ====== Mystery House supported versions ======
 * GAME_VER_HR1_SIMI:
 * - Instructions always shown (no prompt)
 * - Instructions contain Simi Valley address
 * - On-Line Systems title screen in main executable only and waits for key
 * GAME_VER_HR1_COARSE:
 * - Longer instructions, now containing Coarsegold address
 * - On-Line Systems title screen with instructions prompt
 * GAME_VER_HR1_PD:
 * - Public Domain disclaimer on startup
 * - Sierra On-Line title screen with instructions prompt
 *
 * Note: there are probably at least two or three more variants
 */

enum GameVersion {
	GAME_VER_NONE = 0,
	GAME_VER_HR1_SIMI = 0,
	GAME_VER_HR1_COARSE,
	GAME_VER_HR1_VF1,
	GAME_VER_HR1_VF2,
	GAME_VER_HR1_PD,
	GAME_VER_HR4_V1_0,
	GAME_VER_HR4_V1_1,
	GAME_VER_HR4_LNG
};

struct AdlGameDescription {
	AD_GAME_DESCRIPTION_HELPERS(desc);

	ADGameDescription desc;
	GameType gameType;
	GameVersion version;
};

// Mystery House was designed for monochrome display, so we default to
// monochrome mode there. All the other games default to color mode.
#define GAMEOPTION_COLOR_DEFAULT_OFF GUIO_GAMEOPTIONS1
#define GAMEOPTION_SCANLINES         GUIO_GAMEOPTIONS2
#define GAMEOPTION_COLOR_DEFAULT_ON  GUIO_GAMEOPTIONS3
#define GAMEOPTION_NTSC              GUIO_GAMEOPTIONS4
#define GAMEOPTION_MONO_TEXT         GUIO_GAMEOPTIONS5
#define GAMEOPTION_APPLE2E_CURSOR    GUIO_GAMEOPTIONS6

} // End of namespace Adl

#endif // ADL_DETECTION_H
