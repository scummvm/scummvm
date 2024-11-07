/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1994-1998 Revolution Software Ltd.
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
 */

#include "common/translation.h"

namespace Sword2 {

static const ADGameDescription gameDescriptions[] = {
	{
		"sword2",
		"Demo",
		AD_ENTRY2s("general.clu", "11e824864a75195652610e8b397382a6", 8030769,
				   "docks.clu",   "b39246fbb5b955a29f9a207c69bfc318", 20262263),
		Common::EN_ANY,
		Common::kPlatformWindows,
		ADGF_DEMO,
		GUIO0()
	},

	{
		"sword2",
		"PC Gamer Demo",
		AD_ENTRY2s("general.clu", "522ecd261027f0b55315a32aaef97295", 4519015,
				   "docks.clu",   "b3583fcf8a8f02109f3f528a4f64c1e6", 21017250),
		Common::EN_ANY,
		Common::kPlatformWindows,
		ADGF_DEMO,
		GUIO0()
	},

	{
		"sword2",
		"Demo",
		AD_ENTRY2s("players.clu", "5068815a62ba932afba7267bafc9786d", 9904289,
				   "docks.clu",   "c46d1150d826fff7f343edcc1cc430f7", 20375248),
		Common::ES_ESP,
		Common::kPlatformWindows,
		ADGF_DEMO | GF_SPANISHDEMO,
		GUIO0()
	},

	{
		"sword2",
		"Demo",
		AD_ENTRY2s("players.clu", "e8786804d399310bda3fcbf897bc44f7", 3085812,
				   "docks.clu",   "14470523a50333defc82c78afdf87b6b", 5818340),
		Common::EN_ANY,
		Common::kPlatformPSX,
		ADGF_DEMO,
		GUIO0()
	},

	{
		"sword2",
		"",
		AD_ENTRY4s("general.clu", "31db8564f9187538f24d9fda0677f666", 7059728,
				   "text.clu",    "9b344d976ca8d19a1cf5aa4413397f6b", 304968,
				   "speech1.clu", "a403904a0e825356107d228f8f74092e", 176260048,
				   "docks.clu",   "b39246fbb5b955a29f9a207c69bfc318", 20262263),
		Common::EN_ANY,
		Common::kPlatformWindows,
		ADGF_NO_FLAGS,
		GUIO0()
	},

	{ // Korean fan translation
		"sword2",
		"",
		AD_ENTRY6s("general.clu", "31db8564f9187538f24d9fda0677f666", 7059728,
				   "text.clu",    "9b344d976ca8d19a1cf5aa4413397f6b", 304968,
				   "speech1.clu", "a403904a0e825356107d228f8f74092e", 176260048,
				   "docks.clu",   "b39246fbb5b955a29f9a207c69bfc318", 20262263,
				   "bs2k.fnt",    nullptr,                            1222000,
				   "korean.clu",  nullptr,                            AD_NO_SIZE),
		Common::KO_KOR,
		Common::kPlatformWindows,
		ADGF_NO_FLAGS,
		GUIO0()
	},

	{ // GOG.com release version 2.0.0.6
		"sword2",
		"GOG",
		AD_ENTRY5s("general.clu", "31db8564f9187538f24d9fda0677f666", 7059728,
				   "text.clu",    "9b344d976ca8d19a1cf5aa4413397f6b", 304968,
				   "speech1.clu", "a403904a0e825356107d228f8f74092e", 176260048,
				   "docks.clu",   "b39246fbb5b955a29f9a207c69bfc318", 20262263,
				   "eye.dxa",     "7aef7fcb4faae760e82e0c7d3b336ac9", 7052599),
		Common::EN_ANY,
		Common::kPlatformWindows,
		ADGF_NO_FLAGS,
		GUIO0()
	},

	{
		"sword2",
		"",
		AD_ENTRY5s("general.clu", "31db8564f9187538f24d9fda0677f666", 7059728,
				   "text.clu",    "9b344d976ca8d19a1cf5aa4413397f6b", 304968,
				   "speech1.clu", "a403904a0e825356107d228f8f74092e", 176260048,
				   "docks.clu",   "b39246fbb5b955a29f9a207c69bfc318", 20262263,
				   "bs2.dat",   "c8238e7d017faa3b48d98df3f42a63e6", 336246),
		Common::ZH_CHN,
		Common::kPlatformWindows,
		ADGF_NO_FLAGS,
		GUIO0()
	},

	{
		"sword2",
		"1CD release",
		AD_ENTRY3s("general.clu",  "31db8564f9187538f24d9fda0677f666", 7059728,
				   "resource.tab", "ee4c0a8a2b8821ca113ea4176968b857", 16588,
				   "docks.clu",    "b39246fbb5b955a29f9a207c69bfc318", 20262263),
		Common::EN_ANY,
		Common::kPlatformWindows,
		ADGF_NO_FLAGS,
		GUIO0()
	},

	{
		"sword2",
		"EU",
		AD_ENTRY4s("general.clu", "31db8564f9187538f24d9fda0677f666", 7059728,
				   "text.clu",    "9b344d976ca8d19a1cf5aa4413397f6b", 304968,
				   "speech1.clg", "d49a5f3683b734d1129cbf6a0f95ae83", 57935499,
				   "docks.clu",   "b39246fbb5b955a29f9a207c69bfc318", 20262263),
		Common::EN_ANY,
		Common::kPlatformWindows,
		ADGF_NO_FLAGS,
		GUIO0()
	},

	{ // USA English, Windows 2-CD. Bugreport #15287
		"sword2",
		"USA 2-CD",
		AD_ENTRY3s("general.clu", "11e824864a75195652610e8b397382a6", 8030769,
				   "text.clu",    "2b3ff1803200fc155c1de09e9b2875b5", 337938,
				   "docks.clu",   "b39246fbb5b955a29f9a207c69bfc318", 20262263),
		Common::EN_USA,
		Common::kPlatformWindows,
		ADGF_NO_FLAGS,
		GUIO0()
	},

	{
		"sword2",
		"",
		AD_ENTRY3s("general.clu", "31db8564f9187538f24d9fda0677f666", 7059728,
				   "text.clu",    "d0cafb4d2982613ca4cf0574a0e4e079", 418165,
				   "docks.clu",   "b39246fbb5b955a29f9a207c69bfc318", 20262263),
		Common::FR_FRA,
		Common::kPlatformWindows,
		ADGF_NO_FLAGS,
		GUIO0()
	},

	{
		"sword2",
		"",
		AD_ENTRY3s("general.clu", "31db8564f9187538f24d9fda0677f666", 7059728,
				   "text.clu",    "5771f52410745029d7f71af05072d3d6", 556961,
				   "docks.clu",   "b39246fbb5b955a29f9a207c69bfc318", 20262263),
		Common::DE_DEU,
		Common::kPlatformWindows,
		ADGF_NO_FLAGS,
		GUIO0()
	},

	{
		"sword2",
		"",
		AD_ENTRY3s("general.clu", "31db8564f9187538f24d9fda0677f666", 7059728,
				   "text.clu",    "56c1197e72249473538c30c912607d01", 418165,
				   "docks.clu",   "b39246fbb5b955a29f9a207c69bfc318", 20262263),
		Common::ES_ESP,
		Common::kPlatformWindows,
		ADGF_NO_FLAGS,
		GUIO0()
	},

	{
		"sword2",
		"",
		AD_ENTRY3s("general.clu", "31db8564f9187538f24d9fda0677f666", 7059728,
				   "text.clu",    "c141e9903e4a1f45252dd1500498b6e2", 488745,
				   "docks.clu",   "b39246fbb5b955a29f9a207c69bfc318", 20262263),
		Common::IT_ITA,
		Common::kPlatformWindows,
		ADGF_NO_FLAGS,
		GUIO0()
	},

	{
		"sword2",
		"English speech",
		AD_ENTRY3s("general.clu", "31db8564f9187538f24d9fda0677f666", 7059728,
				   "text.clu",    "bc45e00cfb737ad61fada3ca6b1b2bfc", 279042,
				   "docks.clu",   "b39246fbb5b955a29f9a207c69bfc318", 20262263),
		Common::CS_CZE,
		Common::kPlatformWindows,
		ADGF_NO_FLAGS,
		GUIO0()
	},

	{
		"sword2",
		"English speech",
		AD_ENTRY3s("general.clu", "11e824864a75195652610e8b397382a6", 8030769,
				   "text.clu",    "9867bb6dfc850bfa165812f0827a5508", 454229,
				   "docks.clu",   "b39246fbb5b955a29f9a207c69bfc318", 20262263),
		Common::FI_FIN,
		Common::kPlatformWindows,
		ADGF_NO_FLAGS,
		GUIO0()
	},

	{
		"sword2",
		"Fanmade/English speech/V1",
		AD_ENTRY3s("general.clu", "31db8564f9187538f24d9fda0677f666", 7059728,
				   "text.clu",    "93ea23ccf78dc746ed9a027fcf66d58d", 248692,
				   "docks.clu",   "b39246fbb5b955a29f9a207c69bfc318", 20262263),
		Common::HE_ISR,
		Common::kPlatformWindows,
		ADGF_NO_FLAGS,
		GUIO0()
	},

	{
		"sword2",
		"Fanmade/English speech/V2",
		AD_ENTRY3s("general.clu", "31db8564f9187538f24d9fda0677f666", 7059728,
				   "text.clu",    "b5442676e958309bf1a4817dd3893aab", 248702,
				   "docks.clu",   "b39246fbb5b955a29f9a207c69bfc318", 20262263),
		Common::HE_ISR,
		Common::kPlatformWindows,
		ADGF_NO_FLAGS,
		GUIO0()
	},

	{
		"sword2",
		"English speech",
		AD_ENTRY3s("general.clu", "31db8564f9187538f24d9fda0677f666", 7059728,
				   "text.clu",    "82714fa70516486174cddc2754958cd4", 304968,
				   "docks.clu",   "b39246fbb5b955a29f9a207c69bfc318", 20262263),
		Common::HU_HUN,
		Common::kPlatformWindows,
		ADGF_NO_FLAGS,
		GUIO0()
	},

	{
		"sword2",
		"English speech",
		AD_ENTRY3s("general.clu", "11e824864a75195652610e8b397382a6", 8030769,
				   "text.clu",    "f1cf2aaa7e56d8bf6572c9b25267931e", 373704,
				   "docks.clu",   "b39246fbb5b955a29f9a207c69bfc318", 20262263),
		Common::PL_POL,
		Common::kPlatformWindows,
		ADGF_NO_FLAGS,
		GUIO0()
	},

	{ // Alternate version. Bugreport #14277
		"sword2",
		"English speech/alternate version",
		AD_ENTRY3s("general.clu", "31db8564f9187538f24d9fda0677f666", 7059728,
				   "text.clu",    "77fb6f58acad0f9c4eebeb5527b32861", 410707,
				   "docks.clu",   "b39246fbb5b955a29f9a207c69bfc318", 20262263),
		Common::PL_POL,
		Common::kPlatformWindows,
		ADGF_NO_FLAGS,
		GUIO0()
	},

	{
		"sword2",
		"English speech",
		AD_ENTRY3s("general.clu", "31db8564f9187538f24d9fda0677f666", 7059728,
				   "text.clu",    "cda6306bedfa63ac4386ff82977bfcd6", 410949,
				   "docks.clu",   "b39246fbb5b955a29f9a207c69bfc318", 20262263),
		Common::PT_BRA,
		Common::kPlatformWindows,
		ADGF_NO_FLAGS,
		GUIO0()
	},

	{
		"sword2",
		"Fargus",
		AD_ENTRY3s("general.clu", "98e43a4fd93227b1d5d44e664eeede0c", 7320908,
				   "text.clu",    "33a2645498ef1f4e63c4f6a50da4a3e2", 288998,
				   "docks.clu",   "b39246fbb5b955a29f9a207c69bfc318", 20262263),
		Common::RU_RUS,
		Common::kPlatformWindows,
		ADGF_NO_FLAGS,
		GUIO0()
	},

	{
		"sword2",
		"Mediahauz/English speech",
		AD_ENTRY3s("general.clu", "31db8564f9187538f24d9fda0677f666", 7059728,
				   "text.clu",    "e85c148037b8bfc02c968d4d22fda5e1", 315178,
				   "docks.clu",   "b39246fbb5b955a29f9a207c69bfc318", 20262263),
		Common::RU_RUS,
		Common::kPlatformWindows,
		ADGF_NO_FLAGS,
		GUIO0()
	},

	{	// TRAC report #14550
		"sword2",
		"Novy Disk",
		AD_ENTRY2s("general.clu", "60d17ec7eb80fec561e1278ff5a32faa", 7093894,
				   "docks.clu",   "b39246fbb5b955a29f9a207c69bfc318", 20262263),
		Common::RU_RUS,
		Common::kPlatformWindows,
		ADGF_NO_FLAGS,
		GUIO0()
	},

	{
		"sword2",
		"",
		AD_ENTRY3s("general.clu", "7ef0353ca03338d59b4f4e3d01a38df1", 2095780,
				   "text.clu",    "06691fc9f749f3f7ad0f622fbfe79467", 302756,
				   "docks.clu",   "95ee20f4c61de6acc3243ba4632e37d8", 5654572),
		Common::EN_USA,
		Common::kPlatformPSX,
		ADGF_NO_FLAGS,
		GUIO0()
	},

	{
		"sword2",
		"",
		AD_ENTRY3s("general.clu", "7ef0353ca03338d59b4f4e3d01a38df1", 2095780,
				   "text.clu",    "be8ad3f1d9d3ddd8881169b16aa23970", 838392,
				   "docks.clu",   "95ee20f4c61de6acc3243ba4632e37d8", 5654572),
		Common::EN_GRB,
		Common::kPlatformPSX,
		ADGF_NO_FLAGS,
		GUIO0()
	},

	{
		"sword2",
		"",
		AD_ENTRY3s("general.clu", "7ef0353ca03338d59b4f4e3d01a38df1", 2095780,
				   "text.clu",    "0920f1aec8bc9d02f8c94f73965c8006", 327668,
				   "docks.clu",   "95ee20f4c61de6acc3243ba4632e37d8", 5654572),
		Common::FR_FRA,
		Common::kPlatformPSX,
		ADGF_NO_FLAGS,
		GUIO0()
	},

	{
		"sword2",
		"",
		AD_ENTRY3s("general.clu", "7ef0353ca03338d59b4f4e3d01a38df1", 2095780,
				   "text.clu",    "5ce53dfc154b80d4ca64b60df808e411", 347456,
				   "docks.clu",   "95ee20f4c61de6acc3243ba4632e37d8", 5654572),
		Common::DE_DEU,
		Common::kPlatformPSX,
		ADGF_NO_FLAGS,
		GUIO0()
	},

	{
		"sword2",
		"",
		AD_ENTRY3s("general.clu", "7ef0353ca03338d59b4f4e3d01a38df1", 2095780,
				   "text.clu",    "03ffcd1eec48f74a3d16d1b7751cee0b", 316124,
				   "docks.clu",   "95ee20f4c61de6acc3243ba4632e37d8", 5654572),
		Common::ES_ESP,
		Common::kPlatformPSX,
		ADGF_NO_FLAGS,
		GUIO0()
	},

	{
		"sword2",
		"",
		AD_ENTRY3s("general.clu", "7ef0353ca03338d59b4f4e3d01a38df1", 2095780,
				   "text.clu",    "298bd6eef464780bf6b0830805eef220", 334784,
				   "docks.clu",   "95ee20f4c61de6acc3243ba4632e37d8", 5654572),
		Common::IT_ITA,
		Common::kPlatformPSX,
		ADGF_NO_FLAGS,
		GUIO0()
	},

	{	// Remastered version, not supported
		"sword2",
		_s("Remastered edition is not supported. Please, use the classic version"),
		AD_ENTRY2s("general.clu", "5b237f3d0bbe05ceb94e271616c6e560", 33964,
				   "docks.clu",   "9b5ddad1fb436b4897df9c6632cccbbe", 21641864),
		Common::UNK_LANG,
		Common::kPlatformUnknown,
		ADGF_UNSUPPORTED,
		GUIO0()
	},


	AD_TABLE_END_MARKER
};

} // End of namespace Sword2
