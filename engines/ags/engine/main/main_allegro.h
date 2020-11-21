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

#ifndef AGS_ENGINE_MAIN_MAINALLEGRO_H
#define AGS_ENGINE_MAIN_MAINALLEGRO_H

// Gets allegro_error as a const string.
// Please, use this getter to acquire error text, do not use allegro_error
// global variable directly.
const char *get_allegro_error();
// Sets allegro_error global variable and returns a resulting string.
// The maximal allowed text length is defined by ALLEGRO_ERROR_SIZE macro
// (usually 256). If the formatted message is larger than that it will be
// truncated. Null terminator is always guaranteed.
const char *set_allegro_error(const char *format, ...);

#endif
