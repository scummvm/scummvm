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

#ifndef DREAMWEB_DETECTION_TABLES_H
#define DREAMWEB_DETECTION_TABLES_H

#include "dreamweb/detection.h"

namespace DreamWeb {

#define GAMEOPTION_ORIGINAL_SAVELOAD GUIO_GAMEOPTIONS1
#define GAMEOPTION_BRIGHTPALETTE     GUIO_GAMEOPTIONS2


static const DreamWebGameDescription gameDescriptions[] = {
	// International floppy release
	{
		{
			"dreamweb",
			"",
			{
				{"dreamweb.r00", 0, "3b5c87717fc40cc5a5ae19c155662ee3", 152918},
				{"dreamweb.r02", 0, "28458718167a040d7e988cf7d2298eae", 210466},
				{"dreamweb.exe", 0, "56b1d73aa56e964b45872ff552402341", 64985},
				AD_LISTEND
			},
			Common::EN_ANY,
			Common::kPlatformDOS,
			0,
			GUIO2(GAMEOPTION_ORIGINAL_SAVELOAD, GAMEOPTION_BRIGHTPALETTE)
		},
	},

	// International CD release
	{
		{
			"dreamweb",
			"CD",
			AD_ENTRY2s("dreamweb.r00", "3b5c87717fc40cc5a5ae19c155662ee3", 152918,
					   "dreamweb.r02", "d6fe5e3590ec1eea42ff65c10b023e0f", 198681),
			Common::EN_ANY,
			Common::kPlatformDOS,
			ADGF_CD,
			GUIO2(GAMEOPTION_ORIGINAL_SAVELOAD, GAMEOPTION_BRIGHTPALETTE)
		},
	},

	// UK-V (Early UK) CD Release - From bug #6035
	// Note: r00 and r02 files are identical to international floppy release
	//       so was misidentified as floppy, resulting in disabled CD speech.
	//       Added executable to detection to avoid this.
	{
		{
			"dreamweb",
			"CD",
			{
				{"dreamweb.r00", 0, "3b5c87717fc40cc5a5ae19c155662ee3", 152918},
				{"dreamweb.r02", 0, "28458718167a040d7e988cf7d2298eae", 210466},
				{"dreamweb.exe", 0, "dd1c7793b151489e67b83cd1ecab51cd", -1},
				AD_LISTEND
			},
			Common::EN_GRB,
			Common::kPlatformDOS,
			ADGF_CD,
			GUIO2(GAMEOPTION_ORIGINAL_SAVELOAD, GAMEOPTION_BRIGHTPALETTE)
		},
	},

	// US CD release
	{
		{
			"dreamweb",
			"CD",
			AD_ENTRY2s("dreamweb.r00", "8acafd7f4418d08d0e16b65b8b10bc50", 152983,
					   "dreamweb.r02", "c0c363715ddf14ab54f2379906a3aa01", 198707),
			Common::EN_USA,
			Common::kPlatformDOS,
			ADGF_CD,
			GUIO2(GAMEOPTION_ORIGINAL_SAVELOAD, GAMEOPTION_BRIGHTPALETTE)
		},
	},

	// French CD release
	{
		{
			"dreamweb",
			"CD",
			AD_ENTRY2s("dreamwfr.r00", "e354582a8564faf5c515df92f207e8d1", 154657,
					   "dreamwfr.r02", "57f3f08d5aefd04184eac76927eced80", 200575),
			Common::FR_FRA,
			Common::kPlatformDOS,
			ADGF_CD,
			GUIO2(GAMEOPTION_ORIGINAL_SAVELOAD, GAMEOPTION_BRIGHTPALETTE)
		},
	},

	// French CD release
	// From bug #6030
	{
		{
			"dreamweb",
			"CD",
			AD_ENTRY2s("dreamwfr.r00", "e354582a8564faf5c515df92f207e8d1", 154657,
					   "dreamwfr.r02", "cb99f08d5aefd04184eac76927eced80", 200575),
			Common::FR_FRA,
			Common::kPlatformDOS,
			ADGF_CD,
			GUIO2(GAMEOPTION_ORIGINAL_SAVELOAD, GAMEOPTION_BRIGHTPALETTE)
		},
	},

	// German floppy release
	{
		{
			"dreamweb",
			"",
			AD_ENTRY2s("dreamweb.r00", "9960dc3baddabc6ad2a6fd75292b149c", 155886,
					   "dreamweb.r02", "48e1f42a53402f963ca2d1ed969f4084", 212823),
			Common::DE_DEU,
			Common::kPlatformDOS,
			0,
			GUIO2(GAMEOPTION_ORIGINAL_SAVELOAD, GAMEOPTION_BRIGHTPALETTE)
		},
	},

	// German CD release
	{
		{
			"dreamweb",
			"CD",
			AD_ENTRY2s("dreamweb.r00", "9960dc3baddabc6ad2a6fd75292b149c", 155886,
					   "dreamweb.r02", "076ca7cd326cb2abfb2091c6cf46ae08", 201038),
			Common::DE_DEU,
			Common::kPlatformDOS,
			ADGF_CD,
			GUIO2(GAMEOPTION_ORIGINAL_SAVELOAD, GAMEOPTION_BRIGHTPALETTE)
		},
	},

	// Spanish floppy release
	{
		{
			"dreamweb",
			"",
			AD_ENTRY2s("dreamweb.r00", "2df07174321de39c4f17c9ff654b268a", 153608,
					   "dreamweb.r02", "f97d435ad5da08fb1bcf6ea3dd6e0b9e", 199499),
			Common::ES_ESP,
			Common::kPlatformDOS,
			0,
			GUIO2(GAMEOPTION_ORIGINAL_SAVELOAD, GAMEOPTION_BRIGHTPALETTE)
		},
	},

	// Spanish CD release
	{
		{
			"dreamweb",
			"CD",
			AD_ENTRY2s("dreamwsp.r00", "2df07174321de39c4f17c9ff654b268a", 153608,
					   "dreamwsp.r02", "577d435ad5da08fb1bcf6ea3dd6e0b9e", 199499),
			Common::ES_ESP,
			Common::kPlatformDOS,
			ADGF_CD,
			GUIO2(GAMEOPTION_ORIGINAL_SAVELOAD, GAMEOPTION_BRIGHTPALETTE)
		},
	},

	// Spanish CD release
	// From bug #6030
	{
		{
			"dreamweb",
			"CD",
			AD_ENTRY2s("dreamwsp.r00", "2df07174321de39c4f17c9ff654b268a", 153608,
					   "dreamwsp.r02", "f97d435ad5da08fb1bcf6ea3dd6e0b9e", 199499),
			Common::ES_ESP,
			Common::kPlatformDOS,
			ADGF_CD,
			GUIO2(GAMEOPTION_ORIGINAL_SAVELOAD, GAMEOPTION_BRIGHTPALETTE)
		},
	},

	// Italian floppy release
	{
		{
			"dreamweb",
			"",
			AD_ENTRY2s("dreamweb.r00", "66dcab08354232f423c590156335f819", 155448,
					   "dreamweb.r02", "87a026e9f80ed4f94169381f871ee305", 199676),
			Common::IT_ITA,
			Common::kPlatformDOS,
			0,
			GUIO2(GAMEOPTION_ORIGINAL_SAVELOAD, GAMEOPTION_BRIGHTPALETTE)
		},
	},

	// Italian CD release
	{
		{
			"dreamweb",
			"CD",
			{
				{"dreamweb.exe", 0, "44d1708535cdb863b9cca372ad0b05dd", 65370},
				{"dreamweb.r00", 0, "66dcab08354232f423c590156335f819", 155448},
				{"dreamweb.r02", 0, "87a026e9f80ed4f94169381f871ee305", 199676},
				AD_LISTEND
			},
			Common::IT_ITA,
			Common::kPlatformDOS,
			ADGF_CD,
			GUIO2(GAMEOPTION_ORIGINAL_SAVELOAD, GAMEOPTION_BRIGHTPALETTE)
		},
	},

	// Czech fan-made translation
	// From bug #7078
	{
		{
			"dreamweb",
			"CD",
			{
				{"dreamweb.r00", 0, "3b5c87717fc40cc5a5ae19c155662ee3", 152918},
				{"dreamweb.r02", 0, "28458718167a040d7e988cf7d2298eae", 210466},
				{"dreamweb.exe", 0, "40cc15bdc8fa3a785b5fd1ecd6194119", 65440},
				AD_LISTEND
			},
			Common::CS_CZE,
			Common::kPlatformDOS,
			ADGF_CD,
			GUIO2(GAMEOPTION_ORIGINAL_SAVELOAD, GAMEOPTION_BRIGHTPALETTE)
		},
	},

	// Russian fan-made translation
	{
		{
			"dreamweb",
			"CD",
			{
				{"dreamweb.r00", 0, "b457b515f1042d345c07e4e58a7ef792", 151975},
				{"dreamweb.r02", 0, "eebf681cef5a06ee12a2630512c5eb83", 197091},
				{"dreamweb.exe", 0, "9386c192d3bdce2ef4de2135c29fa66d", 65370},
				AD_LISTEND
			},
			Common::RU_RUS,
			Common::kPlatformDOS,
			ADGF_CD,
			GUIO2(GAMEOPTION_ORIGINAL_SAVELOAD, GAMEOPTION_BRIGHTPALETTE)
		},
	},

	// English floppy demo
	{
		{
			"dreamweb",
			"Demo",
			{
				{"dreamweb.r57", 0, "dba78ab266054ad98151db0a9aa845f6", 102904},
				{"dreamweb.r59", 0, "b1635ce312c7273b343eb0c2946361af", 199262},
				{"dreamweb.exe", 0, "2362f28683ffe4ae4986c21226e132c9", 63656},
				AD_LISTEND
			},
			Common::EN_ANY,
			Common::kPlatformDOS,
			ADGF_DEMO,
			GUIO2(GAMEOPTION_ORIGINAL_SAVELOAD, GAMEOPTION_BRIGHTPALETTE)
		},
	},

	// English CD demo
	{
		{
			"dreamweb",
			"CD Demo",
			{
				{"dreamweb.r60", 0, "45fb1438d165da9f098852cc8e14ad92", 108933},
				{"dreamweb.r22", 0, "35537525b55837b91b56f2468500ea43", 190222},
				{"dreamweb.exe", 0, "34218e66ffd0e0d65d71282e57ac4fcc", 64445},
				AD_LISTEND
			},
			Common::EN_ANY,
			Common::kPlatformDOS,
			ADGF_CD|ADGF_DEMO,
			GUIO2(GAMEOPTION_ORIGINAL_SAVELOAD, GAMEOPTION_BRIGHTPALETTE)
		},
	},

	// English Amiga demo
	{
		{
			"dreamweb",
			"Demo",
			{
				{"dreamweb.r57", 0, "f52b88b8417c7bddc8c63b684c6ad1dd", 117720},
				{"dreamweb.r59", 0, "996dfe6d963c27a302952c77a297b0fa", 223764},
				{"dreamweb",	 0,	"093103deb9712340ee34c4bed94aab12", 51496},
				AD_LISTEND
			},
			Common::EN_ANY,
			Common::kPlatformAmiga,
			ADGF_DEMO,
			GUIO2(GAMEOPTION_ORIGINAL_SAVELOAD, GAMEOPTION_BRIGHTPALETTE)
		},
	},

	{ AD_TABLE_END_MARKER }
};

} // End of namespace DreamWeb

#endif
