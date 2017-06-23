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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

namespace Xeen {

static const XeenGameDescription gameDescriptions[] = {
	{
		// World of Xeen
		{
			"worldofxeen",
			nullptr,
			{
				{ "xeen.cc", 0, "0cffbab533d9afe140e69ec93096f43e", 13435646 },
				{ "dark.cc", 0, "df194483ecea6abc0511637d712ced7c", 11217676 },
				AD_LISTEND
			},
			Common::EN_ANY,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NONE)
		},
		GType_WorldOfXeen,
		0
	},

	{
		// World of Xeen (German)
		{
			"worldofxeen",
			nullptr,
			{
				{"xeen.cc", 0, "f4e4b3ddc43bd102dbe1637f480f1fa1", 13214150},
				{"dark.cc", 0, "84a86bbbc5f2fe96c0b0325485ed8203", 11173657},
				AD_LISTEND
			},
			Common::DE_DEU,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NONE),
		},
		GType_WorldOfXeen,
		0
	},

	{
		// World of Xeen (2 CD talkie version)
		{
			"worldofxeen",
			nullptr,
			{
				{"xeen.cc", 0, "964078c53f649937ce9a1a3596ce3d9f", 13438429},
				{"dark.cc", 0, "7f755ce39ea614fa6adb016f8bfc6e43", 11288403},
				AD_LISTEND
			},
			Common::EN_ANY,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NONE)
		},
		GType_WorldOfXeen,
		0
	},

	{ AD_TABLE_END_MARKER, 0, 0 }
};

} // End of namespace Xeen
