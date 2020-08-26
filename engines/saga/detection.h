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

#ifndef SAGA_DETECTION_H
#define SAGA_DETECTION_H

#include "engines/advancedDetector.h"

namespace Saga {

struct GameResourceDescription {
	uint32 sceneLUTResourceId;
	uint32 moduleLUTResourceId;
	uint32 mainPanelResourceId;
	uint32 conversePanelResourceId;
	uint32 optionPanelResourceId;
	uint32 mainSpritesResourceId;
	uint32 mainPanelSpritesResourceId;
	uint32 mainStringsResourceId;
	// ITE specific resources
	uint32 actorsStringsResourceId;
	uint32 defaultPortraitsResourceId;
	// IHNM specific resources
	uint32 optionPanelSpritesResourceId;
	uint32 warningPanelResourceId;
	uint32 warningPanelSpritesResourceId;
	uint32 psychicProfileResourceId;
};

struct GameFontDescription {
	uint32 fontResourceId;
};

struct GamePatchDescription {
	const char *fileName;
	uint16 fileType;
	uint32 resourceId;
};

enum GameIds {
	GID_ITE = 0,
	GID_IHNM = 1,
	GID_DINO = 2,
	GID_FTA2 = 3
};

enum GameFeatures {
	GF_ITE_FLOPPY        = 1 << 0,
	GF_ITE_DOS_DEMO      = 1 << 1,
	GF_EXTRA_ITE_CREDITS = 1 << 2,
	GF_8BIT_UNSIGNED_PCM = 1 << 3,
	GF_IHNM_COLOR_FIX    = 1 << 4,
	GF_SOME_MAC_RESOURCES= 1 << 5
};

enum GameFileTypes {
	// Common
	GAME_RESOURCEFILE     = 1 << 0,    // Game resources
	GAME_SCRIPTFILE       = 1 << 1,    // Game scripts
	GAME_SOUNDFILE        = 1 << 2,    // SFX (also contains voices and MIDI music in SAGA 2 games)
	GAME_VOICEFILE        = 1 << 3,    // Voices (also contains SFX in the ITE floppy version)
	// ITE specific
	GAME_DIGITALMUSICFILE = 1 << 4,    // ITE digital music, added by Wyrmkeep
	GAME_MACBINARY        = 1 << 5,    // ITE Mac CD Guild
	GAME_DEMOFILE         = 1 << 6,    // Early ITE demo
	GAME_SWAPENDIAN       = 1 << 7,    // Used to identify the BE voice file in the ITE combined version
	// IHNM specific
	GAME_MUSICFILE_FM     = 1 << 8,    // IHNM
	GAME_MUSICFILE_GM     = 1 << 9,    // IHNM, ITE Mac CD Guild
	GAME_PATCHFILE        = 1 << 10,   // IHNM patch file (patch.re_/patch.res)
	// SAGA 2 (Dinotopia, FTA2)
	GAME_IMAGEFILE        = 1 << 11,   // Game images
	GAME_OBJRESOURCEFILE  = 1 << 12    // Game object data
};

struct SAGAGameDescription {
	ADGameDescription desc;

	int gameId;
	uint32 features;
	int startSceneNumber;
	const GameResourceDescription *resourceDescription;
	int fontsCount;
	const GameFontDescription *fontDescriptions;
	const GamePatchDescription *patchDescriptions;
};

} // End of namespace Saga

#endif // SAGA_DETECTION_H
