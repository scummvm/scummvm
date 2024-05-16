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

namespace Alcachofa {

const PlainGameDescriptor alcachofaGames[] = {
	{ "mort_phil_adventura_de_cine", "Mort&Phil: A movie adventure" },
	{ 0, 0 }
};

const ADGameDescription gameDescriptions[] = {
	{
		"mort_phil_adventura_de_cine",
		nullptr,
		AD_ENTRY1s("Textos/Objetos.nkr", "a2b1deff5ca7187f2ebf7f2ab20747e9", 17606),
		Common::DE_DEU,
		Common::kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO1(GUIO_NONE)
	},

	AD_TABLE_END_MARKER
};

} // End of namespace Alcachofa
