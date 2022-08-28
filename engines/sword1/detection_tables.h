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


namespace Sword1 {

static const SwordGameDescription gameDescriptions[] = {
	{
		{
			"sword1",
			"Demo",
			AD_ENTRY2s("compacts.clu", "7128bc4e99701d99c22e5841051671d3", 200156,
			           "swordres.rif", "bc01bc995f23e46bb076f66ba5c516c4", 58388),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_DEMO,
			GUIO0()
		},
		GF_DEMO
	},

	{
		{
			"sword1",
			"Demo",
			AD_ENTRY2s("compacts.clu", "7128bc4e99701d99c22e5841051671d3", 200852,
			           "swordres.rif", "3786c6850e51ecbadb65bbedb5395664", 59788),
			Common::ES_ESP,
			Common::kPlatformWindows,
			ADGF_DEMO,
			GUIO0()
		},
		GF_DEMO
	},

	{
		{
			"sword1",
			"Demo",
			AD_ENTRY2s("compacts.clm", "3a77d3cd0ca6c91a4f4e94459b1da773", 200852,
			           "swordres.rif", "6b579d7cd94756f5c1e362a9b61f94a3", 59788),
			Common::EN_ANY,
			Common::kPlatformMacintosh,
			ADGF_DEMO,
			GUIO0()
		},
		GF_DEMO
	},

	{
		{
			"sword1",
			"Demo",
			AD_ENTRY2s("compacts.clu", "7128bc4e99701d99c22e5841051671d3", 200852,
			           "swordres.rif", "5bd8928071b91830be6fbcdb7f59786d", 59788),
			Common::EN_ANY,
			Common::kPlatformPSX,
			ADGF_DEMO,
			GUIO0()
		},
		GF_DEMO
	},

	{ AD_TABLE_END_MARKER, 0 }
};

} // End of namespace Sword1
