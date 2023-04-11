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
 * but WITHOUT ANY WARRANTY| without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

	.globl	_asm_screen_tt_save
	.globl	_asm_screen_falcon_save

	.globl	_asm_screen_tt_restore
	.globl	_asm_screen_falcon_restore

	.globl	_asm_screen_set_tt_palette
	.globl	_asm_screen_set_falcon_palette

	.globl	_asm_screen_set_vram
	.globl	_asm_screen_set_scp_res

	.text

| extern void asm_screen_tt_save(void);
|
_asm_screen_tt_save:
	bsr	wait_vbl			| avoid flickering

	lea	0xffff8400.w,a0
	lea	save_pal,a1
	moveq	#256/2-1,d0

tt_save_loop:
	move.l	(a0)+,(a1)+
	dbra	d0,tt_save_loop

	lea	save_video,a1
	move.l	0xffff8200.w,(a1)+		| vidhm
	move.w	0xffff820c.w,(a1)+		| vidl
	move.w	0xffff8262.w,(a1)+		| tt shifter
	rts

| extern void asm_screen_falcon_save(void);
|
_asm_screen_falcon_save:
	movem.l	d2-d7/a2,-(sp)

	bsr	wait_vbl			| avoid flickering

	lea	0xffff9800.w,a0			| save falcon palette
	lea	save_pal,a1			|
	moveq	#256/2-1,d7			|
						|
falcon_save_loop:
	move.l	(a0)+,(a1)+			|
	move.l	(a0)+,(a1)+			|
	dbra	d7,falcon_save_loop		|

	movem.l	0xffff8240.w,d0-d7		| save st palette
	movem.l	d0-d7,(a1)			|

	lea	save_video,a0
	move.l	0xffff8200.w,(a0)+		| vidhm
	move.w	0xffff820c.w,(a0)+		| vidl

	move.l	0xffff8282.w,(a0)+		| h-regs
	move.l	0xffff8286.w,(a0)+		|
	move.l	0xffff828a.w,(a0)+		|

	move.l	0xffff82a2.w,(a0)+		| v-regs
	move.l	0xffff82a6.w,(a0)+		|
	move.l	0xffff82aa.w,(a0)+		|

	move.w	0xffff82c0.w,(a0)+		| vco
	move.w	0xffff82c2.w,(a0)+		| c_s

	move.l	0xffff820e.w,(a0)+		| offset+width
	move.w	0xffff820a.w,(a0)+		| sync

	move.b	0xffff8265.w,(a0)+		| p_o

	cmpi.w	#0xb0,0xffff8282.w		| st(e) / falcon test
	sle	(a0)+				| it's a falcon resolution

	move.w	0xffff8266.w,(a0)+		| f_s
	move.w	0xffff8260.w,(a0)+		| st_s

	| install custom VBL handler
	move	sr,-(sp)
	or	#0x700,sr

	move.l	0x70.w,old_vbl
	move.l	#vbl,0x70.w

	move	(sp)+,sr

	movem.l	(sp)+,d2-d7/a2
	rts

| extern void asm_screen_tt_restore(void);
|
_asm_screen_tt_restore:
	bsr	wait_vbl			| avoid flickering

	lea	save_video,a1
	move.l	(a1)+,0xffff8200.w		| vidhm
	move.w	(a1)+,0xffff820c.w		| vidl
	move.w	(a1)+,0xffff8262.w		| tt shifter

	lea	save_pal,a0
	lea	0xffff8400.w,a1
	moveq	#256/2-1,d0

.loop:		move.l	(a0)+,(a1)+
	dbra	d0,.loop
	rts

| extern void asm_screen_falcon_restore(void);
|
_asm_screen_falcon_restore:
	movem.l	d2-d7/a2,-(sp)

	| uninstall custom VBL handler
	move	sr,-(sp)
	or	#0x700,sr

	move.l	old_vbl,0x70.w

	move	(sp)+,sr

	bsr	wait_vbl			| avoid flickering

	lea	save_video,a0

	move.l	(a0)+,0xffff8200.w		| videobase_address:h&m
	move.w	(a0)+,0xffff820c.w		| l

	move.l	(a0)+,0xffff8282.w		| h-regs
	move.l	(a0)+,0xffff8286.w		|
	move.l	(a0)+,0xffff828a.w		|

	move.l	(a0)+,0xffff82a2.w		| v-regs
	move.l	(a0)+,0xffff82a6.w		|
	move.l	(a0)+,0xffff82aa.w		|

	move.w	(a0)+,0xffff82c0.w		| vco
	move.w	(a0)+,0xffff82c2.w		| c_s

	move.l	(a0)+,0xffff820e.w		| offset+width
	move.w	(a0)+,0xffff820a.w		| sync

	move.b	(a0)+,0xffff8265.w		| p_o

	tst.b	(a0)+				| st(e) compatible mode?
	bne	falcon_restore_st_comp		| yes

falcon_restore_falcon:
	move.l	a0,-(sp)
	bsr	wait_vbl			| Patch to avoid
	clr.w	0xffff8266.w			| monochrome sync errors
	bsr	wait_vbl			| (ripped from
	move.l	(sp)+,a0			| FreeMiNT kernel,
	move.w	(a0),0xffff8266.w		| by Draco/Yescrew)

	bra	falcon_restore_restored

