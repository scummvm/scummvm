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

/* Detection tables for Gobliins 2: The Prince Buffoon. */

#ifndef GOB_DETECTION_TABLES_GOB2_H
#define GOB_DETECTION_TABLES_GOB2_H

// -- DOS VGA Floppy --

{
	{
		"gob2",
		"",
		AD_ENTRY1("intro.stk", "b45b984ee8017efd6ea965b9becd4d66"),
		EN_GRB,
		kPlatformDOS,
		ADGF_NO_FLAGS,
		GUIO3(GUIO_NOSUBTITLES, GUIO_NOSPEECH, GAMEOPTION_COPY_PROTECTION)
	},
	kFeaturesAdLib,
	0, 0, 0
},
{
	{
		"gob2",
		"v1.03",
		AD_ENTRY1("intro.stk", "dedb5d31d8c8050a8cf77abedcc53dae"),
		EN_USA,
		kPlatformDOS,
		ADGF_NO_FLAGS,
		GUIO3(GUIO_NOSUBTITLES, GUIO_NOSPEECH, GAMEOPTION_COPY_PROTECTION)
	},
	kFeaturesAdLib,
	0, 0, 0
},
{ // Supplied by raziel_ in bug report #3621
	{
		"gob2",
		"v1.02",
		AD_ENTRY1s("intro.stk", "25a99827cd59751a80bed9620fb677a0", 893302),
		EN_USA,
		kPlatformDOS,
		ADGF_NO_FLAGS,
		GUIO3(GUIO_NOSUBTITLES, GUIO_NOSPEECH, GAMEOPTION_COPY_PROTECTION)
	},
	kFeaturesAdLib,
	0, 0, 0
},
{
	{
		"gob2",
		"",
		AD_ENTRY1s("intro.stk", "a13ecb4f6d8fd881ebbcc02e45cb5475", 837275),
		FR_FRA,
		kPlatformDOS,
		ADGF_NO_FLAGS,
		GUIO3(GUIO_NOSUBTITLES, GUIO_NOSPEECH, GAMEOPTION_COPY_PROTECTION)
	},
	kFeaturesAdLib,
	0, 0, 0
},
{ // Supplied by blackwhiteeagle in bug report #2934
	{
		"gob2",
		"v1.02",
		AD_ENTRY1("intro.stk", "3e4e7db0d201587dd2df4003b2993ef6"),
		DE_DEU,
		kPlatformDOS,
		ADGF_NO_FLAGS,
		GUIO3(GUIO_NOSUBTITLES, GUIO_NOSPEECH, GAMEOPTION_COPY_PROTECTION)
	},
	kFeaturesAdLib,
	0, 0, 0
},
{
	{
		"gob2",
		"",
		AD_ENTRY1("intro.stk", "a13892cdf4badda85a6f6fb47603a128"),
		DE_DEU,
		kPlatformDOS,
		ADGF_NO_FLAGS,
		GUIO3(GUIO_NOSUBTITLES, GUIO_NOSPEECH, GAMEOPTION_COPY_PROTECTION)
	},
	kFeaturesAdLib,
	0, 0, 0
},
{ // Supplied by goodoldgeorg in bug report #4163
	{
		"gob2",
		"",
		AD_ENTRY1("intro.stk", "c47faf1d406504e6ffe63243610bb1f4"),
		IT_ITA,
		kPlatformDOS,
		ADGF_NO_FLAGS,
		GUIO3(GUIO_NOSUBTITLES, GUIO_NOSPEECH, GAMEOPTION_COPY_PROTECTION)
	},
	kFeaturesAdLib,
	0, 0, 0
},
{
	{
		"gob2",
		"v1.02",
		AD_ENTRY1("intro.stk", "cd3e1df8b273636ee32e34b7064f50e8"),
		RU_RUS,
		kPlatformDOS,
		ADGF_NO_FLAGS,
		GUIO3(GUIO_NOSUBTITLES, GUIO_NOSPEECH, GAMEOPTION_COPY_PROTECTION)
	},
	kFeaturesAdLib,
	0, 0, 0
},
{ // Supplied by arcepi in bug report #3060
	{
		"gob2",
		"",
		AD_ENTRY1s("intro.stk", "5f53c56e3aa2f1e76c2e4f0caa15887f", 829232),
		ES_ESP,
		kPlatformDOS,
		ADGF_NO_FLAGS,
		GUIO3(GUIO_NOSUBTITLES, GUIO_NOSPEECH, GAMEOPTION_COPY_PROTECTION)
	},
	kFeaturesAdLib,
	0, 0, 0
},

// -- DOS VGA CD --

