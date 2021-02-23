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
extern struct SSound     GSample[];
extern struct SAnim      AnimTab[];

/*-------------------------------------------------------------------------*/
/*                                 SCRIPT           					   */
/*-------------------------------------------------------------------------*/
extern struct SScriptFrame ScriptFrame[];
extern struct SScript Script[];

/*-------------------------------------------------------------------------*/
/*                            VARIABILI ATTUALI          				   */
/*-------------------------------------------------------------------------*/
extern bool _characterInMovement;
extern int8 _characterGoToPosition;

/*-------------------------------------------------------------------------*/
/*                          VARIABILI DI SERVIZIO         				   */
/*-------------------------------------------------------------------------*/
extern int8 OldObjStatus[];
extern int8 VideoObjStatus[];

extern struct ScreenRect VideoTop;
extern struct ScreenRect VideoCent;
extern struct ScreenRect VideoBott;

extern struct SSortTable SortTable[];

/*-------------------------------------------------------------------------*/
/*                             FLAGS DI STATO          					   */
/*-------------------------------------------------------------------------*/
extern bool  Flagscriptactive;
extern bool  FlagDialogActive;
extern bool  FlagDialogMenuActive;
extern bool  Flagskiptalk;
extern bool  Flagskipenable;
extern bool  FlagMouseEnabled;
extern bool  FlagScreenRefreshed;
extern bool  FlagPaintCharacter;
extern bool  FlagShowCharacter;
extern bool  FlagSomeOneSpeak;
extern bool  FlagCharacterSpeak;
extern bool  FlagInventoryLocked;
extern bool  FlagUseWithStarted;
extern bool  FlagUseWithLocked;
extern bool  FlagMousePolling;
extern bool  FlagDialogSolitaire;
extern bool  FlagCharacterExist;
extern bool  FlagWalkNoInterrupt;
extern bool  FlagNoPaintScreen;
extern bool  FlagForceRegenInventory;
extern bool  FlagWaitRegen;

extern bool  FlagSaveInventory;
extern bool  FlagLoadInventory;

/*-------------------------------------------------------------------------*/
/*                                  VARIE           					   */
/*-------------------------------------------------------------------------*/
extern uint32 TheTime;
extern int16 mx, my;
extern bool mleft, mright;
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
