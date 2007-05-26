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

#ifndef CRUISE_VOLUME_H
#define CRUISE_VOLUME_H

namespace Cruise {

int16 readVolCnf(void);
int16 findFileInDisks(uint8 * fileName);
void freeDisk(void);
int16 findFileInList(uint8 * fileName);

////////////////

void strToUpper(uint8 * fileName);
void drawMsgString(uint8 * string);
void askDisk(int16 discNumber);
void setObjectPosition(int16 param1, int16 param2, int16 param3, int16 param4);

} // End of namespace Cruise

#endif
