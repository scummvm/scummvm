/* ScummVMDS - Scumm Interpreter DS Port
 * Copyright (C) 2002-2004 The ScummVM project and Neil Millstone
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 */
 
#include "stdafx.h"
#define CHARSET_MASK_TRANSPARENCY 253

namespace DS {

void asmDrawStripToScreen(int height, int width, byte const* text, byte const* src, byte* dst, 
	int vsPitch, int vmScreenWidth, int textSurfacePitch) {


	if (height <= 0) height = 1;
	if (width < 4) return;
	
	
	width &= ~4;
//	src = (const byte *) (((int) (src)) & (~4));
//	dst = (byte *) (((int) (dst)) & (~4));
//	text = (const byte *) (((int) (text)) & (~4));
	
	asm (	"mov r5, %0\n"				// Height
			"yLoop:\n"			
			"mov r3, #0\n"				// X pos
			
			"xLoop:\n"
			
			"ldr r4, [%2, r3]\n"		// Load text layer word
			"cmp r4, %5\n"
			"bne singleByteCompare\n"
			"ldr r4, [%3, r3]\n"
			"str r4, [%4, r3]\n"
			"add r3, r3, #4\n"
			"cmp r3, %1\n"				// x == width?
			"blt xLoop\n"
			
			"add %2, %2, %8\n"			// src += vs->pitch
			"add %3, %3, %6\n"			// dst += _vm->_screenWidth
			"add %4, %4, %7\n"			// text += _textSurface.pitch
			"sub r5, r5, #1\n"			// y -= 1
			"cmp r5, #0\n"				// y == 0?
			"bne yLoop\n"
			"b end\n"
			
			
			"singleByteCompare:\n"
			"ldrb r4, [%2, r3]\n"		// Load text byte
			"cmps r4, %5, lsr #24\n"	// Compare with mask
			"strneb r4, [%4, r3]\n"		// Store if not equal
			"ldreqb r4, [%3, r3]\n"		// Otherwise Load src byte
			"streqb r4, [%4, r3]\n"		// Store it
			"add r3, r3, #1\n"

			"ldrb r4, [%2, r3]\n"		// Load text byte
			"cmps r4, %5, lsr #24\n"	// Compare with mask
			"strneb r4, [%4, r3]\n"		// Store if not equal
			"ldreqb r4, [%3, r3]\n"		// Otherwise Load src byte
			"streqb r4, [%4, r3]\n"		// Store it
			"add r3, r3, #1\n"
			
			"ldrb r4, [%2, r3]\n"		// Load text byte
			"cmps r4, %5, lsr #24\n"	// Compare with mask
			"strneb r4, [%4, r3]\n"		// Store if not equal
			"ldreqb r4, [%3, r3]\n"		// Otherwise Load src byte
			"streqb r4, [%4, r3]\n"		// Store it
			"add r3, r3, #1\n"

			"ldrb r4, [%2, r3]\n"		// Load text byte
			"cmps r4, %5, lsr #24\n"	// Compare with mask
			"strneb r4, [%4, r3]\n"		// Store if not equal
			"ldreqb r4, [%3, r3]\n"		// Otherwise Load src byte
			"streqb r4, [%4, r3]\n"		// Store it
			"add r3, r3, #1\n"			

			"cmps r3, %1\n"				// x == width?
			"blt xLoop\n"				// Repeat
			"add %2, %2, %8\n"			// src += vs->pitch
			"add %3, %3, %6\n"			// dst += _vm->_screenWidth
			"add %4, %4, %7\n"			// text += _textSurface.pitch
			"sub r5, r5, #1\n"			// y -= 1
			"cmp r5, #0\n"				// y == 0?
			"bne yLoop\n"
			
			"end:\n"
		: /* no output registers */
		: "r" (height), "r" (width), "r" (text), "r" (src), "r" (dst), "r" (CHARSET_MASK_TRANSPARENCY | (CHARSET_MASK_TRANSPARENCY << 8) | (CHARSET_MASK_TRANSPARENCY << 16) | (CHARSET_MASK_TRANSPARENCY << 24)), 
			"r" (vsPitch), "r" (vmScreenWidth), "r" (textSurfacePitch)
		: "r5", "r3", "r4", "%2", "%3", "%4", "memory");
}



void asmCopy8Col(byte* dst, int dstPitch, const byte* src, int height) {
	asm("ands r0, %3, #1\n"
		 "addne %3, %3, #1\n"
		 "bne roll2\n"
			
		 "yLoop2:\n"
		 "ldr r0, [%2, #0]\n"
		 "str r0, [%0, #0]\n"
		 "ldr r0, [%2, #4]\n"
		 "str r0, [%0, #4]\n"
		 "add %0, %0, %1\n"
		 "add %2, %2, %1\n"
		 "roll2:\n"
		 "ldr r0, [%2, #0]\n"
		 "str r0, [%0, #0]\n"
		 "ldr r0, [%2, #4]\n"
		 "str r0, [%0, #4]\n"
		 "add %0, %0, %1\n"
		 "add %2, %2, %1\n"
		 "subs %3, %3, #2\n"
		 "bne yLoop2\n"		 

		: /* no output registers */
		: "r" (dst), "r" (dstPitch), "r" (src), "r" (height)
		: "r0", "%0", "%2", "%3");
}

}
/* ScummVMDS - Scumm Interpreter DS Port
 * Copyright (C) 2002-2004 The ScummVM project and Neil Millstone
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 */
 
