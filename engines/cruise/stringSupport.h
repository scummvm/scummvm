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
 * $URL$
 * $Id$
 *
 */

#ifndef CRUISE_STRING_SUPPORT_H
#define CRSUIE_STRING_SUPPORT_H

namespace Cruise {

// TODO: Get rid of these and this file (either use strcpy directly, or switch to Common::String)

void strcpyuint8(void *dest, const void *source);
void strcatuint8(void *dest, const void *source);
uint8 strcmpuint8(const void *string1, const void *string2);

} // End of namespace Cruise

#endif
