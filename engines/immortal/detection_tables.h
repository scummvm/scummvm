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

namespace Immortal {

const PlainGameDescriptor immortalGames[] = {
	{ "immortal", "The Immortal" },
	{ 0, 0 }
};

const ADGameDescription gameDescriptions[] = {
	{
		"immortal",
		nullptr,
		AD_ENTRY1s("IMMORTAL.dsk", "094941fd71e6d2cdaa96c9664d32329e", 819200),
		Common::EN_ANY,
		Common::kPlatformApple2GS,
		ADGF_UNSTABLE,
		GUIO1(GUIO_NONE)
	},

	AD_TABLE_END_MARKER
};

} // namespace Immortal
