/* ScummVM - Scumm Interpreter
 * Copyright (C) 2006 The ScummVM project
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

#include "common/stdafx.h"

#include "base/plugins.h"

#include "common/advancedDetector.h"
#include "common/file.h"

#include "parallaction/parallaction.h"

namespace Parallaction {
static DetectedGameList GAME_detectGames(const FSList &fslist);
}

static const PlainGameDescriptor parallactionGames[] = {
	{"nippon", "Nippon Safes Inc."},
	{0, 0}
};

ADVANCED_DETECTOR_DEFINE_PLUGIN(PARALLACTION, Parallaction::Parallaction, Parallaction::GAME_detectGames, parallactionGames, 0);

REGISTER_PLUGIN(PARALLACTION, "Parallaction engine", "Nippon Safes Inc. (C) Dynabyte");


namespace Parallaction {

#define FILE_MD5_BYTES 5000

static const PARALLACTIONGameDescription gameDescriptions[] = {
	{
		{
			"nippon",
			"",
			{
				{ "disk1",	0, "610363308258e926dbabd5a9e7bb769f"},
				{ "disk2",	0, "bfdd7bcfbc226f4acf3f67fa9efa2826"},
				{ "disk3",	0, "eec08180240888d76e3cfe3e183d5d5d"},
				{ "disk4",	0, "5bffddc7db226bdaa7dd3e10e5a15e68"},
				{ "en", 	0, "65cbfa81eafe308621184796ed116700"},
				{ "fr", 	0, "ac20c743ea10f2cb4491f76c5644582c"},
				{ "ge", 	0, "50916bfa34aee1380e0e959b37eceb5a"},
				{ "it", 	0, "89964aef04d2c53a615ee8983caf2775"},
				{ NULL, 0, NULL}
			},
			Common::EN_ANY,
			Common::kPlatformPC,
		},
		GType_Nippon,
		0,
	},
	{ { NULL, NULL, { { NULL, 0, NULL } }, Common::UNK_LANG, Common::kPlatformUnknown }, 0, 0 }
};


bool Parallaction::detectGame() {
	int i = Common::ADVANCED_DETECTOR_DETECT_INIT_GAME(
		(const byte *)gameDescriptions,
		sizeof(PARALLACTIONGameDescription),
		FILE_MD5_BYTES,
		parallactionGames
		);
	_gameDescription = &gameDescriptions[i];
	return true;
}

DetectedGameList GAME_detectGames(const FSList &fslist) {
	return Common::ADVANCED_DETECTOR_DETECT_GAMES_FUNCTION(
		fslist,
		(const byte *)gameDescriptions,
		sizeof(PARALLACTIONGameDescription),
		FILE_MD5_BYTES,
		parallactionGames
	);
}

} // End of namespace Parallaction
