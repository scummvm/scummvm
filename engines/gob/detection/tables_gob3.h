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

/* Detection tables for Goblins 3 / Goblins Quest 3. */

#ifndef GOB_DETECTION_TABLES_GOB3_H
#define GOB_DETECTION_TABLES_GOB3_H

// -- DOS VGA Floppy --

{
	{
		"gob3",
		"v1.00",
		AD_ENTRY1s("intro.stk", "32b0f57f5ae79a9ae97e8011df38af42", 157084),
		EN_GRB,
		kPlatformDOS,
		ADGF_NO_FLAGS,
		GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
	},
	kFeaturesAdLib,
	0, 0, 0
},
{
	{
		"gob3",
		"",
		AD_ENTRY1s("intro.stk", "904fc32032295baa3efb3a41f17db611", 178582),
		HE_ISR,
		kPlatformDOS,
		ADGF_NO_FLAGS,
		GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
	},
	kFeaturesAdLib,
	0, 0, 0
},
{ // Supplied by raziel_ in bug report #3622
	{
		"gob3",
		"v0.50",
		AD_ENTRY1s("intro.stk", "16b014bf32dbd6ab4c5163c44f56fed1", 445104),
		EN_GRB,
		kPlatformDOS,
		ADGF_NO_FLAGS,
		GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
	},
	kFeaturesAdLib,
	0, 0, 0
},
{ // dated March 17, 1994
	{
		"gob3",
		"v1.00",
		AD_ENTRY1s("intro.stk", "1e2f64ec8dfa89f42ee49936a27e66e7", 159444),
		EN_USA,
		kPlatformDOS,
		ADGF_NO_FLAGS,
		GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
	},
	kFeaturesAdLib,
	0, 0, 0
},
{ // Supplied by paul66 in bug report #3045
	{
		"gob3",
		"",
		AD_ENTRY1s("intro.stk", "f6d225b25a180606fa5dbe6405c97380", 161516),
		DE_DEU,
		kPlatformDOS,
		ADGF_NO_FLAGS,
		GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
	},
	kFeaturesAdLib,
	0, 0, 0
},
{
	{
		"gob3",
		"",
		AD_ENTRY1("intro.stk", "e42a4f2337d6549487a80864d7826972"),
		FR_FRA,
		kPlatformDOS,
		ADGF_NO_FLAGS,
		GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
	},
	kFeaturesAdLib,
	0, 0, 0
},
{ // Supplied by Paranoimia on #scummvm
	{
		"gob3",
		"",
		AD_ENTRY1s("intro.stk", "fe8144daece35538085adb59c2d29613", 159402),
		IT_ITA,
		kPlatformDOS,
		ADGF_NO_FLAGS,
		GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
	},
	kFeaturesAdLib,
	0, 0, 0
},
{
	{
		"gob3",
		"v0.50",
		AD_ENTRY1s("intro.stk", "4e3af248a48a2321364736afab868527", 204265),
		RU_RUS,
		kPlatformDOS,
		ADGF_NO_FLAGS,
		GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
	},
	kFeaturesAdLib,
	0, 0, 0
},
{
	{
		"gob3",
		"",
		AD_ENTRY1("intro.stk", "8d28ce1591b0e9cc79bf41cad0fc4c9c"),
		UNK_LANG,
		kPlatformDOS,
		ADGF_NO_FLAGS,
		GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
	},
	kFeaturesAdLib,
	0, 0, 0
},
{ // Supplied by SiRoCs in bug report #3949
	{
		"gob3",
		"v1.00",
		AD_ENTRY1s("intro.stk", "d3b72938fbbc8159198088811f9e6d19", 160382),
		ES_ESP,
		kPlatformDOS,
		ADGF_NO_FLAGS,
		GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
	},
	kFeaturesAdLib,
	0, 0, 0
},

// -- Windows --

