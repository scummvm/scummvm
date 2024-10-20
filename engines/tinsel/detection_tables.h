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

#include "common/translation.h"
namespace Tinsel {

static const TinselGameDescription gameDescriptions[] = {

	// Note: The following is the (hopefully) definitive list of version details:
	// TINSEL_V0: Used only by the Discworld 1 demo - this used a more primitive version
	//   of the Tinsel engine and graphics compression
	// TINSEL_V1: There were two versions of the Discworld 1 game - the first used .GRA
	//   files, and the second used .SCN files. The second also provided some fixes to
	//   various script bugs and coding errors, but is still considered TINSEL_V1,
	//   as both game versions work equally well with the newer code.
	// TINSEL_V2: The Discworld 2 game used this updated version of the Tinsel 1 engine,
	//   and as far as we know there aren't any variations of this engine.

	// ==== Discworld 1 early (TinselV0) entries ==============================

	{	// Floppy Demo V0 from https://web.archive.org/web/20100415160943/http://www.adventure-treff.de/specials/dl_demos.php
		{
			"dw",
			"Floppy Demo",
			AD_ENTRY1s("dw.gra", "ce1b57761ba705221bcf70955b827b97", 441192),
			//AD_ENTRY1s("dw.scn", "ccd72f02183d0e96b6e7d8df9492cda8", 23308),
			Common::EN_ANY,
			Common::kPlatformDOS,
			ADGF_DEMO,
			GUIO3(GUIO_NOSPEECH, GUIO_NOSFX, GUIO_NOMUSIC)
		},
		GID_DW1,
		0,
		0,
		TINSEL_V0,
	},

	// ==== Discworld 1 entries ===============================================

	{	// CD Demo V1 version, with *.gra files
		{
			"dw",
			"CD Demo",
			AD_ENTRY2s("dw.gra", "ef5a2518c9e205f786f5a4526396e661", 781676,
					   "english.smp", NULL, AD_NO_SIZE),
			Common::EN_ANY,
			Common::kPlatformDOS,
			ADGF_DEMO | ADGF_CD,
			GUIO0()
		},
		GID_DW1,
		0,
		0,
		TINSEL_V1,
	},

	{	// Macintosh CD Demo V1 version, with *.scn files, see tracker #5517
		{
			"dw",
			"CD Demo",
			{
				{"dw.scn", 0, "cfc40a8d5d476a1c9d3abf826fa46f8c", 1272686},
				{"english.txt", 0, "c69b5d2067e9114a63569a61e9a82faa", 228878},
				{"english.smp", 0, NULL, AD_NO_SIZE},
				AD_LISTEND
			},
			Common::EN_ANY,
			Common::kPlatformMacintosh,
			ADGF_DEMO | ADGF_CD,
			GUIO0()
		},
		GID_DW1,
		0,
		GF_SCNFILES,
		TINSEL_V1,
	},

	{	// Multilingual Floppy V1 with *.gra files.
		// Note: It contains no english subtitles.
		{
			"dw",
			"Floppy",
			{
				{"dw.gra", 0, "c8808ccd988d603dd35dff42013ae7fd", 781656},
				{"french.txt", 0, NULL, AD_NO_SIZE},
				{"german.txt", 0, NULL, AD_NO_SIZE},
				{"italian.txt", 0, NULL, AD_NO_SIZE},
				{"spanish.txt", 0, NULL, AD_NO_SIZE},
				AD_LISTEND
			},
			Common::FR_FRA,
			Common::kPlatformDOS,
			ADGF_DROPLANGUAGE,
			GUIO1(GUIO_NOSPEECH)
		},
		GID_DW1,
		0,
		GF_USE_4FLAGS | GF_ENHANCED_AUDIO_SUPPORT,
		TINSEL_V1,
	},

	{
		{
			"dw",
			"Floppy",
			{
				{"dw.gra", 0, "c8808ccd988d603dd35dff42013ae7fd", 781656},
				{"french.txt", 0, NULL, AD_NO_SIZE},
				{"german.txt", 0, NULL, AD_NO_SIZE},
				{"italian.txt", 0, NULL, AD_NO_SIZE},
				{"spanish.txt", 0, NULL, AD_NO_SIZE},
				AD_LISTEND
			},
			Common::DE_DEU,
			Common::kPlatformDOS,
			ADGF_DROPLANGUAGE,
			GUIO1(GUIO_NOSPEECH)
		},
		GID_DW1,
		0,
		GF_USE_4FLAGS | GF_ENHANCED_AUDIO_SUPPORT,
		TINSEL_V1,
	},

	{
		{
			"dw",
			"Floppy",
			{
				{"dw.gra", 0, "c8808ccd988d603dd35dff42013ae7fd", 781656},
				{"french.txt", 0, NULL, AD_NO_SIZE},
				{"german.txt", 0, NULL, AD_NO_SIZE},
				{"italian.txt", 0, NULL, AD_NO_SIZE},
				{"spanish.txt", 0, NULL, AD_NO_SIZE},
				AD_LISTEND
			},
			Common::IT_ITA,
			Common::kPlatformDOS,
			ADGF_DROPLANGUAGE,
			GUIO1(GUIO_NOSPEECH)
		},
		GID_DW1,
		0,
		GF_USE_4FLAGS | GF_ENHANCED_AUDIO_SUPPORT,
		TINSEL_V1,
	},

	{
		{
			"dw",
			"Floppy",
			{
				{"dw.gra", 0, "c8808ccd988d603dd35dff42013ae7fd", 781656},
				{"french.txt", 0, NULL, AD_NO_SIZE},
				{"german.txt", 0, NULL, AD_NO_SIZE},
				{"italian.txt", 0, NULL, AD_NO_SIZE},
				{"spanish.txt", 0, NULL, AD_NO_SIZE},
				AD_LISTEND
			},
			Common::ES_ESP,
			Common::kPlatformDOS,
			ADGF_DROPLANGUAGE,
			GUIO1(GUIO_NOSPEECH)
		},
		GID_DW1,
		0,
		GF_USE_4FLAGS | GF_ENHANCED_AUDIO_SUPPORT,
		TINSEL_V1,
	},

	{	// Floppy V1 version, with *.gra files
		{
			"dw",
			"Floppy",
			AD_ENTRY1s("dw.gra", "c8808ccd988d603dd35dff42013ae7fd", 781656),
			Common::EN_ANY,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOSPEECH)
		},
		GID_DW1,
		0,
		GF_ENHANCED_AUDIO_SUPPORT,
		TINSEL_V1,
	},

