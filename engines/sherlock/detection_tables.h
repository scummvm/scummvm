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

namespace Sherlock {

static const SherlockGameDescription gameDescriptions[] = {
	{
		// Case of the Serrated Scalpel - English 3.5" Floppy
		// The HitSquad CD version has the same MD5
		{
			"scalpel",
			0,
			AD_ENTRY1s("talk.lib", "ad0c4d6865edf15da4e9204c08815875", 238928),
			Common::EN_ANY,
			Common::kPlatformDOS,
			ADGF_UNSTABLE | ADGF_NO_FLAGS,
			GUIO2(GUIO_NOSPEECH, GAMEOPTION_ORIGINAL_SAVES)
		},
		GType_SerratedScalpel,
	},

	{
		// Case of the Serrated Scalpel - Interactive English Demo
		// Provided by Strangerke
		{
			"scalpel",
			"Interactive Demo",
			AD_ENTRY1s("talk.lib", "dbdc8a20c96900aa7e4d02f3fe8a274c", 121102),
			Common::EN_ANY,
			Common::kPlatformDOS,
			ADGF_UNSTABLE | ADGF_DEMO,
			GUIO1(GUIO_NOSPEECH)
		},
		GType_SerratedScalpel,
	},

	{
		// Case of the Serrated Scalpel - Non-Interactive English Demo
		// Provided by Strangerke
		{
			"scalpel",
			"Non Interactive Demo",
			AD_ENTRY1s("music.lib", "ec19a09b7fef6fd90b1ab812ce6e9739", 38563),
			Common::EN_ANY,
			Common::kPlatformDOS,
			ADGF_UNSTABLE | ADGF_DEMO,
			GUIO1(GUIO_NOSPEECH)
		},
		GType_SerratedScalpel,
	},

	{
		// Case of the Rose Tattoo - French CD
		// Provided by Strangerke
		{
			"rosetattoo",
			"CD",
			AD_ENTRY1s("talk.lib", "22e8e6406dd2fbbb238c9898928df42e", 770756),
			Common::EN_ANY,
			Common::kPlatformDOS,
			ADGF_UNSTABLE,
			GUIO0()
		}
	},

	{
		// Case of the Rose Tattoo - English CD
		// Provided by dreammaster
		{
			"rosetattoo",
			"CD",
			AD_ENTRY1s("talk.lib", "9639a756b0993ebd71cb5f4d8b78b2dc", 765134),
			Common::EN_ANY,
			Common::kPlatformDOS,
			ADGF_UNSTABLE,
			GUIO0()
		},
		GType_RoseTattoo,
	},

	{ AD_TABLE_END_MARKER, (GameType)0 }
};

} // End of namespace Sherlock
