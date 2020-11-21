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

#include "ac/global_walkbehind.h"
#include "ac/common.h"
#include "ac/common_defines.h"
#include "ac/draw.h"
#include "ac/roomstatus.h"
#include "ac/walkbehind.h"
#include "debug/debug_log.h"

extern RoomStatus*croom;
extern int walk_behind_baselines_changed;

void SetWalkBehindBase(int wa,int bl) {
  if ((wa < 1) || (wa >= MAX_WALK_BEHINDS))
    quit("!SetWalkBehindBase: invalid walk-behind area specified");

  if (bl != croom->walkbehind_base[wa]) {
    walk_behind_baselines_changed = 1;
    invalidate_cached_walkbehinds();
    croom->walkbehind_base[wa] = bl;
    debug_script_log("Walk-behind %d baseline changed to %d", wa, bl);
  }
}
