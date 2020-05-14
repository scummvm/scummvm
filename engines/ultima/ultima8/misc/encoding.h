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

#ifndef ULTIMA8_MISC_ENCODING_H
#define ULTIMA8_MISC_ENCODING_H

namespace Ultima {
namespace Ultima8 {

extern uint8 reverse_encoding[256];
extern uint16 encoding[256];

// first byte in shift_jis character stream encoded as LSbyte in sjis
// if first byte is >= 0x80 then second byte in character stream as MSB in sjis
// return value is index in U8's japanese font. 0 for missing codepoints

uint16 shiftjis_to_ultima8(uint16 sjis);
uint32 shiftjis_to_unicode(uint16 sjis);


} // End of namespace Ultima8
} // End of namespace Ultima

#endif
