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

namespace Macs2 {

const PlainGameDescriptor macs2Games[] = {
	{ "sis", "Schatz im Silbersee" },
	{ 0, 0 }
};

const ADGameDescription gameDescriptions[] = {
	{
		"sis",
		nullptr,
		// Original file
		// AD_ENTRY1s("RESOURCE.MCS", "b8646d5cc2e45442a414220b1a65ba73", 8621636),
		// Adjusted file
		AD_ENTRY1s("RESOURCE.MCS", "5a6cdeecdabae42872ab9278ab895bad", 8621636),
		Common::EN_ANY,
		Common::kPlatformDOS,
		// TODO: Unstable
		ADGF_NO_FLAGS,
		GUIO1(GUIO_NONE)
	},

	AD_TABLE_END_MARKER
};

} // End of namespace Macs2