{
	{
		"gob2",
		"CD v1.000",
		AD_ENTRY1("intro.stk", "9de5fbb41cf97182109e5fecc9d90347"),
		EN_USA,
		kPlatformDOS,
		ADGF_CD,
		GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
	},
	kFeaturesCD,
	0, 0, 0
},
{ // Supplied by pykman in bug report #5365
	{
		"gob2",
		"CD v2.01",
		AD_ENTRY1s("intro.stk", "3025f05482b646c18c2c79c615a3a1df", 5011726),
		PL_POL,
		kPlatformDOS,
		ADGF_CD,
		GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
	},
	kFeaturesCD,
	0, 0, 0
},
{ // Supplied by pykman in bug report #5365
	{
		"gob2",
		"CD v1.02",
		AD_ENTRY1s("intro.stk", "978afddcac81bb95a04757b61f78471c", 619825),
		PL_POL,
		kPlatformDOS,
		ADGF_CD,
		GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
	},
	kFeaturesCD,
	0, 0, 0
},
{
	{
		"gob2",
		"CD v2.01",
		AD_ENTRY1("intro.stk", "24a6b32757752ccb1917ce92fd7c2a04"),
		EN_ANY,
		kPlatformDOS,
		ADGF_CD,
		GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
	},
	kFeaturesCD,
	0, 0, 0
},
{
	{
		"gob2",
		"CD v2.01",
		AD_ENTRY1("intro.stk", "24a6b32757752ccb1917ce92fd7c2a04"),
		DE_DEU,
		kPlatformDOS,
		ADGF_CD,
		GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
	},
	kFeaturesCD,
	0, 0, 0
},
{
	{
		"gob2",
		"CD v2.01",
		AD_ENTRY1("intro.stk", "24a6b32757752ccb1917ce92fd7c2a04"),
		FR_FRA,
		kPlatformDOS,
		ADGF_CD,
		GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
	},
	kFeaturesCD,
	0, 0, 0
},
{
	{
		"gob2",
		"CD v2.01",
		AD_ENTRY1("intro.stk", "24a6b32757752ccb1917ce92fd7c2a04"),
		IT_ITA,
		kPlatformDOS,
		ADGF_CD,
		GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
	},
	kFeaturesCD,
	0, 0, 0
},
{
	{
		"gob2",
		"CD v2.01",
		AD_ENTRY1("intro.stk", "24a6b32757752ccb1917ce92fd7c2a04"),
		ES_ESP,
		kPlatformDOS,
		ADGF_CD,
		GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
	},
	kFeaturesCD,
	0, 0, 0
},
{ // Hebrew fan translation
	{
		"gob2",
		"CD v2.01",
		AD_ENTRY1s("intro.stk", "b768039f8d0a12c39ca28dcd33d584ba", 4696209),
		HE_ISR,
		kPlatformDOS,
		ADGF_CD,
		GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
	},
	kFeaturesCD,
	0, 0, 0
},
{ // Supplied by goodoldgeorg in bug report #4375
	{
		"gob2",
		"CD v1.02",
		AD_ENTRY1s("intro.stk", "5ba85a4769a1ab03a283dd694588d526", 5006236),
		HU_HUN,
		kPlatformDOS,
		ADGF_CD,
		GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
	},
	kFeaturesCD,
	0, 0, 0
},
{ // Supplied by goodoldgeorg in bug report #4375
	{
		"gob2",
		"CD v1.02",
		AD_ENTRY1s("intro.stk", "5ba85a4769a1ab03a283dd694588d526", 5006236),
		FR_FRA,
		kPlatformDOS,
		ADGF_CD,
		GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
	},
	kFeaturesCD,
	0, 0, 0
},
{ // Supplied by goodoldgeorg in bug report #4375
	{
		"gob2",
		"CD v1.02",
		AD_ENTRY1s("intro.stk", "5ba85a4769a1ab03a283dd694588d526", 5006236),
		DE_DEU,
		kPlatformDOS,
		ADGF_CD,
		GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
	},
	kFeaturesCD,
	0, 0, 0
},
{ // Supplied by goodoldgeorg in bug report #4375
	{
		"gob2",
		"CD v1.02",
		AD_ENTRY1s("intro.stk", "5ba85a4769a1ab03a283dd694588d526", 5006236),
		ES_ESP,
		kPlatformDOS,
		ADGF_CD,
		GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
	},
	kFeaturesCD,
	0, 0, 0
},
{ // Supplied by goodoldgeorg in bug report #4375
	{
		"gob2",
		"CD v1.02",
		AD_ENTRY1s("intro.stk", "5ba85a4769a1ab03a283dd694588d526", 5006236),
		IT_ITA,
		kPlatformDOS,
		ADGF_CD,
		GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
	},
	kFeaturesCD,
	0, 0, 0
},

// -- Windows --

