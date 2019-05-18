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

/* TADS OS interface file type definitions
 *
 * Defines certain datatypes used in the TADS operating system interface
 */

#ifndef GLK_TADS_OS_GLK
#define GLK_TADS_OS_GLK

namespace Glk {
namespace TADS {

#define TADS_RUNTIME_VERSION "2.5.17"
#define OS_SYSTEM_NAME "ScummGlk"

#define OSPATHCHAR '/'
#define OSPATHALT ""
#define OSPATHURL "/"
#define OSPATHSEP ':'
#define OS_NEWLINE_SEQ "\n"

/* maximum width (in characters) of a line of text */
#define OS_MAXWIDTH  255

/* round a size to worst-case alignment boundary */
#define osrndsz(s) (((s)+3) & ~3)

/* round a pointer to worst-case alignment boundary */
#define osrndpt(p) ((uchar *)((((unsigned long)(p)) + 3) & ~3))

/* read unaligned portable unsigned 2-byte value, returning int */
#define osrp2(p) READ_LE_UINT16(p)

/* read unaligned portable signed 2-byte value, returning int */
#define osrp2s(p) READ_LE_INT16(p)

/* write int to unaligned portable 2-byte value */
#define oswp2(p, i) WRITE_LE_UINT16(p, i)
#define oswp2s(p, i) WRITE_LE_INT16(p, i)

/* read unaligned portable 4-byte value, returning unsigned long */
#define osrp4(p) READ_LE_UINT32(p)
#define osrp4s(p) READ_LE_INT32(p)

#define oswp4(p, l) WRITE_LE_UINT32(p, l)
#define oswp4s(p, l) WRITE_LE_INT32(p, l)

} // End of namespace TADS
} // End of namespace Glk

#endif
