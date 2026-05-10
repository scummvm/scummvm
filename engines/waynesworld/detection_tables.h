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

namespace WaynesWorld {

static const ADGameDescription gameDescriptions[] = {
	{
		"waynesworld",
		"",
		AD_ENTRY2s("R00.GXL", "197ae9fb74a79a6dad4e3336f94c7545", 36272,
				"e00.txt", "78769d60d48aea314ed6be2f725c8c2f", 1293),
		Common::EN_ANY,
		Common::kPlatformDOS,
		ADGF_UNSTABLE,
		GUIO1(GUIO_NOASPECT)
	},

	// Provided by gabberhead, ticket #16627
	{
		"waynesworld",
		 "",
		 AD_ENTRY2s("R00.GXL", "197ae9fb74a79a6dad4e3336f94c7545", 36272,
					"e00.txt", "b5042542d0df2fcd8c8e617ee0900ce8", 1447),
		 Common::DE_DEU,
		 Common::kPlatformDOS,
		 ADGF_UNSTABLE,
		 GUIO1(GUIO_NOASPECT)
	},

	{
		"waynesworld",
		"Demo",
		AD_ENTRY2s("R00.GXL", "6a05df806dedb69f1333cb9d81b6821c", 34392,
					"CAPSPIN.GXL", "b9d2cd46b4ebe49fcf634b221f11f8df", 281089),
		Common::EN_ANY,
		Common::kPlatformDOS,
		ADGF_DEMO | ADGF_UNSTABLE,
		GUIO1(GUIO_NOASPECT)
	},

	{
		"waynesworld",
		"Selectware Demo",
		AD_ENTRY2s("R00.GXL", "6a05df806dedb69f1333cb9d81b6821c", 34392,
					"CAP.GXL", "4823b4ae534716b8ea115ab34b7eac6b", 87864),
		Common::EN_ANY,
		Common::kPlatformDOS,
	 ADGF_DEMO | ADGF_UNSTABLE,
		GUIO1(GUIO_NOASPECT)
	},

	AD_TABLE_END_MARKER
};

} // End of namespace WaynesWorld
