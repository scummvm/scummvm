/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "adlib.h"
#include "audio/fmopl.h"
#include <common/memstream.h>
#include "common/debug.h"

namespace Macs2 {

void Adlib::Func2792(byte registerIndex, byte value) {
	// fn0017_2792 proc
	debug("OPL: Write %.2x to port %.2x", value, registerIndex);
	// _opl->write(0x388, registerIndex);
	_opl->writeReg(registerIndex, value);

	// TODO: What I think happens here is that we save a copy of the register values
	// at the +229Ch location
	/*
	mov	dl,[bp+6h]
	mov	al,[bp+8h]
	xor	ah,ah
	mov	di,ax
	mov	[di+229Ch],dl
	xor	ax,ax
	mov	[bp-2h],ax
	jmp	27B5h
	*/

	/*

	l0017_27B2:
	inc	word ptr [bp-2h]

l0017_27B5:
	mov	dx,388h
	in	al,dx
	mov	[bp-3h],al
	cmp	word ptr [bp-2h],6h
	jnz	27B2h

l0017_27C2:
	;; The last pushed value is what we write out
	mov	al,[bp+6h]
	mov	dx,389h
	out	dx,al
	xor	ax,ax
	mov	[bp-2h],ax
	jmp	27D3h

l0017_27D0:
	inc	word ptr [bp-2h]

l0017_27D3:
	mov	dx,388h
	in	al,dx
	mov	[bp-3h],al
	cmp	word ptr [bp-2h],24h
	jnz	27D0h

l0017_27E0:
	leave
	retf	4h

	*/
}

void Adlib::Func2686() {
	// Ignoring this code for now, maybe just fancy sync stuff not needed on the emulator
	/*

	fn0017_2686 proc
	enter	108h,0h
	mov	byte ptr [0062h],7h
	mov	word ptr [0063h],220h
	xor	ax,ax
	mov	[bp-4h],ax
	cli
	mov	al,0FFh
	out	21h,al
	xor	ax,ax
	mov	[bp-8h],ax
	jmp	26A9h

l0017_26A6:
	inc	word ptr [bp-8h]

l0017_26A9:
	mov	word ptr [0065h],3E8h
	mov	word ptr [0067h],3E8h
	push	4h
	push	80h
	call	far 0017h:2792h
	push	3h
	push	0C0h
	call	far 0017h:2792h
	xor	ax,ax
	mov	[0067h],ax
	push	4h
	push	2h
	call	far 0017h:2792h
	mov	dx,388h
	in	al,dx
	and	al,20h
	cmp	al,20h
	jnz	26E7h

l0017_26DF:
	mov	word ptr [bp-2h],2h
	jmp	2774h

l0017_26E7:
	mov	dx,388h
	in	al,dx
	and	al,20h
	or	al,al
	jnz	270Ch

l0017_26F1:
	cmp	word ptr [bp-4h],0FFh
	jnc	270Ch

l0017_26F7:
	inc	word ptr [bp-4h]
	xor	ax,ax
	mov	[bp-6h],ax
	jmp	2704h

l0017_2701:
	inc	word ptr [bp-6h]

l0017_2704:
	cmp	word ptr [bp-6h],63h
	jnz	2701h

l0017_270A:
	jmp	26E7h

l0017_270C:
	cmp	word ptr [bp-4h],0FFh
	jnz	2719h

l0017_2712:
	mov	word ptr [bp-2h],2h
	jmp	2774h

l0017_2719:
	cmp	word ptr [bp-8h],0h
	jz	2727h

l0017_271F:
	mov	ax,[bp-4h]
	shr	ax,1h
	mov	[bp-4h],ax

l0017_2727:
	cmp	word ptr [bp-8h],7h
	jz	2730h

l0017_272D:
	jmp	26A6h

l0017_2730:
	mov	al,0h
	out	21h,al
	sti
	mov	ax,[bp-4h]
	xor	dx,dx
	mov	cx,46h
	div	cx
	mov	[0065h],ax
	imul	ax,[0065h],7h
	mov	[0067h],ax
	mov	word ptr [bp-4h],2h
	jmp	2754h

	inc	word ptr [bp-4h]
	*/
// l0017_2751:
// l0017_2754:
	// Reset all register to 0
	for (int i = 2; i < 256; i++) {
		Func2792(i, 0);
	}

	// l0017_2766:
	// Waveform select of register 1
	Func2792(0x1, 0x20);
	/*


	push	1h
	push	20h
	call	far 0017h:2792h
	xor	ax,ax
	mov	[bp-2h],ax

l0017_2774:
	mov	ax,[bp-2h]
	leave
	retf

		*/
}

void Adlib::Func24FD() {
	/*

	fn0017_24FD proc
	enter	6h,0h
	cmp	byte ptr [0036h],0h
	jz	2510h

l0017_2508:
	mov	word ptr [bp-2h],3h
	jmp	2648h

l0017_2510:
	call	far 0017h:2686h
	mov	[bp-6h],ax
	cmp	word ptr [bp-6h],0h
	jz	2527h

l0017_251E:
	mov	ax,[bp-6h]
	mov	[bp-2h],ax
	jmp	2648h
*/
	// TODO: There is some conditional code around this one
// l0017_2527:
	// Waveform select of register 1
	Func2792(0x1, 0x20);
	// TODO: Figure out this call
	/*
	push	8h
	mov	di,2292h
	push	ds
	push	di
	call	far 0017h:2FBEh
	*/
	// TODO: Figure out this part
	/*
	cli
	mov	ax,[2244h]
	mov	dx,[2246h]
	mov	[2250h],ax
	mov	[2252h],dx
	mov	byte ptr [225Eh],0h
	xor	ax,ax
	mov	[225Ah],ax
	mov	[225Ch],ax
	mov	byte ptr [2259h],0h
	mov	byte ptr [2291h],9h
	*/
	// rite to BD, Amplitude Modulation Depth / Vibrato Depth / Rhythm - all reset
	Func2792(0xBD, 0);
	/* 
	xor	ax,ax
	mov	[223Eh],ax
	mov	[2240h],ax
	xor	ax,ax
	mov	[bp-6h],ax
	jmp	257Dh

l0017_257A:
	inc	word ptr [bp-6h]

l0017_257D:
	mov	di,[bp-6h]
	mov	byte ptr [di+225Fh],0h
	mov	di,[bp-6h]
	mov	byte ptr [di+226Fh],0h
	cmp	word ptr [bp-6h],0Fh
	jnz	257Ah

l0017_2593:
	xor	ax,ax
	mov	[bp-6h],ax
	jmp	259Dh

l0017_259A:
	inc	word ptr [bp-6h]

l0017_259D:
	mov	di,[bp-6h]
	mov	byte ptr [di+222Ch],1h
	mov	di,[bp-6h]
	mov	byte ptr [di+227Fh],0FFh
	mov	di,[bp-6h]
	mov	byte ptr [di+2288h],0FFh
	mov	di,[bp-6h]
	mov	byte ptr [di+2235h],0FFh
	cmp	word ptr [bp-6h],8h
	jnz	259Ah

l0017_25C3:
	call	far 0017h:1A03h
	cmp	word ptr [224Eh],12h
	jbe	2629h

l0017_25CF:
	cmp	word ptr [224Eh],0h
	jz	2629h

l0017_25D6:
	add	word ptr [224Eh],9h
	mov	ax,[224Eh]
	xor	dx,dx
	mov	cx,12h
	div	cx
	xchg	dx,ax
	mov	dx,ax
	mov	ax,[224Eh]
	sub	ax,dx
	mov	[224Eh],ax
	xor	ax,ax
	mov	[2296h],ax
	mov	ax,[224Eh]
	xor	dx,dx
	mov	cx,12h
	div	cx
	mov	[2298h],ax
	mov	ax,[224Eh]
	xor	dx,dx
	mov	cx,ax
	mov	bx,dx
	mov	ax,0AE3Ch
	mov	dx,10h
	call	far 00CDh:0CD4h
	mov	[bp-4h],ax
	mov	ax,[bp-4h]
	and	ax,0FFh
	out	40h,al
	mov	ax,[bp-4h]
	shr	ax,8h
	out	40h,al

l0017_2629:
	push	8h
	mov	di,1A9Fh
	mov	ax,17h
	push	ax
	push	di
	call	far 0017h:2FD6h
	mov	byte ptr [0036h],1h
	mov	byte ptr [2258h],10h
	sti
	xor	ax,ax
	mov	[bp-2h],ax

l0017_2648:
	mov	ax,[bp-2h]
	leave
	retf
	*/
}

void Adlib::Func2839() {
	
	// TODO: We are doing some loading of values in bp-1 and bp-2 from some central data, skipping this
	// and hardcoding for now
	/*
	fn0017_2839 proc
	enter	2h,0h
	mov	al,[bp+0Ah]
	xor	ah,ah
	mov	di,ax
	mov	al,[di+8Dh]
	mov	[bp-1h],al
	mov	al,[bp+0Ah]
	xor	ah,ah
	mov	di,ax
	mov	al,[di+96h]
	mov	[bp-2h],al
	mov	al,[bp-1h]
	xor	ah,ah
	add	ax,20h
	push	ax
	les	di,[bp+6h]
	;; Looks like we are drawing data from this, let's see if this comes from the file
	mov	al,es:[di]
	push	ax
	call	far 0017h:2792h
	*/
	uint16 di = 0x01FE;
	uint16 bp1 = 0x0;
	bp1 += 0x20;
	data->seek(di + 0, SEEK_SET);
	uint8 v0 = data->readByte();
	Func2792(bp1, v0);
	// [bp-2h] - TODO: Hardcoded to 3, but actually loaded
	uint16 bp2 = 0x3;
	bp2 += 0x20;
	// TODO: Get proper DI
	data->seek(di + 1, SEEK_SET);
	uint8 v1 = data->readByte();
	Func2792(bp2, v1);
	/* push ax
		les di,
		[bp + 6h]

		mov al,
		es : [di + 1h] push ax
				 call far 0017h : 2792h
	*/
	bp1 = 0;
	bp1 += 0x40;
	// TODO: Get proper DI
	data->seek(di + 2, SEEK_SET);
	uint8 v2 = data->readByte();
	Func2792(bp1, v2);
	/*
	mov	al,[bp-1h]
	xor	ah,ah
	add	ax,40h
	push	ax
	les	di,[bp+6h]
	mov	al,es:[di+2h]
	push	ax
	call	far 0017h:2792h */
	bp2 = 0x3;
	bp2 += 0x40;
	data->seek(di + 3, SEEK_SET);
	uint8 v3 = data->readByte();
	Func2792(bp2, v3);
	/*
	mov	al,[bp-2h]
	xor	ah,ah
	add	ax,40h
	push	ax
	les	di,[bp+6h]
	mov	al,es:[di+3h]
	push	ax
	call	far 0017h:2792h */
	bp1 = 0;
	bp1 += 0x60;
	data->seek(di + 4, SEEK_SET);
	uint8 v4 = data->readByte();
	Func2792(bp1, v4);
	/*
	mov	al,[bp-1h]
	xor	ah,ah
	add	ax,60h
	push	ax
	les	di,[bp+6h]
	mov	al,es:[di+4h]
	push	ax
	call	far 0017h:2792h
	*/
	bp2 = 0x3;
	bp2 += 0x60;
	data->seek(di + 5, SEEK_SET);
	uint8 v5 = data->readByte();
	Func2792(bp2, v5);

	/*
	mov	al,[bp-2h]
	xor	ah,ah
	add	ax,60h
	push	ax
	les	di,[bp+6h]
	mov	al,es:[di+5h]
	push	ax
	call	far 0017h:2792h
	*/
	bp1 = 0;
	bp1 += 0x80;
	data->seek(di + 6, SEEK_SET);
	uint8 v6 = data->readByte();
	Func2792(bp1, v6);

	/*
	mov	al,[bp-1h]
	xor	ah,ah
	add	ax,80h
	push	ax
	les	di,[bp+6h]
	mov	al,es:[di+6h]
	push	ax
	call	far 0017h:2792h
	*/
	bp2 = 0x3;
	bp2 += 0x80;
	data->seek(di + 7, SEEK_SET);
	uint8 v7 = data->readByte();
	Func2792(bp2, v7);
	/*
	mov	al,[bp-2h]
	xor	ah,ah
	add	ax,80h
	push	ax
	les	di,[bp+6h]
	mov	al,es:[di+7h]
	push	ax
	call	far 0017h:2792h
	*/
	bp1 = 0;
	bp1 += 0xE0;
	data->seek(di + 8, SEEK_SET);
	uint8 v8 = data->readByte();
	Func2792(bp1, v8);

	/*
	mov	al,[bp-1h]
	xor	ah,ah
	add	ax,0E0h
	push	ax
	les	di,[bp+6h]
	mov	al,es:[di+8h]
	push	ax
	call	far 0017h:2792h */

	bp2 = 0x3;
	bp2 += 0xE0;
	data->seek(di + 9, SEEK_SET);
	uint8 v9 = data->readByte();
	Func2792(bp2, v9);

	/*
	mov	al,[bp-2h]
	xor	ah,ah
	add	ax,0E0h
	push	ax
	les	di,[bp+6h]
	mov	al,es:[di+9h]
	push	ax
	call	far 0017h:2792h */
	// TODO: This is a parameter
	
	uint16 bp0A = 0x100;
	bp0A += 0xC0;
	data->seek(di + 10, SEEK_SET);
	uint8 v10 = data->readByte();
	Func2792(bp0A, v10);
	/*
	mov	al,[bp+0Ah]
	xor	ah,ah
	add	ax,0C0h
	push	ax
	les	di,[bp+6h]
	mov	al,es:[di+0Ah]
	push	ax
	call	far 0017h:2792h
	*/
	// TODO: Refactor, this code is unnecessarily cumbersome
}

void Adlib::OnTimer() {

	/* TODO: Skipped:
	mov	ax,0DDh
	mov	ds,ax
	*/
	g2296++;

	if (g2296 >= g2298) {
		// l0017_1ABD:
		g2296 = 0;

		/* TODO: Skipped parts with a function pointer
		xor	ax,ax
		mov	[2296h],ax
		pushf
		call	dword ptr [2292h]
		mov	byte ptr [229Ah],1h
		jmp	1AD3h
		*/
	} else {
		// l0017_1ACE:
		g229A = false;
	}
	// l0017_1AD3:
	// TODO: Continue here
	if (!g229A) {
		// l0017_1ADA:
		// TODO: Nothing to do in our implementation, the original confirms we are handling the
		// interrupt
	}
	// l0017_1ADE:
	;
	;
	g2258 = g2258 & 0xDF;
	if (g2258 & 0x2) {
		// l0017_1AEF:
		g2258 |= 0x40;
	}
	// l0017_1AF7:
	// TODO: Continue from here
	
	
}

uint16 Adlib::Func19BE(uint8 offset) {
	uint16 pos = data->pos();
	if (offset > 0xF8) {
		// l0017_19D8:
		// TODO: Confirm that this works as expected
		pos &= 0xF;
	}
	// l0017_19EA
	pos += offset;
	return pos;
}

void Adlib::Func1A03() {

	_nextEventTimer = 0;
	uint8 bp1;
	// l0017_1A0F:
	do {

	bp1 = data->readByte();
	// Go back to allow 19BE below to handle it properly
	data->seek(-1, SEEK_CUR);

	uint32 timer = _nextEventTimer;
	// TODO: Not sure what this does in practice
	timer << 7;
	_nextEventTimer = timer;
	_nextEventTimer += bp1;
	// TODO: Check if this can also change the segment address
	uint16 newPos = Func19BE(1);
	data->seek(newPos, SEEK_SET);
	// TODO: Not yet implemented
	// add	word ptr [225Ah],1h
	// adc word ptr[225Ch], 0h
	} while ((bp1 & 0x80) != 0);
}

void Adlib::Init() {
	_opl = OPL::Config::create();
	int status = _opl->init();

	#define CALLBACKS_PER_SECOND 10
	_opl->start(new Common::Functor0Mem<void, Adlib>(this, &Adlib::OnTimer), CALLBACKS_PER_SECOND);

	// Hardcoded test to see if I got the logic right
	data->seek(0x06DE, SEEK_SET);
	Func1A03();

	/* _opl->writeReg(0x20, 0x01);
	_opl->writeReg(0x40, 0x10);
	_opl->writeReg(0x60, 0xF0);
	_opl->writeReg(0x80, 0x77);
	_opl->writeReg(0xA0, 0x98);
	_opl->writeReg(0x23, 0x01);
	_opl->writeReg(0x43, 0x00);
	_opl->writeReg(0x63, 0xF0);
	_opl->writeReg(0x83, 0x77);
	_opl->writeReg(0xB0, 0x31); */

	// return;

	// TODO: Check where this is called from and if we need to implement that one as well
	// TODO: CHeck if we need to react to return value
	Func2686();
	Func24FD();
	// TODO: Consider adding the caller
	Func2839();

	// TODO: More hardcoded:
	// TODO: I think this was not actual code from the game but from the example I found!
	// Probably this https://bespin.org/~qz/pc-gpe/adlib.txt
	Func2792r(0x92, 0x20);
	Func2792r(0x01, 0x23);
	Func2792r(0x52, 0x40);
	Func2792r(0x00, 0x43);
	Func2792r(0xdc, 0x60);
	Func2792r(0xf5, 0x63);
	Func2792r(0x23, 0x80);
	Func2792r(0x13, 0x83);
	Func2792r(0x80, 0xe0);
	Func2792r(0x81, 0xe3);
	Func2792r(0x0c, 0xc0);
	Func2792r(0x00, 0xb0);
	Func2792r(0x04, 0x43);
	Func2792r(0x54, 0x40);
	Func2792r(0x98, 0xa0);
	Func2792r(0x29, 0xb0);


	// Hardcoded test below TODO Implement properly
	// Trying to hardcode the delta
	/*  Func2792(0xb0, 0x00);
	Func2792(0x43, 0x04);
	Func2792(0x40, 0x54);


	// Just for the hell of it, try to send the first note on to see if it works
	Func2792(0xa0, 0x98);
	Func2792(0xb0, 0x29);
	_opl->writeReg(0xB0, 0x31); */
		


}

void Adlib::Deinit() {
	_opl->stop();
	delete _opl;
}

} // End of namespace Macs2
