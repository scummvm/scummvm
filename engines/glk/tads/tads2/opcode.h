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

#ifndef GLK_TADS_TADS2_OPCODE
#define GLK_TADS_TADS2_OPCODE

/*
 * Opcode definitions
 *
 * Lifted largely from the old TADS, since the basic run - time interpreter's
 * operation is essentially the same.
 */

#include "glk/tads/tads.h"
#include "glk/tads/tads2/data.h"

namespace Glk {
namespace TADS {
namespace TADS2 {

#define OPCPUSHNUM  1       /* push a constant numeric value */
#define OPCPUSHOBJ  2       /* push an object */
#define OPCNEG      3       /* unary negation */
#define OPCNOT      4       /* logical negation */
#define OPCADD      5       /* addition/list concatenation */
#define OPCSUB      6       /* subtraction/list difference */
#define OPCMUL      7       /* multiplication */
#define OPCDIV      8       /* division */
#define OPCAND      9       /* logical AND */
#define OPCOR       10      /* logical OR */
#define OPCEQ       11      /* equality */
#define OPCNE       12      /* inequality */
#define OPCGT       13      /* greater than */
#define OPCGE       14      /* greater or equal */
#define OPCLT       15      /* less than */
#define OPCLE       16      /* less or equal */
#define OPCCALL     17      /* call a function */
#define OPCGETP     18      /* get property */
#define OPCGETPDATA 19      /* get a property, allowing only data values */
#define OPCGETLCL   20      /* get a local variable's value */
#define OPCPTRGETPDATA 21   /* get property via pointer; only allow data */   
#define OPCRETURN   22      /* return without a value */
#define OPCRETVAL   23      /* return a value */
#define OPCENTER    24      /* enter a function */
#define OPCDISCARD  25      /* discard top of stack */
#define OPCJMP      26      /* unconditional jump */
#define OPCJF       27      /* jump if false */
#define OPCPUSHSELF 28      /* push current object */
#define OPCSAY      29      /* implicit printout for doublequote strings */
#define OPCBUILTIN  30      /* call a built-in function */
#define OPCPUSHSTR  31      /* push a string */
#define OPCPUSHLST  32      /* push a list */
#define OPCPUSHNIL  33      /* push the NIL value */
#define OPCPUSHTRUE 34      /* push the TRUE value */
#define OPCPUSHFN   35      /* push the address of a function */
#define OPCGETPSELFDATA 36  /* push property of self; only allow data */

#define OPCPTRCALL  38          /* call function pointed to by top of stack */
#define OPCPTRINH   39          /* inherit pointer to property (stack=prop) */
#define OPCPTRGETP  40          /* get property by pointer (stack=obj,prop) */

#define OPCPASS     41      /* pass to inherited handler */
#define OPCEXIT     42      /* exit turn, but continue with fuses/daemons */
#define OPCABORT    43      /* abort turn, skipping fuses/daemons */
#define OPCASKDO    44      /* ask for a direct object */
#define OPCASKIO    45      /* ask for indirect object and set preposition */

/* explicit superclass inheritance opcodes */
#define OPCEXPINH   46      /* "inherited <superclass>.<property>" */
#define OPCEXPINHPTR 47     /* "inherited <superclass>.<prop-pointer>" */

/*
 *   Special opcodes for peephole optimization.  These are essentially
 *   pairs of operations that occur frequently so have been collapsed into
 *   a single instruction.
 */
#define OPCCALLD    48      /* call function and discard value */
#define OPCGETPD    49      /* evaluate property and discard any value */
#define OPCBUILTIND 50      /* call built-in function and discard value */

#define OPCJE       51      /* jump if equal */
#define OPCJNE      52      /* jump if not equal */
#define OPCJGT      53      /* jump if greater than */
#define OPCJGE      54      /* jump if greater or equal */
#define OPCJLT      55      /* jump if less than */
#define OPCJLE      56      /* jump if less or equal */
#define OPCJNAND    57      /* jump if not AND */
#define OPCJNOR     58      /* jump if not OR */
#define OPCJT       59      /* jump if true */

#define OPCGETPSELF 60      /* get property of the 'self' object */
#define OPCGETPSLFD 61      /* get property of 'self' and discard result */
#define OPCGETPOBJ  62      /* get property of a given object */
                            /*  note: differs from GETP in that object is */
                            /*  encoded into the instruction */
#define OPCGETPOBJD 63      /* get property of an object and discard result */
#define OPCINDEX    64      /* get an indexed entry from a list */

#define OPCPUSHPN   67      /* push a property number */

#define OPCJST      68      /* jump and save top-of-stack if true */
#define OPCJSF      69      /* jump and save top-of-stack if false */
#define OPCJMPD     70      /* discard stack and then jump unconditionally */

#define OPCINHERIT  71      /* inherit a property from superclass */
#define OPCCALLEXT  72      /* call external function */
#define OPCDBGRET   73      /* return to debugger (no stack frame leaving) */

#define OPCCONS     74      /* construct list from top two stack elements */
#define OPCSWITCH   75      /* switch statement */

#define OPCARGC     76      /* get argument count */
#define OPCCHKARGC  77      /* check actual arguments against formal count */

#define OPCLINE     78      /* line record */
#define OPCFRAME    79      /* local variable frame record */
#define OPCBP       80      /* breakpoint - replaces an OPCLINE instruction */
#define OPCGETDBLCL 81                                /* get debugger local */
#define OPCGETPPTRSELF 82                 /* get property pointer from self */
#define OPCMOD      83                                            /* modulo */
#define OPCBAND     84                                        /* binary AND */
#define OPCBOR      85                                         /* binary OR */
#define OPCXOR      86                                        /* binary XOR */
#define OPCBNOT     87                                   /* binary negation */
#define OPCSHL      88                                    /* bit shift left */
#define OPCSHR      89                                   /* bit shift right */

#define OPCNEW      90                                 /* create new object */
#define OPCDELETE   91                                     /* delete object */


/* ----- opcodes 192 and above are reserved for assignment operations ----- */

/*
ASSIGNMENT OPERATIONS
    When (opcode & 0xc0 == 0xc0), we have an assignment operation.
    (Note that this means that opcodes from 0xc0 up are all reserved
    for assignment operations.)  The low six bits of the opcode
    specify exactly what kind of operation is to be performed:
    
    bits 0-1:  specifies destination type:
               00    2-byte operand is local number
               01    2-byte operand is property to set in obj at tos
               10    tos is index, [sp-1] is list to be indexed and set
               11    tos is property pointer, [sp-1] is object
    
    bits 2-4:  specifies assignment operation:
               000   := (direct assignment)
               001   += (add tos to destination)
               010   -= (subtract tos from destination)
               011   *= (multiply destination by tos)
               100   /= (divide destination by tos)
               101   ++ (increment tos)
               110   -- (decrement tos)
               111   *reserved*
               
    bit 5:     specifies what to do with value computed by assignment
               0     leave on stack (implies pre increment/decrement)
               1     discard (implies post increment/decrement)
*/
#define OPCASI_MASK      0xc0                     /* assignment instruction */

#define OPCASIDEST_MASK  0x03              /* mask to get destination field */
#define OPCASILCL        0x00                          /* assign to a local */
#define OPCASIPRP        0x01               /* assign to an object.property */
#define OPCASIIND        0x02             /* assign to an element of a list */
#define OPCASIPRPPTR     0x03                /* assign property via pointer */

#define OPCASITYP_MASK   0x1c          /* mask to get assignment type field */
#define OPCASIDIR        0x00                          /* direct assignment */
#define OPCASIADD        0x04                             /* assign and add */
#define OPCASISUB        0x08                        /* assign and subtract */
#define OPCASIMUL        0x0c                        /* assign and multiply */
#define OPCASIDIV        0x10                          /* assign and divide */
#define OPCASIINC        0x14                                  /* increment */
#define OPCASIDEC        0x18                                  /* decrement */
#define OPCASIEXT        0x1c                     /* other - extension flag */

/* extended assignment flags - next byte when OPCASIEXT is used */
#define OPCASIMOD        1                             /* modulo and assign */
#define OPCASIBAND       2                         /* binary AND and assign */
#define OPCASIBOR        3                          /* binary OR and assign */
#define OPCASIXOR        4                         /* binary XOR and assign */
#define OPCASISHL        5                         /* shift left and assign */
#define OPCASISHR        6                        /* shift right and assign */


#define OPCASIPRE_MASK   0x20                    /* mask for pre/post field */
#define OPCASIPOST       0x00                       /* increment after push */
#define OPCASIPRE        0x20                      /* increment before push */

/* some composite opcodes for convenience */
#define OPCSETLCL (OPCASI_MASK | OPCASILCL | OPCASIDIR)

} // End of namespace TADS2
} // End of namespace TADS
} // End of namespace Glk

#endif
