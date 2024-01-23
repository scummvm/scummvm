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

namespace Macs2 {

	

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

l0017_2751:
	inc	word ptr [bp-4h]

l0017_2754:
	mov	al,[bp-4h]
	push	ax
	push	0h
	call	far 0017h:2792h
	cmp	word ptr [bp-4h],0FFh
	jnz	2751h

l0017_2766:
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



void Adlib::Func2792(byte registerIndex, byte value) {
	// fn0017_2792 proc
	_opl->write(0x388, registerIndex);

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

void Adlib::Init() {
	_opl = OPL::Config::create();
}

} // End of namespace Macs2
