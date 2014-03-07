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

#ifndef VOYEUR_STATICRES_H
#define VOYEUR_STATICRES_H

#include "common/scummsys.h"

namespace Voyeur {

extern const int COMPUTER_DEFAULTS[];

extern const int RESOLVE_TABLE[];

extern const int LEVEL_H[];

extern const int LEVEL_M[];

extern const int BLIND_TABLE[];

extern const int COMPUTER_SCREEN_TABLE[];

extern const char *const SZ_FILENAMES[];

extern const char *const SATURDAY;
extern const char *const SUNDAY;
extern const char *const MONDAY;
extern const char *const AM;
extern const char *const PM;

extern const char *const START_OF_MESSAGE;
extern const char *const END_OF_MESSAGE;

extern const char *const EVENT_TYPE_STRINGS[4];

extern int DOT_LINE_START[9];
extern int DOT_LINE_OFFSET[9];
extern int DOT_LINE_LENGTH[9];

extern const char *const PIRACY_MESSAGE[];

} // End of namespace Voyeur

#endif
