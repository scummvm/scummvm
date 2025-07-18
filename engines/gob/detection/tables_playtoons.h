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

/* Detection tables for the Playtoons series. */
/* This Game uses the DEV6 Engine, more Information can be found here: https://wiki.scummvm.org/index.php?title=DEV6_Information */
/* These games are part of the Playtoons series. For more information, refer to our wiki: https://wiki.scummvm.org/index.php?title=Playtoons */

#ifndef GOB_DETECTION_TABLES_PLAYTOONS_H
#define GOB_DETECTION_TABLES_PLAYTOONS_H

// -- Playtoons 1: Uncle Archibald --

{
	{
		"playtoons1",
		"v1.002",
		AD_ENTRY2s("playtoon.stk",	"8c98e9a11be9bb203a55e8c6e68e519b", 25574338,
				   "archi.stk",		"8d44b2a0d4e3139471213f9f0ed21e81", 5524674),
		FR_FRA,
		kPlatformWindows,
		ADGF_NO_FLAGS,
		GUIO3(GUIO_NOSUBTITLES, GUIO_NOSPEECH, GUIO_NOASPECT)
	},
	kFeatures640x480,
	"intro2.stk", 0, 0
},
{
	{
		"playtoons1",
		"Pack mes histoires animées",
		AD_ENTRY2s("playtoon.stk",	"55f0293202963854192e39474e214f5f", 30448474,
				   "archi.stk",		"8d44b2a0d4e3139471213f9f0ed21e81", 5524674),
		FR_FRA,
		kPlatformWindows,
		ADGF_NO_FLAGS,
		GUIO3(GUIO_NOSUBTITLES, GUIO_NOSPEECH, GUIO_NOASPECT)
	},
	kFeatures640x480,
	"intro2.stk", 0, 0
},
{
	{
		"playtoons1",
		"",
		AD_ENTRY2s("playtoon.stk",	"c5ca2a288cdaefca9556cd9ae4b579cf", 25158926,
				   "archi.stk",		"8d44b2a0d4e3139471213f9f0ed21e81", 5524674),
		DE_DEU,
		kPlatformWindows,
		ADGF_NO_FLAGS,
		GUIO3(GUIO_NOSUBTITLES, GUIO_NOSPEECH, GUIO_NOASPECT)
	},
	kFeatures640x480,
	"intro2.stk", 0, 0
},
{ // Supplied by scoriae in the forums
	{
		"playtoons1",
		"v1.002",
		AD_ENTRY2s("playtoon.stk",	"9e513e993a5b0e2496add3f50c08764b", 30448506,
				   "archi.stk",		"00d8274519dfcf8a0d8ae3099daea0f8", 5532135),
		EN_ANY,
		kPlatformWindows,
		ADGF_NO_FLAGS,
		GUIO3(GUIO_NOSUBTITLES, GUIO_NOSPEECH, GUIO_NOASPECT)
	},
	kFeatures640x480,
	"intro2.stk", 0, 0
},
{
	{
		"playtoonsdemo",
		"Non-Interactive",
		AD_ENTRY1s("play123.scn", "4689a31f543915e488c3bc46ea358add", 258),
		EN_ANY,
		kPlatformWindows,
		ADGF_DEMO,
		GUIO3(GUIO_NOSUBTITLES, GUIO_NOSPEECH, GUIO_NOASPECT)
	},
	kFeatures640x480 | kFeaturesSCNDemo,
	0, 0, 3
},
{
	{
		"playtoons1",
		"Non-Interactive Demo",
		AD_ENTRY2s("e.scn",			"8a0db733c3f77be86e74e8242e5caa61", 124,
				   "demarchg.vmd",	"d14a95da7d8792faf5503f649ffcbc12", 5619415),
		EN_ANY,
		kPlatformWindows,
		ADGF_DEMO,
		GUIO3(GUIO_NOSUBTITLES, GUIO_NOSPEECH, GUIO_NOASPECT)
	},
	kFeatures640x480 | kFeaturesSCNDemo,
	0, 0, 4
},
{
	{
		"playtoons1",
		"Non-Interactive Demo",
		AD_ENTRY1s("i.scn",	"8b3294474d39970463663edd22341730", 285),
		IT_ITA,
		kPlatformWindows,
		ADGF_DEMO,
		GUIO3(GUIO_NOSUBTITLES, GUIO_NOSPEECH, GUIO_NOASPECT)
	},
	kFeatures640x480 | kFeaturesSCNDemo,
	0, 0, 5
},
{
	{
		"playtoons1",
		"Non-Interactive Demo",
		AD_ENTRY1s("s.scn",	"1f527010626b5490761f16ba7a6f639a", 251),
		ES_ESP,
		kPlatformWindows,
		ADGF_DEMO,
		GUIO3(GUIO_NOSUBTITLES, GUIO_NOSPEECH, GUIO_NOASPECT)
	},
	kFeatures640x480 | kFeaturesSCNDemo,
	0, 0, 6
},