falcon_restore_st_comp:
	move.w	(a0)+,0xffff8266.w		| falcon-shift
	move.w	(a0),0xffff8260.w		| st-shift
	lea	save_video,a0
	move.w	32(a0),0xffff82c2.w		| c_s
	move.l	34(a0),0xffff820e.w		| offset+width

falcon_restore_restored:
	lea	save_pal,a0			| restore falcon palette
	lea	0xffff9800.w,a1			|
	moveq	#128-1,d7			|
						|
falcon_restore_loop:
	move.l	(a0)+,(a1)+			|
	move.l	(a0)+,(a1)+			|
	dbra	d7,falcon_restore_loop		|

	movem.l	(a0),d0-d7			| restore st palette
	movem.l	d0-d7,0xffff8240.w		|

	movem.l	(sp)+,d2-d7/a2
	rts

| extern void asm_screen_set_tt_palette(const uint16 pPalette[256]);
|
_asm_screen_set_tt_palette:
	move.l	(4,sp),a0
	lea	0xffff8400.w,a1
	moveq	#256/2-1,d0

set_tt_palette_loop:
	move.l	(a0)+,(a1)+
	dbra	d0,set_tt_palette_loop
	rts

| extern void asm_screen_set_falcon_palette(const byte pPalette[256*3]);
|
_asm_screen_set_falcon_palette:
	move.l	(4,sp),a0
	lea	pending_palette,a1
	move.w	#256-1,d0

set_falcon_palette_loop:
	clr.l	d1
	move.w	(a0)+,d1
	swap	d1
	move.b	(a0)+,d1
	move.l	d1,(a1)+
	dbra	d0,set_falcon_palette_loop

	addq.w	#1,has_pending_palette
	rts

| extern void asm_screen_set_vram(const void* pScreen);
|
_asm_screen_set_vram:
	move.l	(4,sp),pending_vram

	addq.w	#1,has_pending_vram
	rts

| void asm_screen_set_scp_res(const void* pScp);
|
_asm_screen_set_scp_res:
	move.l	(4,sp),a0
	lea	(122,a0),a0
	lea	pending_scp,a1
	moveq	#(158-122)/4-1,d0
set_scp_res_loop:
	move.l	(a0)+,(a1)+
	dbra	d0,set_scp_res_loop

	addq.w	#1,has_pending_scp
	rts

wait_vbl:
	move.w	#0x25,-(sp)			| Vsync()
	trap	#14				|
	addq.l	#2,sp				|
	rts

	.ascii	"XBRA"
	.ascii	"SCUM"
old_vbl:
	dc.l	0
vbl:
	movem.l	d0-d1/a0-a1,-(sp)

	addq.l	#1,0x0462.w			| _vbclock
	addq.l	#1,0x0466.w			| _frclock

	tst.w	has_pending_scp
	beq.b	vbl_no_pending_scp

	lea	pending_scp,a0
	move.l	(a0)+,0xffff8282.w
	move.l	(a0)+,0xffff8286.w
	move.l	(a0)+,0xffff828a.w
	move.l	(a0)+,0xffff82a2.w
	move.l	(a0)+,0xffff82a6.w
	move.l	(a0)+,0xffff82aa.w
	move.w	(a0)+,0xffff820a.w
	move.w	(a0)+,0xffff82c0.w
	clr.w	0xffff8266.w
	tst.w	(a0)+
	bne.b	vbl_st

vbl_falcon:
	move.w	(a0)+,0xffff8266.w
	bra.b	vbl_skip

vbl_st:	addq.l	#1,a0
	move.b	(a0)+,0xffff8260.w

vbl_skip:
	move.w	(a0)+,0xffff82c2.w
	move.w	(a0)+,0xffff8210.w

	clr.w	has_pending_scp
vbl_no_pending_scp:

	tst.w	has_pending_vram
	beq.b	vbl_no_pending_vram

	move.l	pending_vram,d0
	move.l	d0,d1
	lsr.w	#8,d0
	move.l	d0,0xffff8200.w
	move.b	d1,0xffff820d.w

	clr.w	has_pending_vram
vbl_no_pending_vram:

	tst.w	has_pending_palette
	beq.b	vbl_no_pending_palette

	lea	pending_palette,a0
	lea	0xffff9800.w,a1
	moveq	#256/2-1,d0

vbl_falcon_palette_loop:
	move.l	(a0)+,(a1)+
	move.l	(a0)+,(a1)+
	dbra	d0,vbl_falcon_palette_loop

	clr.w	has_pending_palette

vbl_no_pending_palette:
	movem.l	(sp)+,d0-d1/a0-a1
	rte


	.bss
	.even

has_pending_scp:
	ds.w	1
has_pending_vram:
	ds.w	1
has_pending_palette:
	ds.w	1

pending_scp:
	ds.b	158-122				| pending SCP resolution data
pending_vram:
	ds.l	1				| pending vram pointer
pending_palette:
	ds.l	256				| pending palette

save_pal:
	ds.l	256+16/2			| old colours (sized for falcon+ste palette)
save_video:
	ds.b	32+12+2				| old video regs (size of falcon regs)
