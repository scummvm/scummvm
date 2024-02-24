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

	.extern	SYM(g_atari_ikbd_mouse_buttons_state)
	.extern	SYM(g_atari_ikbd_mouse_delta_x)
	.extern	SYM(_atari_ikbd_mouse_delta_y)

	.extern	SYM(g_atari_ikbd_scancodes)
	.extern	SYM(g_atari_ikbd_scancodes_mask)
	.extern	SYM(g_atari_ikbb_scancodes_head)

	.text

	.ascii	"XBRA"
	.ascii	"SCUM"
SYM(atari_old_kbdvec):
	dc.l	0
SYM(atari_kbdvec):
	movem.l	d1/a0,-(sp)

	lea	SYM(g_atari_ikbd_scancodes),a0
	move.w	SYM(g_atari_ikbb_scancodes_head),d1

	| g_atari_ikbd_scancodes[g_atari_ikbb_scancodes_head] = scancode

	move.b	d0,(0.b,a0,d1.w)

	addq.l	#1,d1
	and.w	SYM(g_atari_ikbd_scancodes_mask),d1
	move.w	d1,SYM(g_atari_ikbb_scancodes_head)

	movem.l	(sp)+,d1/a0
	move.l	(atari_old_kbdvec,pc),-(sp)
	rts


	.ascii	"XBRA"
	.ascii	"SCUM"
SYM(atari_old_mousevec):
	dc.l	0
SYM(atari_mousevec):
	movem.l	d0/a0,-(sp)

	move.b	(a0)+,SYM(g_atari_ikbd_mouse_buttons_state)

	move.b	(a0)+,d0
	ext.w	d0
	add.w	d0,SYM(g_atari_ikbd_mouse_delta_x)

	move.b	(a0)+,d0
	ext.w	d0
	add.w	d0,SYM(g_atari_ikbd_mouse_delta_y)

	movem.l	(sp)+,d0/a0
	move.l	(atari_old_mousevec,pc),-(sp)
	rts
