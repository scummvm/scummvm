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

#include    "cge/keybd.h"
#include    "cge/mouse.h"

namespace CGE {

Sprite *KEYBOARD::Client = NULL;
uint8   KEYBOARD::Key[0x60] = { 0 };
uint16  KEYBOARD::Current = 0;
uint16  KEYBOARD::Code[0x60] = {
	0,               Esc,      '1',         '2',       '3',
	'4',             '5',      '6',         '7',       '8',
	'9',             '0',      '-',         '+',       BSp,
	Tab,             'Q',      'W',         'E',       'R',
	'T',             'Y',      'U',         'I',       'O',
	'P',             '[',      ']',         Enter,     0/*Ctrl*/,
	'A',             'S',      'D',         'F',       'G',
	'H',             'J',      'K',         'L',       ';',
	'\'',            '`',      0/*LShift*/, '\\',      'Z',
	'X',             'C',      'V',         'B',       'N',
	'M',             ',',      '.',         '/',       0/*RShift*/,
	'*',             0/*Alt*/, ' ',         0/*Caps*/, F1,
	F2,              F3,       F4,          F5,        F6,
	F7,              F8,       F9,          F10,       0/*NumLock*/,
	0/*ScrollLock*/, Home,     Up,          PgUp,      '-',
	Left,            Ctr,      Right,       '+',       End,
	Down,            PgDn,     Ins,         Del,       0 * 0x54,
	0 * 0x55,        0 * 0x56, F11,         F12,       0 * 0x59,
	0 * 0x5A,        0 * 0x5B, 0 * 0x5C,    0 * 0x5D,  0 * 0x5E,
	0 * 0x5F
};

void (* KEYBOARD::OldKeyboard)(...);


KEYBOARD::KEYBOARD(void) {
	// steal keyboard interrupt
	/* TODO replace totally by scummvm handling
	OldKeyboard = getvect(KEYBD_INT);
	setvect(KEYBD_INT, NewKeyboard);
	*/
	warning("STUB: KEYBOARD::KEYBOARD");
}


KEYBOARD::~KEYBOARD(void) {
	// bring back keyboard interrupt
	/* TODO replace totally by scummvm handling
	setvect(KEYBD_INT, OldKeyboard);
	*/
	warning("STUB: KEYBOARD::~KEYBOARD");
}


Sprite *KEYBOARD::SetClient(Sprite *spr) {
	Swap(Client, spr);
	return spr;
}


void KEYBOARD::NewKeyboard(...) {
	// table address
	/*
	_SI = (uint16) Key;

	// take keyboard code
	asm    in  al,60h
	asm    mov bl,al
	asm    and bx,007Fh
	asm    cmp bl,60h
	asm    jae xit
	asm    cmp al,bl
	asm    je  ok      // key pressed

	// key released...
	asm    cmp [si+bx],bh  // BH == 0
	asm    jne ok
	// ...but not pressed: call the original service
	OldKeyboard();
	return;

	ok:
	asm    shl ax,1
	asm    and ah,1
	asm    xor ah,1
	asm    mov [si+bx],ah
	asm    jz  xit     // released: exit

	// pressed: lock ASCII code
	_SI = (uint16) Code;
	asm    add bx,bx       // uint16 size
	asm    mov ax,[si+bx]
	asm    or  ax,ax
	asm    jz  xit     // zero means NO KEY
	Current = _AX;

	_SI = (uint16) Client;
	asm    or  si,si
	asm    jz  xit             // if (Client) ...
	//--- fill current event entry with mask, key code and sprite
	asm    mov bx,EvtHead          // take queue head pointer
	asm    inc byte ptr EvtHead        // update queue head pointer
	asm    shl bx,3                // * 8
	_AX = Current;
	asm    mov Evt[bx].(struct EVENT)X,ax  // key code
	asm    mov ax,KEYB             // event mask
	asm    mov Evt[bx].(struct EVENT)Msk,ax    // event mask
	//asm  mov Evt[bx].(struct EVENT)Y,dx  // row
	asm    mov Evt[bx].(struct EVENT)Ptr,si    // SPRITE pointer

	xit:

	asm    in  al,61h      // kbd control lines
	asm    push    ax      // save it
	asm    or  al,80h      // set the "enable kbd" bit
	asm    out 61h,al      // and write it out
	asm    pop ax      // original control port value
	asm    out 61h,al      // write it back
	asm    mov al,20h      // send End-Of-Interrupt
	asm    out 20h,al      // to the 8259 IC
	*/
	warning("STUB: KEYBOARD::NewKeyboard");
}

} // End of namespace CGE