{
	{
		"gob3",
		"v1.00",
		AD_ENTRY2s("intro.stk", "16b014bf32dbd6ab4c5163c44f56fed1", 445104,
                   "musmac1.mid", "948c546cad3a9de5bff3fe4107c82bf1", 6404),
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
		"gob3",
		"v1.00",
		AD_ENTRY2s("intro.stk",   "16b014bf32dbd6ab4c5163c44f56fed1", 445104,
		           "musmac1.mid", "948c546cad3a9de5bff3fe4107c82bf1", 6404),
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
		"gob3",
		"v1.00",
		AD_ENTRY2s("intro.stk", "16b014bf32dbd6ab4c5163c44f56fed1", 445104,
				   "musmac1.mid", "948c546cad3a9de5bff3fe4107c82bf1", 6404),
		EN_GRB,
		kPlatformWindows,
		ADGF_NO_FLAGS,
		GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
	},
	kFeaturesAdLib,
	0, 0, 0
},
{
	{
		"gob3",
		"v1.00",
		AD_ENTRY2s("intro.stk", "edd7403e5dc2a14459d2665a4c17714d", 209534,
				   "musmac1.mid", "948c546cad3a9de5bff3fe4107c82bf1", 6404),
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
		"gob3",
		"",
		AD_ENTRY2s("intro.stk", "428e2de130cf3b303c938924539dc50d", 324420,
				   "musmac1.mid", "948c546cad3a9de5bff3fe4107c82bf1", 6404),
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
		"gob3",
		"",
		AD_ENTRY2s("intro.stk", "428e2de130cf3b303c938924539dc50d", 324420,
				   "musmac1.mid", "948c546cad3a9de5bff3fe4107c82bf1", 6404),
		EN_ANY,
		kPlatformWindows,
		ADGF_NO_FLAGS,
		GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
	},
	kFeaturesAdLib,
	0, 0, 0
},
{ // Found in Found in french ADI 2.5 Anglais Multimedia 5e
	{
		"gob3",
		"v1.00",
		AD_ENTRY1s("intro.stk", "edd7403e5dc2a14459d2665a4c17714d", 209534),
		FR_FRA,
		kPlatformWindows,
		ADGF_NO_FLAGS,
		GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
	},
	kFeaturesAdLib,
	0, 0, 0
},

// -- Mac --

{ // Supplied by fac76 in bug report #3272
	{
		"gob3",
		"v1.00",
		AD_ENTRY2s("intro.stk", "32b0f57f5ae79a9ae97e8011df38af42", 157084,
		           "musmac1.mid", "834e55205b710d0af5f14a6f2320dd8e", 8661),
		EN_GRB,
		kPlatformMacintosh,
		ADGF_NO_FLAGS,
		GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
	},
	kFeaturesAdLib,
	0, 0, 0
},

// -- Amiga --

{
	{
		"gob3",
		"",
		AD_ENTRY1s("intro.stk", "bd679eafde2084d8011f247e51b5a805", 197532),
		EN_GRB,
		kPlatformAmiga,
		ADGF_NO_FLAGS,
		GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
	},
	kFeaturesNone,
	0, "menu.tot", 0
},
{
	{
		"gob3",
		"",
		AD_ENTRY1s("intro.stk", "bd679eafde2084d8011f247e51b5a805", 197532),
		DE_DEU,
		kPlatformAmiga,
		ADGF_NO_FLAGS,
		GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
	},
	kFeaturesNone,
	0, "menu.tot", 0
},

// -- DOS VGA CD --