	{	// CD V1 version, with *.gra files (same as the floppy one, with english.smp)
		{
			"dw",
			"CD",
			AD_ENTRY2s("dw.gra", "c8808ccd988d603dd35dff42013ae7fd", 781656,
					   "english.smp", NULL, AD_NO_SIZE),
			Common::EN_ANY,
			Common::kPlatformDOS,
			ADGF_CD,
			GUIO0()
		},
		GID_DW1,
		0,
		GF_ENHANCED_AUDIO_SUPPORT,
		TINSEL_V1,
	},

	{	// Polish fan translation CD V1 version, with *.gra files (same as the floppy one, with english.smp)
		{
			"dw",
			"CD",
			AD_ENTRY2s("dw.gra", "ef05bbd2a754bd11a2e87bcd84ab5ccf", 781864,
					   "english.smp", NULL, AD_NO_SIZE),
			Common::EN_ANY,
			Common::kPlatformDOS,
			ADGF_CD,
			GUIO_NONE
		},
		GID_DW1,
		0,
		GF_ENHANCED_AUDIO_SUPPORT,
		TINSEL_V1,
	},

	{	// Polish fan translaction floppy V1 version, with *.gra files
		{
			"dw",
			"Floppy",
			AD_ENTRY1s("dw.gra", "ef05bbd2a754bd11a2e87bcd84ab5ccf", 781864),
			Common::EN_ANY,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO_NOSPEECH
		},
		GID_DW1,
		0,
		GF_ENHANCED_AUDIO_SUPPORT,
		TINSEL_V1,
	},

