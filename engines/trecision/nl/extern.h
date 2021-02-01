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

#include "proto.h"

namespace Trecision {

/*-------------------------------------------------------------------------*/
/*                                  INIT                                   */
/*-------------------------------------------------------------------------*/
extern struct SRoom      Room[];
extern struct SObject    _obj[];
extern struct SInvObject InvObj[];
extern struct SSound     GSample[];
extern struct SAnim      AnimTab[];

extern uint8 _actionLen[];

extern const char *ObjName[];
extern const char *Sentence[];
extern const char *_sysSent[];

/*-------------------------------------------------------------------------*/
/*                              MESSY SYSTEM          					   */
/*-------------------------------------------------------------------------*/
extern message GameMessage[];
extern message HomoMessage[];
extern message AnimMessage[];

extern message idlemessage;
extern message quitgamemessage;
extern message supereventmessage;

extern message *TheMessage;

extern LLBOOL SuperEventActivate;

extern pqueue Game;
extern pqueue Anim;
extern pqueue Homo;

/*-------------------------------------------------------------------------*/
/*                                 SCRIPT           					   */
/*-------------------------------------------------------------------------*/
extern struct SScriptFrame ScriptFrame[];
extern struct SScript Script[];

/*-------------------------------------------------------------------------*/
/*                            VARIABILI ATTUALI          				   */
/*-------------------------------------------------------------------------*/
extern uint16 _curRoom;
extern uint16 OldRoom;

extern uint16 _curObj;
extern uint16 CurInventory;
extern int32  CurSortTableNum;
extern uint16 CurScriptFrame[];

extern int8 _homoInMovement;
extern int8 _homoGoToPosition;

/*-------------------------------------------------------------------------*/
/*                          VARIABILI DI SERVIZIO         				   */
/*-------------------------------------------------------------------------*/
extern int8 OldObjStatus[];
extern int8 VideoObjStatus[];

extern struct screenrect VideoTop;
extern struct screenrect VideoCent;
extern struct screenrect VideoBott;

extern struct SSortTable SortTable[];

/*-------------------------------------------------------------------------*/
/*                             FLAGS DI STATO          					   */
/*-------------------------------------------------------------------------*/
extern LLBOOL  Semscriptactive;
extern LLBOOL  SemDialogActive;
extern LLBOOL  SemDialogMenuActive;
extern LLBOOL  Semskiptalk;
extern LLBOOL  Semskipenable;
extern LLBOOL  SemMouseEnabled;
extern LLBOOL  SemScreenRefreshed;
extern LLBOOL  SemPaintHomo;
extern LLBOOL  SemShowHomo;
extern LLBOOL  SemSomeOneSpeak;
extern LLBOOL  SemCharacterSpeak;
extern LLBOOL  SemInventoryLocked;
extern LLBOOL  SemUseWithStarted;
extern LLBOOL  SemUseWithLocked;
extern LLBOOL  SemMousePolling;
extern LLBOOL  SemDialogSolitaire;
extern LLBOOL  SemCharacterExist;
extern LLBOOL  SemWalkNoInterrupt;
extern LLBOOL  SemNoPaintScreen;
extern LLBOOL  SemForceRegenInventory;
extern LLBOOL  SemWaitRegen;

extern LLBOOL  SemSaveInventory;
extern LLBOOL  SemLoadInventory;
/*-------------------------------------------------------------------------*/
/*                               Inventory           					   */
/*-------------------------------------------------------------------------*/
extern uint8 _inventory[];
extern uint8 _inventorySize;
extern uint8 _cyberInventory[];
extern uint8 _cyberInventorySize;
extern uint8 TheIconBase;
extern uint8 _inventoryStatus;
extern uint8 LightIcon;
extern uint8 RegenInvStartIcon;
extern uint8 RegenInvStartLine;
extern uint16 _lastCurInventory;
extern uint16 _lastLightIcon;
extern int16  _inventoryCounter;

/*-------------------------------------------------------------------------*/
/*                                 USACON           					   */
/*-------------------------------------------------------------------------*/
extern uint16 UseWith[];
extern LLBOOL  UseWithInv[];

/*-------------------------------------------------------------------------*/
/*                                  VARIE           					   */
/*-------------------------------------------------------------------------*/
extern uint32 TheTime;
extern int16  mx, my, mleft, mright;
extern uint32 OldTime;
extern uint32 CharacterSpeakTime;
extern uint32 SomeOneSpeakTime;

/*-------------------------------------------------------------------------*/
/*                                 DIALOGHI           					   */
/*-------------------------------------------------------------------------*/
extern Dialog _dialog[];
extern DialogChoice  _choice [];
extern DialogSubTitle _subTitles[];

extern char   *dScelta [];
extern char   *dBattuta[];

extern uint16   _curDialog, _curChoice, _curSubTitle;

} // End of namespace Trecision
