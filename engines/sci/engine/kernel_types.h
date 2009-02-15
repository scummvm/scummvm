/***************************************************************************
 kernel_types.h  Copyright (C) 2002 Christoph Reichenbach


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

#ifndef _FREESCI_KERNEL_TYPES_H_
#define _FREESCI_KERNEL_TYPES_H_

#include "sci/include/vm_types.h"

#define KSIG_TERMINATOR 0

/* Uncompiled signatures */
#define KSIG_SPEC_ARITMETIC 'i'
#define KSIG_SPEC_LIST 'l'
#define KSIG_SPEC_NODE 'n'
#define KSIG_SPEC_OBJECT 'o'
#define KSIG_SPEC_REF 'r' /* Said Specs and strings */
#define KSIG_SPEC_ARITHMETIC 'i'
#define KSIG_SPEC_NULL 'z'
#define KSIG_SPEC_ANY '.'
#define KSIG_SPEC_ALLOW_INV '!' /* Allow invalid pointers */
#define KSIG_SPEC_ELLIPSIS '*' /* Arbitrarily more TYPED arguments */

#define KSIG_SPEC_SUM_DONE ('a' - 'A') /* Use small letters to indicate end of sum type */
/* Use capital letters for sum types, e.g.
** "LNoLr" for a function which takes two arguments:
** (1) list, node or object
** (2) list or ref
*/

/* Compiled signatures */
#define KSIG_LIST	0x01
#define KSIG_NODE	0x02
#define KSIG_OBJECT	0x04
#define KSIG_REF	0x08
#define KSIG_ARITHMETIC 0x10

#define KSIG_NULL	0x40
#define KSIG_ANY	0x5f
#define KSIG_ELLIPSIS	0x80
#define KSIG_ALLOW_INV  0x20
#define KSIG_INVALID	KSIG_ALLOW_INV


int
kernel_matches_signature(state_t *s, const char *sig, int argc, reg_t *argv);
/* Determines whether a list of registers matches a given signature
** Parameters: (state_t *) s: The state to operate on
**             (char *) sig: The signature to test against
**             (int) argc: Number of arguments to test
**             (reg_t *) argv: Argument list
** Returns   : (int) 0 iff the signature was not matched
*/

int
determine_reg_type(state_t *s, reg_t reg, int allow_invalid);
/* Determines the type of the object indicated by reg
** Parameters: (state_t *) s: The state to operate on
**             (reg_t) reg: The register to check
**	       (int) allow_invalid: Allow invalid pointer values
** Returns   : one of KSIG_* below KSIG_NULL.
**	       KSIG_INVALID set if the type of reg can be determined, but is invalid.
**	       0 on error.
*/

const char *
kernel_argtype_description(int type);
/* Returns a textual description of the type of an object
** Parameters: (int) type: The type value to describe
** Returns: (const char *) Pointer to a (static) descriptive string
*/

#endif /* ! _FREESCI_KERNEL_TYPES_H_ */
