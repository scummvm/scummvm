/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

namespace Mortevielle {

static const MortevielleGameDescription MortevielleGameDescriptions[] = {
	// French
	{
		{
			"mortevielle",
			"",
			{
				{"menufr.mor", 0, "e413f36b9e14eef16130adc347a9391f", 144},
				{"dxx.mor", 0, "949e68e829ecd5ad29e36a00347a9e7e", 207744},
				AD_LISTEND
			},
			Common::FR_FRA,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO0()
		}, Common::FR_FRA, kUseOriginalData
	},
	// German
	{
		{
			"mortevielle",
			"",
			{
				{"menual.mor", 0, "792aea282b07a1d74c4a4abeabc90c19", 144},
				{"dxx.mor", 0, "949e68e829ecd5ad29e36a00347a9e7e", 207744},
				AD_LISTEND
			},
			Common::DE_DEU,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO0()
		}, Common::DE_DEU, kUseOriginalData
	},

	// German, improved translation
//	{
//		{
//			"mortevielle",
//			"Improved Translation",
//			{
//				{"menual.mor", 0, "792aea282b07a1d74c4a4abeabc90c19", 144},
//				{"dxx.mor", 0, "949e68e829ecd5ad29e36a00347a9e7e", 207744},
//				AD_LISTEND
//			},
//			Common::DE_DEU,
//			Common::kPlatformDOS,
//			ADGF_NO_FLAGS,
//			GUIO0()
//		}, Common::DE_DEU, kUseEngineDataFile
//	},

	// DOS English version doesn't exist. Technically, they are French or German versions,
	// using English strings stored mort.dat

	// English on top of French version
	{
		{
			"mortevielle",
				"",
			{
				{"menufr.mor", 0, "e413f36b9e14eef16130adc347a9391f", 144},
				{"dxx.mor", 0, "949e68e829ecd5ad29e36a00347a9e7e", 207744},
				AD_LISTEND
			},
			Common::EN_ANY,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO0()
		}, Common::FR_FRA, kUseEngineDataFile
	},

	// English on top of German version
	{
		{
			"mortevielle",
				"",
			{
				{"menual.mor", 0, "792aea282b07a1d74c4a4abeabc90c19", 144},
				{"dxx.mor", 0, "949e68e829ecd5ad29e36a00347a9e7e", 207744},
				AD_LISTEND
			},
			Common::EN_ANY,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO0()
		}, Common::DE_DEU, kUseEngineDataFile
	},

	// French, provided by ultrapingu in bug ref #6575
	{
		{
			"mortevielle",
			"",
			{
				{"menu.mor", 0, "3fef0a3f8fca99fdcb6dbca8cbcef46f", 160},
				{"dxx.mor", 0, "949e68e829ecd5ad29e36a00347a9e7e", 207744},
				AD_LISTEND
			},
			Common::FR_FRA,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO0()
		}, Common::FR_FRA, kUseEngineDataFile
	},

	{ AD_TABLE_END_MARKER , Common::EN_ANY, kUseEngineDataFile}
};

} // End of namespace Mortevielle
