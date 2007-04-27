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

#ifndef CRUISE_BACKGROUND_H
#define CRUISE_BACKGROUND_H

namespace Cruise {

struct backgroundTableStruct {
	char name[9];
	char extention[6];
};

typedef struct backgroundTableStruct backgroundTableStruct;

extern short int cvtPalette[0x20];
extern int CVTLoaded;
extern uint8 *backgroundPtrtable[8];
extern backgroundTableStruct backgroundTable[8];

int loadBackground(char *name, int idx);

} // End of namespace Cruise

#endif
