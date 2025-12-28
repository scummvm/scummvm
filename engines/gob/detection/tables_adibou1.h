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

/* Detection tables for Adibou 1 / A.J.'s World of Discovery / ADI Jnr. */
/* These games are part of the Adibou series. For more information, refer to our wiki: https://wiki.scummvm.org/index.php?title=Adibou_Games */

#ifndef GOB_DETECTION_TABLES_ADIBOU1_H
#define GOB_DETECTION_TABLES_ADIBOU1_H

//  -- French: Adibou --

{
	{
		"adibou1",
		"ADIBOU 1 Environnement 4-7 ans",
		AD_ENTRY1s("intro.stk", "6db110188fcb7c5208d9721b5282682a", 4805104),
		FR_FRA,
		kPlatformDOS,
		ADGF_NO_FLAGS,
		GUIO3(GUIO_NOSUBTITLES, GUIO_NOSPEECH, GAMEOPTION_COPY_PROTECTION)
	},
	kFeaturesAdLib,
	0, 0, 0
},

{
	{ // Supplied by sdelamarre
		"adibou1",
		"ADIBOU 1 Environnement 4-7 ans",
		AD_ENTRY1s("intro.stk", "904a93f46687617bb34e672020fc17a4", 248724),
		FR_FRA,
		kPlatformDOS,
		ADGF_NO_FLAGS,
		GUIO3(GUIO_NOSUBTITLES, GUIO_NOSPEECH, GAMEOPTION_COPY_PROTECTION)
	},
	kFeaturesAdLib | kFeatures16Colors,
	0, "base.tot", 0
},

{
	{ // Supplied by sdelamarre
		"adibou1",
		"ADIBOU 1 Environnement 4-7 ans",
		AD_ENTRY1s("intro.stk", "228edf921ebcd9f1c6d566856f264ea4", 2647968),
		FR_FRA,
		kPlatformDOS,
		ADGF_NO_FLAGS,
		GUIO3(GUIO_NOSUBTITLES, GUIO_NOSPEECH, GAMEOPTION_COPY_PROTECTION)
	},
	kFeaturesAdLib,
	0, 0, 0
},
{
	{ // 1994 CD version
		"adibou1",
		"ADIBOU 1 Environnement 4-5 ans (CD)",
		AD_ENTRY2s("intro.stk", "6db110188fcb7c5208d9721b5282682a", 4805104,
				   "c51.stk", "38daec4f7a7fcedbdf5e47b3c5f28e35", 5680126),
		FR_FRA,
		kPlatformWindows,
		ADGF_CD,
		GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
	},
	kFeaturesAdLib | kFeatures640x400,
	0, 0, 0
},

//  -- German: ADI Jr. Spielerisch lernen --

{
	{ // 1994 CD version - Supplied by BJNFNE
		"adibou1",
		"ADI Jr. 4-6 Jahre (CD)",
		AD_ENTRY2s("intro.stk", "4d4c23da4cd7e080cb1769b49ace1805", 4731020,
				   "l51.stk", "0397e893892ffe1d6c64d28841437fd7", 7308050),
		DE_DEU,
		kPlatformWindows,
		ADGF_CD,
		GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
	},
	kFeaturesAdLib | kFeatures640x400,
	0, 0, 0
},
{
	{ // 1994 CD version - Supplied by Indy4-Fan
		"adibou1",
		"ADI Jr. 6-7 Jahre (CD)",
		AD_ENTRY2s("intro.stk", "4d4c23da4cd7e080cb1769b49ace1805", 4731020,
				   "c61.stk", "1aca103ed84241487c5cf394ae37e8d7", 5966096),
		DE_DEU,
		kPlatformWindows,
		ADGF_CD,
		GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
	},
	 kFeaturesAdLib | kFeatures640x400,
	 0, 0, 0
},

// -- English: A.J.'s World of Discovery / ADI Jnr.

// -- DOS VGA Floppy --

{
	{
		"adibou1",
		"AJ's World of Discovery",
		AD_ENTRY1s("intro.stk", "e453bea7b28a67c930764d945f64d898", 3913628),
		EN_GRB,
		kPlatformDOS,
		ADGF_NO_FLAGS,
		GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
	},
	kFeaturesAdLib,
	0, 0, 0
},

// Supplied by jp438-2 in bug report #13972
{
	{
		"adibou1",
		"Adi Jnr.",
		AD_ENTRY1s("intro.stk", "6d234641b74b3bdf746c39a64ff1abcc", 2678326),
		EN_GRB,
		kPlatformDOS,
		ADGF_NO_FLAGS,
		GUIO3(GUIO_NOSUBTITLES, GUIO_NOSPEECH, GAMEOPTION_COPY_PROTECTION)
	},
	kFeaturesAdLib,
	0, 0, 0
},

