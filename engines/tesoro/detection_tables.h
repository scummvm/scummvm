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

namespace Tesoro {

const PlainGameDescriptor tesoroGames[] = {
	{ "tesoro", "El Tesoro de Isla Alcachofa" },
	{ 0, 0 }
};

const ADGameDescription gameDescriptions[] = {
	{
		"tesoro",
		nullptr,
		AD_ENTRY2s("TESORO.EXE", "fa6d43cd4b120a8ec60cd0f5f4052dd1", 200192,
		           "TESORO.DAT", "8fb27fd8a3b31915053372e9d9f7dded", 263612966),
		Common::ES_ESP,
		Common::kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO1(GUIO_NONE)
	},

	AD_TABLE_END_MARKER
};

} // End of namespace Tesoro
