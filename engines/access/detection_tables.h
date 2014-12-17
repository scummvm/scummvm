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

namespace Access {

static const AccessGameDescription gameDescriptions[] = {
	{
		// Amazon Guardians of Eden - Floppy English
		// 3.5" and 5.25" floppies provided by Strangerke had the same md5
		// Except the sound file. The executable is also identical
		{
			"amazon",
			0,
			AD_ENTRY1s("c00.ap", "dcabf69d5a0d911168cb73511ebaead0", 331481),
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
			AD_ENTRY1s("c25.ap", "5baba0c052d22157499bfa05cb1ed5b7", 65458),
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
			AD_ENTRY1s("checksum.crc", "bef85478132fec74cb5d9067f3a37d24", 8),
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
			AD_ENTRY1s("r00.ap", "af98db5ee7f9ef86c6b1f43187a3691b", 31),
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
