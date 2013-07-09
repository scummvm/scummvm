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
 * This code is based on the original source code of Lord Avalot d'Argent version 1.3.
 * Copyright (c) 1994-1995 Mike, Mark and Thomas Thurman.
 */

/* CELER	The unit for updating the screen pics. */

#include "avalanche/avalanche.h"

#include "avalanche/celer2.h"
#include "avalanche/trip6.h"
#include "avalanche/lucerna2.h"
#include "avalanche/gyro2.h"
#include "avalanche/roomnums.h"

#include "common/textconsole.h"

namespace Avalanche {

const int16 Celer::on_disk = -1;


Celer::Celer() {
	num_chunks = 0;
}

void Celer::setParent(AvalancheEngine *vm) {
	_vm = vm;
}

void Celer::pics_link() {
	byte xx;

	if (_vm->_gyro.ddmnow)
		return; /* No animation when the menus are up. */

	
	switch (_vm->_gyro.dna.room) {
	case r__outsideargentpub:
		if ((_vm->_gyro.roomtime % int32(12)) == 0)
			show_one(int32(1) + (_vm->_gyro.roomtime / int32(12)) % int32(4));
		break;

	case r__brummieroad:
		if ((_vm->_gyro.roomtime % int32(2)) == 0)
			show_one(int32(1) + (_vm->_gyro.roomtime / int32(2)) % int32(4));
		break;

	case r__bridge:
		if ((_vm->_gyro.roomtime % int32(2)) == 0)
			show_one(int32(4) + (_vm->_gyro.roomtime / int32(2)) % int32(4));
		break;

	case r__yours:
		if ((!_vm->_gyro.dna.avvy_is_awake) && ((_vm->_gyro.roomtime % int32(4)) == 0))
			show_one(int32(1) + (_vm->_gyro.roomtime / int32(12)) % int32(2));
		break;

	case r__argentpub:
		if (((_vm->_gyro.roomtime % int32(7)) == 1) && (_vm->_gyro.dna.malagauche != 177)) {
			/* Malagauche cycle */
			_vm->_gyro.dna.malagauche += 1;
			switch (_vm->_gyro.dna.malagauche) {
			case 1:
			case 11:
			case 21:
				show_one(12);
				break; /* Looks forwards. */
			case 8:
			case 18:
			case 28:
			case 32:
				show_one(11);
				break; /* Looks at you. */
			case 30:
				show_one(13);
				break; /* Winks. */
			case 33:
				_vm->_gyro.dna.malagauche = 0;
				break;
			}
		}

		switch (_vm->_gyro.roomtime % 200) {
		case 179:
		case 197:
			show_one(5);
			break; /* Dogfood's drinking cycle */
		case 182:
		case 194:
			show_one(6);
			break;
		case 185:
			show_one(7);
			break;
		case 199:
			_vm->_gyro.dna.dogfoodpos = 177;
			break; /* Impossible value for this. */
		}

		if ((_vm->_gyro.roomtime % 200 >= 0) && (_vm->_gyro.roomtime % 200 <= 178)) { /* Normally. */
			if (((_vm->_lucerna.bearing(2) >= 1) && (_vm->_lucerna.bearing(2) <= 90)) || ((_vm->_lucerna.bearing(2) >= 358) && (_vm->_lucerna.bearing(2) <= 360)))
				xx = 3;
			else if ((_vm->_lucerna.bearing(2) >= 293) && (_vm->_lucerna.bearing(2) <= 357))
				xx = 2;
			else if ((_vm->_lucerna.bearing(2) >= 271) && (_vm->_lucerna.bearing(2) <= 292))
				xx = 4;

			if (xx != _vm->_gyro.dna.dogfoodpos) { /* Only if it's changed.*/
				show_one(xx);
				_vm->_gyro.dna.dogfoodpos = xx;
			}
		}
		break;

	case r__westhall:
		if ((_vm->_gyro.roomtime % int32(3)) == 0) {
			switch ((_vm->_gyro.roomtime / int32(3)) % int32(6)) {
			case 4:
				show_one(1);
				break;
			case 1:
			case 3:
			case 5:
				show_one(2);
				break;
			case 0:
			case 2:
				show_one(3);
				break;
			}
		}
		break;

	case r__lustiesroom:
		if (!(_vm->_gyro.dna.lustie_is_asleep)) {
			if ((_vm->_gyro.roomtime % int32(45)) > 42)
				xx = 4; /* du Lustie blinks */

			/* Bearing of Avvy from du Lustie. */
			else if (((_vm->_lucerna.bearing(2) >= 0) && (_vm->_lucerna.bearing(2) <= 45)) || ((_vm->_lucerna.bearing(2) >= 315) && (_vm->_lucerna.bearing(2) <= 360)))
					xx = 1; /* Middle. */
			else if ((_vm->_lucerna.bearing(2) >= 45) && (_vm->_lucerna.bearing(2) <= 180))
					xx = 2; /* Left. */
			else if ((_vm->_lucerna.bearing(2) >= 181) && (_vm->_lucerna.bearing(2) <= 314))
				xx = 3; /* Right. */

			if (xx != _vm->_gyro.dna.dogfoodpos) { /* Only if it's changed.*/
				show_one(xx);
				_vm->_gyro.dna.dogfoodpos = xx; /* We use DogfoodPos here too- why not? */
			}
		}
		break;

	case r__aylesoffice:
		if ((!_vm->_gyro.dna.ayles_is_awake) && (_vm->_gyro.roomtime % int32(14) == 0)) {
			switch ((_vm->_gyro.roomtime / int32(14)) % int32(2)) {
			case 0:
				show_one(1);
				break; /* Frame 2: EGA. */
			case 1:
				show_one(3);
				break; /* Frame 1: Natural. */
			}
		}
		break;

	case r__robins:
		if (_vm->_gyro.dna.tied_up) {
			switch (_vm->_gyro.roomtime % int32(54)) {
			case 20:
				show_one(4);
				break; /* Frame 4: Avalot blinks. */
			case 23:
				show_one(2);
				break; /* Frame 1: Back to normal. */
			}
		}
		break;

	case r__nottspub:
		/* Bearing of Avvy from Port. */
		if (((_vm->_lucerna.bearing(5) >= 0) && (_vm->_lucerna.bearing(5) <= 45)) || ((_vm->_lucerna.bearing(5) >= 315) && (_vm->_lucerna.bearing(5) <= 360)))
			xx = 2; /* Middle. */
		else if ((_vm->_lucerna.bearing(5) >= 45) && (_vm->_lucerna.bearing(5) <= 180))
			xx = 6; /* Left. */
		else if ((_vm->_lucerna.bearing(5) >= 181) && (_vm->_lucerna.bearing(5) <= 314))
			xx = 8; /* Right. */

		if ((_vm->_gyro.roomtime % int32(60)) > 57)
			xx--; /* Blinks */

		if (xx != _vm->_gyro.dna.dogfoodpos) { /* Only if it's changed.*/
			show_one(xx);
			_vm->_gyro.dna.dogfoodpos = xx; /* We use DogfoodPos here too- why not? */
		}

		switch (_vm->_gyro.roomtime % 50) {
		case 45 :
			show_one(9);
			break; /* Spurge blinks */
		case 49 :
			show_one(10);
			break;
		}
		break;

	case r__ducks:
		if ((_vm->_gyro.roomtime % 3) == 0) /* The fire flickers */
			show_one(1 + (_vm->_gyro.roomtime / 3) % 3);

		{/* _vm->_lucerna.bearing of Avvy from Duck. */
		if (((_vm->_lucerna.bearing(2) >= 0) && (_vm->_lucerna.bearing(2) <= 45)) || ((_vm->_lucerna.bearing(2) >= 315) && (_vm->_lucerna.bearing(2) <= 360)))
			xx = 4; /* Middle. */
		else if ((_vm->_lucerna.bearing(2) >= 45) && (_vm->_lucerna.bearing(2) <= 180))
			xx = 6; /* Left. */
		else if ((_vm->_lucerna.bearing(2) >= 181) && (_vm->_lucerna.bearing(2) <= 314))
			xx = 8; /* Right. */

		if ((_vm->_gyro.roomtime % int32(45)) > 42)
			xx += 1; /* Duck blinks */

		if (xx != _vm->_gyro.dna.dogfoodpos) { /* Only if it's changed.*/
			show_one(xx);
			_vm->_gyro.dna.dogfoodpos = xx; /* We use DogfoodPos here too- why not? */
		}
		break;

	}
}

	if ((_vm->_gyro.dna.ringing_bells) && (_vm->_gyro.flagset('B'))) {
		/* They're ringing the bells. */
		switch (_vm->_gyro.roomtime % int32(4)) {
		case 1:
			if (_vm->_gyro.dna.nextbell < 5)
				_vm->_gyro.dna.nextbell = 12;
			_vm->_gyro.dna.nextbell -= 1;
			_vm->_gyro.note(_vm->_gyro.notes[_vm->_gyro.dna.nextbell]);
			break;
		case 2:
			//nosound;
			warning("STUB: Celer::pics_link()");
			break;
		}
	}
}

void Celer::load_chunks(Common::String xx) {
	chunkblocktype ch;
	byte fv;
	
	filename = filename.format("chunk%s.avd", xx.c_str());
	if (!f.open(filename)) {
		warning("AVALANCHE: Celer: File not found: %s", filename.c_str());
		return;
	}

	f.seek(44);
	num_chunks = f.readByte();
	for (byte i = 0; i < num_chunks; i++)
		offsets[i] = f.readSint32LE();

	for (fv = 0; fv < num_chunks; fv++) {
		f.seek(offsets[fv]);
		
		ch.flavour = flavourtype(f.readByte());
		ch.x = f.readSint16LE();
		ch.y = f.readSint16LE();
		ch.xl = f.readSint16LE();
		ch.yl = f.readSint16LE();
		ch.size = f.readSint32LE();
		ch.natural = f.readByte();
		ch.memorise = f.readByte();
				
		if (ch.memorise) {
			memos[fv].x = ch.x;
			memos[fv].xl = ch.xl;
			memos[fv].y = ch.y;
			memos[fv].yl = ch.yl;
			memos[fv].flavour = ch.flavour;
			memos[fv].size = ch.size;

			memory[fv] = new byte[ch.size]; // Celer::forget_chunks() deallocates it.

			/*if (ch.natural) {
			getimage(ch.x * 8, ch.y, (ch.x + ch.xl) * 8, ch.y + ch.yl, memory[fv]);
			} else
			blockread(f, memory[fv], ch.size);*/
			warning("STUB: Celer::load_chunks()");

			if (!ch.natural)
				f.read(memory[fv], ch.size);
		} else
			memos[fv].x = on_disk;
	}
	f.close();
}

void Celer::forget_chunks() {
	for (byte fv = 0; fv < num_chunks; fv ++)
		if (memos[fv].x > on_disk)
			delete[] memory[fv];

	memset(memos, 255, sizeof(memos)); /* x=-1, => on disk. */
}

void Celer::mdrop(int16 x, int16 y, int16 xl, int16 yl, void *p) { /* assembler;
asm
	push ds;      { Strictly speaking, we shouldn't modify DS, so we'll save it.}
	push bp;      { Nor BP! }


	{ DI holds the offset on this page. It starts at the top left-hand corner. }
	{ (It should equal ch.y*80+ch.x. }

	mov ax,y;
	mov dl,80;
	mul dl; { Line offset now calculated. }
	mov di,ax; { Move it into DI. }
	mov ax,x;
	add di,ax; { Full offset now calculated. }

	mov bx,yl; { No. of times to repeat lineloop. }
	inc bx;        { "loop" doesn't execute the zeroth time. }
	mov bh,bl;     { Put it into BH. }

	{ BP holds the length of the Common::String to copy. It's equal to ch.xl.}

	mov ax,uint16(p);   { Data is held at DS:SI. }
	mov si,ax;
	mov ax,uint16(p+2); { This will be moved over into ds in just a tick... }

	mov bp,xl;

	mov ds,ax;


	cld;          { We're allowed to hack around with the flags! }

	mov ax,$AC00; { Top of the first EGA page. }
	mov es,ax;    { Offset on this page is calculated below... }


{    port[$3c4]:=2; port[$3ce]:=4; }

	mov dx,$3c4;
	mov al,2;
	out dx,al;
	mov dx,$3ce;
	mov al,4;
	out dx,al;

	mov cx,4;  { This loop executes for 3, 2, 1, and 0. }
	mov bl,0;


	@mainloop:

	push di;
	push cx;

{    port[$3C5]:=1 shl bit; }
	mov dx,$3C5;
	mov al,1;
	mov cl,bl; { BL = bit. }
	shl al,cl;
	out dx,al;
{     port[$3CF]:=bit; }
	mov dx,$3CF;
	mov al,bl; { BL = bit. }
	out dx,al;

	xor ch,ch;
	mov cl,bh; { BH = ch.yl. }

	@lineloop:

		push cx;

		mov cx,bp;

		repz movsb; { Copy the data. }

		sub di,bp;
		add di,80;

		pop cx;

	loop @lineloop;

	inc bl; { One more on BL. }

	pop cx;
	pop di;

	loop @mainloop;

	pop bp;
	pop ds;       { Get DS back again. }
*/
}



void Celer::display_it(int16 x, int16 y, int16 xl, int16 yl, flavourtype flavour, void *p) {
	warning("STUB: Celer::display_it()");
}

void Celer::show_one(byte which) {
	chunkblocktype ch;
	byte *p;
	
	//setactivepage(3);
	warning("STUB: Celer::show_one()");

	if (memos[which].x > on_disk)
		display_it(memos[which].x, memos[which].y, memos[which].xl, memos[which].yl, memos[which].flavour, memory[which]);
	else {
		if (!f.open(filename)) { /* Filename was set in load_chunks() */
			warning("AVALANCHE: Celer: File not found: %s", filename.c_str());
			return;
		}

		f.seek(offsets[which]);

		ch.flavour = flavourtype(f.readByte());
		ch.x = f.readSint16LE();
		ch.y = f.readSint16LE();
		ch.xl = f.readSint16LE();
		ch.yl = f.readSint16LE();
		ch.size = f.readSint32LE();
		ch.natural = f.readByte();
		ch.memorise = f.readByte();

		p = new byte[ch.size];
		f.read(p, ch.size);

		display_it(ch.x, ch.y, ch.xl, ch.yl, ch.flavour, p);

		delete[] p;
		f.close();
	}

	//setactivepage(1 - cp);
	warning("STUB: Celer::show_one()");

	for (byte fv = 0; fv < 2; fv ++)
		_vm->_trip.getset[fv].remember(r);
}



void Celer::display_it_at(int16 xl, int16 yl, flavourtype flavour, void *p, int16 &xxx, int16 &yyy) {
	warning("STUB: Celer::display_it1()");
}

void Celer::show_one_at(byte which, int16 xxx, int16 yyy) {
	chunkblocktype ch;
	byte *p;

	//setactivepage(3);
	warning("STUB: Celer::show_one_at()");

	if (memos[which].x > on_disk) {
		display_it_at(memos[which].xl, memos[which].yl, memos[which].flavour, memory[which], xxx, yyy);
	} else {
		if (!f.open(filename)) { /* Filename was set in load_chunks() */
			warning("AVALANCHE: Celer: File not found: %s", filename.c_str());
			return;
		}

		f.seek(offsets[which]);
		ch.flavour = flavourtype(f.readByte());
		ch.x = f.readSint16LE();
		ch.y = f.readSint16LE();
		ch.xl = f.readSint16LE();
		ch.yl = f.readSint16LE();
		ch.size = f.readSint32LE();
		ch.natural = f.readByte();
		ch.memorise = f.readByte();

		{
			p = new byte[ch.size];
			f.read(p, ch.size);

			display_it_at(ch.xl, ch.yl, ch.flavour, p, xxx, yyy);

			delete[] p;
			f.close();
		}
	}

	//setactivepage(1 - cp);
	warning("STUB: Celer::show_one_at()");

	for (byte fv = 0; fv < 2; fv ++)
		_vm->_trip.getset[fv].remember(r);
}



} // End of namespace Avalanche.
