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
	{ "secta", "Mortadelo y Filemón: La Sexta Secta" },
	{ "terror", "Mortadelo y Filemón: Terror, Espanto y Pavor" },
	{ "vaqueros", "Mortadelo y Filemón: Dos vaqueros chapuceros" },
	{ 0, 0 }
};

const AlcachofaGameDescription gameDescriptions[] = {
	//
	// A Movie Adventure - Edicion Especial
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
			ADGF_USEEXTRAASTITLE | ADGF_REMASTERED | ADGF_CD,
			GUIO2(GAMEOPTION_32BITS, GAMEOPTION_HIGH_QUALITY)
		},
		EngineVersion::V3_0
	},
	// the russian CD variant
	{
		{
			"aventuradecine",
			"Секретные агенты: Киномонстры атакуют",
			AD_ENTRY1s("Textos/Objetos.nkr", "d0e5c0843e6d9e4dd81654fef3a0c9bf", 16604),
			Common::RU_RUS,
			Common::kPlatformWindows,
			ADGF_USEEXTRAASTITLE | ADGF_REMASTERED | ADGF_CD,
			GUIO2(GAMEOPTION_32BITS, GAMEOPTION_HIGH_QUALITY)
		},
		EngineVersion::V3_1
	},

	//
	// La Sexta Secta
	//
	{
		{
			"secta",
			"Mortadelo y Filemón: La Sexta Secta",
			AD_ENTRY1s("Fondos/MUSEO_O.ANI", "40a880c866aabbb5c09899d9b7ca66b6", 10630),
			Common::ES_ESP,
			Common::kPlatformWindows,
			ADGF_UNSTABLE | ADGF_USEEXTRAASTITLE,
			GUIO0()
		},
		EngineVersion::V2_0
	},

	//
	// A Movie Adventure - Edicion Original
	//
	{
		// Disk files copied into disk1/disk2
		{
			"aventuradecine",
			"Mortadelo y Filemón: Una Aventura de Cine - Edición Original",
			AD_ENTRY2s(
				"disk1/Install/oeste.emc", "b4c1084557d4cfbae336f0e741ec9e9f", 183099320,
				"disk2/Install/terror.emc", "dc9357ee618bff160e2e2afa168ba913", 170113868
			),
			Common::ES_ESP,
			Common::kPlatformWindows,
			ADGF_UNSTABLE | ADGF_USEEXTRAASTITLE | ADGF_CD,
			GUIO1(GAMEOPTION_TEXTURE_FILTER)
		},
		EngineVersion::V1_0
	},

	//
	// Terror, Espanto y Pavor
	//
	{
		// Install folder from the disk/the ISO within the Steam distribution
		{
			"terror",
			"Mortadelo y Filemón: Terror, Espanto y Pavor",
			AD_ENTRY1s("terror.emc", "dc9357ee618bff160e2e2afa168ba913", 170113868),
			Common::ES_ESP,
			Common::kPlatformWindows,
			ADGF_UNSTABLE | ADGF_USEEXTRAASTITLE | ADGF_CD,
			GUIO1(GAMEOPTION_TEXTURE_FILTER)
		},
		EngineVersion::V1_0
	},

	//
	// Dos vaqueros chapuceros
	//
	{
		// Install folder from the disk/the ISO within the Steam distribution
		{
			"vaqueros",
			"Mortadelo y Filemón: Dos vaqueros chapuceros",
			AD_ENTRY1s("oeste.emc", "b4c1084557d4cfbae336f0e741ec9e9f", 183099320),
			Common::ES_ESP,
			Common::kPlatformWindows,
			ADGF_UNSTABLE | ADGF_USEEXTRAASTITLE | ADGF_CD,
			GUIO1(GAMEOPTION_TEXTURE_FILTER)
		},
		EngineVersion::V1_0
	},

	{ AD_TABLE_END_MARKER, EngineVersion::V1_0 }
};

} // End of namespace Alcachofa
