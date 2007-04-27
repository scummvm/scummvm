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

#ifndef _VARS_H_
#define _VARS_H_

#include "common/file.h"

namespace Cruise {

struct menuElementSubStruct
{
  struct menuElementSubStruct* pNext;
  int16 var2;
  int16 var4;
};

typedef struct menuElementSubStruct menuElementSubStruct;

struct menuElementStruct
{
  struct menuElementStruct* next;
  const char* string;
  int x;
  int y;
  int varA;
  int varC;
  unsigned char color;
  gfxEntryStruct* gfx;
  menuElementSubStruct* ptrSub;
};

typedef struct menuElementStruct menuElementStruct;

typedef int32(*opcodeTypeFunction)(void);
typedef int16(*opcodeFunction)(void);


extern uint8* _systemFNT;
extern int16 fontFileIndex;

extern uint8 video2;
extern uint8 video3;
extern uint8 video4;
extern uint8 colorOfSelectedSaveDrive;

extern int16 initVar1;
extern int16 initVar2;
extern int16 initVar3;
extern uint8 initVar4[90];

extern int16 currentActiveBackgroundPlane;
extern int16 main5;
extern int16 var22;


struct mediumVarStruct
{
	uint8 name[16];
	int16 field_10;
	int16 field_12;
	int16 field_14;
	int16 field_16;
	uint8* ptr;
	int16 field_1C;
	int16 field_1E;
	int16 field_20;
};

typedef struct mediumVarStruct mediumVarStruct;

struct filesDataStruct
{
	uint8* field_0;
	uint8* field_4;
};

typedef struct filesDataStruct filesDataStruct;

struct filesData2Struct
{
	int16 field_0;
	int16 field_2;
};

typedef struct filesData2Struct filesData2Struct;

struct fileName
{
	uint8 name[13];
};

typedef struct fileName fileName;

struct setHeaderEntry
{
	int16 field_0;        // offset ptr part 1
	int16 field_2;        // offset ptr part 2
	int16 width;
	int16 height;
	int16 type;           // resource type, ie. sprites 0,1,4,5 and 8
	int16 transparency;
	int16 field_C;
	int16 field_E;
};

typedef struct setHeaderEntry setHeaderEntry;

struct volumeDataStruct
{
	char ident[10];
	fileName* ptr;
	int16 diskNumber;
	int32 size;
};

typedef struct volumeDataStruct volumeDataStruct;

struct fileEntry
{
	uint8 name[14];
	int32 offset;
	int32 size;
	int32 extSize;
	int32 unk3;				// unused
};

typedef struct fileEntry fileEntry;

struct dataFileEntrySub
{
	uint8* ptr;
	int16 index;					// sprite index
	char  name[14];
	int16 transparency;			// sprite transparency
	uint8* ptr2;
	uint8  resourceType;			// sprite and image type 2,4,8 , fnt = 7, spl = 6
	uint8  field_1B;
	int16 field_1C;
};

typedef struct dataFileEntrySub dataFileEntrySub;

struct dataFileEntry
{
	int16 widthInColumn;
	int16 width;
	int16 resType;
	int16 height;
	dataFileEntrySub subData;
};

typedef struct dataFileEntry dataFileEntry;

struct systemStringsStruct
{
	int8 param;
	uint8 string[12];
	uint8 bootScriptName[8];
};

typedef struct systemStringsStruct systemStringsStruct;

extern filesDataStruct filesData[90];
extern filesData2Struct filesData2[90];

extern mediumVarStruct mediumVar[64];

extern volumeDataStruct volumeData[20];

extern int32 volumeDataLoaded;

extern int16 numOfDisks;

extern uint8 scriptNameBuffer[15];
extern int16 currentActiveMenu;
extern int16 main14;
extern int16 mouseVar1;
extern int16 main21;
extern int16 main22;
extern int16 main7;
extern int16 main8;


extern int16 currentDiskNumber;

extern Common::File currentVolumeFile;

extern int16 currentCursor;

extern int16 volumeNumEntry;
extern fileEntry* volumePtrToFileDescriptor;

extern uint32 volumeFileDescriptorSize;
extern int16 volumeSizeOfEntry;
extern int16 volumeNumberOfEntry;

extern int16 affichePasMenuJoueur;

extern int16 globalVars[2000];
extern dataFileEntry filesDatabase[257];

extern int16 bootOverlayNumber;

extern int16 initVar5[12];

extern objectStruct objectHead;

extern opcodeTypeFunction opcodeTypeTable[64];

extern int16 positionInStack;
extern actorStruct actorHead;

extern int16 setup1;

extern uint8* currentData3DataPtr;
extern uint8* scriptDataPtrTable[7];

extern int16 currentScriptOpcodeType;

extern int16 saveOpcodeVar;

extern int16 var30;
extern int16 var31;

extern int16 var1;
extern int16 var2;
extern int16 var3;
extern int16 var4;
extern int16 userEnabled;
extern int16 var5;
extern int16 var6;
extern int16 var7;
extern int16 var8;
extern int16 userDelay;
extern int16 sysKey;
extern int16 var11;
extern int16 var12;
extern int16 var13;
extern int16 var14;
extern int16 var20;
extern int16 var23;
extern int16 var24;
extern int16 automaticMode;
extern int16 var34;
extern int16 var35;
extern int16 animationStart;

extern int16 main17;
extern int16 var39;
extern int16 entrerMenuJoueur;
extern int16 var39;
extern int16 var41;
extern int16 var42;
extern int16 var45;
extern int16 var46;
extern int16 var47;
extern int16 var48;
extern int16 flagCt;

extern int8 var50[64];
extern int16 palette[256*3];

extern systemStringsStruct systemStrings;

extern uint8 currentCtpName[40];

extern int16 saveVar1;
extern uint8 saveVar2[97]; // recheck size

extern int16 numberOfWalkboxes;		// saveVar3
extern int16 walkboxType[15];			// saveVar4	// Type: 0x00 - non walkable, 0x01 - walkable, 0x02 - exit zone
extern int16 walkboxChange[15];		// saveVar5 // walkbox can change its type: 0x00 - not changeable, 0x01 - changeable
												// Assumption: To change the type: walkboxType[i] -= walkboxChane[i] and vice versa
extern uint8 saveVar6[16];

extern int32 loadFileVar1;

extern int16 ctpVar1;
extern int16 ctp_routeCoordCount;			// ctpVar2  // number of path-finding coordinates
extern int16 ctp_routeCoords[20][2];			// ctpVar3	// path-finding coordinates array

/* ctp_routeCoords:

	correct size would be: ctp_routes[routeCoordCount * 4]
	coordinate information with x (2 bytes) and y (2 bytes)
*/

extern int16  ctp_routes[20][10]; // path-finding line information

/* ctp_routes:
   
   correct size would be: ctp_routes[routeCoordCount * 20 * 2]
   array is seperate in 20 * 2 bytes slices.
   first 2 bytes of the slice indicate how many coordinates/lines are following (lineCount)
   after that there are lineCount * 2 bytes following with indexes pointing on the routeCoords table
   the root x,y for the lines is the coordinate in the routeCoords array, which fits to the current slice
   for the 20 * i slice the root x,y is routeCoords[i], routeCoords[i+2]
   the unused rest of the slice if filled up with 0xFF
*/
extern uint16 ctp_walkboxTable[15 * 40];	// ctpVar5	// walkboxes coordinates and lines
extern int8  ctpVar6[32];
extern int16 ctp_scale[15];				// ctpVar7  // scaling information for walkboxes
extern int16 ctpVar8[200];

extern int16 ctpVar14;

extern int16 bgVar1;
extern int16 bgVar2;
extern int16 bgVar3;

extern uint8 globalScreen[320*200];
extern uint8 scaledScreen[640*400];

//extern OSystem *osystem;

} // End of namespace Cruise

#endif
