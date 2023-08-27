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

/* Detection tables for Adiboud'chou / Addy Buschu series. */

#ifndef GOB_DETECTION_TABLES_ADIBOUDCHOU_H
#define GOB_DETECTION_TABLES_ADIBOUDCHOU_H

// -- French: Adiboud'chou series --

// -- German: Addy Buschu series --

{
	{
		"adiboudchoumer",
		_s("Missing game code"), // Addy Buschu am Meer 1.01 (Engine: DEV7 version unknown)
		AD_ENTRY1s("adbc_envir_obc.stk", "46b7db9f7e77a077d9ac8506130ba9a2", 2830950),
		DE_DEU,
		kPlatformWindows,
		ADGF_UNSUPPORTED,
		GUIO0()
	},
	kGameTypeNone,
	kFeatures640x480,
	0, 0, 0
},

{
	{
		"adiboudchoubanquise",
		_s("Missing game code"), // Addy Buschu Schnee & Eis 1.00 (Engine: DEV7 version 1.0.0.0)
		AD_ENTRY1s("adbc_envir_obc.stk", "fde006186b93b4f33486f021826f88a0", 5199806),
		DE_DEU,
		kPlatformWindows,
		ADGF_UNSUPPORTED,
		GUIO0()
	},
	kGameTypeNone,
	kFeatures640x480,
	0,0,0
},
{
	{
		"adiboudchoucampagne",
		_s("Missing game code"), // Addy Buschu auf dem Land 1.00 (Engine: DEV7 version unknown)
		AD_ENTRY1s("adbc_envir_obc.stk", "4b43d3d1a8bc908d80e729069c5bb59f", 2831471),
		DE_DEU,
		kPlatformWindows,
		ADGF_UNSUPPORTED,
		GUIO0()
	},
	kGameTypeNone,
	kFeatures640x480,
	0,0,0
},
{
	{
		"adiboudchoujunglesavane",
		_s("Missing game code"), // Addy Buschu Die bunte Tierwelt 1.00 (Engine: DEV7 version 1.0.0.0)
		AD_ENTRY1s("adbc_envir_obc.stk", "7f33561f295030cbe64a21f941ef1efc", 3188852),
		DE_DEU,
		kPlatformWindows,
		ADGF_UNSUPPORTED,
		GUIO0()
	},
	kGameTypeNone,
	kFeatures640x480,
	0,0,0
},

#endif // GOB_DETECTION_TABLES_ADIBOUDCHOU_H