	{	// Italian CD with english speech and *.gra files.
		// Note: It contains only italian subtitles, but inside english.txt
		{
			"dw",
			"CD",
			{
				{"dw.gra", 0, "c8808ccd988d603dd35dff42013ae7fd", 781656},
				{"english.txt", 0, "15f0703f85477d7fab4280bf938b61c1", 237774},
				{"english.smp", 0, NULL, AD_NO_SIZE},
				AD_LISTEND
			},
			Common::IT_ITA,
			Common::kPlatformDOS,
			ADGF_DROPLANGUAGE | ADGF_CD,
			GUIO0()
		},
		GID_DW1,
		0,
		GF_USE_4FLAGS | GF_ENHANCED_AUDIO_SUPPORT,
		TINSEL_V1,
	},

	{	// Multilingual CD with english speech and *.gra files.
		// Note: It contains no english subtitles.
		{
			"dw",
			"CD",
			{
				{"dw.gra", 0, "c8808ccd988d603dd35dff42013ae7fd", 781656},
				{"english.smp", 0, NULL, AD_NO_SIZE},
				{"french.txt", 0, NULL, AD_NO_SIZE},
				{"german.txt", 0, NULL, AD_NO_SIZE},
				{"italian.txt", 0, NULL, AD_NO_SIZE},
				{"spanish.txt", 0, NULL, AD_NO_SIZE},
				AD_LISTEND
			},
			Common::FR_FRA,
			Common::kPlatformDOS,
			ADGF_DROPLANGUAGE | ADGF_CD,
			GUIO0()
		},
		GID_DW1,
		0,
		GF_USE_4FLAGS | GF_ENHANCED_AUDIO_SUPPORT,
		TINSEL_V1,
	},

	{
		{
			"dw",
			"CD",
			{
				{"dw.gra", 0, "c8808ccd988d603dd35dff42013ae7fd", 781656},
				{"english.smp", 0, NULL, AD_NO_SIZE},
				{"french.txt", 0, NULL, AD_NO_SIZE},
				{"german.txt", 0, NULL, AD_NO_SIZE},
				{"italian.txt", 0, NULL, AD_NO_SIZE},
				{"spanish.txt", 0, NULL, AD_NO_SIZE},
				AD_LISTEND
			},
			Common::DE_DEU,
			Common::kPlatformDOS,
			ADGF_DROPLANGUAGE | ADGF_CD,
			GUIO0()
		},
		GID_DW1,
		0,
		GF_USE_4FLAGS | GF_ENHANCED_AUDIO_SUPPORT,
		TINSEL_V1,
	},
	{
		{
			"dw",
			"CD",
			{
				{"dw.gra", 0, "c8808ccd988d603dd35dff42013ae7fd", 781656},
				{"english.smp", 0, NULL, AD_NO_SIZE},
				{"french.txt", 0, NULL, AD_NO_SIZE},
				{"german.txt", 0, NULL, AD_NO_SIZE},
				{"italian.txt", 0, NULL, AD_NO_SIZE},
				{"spanish.txt", 0, NULL, AD_NO_SIZE},
				AD_LISTEND
			},
			Common::IT_ITA,
			Common::kPlatformDOS,
			ADGF_DROPLANGUAGE | ADGF_CD,
			GUIO0()
		},
		GID_DW1,
		0,
		GF_USE_4FLAGS | GF_ENHANCED_AUDIO_SUPPORT,
		TINSEL_V1,
	},
	{
		{
			"dw",
			"CD",
			{
				{"dw.gra", 0, "c8808ccd988d603dd35dff42013ae7fd", 781656},
				{"english.smp", 0, NULL, AD_NO_SIZE},
				{"french.txt", 0, NULL, AD_NO_SIZE},
				{"german.txt", 0, NULL, AD_NO_SIZE},
				{"italian.txt", 0, NULL, AD_NO_SIZE},
				{"spanish.txt", 0, NULL, AD_NO_SIZE},
				AD_LISTEND
			},
			Common::ES_ESP,
			Common::kPlatformDOS,
			ADGF_DROPLANGUAGE | ADGF_CD,
			GUIO0()
		},
		GID_DW1,
		0,
		GF_USE_4FLAGS | GF_ENHANCED_AUDIO_SUPPORT,
		TINSEL_V1,
	},

