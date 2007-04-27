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

#include "cruise/cruise_main.h"

namespace Cruise {

void strcpyuint8(void* dest, void* source)
{
  strcpy((char*)dest,(char*)source);
}

void strcatuint8(void* dest, void* source)
{
  strcat((char*)dest,(char*)source);
}

uint8 strcmpuint8(void* string1, void* string2)
{
  return strcmp((char*)string1,(char*)string2);
}

FILE* fopenuint8(void* name, void* param)
{
  return fopen((char*)name,(char*)param);
}

} // End of namespace Cruise
