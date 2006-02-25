/* ScummVM - Scumm Interpreter
 * Copyright (C) 2006 The ScummVM project
 *
 * cinE Engine is (C) 2004-2005 by CinE Team
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

#ifndef CINE_PAL_H_
#define CINE_PAL_H_

namespace Cine {

struct palEntryStruct {
	char name[10];
	uint8 pal1[16];
	uint8 pal2[16];
};

typedef struct palEntryStruct palEntryStruct;

void loadPal(const char *fileName);

extern uint16 tempPalette[256];
extern uint8 colorMode256;
extern uint8 palette256[256 * 3];

void loadRelatedPalette(const char *fileName);

} // End of namespace Cine

#endif