#include "stdafx.h"
#define CHARSET_MASK_TRANSPARENCY 253

namespace DS {

void asmDrawStripToScreen(int height, int width, byte const* text, byte const* src, byte* dst, 
	int vsPitch, int vmScreenWidth, int textSurfacePitch) {


	if (height <= 0) height = 1;
	if (width < 4) return;
	
	
	width &= ~4;
//	src = (const byte *) (((int) (src)) & (~4));
//	dst = (byte *) (((int) (dst)) & (~4));
//	text = (const byte *) (((int) (text)) & (~4));
	
	asm (	"mov r5, %0\n"				// Height
			"yLoop:\n"			
			"mov r3, #0\n"				// X pos
			
			"xLoop:\n"
			
			"ldr r4, [%2, r3]\n"		// Load text layer word
			"cmp r4, %5\n"
			"bne singleByteCompare\n"
			"ldr r4, [%3, r3]\n"
			"str r4, [%4, r3]\n"
			"add r3, r3, #4\n"
			"cmp r3, %1\n"				// x == width?
			"blt xLoop\n"
			
			"add %2, %2, %8\n"			// src += vs->pitch
			"add %3, %3, %6\n"			// dst += _vm->_screenWidth
			"add %4, %4, %7\n"			// text += _textSurface.pitch
			"sub r5, r5, #1\n"			// y -= 1
			"cmp r5, #0\n"				// y == 0?
			"bne yLoop\n"
			"b end\n"
			
			
			"singleByteCompare:\n"
			"ldrb r4, [%2, r3]\n"		// Load text byte
			"cmps r4, %5, lsr #24\n"	// Compare with mask
			"strneb r4, [%4, r3]\n"		// Store if not equal
			"ldreqb r4, [%3, r3]\n"		// Otherwise Load src byte
			"streqb r4, [%4, r3]\n"		// Store it
			"add r3, r3, #1\n"

			"ldrb r4, [%2, r3]\n"		// Load text byte
			"cmps r4, %5, lsr #24\n"	// Compare with mask
			"strneb r4, [%4, r3]\n"		// Store if not equal
			"ldreqb r4, [%3, r3]\n"		// Otherwise Load src byte
			"streqb r4, [%4, r3]\n"		// Store it
			"add r3, r3, #1\n"
			
			"ldrb r4, [%2, r3]\n"		// Load text byte
			"cmps r4, %5, lsr #24\n"	// Compare with mask
			"strneb r4, [%4, r3]\n"		// Store if not equal
			"ldreqb r4, [%3, r3]\n"		// Otherwise Load src byte
			"streqb r4, [%4, r3]\n"		// Store it
			"add r3, r3, #1\n"

			"ldrb r4, [%2, r3]\n"		// Load text byte
			"cmps r4, %5, lsr #24\n"	// Compare with mask
			"strneb r4, [%4, r3]\n"		// Store if not equal
			"ldreqb r4, [%3, r3]\n"		// Otherwise Load src byte
			"streqb r4, [%4, r3]\n"		// Store it
			"add r3, r3, #1\n"			

			"cmps r3, %1\n"				// x == width?
			"blt xLoop\n"				// Repeat
			"add %2, %2, %8\n"			// src += vs->pitch
			"add %3, %3, %6\n"			// dst += _vm->_screenWidth
			"add %4, %4, %7\n"			// text += _textSurface.pitch
			"sub r5, r5, #1\n"			// y -= 1
			"cmp r5, #0\n"				// y == 0?
			"bne yLoop\n"
			
			"end:\n"
		: /* no output registers */
		: "r" (height), "r" (width), "r" (text), "r" (src), "r" (dst), "r" (CHARSET_MASK_TRANSPARENCY | (CHARSET_MASK_TRANSPARENCY << 8) | (CHARSET_MASK_TRANSPARENCY << 16) | (CHARSET_MASK_TRANSPARENCY << 24)), 
			"r" (vsPitch), "r" (vmScreenWidth), "r" (textSurfacePitch)
		: "r5", "r3", "r4", "%2", "%3", "%4", "memory");
}



void asmCopy8Col(byte* dst, int dstPitch, const byte* src, int height) {
	asm("ands r0, %3, #1\n"
		 "addne %3, %3, #1\n"
		 "bne roll2\n"
			
		 "yLoop2:\n"
		 "ldr r0, [%2, #0]\n"
		 "str r0, [%0, #0]\n"
		 "ldr r0, [%2, #4]\n"
		 "str r0, [%0, #4]\n"
		 "add %0, %0, %1\n"
		 "add %2, %2, %1\n"
		 "roll2:\n"
		 "ldr r0, [%2, #0]\n"
		 "str r0, [%0, #0]\n"
		 "ldr r0, [%2, #4]\n"
		 "str r0, [%0, #4]\n"
		 "add %0, %0, %1\n"
		 "add %2, %2, %1\n"
		 "subs %3, %3, #2\n"
		 "bne yLoop2\n"		 

		: /* no output registers */
		: "r" (dst), "r" (dstPitch), "r" (src), "r" (height)
		: "r0", "%0", "%2", "%3");
}

}
