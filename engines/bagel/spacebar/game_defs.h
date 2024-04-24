
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

#ifndef BAGEL_SPACEBAR_GAME_DEFS_H
#define BAGEL_SPACEBAR_GAME_DEFS_H

namespace Bagel {
namespace SpaceBar {

#define GAMEWINDOW          CMainWindow

#define TOKEN_GAME_SDEV     "<default>"

#define TOKEN_SDEV_1        "INV"
#define CLASS_SDEV_1        CBagInv
#define TOKEN_SDEV_2        "PDA"
#define CLASS_SDEV_2        CBagPDA
#define TOKEN_SDEV_3        "STASH"
#define CLASS_SDEV_3        CBagStash
#define TOKEN_SDEV_4        "MAP"
#define CLASS_SDEV_4        CBagMap

} // namespace SpaceBar
} // namespace Bagel

#endif
