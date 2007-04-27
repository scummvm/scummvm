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

menuStruct* menuTable[8];

menuStruct* createMenu(int X, int Y, char* menuName)
{
  menuStruct* entry;

  entry = (menuStruct*)malloc(sizeof(menuStruct));
  ASSERT(entry);

  entry->x = X - 80;
  entry->y = Y;
  entry->stringPtr = menuName;
  entry->numElements = 0;
  entry->ptrNextElement = NULL;
  entry->gfx = renderText(160,  (uint8*)menuName);

  return entry;
}

// TODO: rewrite to remove the goto
void addSelectableMenuEntry(int var0, int var1, menuStruct* pMenu, int var2, int color, char* menuText)
{
  menuElementStruct* di;
  menuElementStruct* var_6;
  menuElementStruct* pNewElement;
  menuElementSubStruct* pSubStruct;
  menuElementSubStruct* pSubStructCurrent;

  if(pMenu->numElements <= 48)
  {
    var_6 = pMenu->ptrNextElement;

    if(var_6)
    {
      do
      {
        di = var_6;
        if(var2)
        {
          if(!strcmp(var_6->string, menuText))
          {
            pNewElement = var_6;
            pSubStruct = (menuElementSubStruct*)allocAndZero(sizeof(menuElementSubStruct));
            ASSERT(pSubStruct);

            pSubStruct->pNext = NULL;
            pSubStruct->var2 = var0;
            pSubStruct->var4 = var1;

            pSubStructCurrent = pNewElement->ptrSub;

            if(!pSubStructCurrent)
            {
              pNewElement->ptrSub = pSubStruct;
              return;
            }

            if(pSubStructCurrent->pNext)
            {
              do
              {
                pSubStructCurrent = pSubStructCurrent->pNext;
              }while(pSubStructCurrent->pNext);
            }

            pSubStructCurrent->pNext = pSubStruct;
            return;
          }
        }
        var_6 = var_6->next;
      }
      while(var_6);

      var_6 = di;
    }

    pNewElement = (menuElementStruct*)allocAndZero(sizeof(menuElementStruct));
    ASSERT(pNewElement);
    pSubStruct = (menuElementSubStruct*)allocAndZero(sizeof(menuElementSubStruct));
    ASSERT(pSubStruct);

    pNewElement->string = menuText;
    pNewElement->next = NULL;
    pNewElement->varC = 0;
    pNewElement->color = color;
    pNewElement->gfx = renderText(160,(uint8*)menuText);

    if(var_6 == NULL)
    {
      pMenu->ptrNextElement = pNewElement;
    }
    else
    {
      var_6->next = pNewElement;
    }

    pNewElement->ptrSub = pSubStruct;

    pSubStruct->pNext = NULL;
    pSubStruct->var2 = var0;
    pSubStruct->var4 = var1;

    pMenu->numElements++;
  }
}

void updateMenuMouse(int mouseX, int mouseY, menuStruct* pMenu)
{
  if(pMenu)
  {
    if(pMenu->gfx)
    {
      int height = pMenu->gfx->height; // rustine
      int var_2  = 0;
      menuElementStruct* pCurrentEntry = pMenu->ptrNextElement;

      while(pCurrentEntry)
      {
        pCurrentEntry->varC = 0;

        if(var_2 == 0)
        {
          if((mouseX > pCurrentEntry->x) && ((pCurrentEntry->x + 160) >= mouseX))
          {
            if((mouseY > pCurrentEntry->y) && ((pCurrentEntry->y + height) >= mouseY))
            {
              var_2 = 1;
              pCurrentEntry->varC = 1;
            }
          }
        }

        pCurrentEntry = pCurrentEntry->next;
      }
    }
  }
}

int processMenu(menuStruct* pMenu)
{
  int16 mouseX;
  int16 mouseY;
  int16 mouseButton;
  int di;
  int si;
  currentActiveMenu = 0;

  mainDraw(1);
  flipScreen();

  di = 0;
  si = 0;

  do
  {
    getMouseStatus(&main10, &mouseX, &mouseButton, &mouseY);

    updateMenuMouse(mouseX, mouseY, pMenu);

    if(mouseButton)
    {
      if(di)
      {
        si = 1;
      }
    }
    else
    {
      di = 1;
    }

    mainDraw(1);
    flipScreen();

//    readKeyboard();
  }while(!si);

  currentActiveMenu = -1;

  mainDraw(1);
  flipScreen();

  return 0;
}

int playerMenu(int menuX, int menuY)
{
  int retourMenu;
  int restartGame = 0;

  if(entrerMenuJoueur && affichePasMenuJoueur)
  {
    if(var0)
    {
      systemStrings.param = 0;
      var24 = 0;
      var23 = 0;
      freeStuff2();
    }
/*
    if(currentMenu)
    {
      freeMenu(currentMenu);
      currentMenu = 0;
      var37 = 0;
      var38 = 0;
      main9 = -1;
    }

    if(inventoryMenu)
    {
      freeMenu(inventoryMenu);
      inventoryMenu = 0;
      var37 = 0;
      var38 = 0;
      main9 = -1;
    }*/

/*    if(mouseVar2)
    {
      free3(mouseVar2);
    } */

/*    mouseVar2 = 0;
    mouseVar1 = 0; */
    freeDisk();

    menuTable[0] = createMenu(menuX, menuY, "Menu Joueur");
    ASSERT(menuTable[0]);

    addSelectableMenuEntry(0, 3, menuTable[0], 1, -1, "Lecteur de Sauvegarde");
    if(userEnabled)
    {
      addSelectableMenuEntry(0, 4, menuTable[0], 1, -1, "Sauvegarde");
    }
    addSelectableMenuEntry(0, 5, menuTable[0], 1, -1, "Chargement");
    addSelectableMenuEntry(0, 6, menuTable[0], 1, -1, "Recommencer le jeu");
    addSelectableMenuEntry(0, 7, menuTable[0], 1, -1, "Chargement");

    retourMenu = processMenu(menuTable[0]);
  }

  return 0;
}

void freeGfx(gfxEntryStruct* pGfx)
{
  if(pGfx->imagePtr)
  {
    free(pGfx->imagePtr);
  }

  free(pGfx);
}

void freeMenu(menuStruct* pMenu)
{
  menuElementStruct* pElement = pMenu->ptrNextElement;

  while(pElement)
  {
    menuElementStruct* next;
    menuElementSubStruct* pSub = pElement->ptrSub;

    next = pElement->next;

    while(pSub)
    {
      menuElementSubStruct* next;

      next = pSub->pNext;

      free(pSub);

      pSub=next;
    }

    if(pElement->gfx)
    {
      freeGfx(pElement->gfx);
    }

    free(pElement);

    pElement = next;
  }

  freeGfx(pMenu->gfx);
  free(pMenu);
}

} // End of namespace Cruise