	{	// English CD v2
		{
			"dw",
			"CD",
			AD_ENTRY2s("dw.scn", "70955425870c7720d6eebed903b2ef41", 776188,
					   "english.smp", NULL, AD_NO_SIZE),
			Common::EN_ANY,
			Common::kPlatformDOS,
			ADGF_CD,
			GUIO0()
		},
		GID_DW1,
		0,
		GF_SCNFILES | GF_ENHANCED_AUDIO_SUPPORT,
		TINSEL_V1,
	},

	{	// Hebrew CD v2
		{
			"dw",
			"CD",
			AD_ENTRY2s("dw.scn", "759d1374b4f02af6d52fc07c96679936", 770780,
					   "english.smp", NULL, AD_NO_SIZE),
			Common::HE_ISR,
			Common::kPlatformDOS,
			ADGF_CD,
			GUIO0()
		},
		GID_DW1,
		0,
		GF_SCNFILES | GF_ENHANCED_AUDIO_SUPPORT,
		TINSEL_V1,
	},

	{	// Discworld PSX CD
		{
			"dw",
			"CD",
			AD_ENTRY1s("english.txt", "7526cfc3a64e00f223795de476b4e2c9", 230326),
			Common::EN_ANY,
			Common::kPlatformPSX,
			ADGF_CD,
			GUIO0()
		},
		GID_DW1,
		0,
		GF_SCNFILES | GF_ENHANCED_AUDIO_SUPPORT,
		TINSEL_V1,
	},

	{	// Discworld PSX German CD
		{
			"dw",
			"CD",
			AD_ENTRY1s("dw.scn", "0b34bb57cd3961e4528e4bce48cc0ab9", 339764),
			Common::DE_DEU,
			Common::kPlatformPSX,
			ADGF_CD,
			GUIO0()
		},
		GID_DW1,
		0,
		GF_SCNFILES | GF_ENHANCED_AUDIO_SUPPORT,
		TINSEL_V1,
	},

	{	// Discworld PSX CD Japanese
		{
			"dw",
			"CD",
			AD_ENTRY1s("dw.scn", "bd2e47010565998641ec45a9c9285be0", 328048),
			Common::JA_JPN,
			Common::kPlatformPSX,
			ADGF_CD | ADGF_UNSTABLE,
			GUIO0()
		},
		GID_DW1,
		0,
		GF_SCNFILES | GF_ENHANCED_AUDIO_SUPPORT,
		TINSEL_V1,
	},


	{	// multilanguage PSX demo
		{
			"dw",
			"CD Demo",
			{
				{"french.txt", 0, "e7020d35f58d0d187052ac406d86cc87", 273914},
				{"german.txt", 0, "52f0a01e0ff0d340b02a36fd5109d705", 263942},
				{"italian.txt", 0, "15f0703f85477d7fab4280bf938b61c1", 239834},
				{"spanish.txt", 0, "c324170c3f1922c605c5cc09ba265aa5", 236702},
				{"english.txt", 0, "7526cfc3a64e00f223795de476b4e2c9", 230326},
				AD_LISTEND
			},
			Common::EN_ANY,
			Common::kPlatformPSX,
			ADGF_CD | ADGF_DEMO,
			GUIO0()
		},
		GID_DW1,
		0,
		GF_SCNFILES,
		TINSEL_V1,
	},

	{	// English Saturn CD
		{
			"dw",
			_s("Saturn CD version is not yet supported"),
			AD_ENTRY2s("dw.scn", "6803f293c88758057cc685b9437f7637", 382248,
					   "english.smp", NULL, AD_NO_SIZE),
			Common::EN_ANY,
			Common::kPlatformSaturn,
			ADGF_CD,
			GUIO0()
		},
		GID_DW1,
		0,
		GF_SCNFILES | GF_ENHANCED_AUDIO_SUPPORT | ADGF_UNSUPPORTED,
		TINSEL_V1,
	},

