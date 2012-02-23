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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

namespace Mortevielle {

static const ADGameDescription MortevielleGameDescriptions[] = {
	// French
	{
		"mortevielle",
		"",
		{
			{"menufr.mor", 0, "e413f36b9e14eef16130adc347a9391f", 144},
			{"dxx.mor", 0, "949e68e829ecd5ad29e36a00347a9e7e", 207744},
			AD_LISTEND
		},
		Common::FR_FRA,
		Common::kPlatformPC,
		ADGF_NO_FLAGS,
		GUIO0()
	},
	// French
	{
		"mortevielle",
		"",
		{
			{"menu.mor", 0, "3fef0a3f8fca99fdcb6dbca8cbcef46f", 160},
			{"dxx.mor", 0, "949e68e829ecd5ad29e36a00347a9e7e", 207744},
			AD_LISTEND
		},
		Common::FR_FRA,
		Common::kPlatformPC,
		ADGF_NO_FLAGS,
		GUIO0()
	},
	// German
	{
		"mortevielle",
		"",
		{
			{"menual.mor", 0, "792aea282b07a1d74c4a4abeabc90c19", 144},
			{"dxx.mor", 0, "949e68e829ecd5ad29e36a00347a9e7e", 207744},
			AD_LISTEND
		},
		Common::DE_DEU,
		Common::kPlatformPC,
		ADGF_NO_FLAGS,
		GUIO0()
	},

	// English. Note that this is technically the French version, but English strings in mort.dat
	// will automatically replace all the French strings
	{
		"mortevielle",
		"",
		{
			{"menufr.mor", 0, "e413f36b9e14eef16130adc347a9391f", 144},
			{"dxx.mor", 0, "949e68e829ecd5ad29e36a00347a9e7e", 207744},
			AD_LISTEND
		},
		Common::EN_ANY,
		Common::kPlatformPC,
		ADGF_NO_FLAGS,
		GUIO0()
	},

	AD_TABLE_END_MARKER
};

} // End of namespace Mortevielle
