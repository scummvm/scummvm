/* ScummVM - Scumm Interpreter
 * Copyright (C) 2004 The ScummVM project
 *
 * The ReInherit Engine is (C)2000-2003 by Daniel Balsom.
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */
#ifndef SAGA_SYSTIMER_H__
#define SAGA_SYSTIMER_H__

namespace Saga {

typedef void (*R_SYSTIMER_CALLBACK) (unsigned long, void *);

struct R_SYSTIMER;

int SYSTIMER_InitMSCounter(void);
unsigned long SYSTIMER_ReadMSCounter(void);

int SYSTIMER_ResetMSCounter(void);
int SYSTIMER_Sleep(uint16 msec);
int SYSTIMER_CreateTimer(R_SYSTIMER **,
    unsigned long, void *, R_SYSTIMER_CALLBACK);
int SYSTIMER_DestroyTimer(R_SYSTIMER *);


} // End of namespace Saga

#endif				/* SAGA_SYSTIMER_H__ */
