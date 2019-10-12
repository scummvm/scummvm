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

// Disable symbol overrides so that we can use system headers.
#define FORBIDDEN_SYMBOL_ALLOW_ALL

#include "util.h"
#include "create_cryomni3d_dat.h"

#include "versailles.h"

// In Versailles platform doesn't seem to change anything
#define DEFINE_FUNCS(lang) \
    size_t writeVersailles_ALL_ ## lang ## Header(FILE *f, uint32 offset, uint32 size) { \
        return writeGameHeader(f, VERSAILLES_GAMEID, VERSAILLES_VERSION, LANG_ ## lang, PLATFORM_ALL, \
                               offset, size); \
    } \
    \
    size_t writeVersailles_ALL_ ## lang ## Data(FILE *f) { \
        size_t size = 0; \
        \
        assert(VERSAILLES_LOCALIZED_FILENAMES_COUNT == ARRAYSIZE(versailles ## lang ## localizedFilenames)); \
        size += writeString16Array16(f, versailles ## lang ## localizedFilenames, \
                                     VERSAILLES_LOCALIZED_FILENAMES_COUNT); \
        \
        size += writeString16(f, versailles ## lang ## EpilMsg); \
        size += writeString16(f, versailles ## lang ## EpilPwd); \
        \
        size += writeString16(f, versailles ## lang ## BombPwd); \
        \
        assert(VERSAILLES_MESSAGES_COUNT == ARRAYSIZE(versailles ## lang ## messages)); \
        size += writeString16Array16(f, versailles ## lang ## messages, VERSAILLES_MESSAGES_COUNT); \
        \
        assert(VERSAILLES_PAINTINGS_COUNT == ARRAYSIZE(versailles ## lang ## paintings)); \
        size += writeString16Array16(f, versailles ## lang ## paintings, VERSAILLES_PAINTINGS_COUNT); \
        \
        size += writePadding(f); \
        return size; \
    }

DEFINE_FUNCS(FR)
DEFINE_FUNCS(BR)
DEFINE_FUNCS(DE)
DEFINE_FUNCS(EN)
DEFINE_FUNCS(ES)
DEFINE_FUNCS(IT)
