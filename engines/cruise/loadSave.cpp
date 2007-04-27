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

void loadSavegameDataSub1(FILE* fileHandle)
{
  int i;

  for(i=1;i<numOfLoadedOverlay;i++)
  {
    filesData[i].field_4 = NULL;
    filesData[i].field_0 = NULL;
    filesData2[i].field_0 = 0;

    if(overlayTable[i].alreadyLoaded)
    {
      fread(&filesData2[i].field_0,2,1,fileHandle);

      if(filesData2[i].field_0)
      {
        filesData[i].field_0 = (uint8*)mallocAndZero(filesData2[i].field_0);
        if(filesData[i].field_0)
        {
          fread(filesData[i].field_0,filesData2[i].field_0,1,fileHandle);
        }
      }

      fread(&filesData2[i].field_2,2,1,fileHandle);

      if(filesData2[i].field_2)
      {
        filesData[i].field_4 = (uint8*)mallocAndZero(filesData2[i].field_2*12);
        if(filesData[i].field_4)
        {
          fread(filesData[i].field_4,filesData2[i].field_2*12,1,fileHandle);
        }
      }
    }
  }
}

void loadScriptsFromSave(FILE* fileHandle,scriptInstanceStruct* entry)
{
  short int numScripts;
  int i;

  fread(&numScripts,2,1,fileHandle);

  for(i=0;i<numScripts;i++)
  {
    scriptInstanceStruct* ptr = (scriptInstanceStruct*)mallocAndZero(sizeof(scriptInstanceStruct));

    fread(ptr,0x1C,1,fileHandle); // use 0x1C as our scriptInstanceStruct is bigger than in original because of cross platform problems

    fread(&ptr->varA,2,1,fileHandle);

    if(ptr->varA)
    {
      ptr->var6 = (uint8*)mallocAndZero(ptr->varA);

      fread(ptr->var6,ptr->varA,1,fileHandle);
    }

    /////////
    ptr->bitMask = *((int16*)ptr+1);
    /////////

    ptr->nextScriptPtr = 0;

    entry->nextScriptPtr = ptr;
    entry = ptr;
  }
}

void loadSavegameDataSub2(FILE * f)
{
  unsigned short int n_chunks;
  int i;
  objectStruct *p;
  objectStruct *t;

  objectHead.next = NULL; // Not in ASM code, but I guess the variable is defaulted
                          // to this value in the .exe

  fread(&n_chunks, 2, 1, f);
  // BIG ENDIAN MACHINES, PLEASE SWAP IT

  p = &objectHead;

  for (i = 0; i < n_chunks; i++)
  {
    t = (objectStruct *) mallocAndZero(sizeof(objectStruct));

    fseek(f, 4, SEEK_CUR);
    fread(&t->idx, 1, 0x30, f);

    t->next = NULL;
		p->next = t;
    t->prev = objectHead.prev;
    objectHead.prev = t;
    p = t;
  }
}

void loadSavegameActor(FILE* fileHandle)
{
  short int numEntry;
  actorStruct* ptr;
  int i;

  fread(&numEntry,2,1,fileHandle);

  ptr = &actorHead;

  for(i=0;i<numEntry;i++)
  {
    actorStruct* current = (actorStruct*)mallocAndZero(sizeof(actorStruct));
    fseek(fileHandle, 4, SEEK_CUR);
    fread(&current->var4,0x26,1,fileHandle);

    current->next = NULL;
    ptr->next = current;
    current->prev = actorHead.prev;
    actorHead.prev = current;
    ptr = current->next;
  }
}

void loadSavegameDataSub5(FILE* fileHandle)
{
  if(var1)
  {
    fread(&saveVar1,1,1,fileHandle);

    if(saveVar1)
    {
      fread(saveVar2,saveVar1,1,fileHandle);
    }
  }
  else
  {
    fread(&saveVar1,1,1,fileHandle);
  }

}

void loadSavegameDataSub6(FILE* fileHandle)
{
  int32 var;

  fread(&var,4,1,fileHandle);
  flipLong(&var);

  if(var)
  {
    int i;
    
    fread(&numberOfWalkboxes, 2, 1, fileHandle);

    if(numberOfWalkboxes)
    {
      fread(walkboxType, numberOfWalkboxes * 2, 1, fileHandle);
      fread(walkboxType, numberOfWalkboxes * 2, 1, fileHandle);
    }

    for(i=0;i<10;i++)
    {
      fread(&persoTable[i],4,1,fileHandle);

      if(persoTable[i])
      {
        assert(sizeof(persoStruct) == 0x6AA);
        persoTable[i] = (persoStruct*)mallocAndZero(sizeof(persoStruct));
        fread(persoTable[i],0x6AA,1,fileHandle);
      }
    }
  }
}


