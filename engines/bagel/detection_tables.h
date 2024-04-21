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

namespace Bagel {

const PlainGameDescriptor bagelGames[] = {
	{ "bagel", "Bagel" },
	{ "spacebar", "The Space Bar" },
	{ 0, 0 }
};

const ADGameDescription gameDescriptions[] = {
	{
		"spacebar",
		nullptr,
		AD_ENTRY1s("wld/bar.wld", "34b52995cb7e6c68bac22c14cb6813da", 812794),
		Common::EN_ANY,
		Common::kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO1(GUIO_NONE)
	},

	{
		"spacebar",
		"Demo",
		AD_ENTRY1s("wld/bar.wld", "4d6a0926484e1660035af0ed4004a47f", 27495),
		Common::EN_ANY,
		Common::kPlatformWindows,
		ADGF_UNSTABLE | ADGF_DEMO,
		GUIO1(GUIO_NONE)
	},

	AD_TABLE_END_MARKER
};

} // End of namespace Bagel
