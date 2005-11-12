/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001-2005 The ScummVM project
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
 * $Header$
 *
 */

#include "common/stdafx.h"

#include "backends/fs/fs.h"

#include "base/gameDetector.h"
#include "base/plugins.h"

#include "common/config-manager.h"
#include "common/file.h"
#include "common/md5.h"

#include "simon/simon.h"
#include "simon/intern.h"

using Common::File;

namespace Simon {

static int detectGame(const FSList &fslist, bool mode = false, int start = -1);

struct GameMD5 {
	GameIds id;
	const char *md5;
	const char *filename;
	bool caseSensitive;
};

#define FILE_MD5_BYTES 5000

static GameMD5 gameMD5[] = {
	{ GID_SIMON1ACORNDEMO, "b4a7526ced425ba8ad0d548d0ec69900", "data", false },
	{ GID_SIMON1ACORNDEMO, "425c7d1957699d35abca7e12a08c7422", "gamebase", false },
	{ GID_SIMON1ACORNDEMO, "22107c24dfb31b66ac503c28a6e20b19", "icondata", false},
	{ GID_SIMON1ACORNDEMO, "d9de7542612d9f4e0819ad0df5eac56b", "stripped", false},
	{ GID_SIMON1ACORNDEMO, "d198a80de2c59e4a0cd24b98814849e8", "tbllist", false},

	{ GID_SIMON1ACORN,     "64958b3a38afdcb85da1eeed85169806", "data", false },
	{ GID_SIMON1ACORN,     "28261b99cd9da1242189b4f6f2841bd6", "gamebase", false },
	{ GID_SIMON1ACORN,     "22107c24dfb31b66ac503c28a6e20b19", "icondata", false},
	{ GID_SIMON1ACORN,     "f3b27a3fbb45dcd323a48159496e45e8", "stripped", false},
	{ GID_SIMON1ACORN,     "d198a80de2c59e4a0cd24b98814849e8", "tbllist", false},

	{ GID_SIMON1CD32,      "bab7f19237cf7d7619b6c73631da1854", "gameamiga", true },
	{ GID_SIMON1CD32,      "565ef7a98dcc21ef526a2bb10b6f42ed", "icon.pkd", true },
	{ GID_SIMON1CD32,      "59be788020441e21861e284236fd08c1", "stripped.txt", true},
	{ GID_SIMON1CD32,      "f9d5bf2ce09f82289c791c3ca26e1e4b", "tbllist", true},

	{ GID_SIMON1DOS,       "9f93d27432ce44a787eef10adb640870", "gamepc", false },
	{ GID_SIMON1DOS,       "22107c24dfb31b66ac503c28a6e20b19", "icon.dat", false},
	{ GID_SIMON1DOS,       "2af9affc5981eec44b90d4c556145cb8", "stripped.txt", false},
	{ GID_SIMON1DOS,       "d198a80de2c59e4a0cd24b98814849e8", "tbllist", false},

	{ GID_SIMON1DEMO,      "2be4a21bc76e2fdc071867c130651439", "gdemo", false },
	{ GID_SIMON1DEMO,      "55af3b4d93972bc58bfee38a86b76c3f", "icon.dat", false},
	{ GID_SIMON1DEMO,      "33a2e329b97b2a349858d6a093159eb7", "stripped.txt", false},
	{ GID_SIMON1DEMO,      "1247e024e1f13ca54c1e354120c7519c", "tbllist", false},

	{ GID_SIMON1TALKIE,    "28261b99cd9da1242189b4f6f2841bd6", "gamepc", false },
	{ GID_SIMON1TALKIE,    "22107c24dfb31b66ac503c28a6e20b19", "icon.dat", false},
	{ GID_SIMON1TALKIE,    "64958b3a38afdcb85da1eeed85169806", "simon.gme", false },
	{ GID_SIMON1TALKIE,    "f3b27a3fbb45dcd323a48159496e45e8", "stripped.txt", false},
	{ GID_SIMON1TALKIE,    "d198a80de2c59e4a0cd24b98814849e8", "tbllist", false},

	{ GID_SIMON1TALKIE_FR, "00000000000000000000000000000000", "gamepc", false },
	{ GID_SIMON1TALKIE_FR, "00000000000000000000000000000000", "icon.dat", false},
	{ GID_SIMON1TALKIE_FR, "00000000000000000000000000000000", "simon.gme", false },
	{ GID_SIMON1TALKIE_FR, "00000000000000000000000000000000", "stripped.txt", false},
	{ GID_SIMON1TALKIE_FR, "00000000000000000000000000000000", "tbllist", false},

	{ GID_SIMON1TALKIE_DE, "00000000000000000000000000000000", "gamepc", false },
	{ GID_SIMON1TALKIE_DE, "00000000000000000000000000000000", "icon.dat", false},
	{ GID_SIMON1TALKIE_DE, "00000000000000000000000000000000", "simon.gme", false },
	{ GID_SIMON1TALKIE_DE, "00000000000000000000000000000000", "stripped.txt", false},
	{ GID_SIMON1TALKIE_DE, "00000000000000000000000000000000", "tbllist", false},

	{ GID_SIMON1TALKIE_HB, "bc66e9c0b296e1b155a246917133f71a", "gamepc", false },
	{ GID_SIMON1TALKIE_HB, "22107c24dfb31b66ac503c28a6e20b19", "icon.dat", false},
	{ GID_SIMON1TALKIE_HB, "a34b2c8642f2e3676d7088b5c8b3e884", "simon.gme", false },
	{ GID_SIMON1TALKIE_HB, "9d31bef42db1a8abe4e9f368014df1d5", "stripped.txt", false},
	{ GID_SIMON1TALKIE_HB, "d198a80de2c59e4a0cd24b98814849e8", "tbllist", false},

	{ GID_SIMON1TALKIE_IT, "00000000000000000000000000000000", "gamepc", false },
	{ GID_SIMON1TALKIE_IT, "00000000000000000000000000000000", "icon.dat", false},
	{ GID_SIMON1TALKIE_IT, "00000000000000000000000000000000", "simon.gme", false },
	{ GID_SIMON1TALKIE_IT, "00000000000000000000000000000000", "stripped.txt", false},
	{ GID_SIMON1TALKIE_IT, "00000000000000000000000000000000", "tbllist", false},

	{ GID_SIMON1TALKIE_ES, "439f801ba52c02c9d1844600d1ce0f5e", "gamepc", false },
	{ GID_SIMON1TALKIE_ES, "22107c24dfb31b66ac503c28a6e20b19", "icon.dat", false},
	{ GID_SIMON1TALKIE_ES, "eff2774a73890b9eac533db90cd1afa1", "simon.gme", false },
	{ GID_SIMON1TALKIE_ES, "9d31bef42db1a8abe4e9f368014df1d5", "stripped.txt", false},
	{ GID_SIMON1TALKIE_ES, "d198a80de2c59e4a0cd24b98814849e8", "tbllist", false},

	{ GID_SIMON1WIN,       "c7c12fea7f6d0bfd22af5cdbc8166862", "gamepc", false },
	{ GID_SIMON1WIN,       "22107c24dfb31b66ac503c28a6e20b19", "icon.dat", false},
	{ GID_SIMON1WIN,       "b1b18d0731b64c0738c5cc4a2ee792fc", "simon.gme", false },
	{ GID_SIMON1WIN,       "a27e87a9ba21212d769804b3df47bfb2", "stripped.txt", false},
	{ GID_SIMON1WIN,       "d198a80de2c59e4a0cd24b98814849e8", "tbllist", false},

	{ GID_SIMON1WIN_DE,    "00000000000000000000000000000000", "gamepc", false },
	{ GID_SIMON1WIN_DE,    "00000000000000000000000000000000", "icon.dat", false},
	{ GID_SIMON1WIN_DE,    "00000000000000000000000000000000", "simon.gme", false },
	{ GID_SIMON1WIN_DE,    "00000000000000000000000000000000", "stripped.txt", false},
	{ GID_SIMON1WIN_DE,    "00000000000000000000000000000000", "tbllist", false},

	{ GID_SIMON2DOS,       "00000000000000000000000000000000", "game32", false },
	{ GID_SIMON2DOS,       "00000000000000000000000000000000", "icon.dat", false},
	{ GID_SIMON2DOS,       "00000000000000000000000000000000", "simon2.gme", false},
	{ GID_SIMON2DOS,       "00000000000000000000000000000000", "stripped.txt", false},
	{ GID_SIMON2DOS,       "00000000000000000000000000000000", "tbllist", false},

	{ GID_SIMON2DEMO,      "3794c15887539b8578bacab694ccf08a", "gsptr30", false },
	{ GID_SIMON2DEMO,      "72096a62d36e6034ea9fecc13b2dbdab", "icon.dat", false},
	{ GID_SIMON2DEMO,      "f8c9e6df1e55923a749e115ba74210c4", "simon2.gme", false},
	{ GID_SIMON2DEMO,      "e229f84d46fa83f99b4a7115679f3fb6", "stripped.txt", false},
	{ GID_SIMON2DEMO,      "a0d5a494b5d3d209d1a1d76cc8d76601", "tbllist", false},

	{ GID_SIMON2TALKIE,    "8c301fb9c4fcf119d2730ccd2a565eb3", "gsptr30", false },
	{ GID_SIMON2TALKIE,    "72096a62d36e6034ea9fecc13b2dbdab", "icon.dat", false},
	{ GID_SIMON2TALKIE,    "9c535d403966750ae98bdaf698375a38", "simon2.gme", false },
	{ GID_SIMON2TALKIE,    "e229f84d46fa83f99b4a7115679f3fb6", "stripped.txt", false},
	{ GID_SIMON2TALKIE,    "2082f8d02075e590300478853a91ffd9", "tbllist", false},

	{ GID_SIMON2TALKIE_FR, "43b3a04d2f0a0cbd1b024c814856561a", "gsptr30", false },
	{ GID_SIMON2TALKIE_FR, "72096a62d36e6034ea9fecc13b2dbdab", "icon.dat", false},
	{ GID_SIMON2TALKIE_FR, "8af0e02c0c3344db64dffc12196eb59d", "simon2.gme", false },
	{ GID_SIMON2TALKIE_FR, "5ea27977b4d7dcfd50eb5074e162ebbf", "stripped.txt", false},
	{ GID_SIMON2TALKIE_FR, "2082f8d02075e590300478853a91ffd9", "tbllist", false},

	{ GID_SIMON2TALKIE_DE, "00000000000000000000000000000000", "gsptr30", false },
	{ GID_SIMON2TALKIE_DE, "00000000000000000000000000000000", "icon.dat", false},
	{ GID_SIMON2TALKIE_DE, "00000000000000000000000000000000", "simon2.gme", false },
	{ GID_SIMON2TALKIE_DE, "00000000000000000000000000000000", "stripped.txt", false},
	{ GID_SIMON2TALKIE_DE, "00000000000000000000000000000000", "tbllist", false},

	{ GID_SIMON2TALKIE_HB, "952a2b1be23c3c609ba8d988a9a1627d", "gsptr30", false },
	{ GID_SIMON2TALKIE_HB, "72096a62d36e6034ea9fecc13b2dbdab", "icon.dat", false},
	{ GID_SIMON2TALKIE_HB, "a2b249a82ea182af09789eb95fb6c5be", "simon2.gme", false },
	{ GID_SIMON2TALKIE_HB, "de9dbc24158660e153483fa0cf6c3172", "stripped.txt", false},
	{ GID_SIMON2TALKIE_HB, "2082f8d02075e590300478853a91ffd9", "tbllist", false},

	{ GID_SIMON2TALKIE_IT, "00000000000000000000000000000000", "gsptr30", false },
	{ GID_SIMON2TALKIE_IT, "00000000000000000000000000000000", "icon.dat", false},
	{ GID_SIMON2TALKIE_IT, "00000000000000000000000000000000", "simon2.gme", false },
	{ GID_SIMON2TALKIE_IT, "00000000000000000000000000000000", "stripped.txt", false},
	{ GID_SIMON2TALKIE_IT, "00000000000000000000000000000000", "tbllist", false},

	{ GID_SIMON2TALKIE_ES, "00000000000000000000000000000000", "gsptr30", false },
	{ GID_SIMON2TALKIE_ES, "00000000000000000000000000000000", "icon.dat", false},
	{ GID_SIMON2TALKIE_ES, "00000000000000000000000000000000", "simon2.gme", false },
	{ GID_SIMON2TALKIE_ES, "00000000000000000000000000000000", "stripped.txt", false},
	{ GID_SIMON2TALKIE_ES, "00000000000000000000000000000000", "tbllist", false},

	{ GID_SIMON2WIN,       "608e277904d87dd28725fa08eacc2c0d", "gsptr30", false },
	{ GID_SIMON2WIN,       "72096a62d36e6034ea9fecc13b2dbdab", "icon.dat", false},
	{ GID_SIMON2WIN,       "e749c4c103d7e7d51b34620ed76c5a04", "simon2.gme", false },
	{ GID_SIMON2WIN,       "e229f84d46fa83f99b4a7115679f3fb6", "stripped.txt", false},
	{ GID_SIMON2WIN,       "2082f8d02075e590300478853a91ffd9", "tbllist", false},

	{ GID_SIMON2WIN_DE,    "00000000000000000000000000000000", "gsptr30", false },
	{ GID_SIMON2WIN_DE,    "00000000000000000000000000000000", "icon.dat", false},
	{ GID_SIMON2WIN_DE,    "00000000000000000000000000000000", "simon2.gme", false },
	{ GID_SIMON2WIN_DE,    "00000000000000000000000000000000", "stripped.txt", false},
	{ GID_SIMON2WIN_DE,    "00000000000000000000000000000000", "tbllist", false},
};

// Simon the Sorcerer 1
static GameFileDescription SIMON1CD32_GameFiles[] = {
	{"gameamiga", GAME_BASEFILE},
	{"icon.pkd", GAME_ICONFILE},
	{"stripped.txt", GAME_STRFILE},
	{"tbllist", GAME_TBLFILE},
};

static GameFileDescription SIMON1ACORN_GameFiles[] = {
	{"data", GAME_GMEFILE},
	{"gamebase", GAME_BASEFILE},
	{"icondata", GAME_ICONFILE},
	{"stripped", GAME_STRFILE},
	{"tbllist", GAME_TBLFILE},
};

static GameFileDescription SIMON1DEMO_GameFiles[] = {
	{"gdemo", GAME_BASEFILE},
	{"icon.dat", GAME_ICONFILE},
	{"stripped.txt", GAME_STRFILE},
	{"tbllist", GAME_TBLFILE},
};

static GameFileDescription SIMON1DOS_GameFiles[] = {
	{"gamepc", GAME_BASEFILE},
	{"icon.dat", GAME_ICONFILE},
	{"stripped.txt", GAME_STRFILE},
	{"tbllist", GAME_TBLFILE},
};

static GameFileDescription SIMON1_GameFiles[] = {
	{"gamepc", GAME_BASEFILE},
	{"icon.dat", GAME_ICONFILE},
	{"simon.gme", GAME_GMEFILE},
	{"stripped.txt", GAME_STRFILE},
	{"tbllist", GAME_TBLFILE},
};

// Simon the Sorcerer 2
static GameFileDescription SIMON2DOS_GameFiles[] = {
	{"game32", GAME_BASEFILE},
	{"icon.dat", GAME_ICONFILE},
	{"simon2.gme", GAME_GMEFILE},
	{"stripped.txt", GAME_STRFILE},
	{"tbllist", GAME_TBLFILE},
};

static GameFileDescription SIMON2_GameFiles[] = {
	{"gsptr30", GAME_BASEFILE},
	{"icon.dat", GAME_ICONFILE},
	{"simon2.gme", GAME_GMEFILE},
	{"stripped.txt", GAME_STRFILE},
	{"tbllist", GAME_TBLFILE},
};

static GameDescription gameDescriptions[] = {
	// Simon the Sorcerer 1 - English Acorn CD Demo
	{
		"simon1",
		GType_SIMON1,
		GID_SIMON1ACORNDEMO,
		"Simon the Sorcerer 1 (Acorn CD Demo)",
		ARRAYSIZE(SIMON1ACORN_GameFiles),
		SIMON1ACORN_GameFiles,
		GF_TALKIE,
		Common::EN_USA,
		Common::kPlatformAcorn,
	},

	// Simon the Sorcerer 1 - English Acorn CD
	{
		"simon1",
		GType_SIMON1,
		GID_SIMON1ACORN,
		"Simon the Sorcerer 1 (Acorn CD)",
		ARRAYSIZE(SIMON1ACORN_GameFiles),
		SIMON1ACORN_GameFiles,
		GF_TALKIE,
		Common::EN_USA,
		Common::kPlatformAcorn,
	},

	// Simon the Sorcerer 1 - English Amiga CD32
	{
		"simon1",
		GType_SIMON1,
		GID_SIMON1CD32,
		"Simon the Sorcerer 1 (Amiga CD32)",
		ARRAYSIZE(SIMON1CD32_GameFiles),
		SIMON1CD32_GameFiles,
		GF_TALKIE | GF_OLD_BUNDLE,
		Common::EN_USA,
		Common::kPlatformAmiga,
	},

	// Simon the Sorcerer 1 - English DOS Floppy Demo
	{
		"simon1",
		GType_SIMON1,
		GID_SIMON1DEMO,
		"Simon the Sorcerer 1 (DOS Floppy Demo)",
		ARRAYSIZE(SIMON1DEMO_GameFiles),
		SIMON1DEMO_GameFiles,
		GF_OLD_BUNDLE,
		Common::EN_USA,
		Common::kPlatformPC,
	},

	// Simon the Sorcerer 1 - English DOS Floppy (Infocom)
	{
		"simon1",
		GType_SIMON1,
		GID_SIMON1DOS,
		"Simon the Sorcerer 1 (DOS Floppy)",
		ARRAYSIZE(SIMON1DOS_GameFiles),
		SIMON1DOS_GameFiles,
		GF_OLD_BUNDLE,
		Common::EN_USA,
		Common::kPlatformPC,
	},

	// Simon the Sorcerer 1 - English DOS CD
	{
		"simon1",
		GType_SIMON1,
		GID_SIMON1TALKIE,
		"Simon the Sorcerer 1 (DOS CD)",
		ARRAYSIZE(SIMON1_GameFiles),
		SIMON1_GameFiles,
		GF_TALKIE,
		Common::EN_USA,
		Common::kPlatformPC,
	},

	// Simon the Sorcerer 1 - French DOS CD
	{
		"simon1",
		GType_SIMON1,
		GID_SIMON1TALKIE_FR,
		"Simon the Sorcerer 1 (Fr DOS CD)",
		ARRAYSIZE(SIMON1_GameFiles),
		SIMON1_GameFiles,
		GF_TALKIE,
		Common::FR_FRA,
		Common::kPlatformPC,
	},

	// Simon the Sorcerer 1 - German DOS CD
	{
		"simon1",
		GType_SIMON1,
		GID_SIMON1TALKIE_DE,
		"Simon the Sorcerer 1 (De DOS CD)",
		ARRAYSIZE(SIMON1_GameFiles),
		SIMON1_GameFiles,
		GF_TALKIE,
		Common::DE_DEU,
		Common::kPlatformPC,
	},

	// Simon the Sorcerer 1 - Hebrew DOS CD
	{
		"simon1",
		GType_SIMON1,
		GID_SIMON1TALKIE_HB,
		"Simon the Sorcerer 1 (Hb DOS CD)",
		ARRAYSIZE(SIMON1_GameFiles),
		SIMON1_GameFiles,
		GF_TALKIE,
		Common::HB_ISR,
		Common::kPlatformPC,
	},

	// Simon the Sorcerer 1 - Italian DOS CD
	{
		"simon1",
		GType_SIMON1,
		GID_SIMON1TALKIE_IT,
		"Simon the Sorcerer 1 (It DOS CD)",
		ARRAYSIZE(SIMON1_GameFiles),
		SIMON1_GameFiles,
		GF_TALKIE,
		Common::IT_ITA,
		Common::kPlatformPC,
	},

	// Simon the Sorcerer 1 - Spanish DOS CD
	{
		"simon1",
		GType_SIMON1,
		GID_SIMON1TALKIE_ES,
		"Simon the Sorcerer 1 (Sp DOS CD)",
		ARRAYSIZE(SIMON1_GameFiles),
		SIMON1_GameFiles,
		GF_TALKIE,
		Common::ES_ESP,
		Common::kPlatformPC,
	},

	// Simon the Sorcerer 1 - English Windows CD
	{
		"simon1",
		GType_SIMON1,
		GID_SIMON1WIN,
		"Simon the Sorcerer 1 (Windows CD)",
		ARRAYSIZE(SIMON1_GameFiles),
		SIMON1_GameFiles,
		GF_TALKIE,
		Common::EN_USA,
		Common::kPlatformWindows,
	},

	// Simon the Sorcerer 1 - German Windows CD
	{
		"simon1",
		GType_SIMON1,
		GID_SIMON1WIN_DE,
		"Simon the Sorcerer 1 (De Windows CD)",
		ARRAYSIZE(SIMON1_GameFiles),
		SIMON1_GameFiles,
		GF_TALKIE,
		Common::DE_DEU,
		Common::kPlatformWindows,
	},

	// Simon the Sorcerer 2 - English DOS Floppy
	{
		"simon2",
		GType_SIMON2,
		GID_SIMON2DOS,
		"Simon the Sorcerer 2 (DOS Floppy)",
		ARRAYSIZE(SIMON2DOS_GameFiles),
		SIMON2DOS_GameFiles,
		0,
		Common::EN_USA,
		Common::kPlatformPC,
	},

	// Simon the Sorcerer 2 - English DOS CD Demo
	{
		"simon2",
		GType_SIMON2,
		GID_SIMON2DEMO,
		"Simon the Sorcerer 2 (DOS CD Demo)",
		ARRAYSIZE(SIMON2_GameFiles),
		SIMON2_GameFiles,
		GF_TALKIE,
		Common::EN_USA,
		Common::kPlatformPC,
	},

	// Simon the Sorcerer 2 - English DOS CD
	{
		"simon2",
		GType_SIMON2,
		GID_SIMON2TALKIE,
		"Simon the Sorcerer 2 (DOS CD)",
		ARRAYSIZE(SIMON2_GameFiles),
		SIMON2_GameFiles,
		GF_TALKIE,
		Common::EN_USA,
		Common::kPlatformPC,
	},

	// Simon the Sorcerer 2 - French DOS CD
	{
		"simon2",
		GType_SIMON2,
		GID_SIMON2TALKIE_FR,
		"Simon the Sorcerer 2 (Fr DOS CD)",
		ARRAYSIZE(SIMON2_GameFiles),
		SIMON2_GameFiles,
		GF_TALKIE,
		Common::FR_FRA,
		Common::kPlatformPC,
	},

	// Simon the Sorcerer 2 - German DOS CD
	{
		"simon2",
		GType_SIMON2,
		GID_SIMON2TALKIE_DE,
		"Simon the Sorcerer 2 (De DOS CD)",
		ARRAYSIZE(SIMON2_GameFiles),
		SIMON2_GameFiles,
		GF_TALKIE,
		Common::DE_DEU,
		Common::kPlatformPC,
	},

	// Simon the Sorcerer 2 - Hebrew DOS CD
	{
		"simon2",
		GType_SIMON2,
		GID_SIMON2TALKIE_HB,
		"Simon the Sorcerer 2 (Hb DOS CD)",
		ARRAYSIZE(SIMON2_GameFiles),
		SIMON2_GameFiles,
		GF_TALKIE,
		Common::HB_ISR,
		Common::kPlatformPC,
	},

	// Simon the Sorcerer 2 - Italian DOS CD
	{
		"simon2",
		GType_SIMON2,
		GID_SIMON2TALKIE_IT,
		"Simon the Sorcerer 2 (It DOS CD)",
		ARRAYSIZE(SIMON2_GameFiles),
		SIMON2_GameFiles,
		GF_TALKIE,
		Common::IT_ITA,
		Common::kPlatformPC,
	},

	// Simon the Sorcerer 2 - Spanish DOS CD
	{
		"simon2",
		GType_SIMON2,
		GID_SIMON2TALKIE_ES,
		"Simon the Sorcerer 2 (Sp DOS CD)",
		ARRAYSIZE(SIMON2_GameFiles),
		SIMON2_GameFiles,
		GF_TALKIE,
		Common::ES_ESP,
		Common::kPlatformPC,
	},

	// Simon the Sorcerer 2 - English Windows CD
	{
		"simon2",
		GType_SIMON2,
		GID_SIMON2WIN,
		"Simon the Sorcerer 2 (Windows CD)",
		ARRAYSIZE(SIMON2_GameFiles),
		SIMON2_GameFiles,
		GF_TALKIE,
		Common::EN_USA,
		Common::kPlatformWindows,
	},

	// Simon the Sorcerer 2 - German Windows CD
	{
		"simon2",
		GType_SIMON2,
		GID_SIMON2WIN_DE,
		"Simon the Sorcerer 2 (De Windows CD)",
		ARRAYSIZE(SIMON2_GameFiles),
		SIMON2_GameFiles,
		GF_TALKIE,
		Common::DE_DEU,
		Common::kPlatformWindows,
	},
};

bool SimonEngine::initGame(void) {
	int gameNumber;
	FSList dummy;

	if ((gameNumber = detectGame(dummy)) == -1) {
		warning("No valid games were found in the specified directory.");
		return false;
	}

	debug(0, "Running %s", gameDescriptions[gameNumber].title);

	_gameDescription = &gameDescriptions[gameNumber];

	return true;
}

DetectedGameList GAME_ProbeGame(const FSList &fslist, int **retmatches) {
	DetectedGameList detectedGames;
	int game_n;
	int index = 0, i, j;
	int matches[ARRAYSIZE(gameDescriptions)];
	bool mode = retmatches ? false : true;

	game_n = -1;
	for (i = 0; i < ARRAYSIZE(gameDescriptions); i++)
		matches[i] = -1;

	while (1) {
		game_n = detectGame(fslist, mode, game_n);
		if (game_n == -1)
			break;
		matches[index++] = game_n;
	}

	// We have some resource sets which are superpositions of other
	// Particularly it is ite-demo-linux vs ite-demo-win
	// Now remove lesser set if bigger matches too

	if (index > 1) {
		// Search max number
		int maxcount = 0;
		for (i = 0; i < index; i++) {
			int count = 0;
			for (j = 0; j < ARRAYSIZE(gameMD5); j++)
				if (gameMD5[j].id == gameDescriptions[matches[i]].gameId)
					count++;
			maxcount = MAX(maxcount, count);
		}

		// Now purge targets with number of files lesser than max
		for (i = 0; i < index; i++) {
			int count = 0;
			for (j = 0; j < ARRAYSIZE(gameMD5); j++)
				if (gameMD5[j].id == gameDescriptions[matches[i]].gameId)
					count++;
			if (count < maxcount) {
				debug(2, "Purged: %s", gameDescriptions[matches[i]].title);
				matches[i] = -1;
			}
		}

	}

	// and now push them into list of detected games
	for (i = 0; i < index; i++)
		if (matches[i] != -1)
			detectedGames.push_back(DetectedGame(gameDescriptions[matches[i]].toGameSettings(),
							 gameDescriptions[matches[i]].language,
							 gameDescriptions[matches[i]].platform));
		
	if (retmatches) {
		*retmatches = (int *)calloc(ARRAYSIZE(gameDescriptions), sizeof(int));
		for (i = 0; i < ARRAYSIZE(gameDescriptions); i++)
			(*retmatches)[i] = matches[i];
	}

	return detectedGames;
}

int detectGame(const FSList &fslist, bool mode, int start) {
	int game_count = ARRAYSIZE(gameDescriptions);
	int game_n = -1;
	typedef Common::Map<Common::String, Common::String> StringMap;
	StringMap filesMD5;

	typedef Common::Map<Common::String, bool> StringSet;
	StringSet filesList;

	uint16 file_count;
	uint16 file_n;
	Common::File test_file;
	bool file_missing;

	Common::String tstr, tstr1;
	char md5str[32+1];
	uint8 md5sum[16];

	// First we compose list of files which we need MD5s for
	for (int i = 0; i < ARRAYSIZE(gameMD5); i++) {
		tstr = Common::String(gameMD5[i].filename);
		tstr.toLowercase();

		if (gameMD5[i].caseSensitive && !mode)
			filesList[Common::String(gameMD5[i].filename)] = true;
		else
			filesList[tstr] = true;
	}

	if (mode) {
		// Now count MD5s for required files
		for (FSList::const_iterator file = fslist.begin(); file != fslist.end(); ++file) {
			if (!file->isDirectory()) {
				tstr = file->displayName();
				// FIXME: there is a bug in String class. tstr1 = tstr; tstr.toLowercase()
				// makes tstr1 lowercase as well
				tstr1 = Common::String(file->displayName().c_str());
				tstr.toLowercase();

				if (filesList.contains(tstr) || filesList.contains(tstr1)) {
					if (Common::md5_file(file->path().c_str(), md5sum, NULL, FILE_MD5_BYTES)) {
						for (int j = 0; j < 16; j++) {
							sprintf(md5str + j*2, "%02x", (int)md5sum[j]);
						}
						filesMD5[tstr] = Common::String(md5str);
						filesMD5[tstr1] = Common::String(md5str);
					}
				}
			}
		}
	} else {
		Common::File testFile;

		for (StringSet::const_iterator file = filesList.begin(); file != filesList.end(); ++file) {
			if (testFile.open(file->_key.c_str())) {
				testFile.close();
				if (Common::md5_file(file->_key.c_str(), md5sum, NULL, FILE_MD5_BYTES)) {
					for (int j = 0; j < 16; j++) {
						sprintf(md5str + j*2, "%02x", (int)md5sum[j]);
					}
					filesMD5[file->_key] = Common::String(md5str);
				}
			}
		}
	}

	for (game_n = start + 1; game_n < game_count; game_n++) {
		file_count = gameDescriptions[game_n].filesCount;
		file_missing = false;

		// Try to open all files for this game
		for (file_n = 0; file_n < file_count; file_n++) {
			tstr = gameDescriptions[game_n].filesDescriptions[file_n].fileName;

			if (!filesMD5.contains(tstr)) {
				file_missing = true;
				break;
			}
		}

		// Try the next game, couldn't find all files for the current
		// game
		if (file_missing) {
			continue;
		} else {
			bool match = true;

			debug(0, "Probing game: %s", gameDescriptions[game_n].title);

			for (int i = 0; i < ARRAYSIZE(gameMD5); i++) {
				if (gameMD5[i].id == gameDescriptions[game_n].gameId) {
					tstr = gameMD5[i].filename;

					if (strcmp(gameMD5[i].md5, filesMD5[tstr].c_str())) {
						match = false;
						break;
					}
				}
			}
			if (!match)
				continue;

			debug(0, "Found game: %s", gameDescriptions[game_n].title);

			return game_n;
		}
	}

	if (!filesMD5.isEmpty() && start == -1) {
		printf("MD5s of your game version are unknown. Please, report following data to\n");
		printf("ScummVM team along with your game name and version:\n");

		for (StringMap::const_iterator file = filesMD5.begin(); file != filesMD5.end(); ++file)
			printf("%s: %s\n", file->_key.c_str(), file->_value.c_str());
	}

	return -1;
}

} // End of namespace Simon
