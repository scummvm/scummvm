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

namespace AGOS {

static const AGOSGameDescription fallbackDescs[] = {

	// Simon the Sorcerer 1 - DOS Floppy
	{
		{
			"simon1",
			"Floppy",
			{
				{ "gamepc",			GAME_BASEFILE,	NULL, -1},
				{ "icon.dat",		GAME_ICONFILE,	NULL, -1},
				{ "stripped.txt",	GAME_STRFILE,	NULL, -1},
				{ "tbllist",		GAME_TBLFILE,	NULL, -1},
				AD_LISTEND
			},
			Common::UNK_LANG,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO4(GUIO_NOSPEECH, GAMEOPTION_OPL3_MODE, GAMEOPTION_DOS_TEMPOS, GAMEOPTION_DISABLE_FADE_EFFECTS)
		},

		GType_SIMON1,
		GID_SIMON1DOS,
		GF_OLD_BUNDLE
	},

	// Simon the Sorcerer 1 - DOS CD
	{
		{
			"simon1",
			"CD",
			{
				{ "simon.gme",		GAME_GMEFILE,	NULL, -1},
				{ "gamepc"	,		GAME_BASEFILE,	NULL, -1},
				{ "icon.dat",		GAME_ICONFILE,	NULL, -1},
				{ "stripped.txt",	GAME_STRFILE,	NULL, -1},
				{ "tbllist",		GAME_TBLFILE,	NULL, -1},
				AD_LISTEND
			},
			Common::UNK_LANG,
			Common::kPlatformDOS,
			ADGF_CD,
			GUIO3(GAMEOPTION_OPL3_MODE, GAMEOPTION_DOS_TEMPOS, GAMEOPTION_DISABLE_FADE_EFFECTS)
		},

		GType_SIMON1,
		GID_SIMON1,
		GF_TALKIE
	},

	// Simon the Sorcerer 2 - DOS Floppy
	{
		{
			"simon2",
			"Floppy",
			{
				{ "game32",			GAME_BASEFILE,	NULL, -1},
				{ "icon.dat",		GAME_ICONFILE,	NULL, -1},
				{ "simon2.gme",		GAME_GMEFILE,	NULL, -1},
				{ "stripped.txt",	GAME_STRFILE,	NULL, -1},
				{ "tbllist",		GAME_TBLFILE,	NULL, -1},
				AD_LISTEND
			},
			Common::UNK_LANG,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO2(GUIO_NOSPEECH, GAMEOPTION_DISABLE_FADE_EFFECTS)
		},

		GType_SIMON2,
		GID_SIMON2,
		0
	},

	// Simon the Sorcerer 2 - DOS CD
	{
		{
			"simon2",
			"CD",
			{
				{ "gsptr30",		GAME_BASEFILE,	NULL, -1},
				{ "icon.dat",		GAME_ICONFILE,	NULL, -1},
				{ "simon2.gme",		GAME_GMEFILE,	NULL, -1},
				{ "stripped.txt",	GAME_STRFILE,	NULL, -1},
				{ "tbllist",		GAME_TBLFILE,	NULL, -1},
				AD_LISTEND
			},
			Common::UNK_LANG,
			Common::kPlatformDOS,
			ADGF_CD,
			GUIO1(GAMEOPTION_DISABLE_FADE_EFFECTS)
		},

		GType_SIMON2,
		GID_SIMON2,
		GF_TALKIE
	},

	// The Feeble Files - Windows
	{
		{
			"feeble",
			"CD",
			{
				{ "game22",		GAME_BASEFILE,	NULL, -1},
				{ "tbllist",	GAME_TBLFILE,	NULL, -1},
				AD_LISTEND
			},
			Common::UNK_LANG,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO3(GUIO_NOSUBTITLES, GUIO_NOMUSIC, GUIO_NOASPECT)
		},

		GType_FF,
		GID_FEEBLEFILES,
		GF_OLD_BUNDLE | GF_TALKIE
	},

	{ AD_TABLE_END_MARKER, 0, 0, 0 }
};

static const ADFileBasedFallback fileBased[] = {
	{ &fallbackDescs[ 0].desc, { "gamepc", "icon.dat", "stripped.txt", "tbllist", 0 } },
	{ &fallbackDescs[ 1].desc, { "simon.gme", "gamepc", "icon.dat", "stripped.txt", "tbllist", 0 } },
	{ &fallbackDescs[ 2].desc, { "game32", "icon.dat", "simon2.gme", "stripped.txt", "tbllist", 0 } },
	{ &fallbackDescs[ 3].desc, { "gsptr30", "icon.dat", "simon2.gme", "stripped.txt", "tbllist", 0 } },
	{ &fallbackDescs[ 4].desc, { "game22", "tbllist", 0 } },

	{ 0, { 0 } }
};

} // End of namespace AGOS
