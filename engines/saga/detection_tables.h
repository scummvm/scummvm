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

// Game detection information and MD5s

#include "common/translation.h"

// From sage/scene.h, these are some defines that also
// help with detection.
#include "saga/shared_detection_defines.h"

namespace Saga {

static const SAGAGameDescription gameDescriptions[] = {
	// ITE Section ////////////////////////////////////////////////////////////////////////////////////////////


	// ITE Demos //////////////////////////////////////////////////////////////////////////////////////////////

	// Note: This version is NOT supported yet
	// Based on a very early version of the engine

	// Inherit the earth - DOS Demo version
	// sound unchecked
	{
		{
			"ite",
			_s("Missing game code"), // Reason for being unsupported
			{
				{"ite.rsc",		GAME_RESOURCEFILE,					"986c79c4d2939dbe555576529fd37932", -1},
				//{"ite.dmo",	GAME_DEMOFILE,						"0b9a70eb4e120b6f00579b46c8cae29e", -1},
				{"scripts.rsc", GAME_SCRIPTFILE,					"d5697dd3240a3ceaddaa986c47e1a2d7", -1},
				//{"voices.rsc",	GAME_SOUNDFILE | GAME_VOICEFILE,	"c58e67c506af4ffa03fd0aac2079deb0", -1},
				AD_LISTEND
			},
			Common::EN_ANY,
			Common::kPlatformDOS,
			ADGF_DEMO | ADGF_UNSUPPORTED,
			GUIO1(GUIO_NOSPEECH)
		},
		GID_ITE,
		GF_ITE_DOS_DEMO,
		ITE_DEFAULT_SCENE,
		RESOURCELIST_ITE_DEMO,
		FONTLIST_ITE_DEMO,
		PATCHLIST_NONE,
		INTROLIST_ITE_DOS_DEMO,
		{},
	},

	// Inherit the earth - MAC Demo version
	{
		{
			"ite",
			"Demo 2",
			{
				{"ited.rsc",		GAME_RESOURCEFILE,	"addfc9d82bc2fa1f4cab23743c652c08", 1865461},
				{"scriptsd.rsc",	GAME_SCRIPTFILE,	"fded5c59b8b7c5976229f960d21e6b0b", 70083},
				AD_LISTEND
			},
			Common::EN_ANY,
			Common::kPlatformMacintosh,
			ADGF_DEMO,
			GUIO1(GUIO_NOSPEECH)
		},
		GID_ITE,
		0,
		ITE_DEFAULT_SCENE,
		RESOURCELIST_ITE,
		FONTLIST_ITE_WIN_DEMO,
		PATCHLIST_ITE_MAC,
		INTROLIST_ITE_DEFAULT,
		{},
	},


	// Inherit the earth - MAC Demo version 1
	// Non-interactive demo
	{
		{
			"ite",
			"Demo 1",
			{
				{"ited.rsc",		GAME_RESOURCEFILE,	"addfc9d82bc2fa1f4cab23743c652c08", 1131098},
				{"scriptsd.rsc",	GAME_SCRIPTFILE,	"fded5c59b8b7c5976229f960d21e6b0b", 38613},
				AD_LISTEND
			},
			Common::EN_ANY,
			Common::kPlatformMacintosh,
			ADGF_DEMO,
			GUIO1(GUIO_NOSPEECH)
		},
		GID_ITE,
		0,
		ITE_DEFAULT_SCENE,
		RESOURCELIST_ITE,
		FONTLIST_ITE_WIN_DEMO,
		PATCHLIST_ITE_MAC,
		INTROLIST_ITE_DEFAULT,
		{},
	},


	// Inherit the earth - Win32 Demo version 2/3, Linux Demo version
	// Win32 Version 3 and Linux Demo version have digital music, Win32 version 2 has MIDI music
	{
		{
			"ite",
			"Win Demo 2/3, Linux Demo",
			{
				{"ited.rsc",		GAME_RESOURCEFILE,	"3a450852cbf3c80773984d565647e6ac", 1951395},
				{"scriptsd.rsc",	GAME_SCRIPTFILE,	"3f12b67fa93e56e1a6be39d2921d80bb", 70051},
				AD_LISTEND
			},
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_DEMO,
			GUIO0()
		},
		GID_ITE,
		0,
		ITE_DEFAULT_SCENE,
		RESOURCELIST_ITE,
		FONTLIST_ITE_WIN_DEMO,
		PATCHLIST_ITE,
		INTROLIST_ITE_DEFAULT,
		{},
	},


	// Inherit the earth - Win32 Demo version 1
	// Non-interactive demo
	{
		{
			"ite",
			"Demo 1",
			{
				{"ited.rsc",		GAME_RESOURCEFILE,	"3a450852cbf3c80773984d565647e6ac", 1327323},
				{"scriptsd.rsc",	GAME_SCRIPTFILE,	"3f12b67fa93e56e1a6be39d2921d80bb", 38613},
				AD_LISTEND
			},
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_DEMO,
			GUIO0()
		},
		GID_ITE,
		GF_8BIT_UNSIGNED_PCM,
		ITE_DEFAULT_SCENE,
		RESOURCELIST_ITE,
		FONTLIST_ITE_WIN_DEMO,
		PATCHLIST_ITE,
		INTROLIST_ITE_DEFAULT,
		{},
	},


	{
		{
			"ite",
			"AGA Demo CD",
			{
				{"ite.rtn",		GAME_RESOURCEFILE,	"634d36f78ac151b14dbeed274e169def", 18564},
				{"ite.000",                             0,      "75a2c63fd67d3c87512a37af91537fba", 900096},
				AD_LISTEND
			},
			Common::DE_DEU,
			Common::kPlatformAmiga,
			ADGF_CD,
			GUIO0()
		},
		GID_ITE,
		GF_EXTRA_ITE_CREDITS | GF_AGA_GRAPHICS | GF_EMBED_FONT | GF_POWERPACK_GFX,
		ITE_DEFAULT_SCENE,
		RESOURCELIST_ITE_GERMAN_AGA,
		FONTLIST_NONE,
		PATCHLIST_ITE,
		INTROLIST_ITE_AMIGA_GERMAN_AGA,
		{},
	},
	{
		{
			"ite",
			"AGA Demo Floppy",
			{
				{"ite.rtn",		GAME_RESOURCEFILE,	"634d36f78ac151b14dbeed274e169def", 18564},
				{"ite01.adf", 0, "82877f62357c39cb3a9f892dfce713e2", 901120},
				{"ite02.adf", 0, "ed7bff9c17aacce17a9b5743d39149e7", 901120},
				AD_LISTEND
			},
			Common::DE_DEU,
			Common::kPlatformAmiga,
			ADGF_CD,
			GUIO0()
		},
		GID_ITE,
		GF_ITE_FLOPPY|GF_EXTRA_ITE_CREDITS | GF_AGA_GRAPHICS | GF_EMBED_FONT | GF_POWERPACK_GFX,
		ITE_DEFAULT_SCENE,
		RESOURCELIST_ITE_GERMAN_AGA,
		FONTLIST_NONE,
		PATCHLIST_ITE,
		INTROLIST_ITE_AMIGA_GERMAN_AGA,
		{},
	},
	{
		{
			"ite",
			"ECS Demo CD",
			{
				{"ite.rtn",		GAME_RESOURCEFILE,	"1e77154f045358ef3f09fbdb00ea92a4", 18624},
				{"ite.000",                             0,      "7907e74ed9ce17bb9d6c10e21273d53e", 788221},
				AD_LISTEND
			},
			Common::DE_DEU,
			Common::kPlatformAmiga,
			ADGF_CD,
			GUIO0()
		},
		GID_ITE,
		GF_EXTRA_ITE_CREDITS | GF_ECS_GRAPHICS | GF_EMBED_FONT | GF_POWERPACK_GFX,
		ITE_DEFAULT_SCENE,
		RESOURCELIST_ITE_GERMAN_ECS,
		FONTLIST_NONE,
		PATCHLIST_ITE,
		INTROLIST_ITE_AMIGA_GERMAN_ECS,
		{},
	},
	{
		{
			"ite",
			"ECS Demo Floppy",
			{
				{"ite.rtn",		GAME_RESOURCEFILE,	"1e77154f045358ef3f09fbdb00ea92a4", 18624},
				{"ite01.adf",                           0,      "fc6ba8bae7d86038efa2fba30ebf5e77", 901120},
				{"ite02.adf",                           0,      "8f8cc806fc19066480b868c43e12343a", 901120},
				AD_LISTEND
			},
			Common::DE_DEU,
			Common::kPlatformAmiga,
			ADGF_NO_FLAGS,
			GUIO0()
		},
		GID_ITE,
		GF_ITE_FLOPPY | GF_EXTRA_ITE_CREDITS | GF_ECS_GRAPHICS | GF_EMBED_FONT | GF_POWERPACK_GFX,
		ITE_DEFAULT_SCENE,
		RESOURCELIST_ITE_GERMAN_ECS,
		FONTLIST_NONE,
		PATCHLIST_ITE,
		INTROLIST_ITE_AMIGA_GERMAN_ECS,
		{},
	},


	// ITE Mac versions ///////////////////////////////////////////////////////////////////////////////////////

	// Inherit the earth - MAC CD Guild version
	{
		{
			"ite",
			"CD",
			{
				{"ite resources",	GAME_RESOURCEFILE | GAME_MACBINARY,	"ee65f8e713127cf7f2d56371d2b8e63d", 2264},
				{"ite scripts",		GAME_SCRIPTFILE | GAME_MACBINARY,	"ee65f8e713127cf7f2d56371d2b8e63d", 2264},
				{"ite sounds",		GAME_SOUNDFILE | GAME_MACBINARY,	"ee65f8e713127cf7f2d56371d2b8e63d", 2264},
				{"ite music",		GAME_MUSICFILE_GM | GAME_MACBINARY,	"b0d66d7ae48f35c5c9a3444343b86f85", 3135053},
				// {"ite voices",		GAME_VOICEFILE | GAME_MACBINARY,	"ee65f8e713127cf7f2d56371d2b8e63d", 2264},
				AD_LISTEND
			},
			Common::EN_ANY,
			Common::kPlatformMacintosh,
			ADGF_CD | ADGF_MACRESFORK,
			GUIO0()
		},
		GID_ITE,
		GF_8BIT_UNSIGNED_PCM,
		ITE_DEFAULT_SCENE,
		RESOURCELIST_ITE,
		FONTLIST_ITE_WIN_DEMO,
		PATCHLIST_NONE,
		INTROLIST_ITE_DEFAULT,
		{},
	},

	// Inherit the earth - MAC CD First edition
	{
		{
			"ite",
			"CD",
			{
				{"ite resources.bin",	GAME_RESOURCEFILE | GAME_MACBINARY,	"473768a17d843de5126d608b26ed7250", -1},
				{"ite scripts.bin",		GAME_SCRIPTFILE | GAME_MACBINARY,	"294cad2d7bb6cd7dd602c9a5867873d7", -1},
				{"ite sounds.bin",		GAME_SOUNDFILE | GAME_MACBINARY,	"178fa322aeb8eb51bba821eb128e037b", -1},
				{"ite music.bin",		GAME_MUSICFILE_GM | GAME_MACBINARY,	"023fddf96a39edeaed647f16947de9c1", -1},
				//{"ite voices.bin",		GAME_VOICEFILE | GAME_MACBINARY,	"dba92ae7d57e942250fe135609708369", -1},
				AD_LISTEND
			},
			Common::EN_ANY,
			Common::kPlatformMacintosh,
			ADGF_CD,
			GUIO0()
		},
		GID_ITE,
		GF_8BIT_UNSIGNED_PCM,
		ITE_DEFAULT_SCENE,
		RESOURCELIST_ITE,
		FONTLIST_ITE_WIN_DEMO,
		PATCHLIST_NONE,
		INTROLIST_ITE_DEFAULT,
		{},
	},

	// Inherit the earth - MAC CD Wyrmkeep version
	{
		{
			"ite",
			"Wyrmkeep CD",
			{
				{"ite.rsc",						GAME_RESOURCEFILE,	"4f7fa11c5175980ed593392838523060", -1},
				{"scripts.rsc",					GAME_SCRIPTFILE,	"adf1f46c1d0589083996a7060c798ad0", -1},
				AD_LISTEND
			},
			Common::EN_ANY,
			Common::kPlatformMacintosh,
			ADGF_CD,
			GUIO0()
		},
		GID_ITE,
		0,
		ITE_DEFAULT_SCENE,
		RESOURCELIST_ITE,
		FONTLIST_ITE_WIN_DEMO,
		PATCHLIST_ITE_MAC,
		INTROLIST_ITE_DEFAULT,
		{},
	},

	// Inherit the earth - MAC CD GOG version 1.1
	{
		{
			"ite",
			"GOG CD Mac v1.1",
			{
				{"ite_i.rsc",					GAME_RESOURCEFILE,	"a6433e34b97b15e64fe8214651012db9", 8927165},
				{"scripts_i.rsc",				GAME_SCRIPTFILE,	"a891405405edefc69c9d6c420c868b84", 335927},
				AD_LISTEND
			},
			Common::EN_ANY,
			Common::kPlatformUnknown,	// Most of the resources are Little Endian
			ADGF_CD,
			GUIO0()
		},
		GID_ITE,
		GF_SOME_MAC_RESOURCES,
		ITE_DEFAULT_SCENE,
		RESOURCELIST_ITE,
		FONTLIST_ITE,
		PATCHLIST_NONE,
		INTROLIST_ITE_DEFAULT,
		{},
	},


	// ITE PC CD versions //////////////////////////////////////////////////////////////////////////////////////

	// Inherit the earth - Wyrmkeep combined Windows/Mac/Linux CD

	// version is different from the other Wyrmkeep re-releases in that it does
	// not have any substitute files. Presumably the ite.rsc file has been
	// modified to include the Wyrmkeep changes. The resource files are little-
	// endian, except for the voice file which is big-endian.
	{
		{
			"ite",
			"Multi-OS CD Version",
			{
				{"ite.rsc",						GAME_RESOURCEFILE,					"a6433e34b97b15e64fe8214651012db9", 8927165},
				{"scripts.rsc",					GAME_SCRIPTFILE,					"a891405405edefc69c9d6c420c868b84", -1},
				AD_LISTEND
			},
			Common::EN_ANY,
			Common::kPlatformUnknown,
			ADGF_CD,
			GUIO0()
		},
		GID_ITE,
		0,
		ITE_DEFAULT_SCENE,
		RESOURCELIST_ITE,
		FONTLIST_ITE,
		PATCHLIST_NONE,
		INTROLIST_ITE_DEFAULT,
		{},
	},

	// Inherit the earth - German Wyrmkeep combined Windows/Mac/Linux CD

	// Supplied by user nicode in bug #6428.
	// Contains voices.rsc instead of "Inherit the Earth Voices".
	{
		{
			"ite",
			"Multi-OS CD Version",
			{
				{"ite.rsc",						GAME_RESOURCEFILE,					"420e09cfdbb4db12baefd4bc81d8e154", 8925349},
				{"scripts.rsc",					GAME_SCRIPTFILE,					"a891405405edefc69c9d6c420c868b84", -1},
				AD_LISTEND
			},
			Common::DE_DEU,
			Common::kPlatformUnknown,
			ADGF_CD,
			GUIO0()
		},
		GID_ITE,
		0,
		ITE_DEFAULT_SCENE,
		RESOURCELIST_ITE,
		FONTLIST_ITE,
		PATCHLIST_NONE,
		INTROLIST_ITE_DEFAULT,
		{},
	},

	// Inherit the earth - Italian Wyrmkeep combined Windows/Mac/Linux CD (fan translation)

	// version is different from the other Wyrmkeep re-releases in that it does
	// not have any substitute files. Presumably the ite.rsc file has been
	// modified to include the Wyrmkeep changes. The resource files are little-
	// endian, except for the voice file which is big-endian.
	{
		{
			"ite",
			"Multi-OS CD Version",
			{
				// TODO: add size for ite.rsc
				{"ite.rsc",						GAME_RESOURCEFILE,					"a6433e34b97b15e64fe8214651012db9", -1},
				{"scripts.rsc",					GAME_SCRIPTFILE,					"a891405405edefc69c9d6c420c868b84", -1},
				{"voices.rsc",					GAME_VOICEFILE,						"41bb6b95d792dde5196bdb78740895a6", -1},
				AD_LISTEND
			},
			Common::IT_ITA,
			Common::kPlatformUnknown,
			ADGF_CD,
			GUIO0()
		},
		GID_ITE,
		0,
		ITE_DEFAULT_SCENE,
		RESOURCELIST_ITE,
		FONTLIST_ITE,
		PATCHLIST_NONE,
		INTROLIST_ITE_DEFAULT,
		{},
	},

	// Inherit the earth - Wyrmkeep Windows CD version
	// Reported by eriktorbjorn
	{
		{
			"ite",
			"Windows CD Version",
			{
				{"ite.rsc",		GAME_RESOURCEFILE,	"8f4315a9bb10ec839253108a032c8b54", 8928678},
				{"scripts.rsc",	GAME_SCRIPTFILE,	"a891405405edefc69c9d6c420c868b84", -1},
				{"cave.mid",	0,					"f7619359323058b61ec44fa8ce7888e5", 4441},
				AD_LISTEND
			},
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_CD,
			GUIO0()
		},
		GID_ITE,
		GF_EXTRA_ITE_CREDITS,
		ITE_DEFAULT_SCENE,
		RESOURCELIST_ITE,
		FONTLIST_ITE,
		PATCHLIST_ITE,
		INTROLIST_ITE_DEFAULT,
		{},
	},

	// Inherit the earth - Wyrmkeep Linux CD version
	// Reported by eriktorbjorn
	{
		{
			"ite",
			"Linux CD Version",
			{
				{"ite.rsc",		GAME_RESOURCEFILE,		"8f4315a9bb10ec839253108a032c8b54", 8928678},
				{"scripts.rsc",	GAME_SCRIPTFILE,		"a891405405edefc69c9d6c420c868b84", -1},
				{"music.rsc",	GAME_DIGITALMUSICFILE,	"d6454756517f042f01210458abe8edd4", 52894196},
				AD_LISTEND
			},
			Common::EN_ANY,
			Common::kPlatformLinux,
			ADGF_CD,
			GUIO0()
		},
		GID_ITE,
		GF_EXTRA_ITE_CREDITS,
		ITE_DEFAULT_SCENE,
		RESOURCELIST_ITE,
		FONTLIST_ITE,
		PATCHLIST_ITE,
		INTROLIST_ITE_DEFAULT,
		{},
	},

	// Inherit the earth - DOS CD version - 17-06-1994
	// Reported by eriktorbjorn
	{
		{
			"ite",
			"DOS CD Version 1",
			{
				{"ite.rsc",		GAME_RESOURCEFILE,	"8f4315a9bb10ec839253108a032c8b54", 8928678},
				{"scripts.rsc",	GAME_SCRIPTFILE,	"a891405405edefc69c9d6c420c868b84", -1},
				AD_LISTEND
			},
			Common::EN_ANY,
			Common::kPlatformDOS,
			ADGF_CD,
			GUIO0()
		},
		GID_ITE,
		GF_EXTRA_ITE_CREDITS,
		ITE_DEFAULT_SCENE,
		RESOURCELIST_ITE,
		FONTLIST_ITE,
		PATCHLIST_ITE,
		INTROLIST_ITE_DEFAULT,
		{},
	},

	// Inherit the earth - Wyrmkeep Italian Windows CD version (fan translation)
	{
		{
			"ite",
			"Windows CD Version",
			{
				{"ite.rsc",		GAME_RESOURCEFILE,	"8f4315a9bb10ec839253108a032c8b54", 8929956},
				{"scripts.rsc",	GAME_SCRIPTFILE,	"a891405405edefc69c9d6c420c868b84", 350243},
				{"cave.mid",	GAME_RESOURCEFILE,	"f7619359323058b61ec44fa8ce7888e5", 4441},
				AD_LISTEND
			},
			Common::IT_ITA,
			Common::kPlatformWindows,
			ADGF_CD,
			GUIO0()
		},
		GID_ITE,
		0,
		ITE_DEFAULT_SCENE,
		RESOURCELIST_ITE,
		FONTLIST_ITE,
		PATCHLIST_ITE,
		INTROLIST_ITE_DEFAULT,
		{},
	},

	// Inherit the earth - Wyrmkeep Italian Linux CD version (fan translation)
	{
		{
			"ite",
			"Linux CD Version",
			{
				{"ite.rsc",		GAME_RESOURCEFILE,		"8f4315a9bb10ec839253108a032c8b54", 8929956},
				{"scripts.rsc",	GAME_SCRIPTFILE,		"a891405405edefc69c9d6c420c868b84", 350243},
				{"music.rsc",	GAME_DIGITALMUSICFILE,	"d6454756517f042f01210458abe8edd4", 52894196},
				AD_LISTEND
			},
			Common::IT_ITA,
			Common::kPlatformLinux,
			ADGF_CD,
			GUIO0()
		},
		GID_ITE,
		0,
		ITE_DEFAULT_SCENE,
		RESOURCELIST_ITE,
		FONTLIST_ITE,
		PATCHLIST_ITE,
		INTROLIST_ITE_DEFAULT,
		{},
	},

	// Inherit the earth - Italian DOS CD version (fan translation)
	{
		{
			"ite",
			"DOS CD Version",
			{
				{"ite.rsc",		GAME_RESOURCEFILE,	"8f4315a9bb10ec839253108a032c8b54", 8929956},
				{"scripts.rsc",	GAME_SCRIPTFILE,	"a891405405edefc69c9d6c420c868b84", 350243},
				AD_LISTEND
			},
			Common::IT_ITA,
			Common::kPlatformDOS,
			ADGF_CD,
			GUIO0()
		},
		GID_ITE,
		0,
		ITE_DEFAULT_SCENE,
		RESOURCELIST_ITE,
		FONTLIST_ITE,
		PATCHLIST_ITE,
		INTROLIST_ITE_DEFAULT,
		{},
	},

	// Inherit the earth - Wyrmkeep French Windows CD version (fan translation)
	{
		{
			"ite",
			"Windows CD Version",
			{
				{"ite.rsc",		GAME_RESOURCEFILE,	"8f4315a9bb10ec839253108a032c8b54", 8929384},
				{"scripts.rsc",	GAME_SCRIPTFILE,	"a891405405edefc69c9d6c420c868b84", 355877},
				{"cave.mid",	GAME_RESOURCEFILE,	"f7619359323058b61ec44fa8ce7888e5", 4441},
				AD_LISTEND
			},
			Common::FR_FRA,
			Common::kPlatformWindows,
			ADGF_CD,
			GUIO0()
		},
		GID_ITE,
		0,
		ITE_DEFAULT_SCENE,
		RESOURCELIST_ITE,
		FONTLIST_ITE,
		PATCHLIST_ITE,
		INTROLIST_ITE_DEFAULT,
		{},
	},

	// Inherit the earth - Wyrmkeep French Linux CD version (fan translation)
	{
		{
			"ite",
			"Linux CD Version",
			{
				{"ite.rsc",		GAME_RESOURCEFILE,		"8f4315a9bb10ec839253108a032c8b54", 8929384},
				{"scripts.rsc",	GAME_SCRIPTFILE,		"a891405405edefc69c9d6c420c868b84", 355877},
				{"music.rsc",	GAME_DIGITALMUSICFILE,	"d6454756517f042f01210458abe8edd4", 52894196},
				AD_LISTEND
			},
			Common::FR_FRA,
			Common::kPlatformLinux,
			ADGF_CD,
			GUIO0()
		},
		GID_ITE,
		0,
		ITE_DEFAULT_SCENE,
		RESOURCELIST_ITE,
		FONTLIST_ITE,
		PATCHLIST_ITE,
		INTROLIST_ITE_DEFAULT,
		{},
	},

	// Inherit the earth - French DOS CD version (fan translation)
	{
		{
			"ite",
			"DOS CD Version",
			{
				{"ite.rsc",		GAME_RESOURCEFILE,	"8f4315a9bb10ec839253108a032c8b54", 8929384},
				{"scripts.rsc",	GAME_SCRIPTFILE,	"a891405405edefc69c9d6c420c868b84", 355877},
				AD_LISTEND
			},
			Common::FR_FRA,
			Common::kPlatformDOS,
			ADGF_CD,
			GUIO0()
		},
		GID_ITE,
		0,
		ITE_DEFAULT_SCENE,
		RESOURCELIST_ITE,
		FONTLIST_ITE,
		PATCHLIST_ITE,
		INTROLIST_ITE_DEFAULT,
		{},
	},

	// Inherit the earth - DOS CD version - 30-06-1994
	{
		{
			"ite",
			"DOS CD Version 2",
			{
				{"ite.rsc",		GAME_RESOURCEFILE,	"8f4315a9bb10ec839253108a032c8b54", -1},
				{"scripts.rsc",	GAME_SCRIPTFILE,	"50a0d2d7003c926a3832d503c8534e90", -1},
				AD_LISTEND
			},
			Common::EN_ANY,
			Common::kPlatformDOS,
			ADGF_CD,
			GUIO0()
		},
		GID_ITE,
		0,
		ITE_DEFAULT_SCENE,
		RESOURCELIST_ITE,
		FONTLIST_ITE,
		PATCHLIST_ITE,
		INTROLIST_ITE_DEFAULT,
		{},
	},

	// Inherit the earth - Hebrew fan translation
	{
		{
			"ite",
			"CD Version",
			{
				{"ite.rsc",		GAME_RESOURCEFILE,	"b9354a4c2d138f680306e4baf8585e9d", -1},
				{"scripts.rsc",	GAME_SCRIPTFILE,	"a891405405edefc69c9d6c420c868b84", -1},
				AD_LISTEND
			},
			Common::HE_ISR,
			Common::kPlatformUnknown,
			ADGF_CD,
			GUIO0()
		},
		GID_ITE,
		0,
		ITE_DEFAULT_SCENE,
		RESOURCELIST_ITE,
		FONTLIST_ITE,
		PATCHLIST_NONE,
		INTROLIST_ITE_DEFAULT,
		{},
	},

	// Inherit the earth - DOS CD German version
	// reported by mld. Bestsellergamers cover disk
	{
		{
			"ite",
			"CD Version",
			{
				{"ite.rsc",		GAME_RESOURCEFILE,	"869fc23c8f38f575979ec67152914fee", -1},
				{"scripts.rsc",	GAME_SCRIPTFILE,	"a891405405edefc69c9d6c420c868b84", -1},
				AD_LISTEND
			},
			Common::DE_DEU,
			Common::kPlatformDOS,
			ADGF_CD,
			GUIO0()
		},
		GID_ITE,
		0,
		ITE_DEFAULT_SCENE,
		RESOURCELIST_ITE,
		FONTLIST_ITE,
		PATCHLIST_NONE,
		INTROLIST_ITE_DEFAULT,
		{},
	},

	// Inherit the earth - Windows Trial
	// Not supported and will not be supported, as fixing it
	// enables whole game. Currently, it crashes.
	{
		{
			"ite",
			// I18N: Inherit the Earth had a "trial" version which is a full game with a simple check
			_s("Windows Trial version is not supported"),
			{
				{"ite.rsc",		GAME_RESOURCEFILE,	"a6433e34b97b15e64fe8214651012db9", 8927169},
				{"scripts.rsc",	GAME_SCRIPTFILE,	"bbf929f1e6d6f2af30c41d078798f5c1", 335927},
				AD_LISTEND
			},
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_CD | ADGF_DEMO | ADGF_UNSUPPORTED,
			GUIO0()
		},
		GID_ITE,
		0,
		ITE_DEFAULT_SCENE,
		RESOURCELIST_ITE,
		FONTLIST_ITE,
		PATCHLIST_NONE,
		INTROLIST_ITE_DEFAULT,
		{},
	},

	// Inherit the earth - macOS Trial
	// Linux trial version uses same resources a Wyrmkeep Multi-OS
	{
		{
			"ite",
			// I18N: Inherit the Earth had a "trial" version which is a full game with a simple check
			_s("macOS Trial version is not supported"),
			{
				{"ite_i.rsc",		GAME_RESOURCEFILE,	"a6433e34b97b15e64fe8214651012db9", 8927169},
				{"scripts_i.rsc",	GAME_SCRIPTFILE,	"bbf929f1e6d6f2af30c41d078798f5c1", 335927},
				AD_LISTEND
			},
			Common::EN_ANY,
			Common::kPlatformMacintosh,
			ADGF_CD | ADGF_DEMO | ADGF_UNSUPPORTED,
			GUIO0()
		},
		GID_ITE,
		0,
		ITE_DEFAULT_SCENE,
		RESOURCELIST_ITE,
		FONTLIST_ITE,
		PATCHLIST_NONE,
		INTROLIST_ITE_DEFAULT,
		{},
	},

	// Inherit the earth - Pocket PC Trial
	{
		{
			"ite",
			// I18N: Inherit the Earth had a "trial" version which is a full game with a simple check
			_s("Pocket PC Trial version is not supported"),
			{
				{"ite.rsc",			GAME_RESOURCEFILE,	"a6433e34b97b15e64fe8214651012db9", 8927169},
				{"scripts.rsc",		GAME_SCRIPTFILE,	"a891405405edefc69c9d6c420c868b84", 335927},
				{"voicesv.rsc",		GAME_VOICEFILE,		"7e751eaab3b3127cec5a360e94cafd8b", 43744418},
				AD_LISTEND
			},
			Common::EN_ANY,
			Common::kPlatformPocketPC,
			ADGF_DEMO | ADGF_UNSUPPORTED,
			GUIO0()
		},
		GID_ITE,
		0,
		ITE_DEFAULT_SCENE,
		RESOURCELIST_ITE,
		FONTLIST_ITE,
		PATCHLIST_ITE,
		INTROLIST_ITE_DEFAULT,
		{},
	},

	// Inherit the Earth - Japanese PC-98 CD version
 	{
 		{
 			"ite",
 			"CD",
 			{
			 	{"ite.rsc",		GAME_RESOURCEFILE, "8f4315a9bb10ec839253108a032c8b54", 8921524},
			 	{"scripts.rsc",	GAME_SCRIPTFILE, "875ffcf269efc49088423fc4572859ae", 477849},
			 	AD_LISTEND
 			},
 			Common::JA_JPN,
 			Common::kPlatformPC98,
 			ADGF_CD,
 			GUIO0()
 		},
 		GID_ITE,
		GF_ITE_FLOPPY,	// Even it that game version comes on a CD it behaves like a DOS floppy version
 		ITE_DEFAULT_SCENE,
		RESOURCELIST_ITE,
		FONTLIST_ITE,
		PATCHLIST_NONE,
		INTROLIST_ITE_DEFAULT,
		{},
 	},

	// Inherit the earth - Russian CD version (fan translation)
	{
		{
			"ite",
			"CD Version",
			{
				{ "ite.rsc",		GAME_RESOURCEFILE,	"fedbe4a01170f7a94e3426a2a9550be3", 9779482},
				{ "scripts.rsc",	GAME_SCRIPTFILE,	"50a0d2d7003c926a3832d503c8534e90", 328255},
				AD_LISTEND
			},
			Common::RU_RUS,
			Common::kPlatformDOS,
			ADGF_CD,
			GUIO0()
		},
		GID_ITE,
		0,
		ITE_DEFAULT_SCENE,
		RESOURCELIST_ITE,
		FONTLIST_ITE,
		PATCHLIST_ITE,
		INTROLIST_ITE_DEFAULT,
		{},
	},

	// ITE floppy versions ////////////////////////////////////////////////////////////////////////////////////

	// Inherit the earth - German Floppy version
	{
		{
			"ite",
			"Floppy",
			{
				{"ite.rsc",		GAME_RESOURCEFILE,					"869fc23c8f38f575979ec67152914fee", -1},
				{"scripts.rsc",	GAME_SCRIPTFILE,					"516f7330f8410057b834424ea719d1ef", -1},
				AD_LISTEND
			},
			Common::DE_DEU,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOSPEECH)
		},
		GID_ITE,
		GF_ITE_FLOPPY,
		ITE_DEFAULT_SCENE,
		RESOURCELIST_ITE,
		FONTLIST_ITE,
		PATCHLIST_NONE,
		INTROLIST_ITE_DEFAULT,
		{},
	},

