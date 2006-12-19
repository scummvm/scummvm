/* ScummVM - Scumm Interpreter
 * Copyright (C) 2006 The ScummVM project
 *
 * cinE Engine is (C) 2004-2005 by CinE Team
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
 * $URL:$
 * $Id:$
 *
 */

#include "common/stdafx.h"

#include "base/plugins.h"

#include "common/advancedDetector.h"
#include "common/file.h"

#include "agi/agi.h"


namespace Agi {
static DetectedGameList GAME_detectGames(const FSList &fslist);
}

using Common::File;

static const PlainGameDescriptor agiGames[] = {
	{"agi", "Sierra AGI Engine" },

	{0, 0}
};

ADVANCED_DETECTOR_DEFINE_PLUGIN(AGI, Agi::AgiEngine, Agi::GAME_detectGames, agiGames, 0);

REGISTER_PLUGIN(AGI, "AGI v2 + v3 Engine", "Sierra AGI Engine (C) Sierra On-Line Software");


namespace Agi {

#define FILE_MD5_BYTES 5000

using Common::ADGameFileDescription;
using Common::ADGameDescription;

using Common::File;

// Kings Quest I entries
static const ADGameFileDescription AGI_KQ1_PC_20F_GameFiles[] = {
	{ "logdir",		0,	"10ad66e2ecbd66951534a50aedcd0128"},
};
static const ADGameFileDescription AGI_KQ1_2GS_10S_GameFiles[] = {
	{ "logdir",		0,	"f4277aa34b43d37382bc424c81627617"},
};
static const ADGameFileDescription AGI_KQ1_MAC_20C_GameFiles[] = {
	{ "logdir",		0,	"d4c4739d4ac63f7dbd29255425077d48"},
};

// Kings Quest II Entries
static const ADGameFileDescription AGI_KQ2_PC_22_GameFiles[] = {
	{ "logdir",		0,	"b944c4ff18fb8867362dc21cc688a283"},
};
static const ADGameFileDescription AGI_KQ2_AMIGA_20F_GameFiles[] = {
	{ "logdir",		0,	"b866f0fab2fad91433a637a828cfa410"},
};
static const ADGameFileDescription AGI_KQ2_MAC_20R_GameFiles[] = {
	{ "logdir",		0,	"cbdb0083317c8e7cfb7ac35da4bc7fdc"},
};


static const AGIGameDescription gameDescriptions[] = {
	{
		// kings quest 1 : pc : 2.0F
		{
			"agi",
			"King's Quest 1 (PC 5.25\"/3.5\") 2.0F [AGI 2.917]",
			ARRAYSIZE(AGI_KQ1_PC_20F_GameFiles),
			AGI_KQ1_PC_20F_GameFiles,
			Common::EN_ANY,
			Common::kPlatformPC,
		},
		GType_V2,
		0,
		0x2917,
	},

	{
		// kings quest 1 : 2gs : 1.0S
		{
			"agi",
			"King's Quest 1 (IIgs) 1.0S-88223",
			ARRAYSIZE(AGI_KQ1_2GS_10S_GameFiles),
			AGI_KQ1_2GS_10S_GameFiles,
			Common::EN_ANY,
			Common::kPlatformApple2GS,
		},
		GType_V2,
		0,
		0x2272,
	},


	{
		// kings quest 1 : Mac : 2.0C
		{
			"agi",
			"King's Quest 1 (Mac) 2.0C",
			ARRAYSIZE(AGI_KQ1_MAC_20C_GameFiles),
			AGI_KQ1_MAC_20C_GameFiles,
			Common::EN_ANY,
			Common::kPlatformMacintosh,
		},
		GType_V2,
		0,
		0x2440,
	},


	{
		// kings quest 2 : PC : 2.2
		{
			"agi",
			"King's Quest 2 (PC 5.25\"/3.5\") 2.2 [AGI 2.426]",
			ARRAYSIZE(AGI_KQ2_PC_22_GameFiles),
			AGI_KQ2_PC_22_GameFiles,
			Common::EN_ANY,
			Common::kPlatformPC,
		},
		GType_V2,
		0,
		0x2917,
	},

	{
		// kings quest 2 : Amiga : 2.0J (not sure if this is broken version or not. need to test
		{
			"agi",
			"King's Quest 2 (Amiga) 2.0J",
			ARRAYSIZE(AGI_KQ2_AMIGA_20F_GameFiles),
			AGI_KQ2_AMIGA_20F_GameFiles,
			Common::EN_ANY,
			Common::kPlatformAmiga,
		},
		GType_V2,
		AGI_AMIGA,
		0x2440,
	},

	{
		// kings quest 2 : Mac : 2.0R
		{
			"agi",
			"King's Quest 2 (Mac) 2.0R",
			ARRAYSIZE(AGI_KQ2_MAC_20R_GameFiles),
			AGI_KQ2_MAC_20R_GameFiles,
			Common::EN_ANY,
			Common::kPlatformMacintosh,
		},
		GType_V2,
		0,
		0x2440,
	},


};

bool AgiEngine::initGame() {
	int i = Common::real_ADVANCED_DETECTOR_DETECT_INIT_GAME(
		(const byte *)gameDescriptions,
		sizeof(AGIGameDescription),
		ARRAYSIZE(gameDescriptions),
		FILE_MD5_BYTES,
		agiGames
		);
	_gameDescription = &gameDescriptions[i];
	return true;
}

DetectedGameList GAME_detectGames(const FSList &fslist) {
	return real_ADVANCED_DETECTOR_DETECT_GAMES_FUNCTION(
		fslist,
		(const byte *)gameDescriptions,
		sizeof(AGIGameDescription),
		ARRAYSIZE(gameDescriptions),
		FILE_MD5_BYTES,
		agiGames
	);
}

} // End of namespace Agi