{
	{
		"gob2",
		"v1.02",
		AD_ENTRY2s("intro.stk", "285d7340f98ebad65d465585da12910b", 837286,
				   "musmac1.mid", "834e55205b710d0af5f14a6f2320dd8e", 8661),
		FR_FRA,
		kPlatformWindows,
		ADGF_NO_FLAGS,
		GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
	},
	kFeaturesAdLib,
	0, 0, 0
},
{
	{
		"gob2",
		"v1.02",
		AD_ENTRY2s("intro.stk", "25a99827cd59751a80bed9620fb677a0", 893302,
				   "musmac1.mid", "834e55205b710d0af5f14a6f2320dd8e", 8661),
		EN_USA,
		kPlatformWindows,
		ADGF_NO_FLAGS,
		GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
	},
	kFeaturesAdLib,
	0, 0, 0
},
{
	{
		"gob2",
		"v1.02",
		AD_ENTRY2s("intro.stk", "25a99827cd59751a80bed9620fb677a0", 893302,
				   "musmac1.mid", "834e55205b710d0af5f14a6f2320dd8e", 8661),
		FR_FRA,
		kPlatformWindows,
		ADGF_NO_FLAGS,
		GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
	},
	kFeaturesAdLib,
	0, 0, 0
},
{
	{
		"gob2",
		"v1.02",
		AD_ENTRY2s("intro.stk", "25a99827cd59751a80bed9620fb677a0", 893302,
				   "musmac1.mid", "834e55205b710d0af5f14a6f2320dd8e", 8661),
		DE_DEU,
		kPlatformWindows,
		ADGF_NO_FLAGS,
		GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
	},
	kFeaturesAdLib,
	0, 0, 0
},
{
	{
		"gob2",
		"v1.02",
		AD_ENTRY2s("intro.stk", "6efac0a14c0de4d57dde8592456c8acf", 845172,
				   "musmac1.mid", "834e55205b710d0af5f14a6f2320dd8e", 8661),
		EN_USA,
		kPlatformWindows,
		ADGF_NO_FLAGS,
		GUIO3(GUIO_NOSUBTITLES, GUIO_NOSPEECH, GAMEOPTION_COPY_PROTECTION)
	},
	kFeaturesAdLib,
	0, 0, 0
},
{
	{
		"gob2",
		"v1.02",
		AD_ENTRY2s("intro.stk", "6efac0a14c0de4d57dde8592456c8acf", 845172,
				   "musmac1.mid", "834e55205b710d0af5f14a6f2320dd8e", 8661),
		FR_FRA,
		kPlatformWindows,
		ADGF_NO_FLAGS,
		GUIO3(GUIO_NOSUBTITLES, GUIO_NOSPEECH, GAMEOPTION_COPY_PROTECTION)
	},
	kFeaturesAdLib,
	0, 0, 0
},
{ // Found in french ADI 2 Francais-Maths CM1
	{
		"gob2",
		"v1.03",
		AD_ENTRY1s("intro.stk", "24489330a1d67ff978211f574822a5a6", 883756),
		FR_FRA,
		kPlatformWindows,
		ADGF_NO_FLAGS,
		GUIO3(GUIO_NOSUBTITLES, GUIO_NOSPEECH, GAMEOPTION_COPY_PROTECTION)
	},
	kFeaturesAdLib,
	0, 0, 0
},
{ // Found in french ADI 2.5 Anglais Multimedia 5e
	{
		"gob2",
		"v1.02",
		AD_ENTRY1s("intro.stk", "285d7340f98ebad65d465585da12910b", 837286),
		FR_FRA,
		kPlatformWindows,
		ADGF_NO_FLAGS,
		GUIO3(GUIO_NOSUBTITLES, GUIO_NOSPEECH, GAMEOPTION_COPY_PROTECTION)
	},
	kFeaturesAdLib,
	0, 0, 0
},
{ // Included in a German version of Adi 2
	{
		"gob2",
		"v1.03",
		AD_ENTRY1s("intro.stk", "271863a3dfc27665fac4b3589a0e735f", 947966),
		DE_DEU,
		kPlatformWindows,
		ADGF_NO_FLAGS,
		GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
	},
	kFeaturesAdLib,
	0, 0, 0
},

// -- Mac --

{ // Supplied by fac76 in bug report #3108
	{
		"gob2",
		"v1.02",
		AD_ENTRY2s("intro.stk", "b45b984ee8017efd6ea965b9becd4d66", 828443,
				   "musmac1.mid", "7f96f491448c7a001b32df89cf8d2af2", 1658),
		EN_ANY,
		kPlatformMacintosh,
		ADGF_NO_FLAGS,
		GUIO3(GUIO_NOSUBTITLES, GUIO_NOSPEECH, GAMEOPTION_COPY_PROTECTION)
	},
	kFeaturesAdLib,
	0, 0, 0
},
{ // Supplied by koalet in bug report #4064
	{
		"gob2",
		"",
		AD_ENTRY2s("intro.stk", "a13ecb4f6d8fd881ebbcc02e45cb5475", 837275,
				   "musmac1.mid", "7f96f491448c7a001b32df89cf8d2af2", 1658),
		FR_FRA,
		kPlatformMacintosh,
		ADGF_NO_FLAGS,
		GUIO3(GUIO_NOSUBTITLES, GUIO_NOSPEECH, GAMEOPTION_COPY_PROTECTION)
	},
	kFeaturesAdLib,
	0, 0, 0
},