// -- Playtoons 2: The Case of the Counterfeit Collaborator (Spirou) --

{
	{
		"playtoons2",
		"",
		AD_ENTRY2s("playtoon.stk",	"4772c96be88a57f0561519e4a1526c62", 24406262,
				   "spirou.stk",	"5d9c7644d0c47840169b4d016765cc1a", 9816201),
		EN_ANY,
		kPlatformWindows,
		ADGF_NO_FLAGS,
		GUIO3(GUIO_NOSUBTITLES, GUIO_NOSPEECH, GUIO_NOASPECT)
	},
	kFeatures640x480,
	"intro2.stk", 0, 0
},
{
	{
		"playtoons2",
		"",
		AD_ENTRY2s("playtoon.stk",	"55a85036dd93cce93532d8f743d90074", 17467154,
				   "spirou.stk",	"e3e1b6148dd72fafc3637f1a8e5764f5", 9812043),
		FR_FRA,
		kPlatformWindows,
		ADGF_NO_FLAGS,
		GUIO3(GUIO_NOSUBTITLES, GUIO_NOSPEECH, GUIO_NOASPECT)
	},
	kFeatures640x480,
	"intro2.stk", 0, 0
},
{ // Bugreport #7052
	{
		"playtoons2",
		"v1.002",
		AD_ENTRY2s("playtoon.stk",	"8c98e9a11be9bb203a55e8c6e68e519b", 25574338,
				   "spirou.stk",	"91080dc148de1bbd6a97321c1a1facf3", 9817086),
		FR_FRA,
		kPlatformWindows,
		ADGF_NO_FLAGS,
		GUIO3(GUIO_NOSUBTITLES, GUIO_NOSPEECH, GUIO_NOASPECT)
	},
	kFeatures640x480,
	"intro2.stk", 0, 0
},
{
	{
		"playtoons2",
		"v1.002",
		AD_ENTRY2s("playtoon.stk",	"c5ca2a288cdaefca9556cd9ae4b579cf", 25158926,
				   "spirou.stk",	"91080dc148de1bbd6a97321c1a1facf3", 9817086),
		DE_DEU,
		kPlatformWindows,
		ADGF_NO_FLAGS,
		GUIO3(GUIO_NOSUBTITLES, GUIO_NOSPEECH, GUIO_NOASPECT)
	},
	kFeatures640x480,
	"intro2.stk", 0, 0
},
{ // Supplied by Hkz
	{
		"playtoons2",
		"",
		AD_ENTRY2s("playtoon.stk",	"2572685400852d12759a2fbf09ec88eb", 9698780,
				  "spirou.stk",		"d3cfeff920b6343a2ece55088f530dba", 7076608),
		IT_ITA,
		kPlatformWindows,
		ADGF_NO_FLAGS,
		GUIO3(GUIO_NOSUBTITLES, GUIO_NOSPEECH, GUIO_NOASPECT)
	},
	kFeatures640x480,
	"intro2.stk", 0, 0
},
{ // Supplied by scoriae in the forums
	{
		"playtoons2",
		"v1.002",
		AD_ENTRY2s("playtoon.stk",	"9e513e993a5b0e2496add3f50c08764b", 30448506,
				   "spirou.stk",	"993737f112ca6a9b33c814273280d832", 9825760),
		EN_ANY,
		kPlatformWindows,
		ADGF_NO_FLAGS,
		GUIO3(GUIO_NOSUBTITLES, GUIO_NOSPEECH, GUIO_NOASPECT)
	},
	kFeatures640x480,
	"intro2.stk", 0, 0
},

// -- Playtoons 3: The Secret of the Castle --