	{	// Mac English CD, see tracker #6384
		{
			"dw",
			"CD",
			AD_ENTRY2s("dw.scn", "114643df0d1f1530a0a9c5d4e38917bc", 1268553,
					   "english.smp", NULL, AD_NO_SIZE),
			Common::EN_ANY,
			Common::kPlatformMacintosh,
			ADGF_CD,
			GUIO0()
		},
		GID_DW1,
		0,
		GF_SCNFILES | GF_ENHANCED_AUDIO_SUPPORT,
		TINSEL_V1,
	},

	{	// Mac multilanguage CD
		{
			"dw",
			"CD",
			AD_ENTRY2s("dw.scn", "cfc40a8d5d476a1c9d3abf826fa46f8c", 1265532,
					   "english.smp", NULL, AD_NO_SIZE),
			Common::EN_ANY,
			Common::kPlatformMacintosh,
			ADGF_CD,
			GUIO0()
		},
		GID_DW1,
		0,
		GF_SCNFILES | GF_ENHANCED_AUDIO_SUPPORT,
		TINSEL_V1,
	},

	{	// Mac Japanese CD
		{
			"dw",
			"CD",
			AD_ENTRY2s("dw.scn", "55a11596b16130027fb28d8c203655a8", 1430841,
					   "english.smp", NULL, AD_NO_SIZE),
			Common::JA_JPN,
			Common::kPlatformMacintosh,
			ADGF_CD | ADGF_UNSTABLE,
			GUIO0()
		},
		GID_DW1,
		0,
		GF_SCNFILES | GF_ENHANCED_AUDIO_SUPPORT,
		TINSEL_V1,
	},


	{	// German CD re-release "Neon Edition"
		// Note: This release has ENGLISH.TXT (with german content) instead of GERMAN.TXT
		{
			"dw",
			"CD",
			AD_ENTRY1s("dw.scn", "6182c7986eaec893c62fb6ea13a9f225", 774556),
			Common::DE_DEU,
			Common::kPlatformDOS,
			ADGF_CD,
			GUIO0()
		},
		GID_DW1,
		0,
		GF_SCNFILES | GF_ENHANCED_AUDIO_SUPPORT | GF_ALT_MIDI,
		TINSEL_V1,
	},

	{	// Russian Discworld 1
		{
			"dw",
			"CD",
			{
				{"dw.scn", 0, "133041bde59d05c1bf084fd6f1bdce4b", 776524},
				{"english.txt", 0, "f73dcbd7b136b37c2adf7c9448ea336d", 231821},
				{"english.smp", 0, NULL, AD_NO_SIZE},
				AD_LISTEND
			},
			Common::RU_RUS,
			Common::kPlatformDOS,
			ADGF_CD,
			GUIO1(GUIO_NOASPECT)
		},
		GID_DW1,
		0,
		GF_SCNFILES | GF_ENHANCED_AUDIO_SUPPORT,
		TINSEL_V1,
	},

	{	// English CD "Argentum Collection"
		{
			"dw",
			"CD",
			AD_ENTRY2s("dw.scn", "36795d539e290838fa1d4c11789cb142", 776188,
					   "english.smp", NULL, AD_NO_SIZE),
			Common::EN_ANY,
			Common::kPlatformDOS,
			ADGF_CD,
			GUIO0()
		},
		GID_DW1,
		0,
		GF_SCNFILES | GF_ENHANCED_AUDIO_SUPPORT,
		TINSEL_V1,
	},

	{	// Russian Discworld 1. Fan translation v1.1
		{
			"dw",
			"CD v1.1",
			{
				{"dw.scn", 0, "133041bde59d05c1bf084fd6f1bdce4b", 776524},
				{"english.txt", 0, "317542cf2e50106d9c9421ddcf821e22", 221656},
				{"english.smp", 0, NULL, AD_NO_SIZE},
				AD_LISTEND
			},
			Common::RU_RUS,
			Common::kPlatformDOS,
			ADGF_CD,
			GUIO1(GUIO_NOASPECT)
		},
		GID_DW1,
		0,
		GF_SCNFILES | GF_ENHANCED_AUDIO_SUPPORT,
		TINSEL_V1,
	},

