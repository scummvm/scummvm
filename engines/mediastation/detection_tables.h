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

namespace MediaStation {

const PlainGameDescriptor mediastationGames[] = {
	// Sample Title
	{ "mediastation", "Media Station Game" },

	// Commercially released games

	// Internal betas/unreleased games
	{ 0, 0 }
};

const ADGameDescription gameDescriptions[] = {
	// For testing purposes, any folder with a "MediaStation" file in it can be run.
	{
		"mediastation",
		nullptr,
		AD_ENTRY1s("MediaStation", 0, AD_NO_SIZE),
		Common::EN_ANY,
		Common::kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO1(GUIO_NOASPECT)
	},

	AD_TABLE_END_MARKER
};

} // End of namespace MediaStation
