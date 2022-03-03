/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef CHEWY_NGSDEFS_H
#define CHEWY_NGSDEFS_H

#include "common/events.h"

namespace Chewy {

#define MAXSPRITE 620
#define NOSPEICHER 0
#define MAXFONTBREITE 32
#define MAXFONTHOEHE 32

#define CHECK 0x00
#define WRITE 0x04
#define READ 0x08

#define MCGA_SPEICHER 64000L
#define SETZEN 0
#define UND 1
#define ODER 2
#define XODER 3

#define COPY_PAGE _G(out)->screen2screen

#define END_POOL 32767

#define MAXDIRS 50
#define MAXFNAMEN 50
#define MOD15 10
#define TYPE_ANZ 27
#define TMFDATA 6
#define SCREENSAVE 0
#define SPRITESAVE 1

#define ALT 0x1000

#define MOUSE_LEFT 255
#define MOUSE_RIGHT 1
#define MOUSE_CENTER 254

#define SOUND 5
#define BASE 0x220
#define JA 1
#define NEIN 0
#define ON 1
#define OFF 0

} // namespace Chewy

#endif