int loadSavegameData(int saveGameIdx)
{
  char buffer[256];
  FILE* fileHandle;
  char saveIdentBuffer[6];
  int j;
  int initVar1Save;
  objectStruct* currentObjectHead;

  sprintf(buffer,"CR.%d",saveGameIdx);

  fileHandle = fopen(buffer,"rb");

  if(!fileHandle)
  {
    printInfoBlackBox("Sauvegarde non trouvée...");
    waitForPlayerInput();
    return(-1);
  }

  printInfoBlackBox("Chargement en cours...");

  fread(saveIdentBuffer,6,1,fileHandle);

  if(strcmp(saveIdentBuffer,"SAVPC"))
  {
    fclose(fileHandle);
    return(-1);
  }

  //initVars();

  fread(&var1,2,1,fileHandle);
  fread(&var2,2,1,fileHandle);
  fread(&var3,2,1,fileHandle);
  fread(&var4,2,1,fileHandle);
  fread(&userEnabled,2,1,fileHandle);
  fread(&var6,2,1,fileHandle);
  fread(&var7,2,1,fileHandle);
  fread(&var8,2,1,fileHandle);
  fread(&userDelay,2,1,fileHandle);
  fread(&sysKey,2,1,fileHandle);
  fread(&var11,2,1,fileHandle);
  fread(&var12,2,1,fileHandle);
  fread(&var13,2,1,fileHandle);
  fread(&var14,2,1,fileHandle);
  fread(&affichePasMenuJoueur,2,1,fileHandle);
  fread(&var20,2,1,fileHandle);
  fread(&var22,2,1,fileHandle);
  fread(&var23,2,1,fileHandle);
  fread(&var24,2,1,fileHandle);
  fread(&automaticMode,2,1,fileHandle);

  // video param (not loaded in EGA mode)

  fread(&video4,2,1,fileHandle);
  fread(&video2,2,1,fileHandle);
  fread(&video3,2,1,fileHandle);
  fread(&colorOfSelectedSaveDrive,2,1,fileHandle);

  //

  fread(&var30,2,1,fileHandle);
  fread(&var31,2,1,fileHandle);
  fread(&var34,2,1,fileHandle);
  fread(&var35,2,1,fileHandle);
  fread(&animationStart,2,1,fileHandle);
  fread(&currentActiveBackgroundPlane,2,1,fileHandle);
  fread(&initVar3,2,1,fileHandle);
  fread(&initVar2,2,1,fileHandle);
  fread(&var22,2,1,fileHandle);
  fread(&main5,2,1,fileHandle);
  fread(&numOfLoadedOverlay,2,1,fileHandle);
  fread(&setup1,2,1,fileHandle);
  fread(&fontFileIndex,2,1,fileHandle);
  fread(&currentActiveMenu,2,1,fileHandle);
  fread(&main7,2,1,fileHandle); // ok
  fread(&main17,2,1,fileHandle);
  fread(&main14,2,1,fileHandle);
  fread(&main8,2,1,fileHandle);
  fread(&var39,2,1,fileHandle);
  fread(&var42,2,1,fileHandle);
  fread(&var45,2,1,fileHandle);
  fread(&var46,2,1,fileHandle);
  fread(&var47,2,1,fileHandle);
  fread(&var48,2,1,fileHandle);
  fread(&flagCt,2,1,fileHandle);
  fread(&var41,2,1,fileHandle);
  fread(&entrerMenuJoueur,2,1,fileHandle);

  fread(var50,64,1,fileHandle);
  fread(var50,64,1,fileHandle); // Hu ? why 2 times ?
  fread(&systemStrings,sizeof(systemStrings),1,fileHandle); // ok
  fread(currentCtpName,40,1,fileHandle);
  fread(backgroundTable,120,1,fileHandle);
  fread(palette,256,2,fileHandle); // ok
  fread(initVar5,24,1,fileHandle);
  fread(globalVars,setup1*2,1,fileHandle);
  fread(filesDatabase,9766,1,fileHandle);
  fread(overlayTable,40*numOfLoadedOverlay,1,fileHandle); // ok
  fread(mediumVar,0x880,1,fileHandle);

  loadSavegameDataSub1(fileHandle);
  loadScriptsFromSave(fileHandle,&scriptHandle2);
  loadScriptsFromSave(fileHandle,&scriptHandle1);

  loadSavegameDataSub2(fileHandle);
  loadBackgroundIncrustFromSave(fileHandle);
  loadSavegameActor(fileHandle);
  loadSavegameDataSub5(fileHandle);
  loadSavegameDataSub6(fileHandle);

  fclose(fileHandle); // finished with loading !!!!! Yatta !

  for(j=0;j<64;j++)
  {
    mediumVar[j].ptr=NULL;
  }

  for(j=1;j<numOfLoadedOverlay;j++)
  {
    if(overlayTable[j].alreadyLoaded)
    {
      overlayTable[j].alreadyLoaded = 0;
      loadOverlay((uint8*)overlayTable[j].overlayName);

      if(overlayTable[j].alreadyLoaded)
      {
        ovlDataStruct* ovlData = overlayTable[j].ovlData;

        if(filesData[j].field_0)
        {
          if(ovlData->data4Ptr)
          {
            free(ovlData->data4Ptr);
          }

          ovlData->data4Ptr = (uint8*)filesData[j].field_0;
          ovlData->sizeOfData4 = filesData2[j].field_0;
        }

        if(filesData[j].field_4)
        {
          if(ovlData->objData2WorkTable)
          {
            free(ovlData->objData2WorkTable);
          }

          ovlData->objData2WorkTable = (objectParams*)filesData[j].field_4; // TODO: fix !
          ovlData->size9 = filesData2[j].field_2;
        }

      }
    }
  }

  updateAllScriptsImports();

  saveVar6[0] = 0;

  initVar1Save = initVar1;

  for(j=0;j<257;j++)
  {
    if(filesDatabase[j].subData.ptr)
    {
      int i;
      int k;

      for(i=j+1;i<257;i++)
      {
        if(filesDatabase[i].subData.ptr)
        {
          if(strcmpuint8(filesDatabase[j].subData.name,filesDatabase[i].subData.name))
          {
            break;
          }
        }
        else
        {
          break;
        }
      }

      for(k=j;k<i;k++)
      {
        if(filesDatabase[k].subData.ptr2)
          initVar1 = 0;

        filesDatabase[k].subData.ptr = NULL;
        filesDatabase[k].subData.ptr2 = NULL;
      }

      if(i<2)
      {
        printf("Unsupported mono file load!\n");
        exit(1);
        //loadFileMode1(filesDatabase[j].subData.name,filesDatabase[j].subData.var4);
      }
      else
      {
        loadFileMode2((uint8*)filesDatabase[j].subData.name,filesDatabase[j].subData.index, j, i-j);
        j = i-1;
      }

      initVar1 = initVar1Save;
    }
  }

  saveVar6[0] = 0;

  currentObjectHead = objectHead.next;

  while(currentObjectHead)
  {
    if(currentObjectHead->type == 5)
    {
      uint8* ptr = mainProc14(currentObjectHead->overlay,currentObjectHead->idx);

      ASSERT(0);

      if(ptr)
      {
        ASSERT(0);
        //*(int16*)(currentobjectHead->datas+0x2E) = getSprite(ptr,*(int16*)(currentobjectHead->datas+0xE));
      }
      else
      {
        //*(int16*)(currentobjectHead->datas+0x2E) = 0;
      }
    }

    currentObjectHead = currentObjectHead->next;
  }

  //TODO: here, restart music
  
  if(strlen((char*)currentCtpName))
  {
    ctpVar1 = 1;
    loadCtp(currentCtpName);
    ctpVar1 = 0;
  }

  //prepareFadeOut();
  //gfxModuleData.gfxFunction8();

  for(j=0;j<8;j++)
  {
    if(strlen((char*)backgroundTable[j].name))
    {
      loadBackground(backgroundTable[j].name,j);
    }
  }

  regenerateBackgroundIncrust(&backgroundIncrustHead);

  // to finish

  changeCursor(0);
  mainDraw(1);
  flipScreen();

  return(0);
}

} // End of namespace Cruise
