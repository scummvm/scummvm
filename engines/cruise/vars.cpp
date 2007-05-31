/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
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
 * $URL:https://scummvm.svn.sourceforge.net/svnroot/scummvm/scummvm/branches/gsoc2007-fsnode/engines/cruise/vars.cpp $
 * $Id:vars.cpp 26949 2007-05-26 20:23:24Z david_corrales $
 *
 */

#include "cruise/cruise_main.h"

namespace Cruise {

uint8 *_systemFNT;

uint8 video2 = 1;
uint8 video3 = 3;
uint8 video4 = 2;
uint8 colorOfSelectedSaveDrive = 5;

int16 initVar1;
int16 initVar2;
int16 initVar3;
uint8 initVar4[90];

int16 currentActiveBackgroundPlane;
int16 main5;
int16 var22 = 0;

filesDataStruct filesData[90];
filesData2Struct filesData2[90];

mediumVarStruct mediumVar[64];

volumeDataStruct volumeData[20];

int32 volumeDataLoaded = 0;

int16 numOfDisks;

uint8 scriptNameBuffer[15];
int16 currentActiveMenu;
int16 main14;
int16 mouseVar1;
int16 main21;
int16 main22;
int16 main7;
int16 main8;

int16 currentDiskNumber = 1;

#ifdef PALMOS_MODE
Common::File *_currentVolumeFile;
#else
Common::File currentVolumeFile;
#endif

int16 currentCursor;

int16 volumeNumEntry;
fileEntry *volumePtrToFileDescriptor = NULL;

uint32 volumeFileDescriptorSize;
int16 volumeSizeOfEntry;
int16 volumeNumberOfEntry;

int16 affichePasMenuJoueur = 1;

int16 globalVars[2000];

dataFileEntry filesDatabase[257];

int16 bootOverlayNumber;

int16 initVar5[12];

opcodeTypeFunction opcodeTypeTable[64];

int16 positionInStack;

actorStruct actorHead;

int16 setup1;

uint8 *currentData3DataPtr;
uint8 *scriptDataPtrTable[7];

int16 currentScriptOpcodeType;

int16 saveOpcodeVar;

int16 var30 = 0;
int16 var31 = 0;

int16 var1;
int16 var2;
int16 var3;
int16 var4;
int16 userEnabled;
int16 var5;
int16 var6;
int16 var7;
int16 var8;
int16 userDelay;
int16 sysKey = -1;
int16 var11 = 0;
int16 var12;
int16 var13;
int16 var14;
int16 var20;
int16 var23;
int16 var24;
int16 automaticMode;
int16 var34;
int16 var35;
bool animationStart;

int16 main17;
int16 var39;
int16 entrerMenuJoueur;
int16 var41;
int16 var42;
int16 var45;
int16 var46;
int16 var47;
int16 var48;
int16 flagCt;

int8 var50[64];
int16 palette[256 * 3];

systemStringsStruct systemStrings;

uint8 currentCtpName[40];

int16 saveVar1;
uint8 saveVar2[97];		// recheck size

int16 numberOfWalkboxes;	// saveVar3
int16 walkboxType[15];		// saveVar4
int16 walkboxChange[15];	// saveVar5

uint8 saveVar6[16];

int32 loadFileVar1;

int16 ctpVar1 = 0;
int16 ctp_routeCoordCount;	// ctpVar2
int16 ctp_routeCoords[20][2];	// ctpVar3
int16 ctp_routes[20][10];
uint16 ctp_walkboxTable[15 * 40];	// ctpVar5
int8 ctpVar6[32];
int16 ctp_scale[15];		//  ctpVar7
int16 ctpVar8[200];

int16 ctpVar14;

int16 bgVar1;
int16 bgVar2;
int16 bgVar3;

uint8 globalScreen[320 * 200];
uint8 scaledScreen[640 * 400];

//OSystem *osystem;

} // End of namespace Cruise
