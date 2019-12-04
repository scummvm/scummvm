/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef ULTIMA8_WORLD_LOOPSCRIPT_H
#define ULTIMA8_WORLD_LOOPSCRIPT_H

// Script Tokens
#define LS_TOKEN_AND            '&'
#define LS_TOKEN_OR             '+'
#define LS_TOKEN_NOT            '!'
#define LS_TOKEN_EQUAL          '='
#define LS_TOKEN_GREATER        '>'
#define LS_TOKEN_LESS           '<'
#define LS_TOKEN_GEQUAL         ']'
#define LS_TOKEN_LEQUAL         '['

#define LS_TOKEN_INT            '%'

#define LS_TOKEN_TRUE           1
#define LS_TOKEN_FALSE          0
#define LS_TOKEN_NPCNUM         '#'
#define LS_TOKEN_STATUS         '?'
#define LS_TOKEN_Q              '*'
#define LS_TOKEN_FAMILY         ':'
#define LS_TOKEN_SHAPE          '@'
#define LS_TOKEN_FRAME          '`'

#define LS_TOKEN_END            '$'

//
// Generating Loopscripts aka Abusing the C Preprocessor
//

//
// Highlevel Function Like Scripts
//
// Usage:
//
// LOOPSCRIPT(script, LS_AND(LS_SHAPE_EQUAL(73), LS_Q_EQUAL(4)));
//

//
// Tokenized Scripts
//
// Usage:
//
// const uint8 script[] = {
//	LS_TOKEN_SHAPE,
//	LS_TOKEN_INT,
//	LS_CONSTANT(73),
//	LS_TOKEN_EQUAL,
//	LS_TOKEN_Q,
//	LS_TOKEN_INT,
//	LS_CONSTANT(4),
//	LS_TOKEN_EQUAL,
//	LS_TOKEN_AND,
//	LS_TOKEN_END
// };
//

#define LOOPSCRIPT(name,tokens) const uint8 name[] = { tokens, LS_TOKEN_END }

#define LS_CONSTANT(val)            ((uint8)(val)), ((uint8)((val)>>8))
#define LS_INT(val)                 LS_TOKEN_INT, LS_CONSTANT(val)

#define LS_OP(left, op, right)      left, right, op

#define LS_NOT(left)                left, LS_TOKEN_NOT
#define LS_AND(left,right)          left, right, LS_TOKEN_AND
#define LS_OR(left,right)           left, right, LS_TOKEN_OR

#define LS_EQUAL(left,right)        left, right, LS_TOKEN_EQUAL
#define LS_LEQUAL(left,right)       left, right, LS_TOKEN_LEQUAL
#define LS_GEQUAL(left,right)       left, right, LS_TOKEN_GEQUAL
#define LS_LESS(left,right)         left, right, LS_TOKEN_LESS
#define LS_GREATER(left,right)      left, right, LS_TOKEN_GREATER

#define LS_FAMILY_EQUAL(val)        LS_EQUAL(LS_TOKEN_FAMILY,LS_INT(val))
#define LS_NPCNUM_EQUAL(val)        LS_EQUAL(LS_TOKEN_NPCNUM,LS_INT(val))
#define LS_STATUS_EQUAL(val)        LS_EQUAL(LS_TOKEN_STATUS,LS_INT(val))
#define LS_Q_EQUAL(val)             LS_EQUAL(LS_TOKEN_Q,LS_INT(val))

#define LS_SHAPE_EQUAL(val)         LS_EQUAL(LS_TOKEN_SHAPE,LS_INT(val))
#define LS_SHAPE_EQUAL1(a)          LS_TOKEN_SHAPE+1, LS_CONSTANT(a)
#define LS_SHAPE_EQUAL2(a,b)        LS_TOKEN_SHAPE+2, LS_CONSTANT(a), LS_CONSTANT(b)
#define LS_SHAPE_EQUAL3(a,b,c)      LS_TOKEN_SHAPE+3, LS_CONSTANT(a), LS_CONSTANT(b), LS_CONSTANT(c)
#define LS_SHAPE_EQUAL4(a,b,c,d)    LS_TOKEN_SHAPE+4, LS_CONSTANT(a), LS_CONSTANT(b), LS_CONSTANT(c), LS_CONSTANT(d)

#define LS_FRAME_EQUAL(val)         LS_EQUAL(LS_TOKEN_FRAME,LS_INT(val))
#define LS_FRAME_EQUAL1(a)          LS_TOKEN_FRAME+1, LS_CONSTANT(a)
#define LS_FRAME_EQUAL2(a,b)        LS_TOKEN_FRAME+2, LS_CONSTANT(a), LS_CONSTANT(b)
#define LS_FRAME_EQUAL3(a,b,c)      LS_TOKEN_FRAME+3, LS_CONSTANT(a), LS_CONSTANT(b), LS_CONSTANT(c)
#define LS_FRAME_EQUAL4(a,b,c,d)    LS_TOKEN_FRAME+4, LS_CONSTANT(a), LS_CONSTANT(b), LS_CONSTANT(c), LS_CONSTANT(d)

#endif
