/***************************************************************************
 int_hashmap. Copyright (C) 2001 Christoph Reichenbach


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

#define BUILD_MAP_FUNCTIONS
#include "sci/include/reg_t_hashmap.h"

#include "sci/scicore/hashmap.cpp"

static inline int
compare_reg_t (reg_t lhs, reg_t rhs)
{
	if (lhs.segment == rhs.segment)
		return lhs.offset - rhs.offset;
	else
		return lhs.segment - rhs.segment;
}

DEFINE_FUNCTIONS(reg_t)
