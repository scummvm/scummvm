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

#include "cge/events.h"
#include "cge/events.h"
#include "cge/text.h"
#include "cge/cge_main.h"

namespace CGE {

/*----------------- KEYBOARD interface -----------------*/

Sprite *Keyboard::_client = NULL;
uint8   Keyboard::_key[0x60] = { 0 };
uint16  Keyboard::_current = 0;
uint16  Keyboard::_code[0x60] = {
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

void (* Keyboard::OldKeyboard)(...);


Keyboard::Keyboard() {
	// steal keyboard interrupt
	/* TODO replace totally by scummvm handling
	OldKeyboard = getvect(KEYBD_INT);
	setvect(KEYBD_INT, NewKeyboard);
	*/
	warning("STUB: Keyboard::Keyboard");
}


Keyboard::~Keyboard() {
	// bring back keyboard interrupt
	/* TODO replace totally by scummvm handling
	setvect(KEYBD_INT, OldKeyboard);
	*/
	// FIXME: STUB: KEYBOARD::~KEYBOARD
}


Sprite *Keyboard::setClient(Sprite *spr) {
	Swap(_client, spr);
	return spr;
}


void Keyboard::NewKeyboard(...) {
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
	warning("STUB: Keyboard::NewKeyboard");
}

/*----------------- MOUSE interface -----------------*/

Event Evt[EVT_MAX];

uint16 EvtHead = 0, EvtTail = 0;

MOUSE_FUN *MOUSE::OldMouseFun  = NULL;
uint16     MOUSE::OldMouseMask = 0;


MOUSE::MOUSE(CGEEngine *vm, Bitmap **shpl) : Sprite(vm, shpl), Busy(NULL), Hold(NULL), hx(0), _vm(vm) {
	static Seq ms[] = {
		{ 0, 0, 0, 0, 1 },
		{ 1, 1, 0, 0, 1 }
	};

	setSeq(ms);

	/* TODO Mouse handling
	// Mouse reset
	_AX = 0x0000; // soft & hard reset (0x0021 soft reset does not work)
	__int__(0x33);
	Exist = (_AX != 0);
	Buttons = _BX;
*/
	gotoxy(SCR_WID/2, SCR_HIG/2);
	_z = 127;
	step(1);

	Exist = true;
	warning("STUB: MOUSE::MOUSE");
}


MOUSE::~MOUSE(void) {
	Off();
}


//void MOUSE::SetFun (void)
//{
//}


void MOUSE::On(void) {
	/*
	  if (SeqPtr && Exist)
	    {
	      _CX = X + X;  // horizontal position
	      _DX = Y;      // vertical position
	      _AX = 0x0004; // Set Mouse Position
	      __int__(0x33);
	      // set new mouse fun
	      _ES = FP_SEG(NewMouseFun);
	      _DX = FP_OFF(NewMouseFun);
	      _CX = 0x001F; // 11111b = all events
	      _AX = 0x0014; // Swap User-Interrupt Vector
	      __int__(0x33);
	      // save old mouse fun
	      OldMouseMask = _CX;
	      OldMouseFun = (MOUSE_FUN *) MK_FP(_ES, _DX);

	      // set X bounds
	      _DX = (SCR_WID - W) * 2;      // right limit
	      _CX = 0;              // left limit
	      _AX = 0x0007;         // note: each pixel = 2
	      __int__(0x33);

	      // set Y bounds
	      _DX = SCR_HIG - H;        // bottom limit
	      _CX = 0;              // top limit
	      _AX = 0x0008;
	      __int__(0x33);

	      Step(0);
	      if (Busy) Busy->Step(0);
	    }
	*/
	warning("STUB: MOUSE::On");
}


void MOUSE::Off(void) {
/*
	if (SeqPtr == 0)
	{
	  if (Exist)
	{
	  // bring back old mouse fun
	  _ES = FP_SEG(OldMouseFun);
	  _DX = FP_OFF(OldMouseFun);
	  _CX = OldMouseMask;
	  _AX = 0x0014;     // Swap User-Interrupt Vector
	  __int__(0x33);
	}
	  Step(1);
	  if (Busy) Busy->Step(1);
	}
	*/
	warning("STUB: MOUSE::Off");
}


void MOUSE::ClrEvt(Sprite *spr) {
	if (spr) {
		uint16 e;
		for (e = EvtTail; e != EvtHead; e = (e + 1) % EVT_MAX)
			if (Evt[e]._ptr == spr)
				Evt[e]._msk = 0;
	} else
		EvtTail = EvtHead;
}


void MOUSE::Tick(void) {
	step();
	while (EvtTail != EvtHead) {
		Event e = Evt[EvtTail];
		if (e._msk) {
			if (Hold && e._ptr != Hold)
				Hold->touch(e._msk | ATTN, e._x - Hold->_x, e._y - Hold->_y);

			// update mouse cursor position
			if (e._msk & ROLL)
				gotoxy(e._x, e._y);

			// activate current touched SPRITE
			if (e._ptr) {
				if (e._msk & KEYB)
					e._ptr->touch(e._msk, e._x, e._y);
				else
					e._ptr->touch(e._msk, e._x - e._ptr->_x, e._y - e._ptr->_y);
			} else if (Sys)
					Sys->touch(e._msk, e._x, e._y);

			if (e._msk & L_DN) {
				Hold = e._ptr;
				if (Hold) {
					Hold->_flags._hold = true;
					hx = e._x - Hold->_x;
					hy = e._y - Hold->_y;
				}
			}

			if (e._msk & L_UP) {
				if (Hold) {
					Hold->_flags._hold = false;
					Hold = NULL;
				}
			}
			///Touched = e.Ptr;

			// discard Text if button released
			if (e._msk & (L_UP | R_UP))
				KillText();
		}
		EvtTail = (EvtTail + 1) % EVT_MAX;
	}
	if (Hold)
		Hold->gotoxy(_x - hx, _y - hy);
}


} // End of namespace CGE
