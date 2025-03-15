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
#include "common/debug.h"
#include <common/memstream.h>

namespace Macs2 {

void Adlib::Func2792(byte registerIndex, byte value) {
	// fn0017_2792 proc
	debug("OPL: Write %.2x to port %.2x", value, registerIndex);
	// _opl->write(0x388, registerIndex);
	_opl->writeReg(registerIndex, value);
	gArray229C[registerIndex = value];

	

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

void Adlib::Func27E4() {
	// AI-reverse engineered by DeepSeek v1 via Perplexity
		uint16 local_counter; // bp-2h (2-byte local variable)

		// First loop section (27EF-27F2 labels)
		local_counter = 0;
		do { // l0017_27F2
			// if (local_counter >= 9)
			//	break; // Original jmp condition
			// Note: Deepseek put this here as well as the bottom where it is
			// originally

			// Body of first loop
			uint16 param = local_counter + 0xB0;
			// Note: Deepseek got confused about the arguments being pushed (as did I
			// the first time I saw it)
			uint8 result = Func2779(param);
			// Note: Local hallucination where it mixes it up with a string operation
			result &= 0xDF;                                     // Convert to uppercase
			Func2792(param, result);

			// l0017_27EF
			local_counter++;
		} while (local_counter <= 8); // cmp 8h, jnz 27EFh

		// TODO: Continue from here

		// Second loop section (2813-281D labels)
		local_counter = 0;
		do { // l0017_281D
			// Access memory at [di+69h]

			uint8 mem_value = gArray69[local_counter];
			uint16 param = mem_value + 0x40;

			Func2792(param, 0xFF);

			// l0017_281A
			// Note: Deepseek again mixed up the order in the loop a bit,
			// but is seems to be equivalent in effect
			local_counter++;
		} while (local_counter <= 0x11); // cmp 11h, jnz 281Ah
	}

uint16 Adlib::Func2686() {
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
	// TODO: Hardcoded zero return until this function is implementedf
	return 0;
}

uint8 Adlib::Func2779(uint8 arg1) {
	return gArray229C[arg1];
}

uint16 Adlib::Func24FD() {
	// This function was reverse-engineered with Deepseek R1
	// Local variables (BP-2 = return value, BP-4/BG-6 temps)
	uint16 return_value;
	uint16 temp_var;
	uint16 loop_counter;

	// [Original label: fn0017_24FD proc]

	if (g36 != 0) { // [Original label: l0017_2508]
		return_value = 3;
		goto CLEANUP_2648;
	}

	// [Original label: l0017_2510]
	return_value = Func2686();
	if (return_value != 0) {
		goto CLEANUP_2648;
	}

	// [Original label: l0017_2527]
	// Following code corresponds to the memory/port operations
	Func2792(0x1, 0x20);
	// Additional far call to 17:2FBE
	// That call saves the interrupt vector - not implementing this
	// for now

	// Original clears interrupts here
	shMem2250 = shMem2244;
	g225E = 0;
	// TODO: Not sure if these are actually a pointer or 32 bit value
	g225A = g225C = 0;
	g2259 = 0;
	g2291 = 9;
	Func2792(0xBD, 0);
	g223E = 0;
	g2240 = 0;

	// Loop 1: Initialize 225Fh-226Fh [Original labels: 257A-2593]
	for (loop_counter = 0; loop_counter <= 0x0F; ++loop_counter) {
		gArray225F[loop_counter] = 0;
		gArray226F[loop_counter] = 0;
	}

	// Loop 2: Initialize array elements [Original labels: 259A-25C3]
	for (loop_counter = 0; loop_counter <= 8; ++loop_counter) {
		gArray222C[loop_counter] = 1;
		gArray227F[loop_counter] = 0xFF;
		gArray2288[loop_counter] = 0xFF;
		gArray2235[loop_counter] = 0xFF;
	}

	
	// l0017_25C3:
	Func1A03();

	// Device/hardware operations (timer/speaker?)
	if ((g224E > 0x12) && (g224E != 0)) {
		g224E += 9;
		uint32 temp = g224E;
		temp = temp % 0x12;
		g224E -= temp;
		g2296 = 0;
		g2298 = g224E / 0x12;

		// TODO: This part sets the frequency of the timer,
		// leaving it out for now.
		/*
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
		*/
	}

	// l0017_2629:
	// This part sets an interrupt
	// TODO: Leaving out for now
	/*
	push	8h
	mov	di,1A9Fh
	mov	ax,17h
	push	ax
	push	di
	call	far 0017h:2FD6h
	*/
	g36 = 1;
	g2258 = 0x10;
	// TODO: Enable interrupts - sti
	return_value = 0;

CLEANUP_2648: // [Original label: l0017_2648]
	return return_value;
}

void Adlib::Func2839(uint8 bpp0A, StreamHandler* sh) {
	uint8 bp1 = gArray8d[bpp0A];
	uint8 bp2 = gArray96[bpp0A];
	
	uint8 value = sh->peekByteAtOffset(0, SEEK_CUR);
	Func2792(bp1 + 0x20, value);
	
	value = sh->peekByteAtOffset(1, SEEK_CUR);
	Func2792(bp2 + 0x20, value);
	
	value = sh->peekByteAtOffset(2, SEEK_CUR);
	Func2792(bp1 + 0x40, value);
	
	value = sh->peekByteAtOffset(3, SEEK_CUR);
	Func2792(bp2 + 0x40, value);
	
	value = sh->peekByteAtOffset(4, SEEK_CUR);
	Func2792(bp1 + 0x60, value);
	
	value = sh->peekByteAtOffset(5, SEEK_CUR);
	Func2792(bp2 + 0x60, value);

	value = sh->peekByteAtOffset(6, SEEK_CUR);
	Func2792(bp1 + 0x80, value);

	value = sh->peekByteAtOffset(7, SEEK_CUR);
	Func2792(bp2 + 0x80, value);
	
	value = sh->peekByteAtOffset(8, SEEK_CUR);
	Func2792(bp1 + 0xE0, value);

	value = sh->peekByteAtOffset(9, SEEK_CUR);
	Func2792(bp2 + 0xE0, value);

	value = sh->peekByteAtOffset(0xA, SEEK_CUR);
	Func2792(bpp0A + 0xC0, value);
}

void Adlib::Func294E(uint16 bpp6, uint8 bpp8, uint16 bppA) {
	uint16 bp4;
	uint8 bp6;
	/*
	;; Arguments:
	;; [bp+0Ah]: The channel index
	;; [bp+8h]: The index into the note data structure
	;; [bp+6h]: TODO: Seems to be 0 for a lot (all) of the note heights?

	*/
	uint8 al = gArray9F[bpp8];
	uint16 dx = al;
	al = gArray11F[bpp8];
	// TODO: Check if we need 16 bits
	uint16 bp2 = al << 0x8;
	if (bpp6 != 0) {
		// l0017_297F:
		if (bpp6 < 0x80) {
			// l0017_2985:
			if (bpp8 < 0x7F) {
				// l0017_298B:
				bp6 = bpp8 + 1;
			} else {
				// l0017_2996:
				bp6 = 0x7F;
			}
			// l0017_299B:
			dx = gArray9F[bp6];
			bp4 = (gArray11F[bp6] << 0x8) + dx;
			// Multiplication using 00CDh:0C97h
			// eax:edx = eax:edx * ebx:ecx
			// TODO: Check actual possible range of result
			uint64 product = bpp6 * (bp4 - bp2);
			// Right shift done by 0D7A proc
			product = product >> 0x7;
			// TODO: Check if we can ever overrun
			bp2 = bp2 + product;
		} else {
			// l0017_29EB:
			if (bpp8 > 0) {
				// l0017_29F1:
				bp6 = bpp8 - 1; // TODO: xor ah, ah
			} else {
				// l0017_29FC:#
				bp6 = 0;
			}
			// l0017_2A01:

			bp4 = (gArray11F[bp6] << 0x8) + gArray9F[bp6];
			// Multiplication using 00CDh:0C97h
			// eax:edx = eax:edx * ebx:ecx
			// TODO: Check actual possible range of result
			uint64 product = bpp6 * (bp2 - bp4);
			// Right shift done by 0D7A proc
			// Result is in ax:dx
			product = product >> 0x7;
			// TODO: Check ranges
			bp2 = bp2 - product;
		}
	}
	// l0017_2A4F:

	// TODO: Original code is making sure that only 8 bit are pushed
	// for the second argument using AND FFh - check if ranges matter here
	Func2792(bppA + 0xA0, bp2);
	Func2792(bppA + 0xB0, (bp2 >> 0x8) | 0x20);
}

void Adlib::OnTimer() {

	// TODO: Check if original code pauses the timer if no song is playing
	if (shMem2250 == nullptr) {
		return;
	}

	debug("Entered OnTimer");

	// fn0017_1AA7 proc

	g2296++;

	if (g2296 >= g2298) {
		// Every nth time we execute this code
		// l0017_1ABD:
		SIS_LogEntry(0x01D7, 0x1ABD);
		g2296 = 0;

		/* TODO: Skipped parts with a function pointer
		xor	ax,ax
		mov	[2296h],ax
		pushf
		call	dword ptr [2292h]
		mov	byte ptr [229Ah],1h
		jmp	1AD3h
		*/
		g229A = true;
	} else {
		// l0017_1ACE:
		SIS_LogEntry(0x01D7, 0x1ACE);
		g229A = false;
	}
	// l0017_1AD3:
	if (!g229A) {
		// l0017_1ADA:
		// TODO: Nothing to do in our implementation, the original confirms we are handling the
		// interrupt
	}
	// l0017_1ADE:
	g2258 = g2258 & 0xDF;
	if (g2258 & 0x2) {
		// [2258] & 2 was not zero
		// l0017_1AEF:
		SIS_LogEntry(0x01D7, 0x1AEF);
		g2258 |= 0x40;
	}
	// l0017_1AF7:

	// TODO: This is a huge jump, maybe should go for a separate function
	if (!(g2258 & 0xC3)) {
		// l0017_1B03:
		SIS_LogEntry(0x01D7, 0x1B03);
		if (_nextEventTimer != 0) {
			_nextEventTimer--;
			// Original code also sends the "End of Interrupt" signal, which
			// we don't need
			return;
		}

		// l0017_1B19:
		// TODO: Code does a cli, but probably neglible

		// TODO: Handle the loop properly
		for (;;) {
			//	// l0017_1B1A:
			uint8 current = shMem2250->peekByte();
			int64 pos = shMem2250->pos();
			debug("Loop iteration, [2250] value: %.2X at offset %.2X", current, shMem2250->pos());
			if (current & 0x80) {
				// The first bit of the read value was 0
				// l0017_1B27:
				SIS_LogEntry(0x01D7, 0x1B27);
				g229B = g223E = shMem2250->peekByte();
				shMem2250 = Func19BE_SH(shMem2250, 1);
				g225A++;
			}
			// l0017_1B5F:
			uint8 bp1;
			uint8 bp2;
			uint8 bp3 = g229B & 0x0F;
			uint8 bp6 = g229B;
			uint8 bp4 = shMem2250->peekByte();
			StreamHandler* bp10 = Func19BE_SH(shMem2250, 1);
			// TODO: This and the adjacent value are actually a pointer adjusted by Func19BE
			uint16 bp12;
			uint8 bp5 = bp10->peekByte();


			if ((bp6 & 0xF0) == 0x90) {
				// l0017_1BA1:
				SIS_LogEntry(0x01D7, 0x1BA1);
				if (bp5 != 0) {
					// l0017_1BAA:
					SIS_LogEntry(0x01D7, 0x1BAA);
					shMem2250 = Func19BE_SH(shMem2250, 0x2);
					g225A += 2;

					if (g2291 == 0x09 || bp3 < 0x0B) {
						// l0017_1BE4:
						SIS_LogEntry(0x01D7, 0x1BE4);
						uint8 bp8 = 0;
						do {
							// l0017_1BE9:			
							if (g2291 <= bp8) {
								break;
							}
							// l0017_1BF3:
							if (gArray222C[bp8] == 0) {
								// l0017_1BFD:
								uint8 v = gArray227F[bp8];
								if (v == bp3) {
									// l0017_1C09:
									uint8 v2 = gArray2235[bp8];
									if (v2 == bp4) {
										break;
									}
								}
							}
							// l0017_1C15:
							bp8++;
						} while (true);
					// l0017_1C1A:
					if (g2291 == bp8) {
						// l0017_1C27:
						SIS_LogEntry(0x01D7, 0x1C27);
						uint16 bp0C = 0;
						bp8 = g2291;
						uint16 bp16 = g2291 - 1;
						if (bp16 > 0) {
							SIS_LogEntry(0x01D7, 0x1C44);
							uint16 bp0A = 0;
							do {
								// l0017_1C49:
								// l0017_1C4C:
								if (gArray222C[bp0A] != 0) {
									// l0017_1C56:
									gArray222C[bp0A]++;
								}
								// l0017_1C5D:
								if (gArray222C[bp0A] > bp0C) {
									// l0017_1C6B:
									bp0C = gArray222C[bp0A];
									bp8 = bp0A;
								}
								// l0017_1C7D:
								if (bp0A == bp16) {
									break;
								}
								// l0017_1C44:
								// Original has this at the top but skips
								// it for the first round
								bp0A++;
							} while (true);
						}
						// l0017_1C85:
						if (bp0C != 0) {
							// l0017_1C8B:
							SIS_LogEntry(0x01D7, 0x1C8B);
							gArray222C[bp8] = 0;
							gArray227F[bp8] = bp3;
							if (gArray225F[bp3] != gArray2288[bp8]) {
								// l0017_1CB1:
								SIS_LogEntry(0x01D7, 0x1CB1);
								gArray2288[bp8] = gArray225F[bp3];
								StreamHandler* shBP12 = Func19BE_SH(shMem2248, gArray2288[bp8] << 0x4);
								Func2839(bp8, shBP12);
							}
						}
					}
					// l0017_1CF2:
					if (g2291 == bp8) {
						SIS_LogEntry(0x01D7, 0x1CFF);
						//							// l0017_1CFF:
						//							gArray2235[bp4] = bp8;
						//							// TODO: Argument missing so far
						//							/* push word ptr[224Ah]
						//							push	word ptr [2248h]
						//							*/
						//							// TODO: Original code throws away AH before and after
						//							// the array access
						//							uint16 value = gArray225F[bp3];
						//							bp10 = Func19BE_TODO(value << 4);
						//							// TODO: Not sure about amount of bits necessary
						//							// for the following calculations
						//							uint16 temp = bp5;
						//							temp &= 0x7F;
						//							// TODO: We throw away AH - should be superfluous
						//							// after the AND
						//							temp = temp >> 0x1;
						//							temp -= 0x3F;
						//							temp = temp >> 0x1;
						//							// TODO: Again throwing away AH in the original
						//							bp1 = temp >> 0x1;
						//							// TODO: Need to give as argument
						//							// push word ptr[bp - 0Eh]
						//							// push word ptr[bp - 10h]
						//							bp12 = Func19BE_TODO(0x2);
						//							// TODO: Set result back
						//							// mov	[bp-14h],ax
						//							// mov[bp - 12h], dx

						//							temp = g225E;         // TODO: xor	ah,ah
						//							uint8 temp2 = temp; // bx = ax
						//							// TODO: Read from [bp-14h] - using temp for al
						//							// les	di,[bp-14h]
						//							// mov al, es : [di]
						//							temp &= 0x3F;         // TODO: xor	ah,ah
						//							uint8 temp3 = temp; // dx = ax
						//							temp = 0x3F;
						//							// TODO: Not sure if these should be 16 bit values
						//							temp -= temp3;
						//							temp3 = temp;
						//							temp = bp1; // TODO: xor	ah,ah
						//							// TODO: Should these be 16 bit?
						//							temp = temp * temp3;

						//							// TODO: cwd
						//							uint16 tempCX = 0x3F;
						//							temp /= tempCX;
						//							temp += temp2;
						//							temp3 = temp;
						//							// TODO: Need to read from pointer
						//							// les	di,[bp-14h]
						//							// mov al, es : [di]
						//							temp &= 0x3F; // TODO: xor ah,ah
						//							temp += temp3;
						//							bp2 = temp;
						//							// TODO: Need to pass arguments
						//							// push	word ptr [bp-0Eh]
						//							// push word ptr[bp - 10h]
						//							Func19BE_TODO(0x3);
						//							// TODO: Assign result to pointer
						//							// mov	[bp-14h],ax
						//							// mov[bp - 12h], dx
						//							temp = g225E; // TODO: xor ah,ah
						//							uint16 tempBX = temp;
						//							// TODO: Assign from pointer
						//							// les	di,[bp-14h]
						//							// mov al, es : [di]
						//							temp &= 0x3F; // TODO: xor ah, ah
						//							uint16 tempDX = temp;
						//							temp = 0x3F;
						//							temp -= tempDX;
						//							tempDX = temp;
						//							temp = bp1; // TODO: xor ah, ah
						//							temp *= tempDX;
						//							// TODO: CWD
						//							tempCX = 0x3F;
						//							temp /= tempCX;
						//							temp += tempBX;
						//							tempDX = temp;
						//							// TODO: Load from data
						//							// les	di,[bp-14h]
						//							// mov al, es : [di]
						//							temp &= 0x3F; // TODO: xor ah,ah
						//							temp += tempDX;
						//							bp1 = temp; // TODO: al part only
						//							if (bp1 > 0x3F) {
						//								// l0017_1DEC:
						//								bp1 = 0x3F;
						//							}
						//							// l0017_1DF0:
						//							if (bp2 > 0x3F) {
						//								// l0017_1DF6:
						//								bp2 = 0x3F;
						//							}
						//							// 1DFAh
						//							// TODO: Careful if argument are correct here,
						//							// I stumbled over the 2 pushed values before calling
						//							// the 1-arg function 2779
						//							Func2792(bp8 + 0xb0, 0);

						//							// TODO: Confirm that these are indeed identical
						//							uint8 arg1 = gArray96[bp8] + 0x40;
						//							uint8 arg2 = gArray96[bp8] + 0x40;

						//							uint8 result = Func2779(arg2);
						//							Func2792(arg1, (result & 0xC0) + bp1);
						//							result = Func2779(
						//								gArray8d[bp8] + 0x40);

						//							// Note that we again push one more copy of the
						//							// value which is not used in g2779 above.
						//							Func2792(gArray8d[bp8] + 0x40,
						//									 result & 0xC0 + bp2);
						//							// TODO: Do we need to do something about
						//							// the upper 8 bits?
						//							gArray226F[bp3] = 0;
						//							Func294E(bp8, bp4, gArray226F[bp3]);
						//						}
						//						// TODO: Figure out the logic here, I think
						//						// I got lost with jumps
						//						// l0017_1E91:
						//						// jmp 2095h
						//					}
						//				}
						}
					} else {
						//			// l0017_1E94:
						SIS_LogEntry(0x01D7, 0x1E94);
						//			// TODO: These pushes are not arguments for 19BE
						//			// push	word ptr [224Ah]
						//			// push word ptr[2248h]
						//			// TODO: xor ah, ah
						//			// Note: Original code returns a pointer via ax:dx, we just save the offset here so far
						//			// Original code saves results to bp10 and bp0E
						//			bp10 = Func19BE(gArray225F[bp3] << 0x4);
						//			// TODO: Several xor ah,ahs in here
						//			uint8 bp8 = gArray57[bp3 - 0xB];
						//			if (bp3 == 0xB) {
						//				// l0017_1ED1:
						//				// TODO: These pushes are not arguments for 19BE
						//				// push	word ptr [224Ah]
						//				// push word ptr[2248h]
						//				// TODO: Some xor ah,ah in here
						//				uint16 bp14 = Func19BE(gArray225F[bp3] << 0x4);

						//				// TODO: xor ah,ah
						//				Func2839(gArray5C[bp3 - 0xB], bp14);
						//
						//			} else {
						//				// l0017_1F12:
						//				// TODO: Not sure if the way of using [bp-10h] is working
						//				Func2792(bp8 + 0x20, peekByteAt(bp10));
						//				// TODO: Really need to rework 19BE implementation
						//				// TODO: Actual call and arguments for 19BE
						//				/*
						//				push	word ptr [bp-0Eh]
						//				push	word ptr [bp-10h]
						//				push	2h
						//				call	far 0017h:19BEh
						//				*/
						//				// TODO: Need to start applying these
						//				Common::MemorySeekableReadWriteStream streamBP0E(nullptr, 0);
						//				Common::MemorySeekableReadWriteStream r19BE = Func19BE_2(streamBP0E, 2);
						//				// TODO: For some reason, the argument for 2729 is pushed before
						//				// the call to 19BE above
						//				//
						//				Func2792(bp8 + 0x40, r19BE.readByte());
						//
						//				Common::MemorySeekableReadWriteStream r19BE_2 = Func19BE_2(streamBP0E, 0x4);
						//				Func2792(bp8 + 0x60, r19BE_2.readByte());

						//				Common::MemorySeekableReadWriteStream r19BE_3 = Func19BE_2(streamBP0E, 0x6);
						//				Func2792(bp8 + 0x80, r19BE_3.readByte());

						//				StreamHandler *shBP0E = new StreamHandler(&streamBP0E);
						//				// TODO: Who deletes these when?
						//				StreamHandler *r19BE_SH = Func19BE_SH(shBP0E, 0x8);
						//				Func2792(bp8 + 0xE0, r19BE_SH->readByte());
						//			}
						//			// l0017_1FA9:
						//			// TODO: xor ah,ah
						//			StreamHandler* shBP10 = Func19BE_SH(shMem2248, (gArray225F[bp3] << 0x4) + 0x3);
						//
						//			bp1 = gArray37[((shBP10->peekByte() & 0x3F) >> 0x4) << 0x3 + (bp5 >> 4)];
						//			bp1 += g225E;
						//			if (bp1 > 0x3F) {
						//				// l0017_200C:
						//				bp1 = 0x3F;
						//			}
						//			// l0017_2010:
						//			Func2792(gArray5C[bp3 - 0x0B], 0);
						//			uint8 r2779 = Func2779(bp8 + 0x40);
						//			Func2792(bp8 + 0x40, bp1 + (r2779 & 0xC0));

						//			Func2A80(gArray5C[bp3 - 0x0B], bp4, 0);
						//			uint8 bx = Func2779(0xBD);
						//			uint8 dx = bp3;
						//			dx = 0x0F - dx;
						//			Func2792(0xBD, (1 << dx) | bx);
						//			//    TODO: Continue from here
						//		}
						//		// TODO: This must be 2097h

						//		// TODO: I think I lost the indentation level for this one:
					}
					// l0017_2095:
					// jmp 209Bh
					// TODO: Not sure if this should really be an else
				} else {
					// l0017_2097
					SIS_LogEntry(0x01D7, 0x2097);
					// TODO: Confirm that the following code is correct
					bp6 = 0x80;
				}
			}
			// l0017_209B:
			if ((bp6 & 0xF0) == 0x80) {
				// l0017_20A7:
				SIS_LogEntry(0x01D7, 0x20A7);
				shMem2250 = Func19BE_SH(shMem2250, 0x2);
				g225A += 2;
				uint8 bp16 = g2291 - 1;
				if (0 <= bp16) {
					// l0017_20E1:
					SIS_LogEntry(0x01D7, 0x20E1);
					for (uint8 bp0A = 0; bp0A != bp16; bp0A++) {
						// l0017_20E9:
						if (gArray222C[bp0A] != 0) {
							// l0017_20F3:
							gArray222C[bp0A]++;
						}
					}
				}
				//		// l0017_2102 and l0017_2109:
				//		if (g2291 == 0x09 || bp3 < 0x0B) {
				//			// l0017_210F:
				//			uint8 bp8 = 0;
				//			// l0017_2114:
				//			while (g2291 > bp8) {
				//				// l0017_211E:
				//				// TODO: Continue here
				//				if (gArray222C[bp8] == 0) {
				//					// l0017_2128:
				//					if (gArray227F[bp8] == bp3) {
				//						// l0017_2134:
				//						if (gArray2235[bp8] == bp4) {
				//							break;
				//						}
				//					}
				//				}
				//				// l0017_2140:
				//				bp8++;
				//			}
				//			// l0017_2145:
				//			if (g2291 != bp8) {
				//				// l0017_214F:
				//				Func2A80(bp8, bp4, gArray226F[bp3]);
				//				gArray222C[bp8] = 1;
				//			}
				//		} else {
				//			// l0017_2172:
				//			uint8 bx = Func2779(0xBD);
				//			uint8 dx = bp3;
				//			uint8 ax = 0x0F - dx;
				//			dx = ax;
				//			ax = 1 << dx;
				//			dx = ax;
				//			ax = (0xFF - dx) & bx;
				//			Func2792(0xBD, ax);
				//		}
				//	}
			}
			//	// l0017_21A3 and l0017_21AC:
			if (((bp6 & 0xF0) == 0xE0) || (bp6 & 0xF0) == 0xA0 ) {
				// l0017_21B5:
				SIS_LogEntry(0x01D7, 0x21B5);
				shMem2250 = Func19BE_SH(shMem2250, 0x2);
				g225A += 0x2;
			}
			// l0017_21DF:
			if ((bp6 & 0xF0) == 0xB0) { // Scope ends 231E
				// l0017_21EB:
				SIS_LogEntry(0x01D7, 0x21EB);
				shMem2250 = Func19BE_SH(shMem2250, 0x2);
				g225A += 0x2;

				// Big if-else that ends at 231E
				if (bp4 == 0x66) {
					// l0017_221C:
					SIS_LogEntry(0x01D7, 0x221C);
					g2259 = bp5;
					g2258 = g2258 | 0x20;
					// l0017_222D:
				} else if (bp4 == 0x67) {
					SIS_LogEntry(0x01D7, 0x2231);
			//				// TODO: Continue from here
			//				// l0017_2231:
			//				if (bp5 != 0) {
			//					// l0017_2237:
			//					g2291 = 0x6;
			//					Func2792(0xBD, 0x20);
			//				} else {
			//					// l0017_2247:
			//					g2291 = 0x9;
			//					Func2792(0xBD, 0);
			//				} // l0017_2258:
						} else if (bp4 == 0x69) {
			//					// l0017_225C:
							SIS_LogEntry(0x01D7, 0x225C);
			//					// TODO: Check if the neg works out the right way
			//					bp5 = -bp5;
			//					gArray226F[bp3] = bp5;
			//					uint8 bp16 = g2291 - 1;
			//					if (0 <= bp16) {
			//						// l0017_2289:
			//						for (uint8 bp8 = 0; bp8 != bp16; bp8++) {
			//							// l0017_228C:
			//							if (gArray227F[bp8] != bp3) {
			//								continue;
			//							}
			//							// l0017_2298:
			//							if (gArray222C[bp8] != 0) {
			//								continue;
			//							}

			//							// l0017_22A2:
			//							Func294E(bp8, gArray2235[bp8], bp5);
			//						}
			//					}
							// l0017_22C1:
						} else if (bp4 == 0x68) {
			//					// l0017_22C5:
							SIS_LogEntry(0x01D7, 0x22C5);
			//					gArray226F[bp3] = bp5;
			//					uint16 bp16 = g2291 - 1;
			//					if (0 <= bp16) {
			//						// l0017_22E3:
			//						for (uint8 bp8 = 0; bp8 != bp16; bp8++) {
			//							// l0017_22EB:
			//							if (gArray227F[bp8] != bp3) {
			//								continue;
			//							}
			//							// l0017_22F7:
			//							if (gArray222C[bp8] != 0) {
			//								continue;
			//							}
			//							// l0017_2301:
			//							Func294E(bp8, gArray2235[bp8], bp5);
			//						}
			//					}
			//				}
			//
					
				}
			}
			// l0017_231E:
			if ((bp6 & 0xF0) == 0xC0) {
				// l0017_2327:
				SIS_LogEntry(0x01D7, 0x2327);
				shMem2250 = Func19BE_SH(shMem2250, 0x1);
				g225A++;
				gArray225F[bp3] = bp4;
			}
			// l0017_2355:
			if ((bp6 & 0xF0) == 0xD0) {
			//		// l0017_235E:
				SIS_LogEntry(0x01D7, 0x235E);
			//		Macs2::StreamHandler *sh2252;
			//		Macs2::StreamHandler *sh225A;
			//		Macs2::StreamHandler *shResult = Func19BE_SH(sh2252, 0x1);
			//		sh2252 = shResult;
			//		// TODO: Check if this is the right way to handle the plus operation
			//		sh225A->seek(1, SEEK_CUR);
			}
			// l0017_237E:
			if ((bp6 & 0xF0) == 0xF0) {
			//		// l0017_2387:
				SIS_LogEntry(0x01D7, 0x2387);
			//		if (bp4 == 0x2F) {
			//			// l0017_238D:
			//			Macs2::StreamHandler *sh2244;
			//			Macs2::StreamHandler *sh2250;
			//			sh2250 = sh2244;
			//			// TODO: Setting 225C and 225A to 0 - ?
			//			// TODO: Setting 2259 and 2242 to 1 - Probably understood these wrong
			//			// mov	byte ptr [2259h],0h
			//			// mov byte ptr[2242h], 1h
			//			Func1A03();
			//		} else {
			//			// l0017_23B4:
			//			// TODO: Identical code as the previous branch?
			//		}
			} else {
				// l0017_23DB:
				SIS_LogEntry(0x01D7, 0x23DB);
				Func1A03();
			}
			// l0017_23E0:

			// Deepseek R1 figured out that this part was just checking
			// this inequality by using the msw and lsw
			if (_nextEventTimer > 0x0FFF) {
				// l0017_23F1:
				SIS_LogEntry(0x01D7, 0x23F1);
				shMem2250 = shMem2244;
				g225A = 0;
				g2259 = 0;
				g2242 = 1;
				Func1A03();
			}
			
			// l0017_2416:
			if (_nextEventTimer != 0) {
				break;
			}
		}
	}

	// l0017_2422
	// TODO: sti

	// l0017_2425:
	if ((g2258 & 0xC2) != 0) {
		// l0017_242E:
		SIS_LogEntry(0x01D7, 0x242E);
		// TODO: I think this just calls the function again
		// Func1A74();
	}
	// l0017_2433:
	Func27E4();

	// l0017_2438:
	if (g229A == 0) {
		// l0017_243F
		// TODO
		// mov al,20h
		// out 20h,al
	}
	// l0017_2443:
	// Just epilogue and interrupt return
}

StreamHandler *Adlib::Func19BE_SH(StreamHandler *inHandler, uint16 seekDelta) {
	StreamHandler *result = new StreamHandler(*inHandler);
	uint16 pos = result->pos();
	if (seekDelta > 0xFFF8) {
		// l0017_19D8:
		// TODO: Confirm that this works as expected and if it is every used in
		// the actual game
		pos &= 0xF;
	}
	// l0017_19EA
	pos += seekDelta;
	result->seek(pos, SEEK_SET);
	debug("Adlib seek by %.2x new offset %04x",
		  seekDelta, pos);

	return result;
}

void Adlib::Func244D(StreamHandler *song) {
	// TODO: No idea yet what [0036] does
	/*

	cmp	byte ptr [0036h],0h
	jz	2460h
	
l0017_2458:
	mov	word ptr [bp-2h],3h
	jmp	24F6h
	*/
	StreamHandler* sh = Func19BE_SH(song, 0x6);
	uint16 delta = sh->peekWord();
	shMem2248 = Func19BE_SH(song, delta);
	sh = Func19BE_SH(song, 0x8);
	delta = sh->peekWord();
	shMem2244 = Func19BE_SH(song, delta);
	sh = Func19BE_SH(song, 0x24);
	g2240 = sh->peekWord();
	sh = Func19BE_SH(song, 0xC);
	g224E = sh->peekWord();
	Func24FD();
	// TODO: We should in theory return the resut of 24FD
	/*
	call	far 0017h:24FDh
	mov	[bp-2h],ax

l0017_24F6:
	mov	ax,[bp-2h]
	leave
	retf	4h*/
}


void Adlib::SIS_LogEntry(uint16 seg, uint16 off, Common::String msg) {
	if (msg.empty()) {
		debug("Entered %.4X:%.4X", seg, off);
	} else {
		debug(msg.c_str());
	}
}

void Adlib::Func1A03() {

	_nextEventTimer = 0;
	uint8 bp1;
	// l0017_1A0F:
	uint8 continueCondition;
	do {
		bp1 = shMem2250->peekByte();
		
		// TODO: Not sure what this does in practice
		_nextEventTimer = _nextEventTimer << 7;
		_nextEventTimer += bp1 & 0x7F;
		shMem2250 = Func19BE_SH(shMem2250, 1);
		g225A++;
		continueCondition = bp1 & 0x80;
		debug("1A03 iteration - Value: %.2x at offset %.4x Next timer: %.8x Continuation: %.2x", bp1, shMem2250->pos(), _nextEventTimer, continueCondition);
	} while (continueCondition != 0);
}

void Adlib::Func2A80(uint8 blend_param, uint8 index, uint8 reg_base) {
	// AI-reverse engineered by Deepseek
	// Initial value calculation
	// [bp-2h]
	uint16 base_value = (gArray11F[index] << 8) | gArray9F[index];
	if (blend_param != 0) {
		// l0017_2AB1:
		if (blend_param < 0x80) { // Forward blend case
			// l0017_2AB7:
			// Calculate next index with clamping
			// l0017_2ABD: and l0017_2AC8:
			// [bp-6h]
			uint8 next_idx = (index < 0x7F) ? index + 1 : 0x7F;
			// l0017_2ACD:
			// [bp-4h]
			uint16 next_value = (gArray11F[next_idx] << 8) | gArray9F[next_idx];
			// Linear interpolation - handled in register only in assembler
			int16 delta = next_value - base_value;
			base_value += static_cast<uint16>((delta * blend_param) / 7);
		} else { // Reverse blend case
			// l0017_2B1D:
			// Calculate previous index with clamping
			// [bp-6h]
			uint8 prev_idx = (index > 0) ? index - 1 : 0;
			// l0017_2B33:
			uint16 prev_value = (gArray11F[prev_idx] << 8) | gArray9F[prev_idx];

			// Reverse interpolation
			int16 delta = base_value - prev_value;
			base_value -= static_cast<uint16>((delta * blend_param) / 7);
		}
	}

	// l0017_2B81:
	// Write results to sound chip registers
	uint8 low_reg = 0xA0 + reg_base;  // Low byte register
	uint8 high_reg = 0xB0 + reg_base; // High byte register

	// Write low byte (raw value)
	Func2792(low_reg, static_cast<uint8>(base_value & 0xFF));

	// Write high byte (masked to clear bit 5)
	Func2792(high_reg, static_cast<uint8>((base_value >> 8) & 0xDF));
}

void Adlib::Init() {
	_opl = OPL::Config::create();
	int status = _opl->init();

	gArray229C.resize(256);

	gArray225F.resize(0x10);
	gArray226F.resize(0x10);

	gArray222C.resize(0x9);
	gArray227F.resize(0x9);
	gArray2288.resize(0x9);
	gArray2235.resize(0x9);

#define CALLBACKS_PER_SECOND 10
	_opl->start(new Common::Functor0Mem<void, Adlib>(this, &Adlib::OnTimer), CALLBACKS_PER_SECOND);

	// Func1A03();

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
	// Func24FD();
	// TODO: Consider adding the caller
	// TODO: Add proper arguments here
	// Func2839(0, 0);

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

void Adlib::SetSong(Macs2::StreamHandler *sh) {
	shMem2250 = sh;
	Func244D(shMem2250);
}

void Adlib::ReadDataFromExecutable(Common::MemoryReadStream *fileStream) {
	// TODO: Figure out the actual sizes
	constexpr uint32 size = 255;
	gArray69.resize(size);
	LoadData(fileStream, 0x0001B669, size, gArray69.data());

	gArray8d.resize(size);
	LoadData(fileStream, 0x0001B68D, size, gArray8d.data());

	gArray96.resize(size);
	LoadData(fileStream, 0x0001B696, size, gArray96.data());

}

void Adlib::LoadData(Common::MemoryReadStream *fileStream, int64 pos, uint16 size, void *target) {
	fileStream->seek(pos, SEEK_SET);
	fileStream->read(target, size);
}

inline StreamHandler::StreamHandler(Common::MemorySeekableReadWriteStream *s) : _stream(s), _pos(s->pos()) {
}

StreamHandler::StreamHandler(Common::Array<uint8> *data) {
	_stream = new Common::MemorySeekableReadWriteStream(data->data(), data->size());
	_pos = _stream->pos();
}

bool StreamHandler::eos() const {
	_stream->seek(_pos);
	return _stream->eos();
}

uint32 StreamHandler::read(void *dataPtr, uint32 dataSize) {
	_stream->seek(_pos);
	return _stream->read(dataPtr, dataSize);
}

int64 StreamHandler::pos() const {
	return _pos;
}

int64 StreamHandler::size() const {
	return _stream->size();
}

bool StreamHandler::seek(int64 offset, int whence) {
	_stream->seek(_pos, SEEK_SET);
	bool result = _stream->seek(offset, whence);
	_pos = _stream->pos();
	return result;
}

byte StreamHandler::peekByte() {
	_stream->seek(_pos, SEEK_SET);
	byte result = readByte();
	return result;
}

byte StreamHandler::peekByteAtOffset(int64 offset, int whence) {
	int64 oldPos = _pos;
	seek(offset, whence);
	byte result = peekByte();
	seek(oldPos, SEEK_SET);
	return result;
}

uint16 StreamHandler::peekWord() {
	_stream->seek(_pos, SEEK_SET);
	uint16 result = readUint16LE();
	return result;
}

} // End of namespace Macs2
