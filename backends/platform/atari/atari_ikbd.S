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

	.global	_atari_kbdvec
	.global	_atari_mousevec
	.global	_atari_vkbderr

	.extern	_g_atari_ikbd_mouse_buttons_state
	.extern	_g_atari_ikbd_mouse_delta_x
	.extern	_g_atari_ikbd_mouse_delta_y

	.extern	_g_atari_ikbd_scancodes
	.extern	_g_atari_ikbd_scancodes_mask
	.extern	_g_atari_ikbb_scancodes_head

	.extern	_g_atari_old_kbdvec

	.text

_atari_kbdvec:
	tst.w	(vkbderr_count,pc)
	bne.b	kbdvec_end

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
	move.l	_g_atari_old_kbdvec,a0
	tst.l	a0
	beq.b	kbdvec_end
	jmp	(a0)

key_released_ok:
	clr.b	(a0,d1.l)				| mark as released
	bra.b	kbdvec_process

key_pressed:
	addq.b	#1,(a0,d1.l)				| mark as pressed

kbdvec_process:
	lea	_g_atari_ikbd_scancodes,a0
	move.w	_g_atari_ikbb_scancodes_head,d1

	| g_atari_ikbd_scancodes[g_atari_ikbb_scancodes_head] = scancode

	move.b	d0,(0.b,a0,d1.w)

	addq.l	#1,d1
	and.w	_g_atari_ikbd_scancodes_mask,d1
	move.w	d1,_g_atari_ikbb_scancodes_head

kbdvec_end:
	rts


_atari_vkbderr:
	addq.w	#1,vkbderr_count
	rts


_atari_mousevec:
	clr.w	vkbderr_count

	move.b	(a0)+,_g_atari_ikbd_mouse_buttons_state

	move.b	(a0)+,d0
	ext.w	d0
	add.w	d0,_g_atari_ikbd_mouse_delta_x

	move.b	(a0)+,d0
	ext.w	d0
	add.w	d0,_g_atari_ikbd_mouse_delta_y
	rts

// place it within reach of 32K (PC relative)
vkbderr_count:
	dc.w	0


	.bss

pressed_keys:
	ds.b	128