{
	{
		"gob3",
		"CD v1.000",
		AD_ENTRY1("intro.stk", "6f2c226c62dd7ab0ab6f850e89d3fc47"),
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
		"gob3",
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
{ // Supplied by paul66 and noizert in bug reports #3045 and #3137
	{
		"gob3",
		"CD v1.02",
		AD_ENTRY1s("intro.stk", "c3e9132ea9dc0fb866b6d60dcda10261", 612482),
		EN_ANY,
		kPlatformDOS,
		ADGF_CD,
		GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
	},
	kFeaturesCD,
	0, 0, 0
},
{ // Supplied by paul66 and noizert in bug reports #3045 and #3137
	{
		"gob3",
		"CD v1.02",
		AD_ENTRY1s("intro.stk", "c3e9132ea9dc0fb866b6d60dcda10261", 612482),
		DE_DEU,
		kPlatformDOS,
		ADGF_CD,
		GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
	},
	kFeaturesCD,
	0, 0, 0
},
{ // Supplied by paul66 and noizert in bug reports #3045 and #3137
	{
		"gob3",
		"CD v1.02",
		AD_ENTRY1s("intro.stk", "c3e9132ea9dc0fb866b6d60dcda10261", 612482),
		FR_FRA,
		kPlatformDOS,
		ADGF_CD,
		GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
	},
	kFeaturesCD,
	0, 0, 0
},
{ // Supplied by paul66 and noizert in bug reports #3045 and #3137
	{
		"gob3",
		"CD v1.02",
		AD_ENTRY1s("intro.stk", "c3e9132ea9dc0fb866b6d60dcda10261", 612482),
		IT_ITA,
		kPlatformDOS,
		ADGF_CD,
		GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
	},
	kFeaturesCD,
	0, 0, 0
},
{ // Supplied by paul66 and noizert in bug reports #3045 and #3137
	{
		"gob3",
		"CD v1.02",
		AD_ENTRY1s("intro.stk", "c3e9132ea9dc0fb866b6d60dcda10261", 612482),
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
		"gob3",
		"CD v1.02",
		AD_ENTRY1s("intro.stk", "bfd7d4c6fedeb2cfcc8baa4d5ddb1f74", 616220),
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
		"gob3",
		"CD v1.02",
		AD_ENTRY1s("intro.stk", "bfd7d4c6fedeb2cfcc8baa4d5ddb1f74", 616220),
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
		"gob3",
		"CD v1.02",
		AD_ENTRY1s("intro.stk", "bfd7d4c6fedeb2cfcc8baa4d5ddb1f74", 616220),
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
		"gob3",
		"CD v1.02",
		AD_ENTRY1s("intro.stk", "bfd7d4c6fedeb2cfcc8baa4d5ddb1f74", 616220),
		ES_ESP,
		kPlatformDOS,
		ADGF_CD,
		GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
	},
	kFeaturesCD,
	0, 0, 0
},

// -- Demos --

{
	{
		"gob3",
		"Non-interactive Demo",
		AD_ENTRY1s("intro.stk", "b9b898fccebe02b69c086052d5024a55", 600143),
		FR_FRA,
		kPlatformDOS,
		ADGF_DEMO,
		GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
	},
	kFeaturesAdLib,
	0, 0, 0
},
{
	{
		"gob3",
		"Interactive Demo (v0.02)",
		AD_ENTRY1s("intro.stk", "7aebd94e49c2c5c518c9e7b74f25de9d", 270737),
		FR_FRA,
		kPlatformDOS,
		ADGF_DEMO,
		GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
	},
	kFeaturesAdLib,
	0, 0, 0
},
{
	{
		"gob3",
		"Interactive Demo 2 (v0.02)",
		AD_ENTRY1s("intro.stk", "e5dcbc9f6658ebb1e8fe26bc4da0806d", 590631),
		FR_FRA,
		kPlatformDOS,
		ADGF_DEMO,
		GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
	},
	kFeaturesAdLib,
	0, 0, 0
},
{
	{
		"gob3",
		"Interactive Demo 3 (v0.02)",
		AD_ENTRY1s("intro.stk", "9e20ad7b471b01f84db526da34eaf0a2", 395561),
		EN_ANY,
		kPlatformDOS,
		ADGF_DEMO,
		GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
	},
	kFeaturesAdLib,
	0, 0, 0
},
{ // Supplied by Dark-Star on #scummvm
	{
		"gob3",
		"Interactive Demo 4",
		AD_ENTRY1s("intro.stk", "9c7c9002506fc976128ffe8f308d428c", 395562),
		EN_GRB,
		kPlatformDOS,
		ADGF_DEMO,
		GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
	},
	kFeaturesAdLib,
	0, 0, 0
},
{ // Supplied by Dark-Star on #scummvm
	{
		"gob3",
		"Interactive Demo 4",
		AD_ENTRY1s("intro.stk", "9c7c9002506fc976128ffe8f308d428c", 395562),
		DE_DEU,
		kPlatformDOS,
		ADGF_DEMO,
		GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
	},
	kFeaturesAdLib,
	0, 0, 0
},
{ // Supplied by Dark-Star on #scummvm
	{
		"gob3",
		"Interactive Demo 4",
		AD_ENTRY1s("intro.stk", "9c7c9002506fc976128ffe8f308d428c", 395562),
		FR_FRA,
		kPlatformDOS,
		ADGF_DEMO,
		GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
	},
	kFeaturesAdLib,
	0, 0, 0
},

#endif // GOB_DETECTION_TABLES_GOB3_H
