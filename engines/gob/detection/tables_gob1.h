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

/* Detection tables for Gobliiins. */

#ifndef GOB_DETECTION_TABLES_GOB1_H
#define GOB_DETECTION_TABLES_GOB1_H

// -- DOS EGA Floppy --

{ // Supplied by Florian Zeitz on scummvm-devel
	{
		"gob1",
		"EGA",
		AD_ENTRY1s("intro.stk", "c65e9cc8ba23a38456242e1f2b1caad4", 135561),
		UNK_LANG,
		kPlatformDOS,
		ADGF_NO_FLAGS,
		GUIO3(GUIO_NOSUBTITLES, GUIO_NOSPEECH, GAMEOPTION_COPY_PROTECTION)
	},
	kFeaturesEGA | kFeaturesAdLib,
	0, 0, 0
},
{
	{
		"gob1",
		"EGA",
		AD_ENTRY1("intro.stk", "f9233283a0be2464248d83e14b95f09c"),
		RU_RUS,
		kPlatformDOS,
		ADGF_NO_FLAGS,
		GUIO3(GUIO_NOSUBTITLES, GUIO_NOSPEECH, GAMEOPTION_COPY_PROTECTION)
	},
	kFeaturesEGA | kFeaturesAdLib,
	0, 0, 0
},

// -- DOS VGA Floppy --

{ // Supplied by Theruler76 in bug report #2024
	{
		"gob1",
		"VGA",
		AD_ENTRY1("intro.stk", "26a9118c0770fa5ac93a9626761600b2"),
		EN_ANY,
		kPlatformDOS,
		ADGF_NO_FLAGS,
		GUIO3(GUIO_NOSUBTITLES, GUIO_NOSPEECH, GAMEOPTION_COPY_PROTECTION)
	},
	kFeaturesNone,
	0, 0, 0
},
{ // Supplied by raziel_ in bug report #3620
	{
		"gob1",
		"VGA",
		AD_ENTRY1s("intro.stk", "e157cb59c6d330ca70d12ab0ef1dd12b", 288972),
		EN_GRB,
		kPlatformDOS,
		ADGF_NO_FLAGS,
		GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
	},
	kFeaturesAdLib,
	0, 0, 0
},

// -- DOS VGA CD --

