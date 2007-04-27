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

#include "common/stdafx.h"
#include "common/events.h"

#include "cruise/cruise_main.h"
#include <time.h>

namespace Cruise {

unsigned int timer = 0;

void drawSolidBox(int32 x1, int32 y1, int32 x2, int32 y2, uint8 color)
{
  int32 i;
  int32 j;

  for(i=x1;i<x2;i++)
  {
    for(j=y1;j<y2;j++)
    {
      globalScreen[j*320+i] = color;
    }
  }
}

void drawBlackSolidBoxSmall()
{
//  gfxModuleData.drawSolidBox(64,100,256,117,0);
  drawSolidBox(64,100,256,117,0);
}

void resetRaster(uint8* rasterPtr, int32 rasterSize)
{
  memset(rasterPtr,0,rasterSize);
}


void drawInfoStringSmallBlackBox(uint8* string)
{
  //uint8 buffer[256];
  
  gfxModuleData_field_90();
  gfxModuleData_gfxWaitVSync();
  drawBlackSolidBoxSmall();

  drawString(10,100,string,gfxModuleData.pPage10,video4,300);

  gfxModuleData_flip();

	flipScreen();

	while(1);
}

void loadPakedFileToMem(int fileIdx, uint8* buffer)
{
  //changeCursor(1);

  currentVolumeFile.seek(volumePtrToFileDescriptor[fileIdx].offset,SEEK_SET);
  currentVolumeFile.read(buffer,volumePtrToFileDescriptor[fileIdx].size);
}

int loadScriptSub1(int scriptIdx, int param)
{
  objDataStruct* ptr2;
  int counter;
  int i;
  
  if(!overlayTable[scriptIdx].ovlData)
    return(0);

  ptr2 = overlayTable[scriptIdx].ovlData->objDataTable;

  if(!ptr2)
    return(0);

  if(overlayTable[scriptIdx].ovlData->numObjData==0)
    return(0);

  counter = 0;

  for(i=0;i<overlayTable[scriptIdx].ovlData->numObjData;i++)
  {
		if(ptr2[i].var0 == param)
    {
      counter++;
    }
  }

  return(counter);
}

void saveShort(void* ptr, short int var)
{
  *(int16*)ptr = var;

  flipShort((int16*)ptr);
}

int16 loadShort(void* ptr)
{
  short int temp;

  temp = *(int16*)ptr;

  flipShort(&temp);

  return(temp);
}


void resetFileEntryRange(int param1, int param2)
{
  int i;

  for(i=param1;i<param2;i++)
  {
    resetFileEntry(i);
  }
}

int getProcParam(int overlayIdx, int param2, uint8* name)
{
  int numExport;
  int i;
  exportEntryStruct* exportDataPtr;
  uint8* exportNamePtr;
  uint8 exportName[80];

  if(!overlayTable[overlayIdx].alreadyLoaded)
    return 0;

  if(!overlayTable[overlayIdx].ovlData)
    return 0;

  numExport = overlayTable[overlayIdx].ovlData->numExport;
  exportDataPtr = overlayTable[overlayIdx].ovlData->exportDataPtr;
  exportNamePtr = overlayTable[overlayIdx].ovlData->exportNamesPtr;

  if(!exportNamePtr)
    return 0;

  for(i=0;i<numExport;i++)
  {
    if(exportDataPtr[i].var4 == param2)
    {
      strcpyuint8(exportName,exportDataPtr[i].offsetToName+exportNamePtr);

      if(!strcmpuint8(exportName,name))
      {
        return(exportDataPtr[i].idx);
      }
    }
  }

  return 0;
}

void changeScriptParamInList(int param1, int param2, scriptInstanceStruct* pScriptInstance,int newValue, int param3)
{
  pScriptInstance = pScriptInstance->nextScriptPtr;
  while(pScriptInstance)
  {
    if( (pScriptInstance->overlayNumber == param1 || param1 == -1)
      &&(pScriptInstance->scriptNumber  == param2 || param2 == -1)
      &&(pScriptInstance->var12         == param3 || param3 == -1))
    {
      pScriptInstance->var12 = newValue;
    }

    pScriptInstance = pScriptInstance->nextScriptPtr;
  }
}


void initBigVar3()
{
  int i;

  for(i=0;i<257;i++)
  {
    if(filesDatabase[i].subData.ptr)
    {
      free(filesDatabase[i].subData.ptr);
    }

    filesDatabase[i].subData.ptr = NULL;
    filesDatabase[i].subData.ptr2 = NULL;

    filesDatabase[i].subData.index = -1;
    filesDatabase[i].subData.resourceType = 0;
  }
}

void resetPtr2(scriptInstanceStruct* ptr)
{
  ptr->nextScriptPtr = NULL;
  ptr->scriptNumber = -1;
}

void resetPtr(objectStruct* ptr)
{
  ptr->next = NULL;
  ptr->prev = NULL;
}

void resetActorPtr(actorStruct* ptr)
{
  ptr->next = NULL;
  ptr->prev = NULL;
}

ovlData3Struct* getOvlData3Entry(int32 scriptNumber, int32 param)
{
  ovlDataStruct* ovlData = overlayTable[scriptNumber].ovlData;

  if(!ovlData)
  {
    return NULL;
  }

  if(param<0)
  {
    return NULL;
  }

  if(ovlData->numScripts1 <= param)
  {
    return NULL;
  }

  if(!ovlData->data3Table)
  {
    return NULL;
  }

  return(&ovlData->data3Table[param]);
}

ovlData3Struct* scriptFunc1Sub2(int32 scriptNumber, int32 param)
{
  ovlDataStruct* ovlData = overlayTable[scriptNumber].ovlData;

  if(!ovlData)
  {
    return NULL;
  }

  if(param<0)
  {
    return NULL;
  }

  if(ovlData->numScripts2 <= param)
  {
    return NULL;
  }

  if(!ovlData->ptr1)
  {
    return NULL;
  }

  return((ovlData3Struct*)(ovlData->ptr1+param*0x1C));
}

void scriptFunc2(int scriptNumber,scriptInstanceStruct* scriptHandle, int param, int param2)
{
  if(scriptHandle->nextScriptPtr)
  {
    if(scriptNumber == scriptHandle->nextScriptPtr->overlayNumber || scriptNumber != -1)
    {
      if(param2 == scriptHandle->nextScriptPtr->scriptNumber || param2 != -1)
      {
        scriptHandle->nextScriptPtr->sysKey = param;
      }
    }
  }
}

uint8* getDataFromData3(ovlData3Struct* ptr, int param)
{
  uint8* dataPtr;
  
  if(!ptr)
    return(NULL);

  dataPtr = ptr->dataPtr;

  if(!dataPtr)
    return(NULL);

  switch(param)
  {
  case 0:
    {
      return(dataPtr);
    }
  case 1:
    {
      return(dataPtr + ptr->offsetToSubData3); // strings
    }
  case 2:
    {
      return(dataPtr + ptr->offsetToSubData2);
    }
  case 3:
    {
      return(dataPtr + ptr->offsetToImportData); // import data
    }
  case 4:
    {
      return(dataPtr + ptr->offsetToImportName); // import names
    }
  case 5:
    {
      return(dataPtr + ptr->offsetToSubData5);
    }
  default:
    {
      return(NULL);
    }
  }
}

void printInfoBlackBox(char* string)
{
}

void waitForPlayerInput()
{
}

void getFileExtention(const char* name,char* buffer)
{
  while(*name != '.' && *name)
  {
    name++;
  }

  strcpy(buffer,name);
}

void removeExtention(const char* name, char* buffer) // not like in original
{
  char* ptr;
  
  strcpy(buffer,name);

  ptr = strchr(buffer,'.');

  if(ptr)
    *ptr = 0;
}

int lastFileSize;

int loadFileSub1(uint8** ptr, uint8* name, uint8* ptr2)
{
  int i;
  char buffer[256];
  int fileIdx;
  int unpackedSize;
  uint8* unpackedBuffer;
  
  for(i=0;i<64;i++)
  {
    if(mediumVar[i].ptr)
    {
      if(!strcmpuint8(mediumVar[i].name,name))
      {
        printf("Unsupported code in loadFIleSub1 !\n");
        exit(1);
      }
    }
  }

  getFileExtention((char*)name,buffer);

  if(!strcmp(buffer,".SPL"))
  {
    removeExtention((char*)name,buffer);

   // if(useH32)
    {
      strcatuint8(buffer,".H32");
    }
   /* else
    if(useAdlib)
    {
      strcatuint8(buffer,".ADL");
    }
    else
    { 
      strcatuint8(buffer,".HP");
    }*/
  }
  else
  {
    strcpyuint8(buffer,name);
  }

  fileIdx = findFileInDisks((uint8*)buffer);

  if(fileIdx<0)
    return(-18);

  unpackedSize = loadFileVar1 = volumePtrToFileDescriptor[fileIdx].extSize + 2;

  // TODO: here, can unpack in gfx module buffer
  unpackedBuffer = (uint8*)mallocAndZero(unpackedSize);

  if(!unpackedBuffer)
  {
    return(-2);
  }

	lastFileSize = unpackedSize;

  if(volumePtrToFileDescriptor[fileIdx].size +2 != unpackedSize)
  {
    unsigned short int realUnpackedSize;
    uint8* tempBuffer;
    uint8* pakedBuffer = (uint8*) mallocAndZero(volumePtrToFileDescriptor[fileIdx].size +2);

    loadPakedFileToMem(fileIdx,pakedBuffer);

    realUnpackedSize = *(uint16*)(pakedBuffer+volumePtrToFileDescriptor[fileIdx].size-2);
    flipShort((int16*)&realUnpackedSize);

	lastFileSize = realUnpackedSize;

    tempBuffer = (uint8*)mallocAndZero(realUnpackedSize);

    decomp((uint8*)pakedBuffer+volumePtrToFileDescriptor[fileIdx].size-4,(uint8*)unpackedBuffer+realUnpackedSize,realUnpackedSize);

    free(pakedBuffer);
  }
  else
  {
    loadPakedFileToMem(fileIdx,unpackedBuffer);
  }

  *ptr = unpackedBuffer;

  return(1);
}

void resetFileEntry(int32 entryNumber)
{
  if(entryNumber>=257)
    return;

  if(!filesDatabase[entryNumber].subData.ptr)
    return;

  free(filesDatabase[entryNumber].subData.ptr);

  filesDatabase[entryNumber].subData.ptr = NULL;
  filesDatabase[entryNumber].subData.ptr2 = NULL;
  filesDatabase[entryNumber].widthInColumn = 0;
  filesDatabase[entryNumber].width = 0;
  filesDatabase[entryNumber].resType = 0;
  filesDatabase[entryNumber].height = 0;
  filesDatabase[entryNumber].subData.index = -1;
  filesDatabase[entryNumber].subData.resourceType = 0;
  filesDatabase[entryNumber].subData.field_1C = 0;
  filesDatabase[entryNumber].subData.name[0] = 0;

}

uint8* mainProc14(uint16 overlay, uint16 idx)
{
  ASSERT(0);
  
  return NULL;
}

int initAllData(void)
{
  int i;
  
  setupFuncArray();
  setupOpcodeTable();
  initOverlayTable();

  setup1 = 0;
  currentActiveBackgroundPlane = 0;

  freeDisk();

  initVar5[0] = -1;
  initVar5[3] = -1;
  initVar5[6] = -1;
  initVar5[9] = -1;

  menuTable[0] = NULL;

  for(i=0;i<2000;i++)
  {
    globalVars[i] = 0;
  }

  for(i=0;i<8;i++)
  {
    backgroundTable[i].name[0] = 0;
  }

  for(i=0;i<257;i++)
  {
    filesDatabase[i].subData.ptr = NULL;
    filesDatabase[i].subData.ptr2 = NULL;
  }

  initBigVar3();

  resetPtr2(&scriptHandle2);
  resetPtr2(&scriptHandle1);

  resetPtr(&objectHead);

  resetActorPtr(&actorHead);
  resetBackgroundIncrustList(&backgroundIncrustHead);

  bootOverlayNumber = loadOverlay((uint8*)"AUTO00");

#ifdef DUMP_SCRIPT
	loadOverlay("TITRE");
	loadOverlay("TOM");
	loadOverlay("XX2");
	loadOverlay("SUPER");
	loadOverlay("BEBE1");
	loadOverlay("BIBLIO");
	loadOverlay("BRACAGE");
	loadOverlay("CONVERS");
	loadOverlay("DAF");
	loadOverlay("DAPHNEE");
	loadOverlay("DESIRE");
	loadOverlay("FAB");
	loadOverlay("FABIANI");
	loadOverlay("FIN");
	loadOverlay("FIN01");
	loadOverlay("FINBRAC");
	loadOverlay("GEN");
	loadOverlay("GENDEB");
	loadOverlay("GIFLE");
	loadOverlay("HECTOR");
	loadOverlay("HECTOR2");
	loadOverlay("I00");
	loadOverlay("I01");
	loadOverlay("I04");
	loadOverlay("I06");
	loadOverlay("I07");
	loadOverlay("INVENT");
	loadOverlay("JULIO");
	loadOverlay("LOGO");
	loadOverlay("MANOIR");
	loadOverlay("MISSEL");
	loadOverlay("POKER");
	loadOverlay("PROJ");
	loadOverlay("REB");
	loadOverlay("REBECCA");
	loadOverlay("ROS");
	loadOverlay("ROSE");
	loadOverlay("S01");
	loadOverlay("S02");
	loadOverlay("S03");
	loadOverlay("S04");
	loadOverlay("S06");
	loadOverlay("S07");
	loadOverlay("S08");
	loadOverlay("S09");
	loadOverlay("S10");
	loadOverlay("S103");
	loadOverlay("S11");
	loadOverlay("S113");
	loadOverlay("S12");
	loadOverlay("S129");
	loadOverlay("S131");
	loadOverlay("S132");
	loadOverlay("S133");
	loadOverlay("int16");
	loadOverlay("S17");
	loadOverlay("S18");
	loadOverlay("S19");
	loadOverlay("S20");
	loadOverlay("S21");
	loadOverlay("S22");
	loadOverlay("S23");
	loadOverlay("S24");
	loadOverlay("S25");
	loadOverlay("S26");
	loadOverlay("S27");
	loadOverlay("S29");
	loadOverlay("S30");
	loadOverlay("S31");
	loadOverlay("int32");
	loadOverlay("S33");
	loadOverlay("S33B");
	loadOverlay("S34");
	loadOverlay("S35");
	loadOverlay("S36");
	loadOverlay("S37");
	loadOverlay("SHIP");
	loadOverlay("SUPER");
	loadOverlay("SUZAN");
	loadOverlay("SUZAN2");
	loadOverlay("TESTA1");
	loadOverlay("TESTA2");
	//exit(1);
#endif

  if(bootOverlayNumber)
  {
    positionInStack = 0;

    attacheNewScriptToTail(bootOverlayNumber,&scriptHandle2,0,20,0,0,scriptType_20);
    scriptFunc2(bootOverlayNumber,&scriptHandle2,1,0);
  }

  strcpyuint8(systemStrings.bootScriptName,"AUTO00");

  return(bootOverlayNumber);
}

int removeFinishedScripts(scriptInstanceStruct* ptrHandle)
{
  scriptInstanceStruct* ptr = ptrHandle->nextScriptPtr; // can't destruct the head
  scriptInstanceStruct* oldPtr = ptrHandle;
    
  if(!ptr)
    return(0);

  do
  {
    if(ptr->scriptNumber == -1)
    {
      oldPtr->nextScriptPtr = ptr->nextScriptPtr;

      if(ptr->var6 && ptr->varA)
      {
      //  free(ptr->var6);
      }

      free(ptr);

      ptr = oldPtr->nextScriptPtr;
    }
    else
    {
      oldPtr = ptr;
      ptr = ptr->nextScriptPtr;
    }
  }while(ptr);

  return(0);
}

int nePasAffichierMenuDialogue;
int var37 = 0;
int var38 = 0;

int getCursorFromObject(int mouseX, int mouseY, int* outX, int* outY)
{
  int16 var_2;
  int16 var_4;
  int16 var_14;
  int16 var_16;
  objectParamsQuery params;
  int16 var_10;
  int16 var_E;
  int16 var_C;
//  int16 var_42;
  int16 var_A;
  int16 var_6;

  char objectName[80];

  objectStruct* currentObject = objectHead.prev;

  while(currentObject)
  {
    if(currentObject->overlay >= 0)
    {
      if(overlayTable[currentObject->overlay].alreadyLoaded)
      {
        if(currentObject->type == 4 || currentObject->type == 1 || currentObject->type == 9 || currentObject->type == 3)
        {
          strcpy(objectName, getObjectName(currentObject->idx, overlayTable[currentObject->overlay].ovlData->specialString2));

          if(strlen(objectName))
          {
            if(currentObject->hide == 0)
            {
              var_2 = currentObject->idx;
              var_4 = currentObject->overlay;
              var_14 = currentObject->followObjectIdx;
              var_16 = currentObject->followObjectOverlayIdx;

              getMultipleObjectParam(currentObject->overlay, currentObject->idx, &params);

              var_10 = 0;
              var_E = 0;
              var_C = 0;

              if((var_4 != var_16) && (var_2 != var_14))
              {
                objectParamsQuery params;
                getMultipleObjectParam(var_16, var_14, &params);

                var_C = params.X;
                var_E = params.Y;
                var_10 = params.fileIdx;
              }

              if(params.var5 >= 0 && params.fileIdx >= 0)
              {
                if(currentObject->type == 3)
                {
                  assert(0);

                  var_2 = params.scale;
                  var_A = params.X + var_C;

                  // TODO: this var3 is stupid, investigate...
                  if((var_A <= mouseX) && (var_A + params.fileIdx >= mouseX) && (mouseY >= params.Y + var_E) && (params.Y + var_E + var2 >= mouseY))
                  {
                    *outX = var_16;
                    *outY = var_14;
                    
                    return(currentObject->type);
                  }
                }
                else
                if(currentObject->type == 4 || currentObject->type == 1 || currentObject->type == 9 )
                {
                  int si;
                  int var_8;
                  int di;

                  var_A = params.X + var_C;
                  var_6 = params.Y + var_E;
                  
                  di = params.fileIdx;

                  if(di<0)
                  {
                    di += var_10;
                  }

/*                  if((filesDatabase[di].subData.resourceType == 8) && (filesDatabase[di].subData.ptr))
                  {
                    assert(0);
                  }
                  else */
                  {
                    var_4 = filesDatabase[di].resType;

                    if(var_4 == 1)
                    {
                      var_C = filesDatabase[di].widthInColumn/2;
                    }
                    else
                    {
                      var_C = filesDatabase[di].width;
                    }

                    var_8 = filesDatabase[di].height;

                    var_2 = mouseX - var_A;
                    si = mouseY - var_6;

                    if(var_2>0)
                    {
                      if(var_C>var_2)
                      {
                        if(si>0)
                        {
                          if(var_8>=si)
                          {
                            if(filesDatabase[di].subData.ptr)
                            {
                              if(var_4 == 1)
                              {
                              }
                              else
                              {
                              }

                              printf("should compare to mask in getCursorFromObject...\n");

                              *outX = var_16;
                              *outY = var_14;

                              printf("Selected: %s\n", objectName);

                              return currentObject->type;
                            }
                          }
                        }
                      }
                    }
                  }
                }
              }
            }
          }
        }
      }
    }

    currentObject = currentObject->prev;
  }

  *outX = 0;
  *outY = 0;

  return -1;
}

char keyboardVar = 0;

void freeStuff2(void)
{
  printf("implement freeStuff2\n");
}

void *allocAndZero(int size)
{
  void *ptr;

  ptr = malloc(size);
  memset(ptr,0,size);

  return ptr;
}

char* getObjectName(int index, uint8* string)
{
  int i;
  char* ptr = (char*)string;

  if(!string)
    return NULL;

  for(i=0;i<index;i++)
  {
    while(*ptr)
    {
      ptr++;
    }
    ptr++;
  }
  return ptr;
}

int buildInventorySub1(int overlayIdx, int objIdx)
{
  objDataStruct* pObjectData = getObjectDataFromOverlay(overlayIdx, objIdx);

  if(pObjectData)
  {
    return pObjectData->var1;
  }
  else
  {
    return -11;
  }
}

void buildInventory(int X, int Y)
{
  int numObjectInInventory;
  menuStruct* pMenu;
  
  pMenu = createMenu(X, Y, "Inventaire");

  menuTable[1] = pMenu;

  if(pMenu)
  {
    numObjectInInventory = 0;

    if(numOfLoadedOverlay > 1)
    {
      int i;

      for(i=1;i<numOfLoadedOverlay;i++)
      {
        ovlDataStruct* pOvlData = overlayTable[i].ovlData;

        if(pOvlData && pOvlData->objDataTable)
        {
          int var_2;

          var_2 = 0;

          if(pOvlData->numObjData)
          {
            int j;

            for(j=0;j<pOvlData->numObjData;j++)
            {
              if(buildInventorySub1(i,j) != 3)
              {
                int16 returnVar;

                getSingleObjectParam(i,j,5,&returnVar);

                if(returnVar<-1)
                {
                  addSelectableMenuEntry(i,j,pMenu,1,-1,getObjectName(j, pOvlData->specialString2));
                  numObjectInInventory++;
                }
              }
            }
          }
        }
      }
    }
  }

  if(numObjectInInventory == 0)
  {
    freeMenu(menuTable[1]);
    menuTable[1] = NULL;
  }
}

int currentMenuElementX;
int currentMenuElementY;
menuElementStruct* currentMenuElement;

menuElementSubStruct* getSelectedEntryInMenu(menuStruct* pMenu)
{
  menuElementStruct* pMenuElement;

  if(pMenu == NULL)
  {
    return NULL;
  }

  if(pMenu->numElements == 0)
  {
    return NULL;
  }

  pMenuElement = pMenu->ptrNextElement;

  while(pMenuElement)
  {
    if(pMenuElement->varC)
    {
      currentMenuElementX = pMenuElement->x;
      currentMenuElementY = pMenuElement->y;
      currentMenuElement = pMenuElement;

      return pMenuElement->ptrSub;
    }

    pMenuElement = pMenuElement->next;
  }

  return NULL;
}

int callInventoryObject(int param0, int param1, int x, int y)
{
  int var_2C;
  int var_30;
  int var_28;
  int var_1E;
  int16 returnVar;

  var_30 = -1;

  getSingleObjectParam(param0,param1,5,&returnVar);

  var_2C = 0;
  var_28 = 1;  

  for(var_1E=1;var_1E<numOfLoadedOverlay;var_1E++)
  {
    ovlDataStruct* var_2A = overlayTable[var_1E].ovlData;
    if(var_2A->ptr1)
    {
      int var_18;
      int var_14;

      var_18 = var_2A->numLinkData;

      if(var_18)
      {
        int var_16;

        var_16 = 0;
        
        for(var_14 = 0; var_14 < var_18;var_14++)
        {
          objDataStruct* pObject;
          linkDataStruct* var_34;
          int var_2;

          var_34 = &var_2A->linkDataPtr[var_14];

          var_2 = var_34->stringIdx;

          if(!var_2)
          {
            var_2 = var_1E;
          }

          pObject = getObjectDataFromOverlay(var_2,var_34->stringNameOffset);

          if(var_2 == param0)
          {
            if(param1 == var_34->stringNameOffset)
            {
              if(pObject)
              {
                if(pObject->var1 != 3)
                {
                  char var_214[80];
                  char var_1C4[80];
                  char var_174[80];
                  char var_124[80];
                  char var_D4[80];
                  char var_84[80];

                  ovlDataStruct* var_12;
                  ovlDataStruct* var_22;

                  int var_E = var_34->varIdx;
                  int cx = var_34->stringIdx;
                  int var_C = var_34->procIdx;

                  int di = var_E;
                  if(var_E == 0)
                    di = var_1E;

                  var_2 = cx;
                  if(cx == 0)
                    var_2 = var_1E;

                  if(var_C == 0)
                    var_C = var_1E;

                  var_12 = NULL;
                  var_22 = NULL;

                  var_214[0] = 0;
                  var_1C4[0] = 0;
                  var_174[0] = 0;
                  var_124[0] = 0;
                  var_D4[0] = 0;
                  var_84[0] = 0; 

                  if(di>0)
                  {
                    var_22 = overlayTable[di].ovlData;
                  }

                  if(var_2>0)
                  {
                    var_12 = overlayTable[var_2].ovlData;
                  }

                  if(var_12)
                  {
                    if(var_34->stringNameOffset)
                    {
                      var_30 = var_34->field_1A;
                      if(var_28)
                      {
                        if(var_12->specialString2)
                        {
                          if(var_30==-1 || var_30 == returnVar)
                          {
                            char* ptrName = getObjectName(var_34->stringNameOffset, var_12->specialString2);

                            menuTable[0] = createMenu(x,y,ptrName);
                            var_28 = 0;
                          }
                        }
                      }
                    }
                  }

                  if(var_22)
                  {
                    if(var_34->varNameOffset>=0)
                    {
                      if(var_22->specialString1)
                      {
                        char* ptr = getObjectName(var_34->varNameOffset, var_22->specialString1);

                        strcpy(var_214, ptr);

                        if(var_28 == 0)
                        {
                          if(var_30 ==-1 || var_30 == returnVar)
                          {
                            if(strlen(var_214))
                            {
                              attacheNewScriptToTail(var_1E,&scriptHandle1,var_34->field_2, 30, currentScriptPtr->scriptNumber,currentScriptPtr->overlayNumber, scriptType_30);
                            }
                            else
                            {
                              if(var_22->specialString1)
                              {
                                char* ptr = getObjectName(var_34->varNameOffset, var_22->specialString1);

                                var_2C = 1;

                                addSelectableMenuEntry(var_1E, var_14, menuTable[0], 1, -1, ptr);
                              }
                            }
                          }
                        }
                      }
                    }
                  }
                }
              }
            }
          }
        }
      }
    }
  }

  return var_2C;
}

int processInventory(void)
{
  if(menuTable[1])
  {
    menuElementSubStruct* pMenuElementSub = getSelectedEntryInMenu(menuTable[1]);

    if(pMenuElementSub)
    {
      int var2;
      int var4;

      var2 = pMenuElementSub->var2;
      var4 = pMenuElementSub->var4;

      freeMenu(menuTable[1]);
      menuTable[1] = NULL;

      callInventoryObject(var2, var4, currentMenuElementX+80, currentMenuElementY);

      return 1;
    }
    else
    {
      freeMenu(menuTable[1]);
      menuTable[1] = NULL;
    }
  }

  return 0;
}

int processInput(void)
{
  menuStruct* var_5C;

  int16 mouseX;
  int16 mouseY;
  int16 button;

  /*if(inputSub1keyboad())
  {
    return 1;
  }*/

  button = 0;

  if(sysKey != -1)
  {
    button = sysKey;
    mouseX = var11;
    mouseY = var12;
    sysKey = -1;
  }
  else
  {
    if(automaticMode == 0)
    {
      getMouseStatus(&main10, &mouseX, &button, &mouseY);
    }
  }

  if(button)
  {
    nePasAffichierMenuDialogue = 0;
  }

  if(userDelay)
  {
    userDelay--;
    return 0;
  }

  // test both buttons

  if(((button&3) == 3) || keyboardVar == 0x44 || keyboardVar == 0x53)
  {
    changeCursor(0);
    keyboardVar = 0;
    return(playerMenu(mouseX, mouseY));
  }

  if(!userEnabled)
  {
    return 0;
  }

  if(currentActiveMenu != -1)
  {
    var_5C = menuTable[currentActiveMenu];
    
    if(var_5C)
    {
      updateMenuMouse(mouseX,mouseY,var_5C);
    }
  }

  if(var6)
  {
    ASSERT(0);
  }

  if(button & 1)
  {
    if(nePasAffichierMenuDialogue == 0)
    {
      nePasAffichierMenuDialogue = 1;

      if(mouseVar1)
      {
        ASSERT(0);
      }

      if(var38 == 0) // are we in inventory mode ?
      {
        if(menuTable[0] == 0)
        {
          int X;
          int Y;
          int objIdx;

          objIdx = getCursorFromObject(mouseX, mouseY, &X, &Y);

          if(objIdx != -1)
          {
            //ASSERT(0);
            //moveActor(X,Y,mouseVar1);
          }
          else
          {
            var34 = mouseX;
            var35 = mouseY;
            animationStart=true;
            var38=0;
          }
        }
        //ASSERT(0);
      }
      else
      {
        if(processInventory())
        {
          var37 = 1;
          currentActiveMenu = 0;
          var38 = 0;
        }
        else
        {
          currentActiveMenu = -1;
          var38 = 0;
        }

        return 0;
      }

      //ASSERT(0);
    }
  }

  if((button & 2) || (keyboardVar == 0x43) || (keyboardVar == 0x52))
  {
    if(nePasAffichierMenuDialogue == 0)
    {
      keyboardVar = 0;

      if((mouseVar1 == 0) && (menuTable[0]))
      {
        ASSERT(0);
        freeMenu(menuTable[0]);
        menuTable[0] = NULL;
        var37 = 0;
        var38 = 0;
        currentActiveMenu = -1;
      }

      if(var37 || var38 || menuTable[1])
      {
        nePasAffichierMenuDialogue = 1;
        return 0;
      }

      buildInventory(mouseX, mouseY);

      if(menuTable[1])
      {
        currentActiveMenu = 1;
        var38 = 1;
      }
      else
      {
        var38 = 1;
      }

      nePasAffichierMenuDialogue = 1;
      return 0;
    }
  }
  return 0;
}

int oldMouseX;
int oldMouseY;

void manageEvents(int count) {
	Common::Event event;

	Common::EventManager *eventMan = g_system->getEventManager();
	while (eventMan->pollEvent(event)) {
		switch (event.type) {
	/*	case Common::EVENT_LBUTTONDOWN:
			mouseLeft = 1;
			break;
		case Common::EVENT_RBUTTONDOWN:
			mouseRight = 1;
			break;
		case Common::EVENT_MOUSEMOVE:
			break;*/
		case Common::EVENT_QUIT:
			g_system->quit();
			break;
	/*	case Common::EVENT_KEYDOWN:
			switch (event.kbd.keycode) {
			case '\n':
			case '\r':
			case 261: // Keypad 5
				if (allowPlayerInput) {
					mouseLeft = 1;
				}
				break;
			case 27:  // ESC
				if (allowPlayerInput) {
					mouseRight = 1;
				}
				break;
			case 282: // F1
				if (allowPlayerInput) {
					playerCommand = 0; // EXAMINE
					makeCommandLine();
				}
				break;
			case 283: // F2
				if (allowPlayerInput) {
					playerCommand = 1; // TAKE
					makeCommandLine();
				}
				break;
			case 284: // F3
				if (allowPlayerInput) {
					playerCommand = 2; // INVENTORY
					makeCommandLine();
				}
				break;
			case 285: // F4
				if (allowPlayerInput) {
					playerCommand = 3; // USE
					makeCommandLine();
				}
				break;
			case 286: // F5
				if (allowPlayerInput) {
					playerCommand = 4; // ACTIVATE
					makeCommandLine();
				}
				break;
			case 287: // F6
				if (allowPlayerInput) {
					playerCommand = 5; // SPEAK
					makeCommandLine();
				}
				break;
			case 290: // F9
				if (allowPlayerInput && !inMenu) {
					makeActionMenu();
					makeCommandLine();
				}
				break;
			case 291: // F10
				if (!disableSystemMenu && !inMenu) {
					g_cine->makeSystemMenu();
				}
				break; 
			default:
				//lastKeyStroke = event.kbd.keycode;
				break;
			}
			break; */
		default:
			break;
		}
	}

	/*if (count) {
		mouseData.left = mouseLeft;
		mouseData.right = mouseRight;
		mouseLeft = 0;
		mouseRight = 0;
	}
*/
	int i;

	for (i = 0; i < count; i++) {
		//FIXME(?): Maybe there's a better way to "fix" this?
		//
		//Since not all backends/ports can update the screen 
		//100 times per second, only update the screen every
		//other frame (1000 / 2 * 10 i.e. 50 times per second max.)
		if (i % 2)
			g_system->updateScreen();
		g_system->delayMillis(10);
		manageEvents(0);
	} 
}

void mainLoop(void)
{
  #define SPEED 40              /* Ticks per Frame */
  #define SLEEP_MIN 20          /* Minimum time a sleep takes, usually 2*GRAN */
  #define SLEEP_GRAN 1         /* Granularity of sleep */

  int frames=0;                   /* Number of frames displayed */
  //int32 t_start,t_left;
  //uint32 t_end;
  int32 q=0;                     /* Dummy */

  int enableUser = 0;
  //int16 mouseX;
  //int16 mouseY;
  //int16 mouseButton;

  scriptNameBuffer[0] = 0;
  systemStrings.bootScriptName[0] = 0;
  initVar4[0] = 0;
  currentActiveMenu = -1;
  main14 = -1;
  mouseVar1 = 0;
  main21 = 0;
  main22 = 0;
  main7 = 0;
  main8 = 0;
  main15 = 0;

  if(initAllData())
  {
    int playerDontAskQuit = 1;
    int quitValue2 = 1;
    int quitValue = 0;

    do
    {
      frames++;
//      t_start=Osystem_GetTicks();

//      readKeyboard();
      playerDontAskQuit = processInput();

      //if(enableUser)
      {
        userEnabled = 1;
        enableUser = 0;
      }

      manageScripts(&scriptHandle1);
      manageScripts(&scriptHandle2);

      removeFinishedScripts(&scriptHandle1);
      removeFinishedScripts(&scriptHandle2);

      processActors();

      if(var0)
      {
        ASSERT(0);
    /*    main3 = 0;
        var24 = 0;
        var23 = 0;

        freeStuff2(); */
      }

      if(initVar4[0])
      {
        ASSERT(0);
/*        redrawStrings(0,&initVar4,8);

        waitForPlayerInput();

        initVar4 = 0; */
      }

      if(affichePasMenuJoueur)
      {
        if(main5)
          fadeVar = 0;

        /*if(fadeVar)
        {
        //  TODO!
        }*/

				mainDraw(0);
				flipScreen();

   /*     if(userEnabled && !main7 && !main15 && currentActiveMenu == -1)
        {
          getMouseStatus(&main10, &mouseX, &mouseButton, &mouseY);

          if(mouseX != oldMouseX && mouseY != oldMouseY)
          {
            int cursorType;
            int newCursor1;
            int newCursor2;

            oldMouseX = mouseX;
            oldMouseY = mouseY;

            cursorType = getCursorFromObject(mouseX, mouseY, &newCursor1, &newCursor2);

            if(cursorType == 9)
            {
              changeCursor(5);
            }
            else
            if(cursorType == -1)
            {
              changeCursor(6);
            }
            else
            {
              changeCursor(4);
            }

          } 
        }
        else*/
        {
          changeCursor(0);
        }

        if(main7)
        {
          ASSERT(0);
        }

        if(main15)
        {
          ASSERT(0);
        }

        if(main14 != -1)
        {
          ASSERT(0);
        }
      }

	  // t_end = t_start+SPEED;
//      t_left=t_start-Osystem_GetTicks()+SPEED;
#ifndef FASTDEBUG
  /*    if(t_left>0)
        if(t_left>SLEEP_MIN)
          Osystem_Delay(t_left-SLEEP_GRAN);
      while(Osystem_GetTicks()<t_end){q++;}; */
#endif
	  manageEvents(4);

    }while(!playerDontAskQuit && quitValue2 && quitValue != 7);
  }

}

int oldmain(int argc, char* argv[])
{
  printf("Cruise for a corpse recode\n");

//  OSystemInit();
//  osystem = new OSystem;

  printf("Osystem Initialized\n");

  printf("Initializing engine...\n");
  
//  initBuffer(scaledScreen,640,400);

  fadeVar = 0;

  //lowLevelInit();

  // arg parser stuff

  ptr_something = (ctpVar19Struct*)mallocAndZero(sizeof(ctpVar19Struct)*0x200);

  /*volVar1 = 0;
  fileData1 = 0;*/

  /*PAL_fileHandle = -1;*/

  // video init stuff

  loadSystemFont();

  // another bit of video init

  if(!readVolCnf())
  {
    printf("Fatal: unable to load vol.cnf !\n");
    return(-1);
  }

  printf("Entering main loop...\n");
  mainLoop();

  //freeStuff();

  //freePtr(ptr_something);

  return(0);
}

void changeCursor(uint16 cursorType)
{
  //printf("changeCursor %d\n", cursorType);
}

void* mallocAndZero(int32 size)
{
  void* ptr;

  ptr = malloc(size);
  memset(ptr,0,size);
  return ptr;
}

} // End of namespace Cruise
