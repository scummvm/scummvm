/***************************************************************************
 alpha_mvi_crossblit.c Copyright (C) 2001 Christoph Reichenbach


 This program may be modified and copied freely according to the terms of
 the GNU general public license (GPL), as long as the above copyright
 notice and the licensing information contained herein are preserved.

 Please refer to www.gnu.org for licensing details.

 This work is provided AS IS, without warranty of any kind, expressed or
 implied, including but not limited to the warranties of merchantibility,
 noninfringement, and fitness for a specific purpose. The author will not
 be held liable for any damage caused by this work or derivatives of it.

 By using this source code, you agree to the licensing terms as stated
 above.


 Please contact the maintainer for bug reports or inquiries.

 Current Maintainer:

    Christoph Reichenbach (CR) <jameson@linuxgames.com>

***************************************************************************/

/** MUST be compiled with -mcpu=cv6! */

#include <gfx_tools.h>


#ifdef HAVE_ALPHA_EV6_SUPPORT

#ifdef __DECC
#  include "c_asm.h"
#  define USE_C_CODE
#else /* GNU C */
#  undef USE_C_CODE
#endif

void
FUNCT_NAME(byte *dest, byte *src, int bytes_per_dest_line, int bytes_per_src_line,
           int xl, int yl, byte *alpha, int bytes_per_alpha_line, int bytes_per_alpha_pixel,
           unsigned int alpha_test_mask, int alpha_shift
#ifdef PRIORITY
           , byte *priority_pos , int bytes_per_priority_line, int bytes_per_priority_pixel, int priority
#endif
          ) {
#ifdef USE_C_CODE
#ifdef PRIORITY
	int left_mask = (255  << (((unsigned long) dest) & 7)) & 255;
	int right_mask = (255  >> (((unsigned long) dest + bytes_per_dest_line) & 7));
#endif
	unsigned long data;

	assert(!(bytes_per_alpha_line & 7));
	assert(!(((unsigned long) src) & 3));
	assert(!(((unsigned long) dest) & 3));
	assert(bytes_per_alpha_pixel < 2);

	yl++;
	while (--yl) {
		int x;
		byte *dest_next = dest + bytes_per_dest_line;
		byte *src_next = src + bytes_per_src_line;
#ifdef PRIORITY
		byte *pri_next = priority_pos + bytes_per_priority_line;
#endif
		asm("ldl $31, 0($31)\n\t"
		    "ldl $31, 0($31)\n\t"); /* Prefetch memory for next line */

		if (((unsigned long)src) & 4)
			data = *((unsigned int *) src);

		for (x = xl; x > 0; x--) {
			unsigned long alpha;

			if (!(((unsigned long)src) & 4))
				data = *((unsigned long *) src);
			alpha = (data & alpha_test_mask) >> alpha_shift;

			if (
#ifdef PRIORITY
			    (*priority_pos <= priority) &&
#endif
			    alpha != 255) {
				unsigned long result;
				unsigned long orig;
				unsigned long unpkdata;

				unpkdata = asm("unpkbw %0, %v0\n\t", data);

				result = unpkdata * (255 - alpha);

				orig = *((unsigned int *) dest);

				orig = asm("unpkbw %0, %v0\n\t", orig);

				result += orig * alpha;
				src += 4;

				result >>= 8;

				result = asm("pkwb %0, %v0\n\t", result);

				data >>= 32;
				*((unsigned int *) dest) = result;
#ifdef PRIORITY
				*priority_pos = priority;
#endif
				dest += 4;
			} else {
				data >>= 32;
				src += 4;
				dest += 4;
			}

#ifdef PRIORITY
			priority_pos++;
#endif
		}

		dest = dest_next;
		src = src_next;
#ifdef PRIORITY
		priority_pos = pri_next;
#endif
	}
#else
	unsigned long real_alpha_shift = alpha_shift;
#ifdef PRIORITY
	assert(!(bytes_per_alpha_line & 7));
	assert(bytes_per_alpha_pixel < 2);
	real_alpha_shift |= (unsigned long)(priority << 16) | ((unsigned long) bytes_per_priority_pixel << 32);
	/* Work around gcc design bug allowing only 10 asm parameters */
#endif
	__asm__	__volatile__(
#ifdef PRIORITY
	    /*
	    ** dest: $16
	    ** src: $17
	    ** bytes_per_dest_line: $18
	    ** bytes_per_src_line: $19
	    ** xl : $20
	    ** yl : $21
	    ** alpha_test_mask: $24
	    ** alpha_shift: $25
	    ** 255: $8
	    **
	    ** bytes_per_priority_line: $9
	    ** priority_pos: $10
	    ** priority extended to 8 bytes: $7
	    ** bytes_per_priority_pixel: $6
	    **
	    ** temp_priority_collection: $11
	    ** priority_pos backup: $12
	    ** left border mask: $13
	    ** right border mask: $28
	    ** priority test bit: $15
	    ** ldq4priority result: $26
	    */

	    "lda $30, -88($30) \n\t"
	    "stq $9, 0($30)    \n\t"
	    "stq $10, 8($30)   \n\t"
	    "stq $11, 16($30)  \n\t"
	    "stq $12, 24($30)  \n\t"
	    "stq $13, 32($30)  \n\t"
	    "stq $15, 40($30)  \n\t"
	    "stq $26, 48($30)  \n\t"
	    "stq %8, 56($30)   \n\t"
	    "stq %9, 64($30)   \n\t"
	    "stq $7, 72($30)   \n\t"
	    "stq $6, 80($30)   \n\t"

	    "mov %8, $9        \n\t"
	    "mov %9, $10       \n\t"
#endif
	    "mov %6, $24       \n\t"
	    "mov %7, $25       \n\t"
	    "mov 255, $8       \n\t"
	    "subl $21, 1, $21  \n\t"
#ifdef PRIORITY

	    /* Defrobnicate real_alpha_shift data */
	    "srl $25, 32, $6   \n\t"
	    "srl $25, 16, $7   \n\t"
	    "and $7, $8, $7    \n\t"

	    /* Load first priority data quad */
	    "andnot $10, 7, $0 \n\t"
	    "ldq $26, 0($0)    \n\t" /* Load priority */
	    "and $10, 7, $13   \n\t"
	    "sll $13, 3, $0    \n\t"

	    /* Calculate right border mask */
	    "addl $13, $20, $28\n\t"
	    "and $28, 7, $28   \n\t"
	    "beq $28, 7f       \n\t"
	    "mov 8, $0         \n\t"
	    "subl $0, $28, $28 \n"
	    "7:\n\t"
	    "srl $8, $28, $28  \n\t"
	    /* Left border mask */
	    "sll $8, $13, $13  \n\t"
	    "and $13, $8, $13  \n\t"

	    "mov $10, $12      \n\t"

	    "sll $7, 8, $0          \n\t"
	    "or $7, $0, $7          \n\t"
	    "sll $7, 16, $0         \n\t"
	    "or $7, $0, $7          \n\t"
	    "sll $7, 32, $0         \n\t"
	    "or $7, $0, $7          \n\t"
	    "cmpbge $7, $26, $3     \n\t"

	    "and $10, 7, $0         \n\t" /* Init priority bitptr */
	    "mov 1, $15             \n\t" /* .. */
	    "sll $15, $0, $15       \n\t" /* .. */

	    /* -> Priority buffer */
	    "and $3, $13, $11       \n\t"
	    "cmplt $20, 8, $0       \n\t"
	    "beq $0, 6f             \n\t"
	    "and $11, $28, $11      \n\t"
	    "6:\n\t"
#endif
	    "and $25, $8, $25       \n\t"

	    /***/
	    /*** Variable settings apply NOW ***/
	    /***/

	    "mov $20, $1       \n\t"
	    "mov $16, $2       \n\t"
	    "mov $17, $3       \n"
	    "8:\n\t"
	    "addq $2, $18, $2  \n\t"
	    "ldl $31, 0($2)    \n\t" /* Prefetch dest */
	    "addq $3, $19, $3  \n\t"
	    "ldl $31, 0($3)    \n"   /* Prefetch src */
	    "1:\n\t"
	    "addq $17, 4, $17  \n\t"
	    "beq $20, 3f       \n\t"
	    "subl $20, 1, $20  \n\t"
#ifdef PRIORITY
	    "5:\n\t"
	    "addq $10, 1, $10  \n\t"
	    "and $11, $15, $0  \n\t" /* Other priority beat our priority? */
	    "beq $0, 2f        \n\t"
#endif
	    "ldl $0, -4($17)   \n\t"
	    "unpkbw $0, $5     \n\t"
	    "and $0, $24, $0   \n\t"

	    "xor $0, $24, $4   \n\t"
	    "beq $4, 2f        \n\t"

	    "ldl $4, 0($16)    \n\t"
	    "unpkbw $4, $4     \n\t"
	    "srl $0, $25, $0   \n\t"
	    "mulq $4, $0, $4   \n\t"
	    "subl $8, $0, $0   \n\t"
	    "mulq $5, $0, $5   \n\t"
	    "addq  $4, $5, $4  \n\t"
	    "srl $4, 8, $4     \n\t"
	    "pkwb $4, $0       \n\t"
	    "stl $0, 0($16)    \n\t"
	    "br 9f             \n\t"
	    "2:\n"
	    "andnot $11, $15, $11 \n\t" /* Don't draw priority if we're fully transparent */
	    "9:\n\t"
	    "addq $16, 4, $16  \n\t"
#ifdef PRIORITY
	    "sll $15, 1, $15     \n\t"

	    "and $10, 7, $0    \n\t" /* Do we need to re-load priority mask? */
	    /**/		 "bne $0, 1b        \n\t"

	    /* Write back to priority buffer */
	    "zap $26, $11, $26      \n\t"
	    "zapnot $7, $11, $0     \n\t"
	    "or $0, $26, $0         \n\t"
	    "stq $0, -8($10)        \n\t"

	    "ldq $26, 0($10)    \n\t"  /* Load priority */
	    "cmpbge $7, $26, $11\n\t"

	    "mov 1, $15         \n\t" /* Init bitcmpmask */

	    "cmplt $20, 8, $0  \n\t"
	    /**/		 "beq $0, 1b        \n\t"
	    "and $11, $28, $11 \n\t"
#endif
	    "br 1b             \n"
	    "3:\n\t"
#ifdef PRIORITY
	    "and $10, 7, $16        \n\t"
	    "beq $16, 7f            \n\t"
	    "and $11, $28, $11      \n\t"
	    "zap $26, $11, $26      \n\t"
	    "zapnot $7, $11, $0     \n\t"
	    "or $0, $26, $0         \n\t"
	    "andnot $10, 7, $16     \n\t"
	    "stq $0, 0($16)         \n" /* Write back */
	    "7:\n\t"

	    "addq $9, $12, $12 \n\t"
	    "mov $12, $10      \n\t"
	    "andnot $10, 7, $0 \n\t"
	    "ldq $26, 0($0)    \n\t"

	    "and $10, 7, $0         \n\t"
	    "mov 1, $15             \n\t"
	    "sll $15, $0, $15       \n\t" /* Store priority-induced write-enable mask */

	    "cmpbge $7, $26, $16    \n\t"
	    /* -> Priority buffer */
	    "and $16, $13, $11      \n\t"
#endif
	    "beq $21, 4f       \n\t"
	    "subl $21, 1, $21  \n\t"
	    "mov $1, $20       \n\t"
	    "mov $2, $16       \n\t" /* Set line numbers for next line */
	    "mov $3, $17       \n\t"
	    "br 8b             \n"
	    "4:\n\t"
#ifdef PRIORITY
	    "ldq $9, 0($30)    \n\t"
	    "ldq $10, 8($30)   \n\t"
	    "ldq $11, 16($30)  \n\t"
	    "ldq $12, 24($30)  \n\t"
	    "ldq $13, 32($30)  \n\t"
	    "ldq $15, 40($30)  \n\t"
	    "ldq $26, 48($30)  \n\t"
	    "ldq %8, 56($30)   \n\t"
	    "ldq %9, 64($30)   \n\t"
	    "ldq $7, 72($30)   \n\t"
	    "ldq $6, 80($30)   \n\t"
	    "lda $30, 88($30)  \n\t"
#endif
    :
    :
	    "r"(dest), "r"(src), "r"(bytes_per_dest_line),
	    /*3*/ "r"(bytes_per_src_line), "r"(xl), "r"(yl),
	    /*6*/ "r"((unsigned long) alpha_test_mask), "r"(real_alpha_shift)
#ifdef PRIORITY
	    , "r"(bytes_per_priority_line), "r"(priority_pos)
#endif
			    : "%0", "%1", "%2", "%3", "%4", "%5", "%6",
			    "%7", "$16", "$17", "$18", "$19", "$20", "$21", "$24", "$25",
			    "$8", "memory"
			);
#endif
}

#endif /* __alpha__ */