{ // Provided by pykman in the forums.
	{
		"gob1",
		"CD",
		AD_ENTRY1s("intro.stk", "97d2443948b2e367cf567fe7e101f5f2", 4049267),
		PL_POL,
		kPlatformDOS,
		ADGF_CD,
		GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
	},
	kFeaturesCD,
	0, 0, 0
},
{ // CD 1.000 version.
	{
		"gob1",
		"CD v1.000",
		AD_ENTRY1("intro.stk", "2fbf4b5b82bbaee87eb45d4404c28998"),
		EN_USA,
		kPlatformDOS,
		ADGF_CD,
		GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
	},
	kFeaturesCD,
	0, 0, 0
},
{ // CD 1.000 version.
	{
		"gob1",
		"CD v1.000",
		AD_ENTRY1("intro.stk", "2fbf4b5b82bbaee87eb45d4404c28998"),
		DE_DEU,
		kPlatformDOS,
		ADGF_CD,
		GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
	},
	kFeaturesCD,
	0, 0, 0
},
{ // CD 1.000 version.
	{
		"gob1",
		"CD v1.000",
		AD_ENTRY1("intro.stk", "2fbf4b5b82bbaee87eb45d4404c28998"),
		FR_FRA,
		kPlatformDOS,
		ADGF_CD,
		GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
	},
	kFeaturesCD,
	0, 0, 0
},
{ // CD 1.000 version.
	{
		"gob1",
		"CD v1.000",
		AD_ENTRY1("intro.stk", "2fbf4b5b82bbaee87eb45d4404c28998"),
		IT_ITA,
		kPlatformDOS,
		ADGF_CD,
		GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
	},
	kFeaturesCD,
	0, 0, 0
},
{ // CD 1.000 version.
	{
		"gob1",
		"CD v1.000",
		AD_ENTRY1("intro.stk", "2fbf4b5b82bbaee87eb45d4404c28998"),
		ES_ESP,
		kPlatformDOS,
		ADGF_CD,
		GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
	},
	kFeaturesCD,
	0, 0, 0
},
{ // CD 1.02 version. Multilingual
	{
		"gob1",
		"CD v1.02",
		AD_ENTRY1s("intro.stk", "8bd873137b6831c896ee8ad217a6a398", 3295368),
		EN_USA,
		kPlatformDOS,
		ADGF_CD,
		GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
	},
	kFeaturesCD,
	0, 0, 0
},
{ // CD 1.02 version. Multilingual
	{
		"gob1",
		"CD v1.02",
		AD_ENTRY1s("intro.stk", "8bd873137b6831c896ee8ad217a6a398", 3295368),
		DE_DEU,
		kPlatformDOS,
		ADGF_CD,
		GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
	},
	kFeaturesCD,
	0, 0, 0
},
{ // CD 1.02 version. Multilingual
	{
		"gob1",
		"CD v1.02",
		AD_ENTRY1s("intro.stk", "8bd873137b6831c896ee8ad217a6a398", 3295368),
		FR_FRA,
		kPlatformDOS,
		ADGF_CD,
		GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
	},
	kFeaturesCD,
	0, 0, 0
},
{ // CD 1.02 version. Multilingual
	{
		"gob1",
		"CD v1.02",
		AD_ENTRY1s("intro.stk", "8bd873137b6831c896ee8ad217a6a398", 3295368),
		IT_ITA,
		kPlatformDOS,
		ADGF_CD,
		GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
	},
	kFeaturesCD,
	0, 0, 0
},
{ // CD 1.02 version. Multilingual
	{
		"gob1",
		"CD v1.02",
		AD_ENTRY1s("intro.stk", "8bd873137b6831c896ee8ad217a6a398", 3295368),
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
		"gob1",
		"CD v1.02",
		AD_ENTRY1s("intro.stk", "40d4a53818f4fce3f5997d02c3fafe73", 4049248),
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
		"gob1",
		"CD v1.02",
		AD_ENTRY1s("intro.stk", "40d4a53818f4fce3f5997d02c3fafe73", 4049248),
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
		"gob1",
		"CD v1.02",
		AD_ENTRY1s("intro.stk", "40d4a53818f4fce3f5997d02c3fafe73", 4049248),
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
		"gob1",
		"CD v1.02",
		AD_ENTRY1s("intro.stk", "40d4a53818f4fce3f5997d02c3fafe73", 4049248),
		IT_ITA,
		kPlatformDOS,
		ADGF_CD,
		GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
	},
	kFeaturesCD,
	0, 0, 0
},

// -- Mac --

