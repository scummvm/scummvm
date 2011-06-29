/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL: https://svn.scummvm.org:4444/svn/dreamweb/detection_tables.h $
 * $Id: detection_tables.h 66 2010-11-07 08:31:21Z eriktorbjorn $
 *
 */

#ifndef DREAMWEB_DETECTION_TABLES_H
#define DREAMWEB_DETECTION_TABLES_H

namespace DreamWeb {

using Common::GUIO_NONE;

static const DreamWebGameDescription gameDescriptions[] = {
	{
		{
			"dreamweb",
			"",
			{
				{"dreamweb.r00", 0, "3b5c87717fc40cc5a5ae19c155662ee3", 152918},
				{"dreamweb.r02", 0, "28458718167a040d7e988cf7d2298eae", 210466},
				AD_LISTEND
			},
			Common::EN_ANY,
			Common::kPlatformPC,
			ADGF_UNSTABLE,
			GUIO_NONE
		},
	},

	// International CD release
	{
		{
			"dreamweb",
			"CD",
			{
				{"dreamweb.r00", 0, "3b5c87717fc40cc5a5ae19c155662ee3", 152918},
				{"dreamweb.r02", 0, "d6fe5e3590ec1eea42ff65c10b023e0f", 198681},
				AD_LISTEND
			},
			Common::EN_ANY,
			Common::kPlatformPC,
			ADGF_CD | ADGF_UNSTABLE,
			GUIO_NONE
		},
	},

	// US CD release
	{
		{
			"dreamweb",
			"CD",
			{
				{"dreamweb.r00", 0, "8acafd7f4418d08d0e16b65b8b10bc50", 152983},
				{"dreamweb.r02", 0, "c0c363715ddf14ab54f2379906a3aa01", 198707},
				AD_LISTEND
			},
			Common::EN_USA,
			Common::kPlatformPC,
			ADGF_CD,
			GUIO_NONE
		},
	},

	// German floppy release
	{
		{
			"dreamweb",
			"",
			{
				{"dreamweb.r00", 0, "9960dc3baddabc6ad2a6fd75292b149c", 155886},
				{"dreamweb.r02", 0, "48e1f42a53402f963ca2d1ed969f4084", 212823},
				AD_LISTEND
			},
			Common::DE_DEU,
			Common::kPlatformPC,
			ADGF_UNSTABLE,
			GUIO_NONE
		},
	},

	// German CD release
	{
		{
			"dreamweb",
			"CD",
			{
				{"dreamweb.r00", 0, "9960dc3baddabc6ad2a6fd75292b149c", 155886},
				{"dreamweb.r02", 0, "076ca7cd326cb2abfb2091c6cf46ae08", 201038},
				AD_LISTEND
			},
			Common::DE_DEU,
			Common::kPlatformPC,
			ADGF_CD | ADGF_UNSTABLE,
			GUIO_NONE
		},
	},

	// Spanish floppy release
	{
		{
			"dreamweb",
			"",
			{
				{"dreamweb.r00", 0, "2df07174321de39c4f17c9ff654b268a", 153608},
				{"dreamweb.r02", 0, "f97d435ad5da08fb1bcf6ea3dd6e0b9e", 199499},
				AD_LISTEND
			},
			Common::ES_ESP,
			Common::kPlatformPC,
			ADGF_UNSTABLE,
			GUIO_NONE
		},
	},

	// Italian floppy release
	{
		{
			"dreamweb",
			"",
			{
				{"dreamweb.r00", 0, "66dcab08354232f423c590156335f819", 155448},
				{"dreamweb.r02", 0, "87a026e9f80ed4f94169381f871ee305", 199676},
				AD_LISTEND
			},
			Common::IT_ITA,
			Common::kPlatformPC,
			ADGF_UNSTABLE,
			GUIO_NONE
		},
	},

	{ AD_TABLE_END_MARKER }
};

} // End of namespace DreamWeb

#endif
