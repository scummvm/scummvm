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

namespace MADS {

static const MADSGameDescription gameDescriptions[] = {
#if 0
	{
		// Rex Nebular and the Cosmic Gender Bender DOS English (compressed)
		// Removed for now, until the original floppy compression is supported
		{
			"nebular",
			0,
			{
				{"mpslabs.001", 0, "4df5c557b52abb5b661cf4befe5ae301", 1315354},
				AD_LISTEND
			},
			Common::EN_ANY,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO5(GUIO_NOSPEECH, GAMEOPTION_EASY_MOUSE, GAMEOPTION_ANIMATED_INVENTORY, GAMEOPTION_ANIMATED_INTERFACE, GAMEOPTION_NAUGHTY_MODE)
		},
		GType_RexNebular,
		0
	},
#endif

	{
		// Rex Nebular and the Cosmic Gender Bender DOS English
		{
			"nebular",
			0,
			{
				{"section1.hag", 0, "6f725eb38660de8af31ec7cdd628d615", 927222},
				AD_LISTEND
			},
			Common::EN_ANY,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO5(GUIO_NOSPEECH, GAMEOPTION_EASY_MOUSE, GAMEOPTION_ANIMATED_INVENTORY, GAMEOPTION_ANIMATED_INTERFACE, GAMEOPTION_NAUGHTY_MODE)
		},
		GType_RexNebular,
		0
	},

	{
		// Rex Nebular and the Cosmic Gender Bender DOS English 8.49 Alternate
		{
			"nebular",
			0,
			{
				{ "section1.hag", 0, "d583576923e3437937fb7f46f4b6274f", 927222 },
				AD_LISTEND
			},
			Common::EN_ANY,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO5(GUIO_NOSPEECH, GAMEOPTION_EASY_MOUSE, GAMEOPTION_ANIMATED_INVENTORY, GAMEOPTION_ANIMATED_INTERFACE, GAMEOPTION_NAUGHTY_MODE)
		},
		GType_RexNebular,
		0
	},

	{
		// Return of the Phantom DOS English
		{
			"phantom",
			0,
			{
				{"section1.hag", 0, "76e2d47a7aebafe48edc9884b3d91782", 1130939},
				AD_LISTEND
			},
			Common::EN_ANY,
			Common::kPlatformDOS,
			ADGF_UNSTABLE,
			GUIO1(GAMEOPTION_EASY_MOUSE)
		},
		GType_Phantom,
		0
	},

	{
		// Dragonsphere DOS English
		{
			"dragonsphere",
			0,
			{
				{"section1.hag", 0, "2770e441d296be5e806194693eebd95a", 2061199},
				AD_LISTEND
			},
			Common::EN_ANY,
			Common::kPlatformDOS,
			ADGF_UNSTABLE,
			GUIO1(GAMEOPTION_EASY_MOUSE)
		},
		GType_Dragonsphere,
		0
	},

	{ AD_TABLE_END_MARKER, 0, 0 }
};

} // End of namespace MADS