{ // Supplied by raina in the forums
	{
		"gob1",
		"",
		AD_ENTRY1s("intro.stk", "6d837c6380d8f4d984c9f6cc0026df4f", 192712),
		EN_ANY,
		kPlatformMacintosh,
		ADGF_NO_FLAGS,
		GUIO3(GUIO_NOSUBTITLES, GUIO_NOSPEECH, GAMEOPTION_COPY_PROTECTION)
	},
	kFeaturesNone,
	0, 0, 0
},
{ // Supplied by paul66 in bug report #3045
	{
		"gob1",
		"",
		AD_ENTRY1("intro.stk", "00a42a7d2d22e6b6ab1b8c673c4ed267"),
		EN_ANY,
		kPlatformMacintosh,
		ADGF_NO_FLAGS,
		GUIO3(GUIO_NOSUBTITLES, GUIO_NOSPEECH, GAMEOPTION_COPY_PROTECTION)
	},
	kFeaturesAdLib,
	0, 0, 0
},
{ // Supplied by paul66 in bug report #3045
	{
		"gob1",
		"",
		AD_ENTRY1("intro.stk", "00a42a7d2d22e6b6ab1b8c673c4ed267"),
		DE_DEU,
		kPlatformMacintosh,
		ADGF_NO_FLAGS,
		GUIO3(GUIO_NOSUBTITLES, GUIO_NOSPEECH, GAMEOPTION_COPY_PROTECTION)
	},
	kFeaturesAdLib,
	0, 0, 0
},
{ // Supplied by paul66 in bug report #3045
	{
		"gob1",
		"",
		AD_ENTRY1("intro.stk", "00a42a7d2d22e6b6ab1b8c673c4ed267"),
		FR_FRA,
		kPlatformMacintosh,
		ADGF_NO_FLAGS,
		GUIO3(GUIO_NOSUBTITLES, GUIO_NOSPEECH, GAMEOPTION_COPY_PROTECTION)
	},
	kFeaturesAdLib,
	0, 0, 0
},
{ // Supplied by paul66 in bug report #3045
	{
		"gob1",
		"",
		AD_ENTRY1("intro.stk", "00a42a7d2d22e6b6ab1b8c673c4ed267"),
		IT_ITA,
		kPlatformMacintosh,
		ADGF_NO_FLAGS,
		GUIO3(GUIO_NOSUBTITLES, GUIO_NOSPEECH, GAMEOPTION_COPY_PROTECTION)
	},
	kFeaturesAdLib,
	0, 0, 0
},
{ // Supplied by paul66 in bug report #3045
	{
		"gob1",
		"",
		AD_ENTRY1("intro.stk", "00a42a7d2d22e6b6ab1b8c673c4ed267"),
		ES_ESP,
		kPlatformMacintosh,
		ADGF_NO_FLAGS,
		GUIO3(GUIO_NOSUBTITLES, GUIO_NOSPEECH, GAMEOPTION_COPY_PROTECTION)
	},
	kFeaturesAdLib,
	0, 0, 0
},
{
	{
		"gob1",
		"",
		AD_ENTRY1s("intro.stk", "acdda40f4b20a87d4cfd760d3833a6e1", 453404),
		JA_JPN,
		kPlatformMacintosh,
		ADGF_UNSTABLE,
		GUIO3(GUIO_NOSUBTITLES, GUIO_NOSPEECH, GAMEOPTION_COPY_PROTECTION)
	},
	kFeaturesAdLib,
	0, 0, 0
},

{ // Supplied by Svipur in bug report #14531 (Adapted to CD by A.P.$lasH)
	{
		"gob1",
		"CD adaptatiton",
		AD_ENTRY1s("intro.stk", "dd3975b66f37d2f360f34ee1f83041f1", 3231773),
		RU_RUS,
		kPlatformDOS,
		ADGF_CD,
		GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
	},
	kFeaturesCD,
	0, 0, 0
},

// -- Windows --