{
	{
		"playtoons3",
		"v1.002",
		AD_ENTRY2s("playtoon.stk",	"8c98e9a11be9bb203a55e8c6e68e519b", 25574338,
				   "chato.stk",		"4fa4ed96a427c344e9f916f9f236598d", 6033793),
		FR_FRA,
		kPlatformWindows,
		ADGF_NO_FLAGS,
		GUIO3(GUIO_NOSUBTITLES, GUIO_NOSPEECH, GUIO_NOASPECT)
	},
	kFeatures640x480,
	"intro2.stk", 0, 0
},
{
	{
		"playtoons3",
		"v1.002",
		AD_ENTRY2s("playtoon.stk",	"9e513e993a5b0e2496add3f50c08764b", 30448506,
				   "chato.stk",		"8fc8d0da5b3e758908d1d7298d497d0b", 6041026),
		EN_ANY,
		kPlatformWindows,
		ADGF_NO_FLAGS,
		GUIO3(GUIO_NOSUBTITLES, GUIO_NOSPEECH, GUIO_NOASPECT)
	},
	kFeatures640x480,
	"intro2.stk", 0, 0
},
{
	{
		"playtoons3",
		"Pack mes histoires animées",
		AD_ENTRY2s("playtoon.stk",	"55f0293202963854192e39474e214f5f", 30448474,
				   "chato.stk",		"4fa4ed96a427c344e9f916f9f236598d", 6033793),
		FR_FRA,
		kPlatformWindows,
		ADGF_NO_FLAGS,
		GUIO3(GUIO_NOSUBTITLES, GUIO_NOSPEECH, GUIO_NOASPECT)
	},
	kFeatures640x480,
	"intro2.stk", 0, 0
},
{
	{
		"playtoons3",
		"",
		AD_ENTRY2s("playtoon.stk",	"c5ca2a288cdaefca9556cd9ae4b579cf", 25158926,
				   "chato.stk", 	"3c6cb3ac8a5a7cf681a19971a92a748d", 6033791),
		DE_DEU,
		kPlatformWindows,
		ADGF_NO_FLAGS,
		GUIO3(GUIO_NOSUBTITLES, GUIO_NOSPEECH, GUIO_NOASPECT)
	},
	kFeatures640x480,
	"intro2.stk", 0, 0
},
{ // Supplied by Hkz on #scummvm
	{
		"playtoons3",
		"",
		AD_ENTRY2s("playtoon.stk",	"4772c96be88a57f0561519e4a1526c62", 24406262,
				   "chato.stk",		"bdef407387112bfcee90e664865ac3af", 6033867),
		EN_ANY,
		kPlatformWindows,
		ADGF_NO_FLAGS,
		GUIO3(GUIO_NOSUBTITLES, GUIO_NOSPEECH, GUIO_NOASPECT)
	},
	kFeatures640x480,
	"intro2.stk", 0, 0
},

// -- Playtoons 4: The Mandarin Prince --

{
	{
		"playtoons4",
		"v1.002",
		AD_ENTRY2s("playtoon.stk",	"b7f5afa2dc1b0f75970b7c07d175db1b", 24340406,
				   "manda.stk",		"92529e0b927191d9898a34c2892e9a3a", 6485072),
		FR_FRA,
		kPlatformWindows,
		ADGF_NO_FLAGS,
		GUIO3(GUIO_NOSUBTITLES, GUIO_NOSPEECH, GUIO_NOASPECT)
	},
	kFeatures640x480,
	"intro2.stk", 0, 0
},
{ // Supplied by indy4fan in bug report #13100. Orig title: "Der Mandarin-Prinz"
	{
		"playtoons4",
		"",
		AD_ENTRY2s("playtoon.stk",	"f853153e9be33b9e0ec6970d05642e51", 30448480,
				   "manda.stk",		"fb65d32f43ade3ff573a8534d5a1a91e", 6492732),
		DE_DEU,
		kPlatformWindows,
		ADGF_NO_FLAGS,
		GUIO3(GUIO_NOSUBTITLES, GUIO_NOSPEECH, GUIO_NOASPECT)
	},
	kFeatures640x480,
	"intro2.stk", 0, 0
},
{ //Supplied by goodoldgeorg in bug report #4390
	{
		"playtoons4",
		"v1.002",
		AD_ENTRY2s("playtoon.stk",	"9e513e993a5b0e2496add3f50c08764b", 30448506,
				   "manda.stk",		"69a79c9f61b2618e482726f2ff68078d", 6499208),
		EN_ANY,
		kPlatformWindows,
		ADGF_NO_FLAGS,
		GUIO3(GUIO_NOSUBTITLES, GUIO_NOSPEECH, GUIO_NOASPECT)
	},
	kFeatures640x480,
	"intro2.stk", 0, 0
},

