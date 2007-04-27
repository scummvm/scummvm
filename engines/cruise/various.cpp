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

uint16 var0 = 0;
uint16 fadeVar;
uint16 main15;

int16 readB16(void* ptr)
{
	int16 temp;

	temp = *(int16*)ptr;
	flipShort(&temp);

	return temp;
}

void freeObject(objectStruct* objPtr)
{
  if(objPtr)
  {
  /*  if(objPtr->next)
      free(objPtr->next); */

    //free(objPtr);
  }
}

void removeObjectFromList(int ovlNumber, int objectIdx, objectStruct* objPtr, int backgroundPlane, int arg)
{
  objectStruct* currentObj = objPtr->next;
  objectStruct* previous;

  while(currentObj)
  {
    objectStruct* si;

    si = currentObj;

    if( (si->overlay == ovlNumber || ovlNumber == -1) &&
        (si->idx == objectIdx || objectIdx == -1) &&
        (si->type == arg || arg == -1) &&
        (si->backgroundPlane == backgroundPlane || backgroundPlane == -1) )
    {
      si->type = -1;
    }

    currentObj = si->next;
  }

  previous = objPtr;
  currentObj = objPtr->next;

  while(currentObj)
  {
    objectStruct* si;

    si = currentObj;

    if(si->type == -1)
    {
      objectStruct* dx;
      previous->next = si->next;

      dx = si->next;

      if(!si->next)
      {
        dx = objPtr;
      }

      dx->prev = si->prev;

      freeObject(si);

      free(si);

      currentObj = dx;
    }
    else
    {
      currentObj = si->next;
      previous = si;
    }
  }
}

char* getText(int textIndex, int overlayIndex)
{
  if(!overlayTable[overlayIndex].ovlData)
  {
    return NULL;
  }

  if(!overlayTable[overlayIndex].ovlData->stringTable)
  {
    return NULL;
  }

  return overlayTable[overlayIndex].ovlData->stringTable[textIndex].string;
}

void createTextObject(int overlayIdx, int oldVar8, objectStruct *pObject, int scriptNumber, int scriptOverlayNumber, int backgroundPlane, int16 color, int oldVar2, int oldVar4, int oldVar6)
{
  char* ax;
  objectStruct* savePObject = pObject;
  objectStruct* cx;

  objectStruct* pNewElement;
  objectStruct* si = pObject->next;
  objectStruct* var_2;

  while(si)
  {
    pObject = si;
    si = si->next;
  }

  var_2 = si;

  pNewElement = (objectStruct*)malloc(sizeof(objectStruct));

  pNewElement->next = pObject->next;
  pObject->next = pNewElement;

  pNewElement->idx = oldVar8;
  pNewElement->type = 5;
  pNewElement->backgroundPlane = backgroundPlane;
  pNewElement->overlay = overlayIdx;
  pNewElement->field_A = oldVar6;
  pNewElement->field_C = oldVar4;
  pNewElement->spriteIdx = oldVar2;
  pNewElement->field_10 = color;
  pNewElement->hide = 0;
  pNewElement->field_16 = scriptNumber;
  pNewElement->field_18 = scriptOverlayNumber;
  pNewElement->gfxPtr = NULL;

  if(var_2)
  {
    cx = var_2;
  }
  else
  {
    cx = savePObject;
  }

  pNewElement->prev = cx->prev;
  cx->prev = pNewElement;

  ax = getText(oldVar8, overlayIdx);

  if(ax)
  {
    pNewElement->gfxPtr = renderText(oldVar2, (uint8*)ax);
  }
}

} // End of namespace Cruise



