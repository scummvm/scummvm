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

#include "mads/core/font.h"
#include "mads/core/popup.h"

namespace MADS {
namespace MADSV2 {
namespace RexNebular {

extern void popup_init();
extern void popup_draw();
extern void popup_setup_cycle();
extern void popup_update_ask(const char *string, int maxlen);

} // namespace RexNebular
} // namespace MADSV2
} // namespace MADS

#endif
