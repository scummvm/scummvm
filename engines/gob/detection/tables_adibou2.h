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

/* Detection tables for Adibou / Addy Junior series. */
/* This Game uses the DEV6 Engine, more Information can be found here: https://wiki.scummvm.org/index.php?title=DEV6_Information */
/* These games are part of the Adibou series. For more information, refer to our wiki: https://wiki.scummvm.org/index.php?title=Adibou_Games */

#ifndef GOB_DETECTION_TABLES_ADIBOU2_H
#define GOB_DETECTION_TABLES_ADIBOU2_H

// -- French: Adibou --

{
	{
		"adibou2",
		"ADIBOU 2",
		AD_ENTRY1s("intro.stk", "94ae7004348dc8bf99c23a9a6ef81827", 956162),
		FR_FRA,
		kPlatformWindows,
		ADGF_NO_FLAGS,
		GUIO0()
	},
	kFeatures640x480,
	0, 0, 0
},
{
	{
		"adibou2",
		"Le Jardin Magique d'Adibou",
		AD_ENTRY1s("intro.stk", "a8ff86f3cc40dfe5898e0a741217ef27", 956328),
		FR_FRA,
		kPlatformWindows,
		ADGF_NO_FLAGS,
		GUIO0()
	},
	kFeatures640x480,
	0, 0, 0
},
{
	{
		"adibou2",
		"ADIBOU Version Decouverte 2.11",
		AD_ENTRY1s("intro.stk", "558c14327b79ed39214b49d567a75e33", 8737856),
		FR_FRA,
		kPlatformWindows,
		ADGF_NO_FLAGS,
		GUIO0()
	},
	kFeatures640x480,
	0, 0, 0
},
{
	{
		"adibou2",
		"ADIBOU 2.10 Environnement",
		AD_ENTRY2s("intro.stk", "f2b797819aeedee557e904b0b5ccd82e", 8736454,
				   "BECBF210.CD1",	"bc828c320908a5eaa349956d396bd8e1", 8),
		FR_FRA,
		kPlatformWindows,
		GF_ENABLE_ADIBOU2_FREE_BANANAS_WORKAROUND | GF_ENABLE_ADIBOU2_FLOWERS_INFINITE_LOOP_WORKAROUND,
		GUIO0()
	},
	kFeatures640x480,
	0, 0, 0
},
{
	{
		"adibou2",
		"ADIBOU 2.11 Environnement",
		AD_ENTRY2s("intro.stk", "7b1f1f6f6477f54401e95d913f75e333", 8736904,
				   "BECBF211.CD1",	"bc828c320908a5eaa349956d396bd8e1", 8),
		FR_FRA,
		kPlatformWindows,
		GF_ENABLE_ADIBOU2_FREE_BANANAS_WORKAROUND | GF_ENABLE_ADIBOU2_FLOWERS_INFINITE_LOOP_WORKAROUND,
		GUIO0()
	},
	kFeatures640x480,
	0, 0, 0
},
{
	{
		"adibou2",
		"ADIBOU 2.12 Environnement",
		AD_ENTRY2s("intro.stk", "1e49c39a4a3ce6032a84b712539c2d63", 8738134,
				   "BECBF212.CD1",	"bc828c320908a5eaa349956d396bd8e1", 8),
		FR_FRA,
		kPlatformWindows,
		GF_ENABLE_ADIBOU2_FREE_BANANAS_WORKAROUND | GF_ENABLE_ADIBOU2_FLOWERS_INFINITE_LOOP_WORKAROUND,
		GUIO0()
	},
	kFeatures640x480,
	0, 0, 0
},
{
	{
		"adibou2",
		"ADIBOU 2.13s Environnement",
		AD_ENTRY2s("intro.stk", "092707829555f27706920e4cacf1fada", 8737958,
				   "BECBF213.CD1",	"bc828c320908a5eaa349956d396bd8e1", 8),
		FR_FRA,
		kPlatformWindows,
		GF_ENABLE_ADIBOU2_FREE_BANANAS_WORKAROUND | GF_ENABLE_ADIBOU2_FLOWERS_INFINITE_LOOP_WORKAROUND,
		GUIO0()
	},
	kFeatures640x480,
	0, 0, 0
},
{
	{
		"adibou2",
		"ADIBOO 2.14 Environnement",
		AD_ENTRY1s("intro.stk", "ff63637e3cb7f0a457edf79457b1c6b3", 9333874),
		FR_FRA,
		kPlatformWindows,
		ADGF_NO_FLAGS,
		GUIO0()
	},
	kFeatures640x480,
	0, 0, 0
},

