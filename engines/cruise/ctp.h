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

#ifndef _CTP_H_
#define _CTP_H_

namespace Cruise {

struct ctpVar19SubStruct
{
  uint16 boxIdx;	//0
  uint16 type;		//2
  uint16 minX;		//4
  uint16 maxX;		//6
  uint16 minY;		//8
  uint16 maxY;		//A
};

typedef struct ctpVar19SubStruct ctpVar19SubStruct;

struct ctpVar19Struct
{
  struct ctpVar19Struct* field_0; //0
  ctpVar19SubStruct subStruct;
};

typedef struct ctpVar19Struct ctpVar19Struct;

extern ctpVar19Struct* ptr_something;
extern ctpVar19Struct* polyStruct;
extern ctpVar19Struct* ctpVar11;
extern ctpVar19Struct* ctpVar13;
extern ctpVar19Struct* ctpVar15;

extern uint8* ctpVar17;
extern ctpVar19Struct* ctpVar19;

int loadCtp(uint8* ctpName);
int ctpProc2(int varX, int varY, int paramX, int paramY);

} // End of namespace Cruise

#endif
