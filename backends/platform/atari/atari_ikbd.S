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

 #include "symbols.h"

	.global	SYM(atari_kbdvec)
	.global	SYM(atari_mousevec)

	.global	SYM(atari_old_kbdvec)
	.global	SYM(atari_old_mousevec)

	.extern	SYM(g_atari_ikbd_mousebuttons)
	.extern	SYM(g_atari_ikbd_mousebuttons_mask)
	.extern	SYM(g_atari_ikbd_mousebuttons_head)

	.extern	SYM(g_atari_ikbd_mouse_delta_x)
	.extern	SYM(g_atari_ikbd_mouse_delta_y)

	.extern	SYM(g_atari_ikbd_scancodes)
	.extern	SYM(g_atari_ikbd_scancodes_mask)
	.extern	SYM(g_atari_ikbd_scancodes_head)

	.text

	.ascii	"XBRA"
	.ascii	"SCUM"
SYM(atari_old_kbdvec):
	dc.l	0
SYM(atari_kbdvec):
	lea	pressed_keys,a0
	clr.l	d1
	move.b	d0,d1
	bpl.b	key_pressed				| bit 7 cleared

key_released:
	and.b	#0x7f,d1
	tst.b	(a0,d1.l)				| pressed before?
	bne.b	key_released_ok

	| if we get a sudden release key event,
	| let the original handler process it
	move.l	(atari_old_kbdvec,pc),a0
	jmp	(a0)

key_released_ok:
	clr.b	(a0,d1.l)				| mark as released
	bra.b	kbdvec_process

key_pressed:
	addq.b	#1,(a0,d1.l)				| mark as pressed

kbdvec_process:
	lea	SYM(g_atari_ikbd_scancodes),a0
	move.w	SYM(g_atari_ikbd_scancodes_head),d1

	| g_atari_ikbd_scancodes[g_atari_ikbd_scancodes_head] = scancode

	move.b	d0,(0.b,a0,d1.w)

	addq.l	#1,d1
	and.w	SYM(g_atari_ikbd_scancodes_mask),d1
	move.w	d1,SYM(g_atari_ikbd_scancodes_head)
	rts


	.ascii	"XBRA"
	.ascii	"SCUM"
SYM(atari_old_mousevec):
	dc.l	0
SYM(atari_mousevec):
	move.b	(a0)+,d0
	cmp.b	old_buttons,d0
	beq.b	no_buttons

	move.b	d0,old_buttons

	lea	SYM(g_atari_ikbd_mousebuttons),a1
	move.w	SYM(g_atari_ikbd_mousebuttons_head),d1

	| g_atari_ikbd_mousebuttons[g_atari_ikbd_mousebuttons_head] = buttons

	move.b	d0,(0.b,a1,d1.w)

	addq.w	#1,d1
	and.w	SYM(g_atari_ikbd_mousebuttons_mask),d1
	move.w	d1,SYM(g_atari_ikbd_mousebuttons_head)

no_buttons:
	move.b	(a0)+,d0
	ext.w	d0
	add.w	d0,SYM(g_atari_ikbd_mouse_delta_x)

	move.b	(a0)+,d0
	ext.w	d0
	add.w	d0,SYM(g_atari_ikbd_mouse_delta_y)
	rts

	.bss

pressed_keys:
	ds.b	128
old_buttons:
	ds.b	1
