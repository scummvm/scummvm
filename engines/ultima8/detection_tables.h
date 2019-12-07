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

namespace Ultima8 {

static const Ultima8GameDescription gameDescriptions[] = {
	{
		{
			"ultima8",
			"Gold Edition",
			AD_ENTRY1s("u8.ini", "c09a7ddaf8cde46932d0cb33a6474b20", 80),
			Common::EN_ANY,
			Common::kPlatformDOS,
			0,
			GUIO1(GUIO_NOMIDI)
		},
	},

	{ AD_TABLE_END_MARKER }
};

} // End of namespace Hopkins
