/* ScummVM - Scumm Interpreter
 * Copyright (C) 2004-2005 The ScummVM project
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

// Game detection, general game parameters

#ifndef SAGA_GAME_H_
#define SAGA_GAME_H_

#include "saga/saga.h"

namespace Saga {

#define GAME_PATH_LIMIT 512

// Script lookup table entry sizes for game verification
#define SCR_LUT_ENTRYLEN_ITECD 22
#define SCR_LUT_ENTRYLEN_ITEDISK 16


DetectedGameList GAME_ProbeGame(const FSList &fslist);

} // End of namespace Saga

#endif
