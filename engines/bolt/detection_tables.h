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

namespace Bolt {

const PlainGameDescriptor boltGames[] = {
	{ "carnival", "Cartoon Carnival" },
	{ 0, 0 }
};

const ADGameDescription gameDescriptions[] = {
	{
		"carnival",
		nullptr,
		{
			{"BOOTHS.BLT", 0, "db13e86a850b9844bf1bab19e9ec2f6f", 1326163},
			{"SCOOBY.BLT", 0, "c0918e36e532a24e2fc4942261d907aa", 3733392},
			AD_LISTEND
		},
		Common::EN_USA,
		Common::kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO1(GAMEOPTION_EXTEND_SCREEN)
	},
	{
		"carnival",
		nullptr,
		{
			{"BOOTHS.BLT", 0, "c4d7ce39d1dd754a50bd909c1c2ed695", 1328201},
			{"SCOOBY.BLT", 0, "acf03553bc48c070adf57679307c900c", 3733392},
			AD_LISTEND
		},
		Common::IT_ITA,
		Common::kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO1(GAMEOPTION_EXTEND_SCREEN)
	},
	{
		"carnival",
		"Demo",
		{
			{"BOOTHS.BLT", 0, "3fc80c0feaaa300720a3ed921496233a", 878177 },
			{"SCOOBY.BLT", 0, "4dfbdd87fdd2ec05fbdc7ae6435fb239", 677438 },
			AD_LISTEND
		},
		Common::IT_ITA,
		Common::kPlatformWindows,
		ADGF_UNSTABLE | ADGF_DEMO,
		GUIO1(GAMEOPTION_EXTEND_SCREEN)
	},

	AD_TABLE_END_MARKER

};

} // End of namespace Bolt
