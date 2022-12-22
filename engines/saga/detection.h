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

#ifndef SAGA_DETECTION_H
#define SAGA_DETECTION_H

#include "engines/advancedDetector.h"

namespace Saga {

enum GameIds {
	GID_ITE = 0,
	GID_IHNM = 1
};

enum GameFeatures {
	GF_ITE_FLOPPY        = 1 << 0,
	GF_ITE_DOS_DEMO      = 1 << 1,
	GF_EXTRA_ITE_CREDITS = 1 << 2,
	GF_8BIT_UNSIGNED_PCM = 1 << 3,
	GF_IHNM_COLOR_FIX    = 1 << 4,
	GF_SOME_MAC_RESOURCES= 1 << 5,
	GF_AGA_GRAPHICS      = 1 << 6,
	GF_ECS_GRAPHICS      = 1 << 7,
	GF_INSTALLER         = 1 << 8,
	GF_EMBED_FONT        = 1 << 9,
	GF_POWERPACK_GFX     = 1 << 10,
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
	GAME_PATCHFILE        = 1 << 10    // IHNM patch file (patch.re_/patch.res)
};

// Make sure to update ResourceLists in saga.cpp if this enum is reordered.
enum GameResourceList : uint8 {
	RESOURCELIST_NONE = 0,
	RESOURCELIST_ITE,
	RESOURCELIST_ITE_ENGLISH_ECS,
	RESOURCELIST_ITE_GERMAN_AGA,
	RESOURCELIST_ITE_GERMAN_ECS,
	RESOURCELIST_ITE_DEMO,
	RESOURCELIST_IHNM,
	RESOURCELIST_IHNM_DEMO,
	RESOURCELIST_MAX
};

// Make sure to update FontLists in font.cpp if this enum is reordered.
enum GameFontList : uint8 {
	FONTLIST_NONE = 0,
	FONTLIST_ITE,
	FONTLIST_ITE_DEMO,
	FONTLIST_ITE_WIN_DEMO,
	FONTLIST_IHNM_DEMO,
	FONTLIST_IHNM_CD,
	FONTLIST_IHNM_ZH,
	FONTLIST_MAX
};

// Make sure to update PatchLists in resource.cpp if this enum is reordered.
enum GamePatchList : uint8 {
	PATCHLIST_NONE = 0,
	PATCHLIST_ITE,
	PATCHLIST_ITE_MAC,
	PATCHLIST_MAX
};

// Make sure to update ITE_IntroLists in introproc_ite.cpp if this enum is reordered.
enum GameIntroList : uint8 {
	INTROLIST_NONE = 0,
	INTROLIST_ITE_DEFAULT,
	INTROLIST_ITE_AMIGA_ENGLISH_ECS,
	INTROLIST_ITE_AMIGA_GERMAN_AGA,
	INTROLIST_ITE_AMIGA_GERMAN_ECS,
	INTROLIST_ITE_DOS_DEMO,
	INTROLIST_MAX
};

struct SAGAGameDescription {
	ADGameDescription desc;

	int gameId;
	uint32 features;
	int startSceneNumber;
	GameResourceList resourceList;
	GameFontList fontList;
	GamePatchList patchList;
	GameIntroList introList;
	// Only used if GF_INSTALLER is set
	ADGameFileDescription filesInArchive[5];
};

} // End of namespace Saga

#endif // SAGA_DETECTION_H