	// Inherit the earth - Disk version
	{
		{
			"ite",
			"Floppy",
			{
				{"ite.rsc",		GAME_RESOURCEFILE,					"8f4315a9bb10ec839253108a032c8b54", 8903828},
				{"scripts.rsc",	GAME_SCRIPTFILE,					"516f7330f8410057b834424ea719d1ef", -1},
				AD_LISTEND
			},
			Common::EN_ANY,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOSPEECH)
		},
		GID_ITE,
		GF_ITE_FLOPPY,
		ITE_DEFAULT_SCENE,
		RESOURCELIST_ITE,
		FONTLIST_ITE,
		PATCHLIST_ITE,
		INTROLIST_ITE_DEFAULT,
		{},
	},

	// Same as above but installer
	{
		{
			"ite",
			"Floppy Packed",
			{
				{"inherit.arj", 0, "a7910371d6b43c00a318bc9c6f0062b4", 1413298},
				{"inherit.a01", 0, "e5034686e3bc531ba1bbcb6c6efe90c1", 1456420},
				{"inherit.a02", 0, "a8d3bf36dd7ece019fd2f819a0dfed70", 1456328},
				{"inherit.a03", 0, "456a4ee942cdf653754fbec4e942db46", 1456284},
				{"inherit.a04", 0, "748865cda20e1404b19f17b5aaab255d", 1456289},
				{"inherit.105", 0, "c03f250b95dedf7f10bf188c49047140", 1117200},
				AD_LISTEND
			},
			Common::EN_ANY,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOSPEECH)
		},
		GID_ITE,
		GF_ITE_FLOPPY | GF_INSTALLER,
		ITE_DEFAULT_SCENE,
		RESOURCELIST_ITE,
		FONTLIST_ITE,
		PATCHLIST_ITE,
		INTROLIST_ITE_DEFAULT,
		{
			{"ite.rsc",		GAME_RESOURCEFILE,					"8f4315a9bb10ec839253108a032c8b54", 8903828},
			{"scripts.rsc",	GAME_SCRIPTFILE,					"516f7330f8410057b834424ea719d1ef", -1},
			AD_LISTEND
		},
	},


	// Inherit the earth - Italian Disk version (fan translation)
	{
		{
			"ite",
			"Floppy",
			{
				{"ite.rsc",		GAME_RESOURCEFILE,					"8f4315a9bb10ec839253108a032c8b54", 8905106},
				{"scripts.rsc",	GAME_SCRIPTFILE,					"516f7330f8410057b834424ea719d1ef", 340726},
				AD_LISTEND
			},
			Common::IT_ITA,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOSPEECH)
		},
		GID_ITE,
		GF_ITE_FLOPPY,
		ITE_DEFAULT_SCENE,
		RESOURCELIST_ITE,
		FONTLIST_ITE,
		PATCHLIST_ITE,
		INTROLIST_ITE_DEFAULT,
		{},
	},

	// Inherit the earth - Chinese Disk version
	{
		{
			"ite",
			"Floppy",
			{
				{"ite.rsc",		GAME_RESOURCEFILE,					"8f4315a9bb10ec839253108a032c8b54", 8901704},
				{"scripts.rsc",	GAME_SCRIPTFILE,					"516f7330f8410057b834424ea719d1ef", 281071},
				AD_LISTEND
			},
			Common::ZH_TWN,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOSPEECH)
		},
		GID_ITE,
		GF_ITE_FLOPPY,
		ITE_DEFAULT_SCENE,
		RESOURCELIST_ITE,
		FONTLIST_ITE,
		PATCHLIST_ITE,
		INTROLIST_ITE_DEFAULT,
		{},
	},

	// Inherit the earth - Russian Disk version (fan translation)
	{
		{
			"ite",
			"Floppy",
			{
				{"ite.rsc",		GAME_RESOURCEFILE,					"fedbe4a01170f7a94e3426a2a9550be3", 9150935},
				{"scripts.rsc",	GAME_SCRIPTFILE,					"50a0d2d7003c926a3832d503c8534e90", 328194},
				AD_LISTEND
			},
			Common::RU_RUS,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOSPEECH)
		},
		GID_ITE,
		GF_ITE_FLOPPY,
		ITE_DEFAULT_SCENE,
		RESOURCELIST_ITE,
		FONTLIST_ITE,
		PATCHLIST_ITE,
		INTROLIST_ITE_DEFAULT,
		{},
	},

	// ITE Amiga versions /////////////////////////////////////////////////////////////////////////////////////

	// Official Dreamers Guild German release
	{
		{
			"ite",
			"AGA CD",
			{
				{"ite.rtn",		GAME_RESOURCEFILE,	"9d97f645eb877d1c9eb9d906930c3257", 18564},
				{"aga.exe",                             0,      "f18ff495b8346983400a8eb6175590e5", 203224},
				{"ite.sounds",             GAME_SOUNDFILE,      "f09b29e3204192de7e8cc6b073cb34f5", 640284},
				{"ite.voices",             GAME_VOICEFILE,      "f2efcf1c175c572f91cc4ec66dbb226f", 221124347},
				AD_LISTEND
			},
			Common::DE_DEU,
			Common::kPlatformAmiga,
			ADGF_CD,
			GUIO0()
		},
		GID_ITE,
		GF_EXTRA_ITE_CREDITS | GF_AGA_GRAPHICS,
		ITE_DEFAULT_SCENE,
		RESOURCELIST_ITE_GERMAN_AGA,
		FONTLIST_ITE,
		PATCHLIST_ITE,
		INTROLIST_ITE_AMIGA_GERMAN_AGA,
		{},

	},
	// This is on the same disk as previous but it's for ECS systems
	{
		{
			"ite",
			"ECS CD",
			{
				{"ecs.rtn",		GAME_RESOURCEFILE,	"62d738d3201c1624f0e4bfcc8fb587dd", 18624},
				{"ecs.exe",                             0,      "b6a5ae7b07c620a8101e87bb0d2af136", 205064},
				{"ite.sounds",             GAME_SOUNDFILE,      "f09b29e3204192de7e8cc6b073cb34f5", 640284},
				{"ite.voices",             GAME_VOICEFILE,      "f2efcf1c175c572f91cc4ec66dbb226f", 221124347},
				AD_LISTEND
			},
			Common::DE_DEU,
			Common::kPlatformAmiga,
			ADGF_CD,
			GUIO0()
		},
		GID_ITE,
		GF_EXTRA_ITE_CREDITS | GF_ECS_GRAPHICS,
		ITE_DEFAULT_SCENE,
		RESOURCELIST_ITE_GERMAN_ECS,
		FONTLIST_ITE,
		PATCHLIST_ITE,
		INTROLIST_ITE_AMIGA_GERMAN_ECS,
		{},
	},
	// Amiga Future coverdisk/Wyrmkeep English edition
	{
		{
			"ite",
			"AGA CD",
			{
				{"ite.rtn",		GAME_RESOURCEFILE,	"749885c0f7eaab4e977dc26a41d99ad8", 18524},
				{"aga.exe",                             0,      "e6d93bbf0f89786eb930fbc81e02810d", 202328},
				{"ite.sounds",             GAME_SOUNDFILE,      "f09b29e3204192de7e8cc6b073cb34f5", 640284},
				{"ite.voices",             GAME_VOICEFILE,      "daf62750f5322da03fab395e548b0b8f", 217376825},
				AD_LISTEND
			},
			Common::EN_ANY,
			Common::kPlatformAmiga,
			ADGF_CD,
			GUIO0()
		},
		GID_ITE,
		GF_EXTRA_ITE_CREDITS | GF_AGA_GRAPHICS,
		ITE_DEFAULT_SCENE,
		RESOURCELIST_ITE,
		FONTLIST_ITE,
		PATCHLIST_ITE,
		INTROLIST_ITE_DEFAULT,
		{},
	},
	// This is on the same disk as previous but it's for ECS systems
	{
		{
			"ite",
			"ECS CD",
			{
				{"ecs.rtn",		GAME_RESOURCEFILE,	"c9d09514839d771efdc82ad761413349", 18584},
				{"ecs.exe",                             0,      "29665b96c2758aec3906ebc891079234", 204228},
				{"ite.sounds",             GAME_SOUNDFILE,      "f09b29e3204192de7e8cc6b073cb34f5", 640284},
				{"ite.voices",             GAME_VOICEFILE,      "daf62750f5322da03fab395e548b0b8f", 217376825},
				AD_LISTEND
			},
			Common::EN_ANY,
			Common::kPlatformAmiga,
			ADGF_CD,
			GUIO0()
		},
		GID_ITE,
		GF_EXTRA_ITE_CREDITS | GF_ECS_GRAPHICS,
		ITE_DEFAULT_SCENE,
		RESOURCELIST_ITE_ENGLISH_ECS,
		FONTLIST_ITE,
		PATCHLIST_ITE,
		INTROLIST_ITE_AMIGA_ENGLISH_ECS,
		{},
	},

	// Inherit the earth - German Floppy version
	{
		{
			"ite",
			"AGA Floppy",
			{
				{"ite.rtn",		GAME_RESOURCEFILE,	"38f6a3aca708ef3ab6059d94a268da29", 18564},
				{"ite01.adf",		0,					"4f7913f82d7f8318d24f31b6226731eb", 901120},
				{"ite02.adf",	0,					"9c959343c3e2e4a067426bf4cf28eba0", 901120},
				AD_LISTEND
			},
			Common::DE_DEU,
			Common::kPlatformAmiga,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOSPEECH)
		},
		GID_ITE,
		GF_ITE_FLOPPY | GF_AGA_GRAPHICS | GF_EMBED_FONT | GF_POWERPACK_GFX,
		ITE_DEFAULT_SCENE,
		RESOURCELIST_ITE_GERMAN_AGA,
		FONTLIST_NONE,
		PATCHLIST_NONE,
		INTROLIST_ITE_AMIGA_GERMAN_AGA,
		{},
	},
	{
		{
			"ite",
			"ECS Floppy",
			{
				{"ite.rtn",		GAME_RESOURCEFILE,	"733a15d1816b76ed10fbb02668ae1f93", 18624},
				{"ite01.adf",		0,					"49352821102fc03d1653e0fda13fb15f", 901120},
				{"ite02.adf",	0,					"160189cce6ecec4bfb4e045bf9be3218", 901120},
				AD_LISTEND
			},
			Common::DE_DEU,
			Common::kPlatformAmiga,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOSPEECH)
		},
		GID_ITE,
		GF_ITE_FLOPPY | GF_ECS_GRAPHICS | GF_EMBED_FONT | GF_POWERPACK_GFX,
		ITE_DEFAULT_SCENE,
		RESOURCELIST_ITE_GERMAN_ECS,
		FONTLIST_NONE,
		PATCHLIST_NONE,
		INTROLIST_ITE_AMIGA_GERMAN_ECS,
		{},
	},
	{
		{
			"ite",
			"ECS Floppy",
			{
				{"ite.rtn",		GAME_RESOURCEFILE,	"9d0c6b31ac86b4bd9237e304e4bfb73a", 18624},
				{"ite01.adf",		0,					"532973f8422f46f73db5b8a161681d40", 901120},
				{"ite02.adf",	0,					"794f525b9b81f765a5df18bfae8364ec", 901120},
				AD_LISTEND
			},
			Common::EN_ANY,
			Common::kPlatformAmiga,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOSPEECH)
		},
		GID_ITE,
		GF_ITE_FLOPPY | GF_ECS_GRAPHICS | GF_EMBED_FONT | GF_POWERPACK_GFX,
		ITE_DEFAULT_SCENE,
		RESOURCELIST_ITE_GERMAN_ECS, // Uses German resource list for some reason
		FONTLIST_NONE,
		PATCHLIST_NONE,
		INTROLIST_ITE_AMIGA_GERMAN_ECS, // Uses German intro list for some reason
		{},
	},

	// IHNM Section ///////////////////////////////////////////////////////////////////////////////////////////

	// I Have No Mouth And I Must Scream - Demo version
	{
		{
			"ihnm",
			"Demo",
			{
				{"music.res",		GAME_MUSICFILE_FM,	"0439083e3dfdc51b486071d45872ae52", -1},
				{"scream.res",		GAME_RESOURCEFILE,	"46bbdc65d164ba7e89836a0935eec8e6", 16687179},
				{"scripts.res",		GAME_SCRIPTFILE,	"9626bda8978094ff9b29198bc1ed5f9a", -1},
				{"sfx.res",			GAME_SOUNDFILE,		"1c610d543f32ec8b525e3f652536f269", -1},
				AD_LISTEND
			},
			Common::EN_ANY,
			Common::kPlatformDOS,
			ADGF_DEMO,
			GUIO1(GUIO_NOASPECT)
		},
		GID_IHNM,
		0,
		IHNMDEMO_DEFAULT_SCENE,
		RESOURCELIST_IHNM_DEMO,
		FONTLIST_IHNM_DEMO,
		PATCHLIST_NONE,
		INTROLIST_NONE,
		{},
	},

	// I Have No Mouth And I Must Scream - English CD version
	{
		{
			"ihnm",
			"",
			{
				{"musicfm.res",	GAME_MUSICFILE_FM,					"0439083e3dfdc51b486071d45872ae52", -1},
				{"musicgm.res",	GAME_MUSICFILE_GM,					"80f875a1fb384160d1f4b27166eef583", -1},
				{"scream.res",	GAME_RESOURCEFILE,					"46bbdc65d164ba7e89836a0935eec8e6", 79211140},
				{"patch.re_",	GAME_PATCHFILE | GAME_RESOURCEFILE,	"58b79e61594779513c7f2d35509fa89e", -1},
				{"scripts.res",	GAME_SCRIPTFILE,					"be38bbc5a26be809dbf39f13befebd01", -1},
				//{"sfx.res",		GAME_SOUNDFILE,						"1c610d543f32ec8b525e3f652536f269", -1},
				// There are two English versions of the game, each one with a different sfx.res file
				// Known MD5 checksums for sfx.res in the English version of the game are
				// 1c610d543f32ec8b525e3f652536f269 and 45a9a9f5d37740be24fd2ae2edf36573
				{"sfx.res",		GAME_SOUNDFILE,						NULL, -1},
				AD_LISTEND
			},
			Common::EN_ANY,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOASPECT)
		},
		GID_IHNM,
		0,
		IHNM_DEFAULT_SCENE,
		RESOURCELIST_IHNM,
		FONTLIST_IHNM_CD,
		PATCHLIST_NONE,
		INTROLIST_NONE,
		{},
	},

	// I Have No Mouth And I Must Scream - De CD version
	// Censored CD version (without Nimdok)
	// Reported by mld. German Retail
	{
		{
			"ihnm",
			"",
			{
				{"musicfm.res",	GAME_MUSICFILE_FM,					"0439083e3dfdc51b486071d45872ae52", -1},
				{"musicgm.res",	GAME_MUSICFILE_GM,					"80f875a1fb384160d1f4b27166eef583", -1},
				{"scream.res",	GAME_RESOURCEFILE,					"c92370d400e6f2a3fc411c3729d09224", 78517884},
				{"scripts.res",	GAME_SCRIPTFILE,					"32aa01a89937520fe0ea513950117292", -1},
				{"patch.re_",	GAME_PATCHFILE | GAME_RESOURCEFILE,	"58b79e61594779513c7f2d35509fa89e", -1},
				{"sfx.res",		GAME_SOUNDFILE,						"1c610d543f32ec8b525e3f652536f269", -1},
				AD_LISTEND
			},
			Common::DE_DEU,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOASPECT)
		},
		GID_IHNM,
		GF_IHNM_COLOR_FIX,
		IHNM_DEFAULT_SCENE,
		RESOURCELIST_IHNM,
		FONTLIST_IHNM_CD,
		PATCHLIST_NONE,
		INTROLIST_NONE,
		{},
	},

	// I Have No Mouth And I Must Scream - German fan CD translation
	// English CD version with German text patch (with Nimdok)
	// (English speech - German text)
	{
		{
			"ihnm",
			"fan-made",
			{
				{"musicfm.res",	GAME_MUSICFILE_FM,					"0439083e3dfdc51b486071d45872ae52", 302676},
				{"musicgm.res",	GAME_MUSICFILE_GM,					"80f875a1fb384160d1f4b27166eef583", 314020},
				{"scream.res",	GAME_RESOURCEFILE,					"46bbdc65d164ba7e89836a0935eec8e6", 79219797},
				{"scripts.res",	GAME_SCRIPTFILE,					"be38bbc5a26be809dbf39f13befebd01", 523800},
				{"patch.re_",	GAME_PATCHFILE | GAME_RESOURCEFILE,	"58b79e61594779513c7f2d35509fa89e", 5038599},
				{"sfx.res",		GAME_SOUNDFILE,						"1c610d543f32ec8b525e3f652536f269", 22561056},
				AD_LISTEND
			},
			Common::DE_DEU,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOASPECT)
		},
		GID_IHNM,
		0,
		IHNM_DEFAULT_SCENE,
		RESOURCELIST_IHNM,
		FONTLIST_IHNM_CD,
		PATCHLIST_NONE,
		INTROLIST_NONE,
		{},
	},

	// I Have No Mouth And I Must Scream - Hungarian fan CD translation
	// English CD version with Hungarian text patch
	// (English speech - Hungarian text)
	{
		{
			"ihnm",
			"fan-made",
			{
				{"musicfm.res",	GAME_MUSICFILE_FM,					"0439083e3dfdc51b486071d45872ae52", 302676},
				{"musicgm.res",	GAME_MUSICFILE_GM,					"80f875a1fb384160d1f4b27166eef583", 314020},
				{"scream.res",	GAME_RESOURCEFILE,					"46bbdc65d164ba7e89836a0935eec8e6", 83411541},
				{"scripts.res",	GAME_SCRIPTFILE,					"be38bbc5a26be809dbf39f13befebd01", 505001},
				{"patch.re_",	GAME_PATCHFILE | GAME_RESOURCEFILE,	"58b79e61594779513c7f2d35509fa89e", 5038599},
				{"sfx.res",		GAME_SOUNDFILE,						"1c610d543f32ec8b525e3f652536f269", 22561056},
				AD_LISTEND
			},
			Common::HU_HUN,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOASPECT)
		},
		GID_IHNM,
		0,
		IHNM_DEFAULT_SCENE,
		RESOURCELIST_IHNM,
		FONTLIST_IHNM_CD,
		PATCHLIST_NONE,
		INTROLIST_NONE,
		{},
	},

	// I Have No Mouth And I Must Scream - Sp CD version
	{
		{
			"ihnm",
			"",
			{
				{"musicfm.res",	GAME_MUSICFILE_FM,					"0439083e3dfdc51b486071d45872ae52", -1},
				{"musicgm.res",	GAME_MUSICFILE_GM,					"80f875a1fb384160d1f4b27166eef583", -1},
				{"scream.res",	GAME_RESOURCEFILE,					"c92370d400e6f2a3fc411c3729d09224", 78560025},
				{"patch.re_",	GAME_PATCHFILE | GAME_RESOURCEFILE,	"58b79e61594779513c7f2d35509fa89e", -1},
				{"scripts.res",	GAME_SCRIPTFILE,					"be38bbc5a26be809dbf39f13befebd01", -1},
				{"sfx.res",		GAME_SOUNDFILE,						"1c610d543f32ec8b525e3f652536f269", -1},
				AD_LISTEND
			},
			Common::ES_ESP,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOASPECT)
		},
		GID_IHNM,
		GF_IHNM_COLOR_FIX,
		IHNM_DEFAULT_SCENE,
		RESOURCELIST_IHNM,
		FONTLIST_IHNM_CD,
		PATCHLIST_NONE,
		INTROLIST_NONE,
		{},
	},

	// I Have No Mouth And I Must Scream - Korean CD version
	{
		{
			"ihnm",
			"",
			{
				{"musicfm.res",	GAME_MUSICFILE_FM,					"0439083e3dfdc51b486071d45872ae52", -1},
				{"musicgm.res",	GAME_MUSICFILE_GM,					"80f875a1fb384160d1f4b27166eef583", -1},
				{"scream.res",	GAME_RESOURCEFILE,					"46bbdc65d164ba7e89836a0935eec8e6", 79211140},
				{"patch.re_",	GAME_PATCHFILE | GAME_RESOURCEFILE,	"58b79e61594779513c7f2d35509fa89e", -1},
				{"scripts.res",	GAME_SCRIPTFILE,					"be38bbc5a26be809dbf39f13befebd01", -1},
				{"sfx.res",		GAME_SOUNDFILE,						"1c610d543f32ec8b525e3f652536f269", -1},
				{"sbh1616.fnt",	0,									"ec047bbe048ed9465def705b5bd74d99", -1},
				AD_LISTEND
			},
			Common::KO_KOR,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOASPECT)
		},
		GID_IHNM,
		GF_IHNM_COLOR_FIX,
		IHNM_DEFAULT_SCENE,
		RESOURCELIST_IHNM,
		FONTLIST_IHNM_CD,
		PATCHLIST_NONE,
		INTROLIST_NONE,
		{},
	},

	// I Have No Mouth And I Must Scream - Fr CD version
	// Censored CD version (without Nimdok)
	{
		{
			"ihnm",
			"",
			{
				{"musicfm.res",	GAME_MUSICFILE_FM,					"0439083e3dfdc51b486071d45872ae52", -1},
				{"musicgm.res",	GAME_MUSICFILE_GM,					"80f875a1fb384160d1f4b27166eef583", -1},
				{"scream.res",	GAME_RESOURCEFILE,					"c92370d400e6f2a3fc411c3729d09224", 78519324},
				{"scripts.res",	GAME_SCRIPTFILE,					"32aa01a89937520fe0ea513950117292", -1},
				{"patch.re_",	GAME_PATCHFILE | GAME_RESOURCEFILE,	"58b79e61594779513c7f2d35509fa89e", -1},
				{"sfx.res",		GAME_SOUNDFILE,						"1c610d543f32ec8b525e3f652536f269", -1},
				AD_LISTEND
			},
			Common::FR_FRA,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOASPECT)
		},
		GID_IHNM,
		GF_IHNM_COLOR_FIX,
		IHNM_DEFAULT_SCENE,
		RESOURCELIST_IHNM,
		FONTLIST_IHNM_CD,
		PATCHLIST_NONE,
		INTROLIST_NONE,
		{},
	},

	// I Have No Mouth And I Must Scream - Chinese CD
	// Bugreport #7894
	{
		{
			"ihnm",
			"",
			{
				{"musicfm.res",	GAME_MUSICFILE_FM,					"0439083e3dfdc51b486071d45872ae52", 302676},
				{"musicgm.res",	GAME_MUSICFILE_GM,					"80f875a1fb384160d1f4b27166eef583", 314020},
				{"scream.res",	GAME_RESOURCEFILE,					"4de402af490920e4e5fbb4307d734aec", 78792732},
				{"patch.re_",	GAME_PATCHFILE | GAME_RESOURCEFILE,	"58b79e61594779513c7f2d35509fa89e", 5038599},
				{"scripts.res",	GAME_SCRIPTFILE,					"aac64f4359183a8bed48800be259dcb2", 428943},
				{"sfx.res",		GAME_SOUNDFILE,						"1c610d543f32ec8b525e3f652536f269", 22561056},
				AD_LISTEND
			},
			Common::ZH_TWN,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOASPECT)
		},
		GID_IHNM,
		GF_IHNM_COLOR_FIX,
		IHNM_DEFAULT_SCENE,
		RESOURCELIST_IHNM,
		FONTLIST_IHNM_ZH,
		PATCHLIST_NONE,
		INTROLIST_NONE,
		{},
	},

	// I Have No Mouth And I Must Scream - Italian fan CD translation
	{
		{
			"ihnm",
			"",
			{
				{"musicfm.res",	GAME_MUSICFILE_FM,					"0439083e3dfdc51b486071d45872ae52", -1},
				{"musicgm.res",	GAME_MUSICFILE_GM,					"80f875a1fb384160d1f4b27166eef583", -1},
				{"scream.res",	GAME_RESOURCEFILE,					"46bbdc65d164ba7e89836a0935eec8e6", 79211498},
				{"patch.re_",	GAME_PATCHFILE | GAME_RESOURCEFILE,	"58b79e61594779513c7f2d35509fa89e", -1},
				{"scripts.res",	GAME_SCRIPTFILE,					"be38bbc5a26be809dbf39f13befebd01", -1},
				{"sfx.res",		GAME_SOUNDFILE,						"1c610d543f32ec8b525e3f652536f269", -1},
				AD_LISTEND
			},
			Common::IT_ITA,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOASPECT)
		},
		GID_IHNM,
		0,
		IHNM_DEFAULT_SCENE,
		RESOURCELIST_IHNM,
		FONTLIST_IHNM_CD,
		PATCHLIST_NONE,
		INTROLIST_NONE,
		{},
	},

	// I Have No Mouth And I Must Scream - Russian fan translaction v1.0 (by jack7277 et al)
	{
		{
			"ihnm",
			"",
			{
				{"musicfm.res",	GAME_MUSICFILE_FM,						"0439083e3dfdc51b486071d45872ae52", -1},
				{"musicgm.res",	GAME_MUSICFILE_GM,						"80f875a1fb384160d1f4b27166eef583", -1},
				{"scream.res",	GAME_RESOURCEFILE,						"ac00dd9e6701e8edbb49429dacbc4731", 79210049},
				{"patch.re_",	GAME_PATCHFILE | GAME_RESOURCEFILE,		"58b79e61594779513c7f2d35509fa89e", -1},
				{"scripts.res",	GAME_SCRIPTFILE,						"be38bbc5a26be809dbf39f13befebd01", -1},
				//{"sfx.res",	GAME_SOUNDFILE,							"1c610d543f32ec8b525e3f652536f269", -1},
				// There are two English versions of the game, each one with a different sfx.res file
				// Known MD5 checksums for sfx.res in the English version of the game are
				// 1c610d543f32ec8b525e3f652536f269 and 45a9a9f5d37740be24fd2ae2edf36573
				{"sfx.res",		GAME_SOUNDFILE,							NULL, -1},
				AD_LISTEND
			},
			Common::RU_RUS,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOASPECT)
		},
		GID_IHNM,
		0,
		IHNM_DEFAULT_SCENE,
		RESOURCELIST_IHNM,
		FONTLIST_IHNM_CD,
		PATCHLIST_NONE,
		INTROLIST_NONE,
		{},
	},

	// I Have No Mouth And I Must Scream - Mac English CD
	{
		{
			"ihnm",
			"",
			{
				{"scream.res",	GAME_RESOURCEFILE,					"46bbdc65d164ba7e89836a0935eec8e6", 79211140},
				{"patch.res",	GAME_PATCHFILE | GAME_RESOURCEFILE,	"58b79e61594779513c7f2d35509fa89e", 5038599},
				{"scripts.res",	GAME_SCRIPTFILE,					"be38bbc5a26be809dbf39f13befebd01", 493597},
				{"sfx.res",		GAME_SOUNDFILE,						"0c24f75c30c3c4cffc7bf0e6d997898b", 3082},
				AD_LISTEND
			},
			Common::EN_ANY,
			Common::kPlatformMacintosh,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOASPECT)
		},
		GID_IHNM,
		0,
		IHNM_DEFAULT_SCENE,
		RESOURCELIST_IHNM,
		FONTLIST_IHNM_CD,
		PATCHLIST_NONE,
		INTROLIST_NONE,
		{},
	},

	{ AD_TABLE_END_MARKER, 0, 0, 0, RESOURCELIST_NONE, FONTLIST_MAX, PATCHLIST_MAX, INTROLIST_NONE, {} }
};

} // End of namespace Saga
