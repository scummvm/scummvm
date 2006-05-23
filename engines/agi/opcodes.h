/* ScummVM - Scumm Interpreter
 * Copyright (C) 2006 The ScummVM project
 *
 * Copyright (C) 1999-2001 Sarien Team
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#ifndef __AGI_OPCODES_H
#define __AGI_OPCODES_H

#include "agi/agi.h"

namespace Agi {

struct agi_logicnames {
#ifdef USE_CONSOLE		/* ifndef NO_DEBUG */
	char *name;
#endif
	uint16 num_args;
	uint16 arg_mask;
};

extern struct agi_logicnames logic_names_test[];
extern struct agi_logicnames logic_names_cmd[];
extern struct agi_logicnames logic_names_if[];

void debug_console(int, int, char *);
int test_if_code(int);
void new_room(int);
void execute_agi_command(uint8, uint8 *);

#ifdef PATCH_LOGIC
void patch_logic(int);
#endif

}                             // End of namespace Agi

#endif				/* __AGI_OPCODES_H */
