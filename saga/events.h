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

// Event management module header file

#ifndef SAGA_EVENT_H
#define SAGA_EVENT_H

namespace Saga {

#define R_EVENT_DBGLVL R_DEBUG_NONE
#define R_EVENT_WARNINGCOUNT 1000
#define R_EVENT_MASK 0x00FF

enum R_EVENT_STATUSCODE {
	R_EVENT_INVALIDCODE = 0,
	R_EVENT_DELETE,
	R_EVENT_CONTINUE,
	R_EVENT_BREAK
};

static int HandleContinuous(R_EVENT * event);
static int HandleOneShot(R_EVENT * event);
static int HandleInterval(R_EVENT * event);
static int ProcessEventTime(long msec);
static int InitializeEvent(R_EVENT * event);

} // End of namespace Saga

#endif
