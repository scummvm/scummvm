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
 */

namespace Hopkins {

static const HopkinsGameDescription gameDescriptions[] = {
	{
		// Hopkins FBI Linux Demo 1.00
		{
			"hopkins",
			"Linux Demo v1.00",
			{
				{"Hopkins-PDemo.bin", 0, "88b4d6e14b9b1407083cb3d1213c0fa7", 272027},
				AD_LISTEND
			},
			Common::EN_ANY,
			Common::kPlatformLinux,
			ADGF_DEMO,
			GUIO1(GUIO_NONE)
		},
	},

	{
		// Hopkins FBI Linux Demo 1.02
		{
			"hopkins",
			"Linux Demo v1.02",
			{
				{"Hopkins-PDemo.bin", 0, "f82f4e698f3a189419351be0de2b2f8e", 273760},
				AD_LISTEND
			},
			Common::EN_ANY,
			Common::kPlatformLinux,
			ADGF_DEMO,
			GUIO1(GUIO_NONE)
		},
	},

	{
		// Hopkins FBI OS/2, provided by Strangerke
		{
			"hopkins",
			0,
			{
				{"Hopkins.exe", 0, "63d45f882278e5a9fa1027066223e5d9", 292864},
				AD_LISTEND
			},
			Common::EN_ANY,
			Common::kPlatformOS2,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NONE)
		},
	},
	{
		// Hopkins FBI Win95 Demo, provided by Strangerke
		{
			"hopkins",
			"Win95 Demo",
			{
				{"Hopkins.exe", 0, "0c9ebfe371f4dcf84a49f333f04839a0", 376897},
				AD_LISTEND
			},
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_DEMO,
			GUIO1(GUIO_NONE)
		},
	},
	{
		// Hopkins FBI Win95, provided by Strangerke
		{
			"hopkins",
			0,
			{
				{"Hopkins.exe", 0, "277a5c144bf9ec7d8450ae37afb85090", 419281},
				AD_LISTEND
			},
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NONE)
		},
	},
	{
		// Hopkins FBI Linux, provided by Strangerke
		{
			"hopkins",
			0,
			{
				{"Hopkins.bin", 0, "71611380cb31744bf909b8319a65e6e6", 275844},
				AD_LISTEND
			},
			Common::EN_ANY,
			Common::kPlatformLinux,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NONE)
		},
	},
	{ AD_TABLE_END_MARKER }
};

} // End of namespace Hopkins