// -- Amiga Floppy --

{
	{ // Supplied by eientei95
		"adibou1",
		"Adi Jnr",
		AD_ENTRY1s("intro.stk", "71e7db034890885ac96dd1be43a21c38", 556834),
		EN_ANY,
		kPlatformAmiga,
		ADGF_NO_FLAGS,
		GUIO3(GUIO_NOSUBTITLES, GUIO_NOSPEECH, GAMEOPTION_COPY_PROTECTION)
	},
	kFeaturesNone,
	0, 0, 0
},

// Italian: Adibù
// (missing)

// -- Add-ons : Read 4-5 years --
{
	{
		"adibou1read45",
		"", // Je lis 4-5 ans"
		AD_ENTRY1s("l51.stk", "8eb81211f8ee163885cc8b31d04d9380", 325445),
		FR_FRA,
		kPlatformDOS,
		ADGF_ADDON,
		GUIO0()
	},
	kFeaturesAdLib  | kFeatures16Colors,
	0, 0, 0
},

{
	{
		"adibou1read45",
		"", // Je lis 4-5 ans"
		AD_ENTRY1s("l51.stk", "50004db83a88750d582113e0669a9604", 1437256),
		FR_FRA,
		kPlatformDOS,
		ADGF_ADDON,
		GUIO0()
	},
	kFeaturesAdLib,
	0, 0, 0
},

// -- Add-ons : Count 4-5 years --
{
	{
		"adibou1count45",
		"", // Je calcule 4-5 ans"
		AD_ENTRY1s("c51.stk", "c57292304c2657000bd92dbaee33b52b", 330329),
		FR_FRA,
		kPlatformDOS,
		ADGF_ADDON,
		GUIO0()
	},
	kFeaturesAdLib | kFeatures16Colors,
	0, 0, 0
},

{
	{
		"adibou1count45",
		"", // Je calcule 4-5 ans"
		AD_ENTRY1s("c51.stk", "264e5426bd06d5fedd8edf7c08302984", 359953),
		FR_FRA,
		kPlatformDOS,
		ADGF_ADDON,
		GUIO0()
	},
	kFeaturesAdLib | kFeatures16Colors,
	0, 0, 0
},

{
	{
		"adibou1count45",
		"", // Je calcule 4-5 ans"
		AD_ENTRY1s("c51.stk", "afefebef6256fe4f72bdbdc30fdc0f2d", 1313166),
		FR_FRA,
		kPlatformDOS,
		ADGF_ADDON,
		GUIO0()
	},
	kFeaturesAdLib,
	0, 0, 0
},

// -- Add-ons : Read 6-7 years --
{
	{
		"adibou1read67",
		"", // Je lis 6-7 ans"
		AD_ENTRY1s("l61.stk", "d236b4268d8265b958a90a41eae0f15a", 356444),
		FR_FRA,
		kPlatformDOS,
		ADGF_ADDON,
		GUIO0()
	},
	kFeaturesAdLib | kFeatures16Colors,
	0, 0, 0
},

{
	{
		"adibou1read67",
		"", // Je lis 6-7 ans"
		AD_ENTRY1s("l61.stk", "1c993aa788b4159bbc9591921854d428", 353121),
		FR_FRA,
		kPlatformDOS,
		ADGF_ADDON,
		GUIO0()
	},
	kFeaturesAdLib | kFeatures16Colors,
	0, 0, 0
},

{
	{
		"adibou1read67",
		"", // Je lis 6-7 ans"
		AD_ENTRY1s("l61.stk", "71ff03db9aa9d3be05ac6050d7d5e681", 1396282),
		FR_FRA,
		kPlatformDOS,
		ADGF_ADDON,
		GUIO0()
	},
	kFeaturesAdLib,
	0, 0, 0
},

// -- Add-ons : Count 6-7 years --
{
	{
		"adibou1count67",
		"", // Je calcule 6-7 ans"
		AD_ENTRY1s("c61.stk", "f5ef0318f342083f835426718b74c89a", 318641),
		FR_FRA,
		kPlatformDOS,
		ADGF_ADDON,
		GUIO0()
	},
	kFeaturesAdLib | kFeatures16Colors,
	0, 0, 0
},

{
	{
		"adibou1count67",
		"", // Je calcule 6-7 ans"
		AD_ENTRY1s("c61.stk", "b6849f45151b8dfe48d873fbd468b679", 1242750),
		FR_FRA,
		kPlatformDOS,
		ADGF_ADDON,
		GUIO0()
	},
	kFeaturesAdLib,
	0, 0, 0
},

#endif // GOB_DETECTION_TABLES_ADIBOU1_H