// -- Amiga --

{ // Supplied by fac76 in bug report #3608
	{
		"gob2",
		"",
		AD_ENTRY1s("intro.stk", "eebf2810122cfd17399260cd1468e994", 554014),
		EN_GRB,
		kPlatformAmiga,
		ADGF_NO_FLAGS,
		GUIO3(GUIO_NOSUBTITLES, GUIO_NOSPEECH, GAMEOPTION_COPY_PROTECTION)
	},
	kFeaturesNone,
	0, 0, 0
},
{
	{
		"gob2",
		"",
		AD_ENTRY1("intro.stk", "d28b9e9b41f31acfa58dcd12406c7b2c"),
		DE_DEU,
		kPlatformAmiga,
		ADGF_NO_FLAGS,
		GUIO3(GUIO_NOSUBTITLES, GUIO_NOSPEECH, GAMEOPTION_COPY_PROTECTION)
	},
	kFeaturesNone,
	0, 0, 0
},
{ // Supplied by goodoldgeorg in bug report #4164
	{
		"gob2",
		"",
		AD_ENTRY1("intro.stk", "686c88f7302a80b744aae9f8413e853d"),
		IT_ITA,
		kPlatformAmiga,
		ADGF_NO_FLAGS,
		GUIO3(GUIO_NOSUBTITLES, GUIO_NOSPEECH, GAMEOPTION_COPY_PROTECTION)
	},
	kFeaturesNone,
	0, 0, 0
},
{ // Supplied by aldozx in the forums
	{
		"gob2",
		"",
		AD_ENTRY1s("intro.stk", "abc3e786cd78197773954c75815b278b", 554721),
		ES_ESP,
		kPlatformAmiga,
		ADGF_NO_FLAGS,
		GUIO3(GUIO_NOSUBTITLES, GUIO_NOSPEECH, GAMEOPTION_COPY_PROTECTION)
	},
	kFeaturesNone,
	0, 0, 0
},
{ // Supplied by damsoftPL in bug report #12033
	{
		"gob2",
		"",
		AD_ENTRY1s("intro.stk", "d721383633b7acd6f18752e1ad217473", 559840),
		PL_POL,
		kPlatformAmiga,
		ADGF_NO_FLAGS,
		GUIO3(GUIO_NOSUBTITLES, GUIO_NOSPEECH, GAMEOPTION_COPY_PROTECTION)
	},
	kFeaturesNone,
	0, 0, 0
},


// -- Atari ST --

{ // Supplied by bgk in bug report #3161
	{
		"gob2",
		"",
		AD_ENTRY1s("intro.stk", "4b13c02d1069b86bcfec80f4e474b98b", 554680),
		FR_FRA,
		kPlatformAtariST,
		ADGF_NO_FLAGS,
		GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
	},
	kFeaturesNone,
	0, 0, 0
},

// -- Demos --

{
	{
		"gob2",
		"Non-Interactive Demo",
		AD_ENTRY1("intro.stk", "8b1c98ff2ab2e14f47a1b891e9b92217"),
		EN_GRB,
		kPlatformDOS,
		ADGF_DEMO,
		GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
	},
	kFeaturesAdLib,
	0, "usa.tot", 0
},
{
	{
		"gob2",
		"Interactive Demo (v1.01)",
		AD_ENTRY1s("intro.stk", "cf1c95b2939bd8ff58a25c756cb6125e", 492226),
		EN_GRB,
		kPlatformDOS,
		ADGF_DEMO,
		GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
	},
	kFeaturesAdLib,
	0, 0, 0
},
{
	{
		"gob2",
		"Interactive Demo (v1.02)",
		AD_ENTRY1s("intro.stk", "4b278c2678ea01383fd5ca114d947eea", 575920),
		EN_GRB,
		kPlatformAmiga,
		ADGF_DEMO,
		GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
	},
	kFeaturesNone,
	0, 0, 0
},
{ // Supplied by polluks in bug report #3628
	{
		"gob2",
		"Interactive Demo (v1.0)",
		AD_ENTRY1s("intro.stk", "9fa85aea959fa8c582085855fbd99346", 553063),
		EN_GRB,
		kPlatformAmiga,
		ADGF_DEMO,
		GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
	},
	kFeaturesNone,
	0, 0, 0
},

#endif // GOB_DETECTION_TABLES_GOB2_H