// -- Playtoons 5: The Stone of Wakan --

{
	{
		"playtoons5",
		"v1.002",
		AD_ENTRY2s("playtoon.stk",	"55f0293202963854192e39474e214f5f", 30448474,
				   "wakan.stk",		"f493bf82851bc5ba74d57de6b7e88df8", 5520153),
		FR_FRA,
		kPlatformWindows,
		ADGF_NO_FLAGS,
		GUIO3(GUIO_NOSUBTITLES, GUIO_NOSPEECH, GUIO_NOASPECT)
	},
	kFeatures640x480,
	"intro2.stk", 0, 0
},
{ // Supplied by indy4fan in bug report #13099. Orig title: "Der Stein von Wakan"
	{
		"playtoons5",
		"v1.002",
		AD_ENTRY2s("playtoon.stk",	"9e513e993a5b0e2496add3f50c08764b", 30448506,
				   "wakan.stk",		"5518139580becd8c49bbfbdd4f49187a", 5523417),
		DE_DEU,
		kPlatformWindows,
		ADGF_NO_FLAGS,
		GUIO3(GUIO_NOSUBTITLES, GUIO_NOSPEECH, GUIO_NOASPECT)
	},
	kFeatures640x480,
	"intro2.stk", 0, 0
},

// -- Playtoons Construction Kit 1: Monsters --

{
	{
		"playtnck1",
		"v1.002",
		AD_ENTRY2s("playtoon.stk",	"5f9aae29265f1f105ad8ec195dff81de", 68382024,
				   "dan.itk",		"906d67b3e438d5e95ec7ea9e781a94f3", 3000320),
		FR_FRA,
		kPlatformWindows,
		ADGF_NO_FLAGS,
		GUIO3(GUIO_NOSUBTITLES, GUIO_NOSPEECH, GUIO_NOASPECT)
	},
	kFeatures640x480,
	"intro2.stk", 0, 0
},

// -- Playtoons Construction Kit 2: Knights --

{
	{
		"playtnck2",
		"v1.002",
		AD_ENTRY2s("playtoon.stk",	"5f9aae29265f1f105ad8ec195dff81de", 68382024,
				   "dan.itk",		"74eeb075bd2cb47b243349730264af01", 3213312),
		FR_FRA,
		kPlatformWindows,
		ADGF_NO_FLAGS,
		GUIO3(GUIO_NOSUBTITLES, GUIO_NOSPEECH, GUIO_NOASPECT)
	},
	kFeatures640x480,
	"intro2.stk", 0, 0
},

// -- Playtoons Construction Kit 3: Far West --

{
	{
		"playtnck3",
		"v1.002",
		AD_ENTRY2s("playtoon.stk",	"5f9aae29265f1f105ad8ec195dff81de", 68382024,
				   "dan.itk",		"9a8f62809eca5a52f429b5b6a8e70f8f", 2861056),
		FR_FRA,
		kPlatformWindows,
		ADGF_NO_FLAGS,
		GUIO3(GUIO_NOSUBTITLES, GUIO_NOSPEECH, GUIO_NOASPECT)
	},
	kFeatures640x480,
	"intro2.stk", 0, 0
},
{
	{ // Supplied by laenion in Bugreport #13457
		"playtnck3",
		"",
		AD_ENTRY2s("playtoon.stk",	"9ca3a2c19a3261cf9fa0f40eebc9d3ad", 65879662,
				   "dan.itk",		"e8566d7efb8601a323adc918948d03fe", 3325952),
		DE_DEU,
		kPlatformWindows,
		ADGF_NO_FLAGS,
		GUIO3(GUIO_NOSUBTITLES, GUIO_NOSPEECH, GUIO_NOASPECT)
	},
	kFeatures640x480,
	"intro2.stk", 0, 0
},

// -- Bambou le sauveur de la jungle --

{
	{
		"bambou",
		"",
		AD_ENTRY2s("intro.stk",		"2f8db6963ff8d72a8331627ebda918f4", 3613238,
				   "bambou.itk",	"0875914d31126d0749313428f10c7768", 114440192),
		FR_FRA,
		kPlatformWindows,
		ADGF_NO_FLAGS,
		GUIO3(GUIO_NOSUBTITLES, GUIO_NOSPEECH, GUIO_NOASPECT)
	},
	kFeatures640x480,
	"intro.stk", "intro.tot", 0
},

#endif // GOB_DETECTION_TABLES_PLAYTOONS_H
