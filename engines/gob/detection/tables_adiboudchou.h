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
 *
 * This file is dual-licensed.
 * In addition to the GPLv3 license mentioned above, this code is also
 * licensed under LGPL 2.1. See LICENSES/COPYING.LGPL file for the
 * full text of the license.
 *
 */

/* Detection tables for Adiboud'chou / Addy Buschu series. */
/* This Game uses the DEV7 Engine, more Information can be found here: https://wiki.scummvm.org/index.php?title=DEV7_Information */
/* These games are part of the Adibou series. For more information, refer to our wiki: https://wiki.scummvm.org/index.php?title=Adibou_Games */

#ifndef GOB_DETECTION_TABLES_ADIBOUDCHOU_H
#define GOB_DETECTION_TABLES_ADIBOUDCHOU_H

// -- French: Adiboud'chou series --

{
	{
		"adiboudchoumer",
		MetaEngineDetection::GAME_NOT_IMPLEMENTED, // Adiboud'chou a la mer 1.01 (Engine: DEV7 version unknown)
		AD_ENTRY1s("adbc_envir_obc.stk", "57f0eda5d4029abdb2f6b6201e02905e", 3204281),
		FR_FRA,
		kPlatformWindows,
		ADGF_UNSUPPORTED,
		GUIO0()
	},
	kFeatures800x600,
	"adbc_envir_obc.stk", "adbc_init.obc", 0
},

// -- German: Addy Buschu series --

{
	{ // Supplied by BJNFNE
		"adiboudchoumer",
		MetaEngineDetection::GAME_NOT_IMPLEMENTED, // Addy Buschu am Meer 1.01 (Engine: DEV7 version unknown)
		AD_ENTRY1s("adbc_envir_obc.stk", "46b7db9f7e77a077d9ac8506130ba9a2", 2830950),
		DE_DEU,
		kPlatformWindows,
		ADGF_UNSUPPORTED,
		GUIO0()
	},
	kFeatures800x600,
	"adbc_envir_obc.stk", "adbc_init.obc", 0
},
{
	{ // Supplied by BJNFNE

		"adiboudchoubanquise",
		MetaEngineDetection::GAME_NOT_IMPLEMENTED, // Addy Buschu Schnee & Eis 1.00 (Engine: DEV7 version 1.0.0.0)
		AD_ENTRY1s("adbc_envir_obc.stk", "fde006186b93b4f33486f021826f88a0", 5199806),
		DE_DEU,
		kPlatformWindows,
		ADGF_UNSUPPORTED,
		GUIO0()
	},
	kFeatures800x600,
	"adbc_envir_obc.stk", "adbc_init.obc", 0
},
{
	{ // Supplied by BJNFNE
		"adiboudchoucampagne",
		MetaEngineDetection::GAME_NOT_IMPLEMENTED, // Addy Buschu auf dem Land 1.00 (Engine: DEV7 version unknown)
		AD_ENTRY1s("adbc_envir_obc.stk", "4b43d3d1a8bc908d80e729069c5bb59f", 2831471),
		DE_DEU,
		kPlatformWindows,
		ADGF_UNSUPPORTED,
		GUIO0()
	},
	kFeatures800x600,
	"adbc_envir_obc.stk", "adbc_init.obc", 0
},
{
	{ // Supplied by BJNFNE
		"adiboudchoujunglesavane",
		MetaEngineDetection::GAME_NOT_IMPLEMENTED, // Addy Buschu Die bunte Tierwelt 1.00 (Engine: DEV7 version 1.0.0.0)
		AD_ENTRY1s("adbc_envir_obc.stk", "7f33561f295030cbe64a21f941ef1efc", 3188852),
		DE_DEU,
		kPlatformWindows,
		ADGF_UNSUPPORTED,
		GUIO0()
	},
	kFeatures800x600,
	"adbc_envir_obc.stk", "adbc_init.obc", 0
},

// -- Russian: Антошка

{
	{
		"adiboudchoucampagne",
		MetaEngineDetection::GAME_NOT_IMPLEMENTED, // Антошка. В гостях у друзей 1.00 (Engine: DEV7 version 1.0.0.0)
		AD_ENTRY1s("adbc_envir_obc.stk", "1b65643b2aa794f38f3b18efb9e68b92", 3210008),
		RU_RUS,
		kPlatformWindows,
		ADGF_UNSUPPORTED,
		GUIO0()
	},
	kFeatures800x600,
	"adbc_envir_obc.stk", "adbc_init.obc", 0
},

#endif // GOB_DETECTION_TABLES_ADIBOUDCHOU_H
