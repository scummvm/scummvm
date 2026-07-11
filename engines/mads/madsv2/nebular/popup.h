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

#ifndef MADS_NEBULAR_POPUP_H
#define MADS_NEBULAR_POPUP_H

#include "mads/madsv2/core/font.h"
#include "mads/madsv2/core/popup.h"

namespace MADS {
namespace MADSV2 {
namespace RexNebular {

//extern bool popup_vomitation_flag;

extern void popup_init();
//extern int popup_create(FontPtr font, int horiz_pieces, int x, int y);
//extern void popup_destroy();
extern void popup_draw();

} // namespace RexNebular
} // namespace MADSV2
} // namespace MADS

#endif