// -- German: Addy Junior --

{
	{
		"adibou2",
		"ADDY JR 2.20 Basisprogramm",
		AD_ENTRY2s("intro.stk", "092707829555f27706920e4cacf1fada", 8737958,
				   "BECBD220.CD1",	"bc828c320908a5eaa349956d396bd8e1", 8),
		DE_DEU,
		kPlatformWindows,
		ADGF_NO_FLAGS,
		GUIO0()
	},
	kFeatures640x480,
	0, 0, 0
},

{
	{ // Supplied by lotharsm
		"adibou2",
		"ADI Junior 2",
		AD_ENTRY1s("intro.stk", "80588ad3b5510bb44d3f40d6b07b81e7", 956328),
		DE_DEU,
		kPlatformWindows,
		ADGF_NO_FLAGS,
		GUIO0()
	},
	kFeatures640x480,
	0, 0, 0
},

{
	{ // Supplied by BJNFNE
		"adibou2",
		"ADI Jr.",
		AD_ENTRY2s("intro.stk", "718a51862406136c28639489a9ba950a", 956350,
				   "intro.inf", "d8710732c9bfe3ca52d3ce5aefc06089", 48),
		DE_DEU,
		kPlatformWindows,
		GF_ENABLE_ADIBOU2_FLOWERS_INFINITE_LOOP_WORKAROUND,
		GUIO0()
	},
	kFeatures640x480,
	0, 0, 0
},

{
	{ // Supplied by BJNFNE
		"adibou2",
		"ADDY JR 2.13 Basisprogramm",
		AD_ENTRY2s("intro.stk", "092707829555f27706920e4cacf1fada", 8737958,
				   "BECBD213.CD1",	"bc828c320908a5eaa349956d396bd8e1", 8),
		DE_DEU,
		kPlatformWindows,
		ADGF_NO_FLAGS,
		GUIO0()
	},
	kFeatures640x480,
	0, 0, 0
},

// -- Italian: Adibù --
{
	{
		"adibou2",
		"ADIBÙ 2.13 Ambiente",
		AD_ENTRY2s("intro.stk", "092707829555f27706920e4cacf1fada", 8737958,
				   "BECBI213.CD1",	"bc828c320908a5eaa349956d396bd8e1", 8),
		IT_ITA,
		kPlatformWindows,
		ADGF_NO_FLAGS,
		GUIO0()
	},
	kFeatures640x480,
	0, 0, 0
},

// -- Spanish: Adibù --
{
	{ // Supplied by eientei95
		"adibou2",
		"ADIBÙ 2",
		AD_ENTRY1s("intro.stk", "0b996fcd8929245fecddc4d9169843d0", 956682),
		ES_ESP,
		kPlatformWindows,
		GF_ENABLE_ADIBOU2_FLOWERS_INFINITE_LOOP_WORKAROUND,
		GUIO0()
	},
	kFeatures640x480,
	0, 0, 0
},