{ // Supplied by Hkz on #scummvm
	{
		"gob1",
		"",
		AD_ENTRY2s("intro.stk",   "f5f028ee39c456fa51fa63b606583918", 313472,
				   "musmac1.mid", "4f66903b33df8a20edd4c748809c0b56", 8161),
		FR_FRA,
		kPlatformWindows,
		ADGF_NO_FLAGS,
		GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
	},
	kFeaturesAdLib,
	0, 0, 0
},
{ // Supplied by Hkz on #scummvm
	{
		"gob1",
		"",
		AD_ENTRY2s("intro.stk",   "f5f028ee39c456fa51fa63b606583918", 313472,
			       "musmac1.mid", "4f66903b33df8a20edd4c748809c0b56", 8161),
		IT_ITA,
		kPlatformWindows,
		ADGF_NO_FLAGS,
		GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
	},
	kFeaturesAdLib,
	0, 0, 0
},
{ // Supplied by Hkz on #scummvm
	{
		"gob1",
		"",
		AD_ENTRY2s("intro.stk",   "f5f028ee39c456fa51fa63b606583918", 313472,
				   "musmac1.mid", "4f66903b33df8a20edd4c748809c0b56", 8161),
		EN_GRB,
		kPlatformWindows,
		ADGF_NO_FLAGS,
		GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
	},
	kFeaturesAdLib,
	0, 0, 0
},
{ // Supplied by Hkz on #scummvm
	{
		"gob1",
		"",
		AD_ENTRY2s("intro.stk",   "f5f028ee39c456fa51fa63b606583918", 313472,
				   "musmac1.mid", "4f66903b33df8a20edd4c748809c0b56", 8161),
		DE_DEU,
		kPlatformWindows,
		ADGF_NO_FLAGS,
		GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
	},
	kFeaturesAdLib,
	0, 0, 0
},
{ // Supplied by Hkz on #scummvm
	{
		"gob1",
		"",
		AD_ENTRY2s("intro.stk",   "f5f028ee39c456fa51fa63b606583918", 313472,
				   "musmac1.mid", "4f66903b33df8a20edd4c748809c0b56", 8161),
		ES_ESP,
		kPlatformWindows,
		ADGF_NO_FLAGS,
		GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
	},
	kFeaturesAdLib,
	0, 0, 0
},
{
	{
		"gob1",
		"",
		AD_ENTRY2s("intro.stk",   "e157cb59c6d330ca70d12ab0ef1dd12b", 288972,
				   "musmac1.mid", "4f66903b33df8a20edd4c748809c0b56", 8161),
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
		"gob1",
		"",
		AD_ENTRY2s("intro.stk",   "e157cb59c6d330ca70d12ab0ef1dd12b", 288972,
				   "musmac1.mid", "4f66903b33df8a20edd4c748809c0b56", 8161),
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
		"gob1",
		"",
		AD_ENTRY2s("intro.stk",   "e157cb59c6d330ca70d12ab0ef1dd12b", 288972,
				   "musmac1.mid", "4f66903b33df8a20edd4c748809c0b56", 8161),
		ES_ESP,
		kPlatformWindows,
		ADGF_NO_FLAGS,
		GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
	},
	kFeaturesAdLib,
	0, 0, 0
},
{
	{
		"gob1",
		"",
		AD_ENTRY2s("intro.stk",   "e157cb59c6d330ca70d12ab0ef1dd12b", 288972,
				   "musmac1.mid", "4f66903b33df8a20edd4c748809c0b56", 8161),
		IT_ITA,
		kPlatformWindows,
		ADGF_NO_FLAGS,
		GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
	},
	kFeaturesAdLib,
	0, 0, 0
},
{
	{
		"gob1",
		"",
		AD_ENTRY2s("intro.stk",   "e157cb59c6d330ca70d12ab0ef1dd12b", 288972,
				   "musmac1.mid", "4f66903b33df8a20edd4c748809c0b56", 8161),
		DE_DEU,
		kPlatformWindows,
		ADGF_NO_FLAGS,
		GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
	},
	kFeaturesAdLib,
	0, 0, 0
},
{ // Found in french ADI 2.5 Anglais Multimedia 5e
	{
		"gob1",
		"",
		AD_ENTRY1s("intro.stk", "f5f028ee39c456fa51fa63b606583918", 313472),
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
		"gob1",
		"",
		AD_ENTRY1s("intro.stk", "f5f028ee39c456fa51fa63b606583918", 313472),
		EN_GRB,
		kPlatformWindows,
		ADGF_NO_FLAGS,
		GUIO3(GUIO_NOSUBTITLES, GUIO_NOSPEECH, GAMEOPTION_COPY_PROTECTION)
	},
	kFeaturesAdLib,
	0, 0, 0
},
{ // Found in french ADI 2.5 Anglais Multimedia 5e
	{
		"gob1",
		"",
		AD_ENTRY1s("intro.stk", "f5f028ee39c456fa51fa63b606583918", 313472),
		DE_DEU,
		kPlatformWindows,
		ADGF_NO_FLAGS,
		GUIO3(GUIO_NOSUBTITLES, GUIO_NOSPEECH, GAMEOPTION_COPY_PROTECTION)
	},
	kFeaturesAdLib,
	0, 0, 0
},
{ // Found in french ADI 2.5 Anglais Multimedia 5e
	{
		"gob1",
		"",
		AD_ENTRY1s("intro.stk", "f5f028ee39c456fa51fa63b606583918", 313472),
		IT_ITA,
		kPlatformWindows,
		ADGF_NO_FLAGS,
		GUIO3(GUIO_NOSUBTITLES, GUIO_NOSPEECH, GAMEOPTION_COPY_PROTECTION)
	},
	kFeaturesAdLib,
	0, 0, 0
},
{ // Found in french ADI 2.5 Anglais Multimedia 5e
	{
		"gob1",
		"",
		AD_ENTRY1s("intro.stk", "f5f028ee39c456fa51fa63b606583918", 313472),
		ES_ESP,
		kPlatformWindows,
		ADGF_NO_FLAGS,
		GUIO3(GUIO_NOSUBTITLES, GUIO_NOSPEECH, GAMEOPTION_COPY_PROTECTION)
	},
	kFeaturesAdLib,
	0, 0, 0
},

