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

#ifndef TRECISION_EXTERN_H
#define TRECISION_EXTERN_H

#include "trecision/nl/struct.h"

namespace Trecision {

/*-------------------------------------------------------------------------*/
/*                                  INIT                                   */
/*-------------------------------------------------------------------------*/
extern struct SSound     GSample[];

/*-------------------------------------------------------------------------*/
/*                            VARIABILI ATTUALI          				   */
/*-------------------------------------------------------------------------*/
extern bool _characterInMovement;
extern int8 _characterGoToPosition;

/*-------------------------------------------------------------------------*/
/*                          VARIABILI DI SERVIZIO         				   */
/*-------------------------------------------------------------------------*/
extern bool OldObjStatus[];
extern bool VideoObjStatus[];

extern struct ScreenRect VideoTop;
extern struct ScreenRect VideoCent;
extern struct ScreenRect VideoBott;
extern struct SSortTable SortTable[];

/*-------------------------------------------------------------------------*/
/*                                  VARIE           					   */
/*-------------------------------------------------------------------------*/
extern uint32 TheTime;
extern uint32 OldTime;
extern uint32 CharacterSpeakTime;
extern uint32 SomeOneSpeakTime;

/*-------------------------------------------------------------------------*/
/*                                 DIALOGHI           					   */
/*-------------------------------------------------------------------------*/
extern Dialog _dialog[];
extern DialogSubTitle _subTitles[];

extern uint16   _curDialog, _curChoice, _curSubTitle;
} // End of namespace Trecision
#endif
