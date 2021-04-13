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

#include "common/scummsys.h"
#include "trecision/nl/struct.h"

namespace Trecision {

/*-------------------------------------------------------------------------*/
/*                                  INIT                                   */
/*-------------------------------------------------------------------------*/
SSound GSample[MAXSAMPLE];

/*-------------------------------------------------------------------------*/
/*                            VARIABILI ATTUALI          				   */
/*-------------------------------------------------------------------------*/

bool _characterInMovement = false;
int8 _characterGoToPosition = -1;

/*-------------------------------------------------------------------------*/
/*                          VARIABILI DI SERVIZIO         				   */
/*-------------------------------------------------------------------------*/
bool OldObjStatus[MAXOBJINROOM];
bool VideoObjStatus[MAXOBJINROOM];

ScreenRect VideoCent = {0, TOP, MAXX - 1, (TOP + AREA) - 1};
ScreenRect VideoBott = {0, (TOP + AREA), MAXX - 1, MAXY - 1};

SSortTable SortTable[200];

/*-------------------------------------------------------------------------*/
/*                                  VARIE           					   */
/*-------------------------------------------------------------------------*/
uint32 TheTime;
uint32 CharacterSpeakTime;
uint32 SomeOneSpeakTime;

/*-------------------------------------------------------------------------*/
/*                                 DIALOGHI           					   */
/*-------------------------------------------------------------------------*/
Dialog _dialog[MAXDIALOG];
DialogSubTitle _subTitles[MAXSUBTITLES];

uint16  _curDialog, _curChoice, _curSubTitle;

} // End of namespace Trecision
