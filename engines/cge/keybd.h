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

/*
 * This code is based on original Soltys source code
 * Copyright (c) 1994-1995 Janus B. Wisniewski and L.K. Avalon
 */

#ifndef		__KEYBD__
#define		__KEYBD__

#include	"cge/jbw.h"
#include	"cge/vga13h.h"


#define		KEYBD_INT	9
#define		LSHIFT		42
#define		RSHIFT		54
#define		CTRL		29
#define		ALT		56


class KEYBOARD
{
  static void interrupt (far * OldKeyboard) (...);
  static void interrupt NewKeyboard (...);
  static word Code[0x60];
  static word Current;
  static SPRITE * Client;
public:
  static byte Key[0x60];
  static word Last (void) { _AX = Current; Current = 0; return _AX; }
  static SPRITE * SetClient (SPRITE * spr);
  KEYBOARD (void);
  ~KEYBOARD (void);
};


#endif
