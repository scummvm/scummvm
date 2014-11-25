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

namespace Access {

static const AccessGameDescription gameDescriptions[] = {
	{
		// Amazon Guadians of Eden - Floppy English
		{
			"amazon",
			0,
			{
				{ "c00.ap", 0, "dcabf69d5a0d911168cb73511ebaead0", 331481 },
				AD_LISTEND
			},
			Common::EN_ANY,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NONE)
		},
		GType_Amazon,
		0
	},

	// Amazon Guardians of Eden - Demo English
	{
		{
		"amazon",
		"Demo",
			{
				{ "c25.ap", 0, "5baba0c052d22157499bfa05cb1ed5b7", 65458 },
				AD_LISTEND
			},
			Common::EN_ANY,
			Common::kPlatformDOS,
			ADGF_DEMO,
			GUIO1(GUIO_NONE)
		},
		GType_Amazon,
		0
	},

	{
		// Amazon: Guardians of Eden - CD English
		{
			"amazon",
			"CD",
			{
				{ "checksum.crc", 0, "bef85478132fec74cb5d9067f3a37d24", 8 },
				AD_LISTEND
			},
			Common::EN_ANY,
			Common::kPlatformDOS,
			ADGF_CD,
			GUIO1(GUIO_NONE)
		},
		GType_Amazon,
		0
	},

	{
		// Martian Memorandum
		{
			"martian",
			nullptr,
			{
				{ "r00.ap", 0, "af98db5ee7f9ef86c6b1f43187a3691b", 31 },
				AD_LISTEND
			},
			Common::EN_ANY,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NONE)
		},
		GType_MartianMemorandum,
		0
	},

	{ AD_TABLE_END_MARKER, 0, 0 }
};

} // End of namespace Access
