/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001-2006 The ScummVM project
 * Copyright (C) 2002-2006 Chris Apers - PalmOS Backend
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#include <PenInputMgr.h>
#include "pace.h"

GlobalsType global;


PACE_CLASS_WRAPPER(Err)
	StatShow_68k(void) {
	PACE_PIN_EXEC_NP(pinStatShow, Err)
}

PACE_CLASS_WRAPPER(Err)
	StatHide_68k(void) {
	PACE_PIN_EXEC_NP(pinStatHide, Err)
}

PACE_CLASS_WRAPPER(Err)
	PINSetInputAreaState_68k(UInt16 state) {
	PACE_PARAMS_INIT()
	PACE_PARAMS_ADD16(state)
	PACE_PARAMS_END()
	PACE_PIN_EXEC(pinPINSetInputAreaState, Err)
}