// -- Amiga --

{
	{
		"gob1",
		"",
		AD_ENTRY1s("intro.stk", "c65e9cc8ba23a38456242e1f2b1caad4", 135561),
		UNK_LANG,
		kPlatformAmiga,
		ADGF_NO_FLAGS,
		GUIO3(GUIO_NOSUBTITLES, GUIO_NOSPEECH, GAMEOPTION_COPY_PROTECTION)
	},
	kFeaturesNone,
	0, 0, 0
},
{
	{
		"gob1",
		"",
		AD_ENTRY2s("intro.stk", "c65e9cc8ba23a38456242e1f2b1caad4", 135561,
		           "disk1.stk", "a6ed3c1c9a46c511952bac0c11c691f5", 367048),
		DE_DEU,
		kPlatformAmiga,
		ADGF_NO_FLAGS,
		GUIO3(GUIO_NOSUBTITLES, GUIO_NOSPEECH, GAMEOPTION_COPY_PROTECTION)
	},
	kFeaturesNone,
	0, 0, 0
},


// -- Demos --

{
	{
		"gob1",
		"Demo",
		AD_ENTRY1("intro.stk", "972f22c6ff8144a6636423f0354ca549"),
		EN_GRB,
		kPlatformAmiga,
		ADGF_DEMO,
		GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
	},
	kFeaturesNone,
	0, 0, 0
},
{
	{
		"gob1",
		"Interactive Demo",
		AD_ENTRY1s("intro.stk", "e72bd1e3828c7dec4c8a3e58c48bdfdb", 280044),
		UNK_LANG,
		kPlatformDOS,
		ADGF_DEMO,
		GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
	},
	kFeaturesNone,
	0, 0, 0
},
{
	{
		"gob1",
		"Interactive Demo",
		AD_ENTRY1s("intro.stk", "a796096280d5efd48cf8e7dfbe426eb5", 193595),
		EN_GRB,
		kPlatformDOS,
		ADGF_DEMO,
		GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
	},
	kFeaturesNone,
	0, 0, 0
},
{ // Supplied by goodoldgeorg in bug report #4324
	{
		"gob1",
		"Interactive Demo",
		AD_ENTRY1s("intro.stk", "35a098571af9a03c04e2303aec7c9249", 116582),
		FR_FRA,
		kPlatformDOS,
		ADGF_DEMO,
		GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
	},
	kFeaturesNone,
	0, 0, 0
},
{
	{
		"gob1",
		"",
		AD_ENTRY1s("intro.stk", "0e022d3f2481b39e9175d37b2c6ad4c6", 2390121),
		FR_FRA,
		kPlatformCDi,
		ADGF_NO_FLAGS,
		GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
	},
	kFeaturesAdLib,
	0, "AVT003.TOT", 0
},
{ // Found on ADI Accompagnement Scolaire - Francais-Maths CE1/CE2
	{
		"gob1",
		"CE1/CE2",
		AD_ENTRY1s("intro.stk", "ae38e1dac63576b9a7d34a96fd6eb37c", 5731374),
		FR_FRA,
		kPlatformCDi,
		ADGF_DEMO | ADGF_UNSTABLE,
		GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
	},
	kFeaturesAdLib,
	0, "AVT008.TOT", 0
},
{ // Found on ADI Accompagnement Scolaire - Francais-Maths CM1/CM2
	{
		"gob1",
		"CM1/CM2",
		AD_ENTRY1s("intro.stk", "ca15cc119fea5ee432083e7f6b873c38", 2441216),
		FR_FRA,
		kPlatformCDi,
		ADGF_DEMO | ADGF_UNSTABLE,
		GUIO2(GUIO_NOSUBTITLES, GUIO_NOSPEECH)
	},
	kFeaturesAdLib,
	0, "AVT003.TOT", 0
},

#endif // GOB_DETECTION_TABLES_GOB1_H
