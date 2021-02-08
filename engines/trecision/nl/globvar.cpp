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
 */

#include "trecision/nl/lib/addtype.h"
#include "trecision/nl/sysdef.h"
#include "trecision/nl/struct.h"
#include "trecision/nl/message.h"
#include "trecision/nl/extern.h"

namespace Trecision {

/*-------------------------------------------------------------------------*/
/*                                  INIT                                   */
/*-------------------------------------------------------------------------*/
struct SInvObject InvObj[MAXINVENTORY];
struct SSound     GSample[MAXSAMPLE];
struct SAnim      AnimTab[MAXANIM];

uint8 _actionLen[MAXACTION];

const char *ObjName[MAXOBJNAME];
const char *Sentence[MAXSENTENCE];

/*-------------------------------------------------------------------------*/
/*                              MESSY SYSTEM          					   */
/*-------------------------------------------------------------------------*/
message GameMessage[MAXMESSAGE];
message HomoMessage[MAXMESSAGE];
message AnimMessage[MAXMESSAGE];

message idlemessage  = {MC_IDLE, 0, MP_DEFAULT, 0, 0, 0, 0, 0};
message quitgamemessage = {MC_SYSTEM, ME_QUIT, MP_SYSTEM, 0, 0, 0, 0, 0};
message supereventmessage;

message *TheMessage;

pqueue Game;
pqueue Anim;
pqueue Homo;

/*-------------------------------------------------------------------------*/
/*                                 SCRIPT           					   */
/*-------------------------------------------------------------------------*/
struct SScriptFrame ScriptFrame[MAXSCRIPTFRAME];
struct SScript Script[MAXSCRIPT];

/*-------------------------------------------------------------------------*/
/*                            VARIABILI ATTUALI          				   */
/*-------------------------------------------------------------------------*/


int8 _characterInMovement = false;
int8 _characterGoToPosition = -1;

/*-------------------------------------------------------------------------*/
/*                          VARIABILI DI SERVIZIO         				   */
/*-------------------------------------------------------------------------*/
int8 OldObjStatus[MAXOBJINROOM];
int8 VideoObjStatus[MAXOBJINROOM];

struct screenrect VideoTop = {  0,  0,   0, TOP - 1};
struct screenrect VideoCent = {  0, TOP, 639, (TOP + AREA) - 1};
struct screenrect VideoBott = {  0, (TOP + AREA), 639, 479};

struct SSortTable SortTable[200];

/*-------------------------------------------------------------------------*/
/*                             FLAGS DI STATO          					   */
/*-------------------------------------------------------------------------*/
LLBOOL  Semscriptactive;
LLBOOL  SemDialogActive;
LLBOOL  SemDialogMenuActive;
LLBOOL  Semskiptalk;
LLBOOL  Semskipenable;
LLBOOL  SemMouseEnabled = true;
LLBOOL  SemScreenRefreshed;
LLBOOL  SemPaintHomo;
LLBOOL  SemShowHomo = 1;
LLBOOL  SemSomeOneSpeak;
LLBOOL  SemCharacterSpeak;
LLBOOL  SemInventoryLocked;
LLBOOL  SemUseWithStarted;
LLBOOL  SemUseWithLocked;
LLBOOL  SemMousePolling;
LLBOOL  SemDialogSolitaire;
LLBOOL  SemCharacterExist = true;
LLBOOL  SemWalkNoInterrupt;
LLBOOL  SemNoPaintScreen;
LLBOOL  SemForceRegenInventory;
LLBOOL  SemWaitRegen;

LLBOOL  SemSaveInventory;
LLBOOL  SemLoadInventory;

/*-------------------------------------------------------------------------*/
/*                               Inventory           					   */
/*-------------------------------------------------------------------------*/
uint8 _inventory[MAXICON];
uint8 _inventorySize;
uint8 _cyberInventory[MAXICON];
uint8 _cyberInventorySize;
uint8 TheIconBase;
uint8 _inventoryStatus = INV_OFF;
uint8 LightIcon = 0xFF;
uint8 RegenInvStartIcon;
uint8 RegenInvStartLine = INVENTORY_HIDE;
uint16 _lastCurInventory;
uint16 _lastLightIcon = 0xFF;
int16  _inventoryCounter = INVENTORY_HIDE;

/*-------------------------------------------------------------------------*/
/*                                 USACON           					   */
/*-------------------------------------------------------------------------*/
uint16 UseWith[2];
LLBOOL  UseWithInv[2];

/*-------------------------------------------------------------------------*/
/*                                  VARIE           					   */
/*-------------------------------------------------------------------------*/
uint32 TheTime;
int16  mx, my, mleft, mright;
uint32 OldTime;
uint32 CharacterSpeakTime;
uint32 SomeOneSpeakTime;

/*-------------------------------------------------------------------------*/
/*                                 DIALOGHI           					   */
/*-------------------------------------------------------------------------*/
Dialog _dialog[MAXDIALOG];
DialogChoice  _choice [MAXCHOICE];
DialogSubTitle _subTitles[MAXSUBTITLES];

uint16  _curDialog, _curChoice, _curSubTitle;

} // End of namespace Trecision
