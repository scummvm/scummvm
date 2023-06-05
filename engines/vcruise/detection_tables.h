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

#ifndef VCRUISE_DETECTION_TABLES_H
#define VCRUISE_DETECTION_TABLES_H

#include "engines/advancedDetector.h"

#include "vcruise/detection.h"

namespace VCruise {

static const VCruiseGameDescription gameDescriptions[] = {
	{ // Reah: Face the Unknown, English DVD version
		{
			"reah",
			"English DVD",
			AD_ENTRY2s("Reah.exe", "69aa9832338db3f40f616386e54857e8", 304128,
					   "0170_b.wav", "5b705300b4fee3ceb821a1b55884a722", 129248),
			Common::UNK_LANG,
			Common::kPlatformWindows,
			ADGF_TESTING | VCRUISE_GF_WANT_MP3,
			GUIO0()
		},
		GID_REAH,
		Common::EN_ANY,
	},
	{ // Reah: Face the Unknown, English digital (GOG) version
		{
			"reah",
			"English Digital",
			AD_ENTRY2s("Reah.exe", "60ec19c53f1323cc7f0314f98d396283", 304128,
					   "0170_b.wav", "5b705300b4fee3ceb821a1b55884a722", 129248),
			Common::UNK_LANG,
			Common::kPlatformWindows,
			ADGF_TESTING | VCRUISE_GF_WANT_MP3,
			GUIO0()
		},
		GID_REAH,
		Common::EN_ANY,
	},
	{ // Reah: Face the Unknown, English 6 CD Version
		{
			"reah",
			"English CD",
			AD_ENTRY2s("Reah.exe", "77bc7f7819cdd443f52b193529138c87", 305664,
					   "0170_b.wav", "5b705300b4fee3ceb821a1b55884a722", 129248),
			Common::UNK_LANG,
			Common::kPlatformWindows,
			ADGF_TESTING,
			GUIO0()
		},
		GID_REAH,
		Common::EN_ANY,
	},
	{ // Reah: Face the Unknown, English 6 CD Version (Project Two Interactive variation)
		{
			"reah",
			"English CD",
			AD_ENTRY2s("Reah.exe", "77bc7f7819cdd443f52b193529138c87", 305664,
					   "0170_b.wav", "36c0bf57ab5a748ef6699a159195b3ae", 124356),
			Common::UNK_LANG,
			Common::kPlatformWindows,
			ADGF_TESTING,
			GUIO0()
		},
		GID_REAH,
		Common::EN_ANY,
	},
	{ // Reah: Face the Unknown, German 6 CD Version
		{
			"reah",
			"German CD",
			AD_ENTRY2s("Reah.exe", "be29f9f9fc9a454488f9d2fb68e26326", 305664,
					   "0170_b.wav", "5f84af02d1193ce9c47fe0dc821f0613", 121220),
			Common::UNK_LANG,
			Common::kPlatformWindows,
			ADGF_TESTING,
			GUIO0()
		},
		GID_REAH,
		Common::DE_DEU,
	},
	{ // Reah: Face the Unknown, English digital (GOG) version + German VO community patch
		{
			"reah",
			"English Digital + German Community Patch",
			AD_ENTRY2s("Reah.exe", "60ec19c53f1323cc7f0314f98d396283", 304128,
					   "0170_b.wav", "5f84af02d1193ce9c47fe0dc821f0613", 121220),
			Common::UNK_LANG,
			Common::kPlatformWindows,
			ADGF_TESTING | VCRUISE_GF_WANT_MP3,
			GUIO0()
		},
		GID_REAH,
		Common::EN_ANY,
	},

	{ // Schizm: Mysterious Journey, English CD Version
		{
			"schizm",
			"English CD",
			AD_ENTRY2s("Schizm.exe", "24bb1831a53b3969d9d1a9302740de4a", 368640,
					   "0001_a.wav", "374d93abc3422840623acc618ecb2b1e", 1553784),
			Common::UNK_LANG,
			Common::kPlatformWindows,
			ADGF_TESTING | ADGF_CD | VCRUISE_GF_WANT_OGG_VORBIS | VCRUISE_GF_NEED_JPEG,
			GUIO0()
		},
		GID_SCHIZM,
		Common::EN_GRB,
	},
	{ // Schizm: Mysterious Journey, German DVD Version (installed)
		{
			"schizm",
			"German DVD",
			AD_ENTRY2s("Schizm.exe", "3d63307697c72f3fd6cafb378f61ca2b", 364544,
					   "0001_a.wav", "f2b7eccfb1e9af0282b541c5eac66cc7", 1613240),
			Common::UNK_LANG,
			Common::kPlatformWindows,
			ADGF_TESTING | VCRUISE_GF_WANT_OGG_VORBIS | VCRUISE_GF_NEED_JPEG,
			GUIO0()
		},
		GID_SCHIZM,
		Common::DE_DEU,
	},
		
	{ // Schizm: Mysterious Journey, Polish DVD Version
		{
			"schizm",
			"Polish DVD",
			AD_ENTRY1s("disk1.pak", "a3453878ad86d012b483a82e04276667", 272507257),
			Common::UNK_LANG,
			Common::kPlatformWindows,
			ADGF_TESTING | VCRUISE_GF_WANT_OGG_VORBIS | VCRUISE_GF_NEED_JPEG | VCRUISE_GF_GENTEE_PACKAGE,
			GUIO0()
		},
		GID_SCHIZM,
		Common::PL_POL,
	},
	{ // Schizm: Mysterious Journey, German DVD Version
		{
			"schizm",
			"German DVD",
			AD_ENTRY1s("disk1.pak", "dcb27eb3d8a0029c551df5f779af36fc", 274285596),
			Common::UNK_LANG,
			Common::kPlatformWindows,
			ADGF_TESTING | VCRUISE_GF_WANT_OGG_VORBIS | VCRUISE_GF_NEED_JPEG | VCRUISE_GF_GENTEE_PACKAGE,
			GUIO0()
		},
		GID_SCHIZM,
		Common::DE_DEU,
	},

	{ // Schizm: Mysterious Journey, English digital 10-language (GOG) version
		{
			"schizm",
			"English Digital",
			AD_ENTRY2s("Schizm.exe", "296edd26d951c3bdc4d303c4c88b27cd", 364544,
					   "0001_a.wav", "374d93abc3422840623acc618ecb2b1e", 1553784),
			Common::UNK_LANG,
			Common::kPlatformWindows,
			ADGF_TESTING | VCRUISE_GF_WANT_OGG_VORBIS | VCRUISE_GF_NEED_JPEG,
			GUIO0()
		},
		GID_SCHIZM,
		Common::EN_GRB,
	},
	{ // Schizm: Mysterious Journey, German digital 10-language (GOG) version
		{
			"schizm",
			"German Digital",
			AD_ENTRY2s("Schizm.exe", "296edd26d951c3bdc4d303c4c88b27cd", 364544,
					   "0001_a.wav", "f2b7eccfb1e9af0282b541c5eac66cc7", 1613240),
			Common::UNK_LANG,
			Common::kPlatformWindows,
			ADGF_TESTING | VCRUISE_GF_WANT_OGG_VORBIS | VCRUISE_GF_NEED_JPEG,
			GUIO0()
		},
		GID_SCHIZM,
		Common::DE_DEU,
	},
	{ // Schizm: Mysterious Journey, German digital 10-language (GOG) version
		{
			"schizm",
			"Spanish Digital",
			AD_ENTRY2s("Schizm.exe", "296edd26d951c3bdc4d303c4c88b27cd", 364544,
					   "0001_a.wav", "7a398f17e847a46de629a09fa6178b00", 1409560),
			Common::UNK_LANG,
			Common::kPlatformWindows,
			ADGF_TESTING | VCRUISE_GF_WANT_OGG_VORBIS | VCRUISE_GF_NEED_JPEG,
			GUIO0()
		},
		GID_SCHIZM,
		Common::ES_ESP,
	},
	{ // Schizm: Mysterious Journey, French digital 10-language (GOG) version
		{
			"schizm",
			"French Digital",
			AD_ENTRY2s("Schizm.exe", "296edd26d951c3bdc4d303c4c88b27cd", 364544,
					   "0001_a.wav", "cd46a5df85a879bf293871b5911abcc3", 1305470),
			Common::UNK_LANG,
			Common::kPlatformWindows,
			ADGF_TESTING | VCRUISE_GF_WANT_OGG_VORBIS | VCRUISE_GF_NEED_JPEG,
			GUIO0()
		},
		GID_SCHIZM,
		Common::FR_FRA,
	},
	{ // Schizm: Mysterious Journey, Hungarian digital 10-language (GOG) version
		{
			"schizm",
			"Hungarian Digital",
			AD_ENTRY2s("Schizm.exe", "296edd26d951c3bdc4d303c4c88b27cd", 364544,
					   "0001_a.wav", "9838f59fce21ba3a1eecc3d84c5be8b1", 1502674),
			Common::UNK_LANG,
			Common::kPlatformWindows,
			ADGF_TESTING | VCRUISE_GF_WANT_OGG_VORBIS | VCRUISE_GF_NEED_JPEG,
			GUIO0()
		},
		GID_SCHIZM,
		Common::HU_HUN,
	},
	{ // Schizm: Mysterious Journey, Italian digital 10-language (GOG) version
		{
			"schizm",
			"Italian Digital",
			AD_ENTRY2s("Schizm.exe", "296edd26d951c3bdc4d303c4c88b27cd", 364544,
					   "0001_a.wav", "6d8e4b97710b858525d9965ef1e627ab", 1280506),
			Common::UNK_LANG,
			Common::kPlatformWindows,
			ADGF_TESTING | VCRUISE_GF_WANT_OGG_VORBIS | VCRUISE_GF_NEED_JPEG,
			GUIO0()
		},
		GID_SCHIZM,
		Common::IT_ITA,
	},
	{ // Schizm: Mysterious Journey, Polish digital 10-language (GOG) version
		{
			"schizm",
			"Polish Digital",
			AD_ENTRY2s("Schizm.exe", "296edd26d951c3bdc4d303c4c88b27cd", 364544,
					   "0001_a.wav", "4fc0053f66657e416b69419106d238a5", 1592106),
			Common::UNK_LANG,
			Common::kPlatformWindows,
			ADGF_TESTING | VCRUISE_GF_WANT_OGG_VORBIS | VCRUISE_GF_NEED_JPEG,
			GUIO0()
		},
		GID_SCHIZM,
		Common::PL_POL,
	},
	{ // Schizm: Mysterious Journey, Russian digital 10-language (GOG) version
		{
			"schizm",
			"Russian Digital",
			AD_ENTRY2s("Schizm.exe", "296edd26d951c3bdc4d303c4c88b27cd", 364544,
					   "0001_a.wav", "ba8fc041a92f1afceee35bdbb84eb2cd", 1427794),
			Common::UNK_LANG,
			Common::kPlatformWindows,
			ADGF_TESTING | VCRUISE_GF_WANT_OGG_VORBIS | VCRUISE_GF_NEED_JPEG,
			GUIO0()
		},
		GID_SCHIZM,
		Common::RU_RUS,
	},

	// Steam (16-language) versions
	{ // Schizm: Mysterious Journey, English digital 16-language (Steam) version
		{
			"schizm",
			"English Digital",
			AD_ENTRY3s("Schizm.exe", "296edd26d951c3bdc4d303c4c88b27cd", 364544,
					   "0001_a.wav", "374d93abc3422840623acc618ecb2b1e", 1553784,
					   "Speech10.txt", "88fdaab90be33bca88db423e1acda8c3", 63697),
			Common::UNK_LANG,
			Common::kPlatformWindows,
			ADGF_TESTING | VCRUISE_GF_WANT_OGG_VORBIS | VCRUISE_GF_NEED_JPEG | VCRUISE_GF_STEAM_LANGUAGES,
			GUIO0()
		},
		GID_SCHIZM,
		Common::EN_GRB,
	},
	{ // Schizm: Mysterious Journey, German digital 16-language (Steam) version
		{
			"schizm",
			"German Digital",
			AD_ENTRY3s("Schizm.exe", "296edd26d951c3bdc4d303c4c88b27cd", 364544,
					   "0001_a.wav", "f2b7eccfb1e9af0282b541c5eac66cc7", 1613240,
					   "Speech10.txt", "88fdaab90be33bca88db423e1acda8c3", 63697),
			Common::UNK_LANG,
			Common::kPlatformWindows,
			ADGF_TESTING | VCRUISE_GF_WANT_OGG_VORBIS | VCRUISE_GF_NEED_JPEG | VCRUISE_GF_STEAM_LANGUAGES,
			GUIO0()
		},
		GID_SCHIZM,
		Common::DE_DEU,
	},
	{ // Schizm: Mysterious Journey, Spanish digital 16-language (Steam) version
		{
			"schizm",
			"Spanish Digital",
			AD_ENTRY3s("Schizm.exe", "296edd26d951c3bdc4d303c4c88b27cd", 364544,
					   "0001_a.wav", "7a398f17e847a46de629a09fa6178b00", 1409560,
					   "Speech10.txt", "88fdaab90be33bca88db423e1acda8c3", 63697),
			Common::UNK_LANG,
			Common::kPlatformWindows,
			ADGF_TESTING | VCRUISE_GF_WANT_OGG_VORBIS | VCRUISE_GF_NEED_JPEG | VCRUISE_GF_STEAM_LANGUAGES,
			GUIO0()
		},
		GID_SCHIZM,
		Common::ES_ESP,
	},
	{ // Schizm: Mysterious Journey, French digital 16-language (Steam) version
		{
			"schizm",
			"French Digital",
			AD_ENTRY3s("Schizm.exe", "296edd26d951c3bdc4d303c4c88b27cd", 364544,
					   "0001_a.wav", "cd46a5df85a879bf293871b5911abcc3", 1305470,
					   "Speech10.txt", "88fdaab90be33bca88db423e1acda8c3", 63697),
			Common::UNK_LANG,
			Common::kPlatformWindows,
			ADGF_TESTING | VCRUISE_GF_WANT_OGG_VORBIS | VCRUISE_GF_NEED_JPEG | VCRUISE_GF_STEAM_LANGUAGES,
			GUIO0()
		},
		GID_SCHIZM,
		Common::FR_FRA,
	},
	{ // Schizm: Mysterious Journey, Hungarian digital 16-language (Steam) version
		{
			"schizm",
			"Hungarian Digital",
			AD_ENTRY3s("Schizm.exe", "296edd26d951c3bdc4d303c4c88b27cd", 364544,
					   "0001_a.wav", "9838f59fce21ba3a1eecc3d84c5be8b1", 1502674,
					   "Speech10.txt", "88fdaab90be33bca88db423e1acda8c3", 63697),
			Common::UNK_LANG,
			Common::kPlatformWindows,
			ADGF_TESTING | VCRUISE_GF_WANT_OGG_VORBIS | VCRUISE_GF_NEED_JPEG | VCRUISE_GF_STEAM_LANGUAGES,
			GUIO0()
		},
		GID_SCHIZM,
		Common::HU_HUN,
	},
	{ // Schizm: Mysterious Journey, Italian digital 16-language (Steam) version
		{
			"schizm",
			"Italian Digital",
			AD_ENTRY3s("Schizm.exe", "296edd26d951c3bdc4d303c4c88b27cd", 364544,
					   "0001_a.wav", "6d8e4b97710b858525d9965ef1e627ab", 1280506,
					   "Speech10.txt", "88fdaab90be33bca88db423e1acda8c3", 63697),
			Common::UNK_LANG,
			Common::kPlatformWindows,
			ADGF_TESTING | VCRUISE_GF_WANT_OGG_VORBIS | VCRUISE_GF_NEED_JPEG | VCRUISE_GF_STEAM_LANGUAGES,
			GUIO0()
		},
		GID_SCHIZM,
		Common::IT_ITA,
	},
	{ // Schizm: Mysterious Journey, Polish digital 16-language (Steam) version
		{
			"schizm",
			"Polish Digital",
			AD_ENTRY3s("Schizm.exe", "296edd26d951c3bdc4d303c4c88b27cd", 364544,
					   "0001_a.wav", "4fc0053f66657e416b69419106d238a5", 1592106,
					   "Speech10.txt", "88fdaab90be33bca88db423e1acda8c3", 63697),
			Common::UNK_LANG,
			Common::kPlatformWindows,
			ADGF_TESTING | VCRUISE_GF_WANT_OGG_VORBIS | VCRUISE_GF_NEED_JPEG | VCRUISE_GF_STEAM_LANGUAGES,
			GUIO0()
		},
		GID_SCHIZM,
		Common::PL_POL,
	},
	{ // Schizm: Mysterious Journey, Russian digital 16-language (Steam) version
		{
			"schizm",
			"Russian Digital",
			AD_ENTRY3s("Schizm.exe", "296edd26d951c3bdc4d303c4c88b27cd", 364544,
					   "0001_a.wav", "ba8fc041a92f1afceee35bdbb84eb2cd", 1427794,
					   "Speech10.txt", "88fdaab90be33bca88db423e1acda8c3", 63697),
			Common::UNK_LANG,
			Common::kPlatformWindows,
			ADGF_TESTING | VCRUISE_GF_WANT_OGG_VORBIS | VCRUISE_GF_NEED_JPEG | VCRUISE_GF_STEAM_LANGUAGES,
			GUIO0()
		},
		GID_SCHIZM,
		Common::RU_RUS,
	},

	{ AD_TABLE_END_MARKER, GID_UNKNOWN, Common::UNK_LANG }
};

} // End of namespace VCruise

#endif
