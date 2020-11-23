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

#ifndef AGS_LIB_ALLEGRO_UNICODE_H
#define AGS_LIB_ALLEGRO_UNICODE_H

#include "ags/lib/allegro/base.h"

namespace AGS3 {

#define U_ASCII         AL_ID('A','S','C','8')
#define U_ASCII_CP      AL_ID('A','S','C','P')
#define U_UNICODE       AL_ID('U','N','I','C')
#define U_UTF8          AL_ID('U','T','F','8')
#define U_CURRENT       AL_ID('c','u','r','.')

extern void set_uformat(int format);
extern size_t ustrsize(const char *s);

} // namespace AGS3

#endif
