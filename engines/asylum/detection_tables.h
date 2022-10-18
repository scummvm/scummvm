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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

static const PlainGameDescriptor asylumGames[] = {
	{ "asylum", "Sanitarium" },
	{ 0, 0 }
};

namespace Asylum {

static const char *directoryGlobs[] = {
	"Vids",
	0
};

static const ADGameDescription gameDescriptions[] = {
	{
		// English CD version (unpatched)
		"asylum",
		"CD Unpatched",
		{
			{"SNTRM.DAT", 0, "7cfcc457c1f579fbf9878ac175d29374", 8930},
			{"RES.000",   0, "4d13f1412f9e7dd3eaf0a58f0e00e662", 272057},
			{"SCN.006",   0, "71a5f49cbda597a11e877589075e27b5", 2918330},
			AD_LISTEND
		},
		Common::EN_ANY,
		Common::kPlatformWindows,
		ADGF_DROPPLATFORM,
		GUIO0()
	},
	{
		// English CD version (patched)
		// Level 2 Lockout Bug patch was applied
		"asylum",
		"CD Patched",
		{
			{"SNTRM.DAT", 0, "7cfcc457c1f579fbf9878ac175d29374", 8930},
			{"RES.000",   0, "4d13f1412f9e7dd3eaf0a58f0e00e662", 272057},
			{"SCN.006",   0, "3a5b54da08198012dc0614114782d5fb", 2918330},
			AD_LISTEND
		},
		Common::EN_ANY,
		Common::kPlatformWindows,
		ADGF_DROPPLATFORM,
		GUIO0()
	},
	{
		// English GOG.com version
		"asylum",
		"GOG.com",
		{
			{"SNTRM.DAT", 0, "7cfcc457c1f579fbf9878ac175d29374", 8930},
			{"RES.000",   0, "f58f8dc3e63663f174977d359e11132c", 272057},
			{"SCN.006",   0, "3a5b54da08198012dc0614114782d5fb", 2918330},
			{"MOV000.SMK", 0, NULL, -1},
			AD_LISTEND
		},
		Common::EN_ANY,
		Common::kPlatformWindows,
		ADGF_DROPPLATFORM,
		GUIO0()
	},
	{
		// French GOG.com version
		"asylum",
		"GOG.com",
		{
			{"SNTRM.DAT", 0, "e09a12543c6ede1727c8ecffb74e7fd2", 8930},
			{"RES.000",   0, "2a4677ee3fd7bfa3161e16cd59371238", 277931},
			{"SCN.006",   0, "3a5b54da08198012dc0614114782d5fb", 2918330},
			{"MOV000.SMK", 0, NULL, -1},
			AD_LISTEND
		},
		Common::FR_FRA,
		Common::kPlatformWindows,
		ADGF_DROPPLATFORM,
		GUIO0()
	},
	{
		// English Steam version
		"asylum",
		"Steam",
		{
			{"SNTRM.DAT", 0, "7cfcc457c1f579fbf9878ac175d29374", 8930},
			{"RES.000",   0, "f58f8dc3e63663f174977d359e11132c", 272057},
			{"SCN.006",   0, "3a5b54da08198012dc0614114782d5fb", 2918330},
			{"MOV000_2_SMK.ogv", 0, NULL, -1},
			AD_LISTEND
		},
		Common::EN_ANY,
		Common::kPlatformWindows,
		ADGF_DROPPLATFORM,
		GUIO0()
	},
	{
		// German Steam version
		"asylum",
		"Steam",
		{
			{"SNTRM.DAT", 0, "f427fda37a0e29afd4acf982c4cb9fb0", 8930},
			{"RES.000",   0, "0578f326b40d22f661ac93cf49dc2c19", 285658},
			{"SCN.006",   0, "3a5b54da08198012dc0614114782d5fb", 2918330},
			{"MOV000_2_SMK.ogv", 0, NULL, -1},
			AD_LISTEND
		},
		Common::DE_DEU,
		Common::kPlatformWindows,
		ADGF_DROPPLATFORM,
		GUIO0()
	},
	{
		// French Steam version
		"asylum",
		"Steam",
		{
			{"SNTRM.DAT", 0, "e09a12543c6ede1727c8ecffb74e7fd2", 8930},
			{"RES.000",   0, "94f3c795599ea79492cbf81127944f37", 277935},
			{"SCN.006",   0, "3a5b54da08198012dc0614114782d5fb", 2918330},
			{"MOV000_2_SMK.ogv", 0, NULL, -1},
			AD_LISTEND
		},
		Common::FR_FRA,
		Common::kPlatformWindows,
		ADGF_DROPPLATFORM,
		GUIO0()
	},
	{
		// French CD version
		"asylum",
		"CD",
		{
			{"SNTRM.DAT", 0, "e09a12543c6ede1727c8ecffb74e7fd2", 8930},
			{"RES.000",   0, "2a4677ee3fd7bfa3161e16cd59371238", 277931},
			{"SCN.006",   0, "3a5b54da08198012dc0614114782d5fb", 2918330},
			AD_LISTEND
		},
		Common::FR_FRA,
		Common::kPlatformWindows,
		ADGF_DROPPLATFORM,
		GUIO0()
	},
	{
		// German CD version
		"asylum",
		"CD",
		{
			{"SNTRM.DAT", 0, "f427fda37a0e29afd4acf982c4cb9fb0", 8930},
			{"RES.000",   0, "058ea2fca583c27a8e3d7ccc16a9a3bd", 285660},
			{"SCN.006",   0, "3a5b54da08198012dc0614114782d5fb", 2918330},
			AD_LISTEND
		},
		Common::DE_DEU,
		Common::kPlatformWindows,
		ADGF_DROPPLATFORM,
		GUIO0()
	},
	{
		// Russian CD version
		"asylum",
		"CD",
		{
			{"SNTRM.DAT", 0, "7cfcc457c1f579fbf9878ac175d29374", 8930},
			{"RES.000",   0, "1f8a262bf8b3035bd3cfda24068c4651", 263624},
			{"SCN.006",   0, "3a5b54da08198012dc0614114782d5fb", 2918330},
			AD_LISTEND
		},
		Common::RU_RUS,
		Common::kPlatformWindows,
		ADGF_DROPPLATFORM,
		GUIO0()
	},
	{
		// Unofficial Russian translation (Fargus Multimedia)
		"asylum",
		"GOG.com",
		{
			{"SNTRM.DAT", 0, "7cfcc457c1f579fbf9878ac175d29374", 8930},
			{"RES.000",   0, "1f7b3734a8da87464ce3765bd79be208", 247149},
			{"SCN.006",   0, "3a5b54da08198012dc0614114782d5fb", 2918330},
			{"MOV000.SMK", 0, NULL, -1},
			AD_LISTEND
		},
		Common::RU_RUS,
		Common::kPlatformWindows,
		ADGF_DROPPLATFORM,
		GUIO0()
	},
	{
		// Unofficial Russian translation (Fargus Multimedia)
		"asylum",
		"Steam",
		{
			{"SNTRM.DAT", 0, "7cfcc457c1f579fbf9878ac175d29374", 8930},
			{"RES.000",   0, "1f7b3734a8da87464ce3765bd79be208", 247149},
			{"SCN.006",   0, "3a5b54da08198012dc0614114782d5fb", 2918330},
			{"MOV000_2_SMK.ogv", 0, NULL, -1},
			AD_LISTEND
		},
		Common::RU_RUS,
		Common::kPlatformWindows,
		ADGF_DROPPLATFORM,
		GUIO0()
	},
	{
		// Polish fanmade translation
		"asylum",
		"Fanmade",
		{
			{"SNTRM.DAT", 0, "7cfcc457c1f579fbf9878ac175d29374", 8930},
			{"RES.000",   0, "38989acceb9c7942b8758945768c5f85", 279825},
			{"SCN.006",   0, "3a5b54da08198012dc0614114782d5fb", 2918330},
			AD_LISTEND
		},
		Common::PL_POL,
		Common::kPlatformWindows,
		ADGF_DROPPLATFORM,
		GUIO0()
	},
	{
		// Spanish fanmade translation
		"asylum",
		"Fanmade",
		{
			{"SNTRM.DAT", 0, "7cfcc457c1f579fbf9878ac175d29374", 8930},
			{"RES.000",   0, "4a112c41f58ac89b472be8cbfe82ab4b", 272057},
			{"SCN.006",   0, "3a5b54da08198012dc0614114782d5fb", 2918330},
			AD_LISTEND
		},
		Common::ES_ESP,
		Common::kPlatformWindows,
		ADGF_DROPPLATFORM,
		GUIO0()
	},
	{
		// Italian fanmade translation (CD version)
		"asylum",
		"Fanmade",
		{
			{"SNTRM.DAT", 0, "7cfcc457c1f579fbf9878ac175d29374", 8930},
			{"RES.000",   0, "354521af1d7874fb048b1dea00311fa4", 285198},
			{"SCN.006",   0, "3a5b54da08198012dc0614114782d5fb", 2918330},
			AD_LISTEND
		},
		Common::IT_ITA,
		Common::kPlatformWindows,
		ADGF_DROPPLATFORM,
		GUIO0()
	},
	{
		// Italian fanmade translation (GOG version)
		"asylum",
		"Fanmade",
		{
			{"SNTRM.DAT", 0, "7cfcc457c1f579fbf9878ac175d29374", 8930},
			{"RES.000",   0, "fa342da449bbf87c43b7c8fc51747ee4", 285198},
			{"SCN.006",   0, "3a5b54da08198012dc0614114782d5fb", 2918330},
			AD_LISTEND
		},
		Common::IT_ITA,
		Common::kPlatformWindows,
		ADGF_DROPPLATFORM,
		GUIO0()
	},
	{
		// English Demo
		"asylum",
		"Demo",
		AD_ENTRY2s("SNTRM.DAT", "6a8079e0b49affc99d148624fbb584a1", 326,
				   "RES.000",   "57446ce089c6f9decb7f3faf41d09edf", 9022),
		Common::EN_ANY,
		Common::kPlatformWindows,
		ADGF_DEMO | ADGF_DROPPLATFORM,
		GUIO0()
	},
	{
		// English Alt Demo
		"asylum",
		"Demo",
		AD_ENTRY2s("ASYLUM.DAT", "6a8079e0b49affc99d148624fbb584a1", 326,
				   "RES.000",    "a527449397576a019cdd4884a96403e2", 9012),
		Common::EN_ANY,
		Common::kPlatformWindows,
		ADGF_DEMO | ADGF_DROPPLATFORM,
		GUIO0()
	},
	AD_TABLE_END_MARKER
};

static const ADFileBasedFallback fileBasedFallback[] = {
	{ &gameDescriptions[0], { "SNTRM.DAT", 0 } }, // default to english version
	{ 0, { 0 } }
};

} // End of namespace Asylum
