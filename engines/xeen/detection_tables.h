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
		GType_World,
		0
	},

	{ AD_TABLE_END_MARKER, 0, 0 }
};

} // End of namespace Xeen
