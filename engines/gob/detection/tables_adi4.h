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

/* Detection tables for the ADI / Addy 4 series. */
/* This Game uses the DEV6 Engine, more Information can be found here: https://wiki.scummvm.org/index.php?title=DEV6_Information */
/* These games are part of the Adi series. For more information, refer to our wiki: https://wiki.scummvm.org/index.php?title=Adi_Games */

#ifndef GOB_DETECTION_TABLES_ADI4_H
#define GOB_DETECTION_TABLES_ADI4_H

// -- French: Adi --

{
	{
		"adi4",
		"Adi 4.00 Collège",
		AD_ENTRY1s("intro.stk", "a3c35d19b2d28ea261d96321d208cb5a", 6021466),
		FR_FRA,
		kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO1(GUIO_NOASPECT)
	},
	kFeatures640x480,
	0, 0, 0
},
{
	{
		"adi4",
		"Adi 4.00",
		AD_ENTRY1s("intro.stk", "44491d85648810bc6fcf84f9b3aa47d5", 5834944),
		FR_FRA,
		kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO1(GUIO_NOASPECT)
	},
	kFeatures640x480,
	0, 0, 0
},
{
	{
		"adi4",
		"Adi 4.00 École",
		AD_ENTRY1s("intro.stk", "29374c0e3c10b17dd8463b06a55ad093", 6012072),
		FR_FRA,
		kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO1(GUIO_NOASPECT)
	},
	kFeatures640x480,
	0, 0, 0
},
{
	{
		"adi4",
		"Adi 4.00 Limited Edition",
		AD_ENTRY1s("intro.stk", "ebbbc5e28a4adb695535ed989c1b8d66", 5929644),
		FR_FRA,
		kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO1(GUIO_NOASPECT)
	},
	kFeatures640x480,
	0, 0, 0
},
{
	{
		"adi4",
		"ADI 4.10",
		AD_ENTRY1s("intro.stk", "6afc2590856433b9f5295b032f2b205d", 5923112),
		FR_FRA,
		kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO1(GUIO_NOASPECT)
	},
	kFeatures640x480,
	0, 0, 0
},
{
	{
		"adi4",
		"ADI 4.11",
		AD_ENTRY1s("intro.stk", "6296e4be4e0c270c24d1330881900c7f", 5921234),
		FR_FRA,
		kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO1(GUIO_NOASPECT)
	},
	kFeatures640x480,
	0, 0, 0
},
{
	{
		"adi4",
		"ADI 4.21",
		AD_ENTRY1s("intro.stk", "c5b9f6222c0b463f51dab47317c5b687", 5950490),
		FR_FRA,
		kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO1(GUIO_NOASPECT)
	},
	kFeatures640x480,
	0, 0, 0
},

// -- German: Addy --

{
	{ // Supplied by Indy4-Fan
		"adi4",
		"Addy 4.00 Erdkunde",
		AD_ENTRY1s("intro.stk", "fda1566d233ee55d65b2ad014c1cb485", 188),
		DE_DEU,
		kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO1(GUIO_NOASPECT)
	},
	kFeatures640x480,
	0, "GA2INTRO.TOT", 0
},
{
	{ // Supplied by fischbeck
		"adi4",
		"Addi Simule", // That is not an typo in the name "Addi" that's how this version is called.
		AD_ENTRY1s("simule.stk", "66d97fe54bbf8ea4bbb18534cb28b13f", 2523796),
		DE_DEU,
		kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO1(GUIO_NOASPECT)
	},
	kFeatures640x480,
	"simule.stk", "INTRODD.TOT", 0 // INTRODD.TOT brings up a main menu to select various environmental learning tasks.
},
{
	{
		"adi4",
		"Addy 4 Grundschule Basisprogramm",
		AD_ENTRY1s("intro.stk", "d2f0fb8909e396328dc85c0e29131ba8", 5847588),
		DE_DEU,
		kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO1(GUIO_NOASPECT)
	},
	kFeatures640x480,
	0, 0, 0
},
{
	{
		"adi4",
		"Addy 4.01 Sekundarstufe Basisprogramm",
		AD_ENTRY1s("intro.stk", "367340e59c461b4fa36651cd74e32c4e", 5847378),
		DE_DEU,
		kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO1(GUIO_NOASPECT)
	},
	kFeatures640x480,
	0, 0, 0
},
{
	{
		"adi4",
		"Addy 4.21 Sekundarstufe Basisprogramm",
		AD_ENTRY1s("intro.stk", "534f0b674cd4830df94a9c32c4ea7225", 6878034),
		DE_DEU,
		kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO1(GUIO_NOASPECT)
	},
	kFeatures640x480,
	0, 0, 0
},

// -- English: ADI --

{
	{
		"adi4",
		"ADI 4.10",
		AD_ENTRY1s("intro.stk", "3e3fa9656e37d802027635ace88c4cc5", 5359144),
		EN_GRB,
		kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO1(GUIO_NOASPECT)
	},
	kFeatures640x480,
	0, 0, 0
},

// -- Demos --

{
	{
		"adi4",
		"Adi 4.00 Interactive Demo",
		AD_ENTRY1s("intro.stk", "89ace204dbaac001425c73f394334f6f", 2413102),
		FR_FRA,
		kPlatformWindows,
		ADGF_DEMO | ADGF_UNSTABLE,
		GUIO1(GUIO_NOASPECT)
	},
	kFeatures640x480,
	0, 0, 0
},
{
	{
		"adi4",
		"Adi 4.00 / Adibou 2 Demo",
		AD_ENTRY1s("intro.stk", "d41d8cd98f00b204e9800998ecf8427e", 0),
		FR_FRA,
		kPlatformWindows,
		ADGF_DEMO | ADGF_UNSTABLE,
		GUIO1(GUIO_NOASPECT)
	},
	kFeatures640x480,
	0, 0, 0
},

#endif // GOB_DETECTION_TABLES_ADI4_H
