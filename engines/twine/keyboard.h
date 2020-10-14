/** @file keyboard.h
	@brief
	This file contains movies routines

	TwinEngine: a Little Big Adventure engine

	Copyright (C) 2013 The TwinEngine team
	Copyright (C) 2008-2013 Prequengine team
	Copyright (C) 2002-2007 The TwinEngine team

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation; either version 2
	of the License, or (at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#ifndef KEYBOARD_H
#define KEYBOARD_H

#include "sys.h"

/** Pressed key map - scanCodeTab1 */
extern uint8 pressedKeyMap[29];
/** Pressed key char map - scanCodeTab2 */
extern uint16 pressedKeyCharMap[31];

/** Skipped key - key1 */
int16 skippedKey;
/** Pressed key - printTextVar12 */
int16 pressedKey;
//int printTextVar13;
/** Skip intro variable */
int16 skipIntro;
/** Current key value */
int16 currentKey;
/** Auxiliar key value */
int16 key;

int32 heroPressedKey;
int32 heroPressedKey2;

#endif
