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

namespace Twp {

const PlainGameDescriptor twpGames[] = {
	{"twp", "Thimbleweed Park"},
	{0, 0}};

const TwpGameDescription gameDescriptions[] = {
	// Thimbleweed Park - STEAM/1.0.859
	{
		{
			"twp",
			"STEAM/1.0.859",
			AD_ENTRY1s("ThimbleweedPark.ggpack1", "b1c35b7a6b5b0c2e6f466ea384f22558", 435353845),
			Common::UNK_LANG,
			Common::kPlatformUnknown,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOMIDI)
		},
		GAME_XORKEY_5B6D,
		LS_WITHOUT_RUSSIAN
	},
	// Thimbleweed Park - EPIC Games/1.0.955
	{
		{
			"twp",
			"EPIC Games/1.0.955",
			AD_ENTRY1s("ThimbleweedPark.ggpack1", "a97546ee2d9e19aab59714a267009a31", 502540584),
			Common::UNK_LANG,
			Common::kPlatformUnknown,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOMIDI)
		},
		GAME_XORKEY_566D,
		LS_WITH_RUSSIAN
	},
	// Thimbleweed Park - GOG/1.0.938
	{
		{
			"twp",
			"GOG/1.0.938",
			AD_ENTRY1s("ThimbleweedPark.ggpack1", "5532019821c077dda5cf86b619e4b173", 502495748),
			Common::UNK_LANG,
			Common::kPlatformUnknown,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOMIDI)
		},
		GAME_XORKEY_566D,
		LS_WITH_RUSSIAN
	},
	// Thimbleweed Park - GOG/1.0.951
  	{
		{
			"twp",
			"GOG/1.0.951",
			AD_ENTRY1s("ThimbleweedPark.ggpack1", "f0bd29df9fcaba3d4047eac1046e0abf", 502503739),
			Common::UNK_LANG,
			Common::kPlatformUnknown,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOMIDI)
		},
		GAME_XORKEY_566D,
		LS_WITH_RUSSIAN
	},
	// Thimbleweed Park - GOG/1.0.957
  	{
		{
			"twp",
			"GOG/1.0.957",
			AD_ENTRY1s("ThimbleweedPark.ggpack1", "5631cf51cb7afc4babf7f2d5a8bdfc21", 502661437),
			Common::UNK_LANG,
			Common::kPlatformUnknown,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOMIDI)
		},
		GAME_XORKEY_56AD,
		LS_WITH_RUSSIAN
	},
  	// Thimbleweed Park - GOG/1.0.958
	{
		{
			"twp",
			"GOG/1.0.958",
			AD_ENTRY1s("ThimbleweedPark.ggpack1", "6180145221d18e9e9caac6459e840579", 502661439),
			Common::UNK_LANG,
			Common::kPlatformUnknown,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOMIDI)
		},
		GAME_XORKEY_56AD,
		LS_WITH_RUSSIAN
	},
	// Thimbleweed Park - Switch/1.0.952
	{
		{
			"twp",
			"Switch/1.0.952",
			AD_ENTRY1s("ThimbleweedPark.ggpack1", "6a3f457cf0545d7a5122fb8bcd4d62a6", 502503742),
			Common::UNK_LANG,
			Common::kPlatformUnknown,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOMIDI)
		},
		GAME_XORKEY_566D,
		LS_WITH_RUSSIAN
	},
	{AD_TABLE_END_MARKER,(GameXorKey)0, (LanguageSupported)0}};

} // End of namespace Twp
