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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#include "asylum/resources/actionarea.h"

namespace Asylum {

Common::String ActionArea::toString() {
	Common::String output;

	output += Common::String::format("Action %d: %s\n", id, name);
	output += Common::String::format("           flags=%d      scriptIndex=%d      scriptIndex2=%d   type=%d\n", flags, scriptIndex, actionType, polyIdx);
	output += Common::String::format("           sound=%d      polygon=%d          palette=%d        volume=%d\n", soundResourceId, scriptIndex2, paletteValue, volume);
	output += Common::String::format("           field01=%d    field02=%d          field40=%d        field44=%d\n", field01, field02, field_40, field_44);
	output += Common::String::format("           field7C=%d    field84=%d          field88=%d        field90=%d\n", field_7C, field_84, field_88, field_90);

	return output;
}

} // end of namespace Asylum
