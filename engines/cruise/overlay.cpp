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

overlayStruct overlayTable[90];
int numOfLoadedOverlay;

void initOverlayTable(void)
{
  int i;

  for(i=0;i<90;i++)
  {
    overlayTable[i].overlayName[0] = 0;
    overlayTable[i].ovlData = NULL;
    overlayTable[i].alreadyLoaded = 0;
    overlayTable[i].executeScripts = 0;
  }

  numOfLoadedOverlay = 1;
}

int loadOverlay(uint8* scriptName)
{
  int newNumberOfScript;
  bool scriptNotLoadedBefore;
  int scriptIdx;
  uint8 fileName[50];
  int fileIdx;
  int unpackedSize;
  char* unpackedBuffer;
  char* scriptPtr;
  ovlDataStruct* ovlData;

  printf("Load overlay: %s\n",scriptName);

  newNumberOfScript = numOfLoadedOverlay;

  scriptNotLoadedBefore = false;

  scriptIdx = findOverlayByName((char*)scriptName);

  if(scriptIdx == -4)
  {
    scriptIdx = numOfLoadedOverlay;

    newNumberOfScript++;

    scriptNotLoadedBefore = true;
  }

  if(overlayTable[scriptIdx].alreadyLoaded)
  {
    return (scriptIdx);
  }

  overlayTable[scriptIdx].ovlData = (ovlDataStruct*) mallocAndZero(sizeof(ovlDataStruct));

  if(!overlayTable[scriptIdx].ovlData)
    return(-2);

  strcpyuint8(overlayTable[scriptIdx].overlayName, scriptName);

  overlayTable[scriptIdx].alreadyLoaded = 1;

  numOfLoadedOverlay = newNumberOfScript;

  overlayTable[scriptIdx].ovlData->scriptNumber = scriptIdx;
  
  strcpyuint8(fileName,scriptName);

  strcatuint8(fileName,".OVL");

  printf("Attempting to load overlay file %s...\n", fileName);
  
  fileIdx = findFileInDisks(fileName);

  if(fileIdx<0)
  {
    printf("Unable to load overlay %s !\n", scriptName);
    //releaseScript(scriptName);
    return(-18);
  }

  unpackedSize = volumePtrToFileDescriptor[fileIdx].extSize + 2;
 
  // TODO: here, can unpack in gfx module buffer
  unpackedBuffer = (char*)mallocAndZero(unpackedSize);

  if(!unpackedBuffer)
  {
    return(-2);
  }

  if(volumePtrToFileDescriptor[fileIdx].size +2 != unpackedSize)
  {
    char* tempBuffer;
    uint16 realUnpackedSize;
    char* pakedBuffer = (char*) mallocAndZero(volumePtrToFileDescriptor[fileIdx].size +2);

    loadPakedFileToMem(fileIdx,(uint8*)pakedBuffer);

    realUnpackedSize = *(int16*)(pakedBuffer+volumePtrToFileDescriptor[fileIdx].size-2);
    flipShort(&realUnpackedSize);

    tempBuffer = (char*)mallocAndZero(realUnpackedSize);

    decomp((uint8*)pakedBuffer+volumePtrToFileDescriptor[fileIdx].size-4,(uint8*)unpackedBuffer+realUnpackedSize,realUnpackedSize);

    free(pakedBuffer);
  }
  else
  {
    loadPakedFileToMem(fileIdx,(uint8*)unpackedBuffer);
  }

  printf("OVL loading done...\n");

  scriptPtr = unpackedBuffer;

  ovlData = overlayTable[scriptIdx].ovlData;

  memcpy(ovlData,scriptPtr, sizeof(ovlDataStruct));

	ovlData->data3Table = NULL;
	ovlData->ptr1 = NULL;
	ovlData->objDataTable = NULL;
	ovlData->objData2SourceTable = NULL;
	ovlData->objData2WorkTable = NULL;
	ovlData->stringTable = NULL;
	ovlData->exportDataPtr = NULL;
	ovlData->importDataPtr = NULL;
	ovlData->linkDataPtr = NULL;
	ovlData->specialString1 = NULL;
	ovlData->specialString2 = NULL;
	ovlData->importNamePtr = NULL;
	ovlData->exportNamesPtr = NULL;
	ovlData->data4Ptr = NULL;
	ovlData->ptr8 = NULL;
	ovlData->numScripts1 = readB16(scriptPtr+60);
	ovlData->numScripts2 = readB16(scriptPtr+62);
	ovlData->numExport = readB16(scriptPtr+64);
	ovlData->numImport = readB16(scriptPtr+66);
	ovlData->numLinkData = readB16(scriptPtr+68);
	ovlData->numObjData = readB16(scriptPtr+70);
	ovlData->numStrings = readB16(scriptPtr+72);
	ovlData->size8 = readB16(scriptPtr+74);
	ovlData->size9 = readB16(scriptPtr+76);
	ovlData->nameExportSize = readB16(scriptPtr+78);
	ovlData->exportNamesSize = readB16(scriptPtr+80);
	ovlData->specialString2Length = readB16(scriptPtr+82);
	ovlData->sizeOfData4 = readB16(scriptPtr+84);
	ovlData->size12 = readB16(scriptPtr+86);
	ovlData->specialString1Length = readB16(scriptPtr+88);
	ovlData->scriptNumber = readB16(scriptPtr+90);

  scriptPtr += 92;

  if(ovlData->numExport) // export data
  {
		int i;
    ovlData->exportDataPtr = (exportEntryStruct*)mallocAndZero(ovlData->numExport*sizeof(exportEntryStruct));

    if(!ovlData->exportDataPtr)
    {
      return(-2);
    }

		for(i=0;i<ovlData->numExport;i++)
		{
			ovlData->exportDataPtr[i].var0 = readB16(scriptPtr);
			ovlData->exportDataPtr[i].var2 = readB16(scriptPtr+2);
			ovlData->exportDataPtr[i].var4 = readB16(scriptPtr+4);
			ovlData->exportDataPtr[i].idx = readB16(scriptPtr+6);
			ovlData->exportDataPtr[i].offsetToName = readB16(scriptPtr+8);

			scriptPtr+=10;
		}
  }
  
  if(ovlData->exportNamesSize) // export names
  {
    ovlData->exportNamesPtr = (uint8*)mallocAndZero(ovlData->exportNamesSize);

    if(!ovlData->exportNamesPtr)
    {
      return(-2);
    }

    memcpy(ovlData->exportNamesPtr, scriptPtr, ovlData->exportNamesSize);
    scriptPtr += ovlData->exportNamesSize;
  }

  if(ovlData->numImport) // import data
  {
		int i;

    ovlData->importDataPtr = (importDataStruct*)mallocAndZero(ovlData->numImport * sizeof(importDataStruct));

    if(!ovlData->importDataPtr)
    {
      return(-2);
    }

		for(i=0;i<ovlData->numImport;i++)
		{
			ovlData->importDataPtr[i].var0 = readB16(scriptPtr);
			ovlData->importDataPtr[i].var1 = readB16(scriptPtr+2);
			ovlData->importDataPtr[i].linkType = readB16(scriptPtr+4);
			ovlData->importDataPtr[i].linkIdx = readB16(scriptPtr+6);
			ovlData->importDataPtr[i].nameOffset = readB16(scriptPtr+8);

			scriptPtr+=10;
		}
	}
  
  if(ovlData->nameExportSize) // import name
  {
    ovlData->importNamePtr = (uint8*)mallocAndZero(ovlData->nameExportSize);

    if(!ovlData->importNamePtr)
    {
      return(-2);
    }

    memcpy(ovlData->importNamePtr, scriptPtr, ovlData->nameExportSize);
    scriptPtr += ovlData->nameExportSize;
  }

  if(ovlData->numLinkData) // link data
  {
    ASSERT(sizeof(linkDataStruct) == 0x22);

    ovlData->linkDataPtr = (linkDataStruct*)mallocAndZero(ovlData->numLinkData*sizeof(linkDataStruct));

    if(!ovlData->linkDataPtr)
    {
      return(-2);
    }

    memcpy(ovlData->linkDataPtr, scriptPtr, ovlData->numLinkData*sizeof(linkDataStruct));
    scriptPtr += ovlData->numLinkData*sizeof(linkDataStruct);
    flipGen(ovlData->linkDataPtr,ovlData->numLinkData*sizeof(linkDataStruct));
  }

  if(ovlData->numScripts1) // script
  {
    ovlData3Struct* tempPtr;
    int i;

    ovlData->data3Table = (ovlData3Struct*)mallocAndZero(ovlData->numScripts1 * sizeof(ovlData3Struct));

    if(!ovlData->data3Table)
    {
/*      releaseScript(scriptIdx,scriptName);

      if(freeIsNeeded)
      {
        freePtr(unpackedBuffer);
      } */

      return(-2);
    }

    memcpy(ovlData->data3Table, scriptPtr, ovlData->numScripts1 * sizeof(ovlData3Struct));
    scriptPtr += ovlData->numScripts1 * 0x1C;

    flipGen(ovlData->data3Table,ovlData->numScripts1 * sizeof(ovlData3Struct));

    tempPtr = ovlData->data3Table;

    for(i=0;i<ovlData->numScripts1;i++)
    {
      uint8* ptr = tempPtr->dataPtr = (uint8*) mallocAndZero(tempPtr->sizeOfData);

      if(!ptr)
      {
  /*      releaseScript(scriptIdx,scriptName);

        if(freeIsNeeded)
        {
          freePtr(unpackedBuffer);
        } */

        return(-2);
      }

      memcpy( ptr, scriptPtr, tempPtr->sizeOfData );
      scriptPtr+= tempPtr->sizeOfData;

      if(tempPtr->offsetToImportData)
      {
        flipGen(ptr+tempPtr->offsetToImportData,tempPtr->numImport*10);
      }

      if(tempPtr->offsetToSubData2)
      {
        flipGen(ptr+tempPtr->offsetToImportData,tempPtr->subData2Size*10);
      }

      tempPtr++;
    }
  }

  if(ovlData->numScripts2)
  {
    ovlData3Struct* tempPtr;
    int i;
    
    ovlData->ptr1 = (uint8*)mallocAndZero(ovlData->numScripts2*0x1C);

    if(!ovlData->ptr1)
    {
      return(-2);
    }

    memcpy(ovlData->ptr1, scriptPtr, ovlData->numScripts2 * 0x1C);
    scriptPtr += ovlData->numScripts2*0x1C;
    flipGen(ovlData->ptr1,ovlData->numScripts2*0x1C);

    tempPtr = (ovlData3Struct*)ovlData->ptr1;

    for(i=0;i<ovlData->numScripts2;i++)
    {
      uint8* ptr = tempPtr->dataPtr = (uint8*) mallocAndZero(tempPtr->sizeOfData);

      if(!ptr)
      {
  /*      releaseScript(scriptIdx,scriptName);

        if(freeIsNeeded)
        {
          freePtr(unpackedBuffer);
        } */

        return(-2);
      }

      memcpy( ptr, scriptPtr, tempPtr->sizeOfData );
      scriptPtr+= tempPtr->sizeOfData;

      if(tempPtr->offsetToImportData)
      {
        flipGen(ptr+tempPtr->offsetToImportData,tempPtr->numImport*10);
      }

      if(tempPtr->offsetToSubData2)
      {
        flipGen(ptr+tempPtr->offsetToImportData,tempPtr->subData2Size*10);
      }

      tempPtr++;
    }
  }

  if(ovlData->size12)
  {
    ovlData->ptr8 = (uint8*)mallocAndZero(ovlData->size12);

    if(!ovlData->ptr8)
    {
/*      releaseScript(scriptIdx,scriptName);

      if(freeIsNeeded)
      {
        freePtr(unpackedBuffer);
      } */

      return(-2);
    }

    memcpy(ovlData->ptr8, scriptPtr, ovlData->size12);
    scriptPtr += ovlData->size12;
  }

  if(ovlData->numObjData)
  {
		int i;
    ovlData->objDataTable = (objDataStruct*)mallocAndZero(ovlData->numObjData*sizeof(objDataStruct));

    if(!ovlData->objDataTable)
    {
/*      releaseScript(scriptIdx,scriptName);

      if(freeIsNeeded)
      {
        freePtr(unpackedBuffer);
      } */

      return(-2);
    }

		for(i=0;i<ovlData->numObjData;i++)
		{
			ovlData->objDataTable[i].var0 = *(int16*)scriptPtr;
			scriptPtr+=2;
			flipShort(&ovlData->objDataTable[i].var0);

			ovlData->objDataTable[i].var1 = *(int16*)scriptPtr;
			scriptPtr+=2;
			flipShort(&ovlData->objDataTable[i].var1);

			ovlData->objDataTable[i].var2 = *(int16*)scriptPtr;
			scriptPtr+=2;
			flipShort(&ovlData->objDataTable[i].var2);

			ovlData->objDataTable[i].var3 = *(int16*)scriptPtr;
			scriptPtr+=2;
			flipShort(&ovlData->objDataTable[i].var3);

			ovlData->objDataTable[i].var4 = *(int16*)scriptPtr;
			scriptPtr+=2;
			flipShort(&ovlData->objDataTable[i].var4);

			ovlData->objDataTable[i].var5 = *(int16*)scriptPtr;
			scriptPtr+=2;
			flipShort(&ovlData->objDataTable[i].var5);

			ovlData->objDataTable[i].var6 = *(int16*)scriptPtr;
			scriptPtr+=2;
			flipShort(&ovlData->objDataTable[i].var6);
		}

    if(scriptNotLoadedBefore)
    {
      //int var1;
      //int var2;
      
      overlayTable[scriptIdx].field_14 = (char)setup1;

      var1 = loadScriptSub1(scriptIdx,3);
      var2 = loadScriptSub1(scriptIdx,0);

      setup1 = var1 + var2;
    }
  }

  if(ovlData->size9)
  {
    ovlData->objData2WorkTable = (objectParams*)mallocAndZero(ovlData->size9 * sizeof(objectParams));
    memset(ovlData->objData2WorkTable, 0, ovlData->size9 * sizeof(objectParams));

    if(!ovlData->objData2WorkTable)
    {
/*      releaseScript(scriptIdx,scriptName);

      if(freeIsNeeded)
      {
        freePtr(unpackedBuffer);
      } */

      return(-2);
    }
  }

  if(ovlData->size8)
  {
    ovlData->objData2SourceTable = (objectParams*)mallocAndZero(ovlData->size8 * sizeof(objectParams));

    if(!ovlData->objData2SourceTable)
    {
/*      releaseScript(scriptIdx,scriptName);

      if(freeIsNeeded)
      {
        freePtr(unpackedBuffer);
      } */

      return(-2);
    }

    memcpy(ovlData->objData2SourceTable, scriptPtr, ovlData->size8*12); // TODO: made read item by item
    scriptPtr += ovlData->size8*12;
    flipGen(ovlData->objData2SourceTable,ovlData->size8*12);
  }

  if(ovlData->numStrings)
  {
    int i;
    
    ovlData->stringTable = (stringEntryStruct*)mallocAndZero(ovlData->numStrings*sizeof(stringEntryStruct));

    for(i=0;i<ovlData->numStrings;i++)
    {
      ovlData->stringTable[i].idx = *(int16*)scriptPtr;
      flipShort(&ovlData->stringTable[i].idx);
      scriptPtr+=2;
    }
  }

/*  if(freeIsNeeded)
  {
    freePtr(unpackedBuffer);
  } */

  if(ovlData->sizeOfData4)
  {
    ovlData->data4Ptr = (uint8*)mallocAndZero(ovlData->sizeOfData4);
    memset(ovlData->data4Ptr,0,ovlData->sizeOfData4);

    if(!ovlData->data4Ptr)
    {
      //releaseScript(scriptIdx,scriptName);
      return(-2);
    }
  }

  if(ovlData->specialString1Length /*|| ovlData->specialString2Length*/ || ovlData->stringTable)
  {
    int i;
    //int unpackedSize;
    //int fileIdx;
    //uint8 fileName[50];
    //char* unpackedBuffer;
    
    strcpyuint8(fileName,scriptName);

    strcatuint8(fileName,".FR");

    fileIdx = findFileInDisks(fileName);

    if(fileIdx<0)
    {
      //releaseScript(scriptName);
      return(-18);
    }

    unpackedSize = volumePtrToFileDescriptor[fileIdx].extSize + 2;

    // TODO: here, can unpack in gfx module buffer
    unpackedBuffer = (char*)mallocAndZero(unpackedSize);

    if(!unpackedBuffer)
    {
      return(-2);
    }

    if(volumePtrToFileDescriptor[fileIdx].size +2 != unpackedSize)
    {
      short int realUnpackedSize;
      char* pakedBuffer = (char*) mallocAndZero(volumePtrToFileDescriptor[fileIdx].size +2);

      loadPakedFileToMem(fileIdx,(uint8*)pakedBuffer);

      realUnpackedSize = *(int16*)(pakedBuffer+volumePtrToFileDescriptor[fileIdx].size-2);
      flipShort(&realUnpackedSize);

      decomp((uint8*)pakedBuffer+volumePtrToFileDescriptor[fileIdx].size-4,(uint8*)unpackedBuffer+realUnpackedSize,realUnpackedSize);

      free(pakedBuffer);
    }
    else
    {
      loadPakedFileToMem(fileIdx,(uint8*)unpackedBuffer);
    }

    scriptPtr = unpackedBuffer;

    memcpy(&ovlData->specialString1Length,scriptPtr,2);
    scriptPtr+=2;
    flipShort(&ovlData->specialString1Length); // recheck if needed

    if(ovlData->specialString1Length)
    {
      ovlData->specialString1 = (uint8*)mallocAndZero(ovlData->specialString1Length);

      if(!ovlData->specialString1)
      {
  /*      releaseScript(scriptIdx,scriptName);

        if(freeIsNeeded)
        {
          freePtr(unpackedBuffer);
        } */

        return(-2);
      }

      memcpy(ovlData->specialString1, scriptPtr, ovlData->specialString1Length);
      scriptPtr += ovlData->specialString1Length;
    }

    memcpy(&ovlData->specialString2Length,scriptPtr,2);
    scriptPtr+=2;
    flipShort(&ovlData->specialString2Length); // recheck if needed

    if(ovlData->specialString2Length)
    {
      ovlData->specialString2 = (uint8*)mallocAndZero(ovlData->specialString2Length);

      if(!ovlData->specialString2)
      {
  /*      releaseScript(scriptIdx,scriptName);

        if(freeIsNeeded)
        {
          freePtr(unpackedBuffer);
        } */

        return(-2);
      }

      memcpy(ovlData->specialString2, scriptPtr, ovlData->specialString2Length);
      scriptPtr += ovlData->specialString2Length;
    }

    for(i=0;i<ovlData->numStrings;i++)
    {
      ovlData->stringTable[i].length = *(int16*)scriptPtr;
      scriptPtr +=2;
      flipShort(&ovlData->stringTable[i].length);

      if(ovlData->stringTable[i].length)
      {
        ovlData->stringTable[i].string = (char*)mallocAndZero(ovlData->stringTable[i].length);

        if(!ovlData->stringTable[i].string)
        {
    /*      releaseScript(scriptIdx,scriptName);

          if(freeIsNeeded)
          {
            freePtr(unpackedBuffer);
          } */

          return(-2);
        }

        memcpy(ovlData->stringTable[i].string,scriptPtr,ovlData->stringTable[i].length);
        scriptPtr += ovlData->stringTable[i].length;
      }
    }
  }

#ifdef DUMP_SCRIPT
    {
        int i;
				for(i=0;i<ovlData->numScripts1;i++)
        {
            dumpScript(scriptName,ovlData,i);
        }
    }
#endif
#ifdef DUMP_OBJECT
    {
      int i;
      FILE* fHandle;
      char nameBundle[100];
      sprintf(nameBundle, "%s-objs.txt",scriptName);

      fHandle = fopen(nameBundle, "w+");
      ASSERT(fHandle);

      for(i=0;i<ovlData->numLinkData;i++)
      {
        linkDataStruct* var_34;
        var_34 = &ovlData->linkDataPtr[i];

        if(ovlData->specialString2)
        {
          fprintf(fHandle,"----- object %02d -----\n", i);
          if(var_34->stringNameOffset != 0xFFFF)
          {
            fprintf(fHandle,"name: %s\n",getObjectName(var_34->stringNameOffset, ovlData->specialString2));
          }
        }
      }

      fclose(fHandle);
    }
#endif

  return(scriptIdx);
}

