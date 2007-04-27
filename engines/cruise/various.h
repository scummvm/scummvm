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

#ifndef _VARIOUS_H_
#define _VARIOUS_H_

namespace Cruise {

extern uint16 var0;
extern uint16 fadeVar;
extern uint16 main15;

int16 readB16(void* ptr);

void createTextObject(int overlayIdx, int oldVar8, objectStruct *pObject, int scriptNumber, int scriptOverlayNumber, int backgroundPlane, int16 color, int oldVar2, int oldVar4, int oldVar6);
void removeObjectFromList(int ovlNumber, int objectIdx, objectStruct* objPtr, int backgroundPlane, int arg);
objectStruct* addObject(int16 overlayIdx,int16 param2,objectStruct* pHead,int16 scriptType,int16 scriptNumber,int16 scriptOverlay, int16 param3, int16 param4);
int16 Op_7Sub(int ovlIdx,int param1,int param2);

} // End of namespace Cruise

#endif
