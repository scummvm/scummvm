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

namespace Wage {

#define ADGF_DEFAULT (ADGF_DROPLANGUAGE|ADGF_DROPPLATFORM)
#define ADGF_GENERIC (ADGF_DROPLANGUAGE|ADGF_DROPPLATFORM|ADGF_USEEXTRAASTITLE)

static const ADGameDescription gameDescriptions[] = {
	{
		"wage",
		"3rd Floor",
		AD_ENTRY1s("3rd Floor", "a107d7a177970b2259e32681bd8b47c9", 285056),
		Common::EN_ANY,
		Common::kPlatformMacintosh,
		ADGF_GENERIC,
		GUIO0()
	},
	{
		"afm",
		"v1.8",
		AD_ENTRY1s("Another Fine Mess 1.8", "8e5aa915f3253efb2aab52435647b25e", 1456000),
		Common::EN_ANY,
		Common::kPlatformMacintosh,
		ADGF_DEFAULT,
		GUIO0()
	},
	{
		"amot",
		"v1.8",
		AD_ENTRY1s("A Mess O' Trouble 1.8", "b3ef53afed282671b704e45df829350c", 1895552),
		Common::EN_ANY,
		Common::kPlatformMacintosh,
		ADGF_DEFAULT,
		GUIO0()
	},
	{
		"wage",
		"Bug Hunt",
		AD_ENTRY1s("Bug Hunt", "2ebd3515a87941063ad66c3cf93c5e78", 200064),
		Common::EN_ANY,
		Common::kPlatformMacintosh,
		ADGF_GENERIC,
		GUIO0()
	},
	{
		"wage",
		"Canal District",
		AD_ENTRY1s("Canal District", "8856bc699a20fc5b7fc67accee12cac7", 658176),
		Common::EN_ANY,
		Common::kPlatformMacintosh,
		ADGF_GENERIC,
		GUIO0()
	},
	{
		"cantitoe",
		"",
		AD_ENTRY1s("Camp Cantitoe", "098aa5c11c58e1ef274a30a9e01b4755", 621440),
		Common::EN_ANY,
		Common::kPlatformMacintosh,
		ADGF_DEFAULT,
		GUIO0()
	},
	{
		"wage",
		"Deep Angst",
		AD_ENTRY1s("Deep Angst", "635f62bbc569e72b03cab9107927d03d", 335232),
		Common::EN_ANY,
		Common::kPlatformMacintosh,
		ADGF_GENERIC,
		GUIO0()
	},
	{
		"wage",
		"Escape from School!",
		AD_ENTRY1s("Escape from School!", "a854be48d4af20126d18a9cad93a969b", 51840),
		Common::EN_ANY,
		Common::kPlatformMacintosh,
		ADGF_GENERIC,
		GUIO0()
	},
	{
		"wage",
		"Magic Rings",
		AD_ENTRY1s("Magic Rings", "6e0d1dd561d3dad8f9a7a20ed1f09b16", 112000),
		Common::EN_ANY,
		Common::kPlatformMacintosh,
		ADGF_GENERIC,
		GUIO0()
	},
	{
		"wage",
		"Midnight Snack",
		AD_ENTRY1s("Midnight Snack", "346982a32fc701f53bb19771d72063d0", 69504),
		Common::EN_ANY,
		Common::kPlatformMacintosh,
		ADGF_GENERIC,
		GUIO0()
	},
	{
		"wage",
		"Queen Quest",
		AD_ENTRY1s("Queen Quest", "730605d312efedb5e3ff108522fcac18", 59776),
		Common::EN_ANY,
		Common::kPlatformMacintosh,
		ADGF_GENERIC,
		GUIO0()
	},
	{
		"scepters",
		"",
		AD_ENTRY1s("Scepters", "b80bff315897776dda7689cdf829fab4", 360832),
		Common::EN_ANY,
		Common::kPlatformMacintosh,
		ADGF_DEFAULT,
		GUIO0()
	},
	{
		"wage",
		"Time Bomb",
		AD_ENTRY1s("Time Bomb", "2df84b636237686b624e736a698a16c4", 66432),
		Common::EN_ANY,
		Common::kPlatformMacintosh,
		ADGF_GENERIC,
		GUIO0()
	},
	{
		"wage",
		"ZikTuria",
		AD_ENTRY1s("ZikTuria", "e793155bed1a70fa2074a3fcd696b751", 54784),
		Common::EN_ANY,
		Common::kPlatformMacintosh,
		ADGF_GENERIC,
		GUIO0()
	},
	{
		"wage",
		"Zoony",
		AD_ENTRY1s("Zoony", "e6cc8a914a4215dafbcce6315dd12cf5", 160256),
		Common::EN_ANY,
		Common::kPlatformMacintosh,
		ADGF_GENERIC,
		GUIO0()
	},

	AD_TABLE_END_MARKER
};

} // End of namespace Wage