	{	// Polish fan translaction Discworld 1
		{
			"dw",
			"CD",
			{
				{"dw.scn", 0, "fa169d2c98660215ebd84b49c1899eef", 776396},
				{"english.txt", 0, "c1a53eb7ec812689dab70e2bb22cf2ab", 224151},
				{"english.smp", 0, NULL, AD_NO_SIZE},
				AD_LISTEND
			},
			Common::PL_POL,
			Common::kPlatformDOS,
			ADGF_CD,
			GUIO_NONE
		},
		GID_DW1,
		0,
		GF_SCNFILES | GF_ENHANCED_AUDIO_SUPPORT,
		TINSEL_V1,
	},

	// ==== Discworld 2 entries ===============================================
	// Note: All Discworld 2 versions are CD only, therefore we don't add the ADGF_CD flag
#define DISCWORLD2_GUIOPTIONS GUIO2(GUIO_NOASPECT, GAMEOPTION_CROP_HEIGHT_480_TO_432)

	{	// English Discworld 2 demo (dw2-win-demo-en)
		{
			"dw2",
			"Demo",
			AD_ENTRY2s("dw2.scn",		"853ab998f5136b69bc586991175d6eeb", 4231121,
					   "english.smp",	"b5660a0e031cb4710bcb0ef5629ea61d", 28562357),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_DEMO,
			DISCWORLD2_GUIOPTIONS
		},
		GID_DW2,
		0,
		GF_SCNFILES,
		TINSEL_V2,
	},

	{	// English Discworld 2 demo (second Windows demo: dw2-win-demo-2-en)
		{
			"dw2",
			"",
			AD_ENTRY3s("dw2.scn",     "3f24abb61a058f8faeac7c0768cf21fc", 4224921,
					   "english.smp", "b5660a0e031cb4710bcb0ef5629ea61d", 31360342,
					   "english.txt", "f17e10eccac0fb2d1fea489a951da266", 283144),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_UNSUPPORTED | ADGF_DEMO,
			DISCWORLD2_GUIOPTIONS
		},
		GID_DW2,
		0,
		GF_SCNFILES,
		TINSEL_V2,
	},

	{	// English (US) Discworld 2 demo (DOS demo: dw2-dos-demo-en)
		{
			"dw2",
			"",
			AD_ENTRY2s("dw2.scn",     "05beafadd26562d708f68194d337b2cb", 103221,
					   "us.smp",      "b5660a0e031cb4710bcb0ef5629ea61d", 28320582),
			Common::EN_USA,
			Common::kPlatformDOS,
			ADGF_UNSUPPORTED | ADGF_DEMO,
			DISCWORLD2_GUIOPTIONS
		},
		GID_DW2,
		0,
		GF_SCNFILES,
		TINSEL_V2,
	},

	{	// European/Australian Discworld 2 release
		{
			"dw2",
			"CD",
			AD_ENTRY2s("dw2.scn", "c6d15ce9720a9d8fef06e6582dcf3f34", 103593,
					   "english1.smp", NULL, AD_NO_SIZE),
			Common::EN_GRB,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			DISCWORLD2_GUIOPTIONS
		},
		GID_DW2,
		0,
		GF_SCNFILES,
		TINSEL_V2,
	},

	{	// US Discworld 2 release
		{
			"dw2",
			"CD",
			AD_ENTRY2s("dw2.scn", "c6d15ce9720a9d8fef06e6582dcf3f34", 103593,
					   "us1.smp", NULL, AD_NO_SIZE),
			Common::EN_USA,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			DISCWORLD2_GUIOPTIONS
		},
		GID_DW2,
		0,
		GF_SCNFILES,
		TINSEL_V2,
	},

