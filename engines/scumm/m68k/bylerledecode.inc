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

| kate: tab-width 8; replace-tabs off; hl Motorola 68k (VASM/Devpac);

#include "../../../backends/platform/atari/symbols.h"

	.struct	0
compData.x:
	.space	4	// int x;
compData.y:
	.space	4	// int y;
compData.scaleTable:
	.space	4	// const byte *scaleTable;
//compData.height:
//	.space	4	// int height;
//compData.width:
//	.space	4	// int width;

	.space	2
compData.skipWidth:
	.space	2	// int skipWidth; (used as uint16)

compData.destPtr:
	.space	4	// byte *destPtr;
compData.maskPtr:
	.space	4	// const byte *maskPtr;
compData.scaleXStep:
	.space	4	// int scaleXStep;
compData.mask:
	.space	1	// byte mask;
compData.shr:
	.space	1	// byte shr;
compData.repColor:
	.space	1	// byte repColor;
compData.repLen:
	.space	1	// byte repLen;
compData.boundsRect:
	.space	8	// Common::Rect boundsRect; (4 x int16)
compData.scaleXIndex:
	.space	4	// int scaleXIndex;
compData.scaleYIndex:
	.space	4	// int scaleYIndex;
compData.scaleIndexMask:
	.space	4	// int scaleIndexMask;

	.set	sizeof_local_vars,0
	.macro	add_var name,size
	.set	sizeof_local_vars,sizeof_local_vars+\size
	.set	\name,-sizeof_local_vars
	.endm

	.struct 0
arg_fp:
	.space	4
arg_rts:
	.space	4
arg_pcompData:
	.space	4	// BaseCostumeRenderer::ByleRLEData *pcompData

	.space	3
arg_scaleX:
	.space	1	// const byte _scaleX

	.space	3
arg_scaleY:
	.space	1	// const byte _scaleY

	.space	2
arg_height:
	.space	2	// const int _height (used as uint16)

arg_pitch:
	.space	4	// const int pitch
arg_numStrips:
	.space	4	// const int _numStrips
arg_srcPtr:
	.space	4	// const byte *_srcPtr
arg_shadowTable:
	.space	4	// const byte *_shadowTable
arg_palette:
	.space	4	// const uint16 *_palette
