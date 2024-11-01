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

	// TODO: This is a huge jump, maybe should go for a separate function
	if (g2258 & 0xC3) {
		// l0017_1B03:
		if (_nextEventTimer != 0) {
			_nextEventTimer--;
			// TODO: There is some more to do in theory, but might be neglible
			/*
			* l0017_2438:
				cmp	byte ptr [229Ah],0h
				jnz	2443h

			l0017_243F:
				mov	al,20h
				out	20h,al
			*/
			return;
		}
		// l0017_1B19:
		// TODO: Code does a cli, but probably neglible

		// TODO: Handle the loop properly
		for (;;) {
			// l0017_1B1A:
			uint8 current = peekByte();
			if (current & 0x80) {
				// l0017_1B27:
				uint8 copy = peekByte();
				g229B = copy;
				g223E = copy;
				data->seek(Func19BE(1), SEEK_SET);
				g225A++;
			}
			// l0017_1B5F:
			uint8 bp3 = g229B & 0x0F;
			uint8 bp6 = g229B;
			uint8 bp4 = peekByte();
			uint16 bp10 = Func19BE(1);
			uint8 bp5 = peekByteAt(bp10);
			
			if ((bp6 & 0x0F) == 0x90) {
				// l0017_1BA1:
				if (bp5 != 0) {
					// l0017_1BAA:
					data->seek(Func19BE(2), SEEK_SET);
					g225A += 2;
					
					if (g2291 == 0x09 || bp3 < 0x0B) {
						// l0017_1BE4:
						// TODO: Is this the first usage?
						uint8 bp8 = 0;
						// TODO: Figure out the loop conditions and correct indentation
						do {
							// l0017_1BE9:
							// TODO: Not sure if it's important to remove the higher bits as the
							// original does
							if (g2291 > bp8) {
								// l0017_1BF3:
								if (gArray222C[bp8] == 0) {
									// l0017_1BFD:
									uint8 v = gArray227F[bp8];
									if (v == bp3) {
										// l0017_1C09:
										uint8 v2 = gArray2235[bp8];
										if (v2 != bp4) {
											// l0017_1C15:
											bp8++;
										} else {
											break;
										}
									}
								}	
							}
						// TODO: Check if there is a better exit condition
						} while (true);
						// l0017_1C1A:
						// TODO: Not sure about removal of AH bits in the original
						if (g2291 == bp8) {
							// l0017_1C27:
							uint16 bp0C = 0;
							// TODO: Not sure about upper bits removal for both
							bp8 = g2291;
							uint16 bp16 = g2291 - 1;
							if (bp16 <= 0) {
								// l0017_1C44:
								uint16 bp0A = 0;
								// TODO: Loop condition
								do {
									// l0017_1C49:
									bp0A++;
									// l0017_1C4C:
									if (gArray222C[bp0A] != 0) {
										// l0017_1C56:
										gArray222C[bp0A]++;
									}
									// l0017_1C5D:
									// TODO: Again several struct accesses that are not
									// implemented yet
									if (gArray222C[bp0A] > bp0C) {
										// l0017_1C6B:
										bp0C = gArray222C[bp0A];
										bp8 = bp0A;
									}
									// l0017_1C7D:
									if (bp0A == bp16) {
										break;
									}
									// TODO: Continue from here
								} while (true);
							}
							// l0017_1C85:
							if (bp0C != 0) {
								gArray222C[bp8] = 0;
								gArray227F[bp8] = bp3;
								// TODO: Original code uses a 16 bit register and clears the upper bit
								// Not sure if this is really needed
								if (gArray225F[bp3] != gArray2288[bp8]) {
									// l0017_1CB1:
									// TODO: Original uses 16 bit again and throws away
									// upper 8
									gArray2288[bp8] = gArray225F[bp3];
									// TODO: Access an array via bp-3h
									
									// TODO: Move an offset in global 2248 forward based on
									// the struct
									// TODO: Pushes
									// push word ptr[224Ah]
									// push word ptr[2248h]
									// TODO: Is the data type wide enough?
									// TODO: In the original code, we throw away the upper byte
									// before the calculation

									// TODO: Not sure if I have the resurn value of Func19BE completely
									uint16 bp12 = Func19BE(gArray2288[bp8] << 0x4);
									Func2839();
									/*
									
									mov	di,[bp-8h]
									mov	al,[di+2288h]
									xor	ah,ah
									shl	ax,4h
									push	ax
									call	far 0017h:19BEh
									mov	[bp-14h],ax
									mov	[bp-12h],dx
									mov	al,[bp-8h]
									push	ax
									push	word ptr [bp-12h]
									push	word ptr [bp-14h]
									call	far 0017h:2839h */
									
								}
								// l0017_1CF2:
								if (g2291 == bp8) {
									// TODO: Continue from here
									// l0017_1CFF:
									// TODO: Array access
									/* mov al, [bp - 4h]
									mov	di,[bp-8h]
									mov	[di+2235h],al*/
									// TODO: More array access
									/* push word ptr[224Ah]
									push	word ptr [2248h]
									mov	al,[bp-3h]
									xor	ah,ah
									mov	di,ax
									mov	al,[di+225Fh]
									xor	ah,ah
									shl	ax,4h
									push	ax
									call	far 0017h:19BEh
									*/
								}
								// TODO: The "or" one should go to 1E91h
								// TODO: Should be 1CFF
								
							}
							

								
						}
						// TODO: Should be 1CF2
						// TODO: Check if this is an else or just an if



					}
					// TODO: Should be 1E94h

					
				}
				// TODO: This must be 2097h


				
			}
			// TODO: This must be 209Bh

		
			
		}


		
	}
	// TODO: This needs to be l0017_2425
	
	
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

uint8 Adlib::peekByte() {
	uint8 result = data->readByte();
	data->seek(-1, SEEK_CUR);
	return result;
}

uint8 Adlib::peekByteAt(uint16 offset) {
	int32 originalOffset = data->pos();
	data->seek(offset, SEEK_SET);
	uint8 result = data->readByte();
	data->seek(originalOffset, SEEK_SET);
	return result;
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
