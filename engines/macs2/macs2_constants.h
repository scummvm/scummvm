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

#ifndef MACS2_CONSTANTS_H
#define MACS2_CONSTANTS_H

namespace Macs2 {

// Original game viewport dimensions (all scene maps and buffers use these).
static constexpr int kScreenWidth     = 320;
static constexpr int kScreenWidthLast = kScreenWidth - 1;
static constexpr int kGameHeight      = 200;
static constexpr int kGameHeightLast  = kGameHeight - 1;

// SCUMM-style verb/inventory strip (kEnhUIUX enhancement only).
static constexpr int kUIHeight       = 64;
static constexpr int kScreenHeight   = kGameHeight + kUIHeight;
static constexpr int kScreenHeightLast = kScreenHeight - 1;

} // namespace Macs2

#endif // MACS2_CONSTANTS_H
