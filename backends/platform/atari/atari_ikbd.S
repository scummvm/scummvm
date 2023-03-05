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

/*
 *	IKBD 6301 interrupt routine
 *
 *	Patrice Mandin
 */

	.globl	_atari_ikbd_init
	.globl	_atari_ikbd_shutdown

	.globl	_g_atari_ikbd_mouse_buttons_state
	.globl	_g_atari_ikbd_mouse_delta_x
	.globl	_g_atari_ikbd_mouse_delta_y

	.globl	_g_atari_ikbd_scancodes
	.globl	_g_atari_ikbd_scancodes_size
	.globl	_g_atari_ikbb_scancodes_head


	.text

_atari_ikbd_init:
	| Disable interrupts

	movew	sr,d1
	movew	#0x2700,sr

	| Save MFP registers used for keyboard

	lea	0xfffffa00:w,a0
	btst	#6,a0@(0x09)
	sne	ikbd_ierb
	btst	#6,a0@(0x15)
	sne	ikbd_imrb

	| Set our routine

	movel	0x118:w,old_ikbd
	movel	#ikbd,0x118:w
	bset	#6,0xfffffa09:w	| IERB
	bset	#6,0xfffffa15:w	| IMRB

	| Set mouse relative mode

	moveb	#8,0xfffffc02:w

	| Reenable interrupts

	movew	d1,sr
	rts

_atari_ikbd_shutdown:
	| Disable interrupts

	movew	sr,d1
	movew	#0x2700,sr

	| Restore previous MFP registers

	lea	0xfffffa00:w,a0

	bclr	#6,a0@(0x09)
	tstb	ikbd_ierb
	beqs	ikbd_restoreierb
	bset	#6,a0@(0x09)
ikbd_restoreierb:

	bclr	#6,a0@(0x15)
	tstb	ikbd_imrb
	beqs	ikbd_restoreimrb
	bset	#6,a0@(0x15)
ikbd_restoreimrb:

	movel	old_ikbd,0x118:w

	| Clear keyboard buffer

	lea	0xfffffc00:w,a0
ikbd_videbuffer:
	btst	#0,a0@
	beqs	ikbd_finbuffer
	tstb	a0@(0x02)
	bras	ikbd_videbuffer
ikbd_finbuffer:

	| Reenable interrupts

	movew	d1,sr
	rts

	.bss

	.even
ikbd_ierb:
	.ds.b	1
ikbd_imrb:
	.ds.b	1

/*--- Our custom IKBD vector ---*/

	.text
	.even
	.ascii	"XBRA"
	.ascii	"SCUM"
old_ikbd:
	.dc.l	0
ikbd:
	moveml	d0-d1/a0,sp@-

	| Check if source is IKBD or MIDI

	btst	#0,0xfffffc00.w
	beqs	ikbd_oldmidi

	moveb	0xfffffc02:w,d0

	| Not supported packets ?

	| status report
	cmpb	#0xf6,d0
	beqs	ikbd_endit_stack
	| absolute mouse position record
	cmpb	#0xf7,d0
	beqs	ikbd_endit_stack
	| time-of-day
	cmpb	#0xfc,d0
	beqs	ikbd_endit_stack

	| Joystick packet ?

	| joystick report (both sticks), joystick 0 event, joystick 1 event
	cmpb	#0xfd,d0
	bhss	ikbd_endit_stack

	| Mouse packet ?

	cmpb	#0xf8,d0
	blos	ikbd_no_mouse
	cmpb	#0xfc,d0
	bhss	ikbd_no_mouse

	| Mouse packet, byte #1

ikbd_yes_mouse:
	andw	#3,d0
	moveb	d0,_g_atari_ikbd_mouse_buttons_state

	movel	#ikbd_mousex,0x118:w
	bras	ikbd_endit_stack

	| Keyboard press/release

ikbd_no_mouse:
	lea	_g_atari_ikbd_scancodes,a0
	movew	_g_atari_ikbb_scancodes_head,d1

	| g_atari_ikbd_scancodes[g_atari_ikbb_scancodes_head] = scancode

	moveb	d0,(0.b,a0,d1.w)

	addql	#1,d1
	andw	_g_atari_ikbd_scancodes_mask,d1
	movew	d1,_g_atari_ikbb_scancodes_head

	| End of interrupt

ikbd_endit_stack:
	moveml	sp@+,d0-d1/a0

	bclr	#6,0xfffffa11:w
	rte

	| Call old MIDI interrupt

ikbd_oldmidi:
	moveml	sp@+,d0-d1/a0

	movel	old_ikbd,sp@-
	rts

	| Mouse packet, byte #2

ikbd_mousex:
	moveml	d0-d1/a0,sp@-

	| Check if source is IKBD or MIDI
	btst	#0,0xfffffc00.w
	beqs	ikbd_oldmidi

	moveb	0xfffffc02:w,d0
	extw	d0
	addw	d0,_g_atari_ikbd_mouse_delta_x

	movel	#ikbd_mousey,0x118:w
	bras	ikbd_endit_stack

	| Mouse packet, byte #3

ikbd_mousey:
	moveml	d0-d1/a0,sp@-

	| Check if source is IKBD or MIDI

	btst	#0,0xfffffc00.w
	beqs	ikbd_oldmidi

	moveb	0xfffffc02:w,d0
	extw	d0

	addw	d0,_g_atari_ikbd_mouse_delta_y

	movel	#ikbd,0x118:w
	bras	ikbd_endit_stack
