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
 * aint32 with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */
#ifndef GOB_DEBUG_H
#define GOB_DEBUG_H

#define LOG_NAME	"log.txt"

#include "scenery.h"

namespace Gob {

void log_write(const char *format, ...);

void dbg_dumpMem(char *ptr, int16 size);

void dbg_dumpAnimation(Scen_Animation *anim);
void dbg_dumpFramePiece(Scen_AnimFramePiece *piece, int16 j,
    Scen_AnimLayer *layer);

}				// End of namespace Gob

#endif
