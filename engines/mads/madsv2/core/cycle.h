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

#ifndef MADS_CORE_CYCLE_H
#define MADS_CORE_CYCLE_H

#include "mads/madsv2/core/color.h"

namespace MADS {
namespace MADSV2 {

extern Palette cycling_palette;         /* Palette being used for cycling */

extern CycleList cycle_list;            /* List of cycles being performed */
extern int cycling_active;              /* Flag if cycling active         */
extern int cycling_delay;               /* Cycling delay countdown        */
extern int cycling_threshold;           /* Cycling reset threshold        */
extern int total_cycle_colors;          /* Total # of colors being cycled */

extern long cycle_timing[COLOR_MAX_CYCLES];     /* Timing for each cycle  */

void cycle_init(CycleListPtr new_cycle_list, int activate);
void cycle_colors(void);

} // namespace MADSV2
} // namespace MADS

#endif
