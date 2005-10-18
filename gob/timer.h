/* ScummVM - Scumm Interpreter
 * Copyright (C) 2004 Ivan Dubrov
 * Copyright (C) 2004-2005 The ScummVM project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $Header$
 *
 */
#ifndef GOB_TIMER_H
#define GOB_TIMER_H

namespace Gob {

typedef void (* TickHandler) (void);

void timer_enableTimer(void);
void timer_disableTimer(void);
void timer_setHandler(void);
void timer_restoreHandler(void);
void timer_addTicks(int16 ticks);
void timer_setTickHandler(TickHandler handler);
int32 timer_getTicks(void);

}				// End of namespace Gob

#endif