// -- English: Adiboo --
{
	{ // Supplied by sdelamarre
		"adibou2",
		"ADIBOO 2",
		AD_ENTRY2s("intro.stk", "718a51862406136c28639489a9ba950a", 956350,
				   "intro.inf", "9369aa62939f5f7c11b1e02a45038050", 44),
		EN_GRB,
		kPlatformWindows,
		GF_ENABLE_ADIBOU2_FLOWERS_INFINITE_LOOP_WORKAROUND,
		GUIO0()
	},
	kFeatures640x480,
	0, 0, 0
},
{
	{
		"adibou2",
		"ADIBOO 2.13 Environment",
		AD_ENTRY1s("intro.stk", "ff63637e3cb7f0a457edf79457b1c6b3", 9333874),
		EN_GRB,
		kPlatformWindows,
		ADGF_NO_FLAGS,
		GUIO0()
	},
	kFeatures640x480,
	0, 0, 0
},

// -- Demos --

{
	{
		"adibou2",
		"ADIBOU 2 Demo",
		AD_ENTRY1s("intro.stk", "0f197c6b8f1cef3fb4aa37438a52e031", 954276),
		FR_FRA,
		kPlatformWindows,
		ADGF_DEMO,
		GUIO0()
	},
	kFeatures640x480,
	0, 0, 0
},
{
	// Titlescreen says "ADIBOO: Limited version!", Sierra setup says "Adiboo 2 Demo"
	// Supplied by eientei95
	{
		"adibou2",
		"ADIBOO 2 Demo",
		AD_ENTRY1s("intro.stk", "ea6c2d25f33135db763c1175979d904a", 528108),
		EN_GRB,
		kPlatformWindows,
		ADGF_DEMO,
		GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
	},
	kFeatures640x480,
	0, 0, 0
},
{
	{
		"adibou2",
		"Non-Interactive Demo",
		AD_ENTRY2s("demogb.scn", "9291455a908ac0e6aaaca686e532609b", 105,
				   "demogb.vmd", "bc9c1db97db7bec8f566332444fa0090", 14320840),
		EN_GRB,
		kPlatformWindows,
		ADGF_DEMO,
		GUIO3(GUIO_NOSUBTITLES, GUIO_NOSPEECH, GUIO_NOASPECT)
	},
	kFeatures640x480 | kFeaturesSCNDemo,
	0, 0, 9
},
{
	{
		"adibou2",
		"Non-Interactive Demo",
		AD_ENTRY2s("demoall.scn", "c8fd308c037b829800006332b2c32674", 106,
				   "demoall.vmd", "4672b2deacc6fca97484840424b1921b", 14263433),
		DE_DEU,
		kPlatformWindows,
		ADGF_DEMO,
		GUIO3(GUIO_NOSUBTITLES, GUIO_NOSPEECH, GUIO_NOASPECT)
	},
	kFeatures640x480 | kFeaturesSCNDemo,
	0, 0, 10
},
{
	{
		"adibou2",
		"Non-Interactive Demo",
		AD_ENTRY2s("demofra.scn", "d1b2b1618af384ea1120def8b986c02b", 106,
				   "demofra.vmd", "b494cdec1aac7e54c3f2480512d2880e", 14297100),
		FR_FRA,
		kPlatformWindows,
		ADGF_DEMO,
		GUIO3(GUIO_NOSUBTITLES, GUIO_NOSPEECH, GUIO_NOASPECT)
	},
	kFeatures640x480 | kFeaturesSCNDemo,
	0, 0, 11
},
{ // Shipped as an Demo / Preview for Nature et Sciences on Adibou presente Dessin CD
  // Supplied by BJNFNE
  // ToDo: adding unimplemented opcodes is required.
  // Also support needs added for static images (.TGA files), The Banner for Nature et Sciences can't be loaded because of the missing opcode.
	{
		"adibou2",
		"Nature et Sciences Preview",
		AD_ENTRY1s("intro.stk", "22b997d97eef71c867b49092bd89c2b8", 38128),
		FR_FRA,
		kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO0()
	},
	kFeatures640x480,
	0, 0, 0
},
#endif // GOB_DETECTION_TABLES_ADIBOU2_H
