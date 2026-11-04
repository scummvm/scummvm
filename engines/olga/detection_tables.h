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

namespace Olga {

const PlainGameDescriptor olgaGames[] = {
	{ "olga", "Olga's Arbejdsmilj\xC3\xB8" },
	{ 0, 0 }
};

const ADGameDescription gameDescriptions[] = {
	{
		"olga",
		nullptr,
		AD_ENTRY2s("OLGA.EXE", "26b09e9c10a8ecbbb2dcc146c1a02d56", 127488,
				   "OLGA.OVR", "539db21a7d6a6880c4448829266bf679", 27052),
		Common::DA_DNK,
		Common::kPlatformDOS,
		ADGF_UNSTABLE,
		GUIO2(GUIO_NOSPEECH, GUIO_NOMIDI)
	},

	AD_TABLE_END_MARKER
};

} // End of namespace Olga
