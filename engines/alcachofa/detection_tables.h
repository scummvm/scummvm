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

const AlcachofaGameDescription gameDescriptions[] = {
	//
	// A Movie Adventure
	//
	{
		{
			"aventuradecine",
			"Clever & Smart - A Movie Adventure",
			AD_ENTRY2s(
				"Textos/Objetos.nkr", "a2b1deff5ca7187f2ebf7f2ab20747e9", 17606,
				"Data/DATA02.BIN", "ab6d8867585fbc0f555f5b13d8d1bdf3", 55906308
			),
			Common::DE_DEU,
			Common::kPlatformWindows,
			ADGF_USEEXTRAASTITLE | ADGF_REMASTERED,
			GUIO2(GAMEOPTION_32BITS, GAMEOPTION_HIGH_QUALITY)
		},
		EngineVersion::V3_1
	},
	{
		{
			"aventuradecine",
			"Clever & Smart - A Movie Adventure",
			AD_ENTRY2s(
				"Textos/Objetos.nkr", "a2b1deff5ca7187f2ebf7f2ab20747e9", 17606,
				"Data/DATA02.BIN", "4693e52835bad0c6deab63b60ead81fb", 38273192
			),
			Common::DE_DEU,
			Common::kPlatformWindows,
			ADGF_USEEXTRAASTITLE | ADGF_REMASTERED | ADGF_PIRATED,
			GUIO2(GAMEOPTION_32BITS, GAMEOPTION_HIGH_QUALITY)
		},
		EngineVersion::V3_1
	},
	{
		{
			"aventuradecine",
			"Clever & Smart - A Movie Adventure",
			AD_ENTRY1s("Textos/Objetos.nkr", "8dce25494470209d4882bf12f1a5ea42", 19208),
			Common::DE_DEU,
			Common::kPlatformWindows,
			ADGF_USEEXTRAASTITLE | ADGF_REMASTERED | ADGF_DEMO,
			GUIO2(GAMEOPTION_32BITS, GAMEOPTION_HIGH_QUALITY)
		},
		EngineVersion::V3_1
	},


	// The "english" version is just the spanish version with english subtitles...
	{
		{
			"aventuradecine",
			"Mortadelo y Filemón: Una Aventura de Cine - Edición Especial",
			AD_ENTRY1s("Textos/Objetos.nkr", "ad3cb78ad7a51cfe63ee6f84768c7e66", 15895),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_USEEXTRAASTITLE | ADGF_REMASTERED,
			GUIO2(GAMEOPTION_32BITS, GAMEOPTION_HIGH_QUALITY)
		},
		EngineVersion::V3_1
	},
	// the spanish Steam variant
	{
		{
			"aventuradecine",
			"Mortadelo y Filemón: Una Aventura de Cine - Edición Especial",
			AD_ENTRY1s("Textos/Objetos.nkr", "93331e4cc8d2f8f8a0007bfb5140dff5", 16403),
			Common::ES_ESP,
			Common::kPlatformWindows,
			ADGF_USEEXTRAASTITLE | ADGF_REMASTERED,
			GUIO2(GAMEOPTION_32BITS, GAMEOPTION_HIGH_QUALITY)
		},
		EngineVersion::V3_1
	},
	// the spanish CD variant
	{
		{
			"aventuradecine",
			"Mortadelo y Filemón: Una Aventura de Cine - Edición Especial",
			AD_ENTRY1s("Textos/Objetos.nkr", "8a8b23c04fdc4ced8070a7bccd0177bb", 24467),
			Common::ES_ESP,
			Common::kPlatformWindows,
			ADGF_UNSTABLE | ADGF_USEEXTRAASTITLE | ADGF_REMASTERED | ADGF_CD,
			GUIO2(GAMEOPTION_32BITS, GAMEOPTION_HIGH_QUALITY)
		},
		EngineVersion::V3_0
	},

	{ AD_TABLE_END_MARKER, EngineVersion::V1_0 }
};

} // End of namespace Alcachofa
