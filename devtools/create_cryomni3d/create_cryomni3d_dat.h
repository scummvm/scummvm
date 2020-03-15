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

#ifndef CREATE_CRYOMNI3D_DAT_H
#define CREATE_CRYOMNI3D_DAT_H

#ifndef MKTAG16
#define MKTAG16(a0,a1) ((uint16)((a1) | ((a0) << 8)))
#endif

size_t writeFileHeader(FILE *f, uint16 games = 0xdead);
size_t writeGameHeader(FILE *f, uint32 gameId, uint16 version, uint16 lang, uint32 platforms,
                       uint32 offset = 0xdeadfeed, uint32 size = 0xdeadfeed);

#define PLATFORM_WIN                   0x1
#define PLATFORM_DOS                   0x2
#define PLATFORM_MAC                   0x4
#define PLATFORM_PLAYSTATION           0x8
#define PLATFORM_SATURN               0x10
#define PLATFORM_ALL            0xffffffff

#define LANG_BR                       MKTAG16('b', 'r')
#define LANG_DE                       MKTAG16('d', 'e')
#define LANG_EN                       MKTAG16('e', 'n')
#define LANG_ES                       MKTAG16('e', 's')
#define LANG_FR                       MKTAG16('f', 'r')
#define LANG_IT                       MKTAG16('i', 't')
#define LANG_JA                       MKTAG16('j', 'a')
#define LANG_KO                       MKTAG16('k', 'o')
#define LANG_ZT                       MKTAG16('z', 't') // ZH_TWN

#endif
