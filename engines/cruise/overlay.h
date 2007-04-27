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

#ifndef _OVERLAY_H_
#define _OVERLAY_H_

namespace Cruise {

struct importScriptStruct
{
  uint16 var0;
  uint16 var1;
  uint16 type;
  uint16 offset;
  uint16 offsetToName;
};

typedef struct importScriptStruct importScriptStruct;

struct exportEntryStruct
{
    uint16 var0;
    uint16 var2;
    uint16 var4;
    uint16 idx;
    uint16 offsetToName;
};

typedef struct exportEntryStruct exportEntryStruct;

struct ovlData3Struct
{
	uint8* dataPtr; //0
	short int sizeOfData; //4
	short int offsetToSubData3; //6
	short int offsetToImportData; //8
	short int offsetToSubData2;
	short int offsetToImportName;
	short int offsetToSubData5;
	short int sysKey;
	short int var12;
	short int numImport;
	short int subData2Size;
	short int var18;
	short int var1A;
};

typedef struct ovlData3Struct ovlData3Struct;

struct stringEntryStruct
{
	char* string;
	short int length;
	short int idx;
};

typedef struct stringEntryStruct stringEntryStruct;

struct linkDataStruct
{
  uint16 field_0;
  uint16 field_2;
  uint16 field_4;
  uint16 varIdx;
  uint16 varNameOffset;
  uint16 stringIdx;
  uint16 stringNameOffset;
  uint16 procIdx;
  uint16 procNameOffset;

  int16 field_12;
  int16 field_14;
  int16 field_16;
  int16 field_18;
  int16 field_1A;
  int16 field_1C;
  int16 field_1E;
  int16 field_20;
};

typedef struct linkDataStruct linkDataStruct;

struct importDataStruct
{
  uint16 var0; // 0
  uint16 var1; // 2
  uint16 linkType; // 4
  uint16 linkIdx; // 6
  uint16 nameOffset;
};

typedef struct importDataStruct importDataStruct;

struct objDataStruct
{
	int16 var0;
	int16 var1;
	int16 var2;
	int16 var3;
	int16 var4;
	int16 var5;
	int16 var6;
};

typedef struct objDataStruct objDataStruct;

struct objectParams
{
	int16 X;
	int16 Y;
	int16 baseFileIdx;
	int16 var3;
	int16 scale;
	int16 var5;
};

typedef struct objectParams objectParams;

struct ovlDataStruct
{
	ovlData3Struct* data3Table;
	uint8* ptr1;
	objDataStruct* objDataTable;
	objectParams* objData2SourceTable;
	objectParams* objData2WorkTable;
	stringEntryStruct* stringTable;
	exportEntryStruct* exportDataPtr;
	importDataStruct* importDataPtr;
	linkDataStruct* linkDataPtr;
	uint8* specialString1;
	uint8* specialString2;
	uint8* importNamePtr;
	uint8* exportNamesPtr;
	uint8* data4Ptr;
	uint8* ptr8;
	unsigned short int numScripts1;
	unsigned short int numScripts2;
	unsigned short int numExport;
	unsigned short int numImport;
	unsigned short int numLinkData;
	unsigned short int numObjData;
	unsigned short int numStrings;
	unsigned short int size8;
	unsigned short int size9;
	unsigned short int nameExportSize;
	unsigned short int exportNamesSize;
	unsigned short int specialString2Length;
	unsigned short int sizeOfData4;
	unsigned short int size12;
	unsigned short int specialString1Length;
	unsigned short int scriptNumber;
};

typedef struct ovlDataStruct ovlDataStruct;

struct overlayStruct
{
	char overlayName[14];
	ovlDataStruct* ovlData;
	short int alreadyLoaded;
	char field_14;
	char field_15;
	char field_16;
	char field_17;
	char field_18;
	char field_19;
	char field_1A;
	char field_1B;
	char field_1C;
	char field_1D;
	char field_1E;
	char field_1F;
	char field_20;
	char field_21;
	char field_22;
	char field_23;
	char field_24;
	char field_25;
	short int executeScripts;
};

typedef struct overlayStruct overlayStruct;

extern overlayStruct overlayTable[90];
extern int numOfLoadedOverlay;

void initOverlayTable(void);
int loadOverlay(uint8* scriptName);
int32 findOverlayByName2(uint8* name);
int findOverlayByName(const char* overlayName);
int releaseOverlay(const char* name);


} // End of namespace Cruise

#endif
