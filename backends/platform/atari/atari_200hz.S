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

	.globl	_atari_200hz_init
	.globl	_atari_200hz_shutdown

	.globl	_counter_200hz

	.text

_atari_200hz_init:
	move	sr,-(sp)
	or	#0x700,sr

	move.l	0x114.w,old_200hz
	move.l	#my_200hz,0x114.w

	move	(sp)+,sr
	rts

_atari_200hz_shutdown:
	move	sr,-(sp)
	or	#0x700,sr

	move.l	old_200hz,0x114.w

	move	(sp)+,sr
	rts

	.ascii	"XBRA"
	.ascii	"SCUM"
old_200hz:
	dc.l	0
my_200hz:
	addq.l	#1,_counter_200hz

	move.l	old_200hz,-(sp)
	rts


	.bss
	.even

_counter_200hz:
	ds.l	1