int releaseOverlay(const char* name)
{
  int overlayIdx;
  ovlDataStruct* ovlDataPtr;
  
  overlayIdx = findOverlayByName(name);

  if(overlayIdx == -4)
    return -4;

  if(overlayTable[overlayIdx].alreadyLoaded == 0)
    return -4;

  overlayTable[overlayIdx].alreadyLoaded = 0;

  ovlDataPtr = overlayTable[overlayIdx].ovlData;

  if(!ovlDataPtr)
    return -4;
/*
  if(overlayTable[overlayIdx].var1E)
  {
    free(overlayTable[overlayIdx].var1E);
    overlayTable[overlayIdx].var1E = NULL;
  }

  if(overlayTable[overlayIdx].var16)
  {
    free(overlayTable[overlayIdx].var16);
    overlayTable[overlayIdx].var16 = NULL;
  } */

  removeScript(overlayIdx,-1,&scriptHandle2);
  removeScript(overlayIdx,-1,&scriptHandle2);

  removeScript(overlayIdx,-1,&scriptHandle1);
  removeScript(overlayIdx,-1,&scriptHandle1);

  printf("releaseOverlay: finish !\n");
  
  return 0;
}

int32 findOverlayByName2(uint8* name)
{
  int i;

  for(i=1;i<numOfLoadedOverlay;i++)
  {
    if(!strcmpuint8(overlayTable[i].overlayName,name))
      return(i);
  }

  return(-4);
}

int findOverlayByName(const char* overlayName)
{
  int i;

  for(i=1;i<numOfLoadedOverlay;i++)
  {
    if(!strcmp(overlayTable[i].overlayName,overlayName))
    {
      return(i);
    }
  }

  return(-4);
}


} // End of namespace Cruise
