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
	{ "aventuradecine", "Mort & Phil: A Movie Adventure" },
	{ 0, 0 }
};

const ADGameDescription gameDescriptions[] = {
	//
	// A Movie Adventure
	//
	{
		"aventuradecine",
		"Clever & Smart - A Movie Adventure",
		AD_ENTRY1s("Textos/Objetos.nkr", "a2b1deff5ca7187f2ebf7f2ab20747e9", 17606),
		Common::DE_DEU,
		Common::kPlatformWindows,
		ADGF_TESTING | ADGF_USEEXTRAASTITLE | ADGF_REMASTERED,
		GUIO2(GAMEOPTION_32BITS, GAMEOPTION_HIGH_QUALITY)
	},
	{
		"aventuradecine",
		"Clever & Smart - A Movie Adventure",
		AD_ENTRY1s("Textos/Objetos.nkr", "8dce25494470209d4882bf12f1a5ea42", 19208),
		Common::DE_DEU,
		Common::kPlatformWindows,
		ADGF_TESTING | ADGF_USEEXTRAASTITLE | ADGF_REMASTERED | ADGF_DEMO,
		GUIO2(GAMEOPTION_32BITS, GAMEOPTION_HIGH_QUALITY)
	},


	// The "english" version is just the spanish version with english subtitles...
	{
		"aventuradecine",
		"Mortadelo y Filemón: Una Aventura de Cine - Edición Especial",
		AD_ENTRY1s("Textos/Objetos.nkr", "ad3cb78ad7a51cfe63ee6f84768c7e66", 15895),
		Common::EN_ANY,
		Common::kPlatformWindows,
		ADGF_TESTING | ADGF_USEEXTRAASTITLE | ADGF_REMASTERED,
		GUIO2(GAMEOPTION_32BITS, GAMEOPTION_HIGH_QUALITY)
	},
	{
		"aventuradecine",
		"Mortadelo y Filemón: Una Aventura de Cine - Edición Especial",
		AD_ENTRY1s("Textos/Objetos.nkr", "93331e4cc8d2f8f8a0007bfb5140dff5", 16403),
		Common::ES_ESP,
		Common::kPlatformWindows,
		ADGF_TESTING | ADGF_USEEXTRAASTITLE | ADGF_REMASTERED,
		GUIO2(GAMEOPTION_32BITS, GAMEOPTION_HIGH_QUALITY)
	},

	AD_TABLE_END_MARKER
};

} // End of namespace Alcachofa