	{	// French version of Discworld 2
		{
			"dw2",
			"CD",
			AD_ENTRY2s("dw2.scn", "c6d15ce9720a9d8fef06e6582dcf3f34", 103593,
					   "french1.smp", NULL, AD_NO_SIZE),
			Common::FR_FRA,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			DISCWORLD2_GUIOPTIONS
		},
		GID_DW2,
		0,
		GF_SCNFILES,
		TINSEL_V2,
	},

	{	// German Discworld 2 re-release "Neon Edition"
		{
			"dw2",
			"CD",
			AD_ENTRY2s("dw2.scn", "c6d15ce9720a9d8fef06e6582dcf3f34", 103593,
					   "german1.smp", NULL, AD_NO_SIZE),
			Common::DE_DEU,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			DISCWORLD2_GUIOPTIONS
		},
		GID_DW2,
		0,
		GF_SCNFILES,
		TINSEL_V2,
	},

	{	// Italian/Spanish Discworld 2
		{
			"dw2",
			"CD",
			{
				{"dw2.scn", 0, "c6d15ce9720a9d8fef06e6582dcf3f34", 103593},
				{"english1.smp", 0, NULL, AD_NO_SIZE},
				{"italian1.txt", 0, "d443249f8b55489b5888c227b9096f4e", 246495},
				AD_LISTEND
			},
			Common::IT_ITA,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			DISCWORLD2_GUIOPTIONS
		},
		GID_DW2,
		0,
		GF_SCNFILES,
		TINSEL_V2,
	},
	{
		{
			"dw2",
			"CD",
			{
				{"dw2.scn", 0, "c6d15ce9720a9d8fef06e6582dcf3f34", 103593},
				{"english1.smp", 0, NULL, AD_NO_SIZE},
				{"spanish1.txt", 0, "bc6e147c5f542db228ac577357e4d897", 230323},
				AD_LISTEND
			},
			Common::ES_ESP,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			DISCWORLD2_GUIOPTIONS
		},
		GID_DW2,
		0,
		GF_SCNFILES,
		TINSEL_V2,
	},

	{	// Russian Discworld 2 release by Fargus
		{
			"dw2",
			"CD",
			{
				{"dw2.scn", 0, "c6d15ce9720a9d8fef06e6582dcf3f34", 103593},
				{"english1.smp", 0, NULL, AD_NO_SIZE},
				{"english1.txt", 0, "b522e19d7b2cd7b85e50e36fe48e36a9", 274444},
				AD_LISTEND
			},
			Common::RU_RUS,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			DISCWORLD2_GUIOPTIONS
		},
		GID_DW2,
		0,
		GF_SCNFILES,
		TINSEL_V2,
	},

	// ==== Discworld Noir entries ===============================================
	// Note: All Discworld Noir versions are CD only, therefore we don't add the ADGF_CD flag

#define NOIR_GUIOPTIONS GUIO1(GAMEOPTION_CROP_HEIGHT_480_TO_432)
	{ // Discworld Noir, Windows 3CD version
		{
			"noir",
			"CD",
			AD_ENTRY2s("dw3.scn",		"16104acdc66cda903f860acac02a96bd", AD_NO_SIZE,
					   "english.smp",	"94e510fd33c5c4a67b274bf5c068a87a", AD_NO_SIZE),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_UNSTABLE,
			NOIR_GUIOPTIONS

		},
		GID_NOIR,
		0,
		GF_SCNFILES,
		TINSEL_V3,
	},

	{ // Discworld Noir, Interactive Windows Demo
		{
			"noir",
			"Demo",
			AD_ENTRY2s("english.smp",	"ecca3ec84ad6460bf289e6171ac32048", 73494004,
					   "dw3.scn",		"f8e50c19b44a2c6eaee5f78654647878", 803781),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_UNSTABLE | ADGF_DEMO,
			NOIR_GUIOPTIONS
		},
		GID_NOIR,
		0,
		GF_SCNFILES,
		TINSEL_V3,
	},

	{ AD_TABLE_END_MARKER, 0, 0, 0, 0 }
};

} // End of namespace Tinsel
